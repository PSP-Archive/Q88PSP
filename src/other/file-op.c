/************************************************************************/
/*									*/
/* ファイル処理、ファイル名操作関数 (OS依存)				*/
/*									*/
/* 【関数】								*/
/*									*/
/*	《ディレクトリの閲覧》						*/
/*									*/
/* T_DIR_INFO *osd_opendir( const char *filename )			*/
/*									*/
/*	filename で指定されたディレクトリを開き、その情報ワークへの	*/
/*	ポインタを返す。この情報ワークへのポインタは osd_readdir,	*/
/*	osd_closedir にて使用し、呼び出し側が直接参照することはない。	*/
/*									*/
/*	    引数    *filename ディレクトリ名				*/
/*	    返値    正常終了時、ディレクトリ情報ワークへのポインタ。	*/
/*		    異常終了時、NULL (ディレクトリ開けず/他のエラー等)	*/
/*									*/
/*									*/
/* const T_DIR_ENTRY *osd_readdir( T_DIR_INFO *dirp )			*/
/*									*/
/*	osd_opendir にて開いたディレクトリからエントリを一つ読みとり	*/
/*	内容を T_DIR_ENTRY型のワークにセットし、そのポインタを返す。	*/
/*									*/
/*		T_DIR_ENTRY型						*/
/*		    typedef struct {					*/
/*			int    type;	… ファイルの属性		*/
/*			char  *name;	… ファイル名			*/
/*		    } T_DIR_ENTRY;					*/
/*									*/
/*		ファイル属性は、以下の値をとる。			*/
/*		    DIR_ENTRY_TYPE_DIR		… ディレクトリ		*/
/*		    DIR_ENTRY_TYPE_NOT_DIR	… ファイル(または不明)	*/
/*									*/
/*		ここで得られるファイル名は、ディレクトリ閲覧時の表示に	*/
/*		使用する。なので、実際のファイル名と同じである必要は	*/
/*		ない。(例えばディレクトリ名は <> で囲むなどの装飾も可)	*/
/*		また、取得した順にファイル名の表示を行うので、予め	*/
/*		ソートしておくと表示の際に見やすくなる。		*/
/*									*/
/*	    引数    *dirp	ディレクトリ情報ワークへのポインタ	*/
/*				( osd_opendir() の返り値) 		*/
/*	    返値    正常終了時、T_DIR_ENTRY 構造体へのポインタ		*/
/*	            これ以上エントリが無いないしエラー時、NULL		*/
/*									*/
/*									*/
/* void osd_closedir( T_DIR_INFO *dirp )				*/
/*									*/
/*	ディレクトリを閉じる						*/
/*									*/
/*	    引数    *dirp	ディレクトリ情報ワークへのポインタ	*/
/*				( osd_opendir() の返り値) 		*/
/*									*/
/*									*/
/* void osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,	*/
/*			      int maxsize )				*/
/*									*/
/*	osd_readdir にて取得したファイル名を、アクセス可能なファイル名	*/
/*	変換する。例えば、osd_readdir の時に "<dirname>" というファイル	*/
/*	名を取得した場合、この関数でもって "dirname/" に変換する。	*/
/*									*/
/*	    引数    *realname	アクセス可能なファイル名の返るワーク	*/
/*		    *dir	ディレクトリエントリ			*/
/*				( osd_opendir() の返り値) 		*/
/*		    max_size	realname の長さ				*/
/*									*/
/*									*/
/*	《ファイル属性》						*/
/*									*/
/* int osd_file_stat( const char *filename )				*/
/*									*/
/*	ファイルか、ディレクトリか、不明かを判別する。			*/
/*									*/
/*	    引数    *filename		ファイル名			*/
/*	    返値    FILE_STAT_DIR	ディレクトリ			*/
/*		    FILE_STAT_FILE	ファイル			*/
/*		    FILE_STAT_ANOTHER	それ以外(存在しないも含む)	*/
/*									*/
/*									*/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*									*/
/*	2つのファイルが、同じものかどうかチェックする。			*/
/*									*/
/*	    引数    *s_file		ファイル名 1			*/
/*		    *t_file		ファイル名 2			*/
/*	    返値    FILE_CMP_OK		一致				*/
/*		    FILE_CMP_NOT_OK	不一致				*/
/*		    FILE_CMP_ERR	ファイルアクセスエラー		*/
/*									*/
/*									*/
/* long	osd_file_size( const char *filename )				*/
/*									*/
/*	ファイルのサイズ(バイト数)を返す。				*/
/*	ファイルサイズが不明の場合、0 を返しても差し支えない		*/
/*									*/
/*	    引数    *filename	ファイル名				*/
/*	    返値    -1		アクセスエラー				*/
/*		    0～		ファイルサイズ				*/
/*									*/
/*									*/
/*	《ファイル名、パス名処理》					*/
/*									*/
/* void osd_path_regularize( char *filename, int max_size )		*/
/*									*/
/*	パス名を正規化(?)する。						*/
/*	例) "dir1/./dir2//dir3/../../file" は "dir1/file" に変換される	*/
/*									*/
/*	    引数    *filename	ファイル名 またはディレクトリ名		*/
/*				結果は、引数自体に上書きされる。	*/
/*		    max_size	filename の長さ				*/
/*									*/
/*									*/
/* const char *osd_path_separate( char *result_path, 			*/
/*				  const char *filename,int max_size )	*/
/*									*/
/*	filename のディレクトリ部分を result_path にセットする。	*/
/*	結果は、result_path に、最大 max_size バイト 格納される		*/
/*	filename が max_file よりも大きい場合や、filenameに '/' が	*/
/*	含まれない場合、result_path には 空文字列 ("") がセットされる。	*/
/*	返値として、可能ならば filename のファイル名部分の先頭のアド	*/
/*	レスが戻る。それが不可能なら NULL が返る			*/
/*	例) filename   が "/local/tmp/filename" の場合、		*/
/*	 resule_path には "/local/tmp/"         がセットされ、		*/
/*	 返値は、filename の 'f' を指すポインタとなる。			*/
/*									*/
/*	    引数    *result_path	結果の返るワーク		*/
/*		    *filename		ファイル名			*/
/*		    max_size		result_path の長さ		*/
/*	    返値    filenameの、ファイル名部分の先頭を指すポインタ、	*/
/*		    ないしは、NULL					*/
/*									*/
/*									*/
/* void osd_path_connect( char *result_path, const char *parent_path,	*/
/*			   const char *child_path, int max_size )	*/
/*									*/
/*	parent_path に、child_path をつなげて、一つのパスにする。	*/
/*	ただし、child_path が絶対パス ('/' で始まる) の場合は、		*/
/*	child_path そのものを返す。					*/
/*	結果は、result_path に、最大 max_size バイト 格納される		*/
/*									*/
/*	    引数    *result_path	結果の返るワーク		*/
/*		    *parent_path	親のパス名			*/
/*		    *child_path		子のパス名			*/
/*		    max_size		result_path の長さ		*/
/*									*/
/*									*/
/*	《環境変数処理》						*/
/*									*/
/* void	osd_get_environment( int *status )				*/
/*	環境変数を取得し、内部ワークを初期化する。			*/
/*	state が NULL でなければ、エラー情報をセットする		*/
/*									*/
/* void	osd_set_image_dir( const char *conf_dir_rom,			*/
/*			   const char *conf_dir_disk )			*/
/*	ROMイメージを開くディレクトリ を conf_dir_rom に設定する。	*/
/*	DISKイメージを開くディレクトリ を conf_dir_disk に設定する。	*/
/*	ただし、引数が NULL ならば設定しない。				*/
/*									*/
/* const char *osd_get_disk_dir( void )					*/
/*	ディスクイメージを開くデフォルトのディレクトリ(dir_disk)を返す。*/
/*	未定義の場合はカレントディレクトリ(osd_get_current_dir)が返る。	*/
/*									*/
/* const char *osd_get_current_dir( void )				*/
/*	カレントディレクトリを返す。カレントディレクトリが取得できない	*/
/*	場合は、"." などのといった代わりの文字列が返る。		*/
/*									*/
/* char *osd_get_home_filename( const char *fname, int *status )	*/
/*	ファイル名 ${HOME}/fname を返す。				*/
/*	返値は malloc された領域なので、呼出元で free すること。	*/
/*									*/
/*	    返値    正常終了時、ファイル名が、異常終了時 NULL が返る。	*/
/*		    引数 status が NULL でなければ、status には		*/
/*		    以下のいずれかの値がセットされる			*/
/*			HOME_F_OK	正常終了			*/
/*			HOME_F_NO_HOME	環境変数 HOME が未設定		*/
/*			HOME_F_NO_MEM	malloc に失敗			*/
/*									*/
/*									*/
/*	《ファイルオープン》						*/
/*									*/
/* FILE *osd_fopen_image( const char *filename, int read_only,		*/
/*			  char **real_filename, int *status )		*/
/*									*/
/*	D88イメージファイル (バイナリ) を開く。				*/
/*	引数 read_only が真の時は "rb" で、偽の時は "r+b" で開く。	*/
/*	( ただし、"r+b"で開こうとして失敗した場合、"rb" で開く )	*/
/*									*/
/*	    引数    *filename		ファイル名			*/
/*		    read_only		真の時、READ ONLY で開く	*/
/*		    **real_filename	実際に開いたファイルの名前	*/
/*		    *status		処理の結果			*/
/*									*/
/*	    返値    正常終了時は、ファイルストリームへのポインタが、	*/
/*		    異常終了時は、NULL が返る。				*/
/*		    引数 status が NULL でなければ、status には		*/
/*		    以下のいずれかの値がセットされる			*/
/*			FOPEN_OK_RW	"r+b" でファイルオープン成功	*/
/*			FOPEN_OK_RO	"rb"  でファイルオープン成功	*/
/*			FOPEN_NO_MEM	処理時の malloc() に失敗	*/
/*			FOPEN_ERR	ファイルが開けなかった		*/
/*		    正常終了時には、real_filename に開いたファイル名が	*/
/*		    セットされる。これは malloc()された領域なので、	*/
/*		    呼出側で適宜開放する。				*/
/*									*/
/*									*/
/* FILE *osd_fopen_rom( const char *filename, int *status )		*/
/*									*/
/*	ROMイメージファイル (バイナリ) を開く				*/
/*									*/
/*	    引数    *filename		ファイル名			*/
/*		    *status		処理の結果			*/
/*									*/
/*	    返値    正常終了時は、ファイルストリームへのポインタが、	*/
/*		    異常終了時は、NULL が返る。				*/
/*		    引数 status が NULL でなければ、status には		*/
/*		    以下のいずれかの値がセットされる			*/
/*			FOPEN_OK	ファイルオープン成功		*/
/*			FOPEN_NO_ENV	ディレクトリが未定義		*/
/*			FOPEN_NO_MEM	処理時の malloc() に失敗	*/
/*			FOPEN_ERR	ファイルが開けなかった		*/
/*									*/
/*									*/
/* FILE *osd_fopen_rcfile( const char *filename, int *status )		*/
/*									*/
/*	設定ファイル (テキスト) を開く					*/
/*									*/
/*	    引数    *filename		ファイル名			*/
/*		    *status		処理の結果			*/
/*									*/
/*	    返値    正常終了時は、ファイルストリームへのポインタが、	*/
/*		    異常終了時は、NULL が返る。				*/
/*		    引数 status が NULL でなければ、status には		*/
/*		    以下のいずれかの値がセットされる			*/
/*			FOPEN_OK	ファイルオープン成功		*/
/*			FOPEN_NO_ENV	環境変数の取得に失敗		*/
/*			FOPEN_NO_MEM	処理時の malloc() に失敗	*/
/*			FOPEN_ERR	ファイルが開けなかった		*/
/*									*/
/************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"



/* 使用するディレクトリ。いずれも不明時は NULL になる。*/
static		char	*dir_cwd  = NULL;	/* cwd			*/
static	const	char	*dir_home = NULL;	/* ${HOME}		*/
static	const	char	*dir_rom  = NULL;	/* ${QUASI88_ROM_DIR}	*/
static	const	char	*dir_disk = NULL;	/* ${QUASI88_DISK_DIR}	*/



/*----------------------------------------------------------------------*/
/* ディレクトリの閲覧							*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/*T_DIR_INFO *osd_opendir( const char *filename )			*/
/*	ディレクトリを操作する一般的な方法がないので、			*/
/*	とりあえず、NULL (エラー) を返す。				*/
/************************************************************************/
T_DIR_INFO	*osd_opendir( const char *filename )
{
  return NULL;
}

/************************************************************************/
/* void osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,	*/
/*			      int maxsize )				*/
/*	ディレクトリを操作する一般的な方法がないので、			*/
/*	とりあえず、そのままの文字列を返す。				*/
/************************************************************************/
void		osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,
				      int maxsize )
{
  strncpy( realname, dir->name, maxsize -1 );	/* そのままの文字列を返す */
  realname[ maxsize - 1 ] = '\0';
}


/************************************************************************/
/* T_DIR_ENTRY *osd_readdir( T_DIR_INFO *dirp )				*/
/*	ディレクトリを操作する一般的な方法がないので、			*/
/*	とりあえず、NULL (エラー) を返す。				*/
/************************************************************************/
T_DIR_ENTRY	*osd_readdir( T_DIR_INFO *dirp )
{
  return NULL;
}


/************************************************************************/
/* void osd_closedir( T_DIR_INFO *dirp )				*/
/*	ディレクトリを操作する一般的な方法がないので、この関数はダミー	*/
/************************************************************************/
void		osd_closedir( T_DIR_INFO *dirp )
{
}







/*----------------------------------------------------------------------*/
/* ファイル属性								*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* int osd_file_stat( const char *filename )				*/
/*	filename を開いてみて成功すればファイル、失敗すれば 不明。	*/
/*	(ディレクトリかどうかを知る一般的な方法はない)			*/
/************************************************************************/
int	osd_file_stat( const char *filename )
{
  FILE *fp;

  if( (fp = fopen( filename, "r" )) ){	/* ファイルとして開く     */
    fclose( fp );			/* 成功したらファイル	  */
    return FILE_STAT_FILE;
  }else{				/* ディレクトリの判断はなし */
    return FILE_STAT_ANOTHER;
  }
}



/************************************************************************/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*	ファイル名が同じなら、とりあえず同じファイルとみなす。		*/
/************************************************************************/
int	osd_file_cmp( const char *s_file, const char *t_file )
{
  if( strcmp( s_file, t_file )==0 ) return FILE_CMP_OK;
  else                              return FILE_CMP_NOT_OK;
}



/************************************************************************/
/* long	osd_file_size( const char *filename )				*/
/*	fseek と ftell にてサイズを得る					*/
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
  return 0;	/* サイズ不明の場合は 0 を返す。基本的に -1 は返さないこと */
}



/*----------------------------------------------------------------------*/
/* ファイル名、パス名処理						*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void osd_path_regularize( char *pathname, int max_size )		*/
/*	ファイル名やパスについての処理は機種依存なので、この関数はダミー*/
/************************************************************************/
void	osd_path_regularize( char *filename, int max_size )
{
}


/************************************************************************/
/* void	osd_path_separate( char *result_path, const char *filename, 	*/
/*			   int max_size )				*/
/*	ファイル名やパスについての処理は機種依存なので、		*/
/*	とりあえず、常に失敗を返す。					*/
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
/*	ファイル名やパスについての処理は機種依存なので、		*/
/*	とりあえず、常に child_path を返す				*/
/************************************************************************/
void	osd_path_connect( char *result_path, const char *parent_path,
			  const char *child_path, int max_size )
{
  strncpy( result_path, child_path, max_size-1 );
  result_path[ max_size-1 ] = '\0';
}




/*----------------------------------------------------------------------*/
/* 環境変数処理								*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void	osd_get_environment( void )					*/
/************************************************************************/
void	osd_get_environment( int *status )
{
  if( status ) *status = ENV_OK;

  /* 環境変数はどうでもいい (使わない) */

  dir_home = getenv( "HOME" );			/* ${HOME}		*/
  dir_rom  = getenv( "QUASI88_ROM_DIR" );	/* ${QUASI88_ROM_DIR}	*/
  dir_disk = getenv( "QUASI88_DISK_DIR" );	/* ${QUASI88_DISK_DIR}	*/
}

/************************************************************************/
/* void	osd_set_image_dir( const char *conf_dir_rom,			*/
/*			   const char *conf_dir_disk );			*/
/* ディレクトリを操作する一般的な方法ないので、この関数はダミー		*/
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
/*	つねに、"" を返す						*/
/************************************************************************/
const	char	*osd_get_current_dir( void )
{
  return "";
}



/************************************************************************/
/* char *osd_get_home_filename( const char *fname )			*/
/*	常に、fname を返す						*/
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
/* ファイルオープン							*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* FILE *osd_fopen_image( const char *filename, int read_only,		*/
/*			  char **real_filename, int *status )		*/
/*	指定された filename をそのまま開く				*/
/************************************************************************/

/* "r+b"で開いてみて、失敗したら "rb" で再度開いてみるマクロ */
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

	/* ファイルオープン失敗の場合は、NULL を返す */

  if( ! open_success ){
    if( status ) *status = FOPEN_ERR;
    return NULL;
  }


  *real_filename = fname;
  return fp;
}



/************************************************************************/
/* FILE *osd_fopen_rom( const char *filename, int *status )		*/
/*	指定された filename をそのまま開く				*/
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
/*	指定された filename をそのまま開く				*/
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
