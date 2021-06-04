/************************************************************************/
/*									*/
/* グラフィック処理 (OS依存)						*/
/*									*/
/*									*/
/* 【関数】								*/
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
#include <SDL.h>

#include "quasi88.h"
#include "initval.h"
#include "graph.h"
#include "screen.h"
#include "file-op.h"

#include "device.h"


#ifdef USE_DGA
int	use_DGA;			/* フルスクリーン表示する?	*/
#endif

int	colormap_type   = 0;		/* カラーマップのタイプ	0/1/2	*/
int	hide_mouse	= FALSE;	/* マウス非表示するかどうか	*/
int	grab_mouse	= FALSE;	/* グラブするかどうか		*/

int	WIDTH  = 0;			/* 画面バッファ横サイズ		*/
int	HEIGHT = 0;			/* 画面バッファ縦サイズ		*/
int	DEPTH  = 16;			/* 色ビット数	(8 or 16 or 32)	*/
int	SCREEN_W = 0;			/* 描画エリア横サイズ		*/
int	SCREEN_H = 0;			/* 描画エリア縦サイズ		*/
int	SCREEN_DX = 0;
int	SCREEN_DY = 0;
int	VIEWPORT_W = 0;			/* 表示エリア横サイズ		*/
int	VIEWPORT_H = 0;			/* 表示エリア縦サイズ		*/
int	VIEWPORT_DX = 0;
int	VIEWPORT_DY = 0;
int	SCREEN_OFFSET;

static	int     SIZE_OF_DEPTH;		/* 色バイト数	(1 or 2 or 4)	*/

int	screen_size = SCREEN_SIZE_FULL;	/* 画面サイズ 0:半分/1:標準/2:倍*/
int	screen_size_max = SCREEN_SIZE_FULL;

char	*screen_buf;			/* 表示用バッファ	 	*/
					/* 640x400 or 320x200 or1280x800*/
					/* 8(char)or16(short)or32(long)	*/

char	*screen_start;			/* 表示バッファ描画開始位置	*/


	Ulong	color_pixel[16];	/* 16パレット分のpixel値	*/
	Ulong	color_half_pixel[16][16];	/* halfサイズ用の
					 フィルタリングしたパレット値	*/



SDL_Surface *display;
SDL_Surface *offscreen;
SDL_Surface *fn_button;

	Ulong    white, black;

static const struct{
  int w, h;
} screen_size_tbl[ END_of_SCREEN_SIZE ] = {
  {  320,  200, },	/* SCREEN_SIZE_HALF	*/
  {  640,  400, },	/* SCREEN_SIZE_FULL	*/
  { 1280,  800, },	/* SCREEN_SIZE_DOUBLE	*/
};

#ifdef	USE_ZAURUS
static const struct{
  int w, h;
} display_size_tbl[ END_of_SCREEN_SIZE ] = {
  {  320,  240, },	/* SCREEN_SIZE_HALF	*/
  {  640,  480, },	/* SCREEN_SIZE_FULL	*/
  { 1280,  960, },	/* SCREEN_SIZE_DOUBLE	*/
};
#endif

/******************************************************************************

                                 WIDTH
	 ←────────────────────────→

	+----------------------------------------------------+
	| VIEWPORT_DY↑   ↑ SCREEN_DY                       |  ↑
	|            ↓   │                                 |  │
	|            +----│--------------------+            |  │
	|←────→|←─│─────────→|            |  │
	|VIEWPORT_DX |    ↓     VIEWPORT_W     |            |  │
	|            |   +------------------+   |            |  │
	|←──────→|←↑──────→|   |            |  │
	|    SCREEN_DX   |  │   SCREEN_W   |   |            |  │ HEIGHT
	|            |   |  │              |   |            |  │
	|            |   |  │SCREEN_H      |   |            |  │
	|            |   |  │              |   |            |  │
	|            |   |  ↓              |   |            |  │
	|            |   +------------------+   |            |  │
	|            |                          |←─────… │
       …─────→+--------------------------+            |  │
       SCREEN_OFFSET                                         |  │
	|                                                    |  ↓
	+----------------------------------------------------+

	WIDTH		確保したグラフィックバッファの横サイズ
	HEIGHT		              〃              縦サイズ

	SCREEN_W	描画エリアのサイズ 横 (320/640/1280のいずれか)
	SCREEN_H	        〃         縦 (200/400/ 800のいずれか)
	SCREEN_DX	バッファの左端から描画エリアの左端までの距離
	SCREEN_DY	    〃    上端から     〃     上端までの距離

	VIEWPORT_W	表示エリアのサイズ 横
	VIEWPORT_H	        〃         縦
	VIEWPORT_DX	バッファの左端から表示エリアの左端までの距離
	VIEWPORT_DY	    〃    上端から     〃     上端までの距離

	(通常は、SCREEN_* と VIEWPORT_* は一致)


	SCREEN_OFFSET	描画エリアの右端から、次の左端までの距離
				= WIDTH - SCREEN_W;
	DEPTH		色のdepth (8/16/32)
	SIZE_OF_DEPTH	log2(DEPTH)/8  = 1/2/4

******************************************************************************/




static int zaurus_mode = FALSE,zaurus_mx = 0,zaurus_my = 0,func_side = 0;

static const char *parse_tilda( const char *fname );

/************************************************************************/
/* グラフィックシステム (X11) の初期化					*/
/************************************************************************/
int	graphic_system_init( void )
{
  int ret;


  if( verbose_proc ) printf("Initializing Graphic System (SDL)\n");


	/* ディスプレイを開く */

  if( verbose_proc ) printf("  Opening display...");
  ret = SDL_Init( SDL_INIT_VIDEO );
  if( ret != 0 ){ if( verbose_proc ) printf("FAILED\n"); return(0); }
  else          { if( verbose_proc ) printf("OK\n");                }

  black = 0;
  white = 0xffffffff;

	/* ウインドウサイズをセット		*/
	/* WIDTH/HEIGHT はコマンドラインで取得	*/

  SCREEN_W = screen_size_tbl[screen_size].w;
  SCREEN_H = screen_size_tbl[screen_size].h;
#ifndef	USE_ZAURUS
  if( WIDTH  < SCREEN_W ) WIDTH  = SCREEN_W;
  if( HEIGHT < SCREEN_H ) HEIGHT = SCREEN_H;
#else
  WIDTH = display_size_tbl[screen_size].w;
  HEIGHT = display_size_tbl[screen_size].h;  
#endif
  SCREEN_OFFSET = WIDTH - SCREEN_W;

  SCREEN_DX = ( ( WIDTH - SCREEN_W ) / 2 ) & ~7;	/* 8の倍数 */
  SCREEN_DY = ( ( HEIGHT- SCREEN_H ) / 2 ) & ~1;	/* 2の倍数 */

  VIEWPORT_W  = WIDTH;
  VIEWPORT_H  = HEIGHT;
  VIEWPORT_DX = 0;
  VIEWPORT_DY = 0;

  /* ウインドウのタイトルを表示 */
  SDL_WM_SetCaption( Q_TITLE " ver " Q_VERSION, Q_TITLE " ver " Q_VERSION );

  /* アイコンを設定するならここで。WIN32の場合、32x32 に限る */
  /* SDL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask); */



  if( use_DGA )					/* フルスクリーン表示の場合 */
  display = SDL_SetVideoMode(WIDTH,HEIGHT, 16,
			     SDL_FULLSCREEN|SDL_HWPALETTE|
			     SDL_HWSURFACE|SDL_DOUBLEBUF);
  else						/* ウインドウ表示の場合 */
  display = SDL_SetVideoMode(WIDTH,HEIGHT, 16,
			     SDL_HWPALETTE|
			     SDL_HWSURFACE|SDL_DOUBLEBUF);


  if( verbose_proc )
      printf("SDL video %d,%d %d/%d %d %X\n", display->w,display->h,
	     display->format->BitsPerPixel,
	     display->format->BytesPerPixel,
	     display->pitch,display->pixels);
  DEPTH = 16;
  SIZE_OF_DEPTH = 2;

  {
      FILE *mfile;
      char *filename;
      int p1 = 0,p2 = 0,p3 = 0,p4 = 0;

      fn_button = NULL;
      if( WIDTH >= 640 && HEIGHT >= 460 )
      {
	filename = osd_get_home_filename( "fnbtn.bmp", NULL );

	if( filename != NULL ){
	  fn_button = SDL_LoadBMP( filename );
	  if (fn_button ){
	    if( verbose_proc ) printf("loading %s..,OK\n", filename );
	  }
	  ex_free( filename );
	}
      }


      mfile = osd_fopen_rcfile( "mfile.rc", NULL );
      if (mfile != NULL) {
	  fscanf(mfile, "%d,%d,%d,%d", &p1, &p2, &p3, &p4);

	  if( verbose_proc ) printf("Zaurus Ext %d,%d,%d,%d\n", p1,p2,p3,p4);

	  if (p1) {
	      zaurus_mode = TRUE;
	      zaurus_mx = p3;
	      zaurus_my = p4;
	  }

	  func_side = p2;

	  ex_fclose(mfile);
      }
  }

  if (fn_button != NULL){
      SDL_Rect drect = { 0,0,0,0 };
      if (func_side == 0)
	  drect.y = HEIGHT-12-8;
      else
	  drect.y = 8;
      
      SDL_BlitSurface (fn_button, NULL, display, &drect);
  }

  /* スクリーンバッファを確保 */
  offscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT,
				   16, 0,0,0,0);
  screen_buf = (char *)offscreen->pixels;
  if( verbose_proc ) printf("  Allocating screen buffer...OK\n");


  /* スクリーンバッファの、描画開始位置を設定	*/
  
  screen_start = &screen_buf[ (WIDTH*SCREEN_DY + SCREEN_DX) * SIZE_OF_DEPTH ];
  /* screen_start = screen_buf; */
  


	/* 画面を一度、クリア */
  screen_buf_init();


	/* X のマウスカーソル表示／非表示を設定 */

  if( hide_mouse ) set_mouse_invisible();
  else		   set_mouse_visible();

	/* グラブの設定	*/

  if( grab_mouse ){
    SDL_WM_GrabInput( SDL_GRAB_ON );
  }

  return(1);
}

/************************************************************************/
/* グラフィックシステム (X11) の終了					*/
/************************************************************************/
void	graphic_system_term( void )
{
    if( grab_mouse ){
      SDL_WM_GrabInput( SDL_GRAB_OFF );
    }

    SDL_Quit();
}



/********************************************************/
/* パレット設定						*/
/********************************************************/
void	trans_palette( SYSTEM_PALETTE_T syspal[] )
{
  int     i, j;

	/* パレット値をコピー */

  for( i=0; i<16; i++ ){
    color_pixel[i] = (((syspal[i].red  >>3)&0x1f) <<11)|
		     (((syspal[i].green>>3)&0x1f) << 6)|
		     (((syspal[i].blue >>3)&0x1f));
  }


	/* 現在 HALF サイズでフィルタリングが可能かをチェック */

  if( screen_size == SCREEN_SIZE_HALF &&
/*    grph_ctrl & GRPH_CTRL_COLOR &&  */
      use_half_interp ){
    now_half_interp = TRUE;
  }else{
    now_half_interp = FALSE;
  }

	/* HALFサイズフィルタリング可能時はフィルタパレット値を計算 */

  if( now_half_interp ){
    SYSTEM_PALETTE_T hpal[16];
    for( i=0; i<16; i++ ){
      hpal[i].red   = syspal[i].red   >> 1;
      hpal[i].green = syspal[i].green >> 1;
      hpal[i].blue  = syspal[i].blue  >> 1;
    }

    for( i=0; i<16; i++ ){
      color_half_pixel[i][i] = color_pixel[i];
    }
    for( i=0; i<16; i++ ){
      for( j=i+1; j<16; j++ ){
	color_half_pixel[i][j]=((((hpal[i].red  +hpal[j].red  )>>3)&0x1f)<<11)|
			       ((((hpal[i].green+hpal[j].green)>>3)&0x1f)<< 6)|
			       ((((hpal[i].blue +hpal[j].blue )>>3)&0x1f));
	color_half_pixel[j][i] = color_half_pixel[i][j];
      }
    }
  }
}




/************************************************************************/
/* 画面表示								*/
/************************************************************************/
void	put_image( void )
{
  Uint16 *buf = (Uint16 *)screen_buf;

  SDL_Rect srect = { 0,0,0,0 };
  SDL_Rect drect = { 0,0,0,0 };

  srect.x = SCREEN_DX;	srect.w = SCREEN_W;
  srect.y = SCREEN_DY;  srect.h = SCREEN_H;
  drect.x = SCREEN_DX;
  drect.y = SCREEN_DY;

  if(SDL_BlitSurface (offscreen, &srect, display, &drect)<0) 
      fprintf (stderr,"SDL: Warn: Unsuccessful blitting\n");

  /* SDL_UpdateRect(display, 0,0,0,0); */
  SDL_Flip(display);
}






/************************************************************************/
/* マウスの表示／非表示を行なう関数					*/
/************************************************************************/
void	set_mouse_visible( void )
{
  SDL_ShowCursor( SDL_ENABLE );
}

void	set_mouse_invisible( void )
{
  SDL_ShowCursor( SDL_DISABLE );
}



/************************************************************************/
/* グラフィックシステム (X11) の再初期化				*/
/*	グラフィックシステムを一旦終了し、再度、初期化を行う。		*/
/*	前もって、screen_size などを変更しておけば、新たな大きさの	*/
/*	ウインドウが生成されることになる。				*/
/*	再初期化に失敗したときは、どうしようもないので、強制終了する。	*/
/************************************************************************/
void	graphic_system_restart( int redraw_flag )
{
#if 0
  if( ! SDL_WM_ToggleFullScreen( display ) ){
    if( verbose_proc ) printf("Can't toggle full screen\n" );
  }
#endif

  int	verbose_proc_saved = verbose_proc;
  verbose_proc = 0;

  /* グラフィックシステムを一旦終了させる */
  /*graphic_system_term();*/
  SDL_QuitSubSystem( SDL_INIT_VIDEO );



  /* ウインドウ再生成時に、ボーダーのサイズが同じになるようにする */
#ifndef	USE_ZAURUS
  WIDTH  = SCREEN_DX * 2 + screen_size_tbl[screen_size].w;
  HEIGHT = SCREEN_DY * 2 + screen_size_tbl[screen_size].h;
#else
  WIDTH = display_size_tbl[screen_size].w;
  HEIGHT = display_size_tbl[screen_size].h;  
#endif


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
    int fn = -1;
    int xx = x-32,yy;

    if ( fn_button ){
      if (func_side == 0)
	yy = y - (HEIGHT-12-8);
      else
	yy = y - 8;

      if (xx >= 0 && xx <= 12*48 && yy >= 0 && yy <=12){
	fn = xx / 48;
      }
    }
    return fn;
}

void mouse_coord_trans(int x, int y, int *mx, int *my)
{
    if (!zaurus_mode) {
	*mx = x;
	*my = y;
    } else {
	*mx = (WIDTH  - x) - zaurus_mx;
	*my = (HEIGHT - y) - zaurus_my;
    }
}

