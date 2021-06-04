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
#include <stdlib.h>	/* fullpath */
#include <string.h>
#include <sys/stat.h>	/* _stat */
#include <direct.h>	/* getcwd, _getdrives */
#include <io.h>		/* _findfirst */

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"



/* パスの区切り文字。必ず1文字であること */
#define PATH_SEPARATORS	"\\"
#define PATH_SEPARATOR	'\\'


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
  int		nr_total;
  T_DIR_ENTRY	*entry;
};



/* ディレクトリ内のファイル名のソーティングに使う関数 */

	/* 大文字小文字を無視してファイル名ソート  */
static int namecmp( const void *p1, const void *p2 )
{
  T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
  T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
  return _stricmp( s1->name, s2->name );
}
	/* 大文字小文字を無視してディレクトリ名ソート  */
static int dircmp( const void *p1, const void *p2 )
{
  T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
  T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
  if( s1->name[0]=='<' && s2->name[0]!='<' ) return -1;	  /* <～> はドライブ */
  if( s1->name[0]!='<' && s2->name[0]=='<' ) return +1;	  /* なので高優先    */
  return _stricmp( s1->name, s2->name );
}
	/* ファイルとディレクトリとを分離 */
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
/*	_findfirst(), _findnext(), _findclose() を駆使し、		*/
/*	ディレクトリの全てのエントリの ファイル名と属性をワークにセット	*/
/*	する。ワークは適宜、malloc() で確保するが、確保に失敗した場合は	*/
/*	エラー (NULL) を返す。						*/
/*	全ての処理後、ファイル名をソーティングしディレクトリのファイル	*/
/*	名に '[' と ']' を付加して戻る					*/
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

						/* T_DIR_INFO ワーク確保 */
  if( (dir = (T_DIR_INFO *)malloc( sizeof(T_DIR_INFO) ))==NULL ){
    return NULL;
  }

  if( strcmp( &filename[1], ":" )==0   ||	/* x: や x:\ はトップ DIR */
      strcmp( &filename[1], ":\\" )==0 ) top_dir = TRUE;

  if( strncmp( filename, "\\\\", 2 )== 0 ){	/* \\ で始まる場合、	*/
    int j=0;					/* 先頭から \ を検索し、*/
    len = strlen( filename ) -1;
    for( i=2; i<len; i++ ) 			/* その数を数える。	*/
      if( filename[i]=='\\' ) j++;
    if( j==1 ) top_dir = TRUE;			/* 1個だけなら トップ DIR */
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

  dir->nr_entry = 0;				/* ファイル数のチェック */
  dirp = _findfirst( fname, &dp );
  if( dirp != -1 ){
    do{
      dir->nr_entry ++;
    } while( _findnext( dirp, &dp )==0 );
    _findclose( dirp );
  }



						/* T_DIR_ENTRY ワーク確保 */
  dir->nr_total = dir->nr_entry + 26;
  dir->entry = (T_DIR_ENTRY *)malloc( dir->nr_total * sizeof(T_DIR_ENTRY) );
  if( dir->entry==NULL ){
    free( dir );
    free( fname );
    return NULL;
  }



  dirp = -1;
  for( i=0; i<dir->nr_entry; i++ ) {		/* ファイル名格納ループ */
    
    if( i==0 ){
      dirp = _findfirst( fname, &dp );
      if( dirp==-1 ){				/* 	いきなりファイルが無 */
	dir->nr_entry = i;			/*	い時は中断(成功扱い) */
	break;
      }
    }else{
      if( _findnext( dirp, &dp )!= 0 ){		/*	途中でファイルが減少 */
	dir->nr_entry = i;			/*	したら中断(成功扱い) */
	break;
      }
    }

						/*	ファイル名コピー */

    /* ファイル名を保持する分のメモリを確保する。
       ファイル名は SJISであると思われるが、これだとメニューでの表示時に
       文字化けするので EUCに変換しておく。 この場合ファイル名の長さは
       最大2倍になる。また、ディレクトリは [ ] で囲んで表示したいので、
       この 2文字分が余分に必要。よって確保するメモリのサイズは 末尾の \0 を
       含めて、 strlen(dp.name)*2 + 2 +1 となる。*/

    dir->entry[ i ].name = (char *)malloc( (strlen(dp.name))*2 +2 +1 );
    if( dir->entry[ i ].name==NULL ){
      dir->nr_entry = i;			/*	malloc に失敗        */
      break;					/*	したら中断(成功扱い) */
    }
    /* strcpy( dir->entry[ i ].name, dp.name ); */
    sjis2euc( dir->entry[ i ].name, dp.name );		/* EUCで保持する */

    if (dp.attrib & _A_SUBDIR) {			/*	属性を設定 */
      dir->entry[ i ].type = DIR_ENTRY_TYPE_DIR;
    }else{
      dir->entry[ i ].type = DIR_ENTRY_TYPE_NOT_DIR;
    }
  }

  if( dirp!=-1 )
    _findclose( dirp );				/* ディレクトリを閉じる */


	/* エントリがない(取得失敗)場合や、ルートディレクトリの場合は、
	   ドライブをエントリに追加してあげよう */

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
      drv_name[1] ++;	/* "<x:>" の x を A→Zに置換していく */
    }
  }



	/* ファイル名をソート 属性で分離し、各々をソートする */
  qsort( dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), typecmp );
  {
    T_DIR_ENTRY *p = dir->entry;
    for( i=0; i<dir->nr_entry; i++, p++ ){
      if( p->type != DIR_ENTRY_TYPE_DIR ) break;
    }
    qsort( &dir->entry[0], i, sizeof(T_DIR_ENTRY), dircmp );
    qsort( &dir->entry[i], dir->nr_entry-i, sizeof(T_DIR_ENTRY), namecmp );
  }

	/* ファイル名に装飾をつける */
  {
    T_DIR_ENTRY *p = dir->entry;
    for( i=0; i<dir->nr_entry; i++, p++ ){
      if( p->type == DIR_ENTRY_TYPE_DIR ){		/* ディレクトリは */
	memmove( &p->name[1], &p->name[0], strlen( p->name )+1 );
	p->name[0] = '[';				/* [ ] で囲む     */
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
/*	osd_readdir で取得した装飾付のファイル名を、アクセス可能な	*/
/*	ファイル名に変換する						*/
/*	要は、ディレクトリなら前後の [] を削って \\ を付加する		*/
/************************************************************************/
void		osd_readdir_realname( char *realname, const T_DIR_ENTRY *dir,
				      int maxsize )
{
  if( dir->type == DIR_ENTRY_TYPE_DIR ){	/* ディレクトリの場合 */
    int  len    = strlen( dir->name );
    char drv[4] = "x:\\";

    if( len==6 && dir->name[1] == '<' ){	    /* [<x:>] の場合 */

      drv[0] = dir->name[2];				/* x←ドライブA～Z */
      if( maxsize > 3 ) maxsize = 4;
      strncpy( realname, drv, maxsize-1 );		/* x:\ をセット */
      realname[ maxsize - 1 ] = '\0';
      return;

    }else if( len>2 ){				    /* [～] の場合 */

      strncpy( realname, &dir->name[1], maxsize-1 );	/* 先頭の [ を削り */
      len --;

      if( len <= maxsize -1 ){
	realname[ len-1 ] = '\\';			/* 末尾の ] を \ に */
	realname[ len ]   = '\0';			/* 置き換える       */
      }else{
	realname[ maxsize - 1 ] = '\0';
      }
      euc2sjis( realname, realname );			/* SJISに変換 */
      return;
    }
  }
						/* ファイルの場合 */
  strncpy( realname, dir->name, maxsize -1 );		/* そのまま */
  realname[ maxsize - 1 ] = '\0';
  euc2sjis( realname, realname );			/* SJISに変換 */

  /* EUC から SJISに変換する場合、文字列長さは同じか、短くなる。
     なので、euc2sjis() の引数に同じポインタを指定しても大丈夫なはず… */
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

  for( i=0; i<dirp->nr_entry; i++ ) 
    free( dirp->entry[i].name );
  free( dirp->entry );
  free( dirp );
}







/*----------------------------------------------------------------------*/
/* ファイル属性								*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* int osd_file_stat( const char *filename )				*/
/*	_findfirst() にて filename の属性をチェック			*/
/************************************************************************/
int	osd_file_stat( const char *filename )
{
  char *fname;
  int i;
  long dirp;
  struct _finddata_t dp;

  /*
    ""        ならエラー、
    "\\"      なら(ルート)ディレクトリ
    "x:\\"    なら(ルート)ディレクトリ、
    "x:"      ならディレクトリ
    "\\\\*"   なら(ネットワーク)ディレクトリ
    "\\\\*\*" なら(ネットワーク)ディレクトリ
  */

  i = strlen(filename);
  if( i==0 ) return FILE_STAT_ANOTHER;
  if( strcmp(  filename,    "\\"  )== 0 ) return FILE_STAT_DIR;
  if( strcmp( &filename[1], ":"   )== 0 ) return FILE_STAT_DIR;
  if( strcmp( &filename[1], ":\\" )== 0 ) return FILE_STAT_DIR;

  fname = (char *)malloc( i + 1 );
  if( fname==NULL ) return FILE_STAT_ANOTHER;

  strcpy( fname, filename );		/* 末尾が \\ なら削る */
  if( fname[i-1] == '\\' ) fname[i-1] = '\0';

  if( strncmp( fname, "\\\\", 2 )== 0 ){	/* \\ で始まる場合、	*/
    int j=0;					/* 先頭から \ を検索し、*/
    for( i=2; fname[i]; i++ ) 			/* その数を数える。	*/
      if( fname[i]=='\\' ) j++;
    if( j==1 ){					/* 1個だけならネット	*/
      free( fname );				/* ワークディレクトリと	*/
      return FILE_STAT_DIR;			/* みなそう		*/
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
/*	_stat() の結果とファイル名が一致すれば同じファイルとみなす	*/
/************************************************************************/
int	osd_file_cmp( const char *s_file, const char *t_file )
{
  struct _stat	sb[2];
  int		flag = 0;

  if( _stat( s_file, &sb[0] ) ){		/* s_file の情報を得る */
    flag |= 0x01;
  }
  if( _stat( t_file, &sb[1] ) ){		/* t_file の情報を得る */
    flag |= 0x02;
  }

  if( flag ) return FILE_CMP_ERR;		/* アクセスエラー時戻る	*/


  if( sb[0].st_dev   == sb[1].st_dev   &&	/* 比較する		*/
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
/*	_findfirst にて ファイルサイズを得る				*/
/************************************************************************/
long	osd_file_size( const char *filename )
{
  long dirp;
  struct _finddata_t dp;

  dirp = _findfirst( filename, &dp );
  if( dirp==-1 ){			/* ファイルが無いぞ… */
    return 0;
  }
  _findclose( dirp );

  if (dp.attrib & _A_SUBDIR) 		/* それはディレクトリだ */
    return -1;
  else					/* OK, you got it	*/
    return dp.size;
}



/*----------------------------------------------------------------------*/
/* ファイル名、パス名処理						*/
/*----------------------------------------------------------------------*/
/************************************************************************/
/* void osd_path_regularize( char *pathname, int max_size )		*/
/*	_fullpathの結果をそのまま返す。   				*/
/*	_fullpathの結果が NULL なら、pathname をそのまま返す		*/
/*	ディレクトリの場合、入力元の末尾が \ なら結果も \ がつくが、	*/
/*	入力元の末尾が \ でないなら、結果も \ はつかない。		*/
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
/*	filename の最後の '\\' より前の部分を返す			*/
/************************************************************************/
const char *osd_path_separate( char *result_path, const char *filename, 
			       int max_size )
{
  int	pos = strlen( filename );

  if( pos==0 || pos >= max_size ){
    result_path[0] = '\0';
    return filename;
  }

  do{				/* '\' をファイル名の尻から探す */
    if( filename[pos-1] == PATH_SEPARATOR ) break;
    pos --;
  }while( pos );

  if( pos ){			/* '\' が見つかったら		*/
    memmove( result_path, filename, pos );	/* 先頭～'\'までをコピー*/
    result_path[pos] = '\0';			/* '\' も含まれます	*/
    return &filename[pos];

  }else{			/* '\' が見つからなかった	*/
    result_path[0] = '\0';
    return filename;
  }
}



/************************************************************************/
/* void osd_path_connect( char *result_path, const char *parent_path,	*/
/*			  const char *child_path, int max_size )	*/
/*	chile_path が \ で始まっていたら、そのまま返す。		*/
/*	chile_path が x:\ の場合も、そのまま返す。			*/
/*	そうでなければ、"parent_path" + "/" + "child_path" を返す	*/
/************************************************************************/
void	osd_path_connect( char *result_path, const char *parent_path,
			  const char *child_path, int max_size )
{
  int	len = strlen( child_path );

  if( child_path[0] == PATH_SEPARATOR   ||		/* 絶対パス */
      strncmp( &child_path[1], ":\\", 2 ) == 0 ){

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
/************************************************************************/
/* void	osd_get_environment( void )					*/
/*	dir_cwd  … CWDをセット。が取得できなければ、NULL		*/
/************************************************************************/
void	osd_get_environment( int *status )
{
  char *p;

  if( status ) *status = ENV_OK;


  dir_cwd = (char *)malloc( OSD_MAX_FILENAME );	/* CWD をセットする	*/
  if( dir_cwd ){
    if( _getcwd( dir_cwd, OSD_MAX_FILENAME )==NULL ){
      free( dir_cwd );
      dir_cwd = NULL;
    }else{
      osd_path_regularize( dir_cwd, OSD_MAX_FILENAME );
    }
  }

  /* 環境変数 */

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
    /* しまった、以前の dir_disk を free() していない…… */
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
/* ファイルオープン							*/
/*----------------------------------------------------------------------*/

/************************************************************************/
/* FILE *osd_fopen_image( const char *filename, int read_only,		*/
/*			  char **real_filename, int *status )		*/
/*	${QUASI88_DISK_DIR} で設定されたディレクトリでfilename を開く	*/
/*	filename が絶対パスなら、そこから開く				*/
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
    安易にこうやると、filenameが絶対パスの時にはまるので注意
  */
  osd_path_connect( fname, dirname, filename, size );
  osd_path_regularize( fname, size );

  FOPEN_IMAGE( fname );
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
/*	${QUASI88_ROM_DIR} で設定されたディレクトリでファイルを読み込む	*/
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
