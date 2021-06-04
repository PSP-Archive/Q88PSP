#ifndef GETCONF_H_INCLUDED
#define GETCONF_H_INCLUDED


/* チェック行なわないオプション */

typedef struct {
  const	char	*str;		/* オプション名 (ハイフン抜き)	*/
  int		argc;		/* 引数の数 (1 or 2)		*/
} T_IGNORE_OPT;


/* ここの関数は、全て main()関数からのみ呼ばれる */

int	config_init( int argc, char *argv[] );
void	disk_set_args_file( void );
void	bootup_work_init( void );
int	exist_rcfile( void );
void	add_diskimage(char *filename,int image,int Drive);
char *DiskName(int Drive);


#endif	/* GETCONF_H_INCLUDED */
