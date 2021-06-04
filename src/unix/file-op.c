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
/* void	osd_set_image_dir( const char *new_dir_rom,			*/
/*			   const char *new_dir_disk )			*/
/*	ROM�C���[�W���J���f�B���N�g�� �� new_dir_rom �ɐݒ肷��B	*/
/*	DISK�C���[�W���J���f�B���N�g�� �� new_dir_disk �ɐݒ肷��B	*/
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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"



#ifndef	OSD_MAX_FILENAME
#define	OSD_MAX_FILENAME	(1024)
#endif

/* �p�X�̋�؂蕶���B�K��1�����ł��邱�� */
#define PATH_SEPARATORS	"/"
#define PATH_SEPARATOR	'/'


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
  T_DIR_ENTRY	*entry;
};



/* �f�B���N�g�����̃t�@�C�����̃\�[�e�B���O�Ɏg���֐� */

static int namecmp( const void *p1, const void *p2 )
{
  T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
  T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;

  return strcmp( s1->name, s2->name );
}




/************************************************************************/
/*T_DIR_INFO *osd_opendir( const char *filename )			*/
/*	opendir()�Arewinddir()�Areaddir()�Aclosedir() ����g���A	*/
/*	�f�B���N�g���̑S�ẴG���g���� �t�@�C�����Ƒ��������[�N�ɃZ�b�g	*/
/*	����B���[�N�͓K�X�Amalloc() �Ŋm�ۂ��邪�A�m�ۂɎ��s�����ꍇ��	*/
/*	�G���[ (NULL) ��Ԃ��B						*/
/*	�S�Ă̏�����A�t�@�C�������\�[�e�B���O���f�B���N�g���̃t�@�C��	*/
/*	���� '/' ��t�����Ė߂�						*/
/************************************************************************/
T_DIR_INFO	*osd_opendir( const char *filename )
{
  int	i;
  T_DIR_INFO	*dir;

  DIR	*dirp;
  struct dirent *dp;

						/* T_DIR_INFO ���[�N�m�� */
  if( (dir = (T_DIR_INFO *)malloc( sizeof(T_DIR_INFO) ))==NULL ){
    return NULL;
  }


  dirp = opendir( filename );			/* �f�B���N�g�����J�� */
  if( dirp==NULL ){
    free( dir );
    return NULL;
  }


  dir->nr_entry = 0;				/* �t�@�C�����̃`�F�b�N */
  while( readdir(dirp) ) dir->nr_entry ++;
  rewinddir( dirp );

						/* T_DIR_ENTRY ���[�N�m�� */
  dir->entry = (T_DIR_ENTRY *)malloc( dir->nr_entry * sizeof(T_DIR_ENTRY) );
  if( dir->entry==NULL ){
    closedir( dirp );
    free( dir );
    return NULL;
  }



  for( i=0; i<dir->nr_entry; i++ ) {		/* �t�@�C�����i�[���[�v */
    
    dp = readdir( dirp );			/*	�t�@�C�����擾	*/

    if( dp==NULL ){				/*	�r���Ńt�@�C�������� */
      dir->nr_entry = i;			/*	�����璆�f(��������) */
      break;
    }
						/*	�t�@�C�����R�s�[ */
    dir->entry[ i ].name = (char *)malloc( strlen(dp->d_name) +1  +1 );
    if( dir->entry[ i ].name==NULL ){	/* �����p�� +1 �]���Ɋm�� �� */
      dir->nr_entry = i;			/*	malloc �Ɏ��s        */
      break;					/*	�����璆�f(��������) */
    }
    strcpy( dir->entry[ i ].name, dp->d_name );

    {						/*	�������擾�A�ݒ� */
      char *fullname;
      struct stat sb;

	/* �f�B���N�g����(filename) �� �t�@�C����(dp->d_name) ����A
	   �t���p�X�̃t�@�C�����𐶐����Astat()�֐��ő����𓾂� */

      fullname = (char*)malloc( strlen(filename) + 1 + strlen(dp->d_name) + 1);

      if( fullname ){
	strcpy( fullname, filename );
	strcat( fullname, PATH_SEPARATORS );
	strcat( fullname, dp->d_name );

	if( stat( fullname, &sb )==0 ){
	  dir->entry[ i ].type = ((sb.st_mode&S_IFMT)==S_IFDIR )
						? DIR_ENTRY_TYPE_DIR
						: DIR_ENTRY_TYPE_NOT_DIR;
	}else{
	  /* TODO :
	     �����ŃG���[���N���闝�R�́A���� �����̓r���ł��̃t�@�C�����폜
	     ���ꂽ�A���邢�͂��̃t�@�C�����V���{���b�N�����N�ŁA���̐��
	     �t�@�C�������݂��Ȃ��A�Ȃǂł���B���̏ꍇ�́A���̃G���g�����폜
	     ����K�v������̂����A�����͎蔲�����āA�G���[�𖳎����Ă���B*/
	  dir->entry[ i ].type = DIR_ENTRY_TYPE_NOT_DIR;
	}
	free( fullname );
      }
      else{					/*	malloc���s�ł��p�� */
	dir->entry[ i ].type = DIR_ENTRY_TYPE_NOT_DIR;
      }
    }
  }


  closedir( dirp );				/* �f�B���N�g������� */


	/* �t�@�C�������\�[�g */
  qsort( dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), namecmp );

	/* �t�@�C�����ɑ��������� */
  {
    T_DIR_ENTRY *p = dir->entry;
    for( i=0; i<dir->nr_entry; i++, p++ ){
      if( p->type == DIR_ENTRY_TYPE_DIR ){	/* �f�B���N�g���ɂ� / ��t�� */
	strcat( p->name, PATH_SEPARATORS );
      }
    }
  }


  dir->cur_entry = 0;
  return dir;
}


/************************************************************************/
/* void osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,	*/
/*				      int maxsize )			*/
/*	osd_readdir �Ŏ擾���������t�̃t�@�C�������A�A�N�Z�X�\��	*/
/*	�t�@�C�����ɕϊ�����						*/
/************************************************************************/
void		osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,
				      int maxsize )
{
  strncpy( realname, dir->name, maxsize -1 );	/* ���̂܂܂̕������Ԃ� */
  realname[ maxsize - 1 ] = '\0';		/* �f�B���N�g���� / �t���� */
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

  for( i=0; i<dirp->nr_entry; i++ ) free( dirp->entry[i].name );
  free( dirp->entry );
  free( dirp );
}







/*----------------------------------------------------------------------*/
/* �t�@�C������								*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* int osd_file_stat( const char *filename )				*/
/*	filename �� �f�B���N�g���Ƃ��ĊJ���Ă݂�B��������΁A�����	*/
/*	�f�B���N�g���B���s����΁A���x�̓t�@�C���Ƃ��ĊJ���Ă݂�B	*/
/*	��������΁A����̓t�@�C���B���s����΁A����͕s���B		*/
/************************************************************************/
int	osd_file_stat( const char *filename )
{
  DIR	*dirp;
  FILE	*fp;

  if( (dirp = opendir( filename )) ){		/* �f�B���N�g���Ƃ��ĊJ�� */
    closedir( dirp );				/* ����������f�B���N�g�� */
    return FILE_STAT_DIR;
  }else{
    if( (fp = fopen( filename, "r" )) ){	/* �t�@�C���Ƃ��ĊJ��     */
      fclose( fp );				/* ����������t�@�C��	  */
      return FILE_STAT_FILE;
    }else{
      return FILE_STAT_ANOTHER;			/* �ǂ���Ƃ����s	  */
    }
  }
}



/************************************************************************/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*	stat() �֐����g�p���A��̃t�@�C���́Ast_dev �� st_ino ����v	*/
/*	����΁A�����t�@�C���Ƃ݂Ȃ��B					*/
/************************************************************************/
int	osd_file_cmp( const char *s_file, const char *t_file )
{
  struct stat	sb[2];
  int		flag = 0;

  if( stat( s_file, &sb[0] ) ){			/* s_file �̏��𓾂� */
    flag |= 0x01;
  }
  if( stat( t_file, &sb[1] ) ){			/* t_file �̏��𓾂� */
    flag |= 0x02;
  }

  if( flag ) return FILE_CMP_ERR;		/* �A�N�Z�X�G���[���߂�	*/


  if( sb[0].st_dev == sb[1].st_dev &&		/* ��r����		*/
      sb[0].st_ino == sb[1].st_ino ){
    return FILE_CMP_OK;
  }else{
    return FILE_CMP_NOT_OK;
  }
}



/************************************************************************/
/* long	osd_file_size( const char *filename )				*/
/*	stat() �֐����g�p���Ast_size ��Ԃ�				*/
/************************************************************************/
long	osd_file_size( const char *filename )
{
  struct stat	sb;

  if( stat( filename, &sb ) ){			/* file �̏��𓾂� */
    return -1;
  }else{
    return sb.st_size;				/* st_size ���t�@�C���T�C�Y */
  }
}



/*----------------------------------------------------------------------*/
/* �t�@�C�����A�p�X������						*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void osd_path_regularize( char *pathname )				*/
/************************************************************************/
void	osd_path_regularize( char *filename, int max_size )
{
  int	i, level = 0;
  char	**s;
  char	*p, *q;


  for( i=0; i<(int)strlen(filename); i++ ){	/* �p�X�̊K�w�𐔂��� */
    if( filename[i]==PATH_SEPARATOR ) level ++;
  }

  if( (s = (char **)malloc( (level+1) * sizeof(char *) )) ){

    p = q = &filename[0];
    level = 0;


		/* ./ �� ../ �� // ������ */

    while( *q ){
      if( *q++==PATH_SEPARATOR ){
	if      ( level > 0 && strncmp( p,      PATH_SEPARATORS, 1 )==0 ){
	  /* skip */
	}else if( level > 0 && strncmp( p,  "." PATH_SEPARATORS, 2 )==0 ){
	  /* skip */
	}else if( level > 0 && strncmp( p, ".." PATH_SEPARATORS, 3 )==0 ){
	  if      ( strncmp( s[level-1],      PATH_SEPARATORS, 1 )==0 ){
	    /* skip */
	  }else if( strncmp( s[level-1],  "." PATH_SEPARATORS, 2 )==0 ){
	    s[level-1] = p;
	  }else if( strncmp( s[level-1], ".." PATH_SEPARATORS, 3 )==0 ){
	    s[level++] = p;
	  }else{
	    level--;
	  }
	}else{
	  s[level++] = p;
	}
	p=q;
      }
    }

		/* �p�X���� . �� .. �� file �ŏI���Ă���ꍇ�̏��� */
		/* ( �����ւ���΁A�p�X���� / �ŏI���Ă��Ȃ��ꍇ ) */

    if      ( level > 0 && strcmp( p, "."  )==0 ){
      /* skip */
    }else if( level > 0 && strcmp( p, ".." )==0 ){
      if      ( strncmp( s[level-1],      PATH_SEPARATORS, 1 )==0 ){
	/* none */
      }else if( strncmp( s[level-1],  "." PATH_SEPARATORS, 2 )==0 ){
	s[level-1] = p;
      }else if( strncmp( s[level-1], ".." PATH_SEPARATORS, 3 )==0 ){
	s[level++] = p;
      }else{
	level--;
      }
    }else if( *p ){
      s[level++] = p;
    }


		/* �]�v�Ȃ��̂��Ȃ����p�X���Ȃ��� */

    if( level<=0 ){
      strcpy( filename, "." PATH_SEPARATORS );
    }else{
      q = &filename[0];
      for( i=0; i<level; i++ ){
	p = s[i];
	while( *p!='\0' && *p!=PATH_SEPARATOR ) *q++ = *p++;
	*q++ = *p;
      }
      if( *(q-1) ) *q = '\0';
    }

    free( s );
  }
}


/************************************************************************/
/* void	osd_path_separate( char *result_path, const char *filename, 	*/
/*			   int max_size )				*/
/*	filename �̍Ō�� '/' ���O�̕�����Ԃ�			*/
/************************************************************************/
const char *osd_path_separate( char *result_path, const char *filename, 
			       int max_size )
{
  int	pos = strlen( filename );

  if( pos==0 || pos >= max_size ){
    result_path[0] = '\0';
    return filename;
  }

  do{				/* '/' ���t�@�C�����̐K����T�� */
    if( filename[pos-1] == PATH_SEPARATOR ) break;
    pos --;
  }while( pos );

  if( pos ){			/* '/' ������������		*/
    memmove( result_path, filename, pos );	/* �擪�`'/'�܂ł��R�s�[*/
    result_path[pos] = '\0';			/* '/' ���܂܂�܂�	*/
    return &filename[pos];

  }else{			/* '/' ��������Ȃ�����	*/
    result_path[0] = '\0';
    return filename;
  }
}



/************************************************************************/
/* void osd_path_connect( char *result_path, const char *parent_path,	*/
/*			  const char *child_path, int max_size )	*/
/*	chile_path �� / �Ŏn�܂��Ă�����A���̂܂ܕԂ��B		*/
/*	�����łȂ���΁A"parent_path" + "/" + "child_path" ��Ԃ�	*/
/************************************************************************/
void	osd_path_connect( char *result_path, const char *parent_path,
			  const char *child_path, int max_size )
{
  int	len;

  if( child_path[0] == PATH_SEPARATOR ){		/* ��΃p�X */

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
static const char *parse_tilda( const char *fname );

/************************************************************************/
/* void	osd_get_environment( void )					*/
/*	dir_cwd  �c CWD���Z�b�g�B���擾�ł��Ȃ���΁ANULL		*/
/************************************************************************/
void	osd_get_environment( int *status )
{
  if( status ) *status = ENV_OK;


  dir_cwd = (char *)malloc( OSD_MAX_FILENAME );	/* CWD ���Z�b�g����	*/
  if( dir_cwd ){
    if( getcwd( dir_cwd, OSD_MAX_FILENAME-1 )==NULL ){
      free( dir_cwd );
      dir_cwd = NULL;
    }
  }


  dir_home = getenv( "HOME" );			/* ${HOME}		*/
						/* ����`�̎��́ANULL	*/
  if( dir_home==NULL ){
    if( status ) *status |= ENV_NO_HOME;
  }
  else{						/* ��`���Ă����Ă��A   */
    if( dir_home[0] == '\0' || 			/* �擪�� / �łȂ������� */
	dir_home[0] != PATH_SEPARATOR ){	/* ��̏ꍇ�́ANULL      */
      if( status ) *status |= ENV_INVALID_HOME;
      dir_home = NULL;
    }
  }



  dir_rom = getenv( "QUASI88_ROM_DIR" );	/* ${QUASI88_ROM_DIR}	*/
  if( dir_rom == NULL ){			/* ����`�̎��� ROM_DIR	*/
    dir_rom = parse_tilda( ROM_DIR );		/* (Makefile�Œ�`)	*/
    if( dir_rom == NULL ){
      if( status ) *status |= ENV_NO_MEM_ROM_DIR;
      dir_rom = ROM_DIR;
    }
  }

  dir_disk = getenv( "QUASI88_DISK_DIR" );	/* ${QUASI88_DISK_DIR}	*/
  if( dir_disk == NULL ){			/* ����`�̎��� DISK_DIR*/
    dir_disk = parse_tilda( DISK_DIR );		/* (Makefile�Œ�`)	*/
    if( dir_disk == NULL ){
      if( status ) *status |= ENV_NO_MEM_DISK_DIR;
      dir_disk = DISK_DIR;
    }
  }

}

/************************************************************************/
/* void	osd_set_image_dir( const char *new_dir_rom,			*/
/*			   const char *new_dir_disk );			*/
/*	dir_rom, dir_disk ��u������					*/
/************************************************************************/
void	osd_set_image_dir( const char *new_dir_rom,
			   const char *new_dir_disk )
{
  /*
   *	�ȉ��̃R�[�h�́Afloi ���ɂ��񋟂���܂����B
   */

  if( new_dir_rom ){
    const char *new_dir = parse_tilda( new_dir_rom );
    if( osd_file_stat( new_dir ) == FILE_STAT_DIR )
      dir_rom = new_dir;
  }

  if( new_dir_disk ){
    const char *new_dir = parse_tilda( new_dir_disk );
    if( osd_file_stat( new_dir ) == FILE_STAT_DIR )
      dir_disk = new_dir;
  }
}

/*
  fname �� ~ �Ŏn�܂��Ă�����A${HOME}�ɒu�������ĕԂ��֐�
*/

static const char *parse_tilda( const char *fname )
{
  char *p = NULL;
  int pos;

  if( fname[0]=='~' ){			/* fname�� ~ �Ŏn�܂��${HOME}�ɒu�� */
    if( dir_home ){
      p = (char *)malloc( strlen( dir_home ) + strlen( fname ) + 1 );

      if( p ){
	if( fname[1]==PATH_SEPARATOR ||
	    fname[1]=='\0' ){			/* ~/ �� ~/xxx �� ~ �̏ꍇ */
	  strcpy( p, dir_home );
	  strcat( p, PATH_SEPARATORS );
	  strcat( p, &fname[1] );
	  osd_path_regularize( p, strlen(p) );

	}else{					/* ~xxx �� ~xxx/ �̏ꍇ */

	  strcpy( p, dir_home );		    /* ${HOME} �̐K����	*/
	  pos = strlen( p ) -1;			    /* / ��T��. ������ */
	  if( pos>0 ){				    /* ������ / �͏���	*/
	    do{
	      if( p[pos-1] == PATH_SEPARATOR ) break;
	      pos --;
	    }while( pos );
	    strcpy( &p[pos], &fname[1] );	    /* ���� / �ɁA~xxx/ */
	    osd_path_regularize( p, strlen(p) );    /* �� xxx/���Ȃ�  */

	  }else{
	    free( p );
	    return fname;
	  }
	}
      }

    }else{				/* ${HOME}���Ȃ��̂Œu���ł��Ȃ� */
      return fname;
    }
  }else{				/* ~ �Ŏn�܂�Ȃ��̂ŁA�u�����Ȃ� */
    return fname;
  }

  return (const char *)p;
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

    size = strlen( dir_home ) + strlen( fname ) + 2;

    if( (p = (char *)malloc( size ) ) ){

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
/*	${QUASI88_DISK_DIR} �܂��́ADISK_DIR �Őݒ肳�ꂽ�f�B���N�g����	*/
/*	�t�@�C�� filename ���J���B���邢�͂��̂܂� filename ���J���B	*/
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
  int	i;


		/* filename �� '/' �����邩�T�� */

  for( i = strlen( filename )-1; i>=0; i-- ){
    if( filename[i] == PATH_SEPARATOR ) break;
  }

  if( i<0 ){

	/* filename �� '/' ���܂܂�Ȃ��ꍇ			*/

		/*  1) dir_disk �Ŏ����f�B���N�g���Ńt�@�C�����J��	*/
		/*	(dir_disk �̓R���p�C���� or ���ϐ��Ŋm��)	*/

    fname = (char *)malloc( strlen(dir_disk) + 1 + strlen(filename) + 1 );
    if( fname == NULL ){
      if( status ) *status = FOPEN_NO_MEM;
      return NULL;
    }
    strcpy( fname, dir_disk );		/* dir_disk + filename �ŊJ�� */
    strcat( fname, PATH_SEPARATORS );
    strcat( fname, filename );
    osd_path_regularize( fname, strlen( fname ) );

    FOPEN_IMAGE( fname );
    if( ! open_success ) free( fname );

		/*  2) �J���Ȃ�������Adir_cwd �f�B���N�g���ŊJ��	*/
		/*	(dir_cwd �� �J�����g�f�B���N�g�� or NULL )	*/

    if( ! open_success ){
      if( dir_cwd ){
	fname = (char *)malloc( strlen(dir_cwd) + 1 + strlen(filename) + 1 );
	if( fname == NULL ){
	  if( status ) *status = FOPEN_NO_MEM;
	  return NULL;
	}
	strcpy( fname, dir_cwd );	/* dir_cwd + filename �ŊJ�� */
	strcat( fname, PATH_SEPARATORS );
	strcat( fname, filename );
	osd_path_regularize( fname, strlen( fname ) );

	FOPEN_IMAGE( fname );
	if( ! open_success ) free( fname );
      }
    }

		/*  3) ����ł��J���Ȃ�������A���ڊJ��			*/

    if( ! open_success ){
      fname = (char *)malloc( strlen(filename) + 1 );
      if( fname == NULL ){
	if( status ) *status = FOPEN_NO_MEM;
	return NULL;
      }
      strcpy( fname, filename );	/* filename �ŊJ�� */
      osd_path_regularize( fname, strlen( fname ) );

      FOPEN_IMAGE( filename );
      if( ! open_success ) free( fname );
    }

  }
  else
  {
	/* filename �� '/' ���܂܂��ꍇ			*/

		/*  1) ��΃p�X�Ȃ炻�̂܂܃t�@�C�����J��		*/

    if( filename[0] == PATH_SEPARATOR ){

      fname = (char *)malloc( strlen(filename) + 1 );
      if( fname == NULL ){
	if( status ) *status = FOPEN_NO_MEM;
	return NULL;
      }
      strcpy( fname, filename );		/* filename �ŊJ�� */
      osd_path_regularize( fname, strlen( fname ) );

      FOPEN_IMAGE( fname );
      if( ! open_success ) free( fname );

    }else{

		/*  2) ���΃p�X�Ȃ�Adir_cwd �f�B���N�g���ŊJ��		*/
		/*	(dir_cwd �� �J�����g�f�B���N�g�� or NULL )	*/

      if( dir_cwd ){
	fname = (char *)malloc( strlen(dir_cwd) + 1 + strlen(filename) + 1 );
	if( fname == NULL ){
	  if( status ) *status = FOPEN_NO_MEM;
	  return NULL;
	}
	strcpy( fname, dir_cwd );		/* dir_cwd + filename �ŊJ�� */
	strcat( fname, PATH_SEPARATORS );
	strcat( fname, filename );
	osd_path_regularize( fname, strlen( fname ) );

	FOPEN_IMAGE( fname );
	if( ! open_success ) free( fname );
      }

		/*     ����ł��J���Ȃ�������A���ڊJ��			*/

      if( ! open_success ){
	fname = (char *)malloc( strlen(filename) + 1 );
	if( fname == NULL ){
	  if( status ) *status = FOPEN_NO_MEM;
	  return NULL;
	}
	strcpy( fname, filename );		/* filename �ŊJ�� */
	osd_path_regularize( fname, strlen( fname ) );

	FOPEN_IMAGE( fname );
	if( ! open_success ) free( fname );
      }

    }
  }

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
/*	${QUASI88_ROM_DIR} �܂��́AROM_DIR �Őݒ肳�ꂽ�f�B���N�g����	*/
/*	�t�@�C�� filename ���J��					*/
/************************************************************************/

FILE	*osd_fopen_rom( const char *filename, int *status )
{
  char	*fname;
  FILE	*fp;

  if( dir_rom==NULL ){			/* dir_rom ������`�Ȃ�G���[ */
    if( status ) *status = FOPEN_NO_ENV;
    return NULL;
  }

  fname = (char *)malloc( strlen(dir_rom) + 1 + strlen(filename) + 1 );
  if( fname == NULL ){
    if( status ) *status = FOPEN_NO_MEM;
    return NULL;
  }
  strcpy( fname, dir_rom );
  strcat( fname, PATH_SEPARATORS );
  strcat( fname, filename );

  fp = fopen( fname, "rb" );
  if( status ){
    if( fp ) *status = FOPEN_OK;
    else     *status = FOPEN_ERR;
  }

  free( fname );
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
