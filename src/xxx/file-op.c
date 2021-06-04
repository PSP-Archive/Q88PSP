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
#include <stdlib.h>	/* fullpath */
#include <string.h>
#include <sys/stat.h>	/* _stat */

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"

#include "exmem.h"

#include "menu.h"


/* �p�X�̋�؂蕶���B�K��1�����ł��邱�� */
#define PATH_SEPARATORS	"\\"
#define PATH_SEPARATOR	'\\'


/* �g�p����f�B���N�g���B��������s������ NULL �ɂȂ�B*/
static		char	*dir_cwd  = NULL;	/* cwd			*/
static	const	char	*dir_home = NULL;	/* ${HOME}		*/
static	const	char	*dir_rom  = NULL;	/* ${QUASI88_ROM_DIR}	*/
static	const	char	*dir_disk = NULL;	/* ${QUASI88_DISK_DIR}	*/



/*----------------------------------------------------------------------*/
/* �f�B���N�g���̉{��							*/
/*----------------------------------------------------------------------*/

struct	_T_DIR_INFO
{
  int		cur_entry;
  int		nr_entry;
  int		nr_total;
  T_DIR_ENTRY	*entry;
};


/************************************************************************/
/*T_DIR_INFO *osd_opendir( const char *filename )			*/
/*	_findfirst(), _findnext(), _findclose() ����g���A		*/
/*	�f�B���N�g���̑S�ẴG���g���� �t�@�C�����Ƒ��������[�N�ɃZ�b�g	*/
/*	����B���[�N�͓K�X�Amalloc() �Ŋm�ۂ��邪�A�m�ۂɎ��s�����ꍇ��	*/
/*	�G���[ (NULL) ��Ԃ��B						*/
/*	�S�Ă̏�����A�t�@�C�������\�[�e�B���O���f�B���N�g���̃t�@�C��	*/
/*	���� '[' �� ']' ��t�����Ė߂�					*/
/************************************************************************/
T_DIR_INFO	*osd_opendir( const char *filename )
{
 return NULL;
}

/************************************************************************/
/* void osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,	*/
/*			      int maxsize )				*/
/*	osd_readdir �Ŏ擾���������t�̃t�@�C�������A�A�N�Z�X�\��	*/
/*	�t�@�C�����ɕϊ�����						*/
/*	�v�́A�f�B���N�g���Ȃ�O��� [] ������� \\ ��t������		*/
/************************************************************************/
void		osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,
				      int maxsize )
{
}


/************************************************************************/
/* T_DIR_ENTRY *osd_readdir( T_DIR_INFO *dirp )				*/
/*	osd_opendir() �̎��Ɋm�ۂ����A�G���g����񃏁[�N�ւ̃|�C���^��	*/
/*	�����A�Ԃ��Ă����B						*/
/*	���̎��̃t�@�C�����́A�\���p�ɑ�������Ă���̂Œ���		*/
/************************************************************************/
T_DIR_ENTRY	*osd_readdir( T_DIR_INFO *dirp )
{
  return	NULL;
}


/************************************************************************/
/* void osd_closedir( T_DIR_INFO *dirp )				*/
/*	osd_opendir() ���Ɋm�ۂ����S�Ẵ��������J������B		*/
/************************************************************************/
void		osd_closedir( T_DIR_INFO *dirp )
{
}

/*----------------------------------------------------------------------*/
/* �t�@�C������								*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* int osd_file_stat( const char *filename )				*/
/*	_findfirst() �ɂ� filename �̑������`�F�b�N			*/
/************************************************************************/
int	osd_file_stat( const char *filename )
{
}

/************************************************************************/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*	_stat() �̌��ʂƃt�@�C��������v����Γ����t�@�C���Ƃ݂Ȃ�	*/
/************************************************************************/
int	osd_file_cmp( const char *s_file, const char *t_file )
{
	//�Ƃ肠�����s��v�Ƃ��ĕԂ�������Ă܂���
//	return FILE_CMP_NOT_OK;
//	if( strcmp( s_file, t_file ) == 0 ) return FILE_CMP_OK;
	if( strcmp( Drive1, Drive2 ) == 0 ) return FILE_CMP_OK;
	else                              return FILE_CMP_NOT_OK;
}



/************************************************************************/
/* long	osd_file_size( const char *filename )				*/
/*	_findfirst �ɂ� �t�@�C���T�C�Y�𓾂�				*/
/************************************************************************/
long	osd_file_size( const char *filename )
{
	long fsize;
	FILE *fp;

	fp = fopen( filename, "rb" );
	if (!fp) return 0;

	fseek(fp,0,SEEK_END);
	fsize = ftell(fp);
	
	fclose(fp);	
	return fsize;
}



/*----------------------------------------------------------------------*/
/* �t�@�C�����A�p�X������						*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void osd_path_regularize( char *pathname, int max_size )		*/
/*	_fullpath�̌��ʂ����̂܂ܕԂ��B   				*/
/*	_fullpath�̌��ʂ� NULL �Ȃ�Apathname �����̂܂ܕԂ�		*/
/*	�f�B���N�g���̏ꍇ�A���͌��̖����� \ �Ȃ猋�ʂ� \ �������A	*/
/*	���͌��̖����� \ �łȂ��Ȃ�A���ʂ� \ �͂��Ȃ��B		*/
/************************************************************************/
void	osd_path_regularize( char *filename, int max_size )
{
}


/************************************************************************/
/* void	osd_path_separate( char *result_path, const char *filename, 	*/
/*			   int max_size )				*/
/*	filename �̍Ō�� '\\' ���O�̕�����Ԃ�			*/
/************************************************************************/
const char *osd_path_separate( char *result_path, const char *filename, 
			       int max_size )
{
	return NULL;
}



/************************************************************************/
/* void osd_path_connect( char *result_path, const char *parent_path,	*/
/*			  const char *child_path, int max_size )	*/
/*	chile_path �� \ �Ŏn�܂��Ă�����A���̂܂ܕԂ��B		*/
/*	chile_path �� x:\ �̏ꍇ���A���̂܂ܕԂ��B			*/
/*	�����łȂ���΁A"parent_path" + "/" + "child_path" ��Ԃ�	*/
/************************************************************************/
void	osd_path_connect( char *result_path, const char *parent_path,
			  const char *child_path, int max_size )
{
}




/*----------------------------------------------------------------------*/
/* ���ϐ�����								*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void	osd_get_environment( void )					*/
/*	dir_cwd  �c CWD���Z�b�g�B���擾�ł��Ȃ���΁ANULL		*/
/************************************************************************/
void	osd_get_environment( int *status )
{
}



/************************************************************************/
/* void	osd_set_image_dir( const char *conf_dir_rom,			*/
/*			   const char *conf_dir_disk );			*/
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
	return NULL;
}



/************************************************************************/
/* const char *osd_get_current_dir( void )				*/
/************************************************************************/
const	char	*osd_get_current_dir( void )
{
         return ".";
}



/************************************************************************/
/* char *osd_get_home_filename( const char *fname )			*/
/*	"${HOME}" + "\" + "fname" ��Ԃ�				*/
/*	�ł��Ȃ���΁ANULL						*/
/************************************************************************/
char	*osd_get_home_filename( const char *fname, int *status )
{
 return NULL;
}


/*----------------------------------------------------------------------*/
/* �t�@�C���I�[�v��							*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* FILE *osd_fopen_image( const char *filename, int read_only,		*/
/*			  char **real_filename, int *status )		*/
/*	${QUASI88_DISK_DIR} �Őݒ肳�ꂽ�f�B���N�g����filename ���J��	*/
/*	filename ����΃p�X�Ȃ�A��������J��				*/
/************************************************************************/


FILE	*osd_fopen_image( const char *filename, int read_only,
			  char **real_filename, int *status )
{
	FILE *fp;
//	fp = ex_fopen( filename, "rb" );
	fp = ex_fopen( filename, "w" );

  if( status ){
    if( fp ) *status = FOPEN_OK;
    else     
		fp = ex_fopen( filename, "rb" );
	    if( fp ) *status = FOPEN_OK_RO;
			     *status = FOPEN_ERR;
  }
  return fp;
}



/************************************************************************/
/* FILE *osd_fopen_rom( const char *filename, int *status )		*/
/*	${QUASI88_ROM_DIR} �Őݒ肳�ꂽ�f�B���N�g���Ńt�@�C����ǂݍ���	*/
/************************************************************************/

FILE	*osd_fopen_rom( const char *filename, int *status )
{
	FILE *fp;
  fp = ex_fopen( filename, "rb" );
  if( status ){
    if( fp ) *status = FOPEN_OK;
    else     *status = FOPEN_ERR;
  }
  return fp;
}





/************************************************************************/
/* FILE *osd_fopen_rcfile( const char *filename, int *status )		*/
/*	${HOME} �ɂāAfilename ���J��					*/
/************************************************************************/

FILE	*osd_fopen_rcfile( const char *filename, int *status )
{
  return NULL;
}
