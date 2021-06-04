#ifndef WAIT_H_INCLUDED
#define WAIT_H_INCLUDED

/************************************************************************/
/* �E�G�C�g�����p�ϐ�							*/
/************************************************************************/
extern	double	wait_freq_hz;		/* �E�G�C�g�����p���g�� [Hz] */
extern	int	wait_by_sleep;		/* �E�G�C�g���Ԓ� sleep ���� */

extern	long	wait_sleep_min_us;	/* �c��idle���Ԃ��A���� ��s
					   �ȉ��Ȃ� sleep ���Ȃ�     */



/************************************************************************/
/* �E�G�C�g�����p�֐�							*/
/************************************************************************/
int	wait_vsync_init( void );
void	wait_vsync_term( void );

void	wait_vsync_reset( void );
void	wait_vsync( void );



#endif	/* WAIT_H_INCLUDED */
