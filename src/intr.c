/************************************************************************/
/*									*/
/* 割り込みのエミュレート						*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "initval.h"
#include "intr.h"

#include "pc88cpu.h"
#include "pc88main.h"
#include "screen.h"
#include "keyboard.h"
#include "sound.h"
#include "joystick.h"

#include "emu.h"
#include "snddrv.h"
#include "wait.h"
#include "suspend.h"



int	intr_level;			/* OUT[E4] 割り込みレベル	*/
int	intr_priority;			/* OUT[E4] 割り込み優先度	*/
int	intr_sio_enable;		/* OUT[E6] 割り込みマスク SIO	*/ 
int	intr_vsync_enable;		/* OUT[E6] 割り込みマスク VSYNC	*/ 
int	intr_rtc_enable;		/* OUT[E6] 割り込みマスク RTC	*/ 



double	cpu_clock_mhz   = DEFAULT_CPU_CLOCK_MHZ;   /* MAIN CPU Clock   [MHz] */
double	sound_clock_mhz = DEFAULT_SOUND_CLOCK_MHZ; /* SOUND chip Clock [MHz] */
double	vsync_freq_hz   = DEFAULT_VSYNC_FREQ_HZ;   /* VSYNC 割込周期   [Hz]  */



#define	CPU_CLOCK_MHZ		cpu_clock_mhz
#define	SOUND_CLOCK_MHZ		sound_clock_mhz
#define	VSYNC_FREQ_HZ		vsync_freq_hz

#define	VRTC_RATE		(48.0/448.0)
#define	RTC_FREQ_HZ		(600)

#define	TIMER_A_CONST		(12)
#define	TIMER_B_CONST		(192)

#define	CPU_CLOCK		(CPU_CLOCK_MHZ * 1000000)

/*****************************************************************************/

int	state_of_cpu = 0;		/* メインCPUが処理した命令数	*/
int	state_of_vsync;			/* VSYNC周期のステート数	*/

int	cpu_load      = 0;		/* ウエイト 小さいほど遅い	*/
int	no_wait	      = FALSE;		/* ウエイトなし			*/


	int	RS232C_flag    = FALSE;	/* RS232C */
static	int	rs232c_intr_base;
static	int	rs232c_intr_timer;

	int	VSYNC_flag     = FALSE;	/* VSYNC */
static	int	vsync_intr_base;
static	int	vsync_intr_timer;

	int	ctrl_vrtc      = 1;	/* VRTC (垂直帰線中:1 / 表示中:0) */
static	int	vrtc_base;
static	int	vrtc_timer;

	int	RTC_flag       = FALSE;	/* RTC */
static	int	rtc_intr_base;
static	int	rtc_intr_timer;

	int	SOUND_flag     = FALSE;	/* SOUND Timer-A/Timer-B */
static	int	sd_A_intr_base;
static	int	sd_A_intr_timer;
static	int	sd_B_intr_base;
static	int	sd_B_intr_timer;
static	int	sd2_BRDY_intr_base;
static	int	sd2_BRDY_intr_timer;
static	int	sd2_EOS_intr_base;
static	int	sd2_EOS_intr_timer;




/*------------------------------------------------------
 * タイマー割り込みエミュレートのワークを初期化
 *	VSYNC / VRTC / RTC         ワークは起動時に初期化
 *	RS232C / Timer-A / TImer-B ワークは設定時に初期化
 *------------------------------------------------------*/

/*
 * 割り込みミュレート初期化 … Z80 の起動時に呼ぶ
 */
static	void	interval_work_init_generic( void )
{
  vsync_intr_timer = vsync_intr_base = CPU_CLOCK / VSYNC_FREQ_HZ;
  vrtc_timer       = vrtc_base       = (vsync_intr_base * VRTC_RATE);

  rtc_intr_timer   = rtc_intr_base   = CPU_CLOCK / RTC_FREQ_HZ;

  select_main_cpu = TRUE;
  dual_cpu_count  = 0;

  state_of_vsync = vsync_intr_base;
  state_of_cpu   = 0;
}


/*
 * RS232C割り込みエミュレート初期化 … Z80起動時に呼ぶ
 */
static	void	interval_work_init_RS232C( void )
{
  interval_work_set_RS232C( 0, 0 );
}

/*
 * サウンド割り込みエミュレート初期化 … Z80起動時に呼ぶ
 */
static	void	interval_work_init_TIMER_A( void )
{
  interval_work_set_TIMER_A();
  sd_A_intr_timer = sd_A_intr_base;
}
static	void	interval_work_init_TIMER_B( void )
{
  interval_work_set_TIMER_B();
  sd_B_intr_timer = sd_B_intr_base;
}




/*
 * 全割り込みエミュレート初期化 … Z80起動時や、クロック変更時などに呼ぶ
 */
void	interval_work_init_all( void )
{
  interval_work_init_generic();
  interval_work_init_RS232C();
  interval_work_init_TIMER_A();
  interval_work_init_TIMER_B();
}



/*
 * RS232C割り込みエミュレート再初期化 … RS232C割込設定時に呼ぶ
 */
void	interval_work_set_RS232C( int bps, int framesize )
{
  if( bps == 0 || framesize == 0 ){
    rs232c_intr_base = 0x7fffffff;
  }else{
    rs232c_intr_base = CPU_CLOCK / ( (double)bps / (double)framesize );
    if( rs232c_intr_base < 100 ) rs232c_intr_base = 100;
  }
  rs232c_intr_timer = rs232c_intr_base;
}




/************************************************************************/
/* 割り込みに関わる サウンドレジスタ更新時の処理			*/
/************************************************************************/
/*
 * サウンドのタイマプリセット値 変更時に呼ぶ
 */
void	interval_work_set_TIMER_A( void )
{
  sd_A_intr_base  = TIMER_A_CONST * sound_prescaler * (1024 - sound_TIMER_A)
					* ( CPU_CLOCK_MHZ / SOUND_CLOCK_MHZ );
}
void	interval_work_set_TIMER_B( void )
{
  sd_B_intr_base  = TIMER_B_CONST * sound_prescaler * (256 - sound_TIMER_B)
					* ( CPU_CLOCK_MHZ / SOUND_CLOCK_MHZ );
}

/*
 * サウンドの 各種フラグ および プリスケーラー値変更時に呼ぶ
 */
static	int	sound_flags_update     = 0;
static	int	sound_prescaler_update = 0;

void	change_sound_flags( int port )
{
  sound_flags_update = (int)port;
  refresh_intr_timing( z80main_cpu );
}
void	change_sound_prescaler( int new_prescaler )
{
  sound_prescaler_update = new_prescaler;
  refresh_intr_timing( z80main_cpu );
}




/*
 * サウンドのプリスケーラー値・各種フラグ変更時の際に、フラグを初期化する。
 *	割り込み更新後に呼ばれる。
 */
static	void	check_sound_parm_update( void )
{
  byte data;

  if( sound_prescaler_update ){		/* 分周 変更があったら		    */
					/* タイマ値を (変更後/変更前)倍して */
					/* タイマ値のつじつまをあわせる。   */
    sd_A_intr_base  = sd_A_intr_base  * sound_prescaler_update/sound_prescaler;
    sd_A_intr_timer = sd_A_intr_timer * sound_prescaler_update/sound_prescaler;
    sd_B_intr_base  = sd_B_intr_base  * sound_prescaler_update/sound_prescaler;
    sd_B_intr_timer = sd_B_intr_timer * sound_prescaler_update/sound_prescaler;
    sound_prescaler = sound_prescaler_update;
    sound_prescaler_update = 0;
  }


  switch( sound_flags_update ){		/* フラグ変更があったら、更新 */

  case 0x27:		/*---------------- RESET_B/A | ENABLE_B/A | LOAD_B/A */
    data = sound_reg[0x27];

					/* LOADの立ち上がりに、タイマ値更新 */
    if( (sound_LOAD_A==0) && (data&0x01) ) sd_A_intr_timer = sd_A_intr_base;
    if( (sound_LOAD_B==0) && (data&0x02) ) sd_B_intr_timer = sd_B_intr_base;
    sound_LOAD_A = data & 0x01;
    sound_LOAD_B = data & 0x02;

					/* ENABLE を保存 */
    sound_ENABLE_A = ( data & 0x04 ) ? 1 : 0;
    sound_ENABLE_B = ( data & 0x08 ) ? 1 : 0;

					/* RESET が 1 なら、フラグクリア */
    if( data & 0x10 ) sound_FLAG_A = 0;
    if( data & 0x20 ) sound_FLAG_B = 0;
    break;

  case 0x29:		/*---------------- EN_ZERO/BRDY/EOS/TB/TA  */
    if( sound_board==SOUND_II ){
      data = sound_reg[0x29];
      sound2_EN_TA   = data & 0x01;
      sound2_EN_TB   = data & 0x02;
      sound2_EN_EOS  = data & 0x04;
      sound2_EN_BRDY = data & 0x08;
      sound2_EN_ZERO = data & 0x10;
    }
    break;

  case 0x10:		/*---------------- IRQ_RET | MSK_ZERO/BRDY/EOS/TB/TA */
    data = sound2_reg[0x10];
    if( data & 0x80 ){
      sound_FLAG_A     = 0;
      sound_FLAG_B     = 0;
      sound2_FLAG_EOS  = 0;
      sound2_FLAG_BRDY = 0;
      sound2_FLAG_ZERO = 0;
    }else{
      sound2_MSK_TA   = data & 0x01;
      sound2_MSK_TB   = data & 0x02;
      sound2_MSK_EOS  = data & 0x04;
      sound2_MSK_BRDY = data & 0x08;
      sound2_MSK_ZERO = data & 0x10;
    }
    break;

  }

  sound_flags_update = 0;

}


/*
 * サウンドボードII関連
 */
void	interval_work_set_BDRY( void )
{
  sd2_BRDY_intr_base  = sound2_intr_base * 2 * ( CPU_CLOCK_MHZ / 4.0 );
  sd2_BRDY_intr_timer = sd2_BRDY_intr_base;

/*printf("%d\n",sd2_BRDY_intr_base);*/
}
void	interval_work_set_EOS( int length )
{
  sd2_EOS_intr_base  = sd2_BRDY_intr_base * length;
  sd2_EOS_intr_timer = sd2_EOS_intr_base;

/*printf("%d\n",sd2_EOS_intr_base);*/
}



/************************************************************************/
/* 1/60 sec の割り込み毎に行なう処理					*/
/************************************************************************/

/* #define だと、VSYNC開始時に表示、#undef だとVBLANK 終了時に表示 */
#undef	DRAW_SCREEN_AT_VSYNC_START

int snd = 0;

static	void	vsync( void )
{
//	if(snd == 0)
  xmame_sound_update();			/* サウンド出力 */
//   snd = !snd;

  if( !no_wait ) wait_vsync();		/* 実時間でウエイトを取る */


  xmame_update_video_and_audio();	/* サウンド出力 その2 */

  scan_joystick();                      /* Joystick 入力 */
  scan_keyboard(0);			/* キー入力	*/
  scan_mouse();				/* マウス入力	*/
  scan_expose();
#ifdef	DRAW_SCREEN_AT_VSYNC_START
  draw_screen();			/* 画面出力	*/
#endif

  scan_focus();				/* ウインドウフォーカスチェック	*/


  state_of_cpu -= state_of_vsync;	/* (== vsync_intr_base) */
}






/*----------------------------------------------------------------------*/
/* 割り込みを生成する。と同時に、次の割り込みまでの、最小 state も計算	*/
/*	帰り値は、Z80処理強制終了のフラグ(TRUE/FALSE)			*/
/*----------------------------------------------------------------------*/
#define	MIN(x,y)	(((x)<(y))?(x):(y))
int	main_INT_update( void )
{

  int	icount = rtc_intr_base;		/* 次の割り込み発生までの最小state数 */
					/* とりあえず、RTC割込の周期で初期化 */


		/* -------- RS232C 割り込み -------- */

  rs232c_intr_timer -= z80main_cpu.state;
  if( rs232c_intr_timer < 0 ){
    rs232c_intr_timer += rs232c_intr_base;
    if( sio_intr() ){
      RS232C_flag = TRUE;
    }
  }
  icount = MIN( icount, rs232c_intr_timer );


		/* -------- VSYNC 割り込み -------- */

  state_of_cpu += z80main_cpu.state;

  vsync_intr_timer -= z80main_cpu.state;
  if( vsync_intr_timer < 0 ){
    vsync();					/* ウエイト、表示、入力 */
    ctrl_vrtc  = 1;				/* VBLANK開始 */
    VSYNC_flag = TRUE;
    vsync_intr_timer += vsync_intr_base;
  }
  icount = MIN( icount, vsync_intr_timer );


		/* -------- VRTC 処理 -------- */

  if( ctrl_vrtc ){
    if( vrtc_timer < 0 ){
      vrtc_timer = vrtc_base;
    }else{
      vrtc_timer -= z80main_cpu.state;
      if( vrtc_timer < 0 ){
	ctrl_vrtc  = 0;				/* VBLANK 終了 */
	/*VSYNC_flag = FALSE;*/			/* VSYNC信号はOFFしない */
		/* OFFすると鍵穴殺人事件とかが動かない   thanks peach ! */
#ifndef	DRAW_SCREEN_AT_VSYNC_START
	draw_screen();				/* 画面出力	*/
#endif
      }
    }
  }
  if( ctrl_vrtc ) icount = MIN( icount, vrtc_timer );


		/* -------- RTC 割り込み -------- */

  rtc_intr_timer -= z80main_cpu.state;
  if( rtc_intr_timer < 0 ){
    RTC_flag = TRUE;
    rtc_intr_timer += rtc_intr_base;
  }
  icount = MIN( icount, rtc_intr_timer );


		/* -------- SOUND TIMER A 割り込み -------- */

  if( sound_LOAD_A ){
    sd_A_intr_timer -= z80main_cpu.state;
    if( sd_A_intr_timer < 0 ){
      xmame_sound_timer_over(0);
      sd_A_intr_timer += sd_A_intr_base;
      if( sound_ENABLE_A ){
	if( sound2_MSK_TA ) sound_FLAG_A = 0;
	else                sound_FLAG_A = 1;
	if( sound_FLAG_A && sound2_EN_TA ) SOUND_flag = TRUE;
      }
    }
    icount = MIN( icount, sd_A_intr_timer );
  }


		/* -------- SOUND TIMER B 割り込み -------- */

  if( sound_LOAD_B ){
    sd_B_intr_timer -= z80main_cpu.state;
    if( sd_B_intr_timer < 0 ){
      xmame_sound_timer_over(1);
      sd_B_intr_timer += sd_B_intr_base;
      if( sound_ENABLE_B ){
	if( sound2_MSK_TB ) sound_FLAG_B = 0;
	else                sound_FLAG_B = 1;
	if( sound_FLAG_B && sound2_EN_TB ) SOUND_flag = TRUE;
      }
    }
    icount = MIN( icount, sd_B_intr_timer );
  }

  if( sound2_FLAG_PCMBSY ){

    sd2_BRDY_intr_timer -= z80main_cpu.state;
    if( sd2_BRDY_intr_timer < 0 ){
      sd2_BRDY_intr_timer += sd2_BRDY_intr_base;
      {
	if( sound2_MSK_BRDY ) sound2_FLAG_BRDY = 0;
	else                  sound2_FLAG_BRDY = 1;
	if( sound2_FLAG_BRDY && sound2_EN_BRDY ) SOUND_flag = TRUE;
      }
    }

    if( sound2_notice_EOS ){
      sd2_EOS_intr_timer -= z80main_cpu.state;
      if( sd2_EOS_intr_timer < 0 ){
	sd2_EOS_intr_timer += sd2_EOS_intr_base;
	if( sound2_MSK_EOS ) sound2_FLAG_EOS = 0;
	else                 sound2_FLAG_EOS = 1;
	if( sound2_FLAG_EOS && sound2_EN_EOS ) SOUND_flag = TRUE;
	if( !sound2_repeat )  sound2_FLAG_PCMBSY = 0;
	sound2_notice_EOS = FALSE;
      }
    }

  }


	/* サウンドの、割り込みに関わるレジスタが変更されてないか確認 */

  check_sound_parm_update();


	/* 次の割り込み発生までの、ステート数をセット */

  if( cpu_load ) icount = MIN( icount, cpu_load );

  z80main_cpu.icount = icount;
  z80main_cpu.state  = 0;


	/* Ctrl-C や、F12 キー入力の際は、ここで通知 */

  if( dual_cpu_count ){ dual_cpu_count--; return 1; }

  if( emu_mode!=EXECUTE ) return 1;
  else                    return 0;
}






/************************************************************************/
/* マスカブル割り込みエミュレート					*/
/************************************************************************/
/*--------------------------------------*/
/* 初期化 (Z80リセット時に呼ぶ)		*/
/*--------------------------------------*/
void	main_INT_init( void )
{
  RS232C_flag  = FALSE;
  VSYNC_flag   = FALSE;
  RTC_flag     = FALSE;
  SOUND_flag   = FALSE;

  interval_work_init_all();
  ctrl_vrtc = 1;

/*
printf("CPU    %f\n",cpu_clock_mhz);
printf("SOUND  %f\n",sound_clock_mhz);
printf("SYNC   %f\n",vsync_freq_hz);
printf("RS232C %d\n",rs232c_intr_base);
printf("VSYNC  %d\n",vsync_intr_base);
printf("VRTC   %d\n",vrtc_base);
printf("RTC    %d\n",rtc_intr_base);
printf("A      %d\n",sd_A_intr_base);
printf("B      %d\n",sd_B_intr_base);
*/
}

/*----------------------------------------------*/
/* チェック (割込許可時 1ステップ毎に呼ばれる)	*/
/*						*/
/*	謎１）IM!=2 の時は割り込みはどうなる？	*/
/*	謎２）intr_priority が真の時は、	*/
/*	      割り込み状態はどうなる？		*/
/*		  (現状、無視)			*/
/*						*/
/*----------------------------------------------*/
int	main_INT_chk( void )
{
  int	intr_no = -1;

  /*  if( z80main_cpu.IM!=2 ) return -1;*/


  if( intr_level==0 ){				/* レベル設定 0 */
    {						/*    割り込みは受け付けない */
      intr_no = -1;
    }
  }
  else if( intr_level>=1 &&			/* レベル設定 1 */
	   intr_sio_enable && RS232C_flag ){	/*    RS232S 受信 割り込み */
    RS232C_flag = FALSE;
    intr_no = 0;
  }
  else if( intr_level>=2 &&			/* レベル設定 2 */
	   intr_vsync_enable && VSYNC_flag ){	/*    VSYNC 割り込み */
    VSYNC_flag = FALSE;
    intr_no = 1;
  }
  else if( intr_level>=3 &&			/* レベル設定 3 */
	   intr_rtc_enable && RTC_flag ){	/*    1/600秒 RTC 割り込み */
    RTC_flag = FALSE;
    intr_no = 2;
  }
  else if( intr_level>=5 &&			/* レベル設定 5 */
	   intr_sound_enable && SOUND_flag ){	/*    SOUND TIMER 割り込み */
    SOUND_flag = FALSE;
    intr_no = 4;
  }


  if( intr_no >= 0 ){
    if( intr_priority ) intr_level = 7;
    else                intr_level = intr_no;
    return intr_no;
  }else{
    return -1;
  }
}








/****************************************************************/	
/* サスペンド／レジューム					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_intr_work[]=
{
  { TYPE_INT,	&intr_level,		},
  { TYPE_INT,	&intr_priority,		},
  { TYPE_INT,	&intr_sio_enable,	},
  { TYPE_INT,	&intr_vsync_enable,	},
  { TYPE_INT,	&intr_rtc_enable,	},

  { TYPE_DOUBLE,&cpu_clock_mhz,		},
  { TYPE_DOUBLE,&sound_clock_mhz,	},
  { TYPE_DOUBLE,&vsync_freq_hz,		},

  { TYPE_INT,	&state_of_cpu,		},
  { TYPE_INT,	&state_of_vsync,	},

  { TYPE_INT,	&cpu_load,		},
  { TYPE_INT,	&no_wait,		},

  { TYPE_INT,	&RS232C_flag,		},
  { TYPE_INT,	&rs232c_intr_base,	},
  { TYPE_INT,	&rs232c_intr_timer,	},

  { TYPE_INT,	&VSYNC_flag,		},
  { TYPE_INT,	&vsync_intr_base,	},
  { TYPE_INT,	&vsync_intr_timer,	},

  { TYPE_INT,	&ctrl_vrtc,		},
  { TYPE_INT,	&vrtc_base,		},
  { TYPE_INT,	&vrtc_timer,		},

  { TYPE_INT,	&RTC_flag,		},
  { TYPE_INT,	&rtc_intr_base,		},
  { TYPE_INT,	&rtc_intr_timer,	},

  { TYPE_INT,	&SOUND_flag,		},
  { TYPE_INT,	&sd_A_intr_base,	},
  { TYPE_INT,	&sd_A_intr_timer,	},
  { TYPE_INT,	&sd_B_intr_base,	},
  { TYPE_INT,	&sd_B_intr_timer,	},

  { TYPE_INT,	&sound_flags_update,	},
  { TYPE_INT,	&sound_prescaler_update,},

  { TYPE_INT,	&sd2_BRDY_intr_base,	},
  { TYPE_INT,	&sd2_BRDY_intr_timer,	},
  { TYPE_INT,	&sd2_EOS_intr_base,	},
  { TYPE_INT,	&sd2_EOS_intr_timer,	},
};


int	suspend_intr( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_intr_work, 
			    countof(suspend_intr_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->intr ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_intr(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_intr_work, 
			   countof(suspend_intr_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}



void	main_INT_init_at_resume( void )
{
  /* レジューム後の再初期化 */
  /* 不要 ……… なのか？ */
}
