/************************************************************************/
/*									*/
/* 画面の表示								*/
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

int	frameskip_rate  = DEFAULT_FRAMESKIP;	/* 画面表示の更新間隔	*/

int	monitor_analog  = TRUE;			/* アナログモニター     */

PC88_PALETTE_T	vram_bg_palette;	/* OUT[52/54-5B]		*/
PC88_PALETTE_T	vram_palette[8];	/*		各種パレット	*/

byte	sys_ctrl;			/* OUT[30] SystemCtrl		*/
byte	grph_ctrl;			/* OUT[31] GraphCtrl		*/
byte	grph_pile;			/* OUT[53] 重ね合わせ		*/

int	do_update = TRUE;			/* 描画の必要あり	*/

int	use_half_interp = FALSE;		/* 画面サイズ半分(320x200)時に補間する */
int	now_half_interp = FALSE;	/* 現在HALF時の補間可能		*/

int	use_interlace = FALSE;		/* インターレース(1ラインおき)表示 */

int	use_auto_skip = TRUE;		/* オートフレームスキップを使用する	*/
int	do_skip_draw = FALSE;		/* スクリーンへの描画をスキップする	*/
int	already_skip_draw = FALSE;	/* スキップしたか */

/*
 *	画面差分更新の処理マクロ
 */

char	screen_update[ 0x4000*2 ];	/* 画面表示差分更新フラグ	*/
int	screen_update_force = TRUE;	/* 画面強制更新フラグ		*/
int	screen_update_palette = TRUE;	/* パレット更新フラグ		*/
int	screen_update_hide = FALSE;	/* VRAM非表示処理済フラグ	*/

#define	reset_screen_update()					\
	do{							\
	  memset( screen_update, 0, sizeof( screen_update ));	\
	  screen_update_force = FALSE;				\
	  text_attr_flipflop ^= 1;    				\
	}while(0)

#define	check_screen_update_force()					\
	do{								\
	  if( !(grph_ctrl&GRPH_CTRL_VDISP) ){	/* VRAM 非表示なら */	\
	    if( screen_update_hide==FALSE ){	/*    前回は表示   */	\
	      screen_update_force = TRUE;				\
	    }else{				/*    前回も非表示 */	\
	      memset( screen_update, 0, sizeof( screen_update )/2 );	\
	    }								\
	  }								\
	  if( screen_update_force ){					\
	    memset( text_attr_buf, 0, sizeof( text_attr_buf ) );	\
	    memset( screen_update, 1, sizeof( screen_update )/2 );	\
	    do_update = TRUE;						\
	  }								\
	  if( !(grph_ctrl & 			/* 400ライン処理   */	\
		(GRPH_CTRL_COLOR|GRPH_CTRL_200) ) ){			\
	    memcpy( &screen_update[80*200], screen_update, 80*200 );	\
	  }								\
	}while(0)



/****************************************************************/
/* テキストVRAMのアトリビュートを専用ワークに設定する		*/
/*	バッファは2個あり、交互に切替えて使用する。		*/
/*	画面書き換えの際は、この2個のバッファを比較し、変化の	*/
/*	あった部分だけを更新する。				*/
/*								*/
/*	ワークは、16bitで、上位8bitが文字コード、下位は属性。	*/
/*		色、グラフィックモード、アンダーライン、	*/
/*		アッパーライン、シークレット、リバース		*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*		|    ASCII 8bit       |Ｇ|Ｒ|Ｂ|GR|LO|UP|SC|RV|	*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*	BLINK属性は、点灯時は無視、消灯時はシークレット。	*/
/*								*/
/*	以下の場合は、空白(表示不要)なので、内容を書き換える。	*/
/*	  ・シークレット属性がオンの場合			*/
/*	  ・asciiが空白で、GR、LO、UPがオフの場合		*/
/*	  ・asciiが00で、GRがオン、LO、UPがオフの場合		*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*	     →	|    ASCII == 0       |Ｇ|Ｒ|Ｂ|０|０|０|０|RV|	*/
/*		+---------------------+--+--+--+--+--+--+--+--+	*/
/*	        色とリバース属性のみ残し、他はすべてクリア	*/
/*								*/
/****************************************************************/
int	text_attr_flipflop = 0;
Ushort	text_attr_buf[2][2048];		/* アトリビュート情報	*/
			/* ↑ 80文字x25行=2000で足りるのだが、	*/
			/* 余分に使うので、多めに確保する。	*/
				   

static	void	make_text_attr( void )
{
  int		global_attr  = (ATTR_G|ATTR_R|ATTR_B);
  int		global_blink = FALSE;
  int		i, j, tmp;
  int		column, attr, attr_rest;
  word		char_start_addr, attr_start_addr;
  word		c_addr, a_addr;
  Ushort	*text_attr = &text_attr_buf[ text_attr_flipflop ][0];


	/* CRTC も DMAC も止まっている場合 */
	/*  (文字もアトリビュートも無効)   */

  if( text_display==TEXT_DISABLE ){		/* ASCII=0、白色、装飾なし */
    for( i=0; i<CRTC_SZ_LINES; i++ ){		/* で初期化する。	   */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ =  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* 全画面反転やカーソルもなし。すぐに戻る  */
  }



	/* ノン・トランスペアレント型の場合 */
	/* (1文字置きに、VRAM、ATTR がある) */

			/* ……… ？詳細不明 				*/
			/*	CRTCの設定パターンからして、さらに行の	*/
			/*	最後に属性がある場合もありえそうだが…?	*/

  if( crtc_attr_non_separate ){

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + 1;

    for( i=0; i<crtc_sz_lines; i++ ){

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;

      for( j=0; j<CRTC_SZ_COLUMNS; j+=2 ){		/* 属性を内部コードに*/
	attr = main_ram[ a_addr ];			/* 変換し、属性ワーク*/
	a_addr += 2;					/* を全て埋める。    */
	global_attr =( global_attr & COLOR_MASK ) |
		     ((attr &  MONO_GRAPH) >> 3 ) |
		     ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
		     ((attr &  MONO_SECRET) << 1 );

					/* BLINKのOFF時はSECRET扱い    */
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
    for( ; i<CRTC_SZ_LINES; i++ ){		/* 残りの行は、SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24行設定対策)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }else{

	/* トランスペアレント型の場合 */
	/* (行の最後に、ATTRがある)   */

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + crtc_sz_columns;

    for( i=0; i<crtc_sz_lines; i++ ){			/* 行単位で属性作成 */

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;


      attr_rest = 0;						/*属性初期化 */
      for( j=0; j<=CRTC_SZ_COLUMNS; j++ ) text_attr[j] = 0;	/* [0]～[80] */


      for( j=0; j<crtc_sz_attrs; j++ ){			/* 属性を指定番目の */
	column = main_ram[ a_addr++ ];			/* 配列に格納       */
	attr   = main_ram[ a_addr++ ];

	if( j!=0 && column==0    ) column = 0x80;		/* 特殊処理?*/
	if( j==0 && column==0x80 ){column = 0;
/*				   global_attr = (ATTR_G|ATTR_R|ATTR_B);
				   global_blink= FALSE;  }*/}

	if( column==0x80  &&  !attr_rest ){			/* 8bit目は */
	  attr_rest = attr | 0x100;				/* 使用済の */
	}							/* フラグ   */
	else if( column <= CRTC_SZ_COLUMNS  &&  !text_attr[ column ] ){
	  text_attr[ column ] = attr | 0x100;
	}
      }


      if( !text_attr[0] && attr_rest ){			/* 指定桁-1まで属性が*/
	for( j=CRTC_SZ_COLUMNS; j; j-- ){		/* 有効、という場合の*/
	  if( text_attr[j] ){				/* 処理。(指定桁以降 */
	    tmp          = text_attr[j];		/* 属性が有効、という*/
	    text_attr[j] = attr_rest;			/* ふうに並べ替える) */
	    attr_rest    = tmp;
	  }
	}
	text_attr[0] = attr_rest;
      }


      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* 属性を内部コードに*/
							/* 変換し、属性ワーク*/
	if( ( attr = *text_attr ) ){			/* を全て埋める。    */
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
					/* BLINKのOFF時はSECRET扱い    */
	  if( global_blink && ((blink_counter&0x03)==0) ){
	    global_attr =  global_attr | ATTR_SECRET;
	  }
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){				/* 1行飛ばし指定時は*/
	if( ++i < crtc_sz_lines ){			/* 次の行をSECRETで */
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* 埋める。         */
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }

    for( ; i<CRTC_SZ_LINES; i++ ){		/* 残りの行は、SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24行設定対策)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }



	/* CRTC や DMAC は動いているけど、 テキストが非表示 */
	/* でVRAM白黒の場合 (アトリビュートの色だけが有効)  */

  if( text_display==TEXT_ATTR_ONLY ){

    text_attr = &text_attr_buf[ text_attr_flipflop ][0];

    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ &=  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* 全画面反転やカーソルは不要。ここでに戻る  */
  }




		/* 全体反転処理 */

  if( crtc_reverse_display && (grph_ctrl & GRPH_CTRL_COLOR)){
    text_attr = &text_attr_buf[ text_attr_flipflop ][0];
    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ ^= ATTR_REVERSE;
      }
    }
  }

		/* カーソル表示処理 */

  if( 0 <= crtc_cursor[0] && crtc_cursor[0] < crtc_sz_columns &&
      0 <= crtc_cursor[1] && crtc_cursor[1] < crtc_sz_lines   ){
    if( !crtc_cursor_blink || (blink_counter&0x01) ){
      text_attr_buf[ text_attr_flipflop ][ crtc_cursor[1]*80 + crtc_cursor[0] ]
							^= crtc_cursor_style;
    }
  }



	/* 空白フォントおよびベタフォントで、装飾が無い場合、	*/
	/* 文字コードを 0x00 に置きかえる			*/
	/* さらに、ベタフォントの場合は、REVERSE属性にする。	*/

  text_attr = &text_attr_buf[ text_attr_flipflop ][0];

  for( i=0; i<CRTC_SZ_LINES; i++ ){
    for( j=0; j<CRTC_SZ_COLUMNS; j++ ){


      if( *text_attr & ATTR_SECRET ){		/* SECRET 属性は、コード00に */
	*text_attr &= 0xff;
      }
      
      if( ( *text_attr & (ATTR_UPPER|ATTR_LOWER) )==0 ){
	int c = *text_attr >> 8;
	int space = FALSE;
	int black = FALSE;

	if( ( *text_attr & ATTR_GRAPH) == 0 ){
	  if( c==0 || c==32 || c==96 || c==127 || c==160 || c>=248 ){/*空白 */
	    if( ( *text_attr & ATTR_REVERSE )==0 ) space = TRUE;
	    else                                   black = TRUE;
	  }else if( c==135 && crtc_font_height==8 ){		     /*ベタ */
	    if( ( *text_attr & ATTR_REVERSE )==0 ) black = TRUE;
	    else                                   space = TRUE;
	  }
	}else{
	  if( c==0 ){						     /*空白 */
	    if( ( *text_attr & ATTR_REVERSE )==0 ) space = TRUE;
	    else                                   black = TRUE;
	  }else if( c==255 && crtc_font_height==8 ){		     /*ベタ */
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
	/* SECRETでも、アンダー・アッパーライン、REVERSEは有効 */
}



/****************************************************************/
/* 指定座標の文字のフォントデータを作成する。			*/
/*	まず、set_font_ptr(x,y) を呼んで、内部ワークを初期化。	*/
/*	続けて、get_font_style を呼ぶたびに、フォントデータが	*/
/*	次々と手に入る。20行の場合は 10回、25行の場合は 8回呼べ	*/
/*	ばよい。						*/
/*		アトリビュート情報が必要なので、		*/
/*		予め make_text_attr_table( ) を呼んでおくこと	*/
/*								*/
/*	フォントデータを font_rom から、内部ワーク font_styleに	*/
/*	コピーしているのだが、この際に、bit32 で一括してコピー	*/
/*	している(ちょっとは高速？)。多分問題ないと思うが………	*/
/****************************************************************/

bit8	*font_ptr;
bit8	font_style[16];			/* 使用するのは [0]～[9]まで */
int	font_color;

void	set_font_style( int attr )
{
  int	chara;
  bit32	*src;
  bit32	*dst = (bit32 *)&font_style[0];;
					/* get_font_style()のための準備 */
  font_ptr   = &font_style[0];
  font_color = ((attr & COLOR_MASK) >> 5) | 8;


  if( ( attr & ~(COLOR_MASK|ATTR_REVERSE) )==0 ){

    if( ( attr & ATTR_REVERSE ) == 0 ){		/* 空白フォント時 */

      *dst++ = 0;
      *dst++ = 0;
      *dst   = 0;

    }else{					/* ベタフォント時 */

      *dst++ = 0xffffffff;
      *dst++ = 0xffffffff;
      *dst   = 0xffffffff;
    }

  }else{					/* 通常フォント時 */

    chara = attr >> 8;

    if( attr & ATTR_GRAPH )
      src = (bit32 *)&font_rom[ (chara | 0x100)*8 ];
    else
      src = (bit32 *)&font_rom[ (chara        )*8 ];

					/* フォントをまず内部ワークにコピー */
    *dst++ = *src++;
    *dst++ = *src;
    *dst   = 0;

					/* 属性により内部ワークフォントを加工*/
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
/* PC8801 G-VRAM/T-VRAM の内容を Xの表示用バッファに転送*/
/********************************************************/

#define	COLOR			/* カラー 640x200 ---------------------------*/

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


#define	MONO			/* 白黒   640x200 ---------------------------*/

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


#define	HIRESO			/* 白黒   640x400 ---------------------------*/

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


#define	CLEAR			/* 消去   640x200/640x400-------------------*/

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
/* 表示用バッファを黒でクリアする (ボーダーも含めて)	*/
/*							*/
/*	black の色でクリアする。			*/
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
  if( grph_ctrl & GRPH_CTRL_VDISP ){		/* VRAM 表示する */

    if( screen_size == SCREEN_SIZE_FULL ){		/* Full Size */

      if( grph_ctrl & GRPH_CTRL_COLOR ){			/* カラー */
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
	if( grph_ctrl & GRPH_CTRL_200 ){			/* 白黒 */
	  if( use_interlace ){
	    if     ( DEPTH<= 8 ) vram2pixel_mono_full_bit8_interlace();
	    else if( DEPTH<=16 ) vram2pixel_mono_full_bit16_interlace();
	    else                 vram2pixel_mono_full_bit32_interlace();
	  }else{
	    if     ( DEPTH<= 8 ) vram2pixel_mono_full_bit8();
	    else if( DEPTH<=16 ) vram2pixel_mono_full_bit16();
	    else                 vram2pixel_mono_full_bit32();
	  }
	}else{							/* 400ライン */
	  if     ( DEPTH<= 8 ) vram2pixel_400_full_bit8();
	  else if( DEPTH<=16 ) vram2pixel_400_full_bit16();
	  else                 vram2pixel_400_full_bit32();
	}
      }

    }else if( screen_size == SCREEN_SIZE_HALF ){	/* Half Size */

      if( grph_ctrl & GRPH_CTRL_COLOR ){			/* カラー */
	if( now_half_interp ){		/* フィルタ有り */
	  if     ( DEPTH<= 8 ) vram2pixel_color_half_bit8_interp();
	  else if( DEPTH<=16 ) vram2pixel_color_half_bit16_interp();
	  else                 vram2pixel_color_half_bit32_interp();
	}else{				/* フィルタ無し */
	  if     ( DEPTH<= 8 ) vram2pixel_color_half_bit8();
	  else if( DEPTH<=16 ) vram2pixel_color_half_bit16();
	  else                 vram2pixel_color_half_bit32();
	}
      }else{
	if( grph_ctrl & GRPH_CTRL_200 ){			/* 白黒 */
	  if     ( DEPTH<= 8 ) vram2pixel_mono_half_bit8();
	  else if( DEPTH<=16 ) vram2pixel_mono_half_bit16();
	  else                 vram2pixel_mono_half_bit32();
	}else{							/* 400ライン */
	  if     ( DEPTH<= 8 ) vram2pixel_400_half_bit8();
	  else if( DEPTH<=16 ) vram2pixel_400_half_bit16();
	  else                 vram2pixel_400_half_bit32();
	}
      }

    }else{						/* Double Size */
    
      if( grph_ctrl & GRPH_CTRL_COLOR ){			/* カラー */
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
	if( grph_ctrl & GRPH_CTRL_200 ){			/* 白黒 */
	  if( use_interlace ){
	    if     ( DEPTH<= 8 ) vram2pixel_mono_double_bit8_interlace();
	    else if( DEPTH<=16 ) vram2pixel_mono_double_bit16_interlace();
	    else                 vram2pixel_mono_double_bit32_interlace();
	  }else{
	    if     ( DEPTH<= 8 ) vram2pixel_mono_double_bit8();
	    else if( DEPTH<=16 ) vram2pixel_mono_double_bit16();
	    else                 vram2pixel_mono_double_bit32();
	  }
	}else{							/* 400ライン */
	  if     ( DEPTH<= 8 ) vram2pixel_400_double_bit8();
	  else if( DEPTH<=16 ) vram2pixel_400_double_bit16();
	  else                 vram2pixel_400_double_bit32();
	}
      }

    }

  }else{					/* VRAM 表示しない */

    screen_update_hide = TRUE;				/* 非表示済フラグON */

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
      if( now_half_interp ){		/* フィルタ有り */
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
/* PC8801 パレット の内容を システムパレットに変換 	*/
/********************************************************/
static	void	set_palette( void )
{
  int     i;
  SYSTEM_PALETTE_T	syspal[16];


	/* VRAM の カラーパレット設定   syspal[0]～[7] */

  if( grph_ctrl & GRPH_CTRL_COLOR ){		/* VRAM カラー */

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

  }else{					/* VRAM 白黒 */

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


	/* TEXT の カラーパレット設定   syspal[8]～[15] */

  if( !crtc_attr_color ){			/* TEXT 白黒 */

    syspal[8].red   = 0;
    syspal[8].green = 0;
    syspal[8].blue  = 0;
    for( i=9; i<16; i++ ){
      syspal[i].red   = 0xff;
      syspal[i].green = 0xff;
      syspal[i].blue  = 0xff;
    }

  }else{					/* TEXT カラー */

    if( grph_ctrl & GRPH_CTRL_COLOR ){			/* VRAM カラー */

      for( i=8; i<16; i++ ){
	syspal[i].red   = (i&0x02) ? 0xff : 0;
	syspal[i].green = (i&0x04) ? 0xff : 0;
	syspal[i].blue  = (i&0x01) ? 0xff : 0;
      }

    }else{						/* VRAM 白黒   */

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
/* screen_buf の初期化							*/
/*	screen_buf を黒色 (black)で塗りつぶす				*/
/*	    ボーダーを含めて塗りつぶすので、グラフィックの初期化時など	*/
/*	    で呼ぶ							*/
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
/* 点滅 (カーソルおよび、文字属性の点滅) 実現のためのワーク設定		*/
/*	frameskip_rate, blink_cycle が変更されるたびに呼ぶ		*/
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
/* イメージ転送 (表示)							*/
/*	1/60 sec 毎に呼ばれる (表示は、指定したフレーム毎に行なう)	*/
/************************************************************************/
int	frame_counter = 0;
void	draw_screen( void )
{

  if( (frame_counter%frameskip_rate)==0 ){

	/* ウエイト無効の場合、オートスキップは無視 */

    if( no_wait || !use_auto_skip || !do_skip_draw ){

      if( screen_update_palette ){
	screen_update_palette = FALSE; 
	set_palette();			/* パレットをシステムに転送	*/
      }

      check_screen_update_force();	/* VRAM更新情報のチェック	*/
      clear_indicator();		/* 不要なインジケータの消去	*/
      make_text_attr();			/* TVRAM の 属性一覧作成	*/
      vram2screen();			/* VRAM/TEXT → screen_buf 転送	*/
      reset_screen_update();		/* VRAM更新情報リセット		*/

      if( indicator_flag & (INDICATE_MES|INDICATE_FDD) ){
	disp_indicator();		/* インジケータ表示		*/
      }

      if( do_update ){
	put_image();			/* screen_buf 表示		*/
	do_update = FALSE;
      }
    }else{
	already_skip_draw = TRUE;	/* オートスキップ */
    }

    if( --blink_ctrl_counter == 0 ){	/* カーソル点滅ワーク更新	*/
      blink_ctrl_counter = blink_ctrl_cycle;
      blink_counter ++;
    }
  }

  ++ frame_counter;
}



/************************************************************************/
/* イメージ転送 (再描画)						*/
/************************************************************************/
void	redraw_screen( int do_disp_indicator )
{

  set_screen_update_force();		/* VRAM更新情報を全変更		*/

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
/* サスペンド／レジューム					*/
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

  /*{ TYPE_INT,	&blink_ctrl_cycle,	},	初期値でも問題ないだろう */
  /*{ TYPE_INT,	&blink_ctrl_counter,	},	初期値でも問題ないだろう */
  /*{ TYPE_INT,	&frame_counter,		},	初期値でも問題ないだろう */
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
















/* デバッグ用の関数 */
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
