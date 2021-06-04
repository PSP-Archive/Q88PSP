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

#include <SDL.h>

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

#ifndef	UNIX_WAIT		/* === �ʏ� (SDL) ���g���ꍇ�͂����� === */

static	Uint32 next_time;

int	wait_vsync_init( void )			/* �E�F�C�g���������� */
{
  if( ! SDL_WasInit( SDL_INIT_TIMER ) ){		/* SDL�^�C�}�@�\ init */
    if( SDL_InitSubSystem( SDL_INIT_TIMER ) != 0 ){
      if( verbose_wait ) printf( "Error Wait (SDL)\n" );
    }
  }

  wait_vsync_reset();
  return TRUE;
}


void	wait_vsync_term( void )			/* �E�F�C�g�����I�� */
{
}



void	wait_vsync_reset( void )		/* �E�F�C�g�����ď����� */
{
  next_time = SDL_GetTicks() + (Uint32)(1000/wait_freq_hz);
}





void	wait_vsync( void )			/* ���ۂ̃E�F�C�g���� */
{
  int	on_time = FALSE;
  Uint32 now;

  if( wait_by_sleep ){	/* ���Ԃ�����܂� sleep ���� */

    now = SDL_GetTicks();
    if ( next_time <= now ) {				    /* ��������over */
      SDL_Delay( 1 );		/* for AUDIO thread ?? */
    }else{						    /* �]�莞�Ԃ��� */
      SDL_Delay( next_time-now );
      on_time = TRUE;
    }
    next_time = SDL_GetTicks() + (Uint32)(1000/wait_freq_hz);


  }else{		/* ���Ԃ�����܂� Tick ���Ď� */

    while(1){
      now = SDL_GetTicks();
      if( next_time <= now ) break;
      ;
      /* SDL_Delay( 1 ); */	/* for AUDIO thread ?? */
      on_time = TRUE;
    }
    next_time = SDL_GetTicks() + (Uint32)(1000/wait_freq_hz);
    SDL_Delay( 1 );		/* for AUDIO thread ?? */
  }


  /*
   * �����t���[���X�L�b�v����		( by floi, thanks ! )
   */
  if( use_auto_skip ){
    if( on_time ){			/* ���ԓ��ɏ����ł��� */
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
      }else{
	do_skip_draw = TRUE;			/* �`����X�L�b�v����K�v�L�� */
      }
    }
  }

  return;
}


#else	/* UNIX_WAIT */		/* === UNIX �̏ꍇ�A��������I���\ === */
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#define	HAVE_GETTIMEOFDAY	/* gettimeofday() ���Ȃ��ꍇ�́A#undef ���� */
#define	HAVE_SELECT		/* select()       ���Ȃ��ꍇ�́A#undef ���� */


typedef struct{			/* struct timeval �Ɠ��� */
  long tv_sec;
  long tv_usec;
} T_WAIT_TIMEVAL;

#define	WAIT_CMP( a, b, cmp )	((a.tv_sec == b.tv_sec) ?	\
				 (a.tv_usec cmp b.tv_usec) :	\
				 (a.tv_sec  cmp b.tv_sec)   )

#define	WAIT_ADD( a, b )	a.tv_sec  += b.tv_sec ;		\
				a.tv_usec += b.tv_usec;		\
				if( a.tv_usec >= 1000000 ){	\
				  a.tv_sec ++;			\
				  a.tv_usec -= 1000000;		\
				}

#define	WAIT_SUB( x, a, b )	x.tv_sec  = a.tv_sec  - b.tv_sec ;	\
				x.tv_usec = a.tv_usec - b.tv_usec;	\
				if( x.tv_usec < 0 ){			\
				  x.tv_sec --;				\
				  x.tv_usec += 1000000;			\
				}

static	int	wait_error = FALSE;		/* �����擾�ňُ픭�����A�^ */


/*
 * ���ݎ����擾�֐� int get_now_timeval( T_WAIT_TIMEVAL *now )
 *	���펞�ɂ́A0 ��Ԃ�
 */

#ifdef  HAVE_GETTIMEOFDAY	/* -------- gettimeofday() ���g�p���� ------ */
#include <sys/time.h>

INLINE	int	get_wait_timeval( T_WAIT_TIMEVAL *now )
{
  struct timeval tv;
  if( gettimeofday( &tv, 0 ) ){
    if( verbose_wait ) printf( "Error Wait (gettimeofday())\n" );
    return -1;
  }else{
    now->tv_sec  = tv.tv_sec;
    now->tv_usec = tv.tv_usec;
    return 0;
  }
}


#else				/* -------- clock() ���g�p���� ------------- */
#include <time.h>

/* #define CLOCK_SLICE	CLK_TCK */		/* ���ꂶ��ʖځH */
#define	CLOCK_SLICE	CLOCKS_PER_SEC		/* �������������H */

INLINE	int	get_wait_timeval( T_WAIT_TIMEVAL *now )
{
  clock_t t = clock();
  if( t == (clock_t)-1 ){
    if( verbose_wait ) printf( "Error Wait\n" );
    return -1;
  }else{
    now->tv_sec  =  t / CLOCK_SLICE;
    now->tv_usec = (double)(t % CLOCK_SLICE) * 1000000.0 / CLOCK_SLICE;
    return 0;
  }
}

#endif				/* ----------------------------------------- */

int	wait_vsync_init( void )
{
  wait_vsync_reset();
  return TRUE;
}


void	wait_vsync_term( void )
{
}



static	T_WAIT_TIMEVAL	next_tick;		/* ����E�G�C�g�������� */
static	T_WAIT_TIMEVAL	add_frame;		/* �E�G�C�g�̎���	*/
static	T_WAIT_TIMEVAL	sleep_min;		/* �E�G�C�g�̍ŏ�����	*/

void	wait_vsync_reset( void )
{
  if( get_wait_timeval( &next_tick ) ){
    wait_error = TRUE;

  }else{
    wait_error = FALSE;

    add_frame.tv_sec  = 0;				/* �E�G�C�g�̎��� */
    add_frame.tv_usec = 1000000 / wait_freq_hz;

    sleep_min.tv_sec  = 0;				/* sleep �ŏ����� */
    sleep_min.tv_usec = wait_sleep_min_us;

    WAIT_ADD( next_tick, add_frame );			/* ��������̎��� */
  }
}


void	wait_vsync( void )
{
  int	on_time = FALSE;
  T_WAIT_TIMEVAL now, delay;
  int sleep_flag = wait_by_sleep;	/* idle �� sleep ���ĉ߂������t���O */

  if( wait_error ) return;

  while( 1 ){

    if( get_wait_timeval( &now ) ){		/* ���ݎ����擾 */
      wait_error = TRUE;
      return;
    }

					/* �E�F�C�g�������ɏ������� ! ====== */

    if( WAIT_CMP( next_tick, now, <= ) ){
#if 0
      next_tick = now;
      on_time = TRUE;
#else
      WAIT_SUB( delay, now, next_tick );	/* �ǂ̈ʂ̒x�ꂩ���Z�o�B   */
      if( WAIT_CMP( add_frame, delay, < ) ){	/* 1�����ȏ�x��Ă���΁A  */
	next_tick = now;			/*    �^�C�}�l�ď�����      */
	on_time = TRUE;
      }else{					/* ����Ȃɒx��ĂȂ��Ȃ�A */
	;					/*    ���̂܂ܑ��s          */
      }
#endif
      WAIT_ADD( next_tick, add_frame );
      break;
    }


					/* �E�F�C�g�������ɏ������� ! ====== */
    on_time = TRUE;

#ifdef	HAVE_SELECT
    if( sleep_flag ){

      WAIT_SUB( delay, next_tick, now );	/* �c�莞�Ԃ��Z�o�B        */

      if( WAIT_CMP( delay, sleep_min, < ) ){	/* �c�菭�Ȃ��ꍇ�́A      */
	sleep_flag = FALSE;			/* 	polling ���ĉ߂��� */

      }else{					/* ���\�c���Ă�ꍇ�́A    */
	struct timeval tv;			/*	sleep ���ĉ߂���   */
#if 0
	WAIT_SUB( delay, delay, sleep_min );
	tv.tv_sec  = delay.tv_sec;
	tv.tv_usec = delay.tv_usec;
	select( 0, NULL, NULL, NULL, &tv );
	sleep_flag = FALSE;
#else
	tv.tv_sec  = delay.tv_sec;
	tv.tv_usec = delay.tv_usec;
	select( 0, NULL, NULL, NULL, &tv );
	WAIT_ADD( next_tick, add_frame );
	break;
#endif
      }

    }
#endif

    /* ���Ԍo�߂��Ď��������� (polling) */
  }



  /*
   * �����t���[���X�L�b�v����		( by floi, thanks ! )
   */
  if( use_auto_skip ){
    if( on_time ){			/* ���ԓ��ɏ����ł��� */
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
	next_tick = now;				/* �^�C�}����������  */
      }else{
	do_skip_draw = TRUE;			/* �`����X�L�b�v����K�v�L�� */
      }
    }
  }

  return;
}

#endif	/* UNIX_WAIT */





/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_wait_work[] =
{
  { TYPE_DOUBLE,	&wait_freq_hz,	},
};


int	suspend_wait( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_wait_work, 
			    countof(suspend_wait_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->wait ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_wait(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_wait_work, 
			   countof(suspend_wait_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}
