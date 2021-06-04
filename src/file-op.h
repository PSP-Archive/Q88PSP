#ifndef FILE_OP_H_INCLUDED
#define FILE_OP_H_INCLUDED



/*----------------------------------------------------------------------*/
/* ディレクトリの閲覧							*/
/*----------------------------------------------------------------------*/

/* ディレクトリ情報構造体 */

typedef	struct	_T_DIR_INFO	T_DIR_INFO;


/* ディレクトリエントリ構造体 */

typedef	struct {
  int	 type; 		/* ファイルの種類 (下参照)	*/
  char	*name;		/* ファイル名			*/
} T_DIR_ENTRY;

#define	DIR_ENTRY_TYPE_DIR	(0)
#define	DIR_ENTRY_TYPE_NOT_DIR	(1)


T_DIR_INFO	*osd_opendir( const char *filename );
T_DIR_ENTRY	*osd_readdir( T_DIR_INFO *dirp );
void		osd_closedir( T_DIR_INFO *dirp );
void		osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,
				      int maxsize );


/*----------------------------------------------------------------------*/
/* ファイル属性								*/
/*----------------------------------------------------------------------*/
#define	FILE_STAT_DIR		(0)
#define	FILE_STAT_FILE		(1)
#define	FILE_STAT_ANOTHER	(2)
int	osd_file_stat( const char *filename );

#define	FILE_CMP_OK		(0)
#define	FILE_CMP_NOT_OK		(1)
#define	FILE_CMP_ERR		(2)
int	osd_file_cmp( const char *s_file, const char *t_file );

long	osd_file_size( const char *filename );

/*----------------------------------------------------------------------*/
/* ファイル名、パス名処理						*/
/*----------------------------------------------------------------------*/
void	osd_path_regularize( char *filename, int max_size );
const char *osd_path_separate( char *result_path, const char *filename, 
			       int max_size );
void	osd_path_connect( char *result_path, const char *parent_path,
			  const char *child_path, int max_size );

/*----------------------------------------------------------------------*/
/* 環境変数処理								*/
/*----------------------------------------------------------------------*/
#define	ENV_OK			(0)
#define	ENV_NO_HOME		(1)
#define	ENV_INVALID_HOME	(2)
#define	ENV_NO_MEM_ROM_DIR	(4)
#define	ENV_NO_MEM_DISK_DIR	(8)
void		osd_get_environment( int *status );
void		osd_set_image_dir( const char *new_dir_rom,
				   const char *new_dir_disk );

const	char	*osd_get_disk_dir( void );
const	char	*osd_get_current_dir( void );


#define	HOME_F_OK	(0)
#define	HOME_F_NO_HOME	(1)
#define	HOME_F_NO_MEM	(2)
char	*osd_get_home_filename( const char *fname, int *status );

/*----------------------------------------------------------------------*/
/* ファイルオープン							*/
/*----------------------------------------------------------------------*/
#define	FOPEN_OK		(0)
#define	FOPEN_OK_RW		(0)
#define	FOPEN_OK_RO		(1)
#define	FOPEN_ERR		(-1)
#define	FOPEN_NO_MEM		(-2)
#define	FOPEN_NO_ENV		(-3)

FILE	*osd_fopen_image( const char *filename, int read_only,
			  char **real_filename, int *status );
FILE	*osd_fopen_rom( const char *filename, int *status );
FILE	*osd_fopen_rcfile( const char *filename, int *status );


/*----------------------------------------------------------------------*/
/* おまけ	(関数は、q8tk-core.c にて定義してある)			*/
/*----------------------------------------------------------------------*/
void	sjis2euc( char *euc_p, const char *sjis_p );
void	euc2sjis( char *sjis_p, const char *euc_p );

#endif	/* FILE_OP_H_INCLUDED */
