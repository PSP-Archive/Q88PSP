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
/* void	osd_set_image_dir( const char *new_dir_rom,			*/
/*			   const char *new_dir_disk )			*/
/*	ROMイメージを開くディレクトリ を new_dir_rom に設定する。	*/
/*	DISKイメージを開くディレクトリ を new_dir_disk に設定する。	*/
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

/* パスの区切り文字。必ず1文字であること */
#define PATH_SEPARATORS	"/"
#define PATH_SEPARATOR	'/'


/* 使用するディレクトリ。いずれも不明時は NULL になる。*/
static		char	*dir_cwd  = NULL;	/* cwd			*/
static	const	char	*dir_home = NULL;	/* ${HOME}		*/
static	const	char	*dir_rom  = NULL;	/* ${QUASI88_ROM_DIR}	*/
static	const	char	*dir_disk = NULL;	/* ${QUASI88_DISK_DIR}	*/



/*----------------------------------------------------------------------*/
/* ディレクトリの閲覧							*/
/*----------------------------------------------------------------------*/

struct	_T_DIR_INFO
{
  int		cur_entry;
  int		nr_entry;
  T_DIR_ENTRY	*entry;
};



/* ディレクトリ内のファイル名のソーティングに使う関数 */

static int namecmp( const void *p1, const void *p2 )
{
  T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
  T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;

  return strcmp( s1->name, s2->name );
}




/************************************************************************/
/*T_DIR_INFO *osd_opendir( const char *filename )			*/
/*	opendir()、rewinddir()、readdir()、closedir() を駆使し、	*/
/*	ディレクトリの全てのエントリの ファイル名と属性をワークにセット	*/
/*	する。ワークは適宜、malloc() で確保するが、確保に失敗した場合は	*/
/*	エラー (NULL) を返す。						*/
/*	全ての処理後、ファイル名をソーティングしディレクトリのファイル	*/
/*	名に '/' を付加して戻る						*/
/************************************************************************/
T_DIR_INFO	*osd_opendir( const char *filename )
{
  int	i;
  T_DIR_INFO	*dir;

  DIR	*dirp;
  struct dirent *dp;

						/* T_DIR_INFO ワーク確保 */
  if( (dir = (T_DIR_INFO *)malloc( sizeof(T_DIR_INFO) ))==NULL ){
    return NULL;
  }


  dirp = opendir( filename );			/* ディレクトリを開く */
  if( dirp==NULL ){
    free( dir );
    return NULL;
  }


  dir->nr_entry = 0;				/* ファイル数のチェック */
  while( readdir(dirp) ) dir->nr_entry ++;
  rewinddir( dirp );

						/* T_DIR_ENTRY ワーク確保 */
  dir->entry = (T_DIR_ENTRY *)malloc( dir->nr_entry * sizeof(T_DIR_ENTRY) );
  if( dir->entry==NULL ){
    closedir( dirp );
    free( dir );
    return NULL;
  }



  for( i=0; i<dir->nr_entry; i++ ) {		/* ファイル名格納ループ */
    
    dp = readdir( dirp );			/*	ファイル名取得	*/

    if( dp==NULL ){				/*	途中でファイルが減少 */
      dir->nr_entry = i;			/*	したら中断(成功扱い) */
      break;
    }
						/*	ファイル名コピー */
    dir->entry[ i ].name = (char *)malloc( strlen(dp->d_name) +1  +1 );
    if( dir->entry[ i ].name==NULL ){	/* 装飾用に +1 余分に確保 ↑ */
      dir->nr_entry = i;			/*	malloc に失敗        */
      break;					/*	したら中断(成功扱い) */
    }
    strcpy( dir->entry[ i ].name, dp->d_name );

    {						/*	属性を取得、設定 */
      char *fullname;
      struct stat sb;

	/* ディレクトリ名(filename) と ファイル名(dp->d_name) から、
	   フルパスのファイル名を生成し、stat()関数で属性を得る */

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
	     ここでエラーが起こる理由は、大抵は 処理の途中でこのファイルが削除
	     された、あるいはこのファイルがシンボリックリンクで、その先の
	     ファイルが存在しない、などである。この場合は、そのエントリを削除
	     する必要があるのだが、ここは手抜きして、エラーを無視してある。*/
	  dir->entry[ i ].type = DIR_ENTRY_TYPE_NOT_DIR;
	}
	free( fullname );
      }
      else{					/*	malloc失敗でも継続 */
	dir->entry[ i ].type = DIR_ENTRY_TYPE_NOT_DIR;
      }
    }
  }


  closedir( dirp );				/* ディレクトリを閉じる */


	/* ファイル名をソート */
  qsort( dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), namecmp );

	/* ファイル名に装飾をつける */
  {
    T_DIR_ENTRY *p = dir->entry;
    for( i=0; i<dir->nr_entry; i++, p++ ){
      if( p->type == DIR_ENTRY_TYPE_DIR ){	/* ディレクトリには / を付加 */
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
/*	osd_readdir で取得した装飾付のファイル名を、アクセス可能な	*/
/*	ファイル名に変換する						*/
/************************************************************************/
void		osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,
				      int maxsize )
{
  strncpy( realname, dir->name, maxsize -1 );	/* そのままの文字列を返す */
  realname[ maxsize - 1 ] = '\0';		/* ディレクトリは / 付加済 */
}


/************************************************************************/
/* T_DIR_ENTRY *osd_readdir( T_DIR_INFO *dirp )				*/
/*	osd_opendir() の時に確保した、エントリ情報ワークへのポインタを	*/
/*	順次、返していく。						*/
/*	この時のファイル名は、表示用に装飾されているので注意		*/
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
/*	osd_opendir() 時に確保した全てのメモリを開放する。		*/
/************************************************************************/
void		osd_closedir( T_DIR_INFO *dirp )
{
  int	i;

  for( i=0; i<dirp->nr_entry; i++ ) free( dirp->entry[i].name );
  free( dirp->entry );
  free( dirp );
}







/*----------------------------------------------------------------------*/
/* ファイル属性								*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* int osd_file_stat( const char *filename )				*/
/*	filename を ディレクトリとして開いてみる。成功すれば、それは	*/
/*	ディレクトリ。失敗すれば、今度はファイルとして開いてみる。	*/
/*	成功すれば、それはファイル。失敗すれば、それは不明。		*/
/************************************************************************/
int	osd_file_stat( const char *filename )
{
  DIR	*dirp;
  FILE	*fp;

  if( (dirp = opendir( filename )) ){		/* ディレクトリとして開く */
    closedir( dirp );				/* 成功したらディレクトリ */
    return FILE_STAT_DIR;
  }else{
    if( (fp = fopen( filename, "r" )) ){	/* ファイルとして開く     */
      fclose( fp );				/* 成功したらファイル	  */
      return FILE_STAT_FILE;
    }else{
      return FILE_STAT_ANOTHER;			/* どちらとも失敗	  */
    }
  }
}



/************************************************************************/
/* int osd_file_cmp( const char *s_file, const char *t_file )		*/
/*	stat() 関数を使用し、二つのファイルの、st_dev と st_ino が一致	*/
/*	すれば、同じファイルとみなす。					*/
/************************************************************************/
int	osd_file_cmp( const char *s_file, const char *t_file )
{
  struct stat	sb[2];
  int		flag = 0;

  if( stat( s_file, &sb[0] ) ){			/* s_file の情報を得る */
    flag |= 0x01;
  }
  if( stat( t_file, &sb[1] ) ){			/* t_file の情報を得る */
    flag |= 0x02;
  }

  if( flag ) return FILE_CMP_ERR;		/* アクセスエラー時戻る	*/


  if( sb[0].st_dev == sb[1].st_dev &&		/* 比較する		*/
      sb[0].st_ino == sb[1].st_ino ){
    return FILE_CMP_OK;
  }else{
    return FILE_CMP_NOT_OK;
  }
}



/************************************************************************/
/* long	osd_file_size( const char *filename )				*/
/*	stat() 関数を使用し、st_size を返す				*/
/************************************************************************/
long	osd_file_size( const char *filename )
{
  struct stat	sb;

  if( stat( filename, &sb ) ){			/* file の情報を得る */
    return -1;
  }else{
    return sb.st_size;				/* st_size がファイルサイズ */
  }
}



/*----------------------------------------------------------------------*/
/* ファイル名、パス名処理						*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void osd_path_regularize( char *pathname )				*/
/************************************************************************/
void	osd_path_regularize( char *filename, int max_size )
{
  int	i, level = 0;
  char	**s;
  char	*p, *q;


  for( i=0; i<(int)strlen(filename); i++ ){	/* パスの階層を数える */
    if( filename[i]==PATH_SEPARATOR ) level ++;
  }

  if( (s = (char **)malloc( (level+1) * sizeof(char *) )) ){

    p = q = &filename[0];
    level = 0;


		/* ./ や ../ や // を処理 */

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

		/* パス名が . や .. や file で終っている場合の処理 */
		/* ( 言い替えれば、パス名が / で終っていない場合 ) */

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


		/* 余計なものを省いたパスをつなげる */

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
/*	filename の最後の '/' より前の部分を返す			*/
/************************************************************************/
const char *osd_path_separate( char *result_path, const char *filename, 
			       int max_size )
{
  int	pos = strlen( filename );

  if( pos==0 || pos >= max_size ){
    result_path[0] = '\0';
    return filename;
  }

  do{				/* '/' をファイル名の尻から探す */
    if( filename[pos-1] == PATH_SEPARATOR ) break;
    pos --;
  }while( pos );

  if( pos ){			/* '/' が見つかったら		*/
    memmove( result_path, filename, pos );	/* 先頭～'/'までをコピー*/
    result_path[pos] = '\0';			/* '/' も含まれます	*/
    return &filename[pos];

  }else{			/* '/' が見つからなかった	*/
    result_path[0] = '\0';
    return filename;
  }
}



/************************************************************************/
/* void osd_path_connect( char *result_path, const char *parent_path,	*/
/*			  const char *child_path, int max_size )	*/
/*	chile_path が / で始まっていたら、そのまま返す。		*/
/*	そうでなければ、"parent_path" + "/" + "child_path" を返す	*/
/************************************************************************/
void	osd_path_connect( char *result_path, const char *parent_path,
			  const char *child_path, int max_size )
{
  int	len;

  if( child_path[0] == PATH_SEPARATOR ){		/* 絶対パス */

    strncpy( result_path, child_path, max_size-1 );
    result_path[ max_size-1 ] = '\0';

  }else{						/* 相対パス */

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
/* 環境変数処理								*/
/*----------------------------------------------------------------------*/
static const char *parse_tilda( const char *fname );

/************************************************************************/
/* void	osd_get_environment( void )					*/
/*	dir_cwd  … CWDをセット。が取得できなければ、NULL		*/
/************************************************************************/
void	osd_get_environment( int *status )
{
  if( status ) *status = ENV_OK;


  dir_cwd = (char *)malloc( OSD_MAX_FILENAME );	/* CWD をセットする	*/
  if( dir_cwd ){
    if( getcwd( dir_cwd, OSD_MAX_FILENAME-1 )==NULL ){
      free( dir_cwd );
      dir_cwd = NULL;
    }
  }


  dir_home = getenv( "HOME" );			/* ${HOME}		*/
						/* 未定義の時は、NULL	*/
  if( dir_home==NULL ){
    if( status ) *status |= ENV_NO_HOME;
  }
  else{						/* 定義してあっても、   */
    if( dir_home[0] == '\0' || 			/* 先頭が / でなかったり */
	dir_home[0] != PATH_SEPARATOR ){	/* 空の場合は、NULL      */
      if( status ) *status |= ENV_INVALID_HOME;
      dir_home = NULL;
    }
  }



  dir_rom = getenv( "QUASI88_ROM_DIR" );	/* ${QUASI88_ROM_DIR}	*/
  if( dir_rom == NULL ){			/* 未定義の時は ROM_DIR	*/
    dir_rom = parse_tilda( ROM_DIR );		/* (Makefileで定義)	*/
    if( dir_rom == NULL ){
      if( status ) *status |= ENV_NO_MEM_ROM_DIR;
      dir_rom = ROM_DIR;
    }
  }

  dir_disk = getenv( "QUASI88_DISK_DIR" );	/* ${QUASI88_DISK_DIR}	*/
  if( dir_disk == NULL ){			/* 未定義の時は DISK_DIR*/
    dir_disk = parse_tilda( DISK_DIR );		/* (Makefileで定義)	*/
    if( dir_disk == NULL ){
      if( status ) *status |= ENV_NO_MEM_DISK_DIR;
      dir_disk = DISK_DIR;
    }
  }

}

/************************************************************************/
/* void	osd_set_image_dir( const char *new_dir_rom,			*/
/*			   const char *new_dir_disk );			*/
/*	dir_rom, dir_disk を置き換え					*/
/************************************************************************/
void	osd_set_image_dir( const char *new_dir_rom,
			   const char *new_dir_disk )
{
  /*
   *	以下のコードは、floi 氏により提供されました。
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
  fname が ~ で始まっていたら、${HOME}に置き換えて返す関数
*/

static const char *parse_tilda( const char *fname )
{
  char *p = NULL;
  int pos;

  if( fname[0]=='~' ){			/* fnameが ~ で始まれば${HOME}に置換 */
    if( dir_home ){
      p = (char *)malloc( strlen( dir_home ) + strlen( fname ) + 1 );

      if( p ){
	if( fname[1]==PATH_SEPARATOR ||
	    fname[1]=='\0' ){			/* ~/ や ~/xxx や ~ の場合 */
	  strcpy( p, dir_home );
	  strcat( p, PATH_SEPARATORS );
	  strcat( p, &fname[1] );
	  osd_path_regularize( p, strlen(p) );

	}else{					/* ~xxx や ~xxx/ の場合 */

	  strcpy( p, dir_home );		    /* ${HOME} の尻から	*/
	  pos = strlen( p ) -1;			    /* / を探す. ただし */
	  if( pos>0 ){				    /* 末尾の / は除く	*/
	    do{
	      if( p[pos-1] == PATH_SEPARATOR ) break;
	      pos --;
	    }while( pos );
	    strcpy( &p[pos], &fname[1] );	    /* その / に、~xxx/ */
	    osd_path_regularize( p, strlen(p) );    /* の xxx/をつなぐ  */

	  }else{
	    free( p );
	    return fname;
	  }
	}
      }

    }else{				/* ${HOME}がないので置換できない */
      return fname;
    }
  }else{				/* ~ で始まらないので、置換しない */
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
/*	"${HOME}" + "\" + "fname" を返す				*/
/*	できなければ、NULL						*/
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
/* ファイルオープン							*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* FILE *osd_fopen_image( const char *filename, int read_only,		*/
/*			  char **real_filename, int *status )		*/
/*	${QUASI88_DISK_DIR} または、DISK_DIR で設定されたディレクトリで	*/
/*	ファイル filename を開く。あるいはそのまま filename を開く。	*/
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
  int	i;


		/* filename に '/' があるか探す */

  for( i = strlen( filename )-1; i>=0; i-- ){
    if( filename[i] == PATH_SEPARATOR ) break;
  }

  if( i<0 ){

	/* filename に '/' が含まれない場合			*/

		/*  1) dir_disk で示すディレクトリでファイルを開く	*/
		/*	(dir_disk はコンパイル時 or 環境変数で確定)	*/

    fname = (char *)malloc( strlen(dir_disk) + 1 + strlen(filename) + 1 );
    if( fname == NULL ){
      if( status ) *status = FOPEN_NO_MEM;
      return NULL;
    }
    strcpy( fname, dir_disk );		/* dir_disk + filename で開く */
    strcat( fname, PATH_SEPARATORS );
    strcat( fname, filename );
    osd_path_regularize( fname, strlen( fname ) );

    FOPEN_IMAGE( fname );
    if( ! open_success ) free( fname );

		/*  2) 開けなかったら、dir_cwd ディレクトリで開く	*/
		/*	(dir_cwd は カレントディレクトリ or NULL )	*/

    if( ! open_success ){
      if( dir_cwd ){
	fname = (char *)malloc( strlen(dir_cwd) + 1 + strlen(filename) + 1 );
	if( fname == NULL ){
	  if( status ) *status = FOPEN_NO_MEM;
	  return NULL;
	}
	strcpy( fname, dir_cwd );	/* dir_cwd + filename で開く */
	strcat( fname, PATH_SEPARATORS );
	strcat( fname, filename );
	osd_path_regularize( fname, strlen( fname ) );

	FOPEN_IMAGE( fname );
	if( ! open_success ) free( fname );
      }
    }

		/*  3) それでも開けなかったら、直接開く			*/

    if( ! open_success ){
      fname = (char *)malloc( strlen(filename) + 1 );
      if( fname == NULL ){
	if( status ) *status = FOPEN_NO_MEM;
	return NULL;
      }
      strcpy( fname, filename );	/* filename で開く */
      osd_path_regularize( fname, strlen( fname ) );

      FOPEN_IMAGE( filename );
      if( ! open_success ) free( fname );
    }

  }
  else
  {
	/* filename に '/' が含まれる場合			*/

		/*  1) 絶対パスならそのままファイルを開く		*/

    if( filename[0] == PATH_SEPARATOR ){

      fname = (char *)malloc( strlen(filename) + 1 );
      if( fname == NULL ){
	if( status ) *status = FOPEN_NO_MEM;
	return NULL;
      }
      strcpy( fname, filename );		/* filename で開く */
      osd_path_regularize( fname, strlen( fname ) );

      FOPEN_IMAGE( fname );
      if( ! open_success ) free( fname );

    }else{

		/*  2) 相対パスなら、dir_cwd ディレクトリで開く		*/
		/*	(dir_cwd は カレントディレクトリ or NULL )	*/

      if( dir_cwd ){
	fname = (char *)malloc( strlen(dir_cwd) + 1 + strlen(filename) + 1 );
	if( fname == NULL ){
	  if( status ) *status = FOPEN_NO_MEM;
	  return NULL;
	}
	strcpy( fname, dir_cwd );		/* dir_cwd + filename で開く */
	strcat( fname, PATH_SEPARATORS );
	strcat( fname, filename );
	osd_path_regularize( fname, strlen( fname ) );

	FOPEN_IMAGE( fname );
	if( ! open_success ) free( fname );
      }

		/*     それでも開けなかったら、直接開く			*/

      if( ! open_success ){
	fname = (char *)malloc( strlen(filename) + 1 );
	if( fname == NULL ){
	  if( status ) *status = FOPEN_NO_MEM;
	  return NULL;
	}
	strcpy( fname, filename );		/* filename で開く */
	osd_path_regularize( fname, strlen( fname ) );

	FOPEN_IMAGE( fname );
	if( ! open_success ) free( fname );
      }

    }
  }

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
/*	${QUASI88_ROM_DIR} または、ROM_DIR で設定されたディレクトリで	*/
/*	ファイル filename を開く					*/
/************************************************************************/

FILE	*osd_fopen_rom( const char *filename, int *status )
{
  char	*fname;
  FILE	*fp;

  if( dir_rom==NULL ){			/* dir_rom が未定義ならエラー */
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
/*	${HOME} にて、filename を開く					*/
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
