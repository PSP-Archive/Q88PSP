#ifndef MONITOR_H_INCLUDED
#define MONITOR_H_INCLUDED



extern	int	debug_mode;			/* �f�o�b�O�@�\(���j�^�[)  */
extern	char	alt_char;			/* ��֕��� */

void	set_signal( void );



void	monitor_init( void );
void	monitor_main( void );

void print_hankaku(FILE *fp, Uchar *str, char npc);

#endif	/* MONITOR_H_INCLUDED */
