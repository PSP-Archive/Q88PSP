#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED


extern	int	WIDTH;			/* �E�B���h�E���T�C�Y		*/
extern	int	HEIGHT;			/* �E�B���h�E�c�T�C�Y		*/
extern	int	DEPTH;			/* �F�r�b�g��	(8/16/32)	*/
extern	int	SCREEN_W;		/* ��ʉ��T�C�Y (320/640/1280)	*/
extern	int	SCREEN_H;		/* ��ʏc�T�C�Y (200/400/800)	*/
extern	int	SCREEN_DX;
extern	int	SCREEN_DY;
extern	int	VIEWPORT_W;
extern	int	VIEWPORT_H;
extern	int	VIEWPORT_DX;
extern	int	VIEWPORT_DY;
extern	int	SCREEN_OFFSET;
extern	Ulong	black;


enum {
  SCREEN_SIZE_HALF,
  SCREEN_SIZE_FULL,
  SCREEN_SIZE_DOUBLE,
  END_of_SCREEN_SIZE
};
extern	int	screen_size;		/* ��ʃT�C�Y 0:�W��/1:����/2:�{*/
extern	int	screen_size_max;

extern	char	*screen_buf;		/* �\���p�o�b�t�@ 	 	*/
					/* 640x400 or 320x200		*/
					/* 8(char)or16(short)or32(long)	*/

extern	char	*screen_start;		/* �\���o�b�t�@�`��J�n�ʒu	*/


extern	int	hide_mouse;		/* �}�E�X��\�����邩�ǂ���	*/
extern	int	grab_mouse;		/* �O���u���邩�ǂ���		*/

extern	Ulong	color_pixel[16];	/* 16�p���b�g����pixel�l	*/
extern	Ulong	color_half_pixel[16][16];	/* half�T�C�Y�p��
						   �t�B���^�p���b�g�l	*/


extern	int	colormap_type;		/* �J���[�}�b�v�̃^�C�v	0/1/2	*/
#ifdef MITSHM
extern	int	use_SHM;		/* MIT-SHM ���g�p���邩�ǂ���	*/
#endif
#ifdef USE_DGA
extern	int	use_DGA;		/* XF86-DGA ���g�p���邩�ǂ���	*/
extern	int	enable_DGA;		/* XF86-DGA ���g�p�������ǂ���	*/
extern	int	restart_DGA;		/* XF86-DGA �ōď�������������	*/
#endif





typedef struct{
  unsigned	char	red;			/* �q�ʋP�x 0x00�`0xff	*/
  unsigned 	char	green;			/* �f�ʋP�x 0x00�`0xff	*/
  unsigned	char	blue;			/* �a�ʋP�x 0x00�`0xff	*/
  unsigned 	char	padding;
} SYSTEM_PALETTE_T;


int	graphic_system_init( void );
void	graphic_system_term( void );
void	graphic_system_restart( int redraw_flag );

void	put_image( void );
void	trans_palette( SYSTEM_PALETTE_T syspal[] );

void	set_mouse_visible( void );
void	set_mouse_invisible( void );




#endif	/* GRAPH_H_INCLUDED */
