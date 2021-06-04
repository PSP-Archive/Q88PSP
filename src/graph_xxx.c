/************************************************************************/
/*									*/
/* �O���t�B�b�N���� (OS�ˑ�)						*/
/*									*/
/*									*/
/* �y�֐��z								*/
/*									*/
/* int  graphic_system_init( void )					*/
/* void graphic_system_term( void )					*/
/* void graphic_system_restart( int redraw_flag )			*/
/* void put_image( void )						*/
/* void trans_palette( SYSTEM_PALETTE_T syspal[] )			*/
/* void set_mouse_visible( void )					*/
/* void set_mouse_invisible( void )					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	/* maybe not need ... future */
// #include <SDL.h>

#include "quasi88.h"
#include "initval.h"
#include "graph.h"
#include "screen.h"
#include "file-op.h"

#include "device.h"


#ifdef USE_DGA
int	use_DGA;			/* �t���X�N���[���\������?	*/
#endif

int	colormap_type   = 0;		/* �J���[�}�b�v�̃^�C�v	0/1/2	*/
int	hide_mouse	= FALSE;	/* �}�E�X��\�����邩�ǂ���	*/
int	grab_mouse	= FALSE;	/* �O���u���邩�ǂ���		*/

int	WIDTH  = 0;			/* ��ʃo�b�t�@���T�C�Y		*/
int	HEIGHT = 0;			/* ��ʃo�b�t�@�c�T�C�Y		*/
int	DEPTH  = 16;			/* �F�r�b�g��	(8 or 16 or 32)	*/
int	SCREEN_W = 0;			/* �`��G���A���T�C�Y		*/
int	SCREEN_H = 0;			/* �`��G���A�c�T�C�Y		*/
int	SCREEN_DX = 0;
int	SCREEN_DY = 0;
int	VIEWPORT_W = 0;			/* �\���G���A���T�C�Y		*/
int	VIEWPORT_H = 0;			/* �\���G���A�c�T�C�Y		*/
int	VIEWPORT_DX = 0;
int	VIEWPORT_DY = 0;
int	SCREEN_OFFSET;

static	int     SIZE_OF_DEPTH;		/* �F�o�C�g��	(1 or 2 or 4)	*/

int	screen_size = SCREEN_SIZE_HALF;	/* ��ʃT�C�Y 0:����/1:�W��/2:�{*/
int	screen_size_max = SCREEN_SIZE_FULL;

char	*screen_buf;			/* �\���p�o�b�t�@	 	*/
					/* 640x400 or 320x200 or1280x800*/
					/* 8(char)or16(short)or32(long)	*/

char	*screen_start;			/* �\���o�b�t�@�`��J�n�ʒu	*/


	Ulong	color_pixel[16];	/* 16�p���b�g����pixel�l	*/
	Ulong	color_half_pixel[16][16];	/* half�T�C�Y�p��
					 �t�B���^�����O�����p���b�g�l	*/



//SDL_Surface *display;
//SDL_Surface *offscreen;
//SDL_Surface *fn_button;

	Ulong    white, black;

static const struct{
  int w, h;
} screen_size_tbl[ END_of_SCREEN_SIZE ] = {
  {  320,  200, },	/* SCREEN_SIZE_HALF	*/
//  {  640,  400, },	/* SCREEN_SIZE_FULL	*/
  {  480,  272, },	/* SCREEN_SIZE_FULL	*/
  { 1280,  800, },	/* SCREEN_SIZE_DOUBLE	*/
};

// 480 x 272

static const struct{
  int w, h;
} display_size_tbl[ END_of_SCREEN_SIZE ] = {
  {  512,  272, },	/* SCREEN_SIZE_HALF	*/
//  {  640,  480, },	/* SCREEN_SIZE_FULL	*/
  {  512,  272, },	/* SCREEN_SIZE_FULL	*/
  { 1280,  960, },	/* SCREEN_SIZE_DOUBLE	*/
};

/******************************************************************************

                                 WIDTH
	 ����������������������������������������������������

	+----------------------------------------------------+
	| VIEWPORT_DY��   �� SCREEN_DY                       |  ��
	|            ��   ��                                 |  ��
	|            +----��--------------------+            |  ��
	|������������|��������������������������|            |  ��
	|VIEWPORT_DX |    ��     VIEWPORT_W     |            |  ��
	|            |   +------------------+   |            |  ��
	|����������������|������������������|   |            |  ��
	|    SCREEN_DX   |  ��   SCREEN_W   |   |            |  �� HEIGHT
	|            |   |  ��              |   |            |  ��
	|            |   |  ��SCREEN_H      |   |            |  ��
	|            |   |  ��              |   |            |  ��
	|            |   |  ��              |   |            |  ��
	|            |   +------------------+   |            |  ��
	|            |                          |�������������c ��
       �c������������+--------------------------+            |  ��
       SCREEN_OFFSET                                         |  ��
	|                                                    |  ��
	+----------------------------------------------------+

	WIDTH		�m�ۂ����O���t�B�b�N�o�b�t�@�̉��T�C�Y
	HEIGHT		              �V              �c�T�C�Y

	SCREEN_W	�`��G���A�̃T�C�Y �� (320/640/1280�̂����ꂩ)
	SCREEN_H	        �V         �c (200/400/ 800�̂����ꂩ)
	SCREEN_DX	�o�b�t�@�̍��[����`��G���A�̍��[�܂ł̋���
	SCREEN_DY	    �V    ��[����     �V     ��[�܂ł̋���

	VIEWPORT_W	�\���G���A�̃T�C�Y ��
	VIEWPORT_H	        �V         �c
	VIEWPORT_DX	�o�b�t�@�̍��[����\���G���A�̍��[�܂ł̋���
	VIEWPORT_DY	    �V    ��[����     �V     ��[�܂ł̋���

	(�ʏ�́ASCREEN_* �� VIEWPORT_* �͈�v)


	SCREEN_OFFSET	�`��G���A�̉E�[����A���̍��[�܂ł̋���
				= WIDTH - SCREEN_W;
	DEPTH		�F��depth (8/16/32)
	SIZE_OF_DEPTH	log2(DEPTH)/8  = 1/2/4

******************************************************************************/




static int zaurus_mode = FALSE,zaurus_mx = 0,zaurus_my = 0,func_side = 0;

static const char *parse_tilda( const char *fname );

/************************************************************************/
/* �O���t�B�b�N�V�X�e�� (X11) �̏�����					*/
/************************************************************************/
int	graphic_system_init( void )
{
  // TODO: allocate screen_start 

 // pgInit();

  black = 0;
  white = 0xffffffff;

	/* �E�C���h�E�T�C�Y���Z�b�g		*/
	/* WIDTH/HEIGHT �̓R�}���h���C���Ŏ擾	*/

  SCREEN_W = screen_size_tbl[screen_size].w;
  SCREEN_H = screen_size_tbl[screen_size].h;

  WIDTH  = display_size_tbl[screen_size].w;
  HEIGHT = display_size_tbl[screen_size].h;  

  SCREEN_OFFSET = WIDTH - SCREEN_W;

  SCREEN_DX = ( ( WIDTH - SCREEN_W ) / 2 - 16) & ~7;	/* 8�̔{�� */
  SCREEN_DY = ( ( HEIGHT- SCREEN_H ) / 2 - 16 ) & ~1;	/* 2�̔{�� */

	if(screen_size == SCREEN_SIZE_FULL){
		SCREEN_DX = 0;
		SCREEN_DY = 0;
	}

  VIEWPORT_W  = WIDTH;
  VIEWPORT_H  = HEIGHT;
  VIEWPORT_DX = 0;
  VIEWPORT_DY = 0;

  DEPTH = 16;
  SIZE_OF_DEPTH = 2;

  screen_buf = pgGetVramAddr(0,0);
  if( verbose_proc ) printf("  Allocating screen buffer...OK\n");


  /* �X�N���[���o�b�t�@�́A�`��J�n�ʒu��ݒ�	*/
  
	screen_start = &screen_buf[ (WIDTH*SCREEN_DY + SCREEN_DX) * SIZE_OF_DEPTH ];
//   screen_start = screen_buf;
  


  return(1);
}

/************************************************************************/
/* �O���t�B�b�N�V�X�e�� (X11) �̏I��					*/
/************************************************************************/
void	graphic_system_term( void )
{
}



/********************************************************/
/* �p���b�g�ݒ�						*/
/********************************************************/
void	trans_palette( SYSTEM_PALETTE_T syspal[] )
{
 // to do: make this routine to display correctly
  int     i, j;

	/* �p���b�g�l���R�s�[ */

  for( i=0; i<16; i++ ){
    color_pixel[i] = (((syspal[i].blue  >>3)&0x1f) <<10)|
		     (((syspal[i].green>>3)&0x1f) << 5)|
		     (((syspal[i].red >>3)&0x1f));
  }


	/* ���� HALF �T�C�Y�Ńt�B���^�����O���\�����`�F�b�N */

  if( screen_size == SCREEN_SIZE_HALF &&
/*    grph_ctrl & GRPH_CTRL_COLOR &&  */
      use_half_interp ){
    now_half_interp = TRUE;
  }else{
    now_half_interp = FALSE;
  }

	/* HALF�T�C�Y�t�B���^�����O�\���̓t�B���^�p���b�g�l���v�Z */
//    now_half_interp = TRUE;//������Bit��⊮���邩�ǂ����B�_���N=�t�@�N�g�̕����̏��������Ȃ邩�ۂ��B

  if( now_half_interp ){
    SYSTEM_PALETTE_T hpal[16];
    for( i=0; i<16; i++ ){
      hpal[i].blue   = syspal[i].blue   >> 1;
      hpal[i].green = syspal[i].green >> 1;
      hpal[i].red  = syspal[i].red  >> 1;
    }

    for( i=0; i<16; i++ ){
      color_half_pixel[i][i] = color_pixel[i];
    }
    for( i=0; i<16; i++ ){
      for( j=i+1; j<16; j++ ){
	color_half_pixel[i][j]=((((hpal[i].blue  +hpal[j].blue  )>>3)&0x1f)<<10)|
			       ((((hpal[i].green+hpal[j].green)>>3)&0x1f)<< 5)|
			       ((((hpal[i].red +hpal[j].red )>>3)&0x1f));
	color_half_pixel[j][i] = color_half_pixel[i][j];
      }
    }
  }


}




/************************************************************************/
/* ��ʕ\��								*/
/************************************************************************/
void	put_image( void )
{
}






/************************************************************************/
/* �}�E�X�̕\���^��\�����s�Ȃ��֐�					*/
/************************************************************************/
void	set_mouse_visible( void )
{
}

void	set_mouse_invisible( void )
{
}



/************************************************************************/
/* �O���t�B�b�N�V�X�e�� (X11) �̍ď�����				*/
/*	�O���t�B�b�N�V�X�e������U�I�����A�ēx�A���������s���B		*/
/*	�O�����āAscreen_size �Ȃǂ�ύX���Ă����΁A�V���ȑ傫����	*/
/*	�E�C���h�E����������邱�ƂɂȂ�B				*/
/*	�ď������Ɏ��s�����Ƃ��́A�ǂ����悤���Ȃ��̂ŁA�����I������B	*/
/************************************************************************/
void	graphic_system_restart( int redraw_flag )
{
  int	verbose_proc_saved = verbose_proc;
  verbose_proc = 0;

  /* �O���t�B�b�N�V�X�e������U�I�������� */
  /*graphic_system_term();*/
//  SDL_QuitSubSystem( SDL_INIT_VIDEO );



  /* �E�C���h�E�Đ������ɁA�{�[�_�[�̃T�C�Y�������ɂȂ�悤�ɂ��� */
  WIDTH = display_size_tbl[screen_size].w;
  HEIGHT = display_size_tbl[screen_size].h;  


  if( graphic_system_init() ){

    verbose_proc = verbose_proc_saved;
    if( redraw_flag ) redraw_screen( FALSE );

  }else{

    fprintf(stderr,"Sorry : Graphic System Fatal Error !!!\n");

    verbose_proc = verbose_proc_saved;
    main_exit(-1);

  }
}

int function_mouse_pos(int x, int y)
{
}

void mouse_coord_trans(int x, int y, int *mx, int *my)
{

}

