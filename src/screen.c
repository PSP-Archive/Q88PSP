/************************************************************************/
/*									*/
/* ��ʂ̕\��								*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "screen.h"
#include "graph.h"

#include "crtcdmac.h"
#include "memory.h"

#include "indicator.h"
#include "suspend.h"

#include "intr.h"

int kaigyou = 0;
int gyou = 1;
int narabi8dot[8] = {0,6,1,2,3,7,4,5};
int narabi16dot[16] = {0,12,1,2,3,13,4,5,6,14,7,8,9,15,10,11};

int	frameskip_rate  = DEFAULT_FRAMESKIP;	/* ��ʕ\���̍X�V�Ԋu	*/

int	monitor_analog  = TRUE;			/* �A�i���O���j�^�[     */

PC88_PALETTE_T	vram_bg_palette;	/* OUT[52/54-5B]		*/
PC88_PALETTE_T	vram_palette[8];	/*		�e��p���b�g	*/

byte	sys_ctrl;			/* OUT[30] SystemCtrl		*/
byte	grph_ctrl;			/* OUT[31] GraphCtrl		*/
byte	grph_pile;			/* OUT[53] �d�ˍ��킹		*/

int	do_update = TRUE;			/* �`��̕K�v����	*/

int	use_half_interp = FALSE;		/* ��ʃT�C�Y����(320x200)���ɕ�Ԃ��� */
int	now_half_interp = FALSE;	/* ����HALF���̕�ԉ\		*/

int	use_interlace = FALSE;		/* �C���^�[���[�X(1���C������)�\�� */

int	use_auto_skip = TRUE;		/* �I�[�g�t���[���X�L�b�v���g�p����	*/
int	do_skip_draw = FALSE;		/* �X�N���[���ւ̕`����X�L�b�v����	*/
int	already_skip_draw = FALSE;	/* �X�L�b�v������ */

/*
 *	��ʍ����X�V�̏����}�N��
 */

char	screen_update[ 0x4000*2 ];	/* ��ʕ\�������X�V�t���O	*/
int	screen_update_force = TRUE;	/* ��ʋ����X�V�t���O		*/
int	screen_update_palette = TRUE;	/* �p���b�g�X�V�t���O		*/
int	screen_update_hide = FALSE;	/* VRAM��\�������σt���O	*/

#define	reset_screen_update()					\
	do{							\
	  memset( screen_update, 0, sizeof( screen_update ));	\
	  screen_update_force = FALSE;				\
	  text_attr_flipflop ^= 1;    				\
	}while(0)

#define	check_screen_update_force()					\
	do{								\
	  if( !(grph_ctrl&GRPH_CTRL_VDISP) ){	/* VRAM ��\���Ȃ� */	\
	    if( screen_update_hide==FALSE ){	/*    �O��͕\��   */	\
	      screen_update_force = TRUE;				\
	    }else{				/*    �O�����\�� */	\
	      memset( screen_update, 0, sizeof( screen_update )/2 );	\
	    }								\
	  }								\
	  if( screen_update_force ){					\
	    memset( text_attr_buf, 0, sizeof( text_attr_buf ) );	\
	    memset( screen_update, 1, sizeof( screen_update )/2 );	\
	    do_update = TRUE;						\
	  }								\
	  if( !(grph_ctrl & 			/* 400���C������   */	\
		(GRPH_CTRL_COLOR|GRPH_CTRL_200) ) ){			\
	    memcpy( &screen_update[80*200], screen_update, 80*200 );	\
	  }								\
	}while(0)



/****************************************************************/
/* �e�L�X�gVRAM�̃A�g���r���[�g���p���[�N�ɐݒ肷��		*/
/*	�o�b�t�@��2����A���݂ɐؑւ��Ďg�p����B		*/
/*	��ʏ��������̍ۂ́A����2�̃o�b�t�@���r���A�ω���	*/
/*	�����������������X�V����B				*/
/*								*/
/*	���[�N�́A16bit�ŁA���8bit�������R�[�h�A���ʂ͑����B	*/
/*		�F�A�O���t�B�b�N���[�h�A�A���_�[���C���A	*/
/*		�A�b�p�[���C���A�V�[�N���b�g�A���o�[�X		*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*		|    ASCII 8bit       |�f|�q|�a|GR|LO|UP|SC|RV|	*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*	BLINK�����́A�_�����͖����A�������̓V�[�N���b�g�B	*/
/*								*/
/*	�ȉ��̏ꍇ�́A��(�\���s�v)�Ȃ̂ŁA���e������������B	*/
/*	  �E�V�[�N���b�g�������I���̏ꍇ			*/
/*	  �Eascii���󔒂ŁAGR�ALO�AUP���I�t�̏ꍇ		*/
/*	  �Eascii��00�ŁAGR���I���ALO�AUP���I�t�̏ꍇ		*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*	     ��	|    ASCII == 0       |�f|�q|�a|�O|�O|�O|�O|RV|	*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*	        �F�ƃ��o�[�X�����̂ݎc���A���͂��ׂăN���A	*/
/*								*/
/****************************************************************/
int	text_attr_flipflop = 0;
Ushort	text_attr_buf[2][2048];		/* �A�g���r���[�g���	*/
			/* �� 80����x25�s=2000�ő����̂����A	*/
			/* �]���Ɏg���̂ŁA���߂Ɋm�ۂ���B	*/
				   

static	void	make_text_attr( void )
{
  int		global_attr  = (ATTR_G|ATTR_R|ATTR_B);
  int		global_blink = FALSE;
  int		i, j, tmp;
  int		column, attr, attr_rest;
  word		char_start_addr, attr_start_addr;
  word		c_addr, a_addr;
  Ushort	*text_attr = &text_attr_buf[ text_attr_flipflop ][0];


	/* CRTC �� DMAC ���~�܂��Ă���ꍇ */
	/*  (�������A�g���r���[�g������)   */

  if( text_display==TEXT_DISABLE ){		/* ASCII=0�A���F�A�����Ȃ� */
    for( i=0; i<CRTC_SZ_LINES; i++ ){		/* �ŏ���������B	   */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ =  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* �S��ʔ��]��J�[�\�����Ȃ��B�����ɖ߂�  */
  }



	/* �m���E�g�����X�y�A�����g�^�̏ꍇ */
	/* (1�����u���ɁAVRAM�AATTR ������) */

			/* �c�c�c �H�ڍוs�� 				*/
			/*	CRTC�̐ݒ�p�^�[�����炵�āA����ɍs��	*/
			/*	�Ō�ɑ���������ꍇ�����肦���������c?	*/

  if( crtc_attr_non_separate ){

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + 1;

    for( i=0; i<crtc_sz_lines; i++ ){

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;

      for( j=0; j<CRTC_SZ_COLUMNS; j+=2 ){		/* ����������R�[�h��*/
	attr = main_ram[ a_addr ];			/* �ϊ����A�������[�N*/
	a_addr += 2;					/* ��S�Ė��߂�B    */
	global_attr =( global_attr & COLOR_MASK ) |
		     ((attr &  MONO_GRAPH) >> 3 ) |
		     ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
		     ((attr &  MONO_SECRET) << 1 );

					/* BLINK��OFF����SECRET����    */
	if( (attr & MONO_BLINK) && ((blink_counter&0x03)==0) ){
	  global_attr |= ATTR_SECRET;
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;
	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){
	if( ++i < crtc_sz_lines ){
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }
    for( ; i<CRTC_SZ_LINES; i++ ){		/* �c��̍s�́ASECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24�s�ݒ�΍�)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }else{

	/* �g�����X�y�A�����g�^�̏ꍇ */
	/* (�s�̍Ō�ɁAATTR������)   */

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + crtc_sz_columns;

    for( i=0; i<crtc_sz_lines; i++ ){			/* �s�P�ʂő����쐬 */

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;


      attr_rest = 0;						/*���������� */
      for( j=0; j<=CRTC_SZ_COLUMNS; j++ ) text_attr[j] = 0;	/* [0]�`[80] */


      for( j=0; j<crtc_sz_attrs; j++ ){			/* �������w��Ԗڂ� */
	column = main_ram[ a_addr++ ];			/* �z��Ɋi�[       */
	attr   = main_ram[ a_addr++ ];

	if( j!=0 && column==0    ) column = 0x80;		/* ���ꏈ��?*/
	if( j==0 && column==0x80 ){column = 0;
/*				   global_attr = (ATTR_G|ATTR_R|ATTR_B);
				   global_blink= FALSE;  }*/}

	if( column==0x80  &&  !attr_rest ){			/* 8bit�ڂ� */
	  attr_rest = attr | 0x100;				/* �g�p�ς� */
	}							/* �t���O   */
	else if( column <= CRTC_SZ_COLUMNS  &&  !text_attr[ column ] ){
	  text_attr[ column ] = attr | 0x100;
	}
      }


      if( !text_attr[0] && attr_rest ){			/* �w�茅-1�܂ő�����*/
	for( j=CRTC_SZ_COLUMNS; j; j-- ){		/* �L���A�Ƃ����ꍇ��*/
	  if( text_attr[j] ){				/* �����B(�w�茅�ȍ~ */
	    tmp          = text_attr[j];		/* �������L���A�Ƃ���*/
	    text_attr[j] = attr_rest;			/* �ӂ��ɕ��בւ���) */
	    attr_rest    = tmp;
	  }
	}
	text_attr[0] = attr_rest;
      }


      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* ����������R�[�h��*/
							/* �ϊ����A�������[�N*/
	if( ( attr = *text_attr ) ){			/* ��S�Ė��߂�B    */
	  if( crtc_attr_color ){
	    if( attr & COLOR_SWITCH ){
	      global_attr =( global_attr & MONO_MASK ) |
			   ( attr & (COLOR_G|COLOR_R|COLOR_B|COLOR_GRAPH));
	    }else{
	      global_attr =( global_attr & (COLOR_MASK|ATTR_GRAPH) ) |
			   ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			   ((attr &  MONO_SECRET) << 1 );
	      global_blink= (attr & MONO_BLINK);
	    }
	  }else{
	    global_attr =( global_attr & COLOR_MASK ) |
			 ((attr &  MONO_GRAPH) >> 3 ) |
			 ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			 ((attr &  MONO_SECRET) << 1 );
	    global_blink= (attr & MONO_BLINK);
	  }
					/* BLINK��OFF����SECRET����    */
	  if( global_blink && ((blink_counter&0x03)==0) ){
	    global_attr =  global_attr | ATTR_SECRET;
	  }
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){				/* 1�s��΂��w�莞��*/
	if( ++i < crtc_sz_lines ){			/* ���̍s��SECRET�� */
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* ���߂�B         */
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }

    for( ; i<CRTC_SZ_LINES; i++ ){		/* �c��̍s�́ASECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24�s�ݒ�΍�)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }



	/* CRTC �� DMAC �͓����Ă��邯�ǁA �e�L�X�g����\�� */
	/* ��VRAM�����̏ꍇ (�A�g���r���[�g�̐F�������L��)  */

  if( text_display==TEXT_ATTR_ONLY ){

    text_attr = &text_attr_buf[ text_attr_flipflop ][0];

    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ &=  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* �S��ʔ��]��J�[�\���͕s�v�B�����łɖ߂�  */
  }




		/* �S�̔��]���� */

  if( crtc_reverse_display && (grph_ctrl & GRPH_CTRL_COLOR)){
    text_attr = &text_attr_buf[ text_attr_flipflop ][0];
    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ ^= ATTR_REVERSE;
      }
    }
  }

		/* �J�[�\���\������ */

  if( 0 <= crtc_cursor[0] && crtc_cursor[0] < crtc_sz_columns &&
      0 <= crtc_cursor[1] && crtc_cursor[1] < crtc_sz_lines   ){
    if( !crtc_cursor_blink || (blink_counter&0x01) ){
      text_attr_buf[ text_attr_flipflop ][ crtc_cursor[1]*80 + crtc_cursor[0] ]
							^= crtc_cursor_style;
    }
  }



	/* �󔒃t�H���g����уx�^�t�H���g�ŁA�����������ꍇ�A	*/
	/* �����R�[�h�� 0x00 �ɒu��������			*/
	/* ����ɁA�x�^�t�H���g�̏ꍇ�́AREVERSE�����ɂ���B	*/

  text_attr = &text_attr_buf[ text_attr_flipflop ][0];

  for( i=0; i<CRTC_SZ_LINES; i++ ){
    for( j=0; j<CRTC_SZ_COLUMNS; j++ ){


      if( *text_attr & ATTR_SECRET ){		/* SECRET �����́A�R�[�h00�� */
	*text_attr &= 0xff;
      }
      
      if( ( *text_attr & (ATTR_UPPER|ATTR_LOWER) )==0 ){
	int c = *text_attr >> 8;
	int space = FALSE;
	int black = FALSE;

	if( ( *text_attr & ATTR_GRAPH) == 0 ){
	  if( c==0 || c==32 || c==96 || c==127 || c==160 || c>=248 ){/*�� */
	    if( ( *text_attr & ATTR_REVERSE )==0 ) space = TRUE;
	    else                                   black = TRUE;
	  }else if( c==135 && crtc_font_height==8 ){		     /*�x�^ */
	    if( ( *text_attr & ATTR_REVERSE )==0 ) black = TRUE;
	    else                                   space = TRUE;
	  }
	}else{
	  if( c==0 ){						     /*�� */
	    if( ( *text_attr & ATTR_REVERSE )==0 ) space = TRUE;
	    else                                   black = TRUE;
	  }else if( c==255 && crtc_font_height==8 ){		     /*�x�^ */
	    if( ( *text_attr & ATTR_REVERSE )==0 ) black = TRUE;
	    else                                   space = TRUE;
	  }
	}
	
	if( space ) *text_attr &= COLOR_MASK;
	if( black ) *text_attr = ( *text_attr & COLOR_MASK ) | ATTR_REVERSE;
      }

      text_attr ++;
    }
  }
	/* SECRET�ł��A�A���_�[�E�A�b�p�[���C���AREVERSE�͗L�� */
}



/****************************************************************/
/* �w����W�̕����̃t�H���g�f�[�^���쐬����B			*/
/*	�܂��Aset_font_ptr(x,y) ���Ă�ŁA�������[�N���������B	*/
/*	�����āAget_font_style ���ĂԂ��тɁA�t�H���g�f�[�^��	*/
/*	���X�Ǝ�ɓ���B20�s�̏ꍇ�� 10��A25�s�̏ꍇ�� 8��Ă�	*/
/*	�΂悢�B						*/
/*		�A�g���r���[�g��񂪕K�v�Ȃ̂ŁA		*/
/*		�\�� make_text_attr_table( ) ���Ă�ł�������	*/
/*								*/
/*	�t�H���g�f�[�^�� font_rom ����A�������[�N font_style��	*/
/*	�R�s�[���Ă���̂����A���̍ۂɁAbit32 �ňꊇ���ăR�s�[	*/
/*	���Ă���(������Ƃ͍����H)�B�������Ȃ��Ǝv�����c�c�c	*/
/****************************************************************/

bit8	*font_ptr;
bit8	font_style[16];			/* �g�p����̂� [0]�`[9]�܂� */
int	font_color;

void	set_font_style( int attr )
{
  int	chara;
  bit32	*src;
  bit32	*dst = (bit32 *)&font_style[0];;
					/* get_font_style()�̂��߂̏��� */
  font_ptr   = &font_style[0];
  font_color = ((attr & COLOR_MASK) >> 5) | 8;


  if( ( attr & ~(COLOR_MASK|ATTR_REVERSE) )==0 ){

    if( ( attr & ATTR_REVERSE ) == 0 ){		/* �󔒃t�H���g�� */

      *dst++ = 0;
      *dst++ = 0;
      *dst   = 0;

    }else{					/* �x�^�t�H���g�� */

      *dst++ = 0xffffffff;
      *dst++ = 0xffffffff;
      *dst   = 0xffffffff;
    }

  }else{					/* �ʏ�t�H���g�� */

    chara = attr >> 8;

    if( attr & ATTR_GRAPH )
      src = (bit32 *)&font_rom[ (chara | 0x100)*8 ];
    else
      src = (bit32 *)&font_rom[ (chara        )*8 ];

					/* �t�H���g���܂��������[�N�ɃR�s�[ */
    *dst++ = *src++;
    *dst++ = *src;
    *dst   = 0;

					/* �����ɂ��������[�N�t�H���g�����H*/
    if( attr & ATTR_UPPER ) font_style[ 0 ] |= 0xff;
    if( attr & ATTR_LOWER ) font_style[ crtc_font_height-1 ] |= 0xff;
    if( attr & ATTR_REVERSE ){
      dst -= 2;
      *dst++ ^= 0xffffffff;
      *dst++ ^= 0xffffffff;
      *dst   ^= 0xffffffff;
    }
  }
}









/********************************************************/
/* PC8801 G-VRAM/T-VRAM �̓��e�� X�̕\���p�o�b�t�@�ɓ]��*/
/********************************************************/

#define	COLOR			/* �J���[ 640x200 ---------------------------*/

#define	FULL
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( color, full, bit8 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( color, full, bit8_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
//#include "screen-base_dbg.h"
INLINE	vram2pixel( color, full, bit16 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( color, full, bit16_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( color, full, bit32 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( color, full, bit32_interlace )
#undef	INTERLACE
#undef	TYPE
#undef	FULL

#define	HALF
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( color, half, bit8 )
#define	INTERP
#include "screen-200.h"
INLINE	vram2pixel( color, half, bit8_interp )
#undef	INTERP
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( color, half, bit16 )
#define	INTERP
#include "screen-200.h"
INLINE	vram2pixel( color, half, bit16_interp )
#undef	INTERP
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( color, half, bit32 )
#define	INTERP
#include "screen-200.h"
INLINE	vram2pixel( color, half, bit32_interp )
#undef	INTERP
#undef	TYPE
#undef	HALF

#define	DOUBLE
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( color, double, bit8 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( color, double, bit8_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( color, double, bit16 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( color, double, bit16_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( color, double, bit32 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( color, double, bit32_interlace )
#undef	INTERLACE
#undef	TYPE
#undef	DOUBLE

#undef	COLOR


#define	MONO			/* ����   640x200 ---------------------------*/

#define	FULL
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( mono, full, bit8 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( mono, full, bit8_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( mono, full, bit16 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( mono, full, bit16_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( mono, full, bit32 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( mono, full, bit32_interlace )
#undef	INTERLACE
#undef	TYPE
#undef	FULL

#define HALF
#include "screen-200.h"
#define	TYPE	bit8
INLINE	vram2pixel( mono, half, bit8 )
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( mono, half, bit16 )
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( mono, half, bit32 )
#undef	TYPE
#undef	HALF

#define	DOUBLE
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( mono, double, bit8 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( mono, double, bit8_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( mono, double, bit16 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( mono, double, bit16_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( mono, double, bit32 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( mono, double, bit32_interlace )
#undef	INTERLACE
#undef	TYPE
#undef	DOUBLE

#undef	MONO


#define	HIRESO			/* ����   640x400 ---------------------------*/

#define	FULL
#define	TYPE	bit8
#include "screen-400.h"
INLINE	vram2pixel( 400, full, bit8 )
#undef	TYPE
#define	TYPE	bit16
#include "screen-400.h"
INLINE	vram2pixel( 400, full, bit16 )
#undef	TYPE
#define	TYPE	bit32
#include "screen-400.h"
INLINE	vram2pixel( 400, full, bit32 )
#undef	TYPE
#undef	FULL

#define	HALF
#define	TYPE	bit8
#include "screen-400.h"
INLINE	vram2pixel( 400, half, bit8 )
#undef	TYPE
#define	TYPE	bit16
#include "screen-400.h"
INLINE	vram2pixel( 400, half, bit16 )
#undef	TYPE
#define	TYPE	bit32
#include "screen-400.h"
INLINE	vram2pixel( 400, half, bit32 )
#undef	TYPE
#undef	HALF

#define	DOUBLE
#define	TYPE	bit8
#include "screen-400.h"
INLINE	vram2pixel( 400, double, bit8 )
#undef	TYPE
#define	TYPE	bit16
#include "screen-400.h"
INLINE	vram2pixel( 400, double, bit16 )
#undef	TYPE
#define	TYPE	bit32
#include "screen-400.h"
INLINE	vram2pixel( 400, double, bit32 )
#undef	TYPE
#undef	DOUBLE

#undef	HIRESO


#define	CLEAR			/* ����   640x200/640x400-------------------*/

#define	FULL
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( clear, full, bit8 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( clear, full, bit8_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( clear, full, bit16 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( clear, full, bit16_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( clear, full, bit32 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( clear, full, bit32_interlace )
#undef	INTERLACE
#undef	TYPE
#undef	FULL

#define	HALF
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( clear, half, bit8 )
#define	INTERP
#include "screen-200.h"
INLINE	vram2pixel( clear, half, bit8_interp )
#undef	INTERP
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( clear, half, bit16 )
#define	INTERP
#include "screen-200.h"
INLINE	vram2pixel( clear, half, bit16_interp )
#undef	INTERP
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( clear, half, bit32 )
#define	INTERP
#include "screen-200.h"
INLINE	vram2pixel( clear, half, bit32_interp )
#undef	INTERP
#undef	TYPE
#undef	HALF

#define	DOUBLE
#define	TYPE	bit8
#include "screen-200.h"
INLINE	vram2pixel( clear, double, bit8 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( clear, double, bit8_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit16
#include "screen-200.h"
INLINE	vram2pixel( clear, double, bit16 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( clear, double, bit16_interlace )
#undef	INTERLACE
#undef	TYPE
#define	TYPE	bit32
#include "screen-200.h"
INLINE	vram2pixel( clear, double, bit32 )
#define	INTERLACE
#include "screen-200.h"
INLINE	vram2pixel( clear, double, bit32_interlace )
#undef	INTERLACE
#undef	TYPE
#undef	DOUBLE





     
/*------------------------------------------------------*/
/* �\���p�o�b�t�@�����ŃN���A���� (�{�[�_�[���܂߂�)	*/
/*							*/
/*	black �̐F�ŃN���A����B			*/
/*------------------------------------------------------*/
#define	pixel_clear_all( type )				\
do{							\
  Uint	i,j;						\
  type *p = (type *)screen_buf;				\
  for( j=HEIGHT; j; j-- ){				\
    for( i=WIDTH; i; i-- ){				\
      *p++ = black;					\
    }							\
  }							\
}while(0)



/*----------------------------------------------------------------------*/

static	void	vram2screen( void )
{
  if( grph_ctrl & GRPH_CTRL_VDISP ){		/* VRAM �\������ */

    if( screen_size == SCREEN_SIZE_FULL ){		/* Full Size */

      if( grph_ctrl & GRPH_CTRL_COLOR ){			/* �J���[ */
	if( use_interlace ){
	  if     ( DEPTH<= 8 ) vram2pixel_color_full_bit8_interlace();
	  else if( DEPTH<=16 ) vram2pixel_color_full_bit16_interlace();
	  else                 vram2pixel_color_full_bit32_interlace();
	}else{
	  if     ( DEPTH<= 8 ) vram2pixel_color_full_bit8();
	  else if( DEPTH<=16 ) vram2pixel_color_full_bit16();
	  else                 vram2pixel_color_full_bit32();
	}
      }else{
	if( grph_ctrl & GRPH_CTRL_200 ){			/* ���� */
	  if( use_interlace ){
	    if     ( DEPTH<= 8 ) vram2pixel_mono_full_bit8_interlace();
	    else if( DEPTH<=16 ) vram2pixel_mono_full_bit16_interlace();
	    else                 vram2pixel_mono_full_bit32_interlace();
	  }else{
	    if     ( DEPTH<= 8 ) vram2pixel_mono_full_bit8();
	    else if( DEPTH<=16 ) vram2pixel_mono_full_bit16();
	    else                 vram2pixel_mono_full_bit32();
	  }
	}else{							/* 400���C�� */
	  if     ( DEPTH<= 8 ) vram2pixel_400_full_bit8();
	  else if( DEPTH<=16 ) vram2pixel_400_full_bit16();
	  else                 vram2pixel_400_full_bit32();
	}
      }

    }else if( screen_size == SCREEN_SIZE_HALF ){	/* Half Size */

      if( grph_ctrl & GRPH_CTRL_COLOR ){			/* �J���[ */
	if( now_half_interp ){		/* �t�B���^�L�� */
	  if     ( DEPTH<= 8 ) vram2pixel_color_half_bit8_interp();
	  else if( DEPTH<=16 ) vram2pixel_color_half_bit16_interp();
	  else                 vram2pixel_color_half_bit32_interp();
	}else{				/* �t�B���^���� */
	  if     ( DEPTH<= 8 ) vram2pixel_color_half_bit8();
	  else if( DEPTH<=16 ) vram2pixel_color_half_bit16();
	  else                 vram2pixel_color_half_bit32();
	}
      }else{
	if( grph_ctrl & GRPH_CTRL_200 ){			/* ���� */
	  if     ( DEPTH<= 8 ) vram2pixel_mono_half_bit8();
	  else if( DEPTH<=16 ) vram2pixel_mono_half_bit16();
	  else                 vram2pixel_mono_half_bit32();
	}else{							/* 400���C�� */
	  if     ( DEPTH<= 8 ) vram2pixel_400_half_bit8();
	  else if( DEPTH<=16 ) vram2pixel_400_half_bit16();
	  else                 vram2pixel_400_half_bit32();
	}
      }

    }else{						/* Double Size */
    
      if( grph_ctrl & GRPH_CTRL_COLOR ){			/* �J���[ */
	if( use_interlace ){
	  if     ( DEPTH<= 8 ) vram2pixel_color_double_bit8_interlace();
	  else if( DEPTH<=16 ) vram2pixel_color_double_bit16_interlace();
	  else                 vram2pixel_color_double_bit32_interlace();
	}else{
	  if     ( DEPTH<= 8 ) vram2pixel_color_double_bit8();
	  else if( DEPTH<=16 ) vram2pixel_color_double_bit16();
	  else                 vram2pixel_color_double_bit32();
	}
      }else{
	if( grph_ctrl & GRPH_CTRL_200 ){			/* ���� */
	  if( use_interlace ){
	    if     ( DEPTH<= 8 ) vram2pixel_mono_double_bit8_interlace();
	    else if( DEPTH<=16 ) vram2pixel_mono_double_bit16_interlace();
	    else                 vram2pixel_mono_double_bit32_interlace();
	  }else{
	    if     ( DEPTH<= 8 ) vram2pixel_mono_double_bit8();
	    else if( DEPTH<=16 ) vram2pixel_mono_double_bit16();
	    else                 vram2pixel_mono_double_bit32();
	  }
	}else{							/* 400���C�� */
	  if     ( DEPTH<= 8 ) vram2pixel_400_double_bit8();
	  else if( DEPTH<=16 ) vram2pixel_400_double_bit16();
	  else                 vram2pixel_400_double_bit32();
	}
      }

    }

  }else{					/* VRAM �\�����Ȃ� */

    screen_update_hide = TRUE;				/* ��\���σt���OON */

    if( screen_size == SCREEN_SIZE_FULL ){		/* Full Size */
      if( use_interlace ){
	if     ( DEPTH<= 8 ) vram2pixel_clear_full_bit8_interlace();
	else if( DEPTH<=16 ) vram2pixel_clear_full_bit16_interlace();
	else                 vram2pixel_clear_full_bit32_interlace();
      }else{
	if     ( DEPTH<= 8 ) vram2pixel_clear_full_bit8();
	else if( DEPTH<=16 ) vram2pixel_clear_full_bit16();
	else                 vram2pixel_clear_full_bit32();
      }
    }else if( screen_size == SCREEN_SIZE_HALF ){	/* Half Size */
      if( now_half_interp ){		/* �t�B���^�L�� */
	if     ( DEPTH<= 8 ) vram2pixel_clear_half_bit8_interp();
	else if( DEPTH<=16 ) vram2pixel_clear_half_bit16_interp();
	else                 vram2pixel_clear_half_bit32_interp();
      }else{
	if     ( DEPTH<= 8 ) vram2pixel_clear_half_bit8();
	else if( DEPTH<=16 ) vram2pixel_clear_half_bit16();
	else                 vram2pixel_clear_half_bit32();
      }
    }else{						/* Double Size */
      if( use_interlace ){
	if     ( DEPTH<= 8 ) vram2pixel_clear_double_bit8_interlace();
	else if( DEPTH<=16 ) vram2pixel_clear_double_bit16_interlace();
	else                 vram2pixel_clear_double_bit32_interlace();
      }else{
	if     ( DEPTH<= 8 ) vram2pixel_clear_double_bit8();
	else if( DEPTH<=16 ) vram2pixel_clear_double_bit16();
	else                 vram2pixel_clear_double_bit32();
      }
    }

  }
}



/********************************************************/
/* PC8801 �p���b�g �̓��e�� �V�X�e���p���b�g�ɕϊ� 	*/
/********************************************************/
static	void	set_palette( void )
{
  int     i;
  SYSTEM_PALETTE_T	syspal[16];


	/* VRAM �� �J���[�p���b�g�ݒ�   syspal[0]�`[7] */

  if( grph_ctrl & GRPH_CTRL_COLOR ){		/* VRAM �J���[ */

    if( monitor_analog ){
#if	0
      for( i=0; i<8; i++ ){
	syspal[i].red   = vram_palette[i].red;
	syspal[i].green = vram_palette[i].green;
	syspal[i].blue  = vram_palette[i].blue;
      }
#else
      for( i=0; i<8; i++ ){
	syspal[i].red   = vram_palette[i].red   * 73 >> 6;
	syspal[i].green = vram_palette[i].green * 73 >> 6;
	syspal[i].blue  = vram_palette[i].blue  * 73 >> 6;
      }
#endif
    }else{
      for( i=0; i<8; i++ ){
	syspal[i].red   = vram_palette[i].red   ? 0xff : 0;
	syspal[i].green = vram_palette[i].green ? 0xff : 0;
	syspal[i].blue  = vram_palette[i].blue  ? 0xff : 0;
      }
    }

  }else{					/* VRAM ���� */

    if( monitor_analog ){
#if	0
      syspal[0].red   = vram_bg_palette.red;
      syspal[0].green = vram_bg_palette.green;
      syspal[0].blue  = vram_bg_palette.blue;
#else
      syspal[0].red   = vram_bg_palette.red   * 73 >> 6;
      syspal[0].green = vram_bg_palette.green * 73 >> 6;
      syspal[0].blue  = vram_bg_palette.blue  * 73 >> 6;
#endif
    }else{
      syspal[0].red   = vram_bg_palette.red   ? 0xff : 0;
      syspal[0].green = vram_bg_palette.green ? 0xff : 0;
      syspal[0].blue  = vram_bg_palette.blue  ? 0xff : 0;
    }
    for( i=1; i<8; i++ ){
      syspal[i].red   = 0;
      syspal[i].green = 0;
      syspal[i].blue  = 0;
    }

  }


	/* TEXT �� �J���[�p���b�g�ݒ�   syspal[8]�`[15] */

  if( !crtc_attr_color ){			/* TEXT ���� */

    syspal[8].red   = 0;
    syspal[8].green = 0;
    syspal[8].blue  = 0;
    for( i=9; i<16; i++ ){
      syspal[i].red   = 0xff;
      syspal[i].green = 0xff;
      syspal[i].blue  = 0xff;
    }

  }else{					/* TEXT �J���[ */

    if( grph_ctrl & GRPH_CTRL_COLOR ){			/* VRAM �J���[ */

      for( i=8; i<16; i++ ){
	syspal[i].red   = (i&0x02) ? 0xff : 0;
	syspal[i].green = (i&0x04) ? 0xff : 0;
	syspal[i].blue  = (i&0x01) ? 0xff : 0;
      }

    }else{						/* VRAM ����   */

      if( monitor_analog ){
#if	0
	for( i=8; i<16; i++ ){
	  syspal[i].red   = vram_palette[i&0x7].red;
	  syspal[i].green = vram_palette[i&0x7].green;
	  syspal[i].blue  = vram_palette[i&0x7].blue;
	}
#else
	for( i=8; i<16; i++ ){
	  syspal[i].red   = vram_palette[i&0x7].red   * 73 >> 6;
	  syspal[i].green = vram_palette[i&0x7].green * 73 >> 6;
	  syspal[i].blue  = vram_palette[i&0x7].blue  * 73 >> 6;
	}
#endif
      }else{
	for( i=8; i<16; i++ ){
	  syspal[i].red   = vram_palette[i&0x7].red   ? 0xff : 0;
	  syspal[i].green = vram_palette[i&0x7].green ? 0xff : 0;
	  syspal[i].blue  = vram_palette[i&0x7].blue  ? 0xff : 0;
	}
      }

    }
  }

  trans_palette( syspal );

}



/************************************************************************/
/* screen_buf �̏�����							*/
/*	screen_buf �����F (black)�œh��Ԃ�				*/
/*	    �{�[�_�[���܂߂ēh��Ԃ��̂ŁA�O���t�B�b�N�̏��������Ȃ�	*/
/*	    �ŌĂ�							*/
/************************************************************************/
void	screen_buf_init( void )
{
  int     i;
  SYSTEM_PALETTE_T	syspal[16];

  for( i=0; i<16; i++ ){
    syspal[i].red   = (i&2) ? 0xff : 0;
    syspal[i].green = (i&4) ? 0xff : 0;
    syspal[i].blue  = (i&1) ? 0xff : 0;
  }
  trans_palette( syspal );

  if     ( DEPTH<= 8 ) pixel_clear_all( bit8  );
  else if( DEPTH<=16 ) pixel_clear_all( bit16 );
  else                 pixel_clear_all( bit32 );

  put_image();
}





/************************************************************************/
/* �_�� (�J�[�\������сA���������̓_��) �����̂��߂̃��[�N�ݒ�		*/
/*	frameskip_rate, blink_cycle ���ύX����邽�тɌĂ�		*/
/************************************************************************/
static	int	blink_ctrl_cycle   = 1;
static	int	blink_ctrl_counter = 0;
void	blink_ctrl_update( void )
{
  int	wk;

  wk = blink_cycle / frameskip_rate;

  if( wk==0 ||
     !( blink_cycle -wk*frameskip_rate < (wk+1)*frameskip_rate -blink_cycle ) )
    wk++;
  
  blink_ctrl_cycle = wk;
  blink_ctrl_counter = blink_ctrl_cycle;
}


/************************************************************************/
/* �C���[�W�]�� (�\��)							*/
/*	1/60 sec ���ɌĂ΂�� (�\���́A�w�肵���t���[�����ɍs�Ȃ�)	*/
/************************************************************************/
int	frame_counter = 0;
void	draw_screen( void )
{

  if( (frame_counter%frameskip_rate)==0 ){

	/* �E�G�C�g�����̏ꍇ�A�I�[�g�X�L�b�v�͖��� */

    if( no_wait || !use_auto_skip || !do_skip_draw ){

      if( screen_update_palette ){
	screen_update_palette = FALSE; 
	set_palette();			/* �p���b�g���V�X�e���ɓ]��	*/
      }

      check_screen_update_force();	/* VRAM�X�V���̃`�F�b�N	*/
      clear_indicator();		/* �s�v�ȃC���W�P�[�^�̏���	*/
      make_text_attr();			/* TVRAM �� �����ꗗ�쐬	*/
      vram2screen();			/* VRAM/TEXT �� screen_buf �]��	*/
      reset_screen_update();		/* VRAM�X�V��񃊃Z�b�g		*/

      if( indicator_flag & (INDICATE_MES|INDICATE_FDD) ){
	disp_indicator();		/* �C���W�P�[�^�\��		*/
      }

      if( do_update ){
	put_image();			/* screen_buf �\��		*/
	do_update = FALSE;
      }
    }else{
	already_skip_draw = TRUE;	/* �I�[�g�X�L�b�v */
    }

    if( --blink_ctrl_counter == 0 ){	/* �J�[�\���_�Ń��[�N�X�V	*/
      blink_ctrl_counter = blink_ctrl_cycle;
      blink_counter ++;
    }
  }

  ++ frame_counter;
}



/************************************************************************/
/* �C���[�W�]�� (�ĕ`��)						*/
/************************************************************************/
void	redraw_screen( int do_disp_indicator )
{

  set_screen_update_force();		/* VRAM�X�V����S�ύX		*/

  set_palette();

  check_screen_update_force();
  make_text_attr();
  vram2screen();
  reset_screen_update();

  if( do_disp_indicator )
    if( indicator_flag & (INDICATE_MES|INDICATE_FDD) ){
      disp_indicator();
      set_screen_update_force();
    }

  put_image();
}









/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_screen_work[]=
{
  { TYPE_INT,	&frameskip_rate,	},
  { TYPE_INT,	&monitor_analog,	},

  { TYPE_CHAR,	&vram_bg_palette.blue,	},
  { TYPE_CHAR,	&vram_bg_palette.red,	},
  { TYPE_CHAR,	&vram_bg_palette.green,	},

  { TYPE_CHAR,	&vram_palette[0].blue,	},
  { TYPE_CHAR,	&vram_palette[0].red,	},
  { TYPE_CHAR,	&vram_palette[0].green,	},
  { TYPE_CHAR,	&vram_palette[1].blue,	},
  { TYPE_CHAR,	&vram_palette[1].red,	},
  { TYPE_CHAR,	&vram_palette[1].green,	},
  { TYPE_CHAR,	&vram_palette[2].blue,	},
  { TYPE_CHAR,	&vram_palette[2].red,	},
  { TYPE_CHAR,	&vram_palette[2].green,	},
  { TYPE_CHAR,	&vram_palette[3].blue,	},
  { TYPE_CHAR,	&vram_palette[3].red,	},
  { TYPE_CHAR,	&vram_palette[3].green,	},
  { TYPE_CHAR,	&vram_palette[4].blue,	},
  { TYPE_CHAR,	&vram_palette[4].red,	},
  { TYPE_CHAR,	&vram_palette[4].green,	},
  { TYPE_CHAR,	&vram_palette[5].blue,	},
  { TYPE_CHAR,	&vram_palette[5].red,	},
  { TYPE_CHAR,	&vram_palette[5].green,	},
  { TYPE_CHAR,	&vram_palette[6].blue,	},
  { TYPE_CHAR,	&vram_palette[6].red,	},
  { TYPE_CHAR,	&vram_palette[6].green,	},
  { TYPE_CHAR,	&vram_palette[7].blue,	},
  { TYPE_CHAR,	&vram_palette[7].red,	},
  { TYPE_CHAR,	&vram_palette[7].green,	},

  { TYPE_BYTE,	&sys_ctrl,	},
  { TYPE_BYTE,	&grph_ctrl,	},
  { TYPE_BYTE,	&grph_pile,	},

  /*{ TYPE_INT,	&blink_ctrl_cycle,	},	�����l�ł����Ȃ����낤 */
  /*{ TYPE_INT,	&blink_ctrl_counter,	},	�����l�ł����Ȃ����낤 */
  /*{ TYPE_INT,	&frame_counter,		},	�����l�ł����Ȃ����낤 */
};


int	suspend_screen( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_screen_work, 
			    countof(suspend_screen_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->screen ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_screen(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_screen_work, 
			   countof(suspend_screen_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}
















/* �f�o�b�O�p�̊֐� */
void attr_misc(void)
{
int i;

  text_attr_flipflop ^= 1;    
  for(i=0;i<80;i++){
    printf("%02X[%02X] ",
    text_attr_buf[text_attr_flipflop][19*80+i]>>8,
    text_attr_buf[text_attr_flipflop][19*80+i]&0xff );
  }
return;
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][9*80+i]>>8,
    text_attr_buf[text_attr_flipflop][9*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][10*80+i]>>8,
    text_attr_buf[text_attr_flipflop][10*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][11*80+i]>>8,
    text_attr_buf[text_attr_flipflop][11*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][12*80+i]>>8,
    text_attr_buf[text_attr_flipflop][12*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][13*80+i]>>8,
    text_attr_buf[text_attr_flipflop][13*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][14*80+i]>>8,
    text_attr_buf[text_attr_flipflop][14*80+i]&0xff );
  }
#if 0
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][15*80+i]>>8,
    text_attr_buf[0][15*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][16*80+i]>>8,
    text_attr_buf[0][16*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][17*80+i]>>8,
    text_attr_buf[0][17*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][18*80+i]>>8,
    text_attr_buf[0][18*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][19*80+i]>>8,
    text_attr_buf[0][19*80+i]&0xff );
  }
  printf("\n");
#endif
}
