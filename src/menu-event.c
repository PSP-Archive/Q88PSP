/************************************************************************/
/*									*/
/* メニューモードにおける、イベントの取得 (OS依存)			*/
/*									*/
/* QUASI88 のメニューモード用ツールキット Q8TK では「マウスボタン押下」	*/
/* 「マウスボタン開放」「マウス移動」「キーボード押下」「強制終了」の	*/
/* いずれかの発生を待つので、これらが発生するまで待機し、発生次第通知	*/
/* する関数が必要である。						*/
/*									*/
/*									*/
/* 【関数】								*/
/*									*/
/* void	menu_event_init( void )						*/
/*	・menu_check_event()などの関数の処理に必要な初期化を、Q8TK の	*/
/*	  初期化に先だって行なう。					*/
/*									*/
/* void menu_event_term( void )						*/
/*	・Q8TK の終了後に menu_event_init() で初期化した設定を破棄する。*/
/*	  また、同時に QUASI88 のエミュレートモードに応じた設定を	*/
/*	  ここで再度行なう。						*/
/*									*/
/*									*/
/* void	menu_check_event( int *type, int *button, int *x, int *y )	*/
/*	・以下のイベントが発生するまで待つ。				*/
/*		MENU_EVENT_KEY_ON	… キーが押された		*/
/*		MENU_EVENT_MOUSE_ON	… マウスボタンが押された	*/
/*		MENU_EVENT_MOUSE_OFF	… マウスホタンが離された	*/
/*		MENU_EVENT_MOUSE_MOVE	… マウスが移動した		*/
/*		MENU_EVENT_QUIT		… Ctrl-C シグナルを検知した	*/
/*	・これらのイベントが発生すると、				*/
/*		○ *type にイベント番号 (MENU_EVENT_XXX) をセット	*/
/*		○キーが押された場合は *button に おされたキーの	*/
/*		  ASCIIコードをセット					*/
/*		○マウスが押されたら *button におされたボタンの番号	*/
/*		  (Q8TK_BUTTON_L/Q8TK_BUTTON_R) をセット		*/
/*		○マウスが離されたら *button に離されたボタンの番号	*/
/*		  (Q8TK_BUTTON_L/Q8TK_BUTTON_R) をセット		*/
/*		○マウスが移動したら、*x、*y にマウスの座標をセット	*/
/*	  して、関数を終える。						*/
/*	・イベントが発生するまで、この関数は終了しない。		*/
/*									*/
/* void	menu_clear_event( void )					*/
/*	・先行入力されると鬱陶しいイベント(文字入力など)をクリアする。	*/
/*	  べつに何もしなくても問題無い。逆に先行入力させたいのなら、	*/
/*	  なにもしてはいけない。					*/
/*									*/
/* void	menu_quit_signal( void )					*/
/*	・MENU_EVENT_QUIT のイベントを生成する。			*/
/*	  この関数は、Ctrl-C が押された時に、そのシグナルハンドラから	*/
/*	  呼ばれる。							*/
/*									*/
/*									*/
/* 【流れ】								*/
/*									*/
/*	{								*/
/*	  menu_event_init();						*/
/*	  q8tk_init();							*/
/*	  q8tk設定;							*/
/*	  q8tk_main();		←内部で menu_check_event()を呼んでいる	*/
/*	  q8tk_term();							*/
/*	  menu_event_term();						*/
/*	}								*/
/*									*/
/************************************************************************/

#include <ctype.h>

#include <SDL.h>

#include "quasi88.h"
#include "menu-event.h"

#include "graph.h"		/* screen_size  need in mouse() */
#include "device.h"
#include "q8tk.h"
#include "menu-screen.h"	/* menu_redraw_screen() etc */

#include "emu.h"

/************************************************************************/
/* メニューモード開始時の処理						*/
/************************************************************************/
void	menu_event_init( void )
{
  /* キー押下を ASCII コードに変換可能とする (処理が重いらしいのて一時的に) */
  SDL_EnableUNICODE(1);

  /* オートリピート ON */
  SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );
}

/************************************************************************/
/* メニューモード終了時の処理						*/
/************************************************************************/
void	menu_event_term( void )
{
  /* キー押下を ASCII コードに変換可能とする */
  SDL_EnableUNICODE(0);

  /* オートリピート OFF */
  SDL_EnableKeyRepeat( 0, 0 );
}


/************************************************************************/
/* MENU_EVENT_QUIT イベントを生成					*/
/*	SIGINT (Ctrl-C) 発生時に呼ばれる → monitor.c			*/
/************************************************************************/
void	menu_quit_signal( void )
{
}

/************************************************************************/
/* 先行入力されると鬱陶しいイベント(文字入力など)をクリアする。		*/
/*	先行入力を防止するものだが、先行入力したい時は、別になにも	*/
/*	しない関数にしよう。						*/
/************************************************************************/
void	menu_clear_event( void )
{
#if 0
  XEvent E;
  while( XCheckWindowEvent( display, window,
			    ExposureMask|KeyPressMask|PointerMotionMask,
			    &E ) );
#endif
}


/************************************************************************/
/* イベントのチェックに先だっての、マウス座標を初期値を得る		*/
/*	○マウスの現在の座標を返す					*/
/************************************************************************/
void	menu_init_event( int *win_x, int *win_y )
{
  SDL_GetMouseState( win_x, win_y );

  if( screen_size == SCREEN_SIZE_HALF ){	/* 画面サイズ半分の時は、 */
    *win_x *= 2;				/* マウスの座標を倍にする */
    *win_y *= 2;
  }else if( screen_size == SCREEN_SIZE_DOUBLE ){
    *win_x /= 2;
    *win_y /= 2;
  }
}


/************************************************************************/
/* 各種イベントが発生すると、そのイベントを返す関数			*/
/*	○キー ON、マウスボタン ON/OFF 、マウス移動、SIGINT (Ctrl-C) の	*/
/*	  発生を通知する。						*/
/*	○画面が再露出したとき、再描画する				*/
/************************************************************************/
void	menu_check_event( int *type, int *button, int *x, int *y )
{
  SDL_Event E;
  int	mx, my;

  SDL_PumpEvents();

#if 1
  if( SDL_PeepEvents(&E, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_QUIT)) ){
      if (E.type == SDL_QUIT) {
	  emu_mode = QUIT;
	  *type = MENU_EVENT_QUIT;
	  return;
      }
  }
#endif
  if( SDL_PeepEvents(&E, 1, SDL_GETEVENT,
		     SDL_EVENTMASK(SDL_KEYDOWN)|SDL_EVENTMASK(SDL_KEYUP)|
		     SDL_EVENTMASK(SDL_MOUSEMOTION)|
		     SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)|
		     SDL_EVENTMASK(SDL_MOUSEBUTTONUP)|
		     SDL_EVENTMASK(SDL_VIDEOEXPOSE)) ){

    /* イベントが起こらなくても即座に返ってきてしまう……… */

      switch( E.type ){

	  case SDL_VIDEOEXPOSE:
	      menu_redraw_screen();
	      break;
	  case SDL_KEYDOWN:
	      *type = MENU_EVENT_KEY_ON;
	      switch( E.key.keysym.sym ){
		  case SDLK_LEFT:	*button = Q8TK_KEY_LEFT;	return;
		  case SDLK_RIGHT:	*button = Q8TK_KEY_RIGHT;	return;
		  case SDLK_UP:		*button = Q8TK_KEY_UP;		return;
		  case SDLK_DOWN:	*button = Q8TK_KEY_DOWN;	return;
		  case SDLK_PAGEUP:	*button = Q8TK_KEY_PAGE_UP;	return;
		  case SDLK_PAGEDOWN:	*button = Q8TK_KEY_PAGE_DOWN;	return;
		  case SDLK_TAB:        *button = Q8TK_KEY_TAB;		return;
		  case SDLK_ESCAPE:	*button = Q8TK_KEY_ESC;		return;
		  case SDLK_RETURN:
		  case SDLK_KP_ENTER:	*button = Q8TK_KEY_RET;		return;
		  case SDLK_BACKSPACE:	*button = Q8TK_KEY_BS;		return;
		      
		  case SDLK_F1:		*button = Q8TK_KEY_F1;		return;
		  case SDLK_F2:		*button = Q8TK_KEY_F2;		return;
		  case SDLK_F3:		*button = Q8TK_KEY_F3;		return;
		  case SDLK_F4:		*button = Q8TK_KEY_F4;		return;
		  case SDLK_F5:		*button = Q8TK_KEY_F5;		return;
		  case SDLK_F6:		*button = Q8TK_KEY_F6;		return;
		  case SDLK_F7:		*button = Q8TK_KEY_F7;		return;
		  case SDLK_F8:		*button = Q8TK_KEY_F8;		return;
		  case SDLK_F9:		*button = Q8TK_KEY_F9;		return;
		  case SDLK_F10:	*button = Q8TK_KEY_F10;		return;
		  case SDLK_F11:	*button = Q8TK_KEY_F11;		return;
		  case SDLK_F12:	*button = Q8TK_KEY_F12;		return;
		  default:
		    if( E.key.keysym.unicode <= 0xff &&
			isprint( E.key.keysym.unicode ) ){
			*button = E.key.keysym.unicode;			return;
		    }
	      }
	      *type = MENU_EVENT_NOTHING;
	      break;

	  case SDL_MOUSEBUTTONDOWN:
	      *type = MENU_EVENT_MOUSE_ON;
	      switch( E.button.button ){
	      case SDL_BUTTON_LEFT:	*button = Q8TK_BUTTON_L;	return;
	      case SDL_BUTTON_RIGHT:	*button = Q8TK_BUTTON_R;	return;
	      case SDL_BUTTON_WHEELUP:	*button = Q8TK_BUTTON_U;	return;
	      case SDL_BUTTON_WHEELDOWN:*button = Q8TK_BUTTON_D;	return;
	      }
	      *type = MENU_EVENT_NOTHING;
	      return;
	      
	  case SDL_MOUSEBUTTONUP:
	      *type = MENU_EVENT_MOUSE_OFF;
	      switch( E.button.button ){
	      case SDL_BUTTON_LEFT:	*button = Q8TK_BUTTON_L;	return;
	      case SDL_BUTTON_RIGHT:	*button = Q8TK_BUTTON_R;	return;
	      case SDL_BUTTON_WHEELUP:	*button = Q8TK_BUTTON_U;	return;
	      case SDL_BUTTON_WHEELDOWN:*button = Q8TK_BUTTON_D;	return;
	      }
	      *type = MENU_EVENT_NOTHING;
	      return;
	      
	  case SDL_MOUSEMOTION:
#if 0
	      mx = E.motion.x;
	      my = E.motion.y;
#else
	      mouse_coord_trans(E.motion.x, E.motion.y, &mx, &my);
#endif	      
	      /* printf("mouse:%d,%d\n",mx,my); */

	      if( SCREEN_DX <= mx && mx <= SCREEN_DX + SCREEN_W &&
		  SCREEN_DY <= my && my <= SCREEN_DY + SCREEN_H ){
		  mx -= (int)SCREEN_DX;
		  my -= (int)SCREEN_DY;
		  if( screen_size == SCREEN_SIZE_HALF ){	/* 画面サイズ半分の時は、 */
		      mx *= 2;				/* マウスの座標を倍にする */
		      my *= 2;
		  }else if( screen_size == SCREEN_SIZE_DOUBLE ){
		      mx /= 2;
		      my /= 2;
		  }
		  *type = MENU_EVENT_MOUSE_MOVE;
		  *x = mx;
		  *y = my;
		  return;
	      }
	  default:
	      *type = MENU_EVENT_NOTHING;
	      return;
      }
      
  }

}
