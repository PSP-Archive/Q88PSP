/************************************************************************/
/*									*/
/* QUASI88 メニュー用 Tool Kit						*/
/*				Graphic lib				*/
/*									*/
/*	Q8TK の API ( q8tk_XXX() 関数 ) 内部で呼ばれる関数群		*/
/*	menu_screen[][]に然るべき値をセットする。			*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <string.h>

#include "quasi88.h"

#include "q8tk.h"
#include "q8tk-glib.h"


/********************************************************/
/* ワーク						*/
/********************************************************/
T_Q8GR_SCREEN	menu_screen[ Q8GR_SCREEN_Y ][ Q8GR_SCREEN_X ];


/********************************************************/
/* menu_screen[][]をクリア				*/
/********************************************************/
void	q8gr_clear_screen( void )
{
  int	i, j;

  for( j=0; j<Q8GR_SCREEN_Y; j++ ){
    for( i=0; i<Q8GR_SCREEN_X; i++ ){
      menu_screen[j][i].background = Q8GR_PALETTE_BACKGROUND;
      menu_screen[j][i].foreground = Q8GR_PALETTE_FOREGROUND;
      menu_screen[j][i].reverse    = FALSE;
      menu_screen[j][i].underline  = FALSE;
      menu_screen[j][i].font_type  = FONT_ANK;
      menu_screen[j][i].addr       = 0;
    }
  }

  q8gr_reset_screen_mask();
}



/********************************************************/
/* スクリーンのマスキング				*/
/********************************************************/
static	int	screen_mask_x0, screen_mask_x1;
static	int	screen_mask_y0, screen_mask_y1;



#define	CHECK_MASK_X_FOR(x)	if     ( (x) < screen_mask_x0 ) continue;\
				else if( (x) >=screen_mask_x1 ) break
#define	CHECK_MASK_Y_FOR(y)	if     ( (y) < screen_mask_y0 ) continue;\
				else if( (y) >=screen_mask_y1 ) break
#define	CHECK_MASK_X(x)		( (x)<screen_mask_x0 || (x)>=screen_mask_x1 )
#define	CHECK_MASK_Y(x)		( (y)<screen_mask_y0 || (y)>=screen_mask_y1 )


void	q8gr_set_screen_mask( int x, int y, int sx, int sy )
{
  screen_mask_x0 = x;
  screen_mask_y0 = y;
  screen_mask_x1 = x + sx;
  screen_mask_y1 = y + sy;
}
void	q8gr_reset_screen_mask( void )
{
  screen_mask_x0 = 0;
  screen_mask_y0 = 0;
  screen_mask_x1 = Q8GR_SCREEN_X;
  screen_mask_y1 = Q8GR_SCREEN_Y;
}




/********************************************************/
/* フォーカス用のスクリーン情報				*/
/********************************************************/
static	void	*focus_screen[ Q8GR_SCREEN_Y ][ Q8GR_SCREEN_X ];


void	q8gr_clear_focus_screen( void )
{
  int	i, j;

  for( j=0; j<Q8GR_SCREEN_Y; j++ )
    for( i=0; i<Q8GR_SCREEN_X; i++ )
      focus_screen[j][i] = NULL;
}

void	q8gr_set_focus_screen( int x, int y, int sx, int sy, void *p )
{
  int	i, j;

  if( p ){
    for( j=y; j<y+sy; j++ ){
      CHECK_MASK_Y_FOR(j);
      for( i=x; i<x+sx; i++ ){
	CHECK_MASK_X_FOR(i);
	focus_screen[j][i] = p;
      }
    }
  }
}
void	*q8gr_get_focus_screen( int x, int y )
{
  return focus_screen[y][x];
}
int	q8gr_scan_focus_screen( void *p )
{
  int	i, j;

  for( j=0; j<Q8GR_SCREEN_Y; j++ ){
    for( i=0; i<Q8GR_SCREEN_X; i++ ){
      if( focus_screen[j][i]==p ) return TRUE;
    }
  }
  return FALSE;
}





/********************************************************/
/* 枠線表示						*/
/********************************************************/
/*        x ←-------sx-------→
 *     y  +--------------------+
 *     ↑ |                    |
 *     sy |                    |
 *     ↓ +--------------------+
 */
static	void	draw_normal_box( int x, int y, int sx, int sy, int shadow_type)
{
  int	i,j;
  int	light, shadow;

  switch( shadow_type ){
  case Q8TK_SHADOW_NONE:
    light  = Q8GR_PALETTE_BACKGROUND;
    shadow = Q8GR_PALETTE_BACKGROUND;
    break;
  case Q8TK_SHADOW_IN:
    light  = Q8GR_PALETTE_SHADOW;
    shadow = Q8GR_PALETTE_LIGHT;
    break;
  case Q8TK_SHADOW_OUT:
    light  = Q8GR_PALETTE_LIGHT;
    shadow = Q8GR_PALETTE_SHADOW;
    break;
  case Q8TK_SHADOW_ETCHED_IN:
    light  = Q8GR_PALETTE_LIGHT;
    shadow = Q8GR_PALETTE_LIGHT;
    break;
  case Q8TK_SHADOW_ETCHED_OUT:
  default:
    light  = Q8GR_PALETTE_SHADOW;
    shadow = Q8GR_PALETTE_SHADOW;
    break;
  }

  for( j=y; j<y+sy; j++ ){
    CHECK_MASK_Y_FOR(j);
    if( j==y || j==y+sy-1 ){
      for( i=x; i<x+sx; i++ ){
	CHECK_MASK_X_FOR(i);
	menu_screen[ j ][ i ].background = Q8GR_PALETTE_BACKGROUND;
	menu_screen[ j ][ i ].foreground = Q8GR_PALETTE_FOREGROUND;
	menu_screen[ j ][ i ].reverse    = FALSE;
	menu_screen[ j ][ i ].underline  = FALSE;
	menu_screen[ j ][ i ].font_type  = FONT_ANK;
	if      ( i==x      && j==y      ){		/* 左上 ┌*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_7 << 3;
	  menu_screen[ j ][ i ].foreground = light;
	}else if( i==x+sx-1 && j==y      ){		/* 右上 ┐*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_9 << 3;
	  menu_screen[ j ][ i ].foreground = shadow;
	}else if( i==x      && j==y+sy-1 ){		/* 左下 └*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_1 << 3;
	  menu_screen[ j ][ i ].foreground = light;
	}else if( i==x+sx-1 && j==y+sy-1 ){		/* 右下 ┘*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_3 << 3;
	  menu_screen[ j ][ i ].foreground = shadow;
	}else{						/* 上下 ─*/
	  menu_screen[ j ][ i ].addr    = Q8GR_G__ << 3;
	  if( j==y )
	    menu_screen[ j ][ i ].foreground = light;
	  else
	    menu_screen[ j ][ i ].foreground = shadow;
	}
      }
    }else{
      for( i=x; i<x+sx; i++ ){
	CHECK_MASK_X_FOR(i);
	menu_screen[ j ][ i ].background = Q8GR_PALETTE_BACKGROUND;
	menu_screen[ j ][ i ].foreground = Q8GR_PALETTE_FOREGROUND;
	menu_screen[ j ][ i ].reverse    = FALSE;
	menu_screen[ j ][ i ].underline  = FALSE;
	menu_screen[ j ][ i ].font_type  = FONT_ANK;
	if( i==x || i==x+sx-1 ){			/* 左右 │*/
	  menu_screen[ j ][ i ].addr     = Q8GR_G_I << 3;	/*0x96*/
	  if( i==x )
	    menu_screen[ j ][ i ].foreground = light;
	  else
	    menu_screen[ j ][ i ].foreground = shadow;
	}else{
	  menu_screen[ j ][ i ].addr     = (Uint)' ' << 3;
	  menu_screen[ j ][ i ].foreground = Q8GR_PALETTE_FOREGROUND;
	}
      }
    }
  }
}



void	q8gr_draw_window( int x, int y, int sx, int sy, int shadow_type )
{
  draw_normal_box( x, y, sx, sy, shadow_type );
}

void	q8gr_draw_button( int x, int y, int sx, int sy, int condition, void *p)
{
  int	i,j;
  int	light  = (condition==Q8TK_BUTTON_OFF) ? Q8GR_PALETTE_LIGHT
					      : Q8GR_PALETTE_SHADOW;
  int	shadow = (condition==Q8TK_BUTTON_OFF) ? Q8GR_PALETTE_SHADOW
					      : Q8GR_PALETTE_LIGHT;
  for( j=y; j<y+sy; j++ ){
    CHECK_MASK_Y_FOR(j);
    if( j==y || j==y+sy-1 ){
      for( i=x; i<x+sx; i++ ){
	CHECK_MASK_X_FOR(i);
	menu_screen[ j ][ i ].background = Q8GR_PALETTE_BACKGROUND;
	menu_screen[ j ][ i ].reverse    = FALSE;
	menu_screen[ j ][ i ].underline  = FALSE;
	menu_screen[ j ][ i ].font_type  = FONT_ANK;
	if      ( i==x      && j==y      ){		/* 左上 ┌*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_7 << 3;
	  menu_screen[ j ][ i ].foreground = light;
	}else if( i==x+sx-1 && j==y      ){		/* 右上 ┐*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_9 << 3;
	  menu_screen[ j ][ i ].foreground = shadow;
	}else if( i==x      && j==y+sy-1 ){		/* 左下 └*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_1 << 3;
	  menu_screen[ j ][ i ].foreground = light;
	}else if( i==x+sx-1 && j==y+sy-1 ){		/* 右下 ┘*/
	  menu_screen[ j ][ i ].addr       = Q8GR_G_3 << 3;
	  menu_screen[ j ][ i ].foreground = shadow;
	}else{						/* 上下 ─*/
	  menu_screen[ j ][ i ].addr    = Q8GR_G__ << 3;
	  if( j==y )
	    menu_screen[ j ][ i ].foreground = light;
	  else
	    menu_screen[ j ][ i ].foreground = shadow;
	}
      }
    }else{
      for( i=x; i<x+sx; i++ ){
	CHECK_MASK_X_FOR(i);
	menu_screen[ j ][ i ].background = Q8GR_PALETTE_BACKGROUND;
	menu_screen[ j ][ i ].reverse    = FALSE;
	menu_screen[ j ][ i ].underline  = FALSE;
	menu_screen[ j ][ i ].font_type  = FONT_ANK;
	if( i==x || i==x+sx-1 ){			/* 左右 │*/
	  menu_screen[ j ][ i ].addr     = Q8GR_G_I << 3;	/*0x96*/
	  if( i==x )
	    menu_screen[ j ][ i ].foreground = light;
	  else
	    menu_screen[ j ][ i ].foreground = shadow;
	}else{
	  menu_screen[ j ][ i ].addr     = (Uint)' ' << 3;
	  menu_screen[ j ][ i ].foreground = Q8GR_PALETTE_FOREGROUND;
	}
      }
    }
  }

  q8gr_set_focus_screen( x, y, sx, sy, p );
}


void	q8gr_draw_check_button( int x, int y, int condition, void *p )
{
  if( !CHECK_MASK_Y(y) ){

    if( !CHECK_MASK_X(x  ) ){
      menu_screen[ y ][ x   ].background = Q8GR_PALETTE_BACKGROUND;
      menu_screen[ y ][ x   ].foreground = Q8GR_PALETTE_FOREGROUND;
      menu_screen[ y ][ x   ].reverse    = FALSE;
      menu_screen[ y ][ x   ].underline  = FALSE;
      menu_screen[ y ][ x   ].font_type  = FONT_ANK;
      menu_screen[ y ][ x   ].addr       = Q8GR_L_RIGHT << 3;
    }
    if( !CHECK_MASK_X(x+1) ){
      menu_screen[ y ][ x+1 ].background = Q8GR_PALETTE_FONT_BG;
      menu_screen[ y ][ x+1 ].foreground = Q8GR_PALETTE_FONT_FG;
      menu_screen[ y ][ x+1 ].reverse    = FALSE;
      menu_screen[ y ][ x+1 ].underline  = FALSE;
      menu_screen[ y ][ x+1 ].font_type  = FONT_ANK;
/*    menu_screen[ y ][ x+1 ].addr       =
			(condition==Q8TK_BUTTON_OFF) ? (Q8GR_B_SPACE << 3)
						     : (Q8GR_B_BOX   << 3);*/
      menu_screen[ y ][ x+1 ].addr       =
			(condition==Q8TK_BUTTON_OFF) ? (Q8GR_B_UL  << 3)
						     : (Q8GR_B_BOX << 3);
    }
    if( !CHECK_MASK_X(x+2) ){
      menu_screen[ y ][ x+2 ].background = Q8GR_PALETTE_BACKGROUND;
      menu_screen[ y ][ x+2 ].foreground = Q8GR_PALETTE_FOREGROUND;
      menu_screen[ y ][ x+2 ].reverse    = FALSE;
      menu_screen[ y ][ x+2 ].underline  = FALSE;
      menu_screen[ y ][ x+2 ].font_type  = FONT_ANK;
      menu_screen[ y ][ x+2 ].addr       = Q8GR_L_LEFT << 3;
    }
  }

#if 0	/* チェックボタン部のみ、クリックに反応する */

  q8gr_set_focus_screen( x+1, y, 1, 1, p );

#else	/* ラベルを子に持つ場合、そのラベルをクリックしても反応する */

  if( ((Q8tkWidget *)p)->child &&
      ((Q8tkWidget *)p)->child->type == Q8TK_TYPE_LABEL &&
      ((Q8tkWidget *)p)->child->visible &&
      ((Q8tkWidget *)p)->child->name ){
    q8gr_set_focus_screen( x+1, y,
			   1+1+euclen( ((Q8tkWidget *)p)->child->name ), 1, p);
  }else{
    q8gr_set_focus_screen( x+1, y, 1, 1, p );
  }
#endif
}


void	q8gr_draw_radio_button( int x, int y, int condition, void *p )
{
  if( !CHECK_MASK_X(x+1) && !CHECK_MASK_Y(y) ){

    menu_screen[ y ][ x+1 ].background = Q8GR_PALETTE_BACKGROUND;
    menu_screen[ y ][ x+1 ].foreground = Q8GR_PALETTE_FOREGROUND;
    menu_screen[ y ][ x+1 ].reverse    = FALSE;
    menu_screen[ y ][ x+1 ].underline  = FALSE;
    menu_screen[ y ][ x+1 ].font_type  = FONT_ANK;
    menu_screen[ y ][ x+1 ].addr       =
			(condition==Q8TK_BUTTON_OFF) ? (Q8GR_B_OFF << 3)
						     : (Q8GR_B_ON  << 3);
  }

#if 0	/* ラジオボタン部のみ、クリックに反応する */

  q8gr_set_focus_screen( x+1, y, 1, 1, p );

#else	/* ラベルを子に持つ場合、そのラベルをクリックしても反応する */

  if( ((Q8tkWidget *)p)->child &&
      ((Q8tkWidget *)p)->child->type == Q8TK_TYPE_LABEL &&
      ((Q8tkWidget *)p)->child->visible &&
      ((Q8tkWidget *)p)->child->name ){
    q8gr_set_focus_screen( x+1, y,
			   1+1+euclen( ((Q8tkWidget *)p)->child->name ), 1, p);
  }else{
    q8gr_set_focus_screen( x+1, y, 1, 1, p );
  }
#endif
}



static	struct{
  int	x, y;
  int	x0, x1;
  int	selected;
} note_w;
void	q8gr_draw_notebook( int x, int y, int sx, int sy )
{
  note_w.x  = x;
  note_w.y  = y;
  note_w.x0 = x;
  note_w.x1 = x+sx-1;
  note_w.selected = FALSE;

  q8gr_draw_button( x, y+2, sx, sy-2, Q8TK_BUTTON_OFF, NULL );
}
void	q8gr_draw_notepage( const char *tag, int select_flag, int active_flag,
			    void *p )
{
  int	i,len = euclen(tag);
  int	bg = Q8GR_PALETTE_BACKGROUND;
  int	light  = Q8GR_PALETTE_LIGHT;
  int	shadow = Q8GR_PALETTE_SHADOW;
  int	focus_x = note_w.x;

  if( select_flag ){

    q8gr_euc_puts( note_w.x+1, note_w.y+1, Q8GR_PALETTE_FOREGROUND, bg,
		   FALSE, active_flag, tag );

    q8gr_ank_putc( note_w.x, note_w.y,   light, bg, FALSE, FALSE, Q8GR_C_7 );
    q8gr_ank_putc( note_w.x, note_w.y+1, light, bg, FALSE, FALSE, Q8GR_G_I );
    if( note_w.x==note_w.x0 )
      q8gr_ank_putc( note_w.x, note_w.y+2, light, bg, FALSE, FALSE, Q8GR_G_I );
    else
      q8gr_ank_putc( note_w.x, note_w.y+2, light, bg, FALSE, FALSE, Q8GR_G_3 );

    note_w.x ++;
    for( i=0; i<len; i++, note_w.x++ ){
      q8gr_ank_putc( note_w.x, note_w.y,   light, bg, FALSE, FALSE, Q8GR_G__ );
      q8gr_ank_putc( note_w.x, note_w.y+2, light, bg, FALSE, FALSE, ' ' );
    }

    q8gr_ank_putc( note_w.x, note_w.y,   shadow, bg, FALSE, FALSE, Q8GR_C_9 );
    q8gr_ank_putc( note_w.x, note_w.y+1, shadow, bg, FALSE, FALSE, Q8GR_G_I );
    if( note_w.x==note_w.x1 )
      q8gr_ank_putc( note_w.x, note_w.y+2, shadow, bg, FALSE, FALSE, Q8GR_G_I);
    else
      q8gr_ank_putc( note_w.x, note_w.y+2, light,  bg, FALSE, FALSE, Q8GR_G_1);
    note_w.x ++;

    note_w.selected = TRUE;

  }else if( note_w.selected==FALSE ){

    q8gr_euc_puts( note_w.x+1, note_w.y+1, Q8GR_PALETTE_FOREGROUND, bg,
		   FALSE, active_flag, tag );

    q8gr_ank_putc( note_w.x, note_w.y,   shadow, bg, FALSE, FALSE, Q8GR_C_7 );
    q8gr_ank_putc( note_w.x, note_w.y+1, shadow, bg, FALSE, FALSE, Q8GR_G_I );
    if( note_w.x==note_w.x0 )
      q8gr_ank_putc( note_w.x, note_w.y+2, light, bg, FALSE, FALSE, Q8GR_G_4 );
    else
      q8gr_ank_putc( note_w.x, note_w.y+2, light, bg, FALSE, FALSE, Q8GR_G_2 );

    note_w.x ++;
    for( i=0; i<len; i++, note_w.x++ ){
      q8gr_ank_putc( note_w.x, note_w.y, shadow, bg, FALSE, FALSE, Q8GR_G__ );
    }

  }else{

    q8gr_euc_puts( note_w.x, note_w.y+1, Q8GR_PALETTE_FOREGROUND, bg,
		   FALSE, active_flag, tag );

    for( i=0; i<len; i++, note_w.x++ ){
      q8gr_ank_putc( note_w.x, note_w.y, shadow, bg, FALSE, FALSE, Q8GR_G__ );
    }

    q8gr_ank_putc( note_w.x, note_w.y,   shadow, bg, FALSE, FALSE, Q8GR_C_9 );
    q8gr_ank_putc( note_w.x, note_w.y+1, light,  bg, FALSE, FALSE, Q8GR_G_I );
    if( note_w.x==note_w.x1 )
      q8gr_ank_putc( note_w.x, note_w.y+2, shadow, bg, FALSE, FALSE, Q8GR_G_6);
    else
      q8gr_ank_putc( note_w.x, note_w.y+2, light,  bg, FALSE, FALSE, Q8GR_G_2);
    note_w.x ++;

  }


  q8gr_set_focus_screen( focus_x, note_w.y, note_w.x-focus_x, 2, p );
}


void	q8gr_draw_vseparator( int x, int y, int height )
{
  int	j;
  if( !CHECK_MASK_X(x) ){
    for( j=y; j<y+height; j++ ){
      CHECK_MASK_Y_FOR(j);
      menu_screen[ j ][ x ].background = Q8GR_PALETTE_BACKGROUND;
      menu_screen[ j ][ x ].foreground = Q8GR_PALETTE_FOREGROUND;
      menu_screen[ j ][ x ].reverse    = FALSE;
      menu_screen[ j ][ x ].underline  = FALSE;
      menu_screen[ j ][ x ].font_type  = FONT_ANK;
      menu_screen[ j ][ x ].addr       = Q8GR_G_I << 3;
    }
  }
}
void	q8gr_draw_hseparator( int x, int y, int width )
{
  int	i;
  if( !CHECK_MASK_Y(y) ){
    for( i=x; i<x+width; i++ ){
      CHECK_MASK_X_FOR(i);
      menu_screen[ y ][ i ].background = Q8GR_PALETTE_BACKGROUND;
      menu_screen[ y ][ i ].foreground = Q8GR_PALETTE_FOREGROUND;
      menu_screen[ y ][ i ].reverse    = FALSE;
      menu_screen[ y ][ i ].underline  = FALSE;
      menu_screen[ y ][ i ].font_type  = FONT_ANK;
      menu_screen[ y ][ i ].addr       = Q8GR_G__ << 3;
    }
  }
}
void	q8gr_draw_frame( int x, int y, int sx, int sy,
			 const char *str, int shadow_type )
{
  draw_normal_box( x, y, sx, sy, shadow_type );

  q8gr_euc_puts( x+1, y, Q8GR_PALETTE_FOREGROUND, Q8GR_PALETTE_BACKGROUND,
		 FALSE, FALSE, str );
}


static	void	q8gr_draw_text( int x, int y, int width, int active, int reverse, int underline, int cursor_pos, const char *text )
{
  int	i, c;
  int	fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;

  for( i=0; i<width; i++, x++ ){
    int	rev = reverse;

    if     (  text==NULL ) c = ' ';
    else if( *text=='\0' ) c = ' ';
    else                   c = *text++;

    if( i==cursor_pos ) rev ^= 1;

    q8gr_ank_putc( x, y, fg, Q8GR_PALETTE_FONT_BG, rev, underline, c );
  }

}

void	q8gr_draw_combo( int x, int y, int width, int active,
			 const char *text, void *p )
{
  int fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;

#if 0		/* ANK文字しか考えない場合 */

  q8gr_draw_text( x, y, width, TRUE, FALSE, FALSE, -1, text );

#else		/* EUC文字も考えるとこうなるが、表示サイズの問題がある。*/

  int	i;

  if( euclen( text ) > width ){

    q8gr_draw_text( x, y, width, TRUE, FALSE, FALSE, -1, "Err! long..." );

  }else{
    q8gr_euc_puts( x, y, fg, Q8GR_PALETTE_FONT_BG, FALSE, FALSE, text );

    for( i=euclen( text ); i<width; i++ ){
      q8gr_ank_putc( x+i, y, fg, Q8GR_PALETTE_FONT_BG, FALSE, FALSE, ' ' );
    }
  }

#endif

  q8gr_ank_putc( x+width,  y, fg, Q8GR_PALETTE_BACKGROUND, TRUE, FALSE, ' ' );
  q8gr_ank_putc( x+width+1,y, fg, Q8GR_PALETTE_BACKGROUND, TRUE, FALSE, Q8GR_A_D );
  q8gr_ank_putc( x+width+2,y, fg, Q8GR_PALETTE_BACKGROUND, TRUE, FALSE, ' ' );
  /*q8gr_set_focus_screen( x+width, y, 3, 1, p );*/
  q8gr_set_focus_screen( x, y, 3+width, 1, p );
}

void	q8gr_draw_list_item( int x, int y, int width, int active, int reverse,
			     int underline, const char *text, void *p )
{
#if 0		/* ANK文字しか考えない場合 */

  q8gr_draw_text( x, y, width, active, reverse, underline, -1, text );

#else		/* EUC文字も考えるとこうなる */

  int	i, fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;

  q8gr_euc_puts( x, y, fg, Q8GR_PALETTE_FONT_BG, reverse, underline, text );

  for( i=euclen( text ); i<width; i++ ){
    q8gr_ank_putc( x+i, y, fg, Q8GR_PALETTE_FONT_BG, reverse, underline, ' ' );
  }

#endif

  q8gr_set_focus_screen( x, y, width, 1, p );
}

static	void	draw_adjustment( int x, int y, int active, Q8Adjust *adj, void *p )
{
  int	i, fg = Q8GR_PALETTE_FOREGROUND;
  if( active ) fg = Q8GR_PALETTE_SCALE_ACT;

  adj->x = x;
  adj->y = y;

  if( adj->horizontal ){			/* HORIZONTAL */

    if( adj->arrow ){
      q8gr_ank_putc( x, y, fg, Q8GR_PALETTE_SCALE_SLD,
		     FALSE, FALSE, Q8GR_A_L );
      x++;
    }
    for( i=0; i<adj->length; i++ )
      if( i==adj->pos ){
	q8gr_ank_putc( x+i, y, fg, Q8GR_PALETTE_SCALE_SLD,
		       FALSE, FALSE, Q8GR_B_B );
      }else{
	q8gr_ank_putc( x+i, y, fg, Q8GR_PALETTE_SCALE_BAR,
		       FALSE, FALSE, ' ' );
      }
    if( adj->arrow ){
      q8gr_ank_putc( x+i, y, fg, Q8GR_PALETTE_SCALE_SLD,
		     FALSE, FALSE, Q8GR_A_R );
      x--;
    }
    q8gr_set_focus_screen( x, y, adj->length +(adj->arrow?2:0), 1, p );

  }else{				/* Virtival */

    if( adj->arrow ){
      q8gr_ank_putc( x, y, fg, Q8GR_PALETTE_SCALE_SLD,
		     FALSE, FALSE, Q8GR_A_U );
      y++;
    }
    for( i=0; i<adj->length; i++ )
      if( i==adj->pos ){
	q8gr_ank_putc( x, y+i, fg, Q8GR_PALETTE_SCALE_SLD,
		       FALSE, FALSE, Q8GR_B_B );
      }else{
	q8gr_ank_putc( x, y+i, fg, Q8GR_PALETTE_SCALE_BAR,
		       FALSE, FALSE, ' ' );
      }
    if( adj->arrow ){
      q8gr_ank_putc( x, y+i, fg, Q8GR_PALETTE_SCALE_SLD,
		     FALSE, FALSE, Q8GR_A_D );
      y--;
    }
    q8gr_set_focus_screen( x, y, 1, adj->length +(adj->arrow?2:0), p );
  }

}



void	q8gr_draw_hscale( int x, int y, Q8Adjust *adj, int active,
			  int draw_value, int value_pos, void *p )
{
  if( draw_value ){
    int  vx, vy;
    char valstr[8];
    int	len = adj->length + (adj->arrow?2:0);

    if     ( adj->value < -99 ) strcpy( valstr, "-**" );
    else if( adj->value > 999 ) strcpy( valstr, "***" );
    else                        sprintf( valstr, "%3d", adj->value );

    switch( value_pos ){
    case Q8TK_POS_LEFT:
      vx = x;	vy = y;
      x += 4;
      break;
    case Q8TK_POS_RIGHT:
      vx = x + len+1;	vy = y;
      break;
    case Q8TK_POS_TOP:
      vx = x + ((adj->pos+3 > len) ? (len-3) : adj->pos);
      vy = y;
      y += 1;
      break;
    case Q8TK_POS_BOTTOM:
    default:
      vx = x + ((adj->pos+3 > len) ? (len-3) : adj->pos);
      vy = y +1;
      break;
    }

    q8gr_euc_puts( vx, vy, Q8GR_PALETTE_FOREGROUND, Q8GR_PALETTE_BACKGROUND,
		   FALSE, FALSE, valstr );

  }

  draw_adjustment( x, y, active, adj, p );
}


void	q8gr_draw_vscale( int x, int y, Q8Adjust *adj, int active,
			  int draw_value, int value_pos, void *p )
{
  if( draw_value ){
    int  vx, vy;
    char valstr[8];

    if     ( adj->value < -99 ) strcpy( valstr, "-**" );
    else if( adj->value > 999 ) strcpy( valstr, "***" );
    else                        sprintf( valstr, "%3d", adj->value );

    switch( value_pos ){
    case Q8TK_POS_LEFT:
      vx = x;	vy = y + adj->pos + (adj->arrow?1:0);
      x += 4;
      break;
    case Q8TK_POS_RIGHT:
      vx = x+1;	vy = y + adj->pos + (adj->arrow?1:0);
      break;
    case Q8TK_POS_TOP:
      vx = x;	vy = y;
      x += 1;	y += 1;
      break;
    case Q8TK_POS_BOTTOM:
    default:
      vx = x;	vy = y + adj->length + (adj->arrow?2:0);
      x += 1;
      break;
    }

    q8gr_euc_puts( vx, vy, Q8GR_PALETTE_FOREGROUND, Q8GR_PALETTE_BACKGROUND,
		   FALSE, FALSE, valstr );

  }

  draw_adjustment( x, y, active, adj, p );
}


void	q8gr_draw_scrolled_window( int x, int y, int sx, int sy,
				   int shadow_type, void *p )
{
  draw_normal_box( x, y, sx, sy, shadow_type );

  q8gr_set_focus_screen( x, y, sx, sy, p );
  if( sx >= 2  &&  sy >= 2 ){
    q8gr_set_focus_screen( x+1, y+1, sx-2, sy-2, p );
  }
}





void	q8gr_draw_entry( int x, int y, int width, const char *text,
			 int disp_pos, int cursor_pos, void *p )
{
  q8gr_draw_text( x, y, width, TRUE, FALSE, FALSE,
		  cursor_pos - disp_pos, text+disp_pos );
  q8gr_set_focus_screen( x, y, width, 1, p );
}


void	q8gr_draw_option_menu( int x, int y, int sx, int sy, int button,
			       void *p )
{
  q8gr_set_focus_screen( x, y, sx, sy, p );

  if( button ){
    q8gr_draw_button( x, y, sx, sy, Q8TK_BUTTON_OFF, p );
    x ++;
    y ++;
    sx -= 2;
  }
  x = x + sx - 3;
  q8gr_ank_putc( x,  y, Q8GR_PALETTE_FOREGROUND,
			Q8GR_PALETTE_BACKGROUND, TRUE, FALSE, ' ' );
  q8gr_ank_putc( x+1,y, Q8GR_PALETTE_FOREGROUND,
			Q8GR_PALETTE_BACKGROUND, TRUE, FALSE, Q8GR_G__ );
  q8gr_ank_putc( x+2,y, Q8GR_PALETTE_FOREGROUND,
			Q8GR_PALETTE_BACKGROUND, TRUE, FALSE, ' ' );
}
void	q8gr_draw_radio_menu_item( int x, int y, int width, int underline,
				   const char *text, void *p )
{
  q8gr_euc_puts( x, y, Q8GR_PALETTE_FOREGROUND, Q8GR_PALETTE_BACKGROUND,
		 FALSE, underline, text );
  q8gr_set_focus_screen( x, y, width, 1, p );
}






/********************************************************/
/* EUC文字列用 puts()					*/
/*			予期していない文字は表示しない	*/
/********************************************************/
void	q8gr_euc_puts( int x, int y, int fg, int bg,
		       int reverse, int underline, const char *str )
{
  const Uchar *p  = (const Uchar *)str;
  Uint	i, type, addr;

  for( i=0; i<strlen(str); ){

    if( *p < 0x80 ){				/* ASCII */

      addr = (Uint)*p << 3;
      type = FONT_ANK;

    }else if( *p == 0x8e ){			/* 半角カナなど */

      p ++;  i ++;
      if( *p < 0x80 ) continue;
      addr = (Uint)*p << 3;
      type = FONT_ANK;

    }else{					/* 漢字 */

      p ++;  i ++;
      if( *p < 0x80 ) continue;
      addr = (((Uint)*(p-1) & 0x7f ) << 8 ) | ( (Uint)*p & 0x7f );
      if( addr < 0x3000 ){
	addr = ((addr&0x0060)<<7) | ((addr&0x0700)<<1) | ((addr&0x001f)<<4);
      }else{
	addr = ((addr&0x0060)<<9) | ((addr&0x1f00)<<1) | ((addr&0x001f)<<4);
      }
      type = FONT_LOW;

    }

    if( !CHECK_MASK_Y(y) ){
      if( type==FONT_ANK ){
	if( !CHECK_MASK_X(x) ){
	  menu_screen[y][x].background = bg;
	  menu_screen[y][x].foreground = fg;
	  menu_screen[y][x].reverse    = reverse;
	  menu_screen[y][x].underline  = underline;
	  menu_screen[y][x].font_type  = FONT_ANK;
	  menu_screen[y][x].addr       = addr;
	}
      }else{
	if( !CHECK_MASK_X(x) ){
	  menu_screen[y][x].background = bg;
	  menu_screen[y][x].foreground = fg;
	  menu_screen[y][x].reverse    = reverse;
	  menu_screen[y][x].underline  = underline;
	  menu_screen[y][x].font_type  = FONT_LOW;
	  menu_screen[y][x].addr       = addr;
	}
	x++;
	if( !CHECK_MASK_X(x) ){
	  menu_screen[y][x].background = bg;
	  menu_screen[y][x].foreground = fg;
	  menu_screen[y][x].reverse    = reverse;
	  menu_screen[y][x].underline  = underline;
	  menu_screen[y][x].font_type  = FONT_HIGH;
	  menu_screen[y][x].addr       = addr;
	}
      }
    }
    x++;

    p ++;  i ++;
  }

}

#if 0
/********************************************************/
/* ANK文字列用 puts()					*/
/*		カーソルの位置対応			*/
/*		カーソル位置は文字列終端よりも後ろも可	*/
/*		カーソル位置が負の場合は非表示		*/
/********************************************************/
void	q8gr_ank_puts( int x, int y, int fg, int bg, int reverse,
		       const char *str, int cursor_pos )
{
  const Uchar *p  = str;
  int	i, rev, put_cursor = (cursor_pos<0) ? TRUE : FALSE;
  Uint	addr;

  for( i=0;  ; i++ ){

    if( *p ){
      addr = (Uint)*p << 3;
      p++;
    }else{
      addr = (Uint)' ' << 3;
    }

    if( i==cursor_pos ){ rev = 1;  put_cursor = TRUE; }
    else               { rev = 0;  }

    if( !CHECK_MASK_X(x) && !CHECK_MASK_Y(y) ){
      menu_screen[y][x].background = bg;
      menu_screen[y][x].foreground = fg;
      menu_screen[y][x].reverse    = reverse ^ rev;
      menu_screen[y][x].underline  = FALSE;
      menu_screen[y][x].font_type  = FONT_ANK;
      menu_screen[y][x].addr       = addr;
    }
    x++;

    if( *p==0 && put_cursor ) break;
  }
}
#endif

/********************************************************/
/* ANK文字列用 putc()					*/
/********************************************************/
void	q8gr_ank_putc( int x, int y, int fg, int bg,
		       int reverse, int underline, int c )
{
  if( !CHECK_MASK_X(x) && !CHECK_MASK_Y(y) ){
    menu_screen[y][x].background = bg;
    menu_screen[y][x].foreground = fg;
    menu_screen[y][x].reverse    = reverse;
    menu_screen[y][x].underline  = underline;
    menu_screen[y][x].font_type  = FONT_ANK;
    menu_screen[y][x].addr       = (c&0xff) << 3;
  }
}
