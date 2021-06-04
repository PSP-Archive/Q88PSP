#ifndef GETCONF_H_INCLUDED
#define GETCONF_H_INCLUDED


/* �`�F�b�N�s�Ȃ�Ȃ��I�v�V���� */

typedef struct {
  const	char	*str;		/* �I�v�V������ (�n�C�t������)	*/
  int		argc;		/* �����̐� (1 or 2)		*/
} T_IGNORE_OPT;


/* �����̊֐��́A�S�� main()�֐�����̂݌Ă΂�� */

int	config_init( int argc, char *argv[] );
void	disk_set_args_file( void );
void	bootup_work_init( void );
int	exist_rcfile( void );
void	add_diskimage(char *filename,int image,int Drive);
char *DiskName(int Drive);


#endif	/* GETCONF_H_INCLUDED */
