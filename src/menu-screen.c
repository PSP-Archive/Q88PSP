/************************************************************************/
/*									*/
/* メニューモードにおける画面の表示					*/
/*									*/
/* QUASI88 の メニューモードでの画面表示には、以下の関数を用いる。	*/
/* なお、これらの関数は、いずれも Q8TK から呼び出される。		*/
/*									*/
/*									*/
/* 【関数】								*/
/*									*/
/* voie	menu_draw( void )						*/
/*	・メニュー画面の表示。ワーク T_MENU_SCREEN menu_screen[][] に	*/
/*	  応じて、screen_buf にイメージを作成し、パレットを設定して、	*/
/*	  表示 (menu_draw_screen()) する。				*/
/*									*/
/* void menu_draw_screen( void )					*/
/*	・メニュー画面を表示 (screen_buf を表示)する。			*/
/*	 「画面露出による再描画」の際にも呼ばれる。			*/
/*									*/
/*									*/
/* 【構造体・ワーク】							*/
/*									*/
/* typedef struct{							*/
/*   Uint background:	4;	背景パレットコード (0～15)		*/
/*   Uint foreground:	4;	表示パレットコード (0～15)		*/
/*   Uint reverse:	1;	反転表示	通常=0 反転=1		*/
/*   Uint underline:	1;	アンダーライン	なし=0 あり=1		*/
/*   Uint font_type:	2;	フォントタイプ (下参照)			*/
/*   Uint addr:		16;	漢字ROM アドレス			*/
/* } T_MENU_SCREEN;							*/
/*									*/
/* enum FontType{		menu_screen[][].font の値 		*/
/*   FONT_ANK,			ASCII (Alphabet, Number, Kana etc)	*/
/*   FONT_LOW,			漢字  (左半分)				*/
/*   FONT_HIGH,			漢字  (右半分)				*/
/*   EndofFontType							*/
/* };									*/
/*									*/
/*	メニュー画面のワーク menu_screen[25][80] の構造体である。	*/
/*	メニュー画面は、80文字×25行で構成されており、その1文字毎に、	*/
/*	T_MENU_SCREEN 型のワークが1個、割り当てられている。		*/
/*									*/
/*	各々のメンバの意味は、横に書いてあるとおり。			*/
/*									*/
/*	実際に表示される文字は、addr (漢字ROMアドレス) で示されているが	*/
/*	font_type によって、その addr の意味が異なる。すなわち、	*/
/*									*/
/*	font_type   フォント						*/
/*	---------   ---------------------------------------------------	*/
/*	FONT_ANK    font_rom[addr] からの 8 バイト			*/
/*	FONT_LOW    addr < 0x0800 の場合 (半角)				*/
/*		    ・kanji_rom[0][addr][0] からの 16 バイト		*/
/*		    addr < 0xc000 の場合 (1/4角)			*/
/*		    ・kanji_rom[0][addr][0] からの 8 バイト		*/
/*		    それ以外の場合       (全角)				*/
/*		    ・kanji_rom[0][addr][0] から 1バイトおきに16 バイト	*/
/*	FONT_HIGH   kanji_rom[0][addr+1][0] から 1バイトおきに16 バイト	*/
/*									*/
/*	が、表示するフォントのビットパターンとなる。			*/
/*	また、QUASI88 の 画面は 640x400 ドット なので、80文字×25行の	*/
/*	場合、1 文字あたり 8x16 ドットとなる。	なお、FONT_ANK や、	*/
/*	FONT_LOW で 0x0800 <= addr < 0xc000 の場合は、ビットパターン	*/
/*	データが 8x8 と、半分しかないので、縦方向に拡大して表示する必要	*/
/*	がある。							*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "menu-screen.h"

#include "q8tk.h"
#include "graph.h"
#include "memory.h"



#define	FULL
#define	TYPE	bit8
#include "menu-screen-draw.h"
INLINE	menu2pixel( full, bit8 )
#undef	TYPE
#define	TYPE	bit16
#include "menu-screen-draw.h"
INLINE	menu2pixel( full, bit16 )
#undef	TYPE
#define	TYPE	bit32
#include "menu-screen-draw.h"
INLINE	menu2pixel( full, bit32 )
#undef	TYPE
#undef	FULL
#define	HALF
#define	TYPE	bit8
#include "menu-screen-draw.h"
INLINE	menu2pixel( half, bit8 )
#undef	TYPE
#define	TYPE	bit16
#include "menu-screen-draw.h"
INLINE	menu2pixel( half, bit16 )
#undef	TYPE
#define	TYPE	bit32
#include "menu-screen-draw.h"
INLINE	menu2pixel( half, bit32 )
#undef	TYPE
#undef	HALF
#define	DOUBLE
#define	TYPE	bit8
#include "menu-screen-draw.h"
INLINE	menu2pixel( double, bit8 )
#undef	TYPE
#define	TYPE	bit16
#include "menu-screen-draw.h"
INLINE	menu2pixel( double, bit16 )
#undef	TYPE
#define	TYPE	bit32
#include "menu-screen-draw.h"
INLINE	menu2pixel( double, bit32 )
#undef	TYPE
#undef	DOUBLE


static	void	menu_set_screen( void )
{
  if( screen_size == SCREEN_SIZE_FULL ){		/* Full Size */
    if     ( DEPTH<= 8 ) menu2pixel_full_bit8();
    else if( DEPTH<=16 ) menu2pixel_full_bit16();
    else                 menu2pixel_full_bit32();
  }else if( screen_size == SCREEN_SIZE_HALF ){		/* Half Size */
    if     ( DEPTH<= 8 ) menu2pixel_half_bit8();
    else if( DEPTH<=16 ) menu2pixel_half_bit16();
    else                 menu2pixel_half_bit32();
  }else{						/* Double Size */
    if     ( DEPTH<= 8 ) menu2pixel_double_bit8();
    else if( DEPTH<=16 ) menu2pixel_double_bit16();
    else                 menu2pixel_double_bit32();
  }
}


/********************************************************/
/* メニュー表示用パレット の内容を Xのパレットに転送 	*/
/*	graph.c の set_palette() をもとに作成		*/
/********************************************************/
static	void	menu_set_palette( void )
{
  int     i;
  SYSTEM_PALETTE_T	syspal[16];

  static const struct {
    int pal;
    int col;
  } pal[ 16 ] = {
    { Q8GR_PALETTE_FOREGROUND, MENU_COLOR_FOREGROUND,	},
    { Q8GR_PALETTE_BACKGROUND, MENU_COLOR_BACKGROUND,	},
    { Q8GR_PALETTE_LIGHT,      MENU_COLOR_LIGHT,	},
    { Q8GR_PALETTE_SHADOW,     MENU_COLOR_SHADOW,	},
    { Q8GR_PALETTE_FONT_FG,    MENU_COLOR_FONT_FG,	},
    { Q8GR_PALETTE_FONT_BG,    MENU_COLOR_FONT_BG,	},
    { Q8GR_PALETTE_SCALE_SLD,  MENU_COLOR_SCALE_SLD,	},
    { Q8GR_PALETTE_SCALE_BAR,  MENU_COLOR_SCALE_BAR,	},
    { Q8GR_PALETTE_SCALE_ACT,  MENU_COLOR_SCALE_ACT,	},
    { Q8GR_PALETTE_DUMMY_09,   0x000000,		},
    { Q8GR_PALETTE_DUMMY_10,   0x000000,		},
    { Q8GR_PALETTE_DUMMY_11,   0x000000,		},
    { Q8GR_PALETTE_DUMMY_12,   0x000000,		},
    { Q8GR_PALETTE_DUMMY_13,   0x000000,		},
    { Q8GR_PALETTE_DUMMY_14,   0x000000,		},
    { Q8GR_PALETTE_DUMMY_15,   0x000000,		},
  };

  for( i=0; i<countof(pal); i++ ){
    syspal[ pal[i].pal ].red   = (pal[i].col >> 16) & 0xff;
    syspal[ pal[i].pal ].green = (pal[i].col >>  8) & 0xff;
    syspal[ pal[i].pal ].blue  = (pal[i].col >>  0) & 0xff;
  }


  trans_palette( syspal );
}

/********************************************************/
/* 出来上がった画面イメージを実際に表示			*/
/********************************************************/
void	menu_redraw_screen( void )
{
  put_image();
}


/********************************************************/
/* Q8TK から呼ばれる関数が、コレ			*/
/********************************************************/
void	menu_draw_screen( void )
{
  menu_set_palette();
  menu_set_screen();
  menu_redraw_screen();
}
