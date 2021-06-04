/************************************************************************/
/* QUASI88 --- PC-8801 emulator						*/
/*	 Copyright (C) Showzoh Fukunaga 1998,1999,2000			*/
/*									*/
/*									*/
/*	  このソフトは、UNIX + X Window System の環境で動作する、	*/
/*	PC-8801 のエミュレータです。					*/
/*									*/
/*	  このソフトの作成にあたり、Marat Fayzullin氏作の fMSX、	*/
/*	Nicola Salmoria氏 ( MAME/XMAME project) 作の mame/xmame、	*/
/*	ゆみたろ氏作の PC6001V のソースを参考にさせてもらいました。	*/
/*									*/
/*	＊注意＊							*/
/*	  サウンドドライバは、mame/xmame のソースを流用しています。	*/
/*	この部分のソースの著作権は、mame/xmame チームあるいはソースに	*/
/*	記載してある著作者にあります。					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "quasi88.h"
#include "initval.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "memory.h"
#include "file-op.h"

#include "emu.h"
#include "drive.h"
#include "getconf.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "indicator.h"
#include "suspend.h"
#include "snapshot.h"

#include "exmem.h"

#include "menu.h"

#include <string.h>

int pg_y = 0;

int	verbose_level	= DEFAULT_VERBOSE;	/* 冗長レベル		*/
int	verbose_proc    = FALSE;		/* 処理の進行状況の表示	*/
int	verbose_z80	= FALSE;		/* Z80処理エラーを表示	*/
int	verbose_io	= FALSE;		/* 未実装I/Oアクセス表示*/
int	verbose_pio	= FALSE;		/* PIO の不正使用を表示 */
int	verbose_fdc	= FALSE;		/* FDイメージ異常を報告	*/
int	verbose_wait	= FALSE;		/* ウエイト時の異常を報告 */
int	verbose_suspend = FALSE;		/* サスペンド時の異常を報告 */
int	verbose_snd	= FALSE;		/* サウンドのメッセージ	*/


/*----------------------------------------------------------------------*/
/* ログを取るための準備／片付け						*/
/*----------------------------------------------------------------------*/

#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
FILE	*LOG = NULL;
#endif

static	void	debug_log_init( void )
{
#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
  LOG = ex_fopen("log","w");
#endif
  if( verbose_proc ){
#if	defined( PIO_DISP ) || defined( PIO_FILE )
    printf("+ Support PIO logging. set variable \"pio_debug\" to 1.\n");
#endif
#if	defined( FDC_DISP ) || defined( FDC_FILE )
    printf("+ Support FDC logging. set variable \"fdc_debug\" to 1.\n");
#endif
#if	defined( MAIN_DISP ) || defined( MAIN_FILE )
    printf("+ Support Main Z80 logging. set variable \"main_debug\" to 1.\n");
#endif
#if	defined( SUB_DISP ) || defined( SUB_FILE )
    printf("+ Support Sub Z80 logging. set variable \"sub_debug\" to 1.\n");
#endif
  }
}

static	void	debug_log_finish( void )
{
#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
  if( LOG ) ex_fclose(LOG);
#endif
}

#include "psp_callback.h"
#include "q88psp_snd.h"

int out_wave = 0;
int buf_pbptr = 0;


//44100,chan:2固定

// downconversion to 22050
// count = number of frames, 1frame = sample * channel
// count = 0xFF
// sndbuflen = 0x4000
static void wavout_snd0_callback(short *buf, unsigned long count)
{
	int len,i,max;

	if (!out_wave)
	{
		memset(buf,0,count*4);
		return;
	}

	for (i = 0; i < count<<1;)
	{
//		if((snd_ptr == (buf_pbptr + 1)) || ((snd_ptr == 0) && (SND_BUFLEN-1 == buf_pbptr))) break;

		buf[i++] = snd_buffer[buf_pbptr];
		buf[i++] = snd_buffer[buf_pbptr];
		buf[i++] = snd_buffer[buf_pbptr];
		buf[i++] = snd_buffer[buf_pbptr];
		
		if(snd_ptr == buf_pbptr) break;

		if (buf_pbptr < SND_BUFLEN-1)
			buf_pbptr++;
		else
			buf_pbptr=0;
	}
}

//return 0 if success
int wavoutInit()
{
	pgaSetChannelCallback(0,wavout_snd0_callback);
	return 0;
}

/************************************************************************/
/* QUASI88 メイン関数							*/
/*	引数解析 → 初期化 → エミュレート関数呼び出し → 終了		*/
/************************************************************************/
static	int	proc;

// renamed for PSP
int	xmain( int argc, char *argv )
{
  int status;
  char temp_dir[MAXPATH];
  char *p;
  char *Name;

  pgInit();

  SetupCallbacks();
  pgScreenFrame(1,0);

  wavoutInit();
	init_ftable();//ファイルテーブル初期化

  pgFillvram(0);
  pgScreenFlipV();
  pgFillvram(0);
  pgScreenFlipV();

  pgcLocate(0,0);
  pgPrint_drawbg(0,0,0xFFFF,0,"QUASI88 --- started ---");
  pg_y++;

  proc = 0;

//  strcpy(temp_dir,"ms0:/PSP/");//ms0:/PSPのルートフォルダにしてみたらどうか。
//  *++p = 0;
//  ex_setdir(temp_dir);

  strcpy(temp_dir,argv);
  p = strrchr(temp_dir,'/');
  *++p = 0;
  ex_setdir(temp_dir);
  strcpy(path_main,temp_dir);

//	menu_init_psp();
//	add_diskimage("disk.d88",0,0);
//	Drive1 = "disk.d88";
//	DiskIn[0] = 1;
//	DiskIn[1] = 1;

	menu_init_psp();
	//Boot Menu
	switch(menu_psp(0)){
		case Menu_exit:
			break;
	}

  osd_get_environment( &status );			/* 環境変数取得	*/
  if( status & ENV_NO_HOME )
    fprintf( stderr, "<warning> ${HOME} is not defined.\n" );
  if( status & ENV_INVALID_HOME )
    fprintf( stderr, "<warning> ${HOME} is invalid.\n" );
  if( status & ENV_NO_MEM_ROM_DIR )
    fprintf( stderr, "<warning> Malloc Failed (ROM_DIR)" );
  if( status & ENV_NO_MEM_DISK_DIR )
    fprintf( stderr, "<warning> Malloc Failed (DISK_DIR)" );

  xmame_system_init();

//  if( ! config_init( argc, argv ) ) main_exit(1);	/* 引数処理	*/

//  if( ! config_init( 1, &argv ) ) 
//		main_exit(1);	/* 引数処理	*/

//  add_diskimage("disk.d88",0,0);
  verbose_level = 1;

  verbose_proc	= verbose_level & 0x01;
  verbose_z80	= verbose_level & 0x02;
  verbose_io	= verbose_level & 0x04;
  verbose_pio	= verbose_level & 0x08;
  verbose_fdc	= verbose_level & 0x10;
  verbose_wait	= verbose_level & 0x20;
  verbose_suspend=verbose_level & 0x40;
  verbose_snd	= verbose_level & 0x80;


  if( verbose_proc ){
    if( exist_rcfile() )
      printf( "${HOME}/" QUASI88RC_FILE " read and initialized\n");
    else
      printf( "${HOME}/" QUASI88RC_FILE " open failed!\n");
  }

  resume_init();			/* サスペンド・レジューム関連初期化 */
  screen_snapshot_init();		/* 画面スナップショット関連初期化 */


  if( graphic_system_init() ){		/* グラフィックシステム初期化	*/
    proc = 1;

    if( xmame_sound_start() ){		/* サウンドドライバ初期化	*/
      proc = 2;

      if( memory_allocate() ){		/* エミュレート用メモリの確保	*/
	proc = 3;

	pgPrint_drawbg(0,pg_y++,0xFFFF,0,"QUASI88 memory clear");

//	set_signal();			/* INTシグナルの処理を設定	*/

	drive_init();			/* フロッピードライブの初期化	*/

	sio_open_tapeload();
	sio_open_tapesave();
	sio_open_serialin();
	sio_open_serialout();
	printer_open();

	if( resume_flag == 0 ){
	  disk_set_args_file();		/*   引数のファイルをセット	*/

	  bootup_work_init();		/* 引数に応じて、ワークを初期化	*/
	  pc88main_init(TRUE);
	  pc88sub_init();

	  power_on_ram_init();		/* RAMの初期化(電源投入直後のみ)*/

	}else{				/* レジュームする場合		*/

	  if( file_resume == NULL ||		/* ファイル名が未定義、	   */
	      ! resume( file_resume ) ){	/* もしくは レジューム失敗 */

	    fprintf( stderr, "resume: Failed ! (filename = %s)\n",
		     file_resume ? file_resume
				 : "${HOME}/" QUASI88STATE_FILE );
	    main_exit(1);
	  }

	  fdc_init_at_resume();

	  pc88main_init_at_resume();
	  pc88sub_init_at_resume();

	}

	debug_log_init();

	if( wait_vsync_init() ){	/* ウエイト用タイマー初期化	*/
	  proc = 4;

	  pgPrint_drawbg(0,pg_y++,0xFFFF,0,"Running");
	  out_wave = 1;

	  if( verbose_proc ) printf( "Running QUASI88...\n" );




	  if( resume_flag == 0 )
	    indicate_bootup_logo();	/* タイトルロゴ表示 */
	  emu();			/* エミュレート メイン */

	  if( verbose_proc ) printf( "Shutting down.....\n" );
	  wait_vsync_term();
	}

	debug_log_finish();

	pc88main_term();
	pc88sub_term();

	disk_eject( 0 );
	disk_eject( 1 );

	sio_close_tapeload();
	sio_close_tapesave();
	sio_close_serialin();
	sio_close_serialout();
	printer_close();

	memory_free();
      }
      xmame_sound_stop( );
    }
    graphic_system_term( );
  }



	/* 詳細表示をしないかった場合の、エラー表示 */

  if( !verbose_proc ){
    switch( proc ){
    case 0:	fprintf(stderr,"graphic system initialize failed!\n");	break;
    case 1:	fprintf(stderr,"sound system initialize failed!\n");	break;
    case 2:	fprintf(stderr,"memory allocate failed!\n");		break;
    case 3:	fprintf(stderr,"timer initialize failed!\n");		break;
    }
  }

  xmame_system_term();

	//一応クロックを222MHzに戻す
	scePowerSetClockFrequency(222,222,111);

  return (0);
}




/************************************************************************/
/* QUASI88 途中終了処理関数						*/
/*	exit() の代わりに呼ぼう。					*/
/*	引数の val が負だと、グラフィックの後始末がされないので、注意。	*/
/************************************************************************/
void	main_exit( int val )
{
  switch( proc ){
  case 4:
    wait_vsync_term();			/* FALLTHROUGH */
  case 3:
    debug_log_finish();
    pc88main_term();
    pc88sub_term();
    disk_eject( 0 );
    disk_eject( 1 );
    memory_free();			/* FALLTHROUGH */
  case 2:
    xmame_sound_stop( );		/* FALLTHROUGH */
  case 1:
    if( val>= 0 ) graphic_system_term( );
  }

  xmame_system_term();

  exit( val );
}





/************************************************************************/
/* QUASI88 起動中のリセット処理関数					*/
/************************************************************************/
void	main_reset( void )
{
  int	empty[2];

  pc88main_term();
  pc88sub_term();
  pc88main_init(FALSE);
  pc88sub_init();

  pc88main_break_point();
  pc88sub_break_point();

  empty[0] = drive_check_empty(0);
  empty[1] = drive_check_empty(1);
  drive_reset();
  if( empty[0] ) drive_set_empty(0);
  if( empty[1] ) drive_set_empty(1);
  if( use_sound ) xmame_sound_reset();
}
