/************************************************************************/
/*									*/
/* スクリーン スナップショット						*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "screen.h"
#include "graph.h"

#include "crtcdmac.h"
#include "memory.h"
#include "file-op.h"
#include "snapshot.h"

#include "exmem.h"

/* デフォルトの、スクリーン スナップショットのファイルベース名 */

#define	SNAPSHOT_FILENAME	"save"


char	*file_snapshot   = NULL;	/* スナップショットファイル名	*/
int	snapshot_format  = 0;		/* スナップショットフォーマット	*/
char	*snapshot_cmd    = NULL;	/* スナップショット後コマンド	*/
char	snapshot_cmd_do  = FALSE;	/* コマンド実行の有無		*/



void	screen_snapshot_init( void )
{
  int dummy;

  snapshot_format = 0;					/* ppm(raw) 形式 */

  file_snapshot = getenv( "QUASI88_SSS_BASE" );		/* ファイル名   */

  if( file_snapshot == NULL ){

    file_snapshot = osd_get_home_filename( SNAPSHOT_FILENAME, &dummy );

    if( file_snapshot == NULL ){
      file_snapshot = "";
    }

    /* どこかで ex_free(file_snapshot)したほうがいいけど…まあいいか */
  }


  snapshot_cmd = getenv("QUASI88_SSS_CMD");		/* 実行コマンド */

  if( snapshot_cmd == NULL ){
    snapshot_cmd = "";
  }

  snapshot_cmd_do = FALSE;	/* 初期値は、コマンド実行『しない』にする */
}





static char		screen_snapshot[ 640*400 ];
static int		line400_snapshot;
static SYSTEM_PALETTE_T	palette_snapshot[16];



#define	COLOR
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( color, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( color, 40 )
#undef	WIDTH40
#undef	COLOR

#define	MONO
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( mono, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( mono, 40 )
#undef	WIDTH40
#undef	MONO

#define	HIRESO
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( hireso, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( hireso, 40 )
#undef	WIDTH40
#undef	HIRESO

#define	CLEAR
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( clear, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( clear, 40 )
#undef	WIDTH40
#undef	CLEAR



/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

static	void	make_snapshot( void )
{
  int i;

  line400_snapshot = FALSE;

	/* VRAM の内容を screen_snapshot[] に転送 */

  if( grph_ctrl & GRPH_CTRL_VDISP ){		/* VRAM 表示する */

    if( grph_ctrl & GRPH_CTRL_COLOR ){		    /* カラー */

      if( sys_ctrl & SYS_CTRL_80 ){
	vram2snapshot_color_width80();
      }else{
	vram2snapshot_color_width40();
      }

    }else{

      if( grph_ctrl & GRPH_CTRL_200 ){		    /* 白黒 */

	if( sys_ctrl & SYS_CTRL_80 ){
	  vram2snapshot_mono_width80();
	}else{
	  vram2snapshot_mono_width40();
	}

      }else{					    /* 400ライン */
	if( sys_ctrl & SYS_CTRL_80 ){
	  vram2snapshot_hireso_width80();
	}else{
	  vram2snapshot_hireso_width40();
	}
	line400_snapshot = TRUE;
      }
    }

  }else{					/* VRAM 表示しない */

    screen_update_hide = TRUE;				/* 非表示済フラグON */

    if( sys_ctrl & SYS_CTRL_80 ){
      vram2snapshot_clear_width80();
    }else{
      vram2snapshot_clear_width40();
    }

  }




	/* VRAM の カラーパレット   palette_snapshot[0]～[7] */

  if( grph_ctrl & GRPH_CTRL_COLOR ){		/* VRAM カラー */

    if( monitor_analog ){
#if	0
      for( i=0; i<8; i++ ){
	palette_snapshot[i].red   = vram_palette[i].red;
	palette_snapshot[i].green = vram_palette[i].green;
	palette_snapshot[i].blue  = vram_palette[i].blue;
      }
#else
      for( i=0; i<8; i++ ){
	palette_snapshot[i].red   = vram_palette[i].red   * 73 >> 6;
	palette_snapshot[i].green = vram_palette[i].green * 73 >> 6;
	palette_snapshot[i].blue  = vram_palette[i].blue  * 73 >> 6;
      }
#endif
    }else{
      for( i=0; i<8; i++ ){
	palette_snapshot[i].red   = vram_palette[i].red   ? 0xff : 0;
	palette_snapshot[i].green = vram_palette[i].green ? 0xff : 0;
	palette_snapshot[i].blue  = vram_palette[i].blue  ? 0xff : 0;
      }
    }

  }else{					/* VRAM 白黒 */

    if( monitor_analog ){
#if	0
      palette_snapshot[0].red   = vram_bg_palette.red;
      palette_snapshot[0].green = vram_bg_palette.green;
      palette_snapshot[0].blue  = vram_bg_palette.blue;
#else
      palette_snapshot[0].red   = vram_bg_palette.red   * 73 >> 6;
      palette_snapshot[0].green = vram_bg_palette.green * 73 >> 6;
      palette_snapshot[0].blue  = vram_bg_palette.blue  * 73 >> 6;
#endif
    }else{
      palette_snapshot[0].red   = vram_bg_palette.red   ? 0xff : 0;
      palette_snapshot[0].green = vram_bg_palette.green ? 0xff : 0;
      palette_snapshot[0].blue  = vram_bg_palette.blue  ? 0xff : 0;
    }
    for( i=1; i<8; i++ ){
      palette_snapshot[i].red   = 0;
      palette_snapshot[i].green = 0;
      palette_snapshot[i].blue  = 0;
    }

  }


	/* TEXT の カラーパレット   palette_snapshot[8]～[15] */

  if( !crtc_attr_color ){			/* TEXT 白黒 */

    palette_snapshot[8].red   = 0;
    palette_snapshot[8].green = 0;
    palette_snapshot[8].blue  = 0;
    for( i=9; i<16; i++ ){
      palette_snapshot[i].red   = 0xff;
      palette_snapshot[i].green = 0xff;
      palette_snapshot[i].blue  = 0xff;
    }

  }else{					/* TEXT カラー */

    if( grph_ctrl & GRPH_CTRL_COLOR ){			/* VRAM カラー */

      for( i=8; i<16; i++ ){
	palette_snapshot[i].red   = (i&0x02) ? 0xff : 0;
	palette_snapshot[i].green = (i&0x04) ? 0xff : 0;
	palette_snapshot[i].blue  = (i&0x01) ? 0xff : 0;
      }

    }else{						/* VRAM 白黒   */

      if( monitor_analog ){
#if	0
	for( i=8; i<16; i++ ){
	  palette_snapshot[i].red   = vram_palette[i&0x7].red;
	  palette_snapshot[i].green = vram_palette[i&0x7].green;
	  palette_snapshot[i].blue  = vram_palette[i&0x7].blue;
	}
#else
	for( i=8; i<16; i++ ){
	  palette_snapshot[i].red   = vram_palette[i&0x7].red   * 73 >> 6;
	  palette_snapshot[i].green = vram_palette[i&0x7].green * 73 >> 6;
	  palette_snapshot[i].blue  = vram_palette[i&0x7].blue  * 73 >> 6;
	}
#endif
      }else{
	for( i=8; i<16; i++ ){
	  palette_snapshot[i].red   = vram_palette[i&0x7].red   ? 0xff : 0;
	  palette_snapshot[i].green = vram_palette[i&0x7].green ? 0xff : 0;
	  palette_snapshot[i].blue  = vram_palette[i&0x7].blue  ? 0xff : 0;
	}
      }

    }
  }


  /* line400==FALSE の時、[0]～[640*200-1] が画像データ 	*/
  /* line400==TRUE  の時、[0]～[640*400-1] が画像データ 	*/
  /* データは、0～15 の値で、RGB値は palette_snapshot[0]～[15]	*/



}


/*----------------------------------------------------------------------*/
/* キャプチャした内容を、xpm 形式でファイルに出力			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_xpm( FILE *fp )
{
  int i, j;
  char *p = screen_snapshot;
  int black_pal = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;

  if( fp==NULL ) return 0;

  fprintf( fp, "/* XPM */\n" ); 
  fprintf( fp, "static char * quasi88_xpm[] = {\n" );
  fprintf( fp, "\"640 400 16 1\",\n" );
  
  for( i=0; i<16; i++ )
    fprintf( fp, "\"%1X      c #%04X%04X%04X\",\n",
	     i,
	     (unsigned short)palette_snapshot[i].red   << 8,
	     (unsigned short)palette_snapshot[i].green << 8,
	     (unsigned short)palette_snapshot[i].blue  << 8 );

  for( i=0; i<200; i++ ){

    fprintf( fp, "\"");
    for( j=0; j<640; j++ ){
      fprintf( fp, "%1X", *p++ );
    }
    fprintf( fp, "\",\n");


#if	0
    if( line400_snapshot==FALSE && use_interlace==FALSE ) p -= 640;

    if( line400_snapshot || use_interlace==FALSE ){
      fprintf( fp, "\"");
      for( j=0; j<640; j++ ){
	fprintf( fp, "%1X", *p++ );
      }
      fprintf( fp, "\",\n");
    }else{
      fprintf( fp, "\"");
      for( j=0; j<640; j++ ){
	fprintf( fp, "%1X", black_pal );
      }
      fprintf( fp, "\",\n");
    }
#else
    fprintf( fp, "\"");
    for( j=0; j<640; j++ ){
      fprintf( fp, "%1X", *p++ );
    }
    fprintf( fp, "\",\n");
#endif

  }

  fprintf( fp, "};\n" );

  return 1;
}



/*----------------------------------------------------------------------*/
/* キャプチャした内容を、ppm 形式(raw)でファイルに出力			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm_raw( FILE *fp )
{
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  fprintf( fp, "P6\n" );
  fprintf( fp, "# QUASI88\n" );
  fprintf( fp, "640 400\n" );
  fprintf( fp, "255\n" );
  
  for( i=0; i<200; i++ ){

    for( j=0; j<640; j++ ){
      fprintf( fp, "%c%c%c",
	       palette_snapshot[ *p ].red,
	       palette_snapshot[ *p ].green,
	       palette_snapshot[ *p ].blue );
      p++;
    }

#if	0
    if( line400_snapshot==FALSE && use_interlace==FALSE ) p -= 640;

    if( line400_snapshot || use_interlace==FALSE ){
      for( j=0; j<640; j++ ){
	fprintf( fp, "%c%c%c",
		 palette_snapshot[ *p ].red,
		 palette_snapshot[ *p ].green,
		 palette_snapshot[ *p ].blue );
	p++;
      }
    }else{
      for( j=0; j<640; j++ ){
	fprintf( fp, "%c%c%c", 0, 0, 0 );
      }
    }
#else
    for( j=0; j<640; j++ ){
      fprintf( fp, "%c%c%c",
       palette_snapshot[ *p ].red,
       palette_snapshot[ *p ].green,
       palette_snapshot[ *p ].blue );
      p++;
    }
#endif

  }

  return 1;
}

/*----------------------------------------------------------------------*/
/* キャプチャした内容を、ppm 形式(ascii)でファイルに出力		*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm_ascii( FILE *fp )
{
  int i, j, k;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  fprintf( fp, "P3\n" );
  fprintf( fp, "# QUASI88\n" );
  fprintf( fp, "640 400\n" );
  fprintf( fp, "255\n" );
  
  for( i=0; i<200; i++ ){

    for( j=0; j<640; j+=5 ){
      for( k=0; k<5; k++ ){
	fprintf( fp, "%3d %3d %3d ", 
		 palette_snapshot[ *p ].red,
		 palette_snapshot[ *p ].green,
		 palette_snapshot[ *p ].blue );
	p++;
      }
      fprintf( fp, "\n");
    }

#if	0
    if( line400_snapshot==FALSE && use_interlace==FALSE ) p -= 640;

    if( line400_snapshot || use_interlace==FALSE ){
      for( j=0; j<640; j+=5 ){
	for( k=0; k<5; k++ ){
	  fprintf( fp, "%3d %3d %3d ", 
		   palette_snapshot[ *p ].red,
		   palette_snapshot[ *p ].green,
		   palette_snapshot[ *p ].blue );
	  p++;
	}
	fprintf( fp, "\n");
      }
    }else{
      for( j=0; j<640; j+=5 ){
	for( k=0; k<5; k++ ){
	  fprintf( fp, "%3d %3d %3d ", 0, 0, 0 );
	}
	fprintf( fp, "\n");
      }
    }
#else
    for( j=0; j<640; j+=5 ){
      for( k=0; k<5; k++ ){
	fprintf( fp, "%3d %3d %3d ",
         palette_snapshot[ *p ].red,
         palette_snapshot[ *p ].green,
         palette_snapshot[ *p ].blue );
	p++;
      }
      fprintf( fp, "\n");
    }
#endif

  }

  return 1;
}





/************************************************************************/
/* 画面のスナップショットをセーブする					*/
/*	現時点のVRAMをもとにスナップショットを作成する。		*/
/*	現在表示されている画面をセーブするわけではない。		*/
/*									*/
/*	環境変数 ${QUASI88_SSS_CMD} が定義されている場合、セーブ後に	*/
/*	この内容をコマンドとして実行する。この際、%a がファイル名に、	*/
/*	%b がファイル名からサフィックスを削除したものに、置き換わる。	*/
/*									*/
/*	例) setenv QUASI88_SSS_CMD 'ppmtopng %a > %b.png'		*/
/*									*/
/************************************************************************/


static	int snapshot_no = 0;

int	save_screen_snapshot( void )
{
  int  result;
  FILE *fp;

  char   *filename, *cmd, *s, *d;
  char   number[8];
  static const char *suffix[] = { ".ppm", ".ppm", ".xpm", };
  static const char *checksuffix[] = { ".ppm", 
				       ".xpm", 
				       ".png", 
				       ".bmp", 
				       ".rgb", 
				       ".gif", 
				       ".xwd", 
				       ".pict", 
				       ".tiff", 
				       ".tif", 
				       ".jpeg", 
				       ".jpg", 
  };
  int    i, len, a_len, b_len;


  if( file_snapshot   ==NULL ) return 0;
  if( file_snapshot[0]=='\0' ) return 0;

  if( snapshot_format >= countof(suffix) ) return 0;


  filename = (char*)ex_malloc( strlen( file_snapshot ) + sizeof( "NNNN.ppm" ) );
  if( filename==NULL ) return 0;


  for( result=10000; result; result-- ){    /* 存在しないファイル名を検索 */

    sprintf( number, "%04d", snapshot_no ++ );
    if( snapshot_no > 9999 ) snapshot_no = 0;
    strcpy( filename, file_snapshot );
    strcat( filename, number );
    len = strlen( filename );

    for( i=countof(checksuffix); i; i-- ){
      filename[len] = '\0';
      strcat( filename, checksuffix[ i-1 ] );
      if( osd_file_stat( filename )!=FILE_STAT_ANOTHER ) break;
    }
    if( i==0 ){
      filename[len] = '\0';
      strcat( filename, suffix[ snapshot_format ] );
      break;
    }
  }

  /* 存在しないファイル名が見つかって、ファイルを開くことに成功したら */

  if( result && (fp = ex_fopen( filename, "wb" ) ) ){

    make_snapshot();

    switch( snapshot_format ){
    case 0:	result = save_snapshot_ppm_raw( fp );	break;
    case 1:	result = save_snapshot_ppm_ascii( fp );	break;
    case 2:	result = save_snapshot_xpm( fp );	break;
    }

    ex_fclose( fp );


    /* 無事書き込みに成功し、実行コマンドがセットされていれば */

#ifdef	USE_SSS_CMD
    if( result &&
	snapshot_cmd && snapshot_cmd_do ){

      a_len = strlen( filename );
      b_len = a_len - 4;		/* サフィックス ".???" の4文字分減算 */

      len = 0;
      s = snapshot_cmd;
      while( *s ){
	if( *s == '%' ){
	  switch( *(s+1) ){
	  case '%': len ++;		s++;	break; 
	  case 'a': len += a_len;	s++;	break; 
	  case 'b': len += b_len;	s++;	break; 
	  default:  len ++;			break; 
	  }
	}else       len ++;

	s++;
      }

      cmd = (char *)ex_malloc( len + 1 );
      if( cmd ){

	s = snapshot_cmd;
	d = cmd;
	while( *s ){
	  if( *s == '%' ){
	    switch( *(s+1) ){
	    case '%': *d++ = *s;				s++;	break; 
	    case 'a': memcpy( d, filename, a_len );  d+=a_len;	s++;	break; 
	    case 'b': memcpy( d, filename, b_len );  d+=b_len;	s++;	break;
	    default:  *d++ = *s;
	    }
	  }else       *d++ = *s;
	  s++;
	}
	*d = '\0';

	fprintf( stderr, "[SNAPSHOT command]%% %s\n",cmd);
	system( cmd );

	ex_free(cmd);
      }
    }
#endif	/* USE_SSS_CMD */

  }else{
    result = 0;
  }

  ex_free( filename );
  return result;
}
