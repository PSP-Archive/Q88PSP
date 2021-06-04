#ifndef INTR_H_INCLUDED
#define INTR_H_INCLUDED


extern	int	intr_level;			/* OUT[E4] 割り込みレベル */
extern	int	intr_priority;			/* OUT[E4] 割り込み優先度 */
extern	int	intr_sio_enable;		/* OUT[E6] 割込マスク SIO */ 
extern	int	intr_vsync_enable;		/* OUT[E6] 割込マスクVSYNC*/ 
extern	int	intr_rtc_enable;		/* OUT[E6] 割込マスク RTC */ 



extern	double	cpu_clock_mhz;		/* メイン CPUのクロック     [MHz] */
extern	double	sound_clock_mhz;	/* サウンドチップのクロック [MHz] */
extern	double	vsync_freq_hz;		/* VSYNC 割り込みの周期	    [Hz]  */


extern	int	state_of_cpu;			/*メインCPUが処理した命令数 */
extern	int	state_of_vsync;			/* VSYNC周期のステート数   */

extern	int	no_wait;			/* ウエイトなし		*/
extern	int	cpu_load;			/* ウエイト 小さいほど遅い */




extern	int	ctrl_vrtc;			/* 1:垂直帰線中  0: 表示中 */

extern	int	VSYNC_flag;			/* 各種割り込み信号フラグ */
extern	int	RTC_flag;
extern	int	SOUND_flag;
extern	int	RS232C_flag;





void	interval_work_init_all( void );

void	interval_work_set_RS232C( int bps, int framesize );

void	interval_work_set_TIMER_A( void );
void	interval_work_set_TIMER_B( void );

void	interval_work_set_BDRY( void );
void	interval_work_set_EOS( int length );

void	change_sound_flags( int port );
void	change_sound_prescaler( int new_prescaler );


void	main_INT_init( void );
int	main_INT_update( void );
int	main_INT_chk( void );







/* 1秒あたりの割り込み数を表示する (デバッグ用) */

/*#define DEBUG_INTR_CNT*/


#ifdef DEBUG_INTR_CNT
extern int disp_total_state;
extern int disp_vsync_count;
extern int disp_rtc_count;
extern int disp_sd_A_count;
extern int disp_sd_B_count;
extern int disp_sd_A_base;
extern int disp_sd_B_base;
#endif


#endif	/* INTR_H_INCLUDED */
