/************************************************************************/
/*									*/
/* �E�G�C�g�����p�֐� (OS�ˑ�)						*/
/*									*/
/* �y�֐��z								*/
/*									*/
/* int  wait_vsync_init( void )		�E�F�C�g����������		*/
/* void wait_vsync_term( void )		�E�F�C�g����������		*/
/* void	wait_vsync_reset( void )	�E�F�C�g�ď����� (�ݒ�ύX��)	*/
/* void wait_vsync( void )		�E�F�C�g			*/
/*									*/
/************************************************************************/
#include <stdio.h>

// #include <SDL.h>
#include "pg.h"
#include "quasi88.h"
#include "initval.h"
#include "wait.h"
#include "suspend.h"

#include "screen.h"	/* auto_skip... */



double	wait_freq_hz = DEFAULT_WAIT_FREQ_HZ;	/* �E�G�C�g�����p���g�� [Hz] */
int	wait_by_sleep = FALSE;			/* �E�G�C�g���Ԓ� sleep ���� */

long	wait_sleep_min_us = 100;		/* �c�� idle���Ԃ��A���� ��s
						   �ȉ��Ȃ� sleep ���Ȃ�
						   ( 1�b�����Őݒ肷�邱��! )*/
						/* UNIX_WAIT �w�莞�̂ݗL��  */


/*
 * �����t���[���X�L�b�v		( by floi, thanks ! )
 */

static	int	skip_counter = 0;		/* ����A���ŃX�L�b�v������ */
static	int	skip_count_max = 15;		/* �ő�X�L�b�v��
						   ����ȏ�ɂȂ����猻�ݎ��������Z�b�g */



/*
 * �E�F�C�g�����֐��Q
 */
double long  next_time;
double long base_time;

int	wait_vsync_init( void )			/* �E�F�C�g���������� */
{
	wait_vsync_reset();
	return TRUE;
}


void	wait_vsync_term( void )			/* �E�F�C�g�����I�� */
{
}


//double long wait_base = 830000;
//double long wait_base = 1095000;//pgWaitV���Ȃ��Ƃ�
//double long wait_base = 1000620;//����Wait��
double long wait_base = 521500;
double long count_time = 0;
double long count_base = 0;
void	wait_vsync_reset( void )		/* �E�F�C�g�����ď����� */
{
	next_time = sceKernelLibcClock() + (unsigned long)(wait_base/60);
//	base_time = sceKernelLibcClock();
//	count_base = base_time;
}


void	wait_vsync( void )			/* ���ۂ̃E�F�C�g���� */
{
#if 0
	pgWaitV();
	do_update = TRUE;
	return;
#else
	int on_time = FALSE;
	double long now;
    while(1){
	    now = sceKernelLibcClock();
		if ( (next_time <= now) && ((now - next_time) > (unsigned long)(wait_base/60))
		|| (next_time > 0xF0000000 && now < 0x10000000)){
//		    next_time = now + (unsigned long)(wait_base/60);
			if((now - next_time) > (unsigned long)(wait_base/60)){
				next_time = now;			/*    �^�C�}�l�ď�����      */
				on_time = TRUE;
				pgWaitV();		/* for AUDIO thread ?? */
			}else{
			}
//		    next_time += (unsigned long)(wait_base/60);
//			do_update = TRUE;
//			return;
			break;
		}

	//���̕ӂŗݐϓ��쎞�ԕ\�����Ă݂�B�����Wait�̒������o����͂��B
//	if(now < count_base){
//		count_time += now + 0xFFFFFFFF + count_base;
//	}else{
//		count_time += now - count_base;
//	}
//	print_dec(0,3,(count_time)/1000);
//	print_dec(0,3,(now - base_time)/1000);
//		print_dec(0,7,next_time);
//		print_dec(0,8,now);
		on_time = TRUE;
	    if((now - next_time) > (unsigned long)(wait_base/60)) break;//���荞�ݎ��Ԃ�茻���������荞�݊Ԋu�����傫���ꍇ
//		if((next_time - now) > (unsigned long)(wait_base/60)*1000) break;//���荞�ݎ��Ԃ��͂邩���Ȃ��̏ꍇ�B��ɐi�݂����ĉ���Ă��܂����ꍇ�������B
//		if(next_time <= now ) break;
		if(((next_time - now) >= 20) || ((next_time < 20) && (20 < (next_time + (0xFFFFFFFF - now))))) pgWaitV();		/* for AUDIO thread ?? */
//		pgWaitV();		/* for AUDIO thread ?? */
    }
//	next_time = now + (unsigned long)(wait_base/60);
    next_time += (unsigned long)(wait_base/60);

  /*
   * �����t���[���X�L�b�v����		( by floi, thanks ! )
   */
  if( use_auto_skip ){
    if( on_time ){			/* ���ԓ��ɏ����ł��� */
/*if(skip_counter)printf("%x\n",skip_counter);*/
      skip_counter = 0;
      do_skip_draw = FALSE;
      if( already_skip_draw ){		/* ���ɕ`����X�L�b�v���Ă����� */
	already_skip_draw = FALSE;
	reset_frame_counter();		/* ���͕K���`�悷�� */
      }
    }else{				/* ���ԓ��ɏ����ł��Ă��Ȃ� */
      skip_counter++;
      if( skip_counter >= skip_count_max ){	/* �X�L�b�v������ */
	skip_counter = 0;
	do_skip_draw = FALSE;
	already_skip_draw = FALSE;
	reset_frame_counter();			/* ���͕K���`�悷�� */
	next_time = now;				/* �^�C�}����������  */
      }else{
	do_skip_draw = TRUE;			/* �`����X�L�b�v����K�v�L�� */
      }
    }
  }

//  do_update = TRUE;
  return;
 #endif
}


/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_wait_work[] =
{
  { TYPE_DOUBLE,	&wait_freq_hz,	},
};


int	suspend_wait( FILE *fp, long offset )
{

  return TRUE;
}


int	resume_wait(  FILE *fp, long offset )
{

  return TRUE;
}
