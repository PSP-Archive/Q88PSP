/************************************************************************/
/*									*/
/* �t�@�C�������A�t�@�C��������֐� (OS�ˑ�)				*/
/*									*/
/* �y�֐��z								*/
/*									*/
/*	�s�f�B���N�g���̉{���t						*/
/*									*/
/* T_DIR_INFO *osd_opendir( const char *filename )			*/
/*									*/
/*	filename �Ŏw�肳�ꂽ�f�B���N�g�����J���A���̏�񃏁[�N�ւ�	*/
/*	�|�C���^��Ԃ��B���̏�񃏁[�N�ւ̃|�C���^�� osd_readdir,	*/
/*	osd_closedir �ɂĎg�p���A�Ăяo���������ڎQ�Ƃ��邱�Ƃ͂Ȃ��B	*/
/*									*/
/*	    ����    *filename �f�B���N�g����				*/
/*	    �Ԓl    ����I�����A�f�B���N�g����񃏁[�N�ւ̃|�C���^�B	*/
/*		    �ُ�I�����ANULL (�f�B���N�g���J����/���̃G���[��)	*/
/*									*/
/*									*/
/* const T_DIR_ENTRY *osd_readdir( T_DIR_INFO *dirp )			*/
/*									*/
/*	osd_opendir �ɂĊJ�����f�B���N�g������G���g������ǂ݂Ƃ�	*/
/*	���e�� T_DIR_ENTRY�^�̃��[�N�ɃZ�b�g���A���̃|�C���^��Ԃ��B	*/
/*									*/
/*		T_DIR_ENTRY�^						*/
/*		    typedef struct {					*/
/*			int    type;	�c �t�@�C���̑���		*/
/*			char  *name;	�c �t�@�C����			*/
/*		    } T_DIR_ENTRY;					*/
/*									*/
/*		�t�@�C�������́A�ȉ��̒l���Ƃ�B			*/
/*		    DIR_ENTRY_TYPE_DIR		�c �f�B���N�g��		*/
/*		    DIR_ENTRY_TYPE_NOT_DIR	�c �t�@�C��(�܂��͕s��)	*/
/*									*/
/*		�����œ�����t�@�C�����́A�f�B���N�g���{�����̕\����	*/
/*		�g�p����B�Ȃ̂ŁA���ۂ̃t�@�C�����Ɠ����ł���K�v��	*/
/*		�Ȃ��B(�Ⴆ�΃f�B���N�g������ <> �ň͂ނȂǂ̑�������)	*/
/*		�܂��A�擾�������Ƀt�@�C�����̕\�����s���̂ŁA�\��	*/
/*		�\�[�g���Ă����ƕ\���̍ۂɌ��₷���Ȃ�B		*/
/*									*/
/*	    ����    *dirp	�f�B���N�g����񃏁[�N�ւ̃|�C���^	*/
/*				( osd_opendir() �̕Ԃ�l) 		*/
/*	    �Ԓl    ����I�����AT_DIR_ENTRY �\���̂ւ̃|�C���^		*/
/*	            ����ȏ�G���g���������Ȃ����G���[���ANULL		*/
/*									*/
/*									*/
/* void osd_closedir( T_DIR_INFO *dirp )				*/
/*									*/
/*	�f�B���N�g�������						*/
/*									*/
/*	    ����    *dirp	�f�B���N�g����񃏁[�N�ւ̃|�C���^	*/
/*				( osd_opendir() �̕Ԃ�l) 		*/
/*									*/
/*									*/
/* void osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,	*/
/*			      int maxsize )				*/
/*									*/
/*	osd_readdir �ɂĎ擾�����t�@�C�������A�A�N�Z�X�\�ȃt�@�C����	*/
/*	�ϊ�����B�Ⴆ�΁Aosd_readdir �̎��� "<dirname>" �Ƃ����t�@�C��	*/
/*	�����擾�����ꍇ�A���̊֐��ł����� "dirname/" �ɕϊ�����B	*/
/*									*/
/*	    ����    *realname	�A�N�Z�X�\�ȃt�@�C�����̕Ԃ郏�[�N	*/
/*		    *dir	�f�B���N�g���G���g��			*/
/*				( osd_opendir() �̕Ԃ�l) 		*/
/*		    max_size	realname �̒���				*/
/*									*/
/*									*/
/*	�s�t�@�C�������t						*/
/*									*/
/* int osd_file_stat( const char *filename )				*/
/*									*/
/*	�t�@�C�����A�f�B���N�g�����A�s�����𔻕ʂ���B			*/
/*									*/
/*	    ����    *filename		�t�@�C����			*/
/*	    �Ԓl    FILE_STAT_DIR	�f�B���N�g��			*/
/*		    FILE_STAT_FILE	�t�@�C��			*/
/*		    FILE_STAT_ANOTHER	����ȊO(���݂��Ȃ����܂�)	*/
/*									*/
/*									*/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*									*/
/*	2�̃t�@�C�����A�������̂��ǂ����`�F�b�N����B			*/
/*									*/
/*	    ����    *s_file		�t�@�C���� 1			*/
/*		    *t_file		�t�@�C���� 2			*/
/*	    �Ԓl    FILE_CMP_OK		��v				*/
/*		    FILE_CMP_NOT_OK	�s��v				*/
/*		    FILE_CMP_ERR	�t�@�C���A�N�Z�X�G���[		*/
/*									*/
/*									*/
/* long	osd_file_size( const char *filename )				*/
/*									*/
/*	�t�@�C���̃T�C�Y(�o�C�g��)��Ԃ��B				*/
/*	�t�@�C���T�C�Y���s���̏ꍇ�A0 ��Ԃ��Ă������x���Ȃ�		*/
/*									*/
/*	    ����    *filename	�t�@�C����				*/
/*	    �Ԓl    -1		�A�N�Z�X�G���[				*/
/*		    0�`		�t�@�C���T�C�Y				*/
/*									*/
/*									*/
/*	�s�t�@�C�����A�p�X�������t					*/
/*									*/
/* void osd_path_regularize( char *filename, int max_size )		*/
/*									*/
/*	�p�X���𐳋K��(?)����B						*/
/*	��) "dir1/./dir2//dir3/../../file" �� "dir1/file" �ɕϊ������	*/
/*									*/
/*	    ����    *filename	�t�@�C���� �܂��̓f�B���N�g����		*/
/*				���ʂ́A�������̂ɏ㏑�������B	*/
/*		    max_size	filename �̒���				*/
/*									*/
/*									*/
/* const char *osd_path_separate( char *result_path, 			*/
/*				  const char *filename,int max_size )	*/
/*									*/
/*	filename �̃f�B���N�g�������� result_path �ɃZ�b�g����B	*/
/*	���ʂ́Aresult_path �ɁA�ő� max_size �o�C�g �i�[�����		*/
/*	filename �� max_file �����傫���ꍇ��Afilename�� '/' ��	*/
/*	�܂܂�Ȃ��ꍇ�Aresult_path �ɂ� �󕶎��� ("") ���Z�b�g�����B	*/
/*	�Ԓl�Ƃ��āA�\�Ȃ�� filename �̃t�@�C���������̐擪�̃A�h	*/
/*	���X���߂�B���ꂪ�s�\�Ȃ� NULL ���Ԃ�			*/
/*	��) filename   �� "/local/tmp/filename" �̏ꍇ�A		*/
/*	 resule_path �ɂ� "/local/tmp/"         ���Z�b�g����A		*/
/*	 �Ԓl�́Afilename �� 'f' ���w���|�C���^�ƂȂ�B			*/
/*									*/
/*	    ����    *result_path	���ʂ̕Ԃ郏�[�N		*/
/*		    *filename		�t�@�C����			*/
/*		    max_size		result_path �̒���		*/
/*	    �Ԓl    filename�́A�t�@�C���������̐擪���w���|�C���^�A	*/
/*		    �Ȃ����́ANULL					*/
/*									*/
/*									*/
/* void osd_path_connect( char *result_path, const char *parent_path,	*/
/*			   const char *child_path, int max_size )	*/
/*									*/
/*	parent_path �ɁAchild_path ���Ȃ��āA��̃p�X�ɂ���B	*/
/*	�������Achild_path ����΃p�X ('/' �Ŏn�܂�) �̏ꍇ�́A		*/
/*	child_path ���̂��̂�Ԃ��B					*/
/*	���ʂ́Aresult_path �ɁA�ő� max_size �o�C�g �i�[�����		*/
/*									*/
/*	    ����    *result_path	���ʂ̕Ԃ郏�[�N		*/
/*		    *parent_path	�e�̃p�X��			*/
/*		    *child_path		�q�̃p�X��			*/
/*		    max_size		result_path �̒���		*/
/*									*/
/*									*/
/*	�s���ϐ������t						*/
/*									*/
/* void	osd_get_environment( int *status )				*/
/*	���ϐ����擾���A�������[�N������������B			*/
/*	state �� NULL �łȂ���΁A�G���[�����Z�b�g����		*/
/*									*/
/* void	osd_set_image_dir( const char *conf_dir_rom,			*/
/*			   const char *conf_dir_disk )			*/
/*	ROM�C���[�W���J���f�B���N�g�� �� conf_dir_rom �ɐݒ肷��B	*/
/*	DISK�C���[�W���J���f�B���N�g�� �� conf_dir_disk �ɐݒ肷��B	*/
/*	�������A������ NULL �Ȃ�ΐݒ肵�Ȃ��B				*/
/*									*/
/* const char *osd_get_disk_dir( void )					*/
/*	�f�B�X�N�C���[�W���J���f�t�H���g�̃f�B���N�g��(dir_disk)��Ԃ��B*/
/*	����`�̏ꍇ�̓J�����g�f�B���N�g��(osd_get_current_dir)���Ԃ�B	*/
/*									*/
/* const char *osd_get_current_dir( void )				*/
/*	�J�����g�f�B���N�g����Ԃ��B�J�����g�f�B���N�g�����擾�ł��Ȃ�	*/
/*	�ꍇ�́A"." �Ȃǂ̂Ƃ���������̕����񂪕Ԃ�B		*/
/*									*/
/* char *osd_get_home_filename( const char *fname, int *status )	*/
/*	�t�@�C���� ${HOME}/fname ��Ԃ��B				*/
/*	�Ԓl�� malloc ���ꂽ�̈�Ȃ̂ŁA�ďo���� free ���邱�ƁB	*/
/*									*/
/*	    �Ԓl    ����I�����A�t�@�C�������A�ُ�I���� NULL ���Ԃ�B	*/
/*		    ���� status �� NULL �łȂ���΁Astatus �ɂ�		*/
/*		    �ȉ��̂����ꂩ�̒l���Z�b�g�����			*/
/*			HOME_F_OK	����I��			*/
/*			HOME_F_NO_HOME	���ϐ� HOME �����ݒ�		*/
/*			HOME_F_NO_MEM	malloc �Ɏ��s			*/
/*									*/
/*									*/
/*	�s�t�@�C���I�[�v���t						*/
/*									*/
/* FILE *osd_fopen_image( const char *filename, int read_only,		*/
/*			  char **real_filename, int *status )		*/
/*									*/
/*	D88�C���[�W�t�@�C�� (�o�C�i��) ���J���B				*/
/*	���� read_only ���^�̎��� "rb" �ŁA�U�̎��� "r+b" �ŊJ���B	*/
/*	( �������A"r+b"�ŊJ�����Ƃ��Ď��s�����ꍇ�A"rb" �ŊJ�� )	*/
/*									*/
/*	    ����    *filename		�t�@�C����			*/
/*		    read_only		�^�̎��AREAD ONLY �ŊJ��	*/
/*		    **real_filename	���ۂɊJ�����t�@�C���̖��O	*/
/*		    *status		�����̌���			*/
/*									*/
/*	    �Ԓl    ����I�����́A�t�@�C���X�g���[���ւ̃|�C���^���A	*/
/*		    �ُ�I�����́ANULL ���Ԃ�B				*/
/*		    ���� status �� NULL �łȂ���΁Astatus �ɂ�		*/
/*		    �ȉ��̂����ꂩ�̒l���Z�b�g�����			*/
/*			FOPEN_OK_RW	"r+b" �Ńt�@�C���I�[�v������	*/
/*			FOPEN_OK_RO	"rb"  �Ńt�@�C���I�[�v������	*/
/*			FOPEN_NO_MEM	�������� malloc() �Ɏ��s	*/
/*			FOPEN_ERR	�t�@�C�����J���Ȃ�����		*/
/*		    ����I�����ɂ́Areal_filename �ɊJ�����t�@�C������	*/
/*		    �Z�b�g�����B����� malloc()���ꂽ�̈�Ȃ̂ŁA	*/
/*		    �ďo���œK�X�J������B				*/
/*									*/
/*									*/
/* FILE *osd_fopen_rom( const char *filename, int *status )		*/
/*									*/
/*	ROM�C���[�W�t�@�C�� (�o�C�i��) ���J��				*/
/*									*/
/*	    ����    *filename		�t�@�C����			*/
/*		    *status		�����̌���			*/
/*									*/
/*	    �Ԓl    ����I�����́A�t�@�C���X�g���[���ւ̃|�C���^���A	*/
/*		    �ُ�I�����́ANULL ���Ԃ�B				*/
/*		    ���� status �� NULL �łȂ���΁Astatus �ɂ�		*/
/*		    �ȉ��̂����ꂩ�̒l���Z�b�g�����			*/
/*			FOPEN_OK	�t�@�C���I�[�v������		*/
/*			FOPEN_NO_ENV	�f�B���N�g��������`		*/
/*			FOPEN_NO_MEM	�������� malloc() �Ɏ��s	*/
/*			FOPEN_ERR	�t�@�C�����J���Ȃ�����		*/
/*									*/
/*									*/
/* FILE *osd_fopen_rcfile( const char *filename, int *status )		*/
/*									*/
/*	�ݒ�t�@�C�� (�e�L�X�g) ���J��					*/
/*									*/
/*	    ����    *filename		�t�@�C����			*/
/*		    *status		�����̌���			*/
/*									*/
/*	    �Ԓl    ����I�����́A�t�@�C���X�g���[���ւ̃|�C���^���A	*/
/*		    �ُ�I�����́ANULL ���Ԃ�B				*/
/*		    ���� status �� NULL �łȂ���΁Astatus �ɂ�		*/
/*		    �ȉ��̂����ꂩ�̒l���Z�b�g�����			*/
/*			FOPEN_OK	�t�@�C���I�[�v������		*/
/*			FOPEN_NO_ENV	���ϐ��̎擾�Ɏ��s		*/
/*			FOPEN_NO_MEM	�������� malloc() �Ɏ��s	*/
/*			FOPEN_ERR	�t�@�C�����J���Ȃ�����		*/
/*									*/
/************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"



/* �g�p����f�B���N�g���B��������s������ NULL �ɂȂ�B*/
static		char	*dir_cwd  = NULL;	/* cwd			*/
static	const	char	*dir_home = NULL;	/* ${HOME}		*/
static	const	char	*dir_rom  = NULL;	/* ${QUASI88_ROM_DIR}	*/
static	const	char	*dir_disk = NULL;	/* ${QUASI88_DISK_DIR}	*/



/*----------------------------------------------------------------------*/
/* �f�B���N�g���̉{��							*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/*T_DIR_INFO *osd_opendir( const char *filename )			*/
/*	�f�B���N�g���𑀍삷���ʓI�ȕ��@���Ȃ��̂ŁA			*/
/*	�Ƃ肠�����ANULL (�G���[) ��Ԃ��B				*/
/************************************************************************/
T_DIR_INFO	*osd_opendir( const char *filename )
{
  return NULL;
}

/************************************************************************/
/* void osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,	*/
/*			      int maxsize )				*/
/*	�f�B���N�g���𑀍삷���ʓI�ȕ��@���Ȃ��̂ŁA			*/
/*	�Ƃ肠�����A���̂܂܂̕������Ԃ��B				*/
/************************************************************************/
void		osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,
				      int maxsize )
{
  strncpy( realname, dir->name, maxsize -1 );	/* ���̂܂܂̕������Ԃ� */
  realname[ maxsize - 1 ] = '\0';
}


/************************************************************************/
/* T_DIR_ENTRY *osd_readdir( T_DIR_INFO *dirp )				*/
/*	�f�B���N�g���𑀍삷���ʓI�ȕ��@���Ȃ��̂ŁA			*/
/*	�Ƃ肠�����ANULL (�G���[) ��Ԃ��B				*/
/************************************************************************/
T_DIR_ENTRY	*osd_readdir( T_DIR_INFO *dirp )
{
  return NULL;
}


/************************************************************************/
/* void osd_closedir( T_DIR_INFO *dirp )				*/
/*	�f�B���N�g���𑀍삷���ʓI�ȕ��@���Ȃ��̂ŁA���̊֐��̓_�~�[	*/
/************************************************************************/
void		osd_closedir( T_DIR_INFO *dirp )
{
}







/*----------------------------------------------------------------------*/
/* �t�@�C������								*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* int osd_file_stat( const char *filename )				*/
/*	filename ���J���Ă݂Đ�������΃t�@�C���A���s����� �s���B	*/
/*	(�f�B���N�g�����ǂ�����m���ʓI�ȕ��@�͂Ȃ�)			*/
/************************************************************************/
int	osd_file_stat( const char *filename )
{
  FILE *fp;

  if( (fp = fopen( filename, "r" )) ){	/* �t�@�C���Ƃ��ĊJ��     */
    fclose( fp );			/* ����������t�@�C��	  */
    return FILE_STAT_FILE;
  }else{				/* �f�B���N�g���̔��f�͂Ȃ� */
    return FILE_STAT_ANOTHER;
  }
}



/************************************************************************/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*	�t�@�C�����������Ȃ�A�Ƃ肠���������t�@�C���Ƃ݂Ȃ��B		*/
/************************************************************************/
int	osd_file_cmp( const char *s_file, const char *t_file )
{
  if( strcmp( s_file, t_file )==0 ) return FILE_CMP_OK;
  else                              return FILE_CMP_NOT_OK;
}



/************************************************************************/
/* long	osd_file_size( const char *filename )				*/
/*	fseek �� ftell �ɂăT�C�Y�𓾂�					*/
/************************************************************************/
long	osd_file_size( const char *filename )
{
  FILE *fp;
  long size;

  if( fp = fopen( filename, "rb" ) ){
    if( fseek( fp, 0, SEEK_END )==0 ){
      size = ftell( fp );
      if( size >= 0 ) return size;
    }
  }
  return 0;	/* �T�C�Y�s���̏ꍇ�� 0 ��Ԃ��B��{�I�� -1 �͕Ԃ��Ȃ����� */
}



/*----------------------------------------------------------------------*/
/* �t�@�C�����A�p�X������						*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void osd_path_regularize( char *pathname, int max_size )		*/
/*	�t�@�C������p�X�ɂ��Ă̏����͋@��ˑ��Ȃ̂ŁA���̊֐��̓_�~�[*/
/************************************************************************/
void	osd_path_regularize( char *filename, int max_size )
{
}


/************************************************************************/
/* void	osd_path_separate( char *result_path, const char *filename, 	*/
/*			   int max_size )				*/
/*	�t�@�C������p�X�ɂ��Ă̏����͋@��ˑ��Ȃ̂ŁA		*/
/*	�Ƃ肠�����A��Ɏ��s��Ԃ��B					*/
/************************************************************************/
const char *osd_path_separate( char *result_path, const char *filename, 
			       int max_size )
{
  result_path[0] = '\0';
  return filename;
}



/************************************************************************/
/* void osd_path_connect( char *result_path, const char *parent_path,	*/
/*			  const char *child_path, int max_size )	*/
/*	�t�@�C������p�X�ɂ��Ă̏����͋@��ˑ��Ȃ̂ŁA		*/
/*	�Ƃ肠�����A��� child_path ��Ԃ�				*/
/************************************************************************/
void	osd_path_connect( char *result_path, const char *parent_path,
			  const char *child_path, int max_size )
{
  strncpy( result_path, child_path, max_size-1 );
  result_path[ max_size-1 ] = '\0';
}




/*----------------------------------------------------------------------*/
/* ���ϐ�����								*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void	osd_get_environment( void )					*/
/************************************************************************/
void	osd_get_environment( int *status )
{
  if( status ) *status = ENV_OK;

  /* ���ϐ��͂ǂ��ł����� (�g��Ȃ�) */

  dir_home = getenv( "HOME" );			/* ${HOME}		*/
  dir_rom  = getenv( "QUASI88_ROM_DIR" );	/* ${QUASI88_ROM_DIR}	*/
  dir_disk = getenv( "QUASI88_DISK_DIR" );	/* ${QUASI88_DISK_DIR}	*/
}

/************************************************************************/
/* void	osd_set_image_dir( const char *conf_dir_rom,			*/
/*			   const char *conf_dir_disk );			*/
/* �f�B���N�g���𑀍삷���ʓI�ȕ��@�Ȃ��̂ŁA���̊֐��̓_�~�[		*/
/************************************************************************/
void	osd_set_image_dir( const char *conf_dir_rom,
			   const char *conf_dir_disk )
{
}


/************************************************************************/
/* char	*osd_get_disk_dir( void )					*/
/************************************************************************/
const	char	*osd_get_disk_dir( void )
{
  return osd_get_current_dir();
}



/************************************************************************/
/* const char *osd_get_current_dir( void )				*/
/*	�˂ɁA"" ��Ԃ�						*/
/************************************************************************/
const	char	*osd_get_current_dir( void )
{
  return "";
}



/************************************************************************/
/* char *osd_get_home_filename( const char *fname )			*/
/*	��ɁAfname ��Ԃ�						*/
/************************************************************************/
char	*osd_get_home_filename( const char *fname, int *status )
{
  char *p;

  if( (p = (char *)malloc( strlen( fname ) + 1 ) ) ){

    strcpy( p, fname );
    if( status ) *status = HOME_F_OK;
    return p;

  }else{
    if( status ) *status = HOME_F_NO_MEM;
    return NULL;
  }
}


/*----------------------------------------------------------------------*/
/* �t�@�C���I�[�v��							*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* FILE *osd_fopen_image( const char *filename, int read_only,		*/
/*			  char **real_filename, int *status )		*/
/*	�w�肳�ꂽ filename �����̂܂܊J��				*/
/************************************************************************/

/* "r+b"�ŊJ���Ă݂āA���s������ "rb" �ōēx�J���Ă݂�}�N�� */
#define	FOPEN_IMAGE( fname )				\
    if( ! read_only ){					\
      if( (fp = fopen( fname, "r+b" )) != NULL ){	\
	if( status ) *status = FOPEN_OK_RW;		\
	open_success = TRUE;				\
      }							\
    }							\
    if( ! open_success ){				\
      if( (fp = fopen( fname, "rb" )) != NULL ){	\
	if( status ) *status = FOPEN_OK_RO;		\
	open_success = TRUE;				\
      }							\
    }


FILE	*osd_fopen_image( const char *filename, int read_only,
			  char **real_filename, int *status )
{
  FILE	*fp = NULL;
  int	open_success = FALSE;
  char	*fname = NULL;


  fname = (char *)malloc( strlen(filename) + 1 );
  if( fname == NULL ){
    if( status ) *status = FOPEN_NO_MEM;
    return NULL;
  }
  strcpy( fname, filename );
  FOPEN_IMAGE( filename );
  if( ! open_success ) free( fname );

	/* �t�@�C���I�[�v�����s�̏ꍇ�́ANULL ��Ԃ� */

  if( ! open_success ){
    if( status ) *status = FOPEN_ERR;
    return NULL;
  }


  *real_filename = fname;
  return fp;
}



/************************************************************************/
/* FILE *osd_fopen_rom( const char *filename, int *status )		*/
/*	�w�肳�ꂽ filename �����̂܂܊J��				*/
/************************************************************************/

FILE	*osd_fopen_rom( const char *filename, int *status )
{
  FILE	*fp;

  fp = fopen( filename, "rb" );
  if( status ){
    if( fp ) *status = FOPEN_OK;
    else     *status = FOPEN_ERR;
  }

  return fp;
}





/************************************************************************/
/* FILE *osd_fopen_rcfile( const char *filename, int *status )		*/
/*	�w�肳�ꂽ filename �����̂܂܊J��				*/
/************************************************************************/

FILE	*osd_fopen_rcfile( const char *filename, int *status )
{
  FILE	*fp;

  fp = fopen( filename, "r" );
  if( status ){
    if( fp ) *status = FOPEN_OK;
    else     *status = FOPEN_ERR;
  }

  return fp;
}
