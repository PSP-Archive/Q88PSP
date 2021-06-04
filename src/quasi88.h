#ifndef QUASI88_H_INCLUDED
#define QUASI88_H_INCLUDED


/*----------------------------------------------------------------------*/
/* SDLに関する定義							*/
/*----------------------------------------------------------------------*/

#ifdef HAVE_SDL
#include <SDL_main.h>
#include <SDL_byteorder.h>


/* エンディアンネスをチェック */
#if	( SDL_BYTEORDER == SDL_LIL_ENDIAN )
#define	LSB_FIRST
#endif

/* select()を使うわけではないが、ms単位の sleep が可能なので定義しとこう */
#define	HAVE_SELECT

/* X11-DGA を使うわけではないが、フルスクリーン切替が可能なので定義しとこう */
#define	USE_DGA

/* SDL版 QUASI88 のための識別用 */

#define	USE_SDL

#endif


/* タイトルとバージョン */
#ifndef	Q_TITLE
#define	Q_TITLE		"QUASI88(SDL)"
#endif
#ifndef	Q_VERSION
#define	Q_VERSION	"0.5.5s1"
#endif

/* PI(π) と M_PI(π) を定義 */
#ifndef	UNIX88
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif
#endif

/* ジョイスティックあり */
#define	USE_JOY
#define	USE_JOY_SDL

/*
  MAME/XMAME のサウンドを組み込む場合、
	USE_SOUND
  を定義しておく。

  FMGEN を組み込む場合は、
	USE_FMGEN
  も定義しておく。

  上記は、コンパイル時に以下のようにして定義する。
  gcc  の場合、コンパイラにオプション -DUSE_SOUND   などと指定する。
  VC++ の場合、コンパイラにオプション /D"USE_SOUND" などと指定する。
  MWP  の場合、コンパイラにオプション -d USE_SOUND  などと指定する。
*/




/*
 *	VC++ depend
 */

#ifdef _MSC_VER

/* VC のインラインキーワード */
#define	INLINE		__inline

#endif

#if	(_MSC_VER) || (__MINGW32__)

/* Windows版 QUASI88 のための識別用 */
#define	WIN88

/* パス・ファイル名の最大長さ ? */
#define	OSD_MAX_FILENAME	(240)

#endif

#define EXPRINTF(format,...) \
{ printf(format,## __VA_ARGS__); } 

/*
 *
 */

#ifndef	BIT32
#define	BIT32		int
#endif


/*----------------------------------------------------------------------*/
/* 共通定義								*/
/*----------------------------------------------------------------------*/

typedef	unsigned char	Uchar;
typedef	unsigned short	Ushort;
typedef	unsigned int	Uint;
typedef	unsigned long	Ulong;

typedef unsigned char  byte;
typedef unsigned short word;
typedef signed   char  offset;


typedef unsigned char  bit8;
typedef unsigned short bit16;
typedef unsigned BIT32 bit32;


#define	countof(arr)	(int)(sizeof(arr)/sizeof((arr)[0]))


#ifdef LSB_FIRST			/* リトルエンディアン */

typedef union
{
  struct { byte l,h; }	B;
  word			W;
} pair;

#else					/* ビッグエンデイアン */

typedef union
{
  struct { byte h,l; }	B;
  word			W;
} pair;

#endif



#ifndef TRUE
#define	TRUE	(1)
#endif
#ifndef FALSE
#define	FALSE	(0)
#endif



#ifndef	INLINE
#define	INLINE	static
#endif



/*----------------------------------------------------------------------*/
/* 変数 (verbose_*)、関数						*/
/*----------------------------------------------------------------------*/
extern	int	verbose_level;		/* 冗長レベル			*/
extern	int	verbose_proc;		/* 処理の進行状況の表示		*/
extern	int	verbose_z80;		/* Z80処理エラーを表示		*/
extern	int	verbose_io;		/* 未実装 I/Oアクセスを報告	*/
extern	int	verbose_pio;		/* PIO の不正使用を表示		*/
extern	int	verbose_fdc;		/* FDイメージ異常を報告		*/
extern	int	verbose_wait;		/* ウエイト待ち時の異常を報告	*/
extern	int	verbose_suspend;	/* サスペンド時の異常を報告	*/
extern	int	verbose_snd;		/* サウンドのメッセージ		*/


void	main_exit( int val );
void	main_reset( void );



/*----------------------------------------------------------------------*/
/*	デバッグ用に画面やファイルにログを出力 (処理速度が低下します)	*/
/*----------------------------------------------------------------------*/

/*#define PIO_DISP*/		/* PIO 関係のログを画面に表示     */
/*#define PIO_FILE*/		/*		   ファイルに出力 */

/*#define FDC_DISP*/		/* FDC 関係のログを画面に表示     */
/*#define FDC_FILE*/		/*		   ファイルに出力 */

/*#define MAIN_DISP*/		/* メイン Z80 関係のログを画面に表示 */
/*#define MAIN_FILE*/		/*		   ファイルに出力    */

/*#define SUB_DISP*/		/* サブ Z80 関係のログを画面に表示 */
/*#define SUB_FILE*/		/*		   ファイルに出力  */


#ifndef	USE_MONITOR
#undef PIO_DISP
#undef PIO_FILE
#undef FDC_DISP
#undef FDC_FILE
#undef MAIN_DISP
#undef MAIN_FILE
#undef SUB_DISP
#undef SUB_FILE
#endif


/* ログをファイルに取る場合は、ファイルを開く */
#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
#include <stdio.h>
extern	FILE	*LOG;
#endif


/* ログ出力のマクロ							*/
/*	・可変長引数の関数を呼び出すマクロである。			*/
/*	・マクロそのものを無効にするには、一般には			*/
/*		#define  logxxx   (void)				*/
/*		#define  logxxx   if(1){}else printf			*/
/*	などを使う。前者ではワーニングが出る時があるので後者を採用した。*/


#if	defined( PIO_DISP ) || defined( PIO_FILE )
void	logpio( const char *format, ... );
#else
#define	logpio	if(1){}else printf
#endif

#if	defined( FDC_DISP ) || defined( FDC_FILE )
void	logfdc( const char *format, ... );
#else
#define	logfdc	if(1){}else printf
#endif

#if	defined( MAIN_DISP ) || defined( MAIN_FILE ) || defined( SUB_DISP ) || defined( SUB_FILE )
void	logz80( const char *format, ... );
#else
#define	logz80	if(1){}else printf
#endif



#endif		/* QUASI88_H_INCLUDED */
