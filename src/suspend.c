/************************************************************************/
/*									*/
/* サスペンド、レジューム処理						*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "suspend.h"
#include "initval.h"
#include "file-op.h"

#include "exmem.h"

int	resume_flag  = FALSE;			/* 起動時のレジューム	*/
int	resume_force = FALSE;			/* 強制レジューム	*/
char	*file_resume = NULL;			/* ステートファイル名	*/



/*----------------------------------------------------------------------*/
/* ステートファイルにデータを記録する関数				*/
/* ステートファイルに記録されたデータを取り出す関数			*/
/*		基本的に、データはリトルエンディアンで記録		*/
/*		int 型、 short 型、char 型、pair 型、256バイトブロック、*/
/*		文字列(1023文字まで)、double型 (1000000倍してintに変換)	*/
/*----------------------------------------------------------------------*/
static	int	suspend_int( FILE *fp, int *val )
{
  Uchar c[4];
  c[0] = ( *val       ) & 0xff;
  c[1] = ( *val >>  8 ) & 0xff;
  c[2] = ( *val >> 16 ) & 0xff;
  c[3] = ( *val >> 24 ) & 0xff;
  if(ex_fwrite( c, sizeof(Uchar), 4, fp )==4 ) return TRUE;
  else                                       return FALSE;
}
static	int	resume_int( FILE *fp, int *val )
{
  Uchar c[4];
  if(ex_fread( c, sizeof(Uchar), 4, fp )!=4 ) return FALSE;
  *val = ((Uint)c[3]<<24) | ((Uint)c[2]<<16) | ((Uint)c[1]<<8) | (Uint)c[0];
  return TRUE;
}
static	int	suspend_short( FILE *fp, short *val )
{
  Uchar c[2];
  c[0] = ( *val       ) & 0xff;
  c[1] = ( *val >>  8 ) & 0xff;
  if(ex_fwrite( c, sizeof(Uchar), 2, fp )==2 ) return TRUE;
  else                                       return FALSE;
}
static	int	resume_short( FILE *fp, short *val )
{
  Uchar c[2];
  if(ex_fread( c, sizeof(Uchar), 2, fp )!=2 ) return FALSE;
  *val = ((Ushort)c[1]<<8) | (Ushort)c[0];
  return TRUE;
}
static	int	suspend_char( FILE *fp, char *val )
{
  if(ex_fwrite( val, sizeof(Uchar), 1, fp )==1 ) return TRUE;
  else                                        return FALSE;
}
static	int	resume_char( FILE *fp, char *val )
{
  if(ex_fread( val, sizeof(Uchar), 1, fp )!=1 ) return FALSE;
  return TRUE;
}


static	int	suspend_pair( FILE *fp, pair *val )
{
  Uchar c[2];
  c[0] = ( (*val).W      ) & 0xff;
  c[1] = ( (*val).W >> 8 ) & 0xff;
  if(ex_fwrite( c, sizeof(Uchar), 2, fp )==2 ) return TRUE;
  else                                       return FALSE;
}
static	int	resume_pair( FILE *fp, pair *val )
{
  Uchar c[2];
  if(ex_fread( c, sizeof(Uchar), 2, fp )!=2 ) return FALSE;
  (*val).W = ((Ushort)c[1]<<8) | (Ushort)c[0];
  return TRUE;
}

static	int	suspend_256( FILE *fp, char *array )
{
  if(ex_fwrite( array, sizeof(Uchar), 256, fp )==256 ) return TRUE;
  else                                               return FALSE;
}
static	int	resume_256( FILE *fp, char *array )
{
  if(ex_fread( array, sizeof(Uchar), 256, fp )!=256 ) return FALSE;
  return TRUE;
}


static	int	suspend_str( FILE *fp, char *str )
{
  char wk[1024];

  if( strlen(str) >= 1024-1 ) return FALSE;

  memset( wk, 0, 1024 );
  strcpy( wk, str );

  if(ex_fwrite( wk, sizeof(Uchar), 1024, fp )==1024 ) return TRUE;
  else                                              return FALSE;
}
static	int	resume_str( FILE *fp, char *str )
{
  if(ex_fread( str, sizeof(Uchar), 1024, fp )!=1024 ) return FALSE;
  return TRUE;
}

static	int	suspend_double( FILE *fp, double *val )
{
  Uchar c[4];
  int	wk;

  wk = *val * 1000000.0;
  c[0] = ( wk       ) & 0xff;
  c[1] = ( wk >>  8 ) & 0xff;
  c[2] = ( wk >> 16 ) & 0xff;
  c[3] = ( wk >> 24 ) & 0xff;
  if(ex_fwrite( c, sizeof(Uchar), 4, fp )==4 ) return TRUE;
  else                                       return FALSE;
}
static	int	resume_double( FILE *fp, double *val )
{
  Uchar c[4];
  int	wk;

  if(ex_fread( c, sizeof(Uchar), 4, fp )!=4 ) return FALSE;
  wk = ((Uint)c[3]<<24) | ((Uint)c[2]<<16) | ((Uint)c[1]<<8) | (Uint)c[0];

  *val = (double)wk / 1000000.0;
  return TRUE;
}





/*======================================================================*/
/* ステートファイルにデータを記録					*/
/*	記録に成功すれば、記録したサイズ(バイト数)が返る		*/
/*	      失敗すれば、-1 が返る					*/
/*======================================================================*/
int	suspend_work( FILE *fp, long offset, T_SUSPEND_W *tbl, int count )
{
  int	i;
  int	size = 0;

  if( ex_fseek( fp, offset, SEEK_SET ) == 0 ){

    if( tbl ){
      for( i=0; i<count; i++ ){
	switch( tbl[i].type ){

	case TYPE_INT:
	  if( suspend_int(fp,(int *)tbl[i].work)==FALSE) goto SUSPEND_ERR;
	  size += 4;
	  break;

	case TYPE_SHORT:
	case TYPE_WORD:
	  if( suspend_short(fp,(short *)tbl[i].work)==FALSE) goto SUSPEND_ERR;
	  size += 2;
	  break;

	case TYPE_CHAR:
	case TYPE_BYTE:
	  if( suspend_char(fp,(char *)tbl[i].work)==FALSE) goto SUSPEND_ERR;
	  size += 1;
	  break;

	case TYPE_PAIR:
	  if( suspend_pair(fp,(pair *)tbl[i].work)==FALSE) goto SUSPEND_ERR;
	  size += 2;
	  break;

	case TYPE_DOUBLE:
	  if( suspend_double(fp,(double *)tbl[i].work)==FALSE)goto SUSPEND_ERR;
	  size += 4;
	  break;

	case TYPE_STR:
	  if( suspend_str(fp,(char *)tbl[i].work)==FALSE) goto SUSPEND_ERR;
	  size += 1024;
	  break;

	case TYPE_256:
	  if( suspend_256(fp,(char *)tbl[i].work)==FALSE) goto SUSPEND_ERR;
	  size += 256;
	  break;

	}
      }
    }
    return size;
  }
    
SUSPEND_ERR:;
  return -1;
}




/*======================================================================*/
/* ステートファイルからデータを取り出す					*/
/*	復元に成功すれば、復元したサイズ(バイト数)が返る		*/
/*	      失敗すれば、-1 が返る					*/
/*======================================================================*/
int	resume_work( FILE *fp, long offset, T_SUSPEND_W *tbl, int count )
{
  int	i;
  int	size = 0;

  if( ex_fseek( fp, offset, SEEK_SET ) == 0 ){

    if( tbl ){
      for( i=0; i<count; i++ ){
	switch( tbl[i].type ){

	case TYPE_INT:
	  if( resume_int(fp,(int *)tbl[i].work)==FALSE) goto RESUME_ERR;
	  size += 4;
	  break;

	case TYPE_SHORT:
	case TYPE_WORD:
	  if( resume_short(fp,(short *)tbl[i].work)==FALSE) goto RESUME_ERR;
	  size += 2;
	  break;

	case TYPE_CHAR:
	case TYPE_BYTE:
	  if( resume_char(fp,(char *)tbl[i].work)==FALSE) goto RESUME_ERR;
	  size += 1;
	  break;

	case TYPE_PAIR:
	  if( resume_pair(fp,(pair *)tbl[i].work)==FALSE) goto RESUME_ERR;
	  size += 2;
	  break;

	case TYPE_DOUBLE:
	  if( resume_double(fp,(double *)tbl[i].work)==FALSE) goto RESUME_ERR;
	  size += 4;
	  break;

	case TYPE_STR:
	  if( resume_str(fp,(char *)tbl[i].work)==FALSE) goto RESUME_ERR;
	  size += 1024;
	  break;

	case TYPE_256:
	  if( resume_256(fp,(char *)tbl[i].work)==FALSE) goto RESUME_ERR;
	  size += 256;
	  break;
	}
      }
    }
    return size;
  }
    
RESUME_ERR:;
  return -1;
}




/*======================================================================*/
/* ステートファイルのヘッダに情報を書き込む				*/
/*======================================================================*/
static	int	suspend_header( FILE *fp, long offset )
{
  char c[128];
  int ptr = 0;

  memset( c, 0, sizeof(c) );

  if( ex_fseek( fp, offset, SEEK_SET ) == 0 ){

    strcpy( &c[ ptr ], Q_TITLE );		/* タイトル */
    ptr += strlen( &c[ ptr ] ) + 1;

    strcpy( &c[ ptr ], Q_VERSION );		/* バージョン */
    ptr += strlen( &c[ ptr ] ) + 1;


    if( verbose_suspend )
      printf( "suspend <%s>: %d byte\n", __FILE__, ptr );
    if( (int)sizeof( ((suspend_0*)0)->header ) < ptr ){
      fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
      main_exit(4);
    }

    if(ex_fwrite( c, sizeof(char), sizeof(c), fp ) == sizeof(c) ) return TRUE;

  }
  return FALSE;
}



/*======================================================================*/
/* ステートファイルのヘッダから情報を読み込む				*/
/*======================================================================*/
static	int	resume_header( FILE *fp, long offset )
{
  char c[128];
  char *title, *ver;


  memset( c, 0, sizeof(c) );

  if( ex_fseek( fp, offset, SEEK_SET ) == 0 ){

    if(ex_fread( c, sizeof(char), sizeof(c), fp ) != sizeof(c) ) return FALSE;

    title = &c[0];				/* タイトル */

    ver = &c[ strlen( title )+1 ];		/* バージョン */

    if( verbose_suspend )
      printf( "resume %s %s\n", title, ver );

    if( strcmp( title, Q_TITLE ) != 0 ){
      fprintf( stderr, "resume...title mismatch ('%s' != '%s')\n",
						       Q_TITLE, title );
      if( resume_force==FALSE )
	return FALSE;
    }

    if( strcmp( ver, Q_VERSION ) != 0 ){
      fprintf( stderr, "resume...version mismatch ('%s' != '%s')\n",
	       						Q_VERSION, ver );
      if( resume_force==FALSE )
	return FALSE;
    }
  }

  return TRUE;
}







/************************************************************************/
/* サスペンドする							*/
/*	成功すれば真が、失敗すれば偽が返る				*/
/************************************************************************/
int	suspend( const char *suspend_filename )
{
  FILE	*fp;

  if( suspend_filename   ==NULL ) return FALSE;
  if( suspend_filename[0]=='\0' ) return FALSE;

  if( verbose_suspend )
    printf( "suspend -------------------- '%s'\n", suspend_filename );

//  if( (fp = ex_fopen( suspend_filename, "wb" )) == NULL ){
  if( (fp = ex_fopen( suspend_filename, "wct" )) == NULL ){
    return FALSE;
  }

  if( !	suspend_header  ( fp, OFFSETOF(suspend_0,header  ) ) )	goto FAILED;

  if( ! suspend_emu	( fp, OFFSETOF(suspend_0,emu     ) ) )	goto FAILED;
  if( ! suspend_memory	( fp, OFFSETOF(suspend_0,memory  ), 
			      OFFSETOF(suspend_0,memory_data))) goto FAILED;
  if( ! suspend_wait	( fp, OFFSETOF(suspend_0,wait    ) ) )	goto FAILED;
  if( ! suspend_pc88main( fp, OFFSETOF(suspend_0,pc88main) ) )	goto FAILED;
  if( ! suspend_crtcdmac( fp, OFFSETOF(suspend_0,crtcdmac) ) )	goto FAILED;
  if( ! suspend_sound	( fp, OFFSETOF(suspend_0,sound   ) ) )	goto FAILED;
  if( ! suspend_pio	( fp, OFFSETOF(suspend_0,pio     ) ) )	goto FAILED;
  if( ! suspend_screen	( fp, OFFSETOF(suspend_0,screen  ) ) )	goto FAILED;
  if( ! suspend_intr	( fp, OFFSETOF(suspend_0,intr    ) ) )	goto FAILED;
  if( ! suspend_keyboard( fp, OFFSETOF(suspend_0,keyboard) ) )	goto FAILED;
  if( ! suspend_pc88sub	( fp, OFFSETOF(suspend_0,pc88sub ) ) )	goto FAILED;
//  if( ! suspend_fdc	( fp, OFFSETOF(suspend_0,fdc     ),
//			      OFFSETOF(suspend_0,fdc_data) ) )  goto FAILED;

  ex_fclose( fp );
  return TRUE;

FAILED:;
  ex_fclose( fp );
  return FALSE;
}




/************************************************************************/
/* レジュームする							*/
/*	成功すれば真が、失敗すれば偽が返る				*/
/************************************************************************/
int	resume( const char *suspend_filename )
{
  FILE	*fp;
  char *s;

  if( suspend_filename   ==NULL ) return FALSE;
  if( suspend_filename[0]=='\0' ) return FALSE;

  if( (fp = ex_fopen( suspend_filename, "rb" )) == NULL ){
    return FALSE;
  }

s="header";
  if( !	resume_header   ( fp, OFFSETOF(suspend_0,header  ) ) )	goto FAILED;

s="emu";
  if( ! resume_emu	( fp, OFFSETOF(suspend_0,emu     ) ) )	goto FAILED;
s="memory";
  if( ! resume_memory	( fp, OFFSETOF(suspend_0,memory  ), 
			      OFFSETOF(suspend_0,memory_data))) goto FAILED;
s="wait";
  if( ! resume_wait	( fp, OFFSETOF(suspend_0,wait    ) ) )	goto FAILED;
s="main";
  if( ! resume_pc88main	( fp, OFFSETOF(suspend_0,pc88main) ) )	goto FAILED;
s="crtc";
  if( ! resume_crtcdmac	( fp, OFFSETOF(suspend_0,crtcdmac) ) )	goto FAILED;
s="sound";
  if( ! resume_sound	( fp, OFFSETOF(suspend_0,sound   ) ) )	goto FAILED;
s="pio";
  if( ! resume_pio	( fp, OFFSETOF(suspend_0,pio     ) ) )	goto FAILED;
s="screen";
  if( ! resume_screen	( fp, OFFSETOF(suspend_0,screen  ) ) )	goto FAILED;
s="intr";
  if( ! resume_intr	( fp, OFFSETOF(suspend_0,intr    ) ) )	goto FAILED;
s="key";
  if( ! resume_keyboard	( fp, OFFSETOF(suspend_0,keyboard) ) )	goto FAILED;
s="sub";
  if( ! resume_pc88sub	( fp, OFFSETOF(suspend_0,pc88sub ) ) )	goto FAILED;
s="fdc";
//  if( ! resume_fdc	( fp, OFFSETOF(suspend_0,fdc     ),
//			      OFFSETOF(suspend_0,fdc_data) ) )  goto FAILED;

  ex_fclose( fp );
  return TRUE;

FAILED:;
  /*printf("err %s\n",s);*/
  ex_fclose( fp );
  return FALSE;
}




/************************************************************************/
/* ステートファイル名を初期化						*/
/*	ただし、初期化できない場合は NULL				*/
/************************************************************************/
void	resume_init( void )
{
  int  status;

  if( resume_flag ){			/* レジュームする場合 */

    if( file_resume==NULL ){			/* -resume 指定時 */

      file_resume = osd_get_home_filename( QUASI88STATE_FILE, &status );
      /* ${HOME} が無い場合は、NULL */

    }else{					/* -refumefile 指定時 */
      ; /* file_resume は定義済 */
    }

  }else{				/* レジュームしない場合 */

    file_resume = osd_get_home_filename( QUASI88STATE_FILE, &status );

    if( file_resume==NULL ){
      file_resume = "";
    }

  }


	/* システム初期化前に、必要なレジューム部分を先にやってしまおう */

  if( resume_flag ){

    if( file_resume ){

      FILE *fp = ex_fopen( file_resume, "rb" );

      if( fp ){
	resume_memory_preparation( fp, OFFSETOF(suspend_0,memory) );
	ex_fclose( fp );
      }

    }

  }
}
