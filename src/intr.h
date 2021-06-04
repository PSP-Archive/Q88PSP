#ifndef INTR_H_INCLUDED
#define INTR_H_INCLUDED


extern	int	intr_level;			/* OUT[E4] ���荞�݃��x�� */
extern	int	intr_priority;			/* OUT[E4] ���荞�ݗD��x */
extern	int	intr_sio_enable;		/* OUT[E6] �����}�X�N SIO */ 
extern	int	intr_vsync_enable;		/* OUT[E6] �����}�X�NVSYNC*/ 
extern	int	intr_rtc_enable;		/* OUT[E6] �����}�X�N RTC */ 



extern	double	cpu_clock_mhz;		/* ���C�� CPU�̃N���b�N     [MHz] */
extern	double	sound_clock_mhz;	/* �T�E���h�`�b�v�̃N���b�N [MHz] */
extern	double	vsync_freq_hz;		/* VSYNC ���荞�݂̎���	    [Hz]  */


extern	int	state_of_cpu;			/*���C��CPU�������������ߐ� */
extern	int	state_of_vsync;			/* VSYNC�����̃X�e�[�g��   */

extern	int	no_wait;			/* �E�G�C�g�Ȃ�		*/
extern	int	cpu_load;			/* �E�G�C�g �������قǒx�� */




extern	int	ctrl_vrtc;			/* 1:�����A����  0: �\���� */

extern	int	VSYNC_flag;			/* �e�튄�荞�ݐM���t���O */
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







/* 1�b������̊��荞�ݐ���\������ (�f�o�b�O�p) */

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
