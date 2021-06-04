/************************************************************************/
/*									*/
/* �f�B�X�N�C���[�W�̃w�b�_���̏��� (�ǂݏ���)				*/
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
/* �w�肳�ꂽ�C���[�W�̐擪�t�@�C���ʒu����A�w�b�_����ǂݏo���B	*/
/*	����	FILE *fp	�t�@�C���|�C���^			*/
/*		long offset	�C���[�W�̐擪�̃t�@�C���ʒu		*/
/*		Uchar header[32] �w�b�_���̊i�[��			*/
/*	�Ԃ�l	0	����I��					*/
/*		1	����ȏ�C���[�W������				*/
/*		2	�C���[�W�ُ�(�w�b�_�j���A�T�C�Y�s��)		*/
/*		3	�V�[�N�G���[					*/
/*		header[32] �ɂ́A�C���[�W�̃w�b�_�̓��e������B		*/
/*		�Ȃ��A�G���[���́Aheader[]�̓��e�͕s��B		*/
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
/* �w�肳�ꂽ���[�h�Ńt�@�C�����J���B					*/
/*	�h���C�u�ɐݒ肳��Ă���t�@�C���̏ꍇ�A���̃|�C���^��Ԃ��B	*/
/*	�����łȂ��ꍇ�́A�t�@�C�����J���Ă��̃|�C���^��Ԃ�		*/
/*	����	char *filename	�t�@�C����				*/
/*		char *mode	���[�h ("r+","w+","a")			*/
/*		int *result	���ʂ�Ԃ����[�N			*/
/*	�Ԃ�l	�|�C���^	�t�@�C���|�C���^			*/
/*				NULL�̏ꍇ�́A�G���[�����B		*/
/*		*result�ɂ̓|�C���^���܂��̓G���[���ʂ�����		*/
/*		���펞)							*/
/*			*result==-1 �c �V�K�Ƀt�@�C�����I�[�v��		*/
/*			*result== 0 �c �h���C�u 0: �̃t�@�C�����I�[�v��	*/
/*			*result== 1 �c �h���C�u 1: �̃t�@�C�����I�[�v��	*/
/*		�ُ펞)							*/
/*			*result==-1 �c �t�@�C���̃I�[�v���G���[		*/
/*			*result== 0 �c �h���C�u 0: �̃t�@�C���B�Ǎ���p	*/
/*			*result== 1 �c �h���C�u 1: �̃t�@�C���B�Ǎ���p	*/
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
/* �w�肳�ꂽ�C���[�W�̍Ō�ɁA�u�����N�f�B�X�N�̃C���[�W��ǉ�����B	*/
/*	�ǉ������̂́A2D �̃f�B�X�N�B2DD/2HD �͖��Ή�			*/
/*	����	FILE *fp	�t�@�C���|�C���^			*/
/*	�Ԃ�l	0	����I��					*/
/*		3	�V�[�N�G���[					*/
/*		4	���C�g�G���[					*/
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

	/* �w�b�_�� */

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

	/* �g���b�N���� */

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

	/* �Z�N�^���� */

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
/* �u�����N�f�B�X�N�̃C���[�W��ǉ���Adrive[]���[�N���X�V����B	*/
/*	����	int drv		�h���C�u�ԍ�				*/
/*	�Ԃ�l	�Ȃ�							*/
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
/* �w�肳�ꂽ�C���[�W�ԍ��́A�v���e�N�g��������������B		*/
/*	����	FILE *fp	�t�@�C���|�C���^			*/
/*		int  img	�C���[�W�ԍ�				*/
/*		char protect    �v���e�N�g���				*/
/*	�Ԃ�l	0	����I��					*/
/*		1	�w��ԍ��̃C���[�W����				*/
/*		2	�C���[�W�ُ�(�w�b�_�j���A�T�C�Y�s��)		*/
/*		3	�V�[�N�G���[					*/
/*		4	���C�g�G���[					*/
/*		5	�C���[�W���I�[�o�[				*/
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
/* �v���e�N�g���[�N�̍X�V��Adrive[]���[�N���X�V����B			*/
/*	����	int drv		�h���C�u�ԍ�				*/
/*	�Ԃ�l	�Ȃ�							*/
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
/* �w�肳�ꂽ�C���[�W�ԍ��́A�C���[�W�̖��O������������B		*/
/*	����	FILE *fp	�t�@�C���|�C���^			*/
/*		int  img	�C���[�W�ԍ�				*/
/*		char *name      �C���[�W�l�[��(16����)			*/
/*	�Ԃ�l	0	����I��					*/
/*		1	�w��ԍ��̃C���[�W����				*/
/*		2	�C���[�W�ُ�(�w�b�_�j���A�T�C�Y�s��)		*/
/*		3	�V�[�N�G���[					*/
/*		4	���C�g�G���[					*/
/*		5	�C���[�W���I�[�o�[				*/
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
/* �C���[�W�̖��O�̍X�V��Adrive[]���[�N���X�V����B			*/
/*	����	int drv		�h���C�u�ԍ�				*/
/*	�Ԃ�l	�Ȃ�							*/
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
/* �w�肳�ꂽ�C���[�W�ԍ��́A�C���[�W���A���t�H�[�}�b�g��Ԃɂ���B	*/
/*	����	FILE *fp	�t�@�C���|�C���^			*/
/*		int  img	�C���[�W�ԍ�				*/
/*	�Ԃ�l	0	����I��					*/
/*		1	�w��ԍ��̃C���[�W����				*/
/*		2	�C���[�W�ُ�(�w�b�_�j���A�T�C�Y�s��)		*/
/*		3	�V�[�N�G���[					*/
/*		4	���C�g�G���[					*/
/*		5	�C���[�W���I�[�o�[				*/
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
/* �w�肳�ꂽ�C���[�W�ԍ��́A�C���[�W���t�H�[�}�b�g����B		*/
/*	2D �́AN88-BASIC �f�[�^�f�B�X�N�Ƃ��ăt�H�[�}�b�g���s�Ȃ��B	*/
/*	����	FILE *fp	�t�@�C���|�C���^			*/
/*		int  img	�C���[�W�ԍ�				*/
/*	�Ԃ�l	0	����I��					*/
/*		1	�w��ԍ��̃C���[�W����				*/
/*		2	�C���[�W�ُ�(�w�b�_�j���A�T�C�Y�s��)		*/
/*		3	�V�[�N�G���[					*/
/*		4	���C�g�G���[					*/
/*		5	�C���[�W���I�[�o�[				*/
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
	if(ex_fwrite( c, sizeof(Uchar), 1, fp)==1 ){		/* 2D �ɐݒ� */
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
/* �C���[�W�̃t�H�[�}�b�g��Adrive[]���[�N���X�V����B			*/
/*	����	int drv		�h���C�u�ԍ�				*/
/*	�Ԃ�l	�Ȃ�							*/
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
