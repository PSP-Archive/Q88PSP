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
#include <direct.h>	/* getcwd, _getdrives */
#include <io.h>		/* _findfirst */

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"



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



/* �f�B���N�g�����̃t�@�C�����̃\�[�e�B���O�Ɏg���֐� */

	/* �啶���������𖳎����ăt�@�C�����\�[�g  */
static int namecmp( const void *p1, const void *p2 )
{
  T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
  T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
  return _stricmp( s1->name, s2->name );
}
	/* �啶���������𖳎����ăf�B���N�g�����\�[�g  */
static int dircmp( const void *p1, const void *p2 )
{
  T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
  T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
  if( s1->name[0]=='<' && s2->name[0]!='<' ) return -1;	  /* <�`> �̓h���C�u */
  if( s1->name[0]!='<' && s2->name[0]=='<' ) return +1;	  /* �Ȃ̂ō��D��    */
  return _stricmp( s1->name, s2->name );
}
	/* �t�@�C���ƃf�B���N�g���Ƃ𕪗� */
static int typecmp( const void *p1, const void *p2 )
{
  T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
  T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;

  if( s1->type == s2->type ) return 0;
  if( s1->type == DIR_ENTRY_TYPE_DIR ) return -1;
  else                                 return +1;
}



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
  int	i;
  T_DIR_INFO	*dir;

  long dirp;
  struct _finddata_t dp;

  int len;
  char *fname;
  long drv_list    = _getdrives();
  char drv_name[5] =  "<A:>";
  int top_dir = FALSE;

						/* T_DIR_INFO ���[�N�m�� */
  if( (dir = (T_DIR_INFO *)malloc( sizeof(T_DIR_INFO) ))==NULL ){
    return NULL;
  }

  if( strcmp( &filename[1], ":" )==0   ||	/* x: �� x:\ �̓g�b�v DIR */
      strcmp( &filename[1], ":\\" )==0 ) top_dir = TRUE;

  if( strncmp( filename, "\\\\", 2 )== 0 ){	/* \\ �Ŏn�܂�ꍇ�A	*/
    int j=0;					/* �擪���� \ ���������A*/
    len = strlen( filename ) -1;
    for( i=2; i<len; i++ ) 			/* ���̐��𐔂���B	*/
      if( filename[i]=='\\' ) j++;
    if( j==1 ) top_dir = TRUE;			/* 1�����Ȃ� �g�b�v DIR */
  }


  len = strlen( filename ) + sizeof( "\\*" );
  if( len >= OSD_MAX_FILENAME ||
      ( ( fname = (char*)malloc( len ) ) == NULL  ) ){
    free( dir );
    return NULL;
  }
  strcpy( fname, filename );
  if( strlen(fname) && fname[ strlen(fname)-1 ] != PATH_SEPARATOR )
    strcat( fname, "\\" );
  strcat( fname, "*" );

  dir->nr_entry = 0;				/* �t�@�C�����̃`�F�b�N */
  dirp = _findfirst( fname, &dp );
  if( dirp != -1 ){
    do{
      dir->nr_entry ++;
    } while( _findnext( dirp, &dp )==0 );
    _findclose( dirp );
  }



						/* T_DIR_ENTRY ���[�N�m�� */
  dir->nr_total = dir->nr_entry + 26;
  dir->entry = (T_DIR_ENTRY *)malloc( dir->nr_total * sizeof(T_DIR_ENTRY) );
  if( dir->entry==NULL ){
    free( dir );
    free( fname );
    return NULL;
  }



  dirp = -1;
  for( i=0; i<dir->nr_entry; i++ ) {		/* �t�@�C�����i�[���[�v */
    
    if( i==0 ){
      dirp = _findfirst( fname, &dp );
      if( dirp==-1 ){				/* 	�����Ȃ�t�@�C������ */
	dir->nr_entry = i;			/*	�����͒��f(��������) */
	break;
      }
    }else{
      if( _findnext( dirp, &dp )!= 0 ){		/*	�r���Ńt�@�C�������� */
	dir->nr_entry = i;			/*	�����璆�f(��������) */
	break;
      }
    }

						/*	�t�@�C�����R�s�[ */

    /* �t�@�C������ێ����镪�̃��������m�ۂ���B
       �t�@�C������ SJIS�ł���Ǝv���邪�A���ꂾ�ƃ��j���[�ł̕\������
       ������������̂� EUC�ɕϊ����Ă����B ���̏ꍇ�t�@�C�����̒�����
       �ő�2�{�ɂȂ�B�܂��A�f�B���N�g���� [ ] �ň͂�ŕ\���������̂ŁA
       ���� 2���������]���ɕK�v�B����Ċm�ۂ��郁�����̃T�C�Y�� ������ \0 ��
       �܂߂āA strlen(dp.name)*2 + 2 +1 �ƂȂ�B*/

    dir->entry[ i ].name = (char *)malloc( (strlen(dp.name))*2 +2 +1 );
    if( dir->entry[ i ].name==NULL ){
      dir->nr_entry = i;			/*	malloc �Ɏ��s        */
      break;					/*	�����璆�f(��������) */
    }
    /* strcpy( dir->entry[ i ].name, dp.name ); */
    sjis2euc( dir->entry[ i ].name, dp.name );		/* EUC�ŕێ����� */

    if (dp.attrib & _A_SUBDIR) {			/*	������ݒ� */
      dir->entry[ i ].type = DIR_ENTRY_TYPE_DIR;
    }else{
      dir->entry[ i ].type = DIR_ENTRY_TYPE_NOT_DIR;
    }
  }

  if( dirp!=-1 )
    _findclose( dirp );				/* �f�B���N�g������� */


	/* �G���g�����Ȃ�(�擾���s)�ꍇ��A���[�g�f�B���N�g���̏ꍇ�́A
	   �h���C�u���G���g���ɒǉ����Ă����悤 */

  if( dir->nr_entry==0 || top_dir ){
    for( i=0; i<26; i++ ){
      if( drv_list & (1L<<i) ){
	  
	dir->entry[ dir->nr_entry ].name = (char *)malloc( 5 );

	if( dir->entry[ dir->nr_entry ].name ){
	  strcpy( dir->entry[ dir->nr_entry ].name, drv_name );
	  dir->entry[ dir->nr_entry ].type = DIR_ENTRY_TYPE_DIR;
	  dir->nr_entry ++;
	}
      }
      drv_name[1] ++;	/* "<x:>" �� x �� A��Z�ɒu�����Ă��� */
    }
  }



	/* �t�@�C�������\�[�g �����ŕ������A�e�X���\�[�g���� */
  qsort( dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), typecmp );
  {
    T_DIR_ENTRY *p = dir->entry;
    for( i=0; i<dir->nr_entry; i++, p++ ){
      if( p->type != DIR_ENTRY_TYPE_DIR ) break;
    }
    qsort( &dir->entry[0], i, sizeof(T_DIR_ENTRY), dircmp );
    qsort( &dir->entry[i], dir->nr_entry-i, sizeof(T_DIR_ENTRY), namecmp );
  }

	/* �t�@�C�����ɑ��������� */
  {
    T_DIR_ENTRY *p = dir->entry;
    for( i=0; i<dir->nr_entry; i++, p++ ){
      if( p->type == DIR_ENTRY_TYPE_DIR ){		/* �f�B���N�g���� */
	memmove( &p->name[1], &p->name[0], strlen( p->name )+1 );
	p->name[0] = '[';				/* [ ] �ň͂�     */
	strcat( p->name, "]" );
      }
    }
  }


  dir->cur_entry = 0;
  free( fname );
  return dir;
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
  if( dir->type == DIR_ENTRY_TYPE_DIR ){	/* �f�B���N�g���̏ꍇ */
    int  len    = strlen( dir->name );
    char drv[4] = "x:\\";

    if( len==6 && dir->name[1] == '<' ){	    /* [<x:>] �̏ꍇ */

      drv[0] = dir->name[2];				/* x���h���C�uA�`Z */
      if( maxsize > 3 ) maxsize = 4;
      strncpy( realname, drv, maxsize-1 );		/* x:\ ���Z�b�g */
      realname[ maxsize - 1 ] = '\0';
      return;

    }else if( len>2 ){				    /* [�`] �̏ꍇ */

      strncpy( realname, &dir->name[1], maxsize-1 );	/* �擪�� [ ����� */
      len --;

      if( len <= maxsize -1 ){
	realname[ len-1 ] = '\\';			/* ������ ] �� \ �� */
	realname[ len ]   = '\0';			/* �u��������       */
      }else{
	realname[ maxsize - 1 ] = '\0';
      }
      euc2sjis( realname, realname );			/* SJIS�ɕϊ� */
      return;
    }
  }
						/* �t�@�C���̏ꍇ */
  strncpy( realname, dir->name, maxsize -1 );		/* ���̂܂� */
  realname[ maxsize - 1 ] = '\0';
  euc2sjis( realname, realname );			/* SJIS�ɕϊ� */

  /* EUC ���� SJIS�ɕϊ�����ꍇ�A�����񒷂��͓������A�Z���Ȃ�B
     �Ȃ̂ŁAeuc2sjis() �̈����ɓ����|�C���^���w�肵�Ă����v�Ȃ͂��c */
}


/************************************************************************/
/* T_DIR_ENTRY *osd_readdir( T_DIR_INFO *dirp )				*/
/*	osd_opendir() �̎��Ɋm�ۂ����A�G���g����񃏁[�N�ւ̃|�C���^��	*/
/*	�����A�Ԃ��Ă����B						*/
/*	���̎��̃t�@�C�����́A�\���p�ɑ�������Ă���̂Œ���		*/
/************************************************************************/
T_DIR_ENTRY	*osd_readdir( T_DIR_INFO *dirp )
{
  T_DIR_ENTRY	*ret_value = NULL;

  if( dirp->cur_entry != dirp->nr_entry ){
    ret_value = &dirp->entry[ dirp->cur_entry ];
    dirp->cur_entry ++;
  }
  return	ret_value;
}


/************************************************************************/
/* void osd_closedir( T_DIR_INFO *dirp )				*/
/*	osd_opendir() ���Ɋm�ۂ����S�Ẵ��������J������B		*/
/************************************************************************/
void		osd_closedir( T_DIR_INFO *dirp )
{
  int	i;

  for( i=0; i<dirp->nr_entry; i++ ) 
    free( dirp->entry[i].name );
  free( dirp->entry );
  free( dirp );
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
  char *fname;
  int i;
  long dirp;
  struct _finddata_t dp;

  /*
    ""        �Ȃ�G���[�A
    "\\"      �Ȃ�(���[�g)�f�B���N�g��
    "x:\\"    �Ȃ�(���[�g)�f�B���N�g���A
    "x:"      �Ȃ�f�B���N�g��
    "\\\\*"   �Ȃ�(�l�b�g���[�N)�f�B���N�g��
    "\\\\*\*" �Ȃ�(�l�b�g���[�N)�f�B���N�g��
  */

  i = strlen(filename);
  if( i==0 ) return FILE_STAT_ANOTHER;
  if( strcmp(  filename,    "\\"  )== 0 ) return FILE_STAT_DIR;
  if( strcmp( &filename[1], ":"   )== 0 ) return FILE_STAT_DIR;
  if( strcmp( &filename[1], ":\\" )== 0 ) return FILE_STAT_DIR;

  fname = (char *)malloc( i + 1 );
  if( fname==NULL ) return FILE_STAT_ANOTHER;

  strcpy( fname, filename );		/* ������ \\ �Ȃ��� */
  if( fname[i-1] == '\\' ) fname[i-1] = '\0';

  if( strncmp( fname, "\\\\", 2 )== 0 ){	/* \\ �Ŏn�܂�ꍇ�A	*/
    int j=0;					/* �擪���� \ ���������A*/
    for( i=2; fname[i]; i++ ) 			/* ���̐��𐔂���B	*/
      if( fname[i]=='\\' ) j++;
    if( j==1 ){					/* 1�����Ȃ�l�b�g	*/
      free( fname );				/* ���[�N�f�B���N�g����	*/
      return FILE_STAT_DIR;			/* �݂Ȃ���		*/
    }
  }

  dirp = _findfirst( fname, &dp );
  free( fname );

  if( dirp==-1 ){
    return FILE_STAT_ANOTHER;
  }
  _findclose( dirp );

  if (dp.attrib & _A_SUBDIR) return FILE_STAT_DIR;
  else                       return FILE_STAT_FILE;

}



/************************************************************************/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*	_stat() �̌��ʂƃt�@�C��������v����Γ����t�@�C���Ƃ݂Ȃ�	*/
/************************************************************************/
int	osd_file_cmp( const char *s_file, const char *t_file )
{
  struct _stat	sb[2];
  int		flag = 0;

  if( _stat( s_file, &sb[0] ) ){		/* s_file �̏��𓾂� */
    flag |= 0x01;
  }
  if( _stat( t_file, &sb[1] ) ){		/* t_file �̏��𓾂� */
    flag |= 0x02;
  }

  if( flag ) return FILE_CMP_ERR;		/* �A�N�Z�X�G���[���߂�	*/


  if( sb[0].st_dev   == sb[1].st_dev   &&	/* ��r����		*/
      sb[0].st_mtime == sb[1].st_mtime &&
      sb[0].st_size  == sb[1].st_size  &&
      stricmp( s_file, t_file )==0 ){
    return FILE_CMP_OK;
  }else{
    return FILE_CMP_NOT_OK;
  }
}



/************************************************************************/
/* long	osd_file_size( const char *filename )				*/
/*	_findfirst �ɂ� �t�@�C���T�C�Y�𓾂�				*/
/************************************************************************/
long	osd_file_size( const char *filename )
{
  long dirp;
  struct _finddata_t dp;

  dirp = _findfirst( filename, &dp );
  if( dirp==-1 ){			/* �t�@�C�����������c */
    return 0;
  }
  _findclose( dirp );

  if (dp.attrib & _A_SUBDIR) 		/* ����̓f�B���N�g���� */
    return -1;
  else					/* OK, you got it	*/
    return dp.size;
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
  char *absPath;

  absPath = (char *)malloc( max_size );
  if( absPath ){
    if( _fullpath( absPath, filename, max_size ) ){
      strncpy( filename, absPath, max_size -1 );
      filename[ max_size -1 ] = '\0';
    }
    free( absPath );
  }
}


/************************************************************************/
/* void	osd_path_separate( char *result_path, const char *filename, 	*/
/*			   int max_size )				*/
/*	filename �̍Ō�� '\\' ���O�̕�����Ԃ�			*/
/************************************************************************/
const char *osd_path_separate( char *result_path, const char *filename, 
			       int max_size )
{
  int	pos = strlen( filename );

  if( pos==0 || pos >= max_size ){
    result_path[0] = '\0';
    return filename;
  }

  do{				/* '\' ���t�@�C�����̐K����T�� */
    if( filename[pos-1] == PATH_SEPARATOR ) break;
    pos --;
  }while( pos );

  if( pos ){			/* '\' ������������		*/
    memmove( result_path, filename, pos );	/* �擪�`'\'�܂ł��R�s�[*/
    result_path[pos] = '\0';			/* '\' ���܂܂�܂�	*/
    return &filename[pos];

  }else{			/* '\' ��������Ȃ�����	*/
    result_path[0] = '\0';
    return filename;
  }
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
  int	len = strlen( child_path );

  if( child_path[0] == PATH_SEPARATOR   ||		/* ��΃p�X */
      strncmp( &child_path[1], ":\\", 2 ) == 0 ){

    strncpy( result_path, child_path, max_size-1 );
    result_path[ max_size-1 ] = '\0';

  }else{						/* ���΃p�X */

    strncpy( result_path, parent_path, max_size-1 );
    result_path[ max_size-1 ] = '\0';

    len = strlen( result_path );
    if( len  &&  result_path[ len-1 ] != PATH_SEPARATOR ){
      strncat( result_path, PATH_SEPARATORS, max_size - len -1 );
    }

    len = strlen( result_path );
    strncat( result_path, child_path, max_size - len -1 );

  }
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
  char *p;

  if( status ) *status = ENV_OK;


  dir_cwd = (char *)malloc( OSD_MAX_FILENAME );	/* CWD ���Z�b�g����	*/
  if( dir_cwd ){
    if( _getcwd( dir_cwd, OSD_MAX_FILENAME )==NULL ){
      free( dir_cwd );
      dir_cwd = NULL;
    }else{
      osd_path_regularize( dir_cwd, OSD_MAX_FILENAME );
    }
  }

  /* ���ϐ� */

  dir_home = getenv( "QUASI88_HOME" );		/* ${HOME}		*/
  if( dir_home==NULL ){
    if( dir_cwd ) dir_home = dir_cwd;
  }

  dir_rom  = getenv( "QUASI88_ROM_DIR" );	/* ${QUASI88_ROM_DIR}	*/


  dir_disk = getenv( "QUASI88_DISK_DIR" );	/* ${QUASI88_DISK_DIR}	*/
  if( dir_disk ){
    p = (char *)malloc( OSD_MAX_FILENAME );
    if( p ){
      strncpy( p, dir_disk, OSD_MAX_FILENAME-1 );
      p[ OSD_MAX_FILENAME-1 ] = '\0';
      osd_path_regularize( p, OSD_MAX_FILENAME );
      dir_disk = p;
    }
  }

/*
  printf("%s\n", dir_cwd  ? dir_cwd  : "NULL" );
  printf("%s\n", dir_home ? dir_home : "NULL" );
  printf("%s\n", dir_rom  ? dir_rom  : "NULL" );
  printf("%s\n", dir_disk ? dir_disk : "NULL" );
*/
}



/************************************************************************/
/* void	osd_set_image_dir( const char *conf_dir_rom,			*/
/*			   const char *conf_dir_disk );			*/
/************************************************************************/
void	osd_set_image_dir( const char *conf_dir_rom,
			   const char *conf_dir_disk )
{
  if( conf_dir_rom ){
    dir_rom = conf_dir_rom;
  }

  if( conf_dir_disk ){
    char *p = (char *)malloc( OSD_MAX_FILENAME );
    if( p ){
      strncpy( p, conf_dir_disk, OSD_MAX_FILENAME-1 );
      p[ OSD_MAX_FILENAME-1 ] = '\0';
      osd_path_regularize( p, OSD_MAX_FILENAME );
      dir_disk = p;
    }
    /* ���܂����A�ȑO�� dir_disk �� free() ���Ă��Ȃ��c�c */
  }
}


/************************************************************************/
/* char	*osd_get_disk_dir( void )					*/
/************************************************************************/
const	char	*osd_get_disk_dir( void )
{
  if( dir_disk ) return dir_disk;
  else           return osd_get_current_dir();
}



/************************************************************************/
/* const char *osd_get_current_dir( void )				*/
/************************************************************************/
const	char	*osd_get_current_dir( void )
{
  if( dir_cwd ) return dir_cwd;
  else          return ".";
}



/************************************************************************/
/* char *osd_get_home_filename( const char *fname )			*/
/*	"${HOME}" + "\" + "fname" ��Ԃ�				*/
/*	�ł��Ȃ���΁ANULL						*/
/************************************************************************/
char	*osd_get_home_filename( const char *fname, int *status )
{
  char *p;
  int size;

  if( dir_home == NULL ){
    if( status ) *status = HOME_F_NO_HOME;
    return NULL;
  }else{

    size = strlen( dir_home ) + 1 + strlen( fname ) + 1;
    if( size < OSD_MAX_FILENAME ) size = OSD_MAX_FILENAME;

    if( (p = (char *)malloc( size )) ){

      strcpy( p, dir_home );
      strcat( p, PATH_SEPARATORS );
      strcat( p, fname );
      osd_path_regularize( p, size );

      if( status ) *status = HOME_F_OK;
      return p;

    }else{
      if( status ) *status = HOME_F_NO_MEM;
      return NULL;
    }
  }
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

  const char  *dirname = osd_get_disk_dir();

  int size = strlen( dirname ) + 1 + strlen(filename) + 1;
  if( size < OSD_MAX_FILENAME ) size = OSD_MAX_FILENAME;

  fname = (char *)malloc( size );
  if( fname == NULL ){
    if( status ) *status = FOPEN_NO_MEM;
    return NULL;
  }

  /*
    strcpy( fname, dirname );
    strcat( fname, PATH_SEPARATORS );
    strcat( fname, filename );
    ���Ղɂ������ƁAfilename����΃p�X�̎��ɂ͂܂�̂Œ���
  */
  osd_path_connect( fname, dirname, filename, size );
  osd_path_regularize( fname, size );

  FOPEN_IMAGE( fname );
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
/*	${QUASI88_ROM_DIR} �Őݒ肳�ꂽ�f�B���N�g���Ńt�@�C����ǂݍ���	*/
/************************************************************************/

FILE	*osd_fopen_rom( const char *filename, int *status )
{
  char	*fname;
  FILE	*fp;

  if( dir_rom==NULL ){

    fp = fopen( filename, "rb" );

  }else{

    fname = (char *)malloc( strlen(dir_rom) + 1 + strlen(filename) + 1 );
    if( fname == NULL ){
      if( status ) *status = FOPEN_NO_MEM;
      return NULL;
    }
    strcpy( fname, dir_rom );
    strcat( fname, PATH_SEPARATORS );
    strcat( fname, filename );

    fp = fopen( fname, "rb" );

    free( fname );
  }

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
  FILE	*fp;
  char *fname = osd_get_home_filename( filename, status );

  if( fname==NULL ){
    return NULL;
  }

  fp = fopen( fname, "r" );
  if( status ){
    if( fp ) *status = FOPEN_OK;
    else     *status = FOPEN_ERR;
  }

  free(fname);
  return fp;
}
