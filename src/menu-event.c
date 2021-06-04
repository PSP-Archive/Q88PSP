/************************************************************************/
/*									*/
/* ���j���[���[�h�ɂ�����A�C�x���g�̎擾 (OS�ˑ�)			*/
/*									*/
/* QUASI88 �̃��j���[���[�h�p�c�[���L�b�g Q8TK �ł́u�}�E�X�{�^�������v	*/
/* �u�}�E�X�{�^���J���v�u�}�E�X�ړ��v�u�L�[�{�[�h�����v�u�����I���v��	*/
/* �����ꂩ�̔�����҂̂ŁA����炪��������܂őҋ@���A��������ʒm	*/
/* ����֐����K�v�ł���B						*/
/*									*/
/*									*/
/* �y�֐��z								*/
/*									*/
/* void	menu_event_init( void )						*/
/*	�Emenu_check_event()�Ȃǂ̊֐��̏����ɕK�v�ȏ��������AQ8TK ��	*/
/*	  �������ɐ悾���čs�Ȃ��B					*/
/*									*/
/* void menu_event_term( void )						*/
/*	�EQ8TK �̏I����� menu_event_init() �ŏ����������ݒ��j������B*/
/*	  �܂��A������ QUASI88 �̃G�~�����[�g���[�h�ɉ������ݒ��	*/
/*	  �����ōēx�s�Ȃ��B						*/
/*									*/
/*									*/
/* void	menu_check_event( int *type, int *button, int *x, int *y )	*/
/*	�E�ȉ��̃C�x���g����������܂ő҂B				*/
/*		MENU_EVENT_KEY_ON	�c �L�[�������ꂽ		*/
/*		MENU_EVENT_MOUSE_ON	�c �}�E�X�{�^���������ꂽ	*/
/*		MENU_EVENT_MOUSE_OFF	�c �}�E�X�z�^���������ꂽ	*/
/*		MENU_EVENT_MOUSE_MOVE	�c �}�E�X���ړ�����		*/
/*		MENU_EVENT_QUIT		�c Ctrl-C �V�O�i�������m����	*/
/*	�E�����̃C�x���g����������ƁA				*/
/*		�� *type �ɃC�x���g�ԍ� (MENU_EVENT_XXX) ���Z�b�g	*/
/*		���L�[�������ꂽ�ꍇ�� *button �� �����ꂽ�L�[��	*/
/*		  ASCII�R�[�h���Z�b�g					*/
/*		���}�E�X�������ꂽ�� *button �ɂ����ꂽ�{�^���̔ԍ�	*/
/*		  (Q8TK_BUTTON_L/Q8TK_BUTTON_R) ���Z�b�g		*/
/*		���}�E�X�������ꂽ�� *button �ɗ����ꂽ�{�^���̔ԍ�	*/
/*		  (Q8TK_BUTTON_L/Q8TK_BUTTON_R) ���Z�b�g		*/
/*		���}�E�X���ړ�������A*x�A*y �Ƀ}�E�X�̍��W���Z�b�g	*/
/*	  ���āA�֐����I����B						*/
/*	�E�C�x���g����������܂ŁA���̊֐��͏I�����Ȃ��B		*/
/*									*/
/* void	menu_clear_event( void )					*/
/*	�E��s���͂����ƟT�������C�x���g(�������͂Ȃ�)���N���A����B	*/
/*	  �ׂɉ������Ȃ��Ă���薳���B�t�ɐ�s���͂��������̂Ȃ�A	*/
/*	  �Ȃɂ����Ă͂����Ȃ��B					*/
/*									*/
/* void	menu_quit_signal( void )					*/
/*	�EMENU_EVENT_QUIT �̃C�x���g�𐶐�����B			*/
/*	  ���̊֐��́ACtrl-C �������ꂽ���ɁA���̃V�O�i���n���h������	*/
/*	  �Ă΂��B							*/
/*									*/
/*									*/
/* �y����z								*/
/*									*/
/*	{								*/
/*	  menu_event_init();						*/
/*	  q8tk_init();							*/
/*	  q8tk�ݒ�;							*/
/*	  q8tk_main();		�������� menu_check_event()���Ă�ł���	*/
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
/* ���j���[���[�h�J�n���̏���						*/
/************************************************************************/
void	menu_event_init( void )
{
  /* �L�[������ ASCII �R�[�h�ɕϊ��\�Ƃ��� (�������d���炵���̂Ĉꎞ�I��) */
  SDL_EnableUNICODE(1);

  /* �I�[�g���s�[�g ON */
  SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );
}

/************************************************************************/
/* ���j���[���[�h�I�����̏���						*/
/************************************************************************/
void	menu_event_term( void )
{
  /* �L�[������ ASCII �R�[�h�ɕϊ��\�Ƃ��� */
  SDL_EnableUNICODE(0);

  /* �I�[�g���s�[�g OFF */
  SDL_EnableKeyRepeat( 0, 0 );
}


/************************************************************************/
/* MENU_EVENT_QUIT �C�x���g�𐶐�					*/
/*	SIGINT (Ctrl-C) �������ɌĂ΂�� �� monitor.c			*/
/************************************************************************/
void	menu_quit_signal( void )
{
}

/************************************************************************/
/* ��s���͂����ƟT�������C�x���g(�������͂Ȃ�)���N���A����B		*/
/*	��s���͂�h�~������̂����A��s���͂��������́A�ʂɂȂɂ�	*/
/*	���Ȃ��֐��ɂ��悤�B						*/
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
/* �C�x���g�̃`�F�b�N�ɐ悾���ẮA�}�E�X���W�������l�𓾂�		*/
/*	���}�E�X�̌��݂̍��W��Ԃ�					*/
/************************************************************************/
void	menu_init_event( int *win_x, int *win_y )
{
  SDL_GetMouseState( win_x, win_y );

  if( screen_size == SCREEN_SIZE_HALF ){	/* ��ʃT�C�Y�����̎��́A */
    *win_x *= 2;				/* �}�E�X�̍��W��{�ɂ��� */
    *win_y *= 2;
  }else if( screen_size == SCREEN_SIZE_DOUBLE ){
    *win_x /= 2;
    *win_y /= 2;
  }
}


/************************************************************************/
/* �e��C�x���g����������ƁA���̃C�x���g��Ԃ��֐�			*/
/*	���L�[ ON�A�}�E�X�{�^�� ON/OFF �A�}�E�X�ړ��ASIGINT (Ctrl-C) ��	*/
/*	  ������ʒm����B						*/
/*	����ʂ��ĘI�o�����Ƃ��A�ĕ`�悷��				*/
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

    /* �C�x���g���N����Ȃ��Ă������ɕԂ��Ă��Ă��܂��c�c�c */

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
		  if( screen_size == SCREEN_SIZE_HALF ){	/* ��ʃT�C�Y�����̎��́A */
		      mx *= 2;				/* �}�E�X�̍��W��{�ɂ��� */
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
