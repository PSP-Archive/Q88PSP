/************************************************************************/
/*									*/
/* ���j���[���[�h�ɂ������ʂ̕\��					*/
/*									*/
/* QUASI88 �� ���j���[���[�h�ł̉�ʕ\���ɂ́A�ȉ��̊֐���p����B	*/
/* �Ȃ��A�����̊֐��́A������� Q8TK ����Ăяo�����B		*/
/*									*/
/*									*/
/* �y�֐��z								*/
/*									*/
/* voie	menu_draw( void )						*/
/*	�E���j���[��ʂ̕\���B���[�N T_MENU_SCREEN menu_screen[][] ��	*/
/*	  �����āAscreen_buf �ɃC���[�W���쐬���A�p���b�g��ݒ肵�āA	*/
/*	  �\�� (menu_draw_screen()) ����B				*/
/*									*/
/* void menu_draw_screen( void )					*/
/*	�E���j���[��ʂ�\�� (screen_buf ��\��)����B			*/
/*	 �u��ʘI�o�ɂ��ĕ`��v�̍ۂɂ��Ă΂��B			*/
/*									*/
/*									*/
/* �y�\���́E���[�N�z							*/
/*									*/
/* typedef struct{							*/
/*   Uint background:	4;	�w�i�p���b�g�R�[�h (0�`15)		*/
/*   Uint foreground:	4;	�\���p���b�g�R�[�h (0�`15)		*/
/*   Uint reverse:	1;	���]�\��	�ʏ�=0 ���]=1		*/
/*   Uint underline:	1;	�A���_�[���C��	�Ȃ�=0 ����=1		*/
/*   Uint font_type:	2;	�t�H���g�^�C�v (���Q��)			*/
/*   Uint addr:		16;	����ROM �A�h���X			*/
/* } T_MENU_SCREEN;							*/
/*									*/
/* enum FontType{		menu_screen[][].font �̒l 		*/
/*   FONT_ANK,			ASCII (Alphabet, Number, Kana etc)	*/
/*   FONT_LOW,			����  (������)				*/
/*   FONT_HIGH,			����  (�E����)				*/
/*   EndofFontType							*/
/* };									*/
/*									*/
/*	���j���[��ʂ̃��[�N menu_screen[25][80] �̍\���̂ł���B	*/
/*	���j���[��ʂ́A80�����~25�s�ō\������Ă���A����1�������ɁA	*/
/*	T_MENU_SCREEN �^�̃��[�N��1�A���蓖�Ă��Ă���B		*/
/*									*/
/*	�e�X�̃����o�̈Ӗ��́A���ɏ����Ă���Ƃ���B			*/
/*									*/
/*	���ۂɕ\������镶���́Aaddr (����ROM�A�h���X) �Ŏ�����Ă��邪	*/
/*	font_type �ɂ���āA���� addr �̈Ӗ����قȂ�B���Ȃ킿�A	*/
/*									*/
/*	font_type   �t�H���g						*/
/*	---------   ---------------------------------------------------	*/
/*	FONT_ANK    font_rom[addr] ����� 8 �o�C�g			*/
/*	FONT_LOW    addr < 0x0800 �̏ꍇ (���p)				*/
/*		    �Ekanji_rom[0][addr][0] ����� 16 �o�C�g		*/
/*		    addr < 0xc000 �̏ꍇ (1/4�p)			*/
/*		    �Ekanji_rom[0][addr][0] ����� 8 �o�C�g		*/
/*		    ����ȊO�̏ꍇ       (�S�p)				*/
/*		    �Ekanji_rom[0][addr][0] ���� 1�o�C�g������16 �o�C�g	*/
/*	FONT_HIGH   kanji_rom[0][addr+1][0] ���� 1�o�C�g������16 �o�C�g	*/
/*									*/
/*	���A�\������t�H���g�̃r�b�g�p�^�[���ƂȂ�B			*/
/*	�܂��AQUASI88 �� ��ʂ� 640x400 �h�b�g �Ȃ̂ŁA80�����~25�s��	*/
/*	�ꍇ�A1 ���������� 8x16 �h�b�g�ƂȂ�B	�Ȃ��AFONT_ANK ��A	*/
/*	FONT_LOW �� 0x0800 <= addr < 0xc000 �̏ꍇ�́A�r�b�g�p�^�[��	*/
/*	�f�[�^�� 8x8 �ƁA���������Ȃ��̂ŁA�c�����Ɋg�債�ĕ\������K�v	*/
/*	������B							*/
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
/* ���j���[�\���p�p���b�g �̓��e�� X�̃p���b�g�ɓ]�� 	*/
/*	graph.c �� set_palette() �����Ƃɍ쐬		*/
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
/* �o���オ������ʃC���[�W�����ۂɕ\��			*/
/********************************************************/
void	menu_redraw_screen( void )
{
  put_image();
}


/********************************************************/
/* Q8TK ����Ă΂��֐����A�R��			*/
/********************************************************/
void	menu_draw_screen( void )
{
  menu_set_palette();
  menu_set_screen();
  menu_redraw_screen();
}
