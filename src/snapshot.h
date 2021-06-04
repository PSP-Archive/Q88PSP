#ifndef SNAPSHOT_H_INCLUDED
#define SNAPSHOT_H_INCLUDED


extern	char	*file_snapshot;		/* スナップショットファイル名	*/
extern	char	*snapshot_cmd;		/* スナップショットコマンド	*/
extern	int	snapshot_format;	/* スナップショットフォーマット	*/
extern	char	snapshot_cmd_do;	/* コマンド実行の有無		*/


void	screen_snapshot_init( void );
int	save_screen_snapshot( void );


#endif	/* SNAPSHOT_H_INCLUDED */
