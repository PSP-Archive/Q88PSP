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

// #include <SDL.h>
#include "pg.h"
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
double long  next_time;
double long base_time;

int	wait_vsync_init( void )			/* ウェイト処理初期化 */
{
	wait_vsync_reset();
	return TRUE;
}


void	wait_vsync_term( void )			/* ウェイト処理終了 */
{
}


//double long wait_base = 830000;
//double long wait_base = 1095000;//pgWaitVがないとき
//double long wait_base = 1000620;//毎回Wait時
double long wait_base = 521500;
double long count_time = 0;
double long count_base = 0;
void	wait_vsync_reset( void )		/* ウェイト処理再初期化 */
{
	next_time = sceKernelLibcClock() + (unsigned long)(wait_base/60);
//	base_time = sceKernelLibcClock();
//	count_base = base_time;
}


void	wait_vsync( void )			/* 実際のウェイト処理 */
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
				next_time = now;			/*    タイマ値再初期化      */
				on_time = TRUE;
				pgWaitV();		/* for AUDIO thread ?? */
			}else{
			}
//		    next_time += (unsigned long)(wait_base/60);
//			do_update = TRUE;
//			return;
			break;
		}

	//この辺で累積動作時間表示してみる。これでWaitの調整が出来るはず。
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
	    if((now - next_time) > (unsigned long)(wait_base/60)) break;//割り込み時間より現時刻が割り込み間隔分より大きい場合
//		if((next_time - now) > (unsigned long)(wait_base/60)*1000) break;//割り込み時間がはるかかなたの場合。主に進みすぎて回ってしまった場合をさす。
//		if(next_time <= now ) break;
		if(((next_time - now) >= 20) || ((next_time < 20) && (20 < (next_time + (0xFFFFFFFF - now))))) pgWaitV();		/* for AUDIO thread ?? */
//		pgWaitV();		/* for AUDIO thread ?? */
    }
//	next_time = now + (unsigned long)(wait_base/60);
    next_time += (unsigned long)(wait_base/60);

  /*
   * 自動フレームスキップ処理		( by floi, thanks ! )
   */
  if( use_auto_skip ){
    if( on_time ){			/* 時間内に処理できた */
/*if(skip_counter)printf("%x\n",skip_counter);*/
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
	next_time = now;				/* タイマ調整し直し  */
      }else{
	do_skip_draw = TRUE;			/* 描画をスキップする必要有り */
      }
    }
  }

//  do_update = TRUE;
  return;
 #endif
}


/****************************************************************/	
/* サスペンド／レジューム					*/
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
