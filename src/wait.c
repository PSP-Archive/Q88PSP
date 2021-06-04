/************************************************************************/
/*									*/
/* ウエイト調整用関数 (OS依存)						*/
/*									*/
/* 【関数】								*/
/*									*/
/* int  wait_vsync_init( void )		ウェイト処理初期化		*/
/* void wait_vsync_term( void )		ウェイト処理初期化		*/
/* void	wait_vsync_reset( void )	ウェイト再初期化 (設定変更時)	*/
/* void wait_vsync( void )		ウェイト			*/
/*									*/
/************************************************************************/
#include <stdio.h>

#include <SDL.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"
#include "suspend.h"

#include "screen.h"	/* auto_skip... */




double	wait_freq_hz = DEFAULT_WAIT_FREQ_HZ;	/* ウエイト調整用周波数 [Hz] */
int	wait_by_sleep = FALSE;			/* ウエイト期間中 sleep する */

long	wait_sleep_min_us = 100;		/* 残り idle時間が、この μs
						   以下なら sleep しない
						   ( 1秒未満で設定すること! )*/
						/* UNIX_WAIT 指定時のみ有効  */


/*
 * 自動フレームスキップ		( by floi, thanks ! )
 */

static	int	skip_counter = 0;		/* 何回連続でスキップしたか */
static	int	skip_count_max = 15;		/* 最大スキップ回数
						   これ以上になったら現在時刻をリセット */



/*
 * ウェイト処理関数群
 */

#ifndef	UNIX_WAIT		/* === 通常 (SDL) を使う場合はこちら === */

static	Uint32 next_time;

int	wait_vsync_init( void )			/* ウェイト処理初期化 */
{
  if( ! SDL_WasInit( SDL_INIT_TIMER ) ){		/* SDLタイマ機能 init */
    if( SDL_InitSubSystem( SDL_INIT_TIMER ) != 0 ){
      if( verbose_wait ) printf( "Error Wait (SDL)\n" );
    }
  }

  wait_vsync_reset();
  return TRUE;
}


void	wait_vsync_term( void )			/* ウェイト処理終了 */
{
}



void	wait_vsync_reset( void )		/* ウェイト処理再初期化 */
{
  next_time = SDL_GetTicks() + (Uint32)(1000/wait_freq_hz);
}





void	wait_vsync( void )			/* 実際のウェイト処理 */
{
  int	on_time = FALSE;
  Uint32 now;

  if( wait_by_sleep ){	/* 時間が来るまで sleep する */

    now = SDL_GetTicks();
    if ( next_time <= now ) {				    /* 処理時間over */
      SDL_Delay( 1 );		/* for AUDIO thread ?? */
    }else{						    /* 余り時間あり */
      SDL_Delay( next_time-now );
      on_time = TRUE;
    }
    next_time = SDL_GetTicks() + (Uint32)(1000/wait_freq_hz);


  }else{		/* 時間が来るまで Tick を監視 */

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
   * 自動フレームスキップ処理		( by floi, thanks ! )
   */
  if( use_auto_skip ){
    if( on_time ){			/* 時間内に処理できた */
      skip_counter = 0;
      do_skip_draw = FALSE;
      if( already_skip_draw ){		/* 既に描画をスキップしていたら */
	already_skip_draw = FALSE;
	reset_frame_counter();		/* 次は必ず描画する */
      }
    }else{				/* 時間内に処理できていない */
      skip_counter++;
      if( skip_counter >= skip_count_max ){	/* スキップしすぎ */
	skip_counter = 0;
	do_skip_draw = FALSE;
	already_skip_draw = FALSE;
	reset_frame_counter();			/* 次は必ず描画する */
      }else{
	do_skip_draw = TRUE;			/* 描画をスキップする必要有り */
      }
    }
  }

  return;
}


#else	/* UNIX_WAIT */		/* === UNIX の場合、こちらも選択可能 === */
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#define	HAVE_GETTIMEOFDAY	/* gettimeofday() がない場合は、#undef する */
#define	HAVE_SELECT		/* select()       がない場合は、#undef する */


typedef struct{			/* struct timeval と同じ */
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

static	int	wait_error = FALSE;		/* 時刻取得で異常発生時、真 */


/*
 * 現在時刻取得関数 int get_now_timeval( T_WAIT_TIMEVAL *now )
 *	正常時には、0 を返す
 */

#ifdef  HAVE_GETTIMEOFDAY	/* -------- gettimeofday() を使用する ------ */
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


#else				/* -------- clock() を使用する ------------- */
#include <time.h>

/* #define CLOCK_SLICE	CLK_TCK */		/* これじゃ駄目？ */
#define	CLOCK_SLICE	CLOCKS_PER_SEC		/* こっちが正解？ */

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



static	T_WAIT_TIMEVAL	next_tick;		/* 次回ウエイト調整時刻 */
static	T_WAIT_TIMEVAL	add_frame;		/* ウエイトの周期	*/
static	T_WAIT_TIMEVAL	sleep_min;		/* ウエイトの最小期間	*/

void	wait_vsync_reset( void )
{
  if( get_wait_timeval( &next_tick ) ){
    wait_error = TRUE;

  }else{
    wait_error = FALSE;

    add_frame.tv_sec  = 0;				/* ウエイトの周期 */
    add_frame.tv_usec = 1000000 / wait_freq_hz;

    sleep_min.tv_sec  = 0;				/* sleep 最小時間 */
    sleep_min.tv_usec = wait_sleep_min_us;

    WAIT_ADD( next_tick, add_frame );			/* 次回周期の時刻 */
  }
}


void	wait_vsync( void )
{
  int	on_time = FALSE;
  T_WAIT_TIMEVAL now, delay;
  int sleep_flag = wait_by_sleep;	/* idle を sleep して過ごそうフラグ */

  if( wait_error ) return;

  while( 1 ){

    if( get_wait_timeval( &now ) ){		/* 現在時刻取得 */
      wait_error = TRUE;
      return;
    }

					/* ウェイト周期内に処理未完 ! ====== */

    if( WAIT_CMP( next_tick, now, <= ) ){
#if 0
      next_tick = now;
      on_time = TRUE;
#else
      WAIT_SUB( delay, now, next_tick );	/* どの位の遅れかを算出。   */
      if( WAIT_CMP( add_frame, delay, < ) ){	/* 1周期以上遅れていれば、  */
	next_tick = now;			/*    タイマ値再初期化      */
	on_time = TRUE;
      }else{					/* そんなに遅れてないなら、 */
	;					/*    そのまま続行          */
      }
#endif
      WAIT_ADD( next_tick, add_frame );
      break;
    }


					/* ウェイト周期内に処理完了 ! ====== */
    on_time = TRUE;

#ifdef	HAVE_SELECT
    if( sleep_flag ){

      WAIT_SUB( delay, next_tick, now );	/* 残り時間を算出。        */

      if( WAIT_CMP( delay, sleep_min, < ) ){	/* 残り少ない場合は、      */
	sleep_flag = FALSE;			/* 	polling して過ごす */

      }else{					/* 結構残ってる場合は、    */
	struct timeval tv;			/*	sleep して過ごす   */
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

    /* 時間経過を監視し続ける (polling) */
  }



  /*
   * 自動フレームスキップ処理		( by floi, thanks ! )
   */
  if( use_auto_skip ){
    if( on_time ){			/* 時間内に処理できた */
      skip_counter = 0;
      do_skip_draw = FALSE;
      if( already_skip_draw ){		/* 既に描画をスキップしていたら */
	already_skip_draw = FALSE;
	reset_frame_counter();		/* 次は必ず描画する */
      }
    }else{				/* 時間内に処理できていない */
      skip_counter++;
      if( skip_counter >= skip_count_max ){	/* スキップしすぎ */
	skip_counter = 0;
	do_skip_draw = FALSE;
	already_skip_draw = FALSE;
	reset_frame_counter();			/* 次は必ず描画する */
	next_tick = now;				/* タイマ調整し直し  */
      }else{
	do_skip_draw = TRUE;			/* 描画をスキップする必要有り */
      }
    }
  }

  return;
}

#endif	/* UNIX_WAIT */





/****************************************************************/	
/* サスペンド／レジューム					*/
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
