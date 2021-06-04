#ifndef WAIT_H_INCLUDED
#define WAIT_H_INCLUDED

/************************************************************************/
/* ウエイト調整用変数							*/
/************************************************************************/
extern	double	wait_freq_hz;		/* ウエイト調整用周波数 [Hz] */
extern	int	wait_by_sleep;		/* ウエイト期間中 sleep する */

extern	long	wait_sleep_min_us;	/* 残りidle時間が、この μs
					   以下なら sleep しない     */



/************************************************************************/
/* ウエイト調整用関数							*/
/************************************************************************/
int	wait_vsync_init( void );
void	wait_vsync_term( void );

void	wait_vsync_reset( void );
void	wait_vsync( void );



#endif	/* WAIT_H_INCLUDED */
