#ifndef SNAPSHOT_H_INCLUDED
#define SNAPSHOT_H_INCLUDED


extern	char	*file_snapshot;		/* �X�i�b�v�V���b�g�t�@�C����	*/
extern	char	*snapshot_cmd;		/* �X�i�b�v�V���b�g�R�}���h	*/
extern	int	snapshot_format;	/* �X�i�b�v�V���b�g�t�H�[�}�b�g	*/
extern	char	snapshot_cmd_do;	/* �R�}���h���s�̗L��		*/


void	screen_snapshot_init( void );
int	save_screen_snapshot( void );


#endif	/* SNAPSHOT_H_INCLUDED */
