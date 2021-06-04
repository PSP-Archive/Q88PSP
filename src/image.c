/************************************************************************/
/*									*/
/* ディスクイメージのヘッダ部の処理 (読み書き)				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "drive.h"
#include "image.h"
#include "file-op.h"





/************************************************************************/
/* 指定されたイメージの先頭ファイル位置から、ヘッダ情報を読み出す。	*/
/*	引数	FILE *fp	ファイルポインタ			*/
/*		long offset	イメージの先頭のファイル位置		*/
/*		Uchar header[32] ヘッダ情報の格納先			*/
/*	返り値	0	正常終了					*/
/*		1	これ以上イメージが無い				*/
/*		2	イメージ異常(ヘッダ破損、サイズ不足)		*/
/*		3	シークエラー					*/
/*		header[32] には、イメージのヘッダの内容が入る。		*/
/*		なお、エラー時は、header[]の内容は不定。		*/
/************************************************************************/
int	fread_header( FILE *fp, long offset, Uchar header[32] )
{
  long	size;
  char	c;

  if( ex_fseek( fp, offset,  SEEK_SET ) ) return 4;	/* Seek Error */

  size =ex_fread( header, sizeof(Uchar), 32, fp );
  if( size==0 )  return 1;				/* File End */
  if( size!=32 ) return 2;				/* Header Broken */


  size = read_size_in_header( header );

  if( ex_fseek( fp, offset+size-1, SEEK_SET ) ) return 3;	/* Seek Error */

  if(ex_fread( &c, sizeof(char), 1, fp )!=1 ) return 2;	/* Image Broken */

  return 0;
}






/************************************************************************/
/* 指定されたモードでファイルを開く。					*/
/*	ドライブに設定されているファイルの場合、そのポインタを返す。	*/
/*	そうでない場合は、ファイルを開いてそのポインタを返す		*/
/*	引数	char *filename	ファイル名				*/
/*		char *mode	モード ("r+","w+","a")			*/
/*		int *result	結果を返すワーク			*/
/*	返り値	ポインタ	ファイルポインタ			*/
/*				NULLの場合は、エラー発生。		*/
/*		*resultにはポインタ情報またはエラー結果が入る		*/
/*		正常時)							*/
/*			*result==-1 … 新規にファイルをオープン		*/
/*			*result== 0 … ドライブ 0: のファイルをオープン	*/
/*			*result== 1 … ドライブ 1: のファイルをオープン	*/
/*		異常時)							*/
/*			*result==-1 … ファイルのオープンエラー		*/
/*			*result== 0 … ドライブ 0: のファイル。読込専用	*/
/*			*result== 1 … ドライブ 1: のファイル。読込専用	*/
/************************************************************************/
FILE	*fopen_image_edit( char *filename, char *mode, int *result )
{
  if      ( drive[0].fp   &&
	    osd_file_cmp( drive[0].filename, filename )==FILE_CMP_OK ){
    *result = 0;
  }else if( drive[1].fp   &&
	    osd_file_cmp( drive[1].filename, filename )==FILE_CMP_OK ){
    *result = 1;
  }else{
    *result = -1;
  }

  switch( *result ){
  case -1:
    return ( ex_fopen( filename, mode ) );
  case 0:
  case 1:
    if( drive[ *result ].read_only ) return NULL;
    else                             return drive[ *result ].fp;
  default:
    printf("Internal Error in fopen_image_edit()\n");
    return NULL;
  }
}




/************************************************************************/
/* 指定されたイメージの最後に、ブランクディスクのイメージを追加する。	*/
/*	追加されるのは、2D のディスク。2DD/2HD は未対応			*/
/*	引数	FILE *fp	ファイルポインタ			*/
/*	返り値	0	正常終了					*/
/*		3	シークエラー					*/
/*		4	ライトエラー					*/
/************************************************************************/
int	append_blank( FILE *fp )
{
  char *s, c[256];
  int	i, j, error = 0;
  int	type = DISK_TYPE_2D;
  int	trk_nr, trk_size, img_size;

  switch( type ){
  case DISK_TYPE_2HD:
    trk_nr   = 164;
    trk_size = 0x2300;
    break;
  case DISK_TYPE_2DD:
    trk_nr   = 164;
    trk_size = 0x1600;
    break;
  case DISK_TYPE_2D:
  default:
    type     = DISK_TYPE_2D;
    trk_nr   = 84;
    trk_size = 0x1600;
    break;
  }
  img_size = (32) + (164*4) + (trk_nr*trk_size);


  if( (ex_fseek( fp, 0,  SEEK_END )) ){
    error = 3;
  }

	/* ヘッダ部 */

  if( error==0 ){
    s = &c[0];
    for(i=0;i<17;i++) *s++ = 0;		/* filename */
    for(i=0;i<9;i++)  *s++ = 0;		/* reserved */
    *s++ = DISK_PROTECT_FALSE;		/* protect  */
    *s++ = type;			/* type     */
    *s++ = (img_size >>  0) & 0xff;	/* size     */
    *s++ = (img_size >>  8) & 0xff;
    *s++ = (img_size >> 16) & 0xff;
    *s++ = (img_size >> 24) & 0xff;
    if(ex_fwrite( c, sizeof(Uchar), 32, fp )!=32 ){
      error = 4;
    }
  }

	/* トラック部分 */

  if( error==0 ){
    j = 0x2b0;				/* size of track data */
    for(i=0;i<164;i++){
      if( i<trk_nr ){
	c[0] = (j >>  0)&0xff;
	c[1] = (j >>  8)&0xff;
	c[2] = (j >> 16)&0xff;
	c[3] = (j >> 24)&0xff;
	j += trk_size;
      }else{
	c[0] = c[1] = c[2] = c[3] = 0;
      }
      if(ex_fwrite( c, sizeof(Uchar), 4, fp )!=4 ) break;
    }
    if( i!=164 ){
      error = 4;
    }
  }

	/* セクタ部分 */

  if( error==0 ){
    for( i=0; i<256; i++ ) c[i] = 0;	/* sec data */
    for( i=0; i<trk_nr*trk_size; ){
      if( trk_nr*trk_size - i < 256 ) j = trk_nr*trk_size - i;
      else                            j = 256;
      if(ex_fwrite( c, sizeof(Uchar), j, fp )!=(size_t)j ) break;
      i += j;
    }
    if( i!=trk_nr*trk_size ){
      error = 4;
    }
  }

  fflush( fp );
  return error;
}


/************************************************************************/
/* ブランクディスクのイメージを追加後、drive[]ワークを更新する。	*/
/*	引数	int drv		ドライブ番号				*/
/*	返り値	なし							*/
/************************************************************************/
void	update_after_append_blank( int drv )
{
  int	i, img;
  int	repeat = (drive[0].fp==drive[1].fp) ? 2 : 1;
  int	type = DISK_TYPE_2D;
  int	img_size;

  switch( type ){
  case DISK_TYPE_2HD:
    img_size = 164 * 0x2300;
    break;
  case DISK_TYPE_2DD:
    img_size = 164 * 0x1600;
    break;
  case DISK_TYPE_2D:
  default:
    type     = DISK_TYPE_2D;
    img_size = 84 * 0x1600;
    break;
  }
  img_size += (32) + (164*4);


  for( i=0; i < repeat; i++ ){

    if( !drive[ drv ].detect_broken_image ){

      drive[ drv ].file_size += img_size;
      if( drive[ drv ].image_nr + 1 <= MAX_NR_IMAGE ){
	img = drive[ drv ].image_nr;
	drive[drv].image[img].name[0] = '\0';
	drive[drv].image[img].protect = DISK_PROTECT_FALSE;
	drive[drv].image[img].type    = type;
	drive[drv].image[img].size    = img_size;
	drive[ drv ].image_nr ++;
      }else{
	drive[ drv ].over_image = TRUE;
      }

    }
    drv ^= 1;
  }

}


/************************************************************************/
/* 指定されたイメージ番号の、プロテクト情報を書き換える。		*/
/*	引数	FILE *fp	ファイルポインタ			*/
/*		int  img	イメージ番号				*/
/*		char protect    プロテクト情報				*/
/*	返り値	0	正常終了					*/
/*		1	指定番号のイメージ無し				*/
/*		2	イメージ異常(ヘッダ破損、サイズ不足)		*/
/*		3	シークエラー					*/
/*		4	ライトエラー					*/
/*		5	イメージ数オーバー				*/
/************************************************************************/
int	fwrite_protect( FILE *fp, int img, char *protect )
{
  Uchar	c[32];
  long	offset = 0;
  int	num = 0, result;

  while( (result=fread_header( fp, offset, c ) ) == 0 ){

    if( img == num ){
      if( ex_fseek( fp, offset+DISK_PROTECT, SEEK_SET )==0 ){
	if(ex_fwrite( protect, sizeof(char), 1, fp)==1 ){
	  /* success */
	}
	else result = 4;
      }else  result = 3;
      break;
    }

    num ++;
    offset += read_size_in_header( c );

    if( offset < 0 || num > 256 ){
      result = 5;
      break;
    }
  }
  fflush( fp );
  return result;
}
/************************************************************************/
/* プロテクトワークの更新後、drive[]ワークを更新する。			*/
/*	引数	int drv		ドライブ番号				*/
/*	返り値	なし							*/
/************************************************************************/
void	update_after_fwrite_protect( int drv, int img, char *protect )
{
  int	i;
  int	repeat = (drive[0].fp==drive[1].fp) ? 2 : 1;

  if( img < MAX_NR_IMAGE ){

    for( i=0; i < repeat; i++ ){

      drive[ drv ].image[ img ].protect = *protect;
      if( drive[ drv ].selected_image == img ){
	drive[ drv ].protect = *protect;
	if( drive[ drv ].read_only ) drive[ drv ].protect = DISK_PROTECT_TRUE;
      }

      drv ^= 1;
    }
  }
}



/************************************************************************/
/* 指定されたイメージ番号の、イメージの名前を書き換える。		*/
/*	引数	FILE *fp	ファイルポインタ			*/
/*		int  img	イメージ番号				*/
/*		char *name      イメージネーム(16文字)			*/
/*	返り値	0	正常終了					*/
/*		1	指定番号のイメージ無し				*/
/*		2	イメージ異常(ヘッダ破損、サイズ不足)		*/
/*		3	シークエラー					*/
/*		4	ライトエラー					*/
/*		5	イメージ数オーバー				*/
/************************************************************************/
int	fwrite_name( FILE *fp, int img, char *name )
{
  Uchar	c[32];
  long	offset = 0;
  int	num = 0, result;

  while( (result=fread_header( fp, offset, c ) ) == 0 ){

    if( img == num ){
      strncpy( (char *)c, name, 16 );
      c[16] = '\0';
      if( ex_fseek( fp, offset+DISK_FILENAME, SEEK_SET )==0 ){
	if(ex_fwrite( c, sizeof(Uchar), 17, fp)==17 ){
	  /* success */
	}
	else result = 4;
      }else  result = 3;
      break;
    }

    num ++;
    offset += read_size_in_header( c );

    if( offset < 0 || num > 256 ){
      result = 5;
      break;
    }
  }
  fflush( fp );
  return result;
}
/************************************************************************/
/* イメージの名前の更新後、drive[]ワークを更新する。			*/
/*	引数	int drv		ドライブ番号				*/
/*	返り値	なし							*/
/************************************************************************/
void	update_after_fwrite_name( int drv, int img, char *name )
{
  int	i;
  int	repeat = (drive[0].fp==drive[1].fp) ? 2 : 1;

  if( img < MAX_NR_IMAGE ){

    for( i=0; i < repeat; i++ ){

      strncpy( drive[ drv ].image[ img ].name, name, 17 );

      drv ^= 1;
    }
  }
}



/************************************************************************/
/* 指定されたイメージ番号の、イメージをアンフォーマット状態にする。	*/
/*	引数	FILE *fp	ファイルポインタ			*/
/*		int  img	イメージ番号				*/
/*	返り値	0	正常終了					*/
/*		1	指定番号のイメージ無し				*/
/*		2	イメージ異常(ヘッダ破損、サイズ不足)		*/
/*		3	シークエラー					*/
/*		4	ライトエラー					*/
/*		5	イメージ数オーバー				*/
/************************************************************************/
int	fwrite_unformat( FILE *fp, int img )
{
  Uchar	c[256];
  long	offset = 0, st, sz, len;
  int	num = 0, result, i;

  while( (result=fread_header( fp, offset, c ) ) == 0 ){

    if( img == num ){
      sz = read_size_in_header( c );
      if( ex_fseek( fp, offset+DISK_TRACK, SEEK_SET )==0 ){
	if(ex_fread( c, sizeof(Uchar), 4, fp)==4 ){
	  st = ((long)c[0]+((long)c[1]<<8)+((long)c[2]<<16)+((long)c[3]<<24));
	  if( ex_fseek( fp, offset+st, SEEK_SET )==0 ){
	    for( i=0; i<256; i++ ) c[i] = 0x00;
	    if( DISK_TRACK+4 <= st && st < sz ){
	      while( st < sz ){
		if( sz - st < 256 ) len = sz - st;
		else                len = 256;
		if(ex_fwrite( c, sizeof(Uchar), len, fp )==(size_t)len ){
		  /* seccess */
		}
		else result = 4;
		if( result ) break;
		st += len;
	      }
	    }else result = 2;
	  }else   result = 3;
	}
	else result = 4;
      }else  result = 3;
      break;
    }

    num ++;
    offset += read_size_in_header( c );

    if( offset < 0 || num > 256 ){
      result = 5;
      break;
    }
  }
  fflush( fp );
  return result;
}



/************************************************************************/
/* 指定されたイメージ番号の、イメージをフォーマットする。		*/
/*	2D の、N88-BASIC データディスクとしてフォーマットを行なう。	*/
/*	引数	FILE *fp	ファイルポインタ			*/
/*		int  img	イメージ番号				*/
/*	返り値	0	正常終了					*/
/*		1	指定番号のイメージ無し				*/
/*		2	イメージ異常(ヘッダ破損、サイズ不足)		*/
/*		3	シークエラー					*/
/*		4	ライトエラー					*/
/*		5	イメージ数オーバー				*/
/************************************************************************/

#define	generate_sector_data()					\
	do{							\
	  if( trk==37 && sec==13 ){				\
	    i = 1;						\
	  }else if( trk==37 && (sec==14||sec==15||sec==16) ){	\
	    i = 2;						\
	  }else{						\
	    i = 0;						\
	  }							\
	  d[i][ DISK_C ] = trk >> 1;				\
	  d[i][ DISK_H ] = trk & 1;				\
	  d[i][ DISK_R ] = sec;					\
	  d[i][ DISK_N ] = 1;					\
	  d[i][ DISK_SEC_NR +0 ] = 16 >> 0;			\
	  d[i][ DISK_SEC_NR +1 ] = 16 >> 8;			\
	  d[i][ DISK_DENSITY ] = DISK_DENSITY_DOUBLE;		\
	  d[i][ DISK_DELETED ] = DISK_DELETED_FALSE;		\
	  d[i][ DISK_STATUS ] = 0;				\
	  d[i][ DISK_SEC_SZ +0 ] = ((256) >> 0) & 0xff;		\
	  d[i][ DISK_SEC_SZ +1 ] = ((256) >> 8) & 0xff;		\
	}while(0)

int	fwrite_format( FILE *fp, int img )
{
  Uchar	c[32];
  Uchar	d[3][ 16 + 256 ];
  long	offset = 0, st, sz;
  int	num = 0, result, trk, sec;

  int	i;
  for( i=0; i<16;  i++ ) d[ 0 ][ i ] = 0x00;
  for( i=0; i<256; i++ ) d[ 0 ][ 16 +i ] = 0xff;
  for( i=0; i<16;  i++ ) d[ 1 ][ i ] = 0x00;
  for( i=0; i<256; i++ ) d[ 1 ][ 16 +i ] = 0x00;
  for( i=0; i<16;  i++ ) d[ 2 ][ i ] = 0x00;
  for( i=0; i<160; i++ ) d[ 2 ][ 16 +i ] = 0xff;
  for(    ; i<256; i++ ) d[ 2 ][ 16 +i ] = 0x00;
  d[ 2 ][ 16 +37*2    ] = 0xfe;
  d[ 2 ][ 16 +37*2 +1 ] = 0xfe;

  while( (result=fread_header( fp, offset, c ) ) == 0 ){

    if( img == num ){
      sz = read_size_in_header( c );
      c[ 0 ] = DISK_TYPE_2D;
      if( ex_fseek( fp, offset+DISK_TYPE, SEEK_SET )==0 ){
	if(ex_fwrite( c, sizeof(Uchar), 1, fp)==1 ){		/* 2D に設定 */
	  for( trk=0; trk<80; trk++ ){
	    if( ex_fseek( fp, offset+(DISK_TRACK+trk*4), SEEK_SET )==0 ){
	      if(ex_fread( c, sizeof(Uchar), 4, fp)==4 ){
		st =  (long)c[0]     +((long)c[1]<<8)
		    +((long)c[2]<<16)+((long)c[3]<<24);
		if( ex_fseek( fp, offset+st, SEEK_SET )==0 ){
		  if( DISK_TRACK+80*4 <= st && st+(16+256)*16 <= sz ){
		    for( sec=1; sec<=16; sec++ ){
		      generate_sector_data();
		      if(ex_fwrite( d[i], sizeof(Uchar), 16+256, fp )==16+256 ){
			/* seccess */
		      }
		      else result = 4;
		      if( result ) break;
		    }
		  }
		  else result = 2;
		}else  result = 3;
	      }
	      else result = 2;
	    }else  result = 3;
	    if( result ) break;
	  }
	}
	else result = 4;
      }else  result = 3;
      break;
    }

    num ++;
    offset += read_size_in_header( c );

    if( offset < 0 || num > 256 ){
      result = 5;
      break;
    }
  }
  fflush( fp );
  return result;
}



/************************************************************************/
/* イメージのフォーマット後、drive[]ワークを更新する。			*/
/*	引数	int drv		ドライブ番号				*/
/*	返り値	なし							*/
/************************************************************************/
void	update_after_fwrite_format( int drv, int img )
{
  int	i;
  int	repeat = (drive[0].fp==drive[1].fp) ? 2 : 1;

  if( img < MAX_NR_IMAGE ){

    for( i=0; i < repeat; i++ ){

      drive[ drv ].image[ img ].type = DISK_TYPE_2D;

      drv ^= 1;
    }
  }
}
