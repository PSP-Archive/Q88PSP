#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED


extern	int	WIDTH;			/* ウィンドウ横サイズ		*/
extern	int	HEIGHT;			/* ウィンドウ縦サイズ		*/
extern	int	DEPTH;			/* 色ビット数	(8/16/32)	*/
extern	int	SCREEN_W;		/* 画面横サイズ (320/640/1280)	*/
extern	int	SCREEN_H;		/* 画面縦サイズ (200/400/800)	*/
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
extern	int	screen_size;		/* 画面サイズ 0:標準/1:半分/2:倍*/
extern	int	screen_size_max;

extern	char	*screen_buf;		/* 表示用バッファ 	 	*/
					/* 640x400 or 320x200		*/
					/* 8(char)or16(short)or32(long)	*/

extern	char	*screen_start;		/* 表示バッファ描画開始位置	*/


extern	int	hide_mouse;		/* マウス非表示するかどうか	*/
extern	int	grab_mouse;		/* グラブするかどうか		*/

extern	Ulong	color_pixel[16];	/* 16パレット分のpixel値	*/
extern	Ulong	color_half_pixel[16][16];	/* halfサイズ用の
						   フィルタパレット値	*/


extern	int	colormap_type;		/* カラーマップのタイプ	0/1/2	*/
#ifdef MITSHM
extern	int	use_SHM;		/* MIT-SHM を使用するかどうか	*/
#endif
#ifdef USE_DGA
extern	int	use_DGA;		/* XF86-DGA を使用するかどうか	*/
extern	int	enable_DGA;		/* XF86-DGA を使用したかどうか	*/
extern	int	restart_DGA;		/* XF86-DGA で再初期化したい時	*/
#endif





typedef struct{
  unsigned	char	red;			/* Ｒ面輝度 0x00～0xff	*/
  unsigned 	char	green;			/* Ｇ面輝度 0x00～0xff	*/
  unsigned	char	blue;			/* Ｂ面輝度 0x00～0xff	*/
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
