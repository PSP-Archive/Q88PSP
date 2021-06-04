/************************************************************************/
/*									*/
/* メニューモード							*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "initval.h"
#include "menu.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "screen.h"
#include "joystick.h"

#include "emu.h"
#include "drive.h"
#include "image.h"
#include "indicator.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "file-op.h"
#include "suspend.h"
#include "snapshot.h"
#include "fdc.h"
#include "sound.h"

#include "menu-event.h"
#include "q8tk.h"

#include "pg.h"

extern	char	*dir_tape;

int	menu_lang	= LANG_JAPAN;		/* メニューの言語	*/


static	void	set_menu_dirname( void );

/*--------------------------------------------------------------*/
/* メニューでの表示メッセージは全て、このファイルの中に		*/
/*--------------------------------------------------------------*/
#include "message.c"




/****************************************************************/
/* ワーク							*/
/****************************************************************/

static	int	menu_last_page = 0;	/* 前回時のメニュータグを記憶 */

				/* ファイルを開く時のディレクトリ名 */
static	char	menu_pathname[ Q8TK_MAX_FILENAME ] = { '\0', };

static	int	menu_boot_dipsw;	/* リセット時の設定を記憶 */
static	int	menu_boot_from_rom;
static	int	menu_boot_basic;
static	int	menu_boot_clock_4mhz;
static	int	menu_boot_version;
static	int	menu_boot_baudrate;

					/* 起動デバイスの制御に必要 */
static	Q8tkWidget	*widget_dipsw_b_boot[ DIPSW_B_XXXX_END ];

static	Q8tkWidget	*menu_accel;	/* メインメニューのキー定義 */
static	Q8tkWidget	*misc_accel;	/* 他のウインドウのキー定義 */


/****************************************************************/
/* 汎用的なダイアログ処理の関数					*/
/****************************************************************/

/*	ここでのダイアログは、	+------------------------------+
 *	複数の見出し、		|            見出し 1          |
 *	セパレータ、		|            見出し 2          |
 *	複数のボタン、		| ---------------------------- |
 *	最大1個のエントリ、	| [エントリ] [ボタン] [ボタン] |
 *	からなる。		+------------------------------+
 *	見出し、セパレータ、ボタン、エントリ合わせて最大(DIA_END-1)個まで。
 *	(内部では最大数のチェックはしてないので、注意)
 *	最後に指定したボタン(エントリ)に、フォーカスがくる。
 */

#define	DIA_MAIN	(0)
#define	DIA_START	(DIA_MAIN+1)
#define	DIA_END		(10)


static	Q8tkWidget	*dialog[DIA_END];
static	int		dialog_num;
static	int		dialog_entry;
static	Q8tkWidget	*dialog_accel;

/*
 *	汎用ダイアログのワーク初期化 (最初に1回呼ぶ)
 */
static	void	dialog_init( void )
{
  int	i;
  for( i=0; i<DIA_END; i++ ){
    dialog[i] = NULL;
  }
  dialog_num = 0;
  dialog_accel = NULL;
}

/*
 *	ダイアログの作成
 */
static	void	dialog_create( void )
{
  dialog_num = DIA_MAIN;

  dialog[ dialog_num ] = q8tk_dialog_new();

  q8tk_misc_set_placement( Q8TK_DIALOG( dialog[ DIA_MAIN ] )->action_area,
			   Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER );

  dialog_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( dialog_accel, dialog[ DIA_MAIN ] );

  dialog_num ++;
}

/*
 *	ダイアログに見出しを追加 (引数…見出しとなる文字列)
 *		見出しは複数個持てる。縦に並ぶ。
 */
static	void	dialog_set_title( const char *label )
{
  dialog[ dialog_num ] = q8tk_label_new( label );
  q8tk_box_pack_start( Q8TK_DIALOG( dialog[ DIA_MAIN ] )->vbox,
		       dialog[ dialog_num ] );
  q8tk_widget_show( dialog[ dialog_num ] );
  q8tk_misc_set_placement( dialog[ dialog_num ],
			   Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP );
  dialog_num ++;
}

/*
 *	ダイアログにセパレータを追加
 *		セパレータは1個。
 */
static	void	dialog_set_separator( void )
{
  dialog[ dialog_num ] = q8tk_hseparator_new();
  q8tk_box_pack_start( Q8TK_DIALOG( dialog[ DIA_MAIN ] )->vbox,
		       dialog[ dialog_num ] );
  q8tk_widget_show( dialog[ dialog_num ] );
  dialog_num ++;
}

/*
 *	ダイアログにボタンを追加 (引数…ボタンの名称、コールバック関数)
 *		ボタンは複数個持てる。横に並ぶ。
 */
static	void	dialog_set_button( const char *label,
				   Q8tkSignalFunc func, void *func_data )
{
  dialog[ dialog_num ] = q8tk_button_new_with_label( label );
  q8tk_box_pack_start( Q8TK_DIALOG( dialog[ DIA_MAIN ] )->action_area,
		       dialog[ dialog_num ] );
  q8tk_widget_show( dialog[ dialog_num ] );
  q8tk_signal_connect( dialog[ dialog_num ], "clicked", func, func_data );

  dialog_num ++;
}

/*
 *	ダイアログにエントリを追加 (引数は…エントリの初期文字列、最大文字数、
 *		エントリは1個だけ持てる。                     コールバック関数)
 */
static	void	dialog_set_entry( const char *text, int max_length,
				  Q8tkSignalFunc func, void *func_data )
{
  dialog[ dialog_num ] = q8tk_entry_new_with_max_length( max_length );
  q8tk_box_pack_start( Q8TK_DIALOG( dialog[ DIA_MAIN ] )->action_area,
		       dialog[ dialog_num ] );
  q8tk_widget_show( dialog[ dialog_num ] );
  q8tk_signal_connect( dialog[ dialog_num ], "activate", func, func_data );
  q8tk_misc_set_size( dialog[ dialog_num ], max_length+1, 0 );
  q8tk_misc_set_placement( dialog[ dialog_num ], 0, Q8TK_PLACEMENT_Y_CENTER );
  q8tk_entry_set_text( dialog[ dialog_num ], text );

  dialog_entry = dialog_num;
  dialog_num ++;
}

/*
 *	ダイアログ内の、エントリの文字列をとり出す
 */
static	const	char	*dialog_get_entry( void )
{
  return	q8tk_entry_get_text( dialog[ dialog_entry ] );
}

/*
 *	ダイアログのボタンに、ショートカットキーを設定
 *		dialog_set_button()の直後に呼べば、そのボタンにキーを割り当てる
 */
static	void	dialog_accel_key( int key )
{
  Q8tkWidget *w = dialog[ dialog_num-1 ];
  q8tk_accel_group_add( dialog_accel, key, w, "clicked" );
}

/*
 *	ダイアログを表示
 */
static	void	dialog_start( void )
{
  q8tk_widget_show( dialog[ DIA_MAIN ] );
  q8tk_grab_add( dialog[ DIA_MAIN ] );

  if( dialog[ dialog_num -1 ] ){
    q8tk_widget_grab_default( dialog[ dialog_num -1 ] );
  }
}

/*
 *	ダイアログを消去
 */
static	void	dialog_destroy( void )
{
  int	i;
  for( i=DIA_START; i<DIA_END; i++ ){
    if( dialog[i] ){
      q8tk_widget_destroy( dialog[i] );
      dialog[i] = NULL;
    }
  }

  q8tk_grab_remove( dialog[ DIA_MAIN ] );
  q8tk_widget_destroy( dialog[ DIA_MAIN ] );
  q8tk_widget_destroy( dialog_accel );
}






/****************************************************************/
/* メニューモード ウイジット処理				*/
/****************************************************************/

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/*				+---------+ +------+ +------+
 *	メイン画面の下のボタン	| MONITOR | | QUIT | | EXIT |
 *				+---------+ +------+ +------+
 *		QUIT を押した時は、確認のダイアログが出る
 */

/*
 * QUIT ボタン押下時の、確認ダイアログ処理
 */

static	void	cb_base_quit_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    emu_mode = QUIT;
    q8tk_main_quit();
  }
}
static	void	cb_base_quit( void )
{
  const char *(*s) = str_base_quit[ menu_lang ];

  dialog_create();
  {
    dialog_set_title( s[ STR_BASE_QUIT_TITLE ] );
    dialog_set_separator();
    dialog_set_button( s[ STR_BASE_QUIT_OK ],
		       cb_base_quit_clicked, (void*)TRUE );
    dialog_accel_key( Q8TK_KEY_F12 );
    dialog_set_button( s[ STR_BASE_QUIT_CANCEL ],
		       cb_base_quit_clicked, (void*)FALSE );
    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}


/*
 *  MONITOR、QUIT、EXIT のボタン押下処理
 */

static	struct{
  int	str_num;
} base_button_data[] =
{
  { STR_BASE_MONITOR, },
  { STR_BASE_QUIT,    },
  { STR_BASE_EXIT,    },
};

static	void	cb_base_button( Q8tkWidget *dummy, void *p )
{
  switch( (int)p ){
  case STR_BASE_EXIT:
    break;
  case STR_BASE_MONITOR:
    emu_mode = MONITOR;
    break;
  case STR_BASE_QUIT:
    cb_base_quit();
    return;
  }
  q8tk_main_quit();
}

static	Q8tkWidget	*base_menu( void )
{
  int	i;
  Q8tkWidget *hbox, *button;
  const char *(*s) = str_base[ menu_lang ];
  
  hbox = q8tk_hbox_new();
  {
    for( i=0; i<countof(base_button_data); i++ ){

      if( base_button_data[i].str_num==STR_BASE_MONITOR  &&
	  debug_mode == FALSE ) continue;

      button = q8tk_button_new_with_label( s[ base_button_data[i].str_num ] );
      q8tk_box_pack_start( hbox, button );
      q8tk_widget_show( button );
      q8tk_signal_connect( button, "clicked",
			 cb_base_button, (void *)base_button_data[i].str_num );

      if( base_button_data[i].str_num==STR_BASE_QUIT ){
	q8tk_accel_group_add( menu_accel, Q8TK_KEY_F12,
			      button, "clicked" );
      }
      if( base_button_data[i].str_num==STR_BASE_EXIT ){
	q8tk_accel_group_add( menu_accel, Q8TK_KEY_ESC,
			      button, "clicked" );
      }
    }
  }
  q8tk_misc_set_placement( hbox, Q8TK_PLACEMENT_X_RIGHT, 0 );
  q8tk_widget_show( hbox );

  return hbox;
}




/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/*					+-------+
 *	メイン画面 -- グラフィック	| GRAPH |
 *					|       +------------
 *
 */

/*
 *  フレームレートの変更(コンボ) 処理
 */

static	struct{
  int		str_num;
  int		frameskip_rate;
} graph_frate_data[] =
{
  { STR_GRAPH_FRATE_60,   1, },
  { STR_GRAPH_FRATE_30,   2, },
  { STR_GRAPH_FRATE_20,   3, },
  { STR_GRAPH_FRATE_15,   4, },
  { STR_GRAPH_FRATE_12,   5, },
  { STR_GRAPH_FRATE_10,   6, },
  { STR_GRAPH_FRATE_6,   10, },
  { STR_GRAPH_FRATE_5,   12, },
  { STR_GRAPH_FRATE_4,   15, },
  { STR_GRAPH_FRATE_3,   20, },
  { STR_GRAPH_FRATE_2,   30, },
  { STR_GRAPH_FRATE_1,   60, },
};

static	void	cb_graph_frate( Q8tkWidget *widget, void *label )
{
  int	i;
  const char	*(*s)[STR_GRAPH_FRATE_END2] = str_graph_frate[ menu_lang ];

  for( i=0; i<countof(graph_frate_data); i++ ){
    if( strcmp( s[i][STR_GRAPH_FRATE_FPS], q8tk_combo_get_text(widget) )==0 ){
      frameskip_rate = graph_frate_data[i].frameskip_rate;
      blink_ctrl_update();
      q8tk_label_set( (Q8tkWidget*)label, s[i][STR_GRAPH_FRATE_SCREEN] );
      return;
    }
  }
}

static	int	get_graph_frate_initval( void )
{
  return	frameskip_rate;
}

							/* thanks floi ! */
static	void	cb_graph_autoskip( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
  use_auto_skip = key;
}

static	int	get_graph_autoskip_initval( void )
{
  return use_auto_skip;
}

static	Q8tkWidget	*menu_graph_frate( void )
{
  int	i;
  Q8tkWidget	*hbox, *combo, *label;
  Q8List	*list = NULL;
  const char	*(*s)[STR_GRAPH_FRATE_END2] = str_graph_frate[ menu_lang ];
  char		wk[32];
  Q8tkWidget	*vbox, *button;

  vbox = q8tk_vbox_new();

  hbox = q8tk_hbox_new();
  label = q8tk_label_new(" fps");

  {
    combo = q8tk_combo_new();

    for( i=0; i<countof(graph_frate_data); i++ ){
      list = q8_list_append( list,
		(void *)s[graph_frate_data[i].str_num][STR_GRAPH_FRATE_FPS] );
    }
    q8tk_combo_popdown_strings( combo, list );
    q8_list_free( list );
  
    i = get_graph_frate_initval();
    if( (60%i)==0 ){
      sprintf( wk, "%d", 60 / i );
    }else{
      sprintf( wk, "%6.3f", 60.0f / get_graph_frate_initval() );
    }
    q8tk_combo_set_text( combo, wk );
    q8tk_misc_set_size( combo, 6, 0 );

    q8tk_signal_connect( combo, "changed", cb_graph_frate, label );

    q8tk_box_pack_start( hbox, combo );
    q8tk_widget_show( combo );
  }
  {
    q8tk_box_pack_start( hbox, label );
    q8tk_widget_show( label );
    cb_graph_frate( combo, (void*)label );
  }
  q8tk_widget_show( hbox );

  q8tk_box_pack_start( vbox, hbox );			/* thanks floi ! */

  label = q8tk_label_new( "" );
  q8tk_widget_show( label );
  q8tk_box_pack_start( vbox, label );

  {
    button = q8tk_check_button_new_with_label( str_graph_autoskip[ menu_lang ] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );

    if( get_graph_autoskip_initval() ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    q8tk_signal_connect( button, "toggled", cb_graph_autoskip, 0 );
  }
  q8tk_widget_show( vbox );

  return vbox;
}




/*
 *  画面サイズ切替え(ラジオボタン) 処理
 */

static	struct{
  int		str_num;
  int		screen_size;
} graph_resize_data[] =
{
  { STR_GRAPH_SIZE_HALF,   SCREEN_SIZE_HALF,   },
  { STR_GRAPH_SIZE_FULL,   SCREEN_SIZE_FULL,   },
  { STR_GRAPH_SIZE_DOUBLE, SCREEN_SIZE_DOUBLE, },
};

static	void	cb_graph_resize( Q8tkWidget *dummy, void *p )
{
  if( screen_size != (int)p &&
      (int)p <= screen_size_max ){
    screen_size = (int)p;
    graphic_system_restart(FALSE);
    set_mouse_visible();
    q8tk_misc_redraw();
  }
}

static	int	get_graph_resize_initval( void )
{
  return screen_size;
}

static	Q8tkWidget	*menu_graph_resize( void )
{
  int	i;
  Q8tkWidget	*hbox, *button;
  Q8List	*list = NULL;
  const char	*(*s) = str_graph_size[ menu_lang ];

  hbox = q8tk_hbox_new();

  for( i=0; i<countof(graph_resize_data); i++ ){

    if( graph_resize_data[i].screen_size > screen_size_max ) break;

    button = q8tk_radio_button_new_with_label( list,
					     s[graph_resize_data[i].str_num] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( hbox, button );
    q8tk_signal_connect( button, "clicked",
		 cb_graph_resize, (void *)graph_resize_data[i].screen_size );
    if( get_graph_resize_initval()==graph_resize_data[i].screen_size ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    list = q8tk_radio_button_group( button );

  }
  q8tk_widget_show( hbox );

  return hbox;
}




/*
 *  各種設定の変更(チェックボタン) 処理
 */
static	struct{
  int		str_num;
} graph_misc_data[] =
{
  { STR_GRAPH_MISC_HIDE_MOUSE, },
  { STR_GRAPH_MISC_15K,        },
  { STR_GRAPH_MISC_DIGITAL,    },
  { STR_GRAPH_MISC_NOINTERP,   },
  { STR_GRAPH_MISC_INTERLACE,  },
};
static	void	cb_graph_misc( Q8tkWidget *widget, void *p )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  switch( (int)p ){
  case STR_GRAPH_MISC_HIDE_MOUSE:
    hide_mouse = key;
    return;
  case STR_GRAPH_MISC_15K:
    if( key ) monitor_15k = 0x02;
    else      monitor_15k = 0x00;
    return;
  case STR_GRAPH_MISC_DIGITAL:
    if( key ) monitor_analog = FALSE;
    else      monitor_analog = TRUE;
    return;
  case STR_GRAPH_MISC_NOINTERP:
    if( key ) use_half_interp = FALSE;
    else      use_half_interp = TRUE;
    return;
  case STR_GRAPH_MISC_INTERLACE:
    if( key ) use_interlace = TRUE;
    else      use_interlace = FALSE;
    return;
  }
}
static	int	get_graph_misc_initval( int type )
{
  switch( type ){
  case STR_GRAPH_MISC_HIDE_MOUSE:  return hide_mouse;
  case STR_GRAPH_MISC_15K:	   return (monitor_15k==0x02) ? TRUE : FALSE;
  case STR_GRAPH_MISC_DIGITAL:     return ( !monitor_analog ) ? TRUE : FALSE;
  case STR_GRAPH_MISC_NOINTERP:   return ( !use_half_interp ) ? TRUE : FALSE;
  case STR_GRAPH_MISC_INTERLACE:   return (  use_interlace  ) ? TRUE : FALSE;
  }
  return FALSE;
}
static	Q8tkWidget	*menu_graph_misc( void )
{
  int	i;
  Q8tkWidget	*vbox, *button;
  const char	*(*s) = str_graph_misc[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(graph_misc_data); i++ ){

    button = q8tk_check_button_new_with_label( s[graph_misc_data[i].str_num] );

    q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );

    if( get_graph_misc_initval(graph_misc_data[i].str_num) ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    q8tk_signal_connect( button, "toggled",
			 cb_graph_misc, (void *)graph_misc_data[i].str_num );
  }

  q8tk_widget_show( vbox );

  return vbox;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_graph( void )
{
  Q8tkWidget *vbox, *frame, *widget;
  const char *(*s) = str_graph[ menu_lang ];

  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_GRAPH_FRATE ] );
    {
      widget = menu_graph_frate();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    frame = q8tk_frame_new( s[ STR_GRAPH_SIZE ] );
    {
      widget = menu_graph_resize();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    frame = q8tk_frame_new( "" );
    {
      widget = menu_graph_misc();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
    q8tk_frame_set_shadow_type( frame, Q8TK_SHADOW_NONE );

  }
  q8tk_widget_show( vbox );

  return vbox;
}








/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*					+-----+
 *	メイン画面 -- CPU設定		| CPU |
 *					|     +------------
 *
 */

/*
 *  CPU処理切替え(ラジオボタン) 処理
 */

static	struct{
  int		str_num;
  int		cpu_timing;
} cpu_cpu_data[] =
{
  { STR_CPU_CPU__1, -1, },
  { STR_CPU_CPU_0,   0, },
  { STR_CPU_CPU_1,   1, },
  { STR_CPU_CPU_2,   2, },
  { STR_CPU_CPU_3,   3, },
};

static	void	cb_cpu_cpu( Q8tkWidget *dummy, void *p )
{
  cpu_timing = (int)p;
}

static	int	get_cpu_cpu_initval( void )
{
  return cpu_timing;
}

static	Q8tkWidget	*menu_cpu_cpu( void )
{
  int	i;
  Q8tkWidget	*vbox, *button;
  Q8List	*list = NULL;
  const char	*(*s) = str_cpu_cpu[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(cpu_cpu_data); i++ ){

    button = q8tk_radio_button_new_with_label( list,
					       s[cpu_cpu_data[i].str_num] );
    if( i ) q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );
    q8tk_signal_connect( button, "clicked",
			 cb_cpu_cpu, (void *)cpu_cpu_data[i].cpu_timing );
    if( get_cpu_cpu_initval()==cpu_cpu_data[i].cpu_timing ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    list = q8tk_radio_button_group( button );

  }
  q8tk_widget_show( vbox );

  return vbox;
}



/*
 *  CPUクロック切替え(エントリ＋ボタン) 処理
 */

static	struct{
  int		str_num;
  int		freq;
} cpu_clock_data[] =
{
  { -1,                  0, },
  { STR_CPU_CLOCK_4MHZ,  (int)(CONST_4MHZ_CLOCK * 1000000.0) },
  { STR_CPU_CLOCK_8MHZ,  (int)(CONST_8MHZ_CLOCK * 1000000.0) },
  { STR_CPU_CLOCK_ANNOUNCE, 0 },
};
static	Q8tkWidget	*cpu_clock_entry;

static	double	get_cpu_clock_initval( void )
{
  return cpu_clock_mhz;
}

static	void	cb_cpu_clock( Q8tkWidget *dummy, void *p )
{
  int	cpumhz = (int)p;	/* -1:入力中 / 0:RET入力 / 0<:設定クロック */
  char	buf[16], *conv_end;
  double clk;

  if( cpumhz < 0 ){
    strncpy( buf, q8tk_entry_get_text( cpu_clock_entry ), 15 );
    buf[15] = '\0';
  }else{
    sprintf( buf, "%8.4f", (double)cpumhz / 1000000.0 );
    q8tk_entry_set_text( cpu_clock_entry, buf );
  }

  clk = strtod( buf, &conv_end );
  if( *conv_end != '\0' ||			/* 変換エラー */
              clk < 0.1 || clk > 100.0 ){
    if( p==0 ){						/* RET入力時自動修正 */
      sprintf( buf, "%8.4f", get_cpu_clock_initval() );
      q8tk_entry_set_text( cpu_clock_entry, buf );
    }	/* 文字入力中は、自動修正しないのだが………仕方ないか */

  }else{					/* 変換成功 */
    cpu_clock_mhz = clk;
    interval_work_init_all();
  }
}

static	Q8tkWidget	*menu_cpu_clock( void )
{
  int	i;
  Q8tkWidget	*vbox, *hbox, *label, *button;
  const char	*(*s) = str_cpu_clock[ menu_lang ];
  char	buf[16];

  vbox = q8tk_vbox_new();

  hbox = q8tk_hbox_new();
  {
    for( i=0; i<countof(cpu_clock_data)-1; i++ ){

      if( cpu_clock_data[i].str_num < 0 ){

	cpu_clock_entry = q8tk_entry_new_with_max_length( 8 );
	q8tk_widget_show( cpu_clock_entry );
	q8tk_box_pack_start( hbox, cpu_clock_entry );
	q8tk_misc_set_placement( cpu_clock_entry, 0, Q8TK_PLACEMENT_Y_CENTER );
	sprintf( buf, "%8.4f", get_cpu_clock_initval() );
	q8tk_entry_set_text( cpu_clock_entry, buf );
	q8tk_signal_connect( cpu_clock_entry,"activate",cb_cpu_clock,(void *)0 );
	q8tk_signal_connect( cpu_clock_entry,"changed", cb_cpu_clock,(void *)-1);

	label = q8tk_label_new( " [MHz] " );
	q8tk_widget_show( label );
	q8tk_box_pack_start( hbox, label );
	q8tk_misc_set_placement( label, 0, Q8TK_PLACEMENT_Y_CENTER );

      }else{

	button = q8tk_button_new_with_label( s[cpu_clock_data[i].str_num] );
	q8tk_widget_show( button );
	q8tk_box_pack_start( hbox, button );
	q8tk_signal_connect( button, "clicked",
			     cb_cpu_clock, (void *)cpu_clock_data[i].freq );
      }
    }
  }
  q8tk_widget_show( hbox );
  q8tk_box_pack_start( vbox, hbox );

  label = q8tk_label_new( s[cpu_clock_data[i].str_num] );
  q8tk_widget_show( label );
  q8tk_box_pack_start( vbox, label );

  q8tk_widget_show( vbox );
  return vbox;
}



/*
 *  ウエイト変更(エントリ) 処理
 */

static	struct{
  int		str_num;
} cpu_wait_data[] =
{
  { STR_CPU_WAIT_NOWAIT,   },
  { STR_CPU_WAIT_FREQ,     },
  { -1,			   },
  { STR_CPU_WAIT_ANNOUNCE, },
};
static	Q8tkWidget	*cpu_wait_entry;

static	int	get_cpu_nowait_initval( void )
{
  return no_wait;
}
static	void	cb_cpu_nowait( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
  no_wait = key;
}

static	double	get_cpu_wait_initval( void )
{
  return wait_freq_hz;
}
static	void	cb_cpu_wait( Q8tkWidget *dummy, void *p )
{
  /* (int)p ... -1:入力中 / 0:RET入力 */

  char buf[16], *conv_end;
  double clk;

  strncpy( buf, q8tk_entry_get_text( cpu_wait_entry ), 15 );
  buf[15] = '\0';

  clk = strtod( buf, &conv_end );
  if( *conv_end != '\0'   ||			/* 変換エラー */
               clk < 10.0 || clk > 240.0 ){
    if( p==0 ){					/* RET入力時自動修正 */
      sprintf( buf, "%5.1f", get_cpu_wait_initval() );
      q8tk_entry_set_text( cpu_wait_entry, buf );
    }	/* 文字入力中は、自動修正しないのだが………仕方ないか */

  }else{					/* 変換成功 */
    wait_freq_hz = clk;
    if( p==0 ){
      sprintf( buf, "%5.1f", clk );
      q8tk_entry_set_text( cpu_wait_entry, buf );
    }
  }
}

static	Q8tkWidget	*menu_cpu_wait( void )
{
  Q8tkWidget	*vbox, *hbox, *label, *button;
  const char	*(*s) = str_cpu_wait[ menu_lang ];
  char	buf[16];

  vbox = q8tk_vbox_new();

  button = q8tk_check_button_new_with_label( s[cpu_wait_data[0].str_num] );
  q8tk_widget_show( button );
  q8tk_box_pack_start( vbox, button );

  if( get_cpu_nowait_initval() ){
    q8tk_toggle_button_set_state( button, TRUE );
  }
  q8tk_signal_connect( button, "toggled", cb_cpu_nowait, 0 );


  label = q8tk_label_new( "" );
  q8tk_widget_show( label );
  q8tk_box_pack_start( vbox, label );

  hbox = q8tk_hbox_new();
  {
    label = q8tk_label_new( s[cpu_wait_data[1].str_num] );
    q8tk_widget_show( label );
    q8tk_box_pack_start( hbox, label );

    cpu_wait_entry = q8tk_entry_new_with_max_length( 6 );
    q8tk_misc_set_size( cpu_wait_entry, 7, 1 );
    q8tk_widget_show( cpu_wait_entry );
    q8tk_box_pack_start( hbox, cpu_wait_entry );
    q8tk_misc_set_placement( cpu_wait_entry, 0, Q8TK_PLACEMENT_Y_CENTER );
    sprintf( buf, "%5.1f", get_cpu_wait_initval() );
    q8tk_entry_set_text( cpu_wait_entry, buf );
    q8tk_signal_connect( cpu_wait_entry, "activate", cb_cpu_wait, (void *)0 );
    q8tk_signal_connect( cpu_wait_entry, "changed",  cb_cpu_wait, (void *)-1 );

    label = q8tk_label_new( " [Hz] " );
    q8tk_widget_show( label );
    q8tk_box_pack_start( hbox, label );
    q8tk_misc_set_placement( label, 0, Q8TK_PLACEMENT_Y_CENTER );
  }
  q8tk_widget_show( hbox );
  q8tk_box_pack_start( vbox, hbox );

  label = q8tk_label_new( s[cpu_wait_data[3].str_num] );
  q8tk_widget_show( label );
  q8tk_box_pack_start( vbox, label );

  q8tk_widget_show( vbox );
  return vbox;
}




/*
 *  各種設定の変更(チェックボタン) 処理
 */
static	struct{
  int		is_button;
  int		str_num;
} cpu_misc_data[] =
{
  { FALSE,	STR_CPU_MISC_DUMMY,     },
  { TRUE,	STR_CPU_MISC_FDCWAIT,   },
  { FALSE,	STR_CPU_MISC_FDCWAIT_X, },
  { FALSE,	STR_CPU_MISC_DUMMY,     },
  { TRUE,	STR_CPU_MISC_HSBASIC,   },
  { FALSE,	STR_CPU_MISC_HSBASIC_X, },
};
static	void	cb_cpu_misc( Q8tkWidget *widget, void *p )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  switch( (int)p ){
  case STR_CPU_MISC_FDCWAIT:
    if( key ) fdc_wait = 1;
    else      fdc_wait = 0;
    return;
  case STR_CPU_MISC_HSBASIC:
    if( key ) highspeed_mode = TRUE;
    else      highspeed_mode = FALSE;
    return;
  }
}
static	int	get_cpu_misc_initval( int type )
{
  switch( type ){
  case STR_CPU_MISC_FDCWAIT:	return ( fdc_wait==0 ) ? FALSE : TRUE;
  case STR_CPU_MISC_HSBASIC:	return highspeed_mode;
  }
  return FALSE;
}
static	Q8tkWidget	*menu_cpu_misc( void )
{
  int	i;
  Q8tkWidget	*vbox, *button;
  const char	*(*s) = str_cpu_misc[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(cpu_misc_data); i++ ){

    if( cpu_misc_data[i].is_button ){
      button = q8tk_check_button_new_with_label( s[cpu_misc_data[i].str_num] );
    }else{
      button = q8tk_label_new( s[cpu_misc_data[i].str_num] );
    }

    q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );

    if( cpu_misc_data[i].is_button ){
      if( get_cpu_misc_initval(cpu_misc_data[i].str_num) ){
	q8tk_toggle_button_set_state( button, TRUE );
      }
      q8tk_signal_connect( button, "toggled",
			   cb_cpu_misc, (void *)cpu_misc_data[i].str_num );
    }else{
      q8tk_misc_set_placement( button, Q8TK_PLACEMENT_X_RIGHT, 0 );
    }
  }

  q8tk_widget_show( vbox );

  return vbox;
}








/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_cpu( void )
{
  Q8tkWidget *vbox, *frame, *widget, *hbox, *vbox2;
  const char *(*s) = str_cpu[ menu_lang ];

  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_CPU_CPU ] );
    {
      widget = menu_cpu_cpu();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    hbox = q8tk_hbox_new();
    {
      vbox2 = q8tk_vbox_new();
      {
	frame = q8tk_frame_new( s[ STR_CPU_CLOCK ] );
	{
	  widget = menu_cpu_clock();
	  q8tk_container_add( frame, widget );
	}
	q8tk_widget_show( frame );
	q8tk_box_pack_start( vbox2, frame );
	
	frame = q8tk_frame_new( s[ STR_CPU_WAIT ] );
	{
	  widget = menu_cpu_wait();
	  q8tk_container_add( frame, widget );
	}
	q8tk_widget_show( frame );
	q8tk_box_pack_start( vbox2, frame );
      }
      q8tk_widget_show( vbox2 );

      q8tk_box_pack_start( hbox, vbox2 );
    }
    {
      widget = menu_cpu_misc();
      q8tk_widget_show( widget );
      q8tk_box_pack_start( hbox, widget );
    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );
  }
  q8tk_widget_show( vbox );

  return vbox;
}








/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*					+-------+
 *	メイン画面 -- リセット		| RESET |
 *					|       +------------
 *
 */


/*
 *  BASICモード切替え(ラジオボタン) 処理
 */

static	struct{
  int		str_num;
  int		boot_basic;
} reset_basic_data[] =
{
  { STR_RESET_BASIC_V1S, BASIC_V1S, },
  { STR_RESET_BASIC_V1H, BASIC_V1H, },
  { STR_RESET_BASIC_V2,  BASIC_V2,  },
  { STR_RESET_BASIC_N,   BASIC_N,   },
};

static	void	cb_reset_basic( Q8tkWidget *dummy, void *p )
{
  menu_boot_basic = (int)p;
}

static	int	get_reset_basic_initval( void )
{
  return menu_boot_basic;
}

static	Q8tkWidget	*menu_reset_basic( void )
{
  int	i;
  Q8tkWidget	*hbox, *button;
  Q8List	*list = NULL;
  const char	*(*s) = str_reset_basic[ menu_lang ];

  hbox = q8tk_hbox_new();

  for( i=0; i<countof(reset_basic_data); i++ ){

    button = q8tk_radio_button_new_with_label( list,
					      s[reset_basic_data[i].str_num] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( hbox, button );
    q8tk_signal_connect( button, "clicked",
		      cb_reset_basic, (void *)reset_basic_data[i].boot_basic );
    if( get_reset_basic_initval()==reset_basic_data[i].boot_basic ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    list = q8tk_radio_button_group( button );

  }
  q8tk_widget_show( hbox );

  return hbox;
}




/*
 *  CLOCKモード切替え(ラジオボタン) 処理
 */

static	struct{
  int		str_num;
  int		boot_clock;
} reset_clock_data[] =
{
  { STR_RESET_CLOCK_4MHZ, CLOCK_4MHZ, },
  { STR_RESET_CLOCK_8MHZ, CLOCK_8MHZ, },
};

static	void	cb_reset_clock( Q8tkWidget *dummy, void *p )
{
  if( (int)p == CLOCK_4MHZ ) menu_boot_clock_4mhz = TRUE;
  else                       menu_boot_clock_4mhz = FALSE;
}

static	int	get_reset_clock_initval( void )
{
  if( menu_boot_clock_4mhz ) return CLOCK_4MHZ;
  else                       return CLOCK_8MHZ;
}

static	Q8tkWidget	*menu_reset_clock( void )
{
  int	i;
  Q8tkWidget	*hbox, *button;
  Q8List	*list = NULL;
  const char	*(*s) = str_reset_clock[ menu_lang ];

  hbox = q8tk_hbox_new();

  for( i=0; i<countof(reset_clock_data); i++ ){

    button = q8tk_radio_button_new_with_label( list,
					      s[reset_clock_data[i].str_num] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( hbox, button );
    q8tk_signal_connect( button, "clicked",
		      cb_reset_clock, (void *)reset_clock_data[i].boot_clock );
    if( get_reset_clock_initval()==reset_clock_data[i].boot_clock ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    list = q8tk_radio_button_group( button );

  }
  q8tk_widget_show( hbox );

  return hbox;
}



/*
 *  バージョン切替え(コンボ) 処理
 */

static	const	char	*reset_version_data[] =
{
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
};

static	void	cb_reset_version( Q8tkWidget *widget, void *dummy )
{
  menu_boot_version = *(q8tk_combo_get_text(widget));
}

static	int	get_reset_version_initval( void )
{
  return menu_boot_version;
}

static	Q8tkWidget	*menu_reset_version( void )
{
  int	i;
  Q8tkWidget	*combo;
  Q8List	*list = NULL;
  char		wk[4];


  combo = q8tk_combo_new();

  for( i=0; i<countof(reset_version_data); i++ ){
    list = q8_list_append( list, (void *)reset_version_data[i] );
  }
  q8tk_combo_popdown_strings( combo, list );
  q8_list_free( list );
  
  wk[0] = get_reset_version_initval();
  wk[1] = '\0';
  q8tk_combo_set_text( combo, wk );
  q8tk_misc_set_size( combo, 4, 0 );

  q8tk_signal_connect( combo, "changed", cb_reset_version, NULL );
  q8tk_widget_show( combo );

  return combo;
}




/*
 *  ディップスイッチ
 */

static	Q8tkWidget	*dipsw_dialog;
static	Q8tkWidget	*dipsw[4];
static	Q8tkWidget	*dipsw_accel;

enum {
  DIPSW_WIN,
  DIPSW_FRAME,
  DIPSW_VBOX,
  DIPSW_QUIT
};

static	void	dipsw_start( void );
static	void	dipsw_finish( void );


/*	戻るボタン押下			*/
static	void	cb_reset_dipsw_end( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dipsw_finish();
}


/*
 *	ディップスイッチ設定ダイアログの作成と表示
 */
static	void	dipsw_start( void )
{
  const char	*(*s) = str_reset[ menu_lang ];

  dipsw[ DIPSW_WIN ] = q8tk_window_new( Q8TK_WINDOW_DIALOG );

  dipsw_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( dipsw_accel, dipsw[ DIPSW_WIN ] );

  dipsw[ DIPSW_FRAME ] = q8tk_frame_new( s[STR_RESET_DIPSW_SET] );
  q8tk_container_add( dipsw[ DIPSW_WIN ], dipsw[ DIPSW_FRAME ] );
  q8tk_widget_show( dipsw[ DIPSW_FRAME ] );

  dipsw[ DIPSW_VBOX ] = q8tk_vbox_new();
  q8tk_container_add( dipsw[ DIPSW_FRAME ], dipsw[ DIPSW_VBOX ] );
  q8tk_widget_show( dipsw[ DIPSW_VBOX ] );
  {
    q8tk_box_pack_start( dipsw[ DIPSW_VBOX ], dipsw_dialog );

    dipsw[ DIPSW_QUIT ] = q8tk_button_new_with_label( s[STR_RESET_DIPSW_QUIT] );

    q8tk_box_pack_start( dipsw[ DIPSW_VBOX ], dipsw[ DIPSW_QUIT ] );
    q8tk_widget_show( dipsw[ DIPSW_QUIT ] );
    q8tk_signal_connect( dipsw[ DIPSW_QUIT ], "clicked", 
			 cb_reset_dipsw_end, 0 );
    q8tk_accel_group_add( dipsw_accel, Q8TK_KEY_ESC, 
			  dipsw[ DIPSW_QUIT ], "clicked" );
  }



  q8tk_widget_show( dipsw[ DIPSW_WIN ] );
  q8tk_grab_add( dipsw[ DIPSW_WIN ] );

  q8tk_widget_grab_default( dipsw[ DIPSW_QUIT ] );
}

/*
 *	ディップスイッチ設定ダイアログの消去
 */
static	void	dipsw_finish( void )
{
  q8tk_widget_destroy( dipsw[ DIPSW_QUIT ] );
  q8tk_widget_destroy( dipsw[ DIPSW_VBOX ] );
  q8tk_widget_destroy( dipsw[ DIPSW_FRAME ] );

  q8tk_grab_remove( dipsw[ DIPSW_WIN ] );
  q8tk_widget_destroy( dipsw[ DIPSW_WIN ] );
  q8tk_widget_destroy( dipsw_accel );
}


static	void	cb_reset_dipsw( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dipsw_start();
}
static	Q8tkWidget	*menu_reset_dipsw( void )
{
  Q8tkWidget	*button;
  const char	*(*s) = str_reset[ menu_lang ];

  button = q8tk_button_new_with_label( s[ STR_RESET_DIPSW ] );
  q8tk_widget_show( button );

  q8tk_toggle_button_set_state( button, TRUE );
  q8tk_signal_connect( button, "clicked", cb_reset_dipsw, 0 );
  return button;
}







/*
 *  設定した条件でリセット(ボタン) 処理
 */

static	void	cb_reset_now( Q8tkWidget *dummy_0, void *dummy_1 )
{
  boot_dipsw      = menu_boot_dipsw;
  boot_from_rom   = menu_boot_from_rom;
  boot_basic      = menu_boot_basic;
  boot_clock_4mhz = menu_boot_clock_4mhz;
  ROM_VERSION     = menu_boot_version;
  baudrate_sw     = menu_boot_baudrate;

  main_reset();

  q8tk_main_quit();

#if 0
printf(  "boot_dipsw      %04x\n",boot_dipsw    );
printf(  "boot_from_rom   %d\n",boot_from_rom   );
printf(  "boot_basic      %d\n",boot_basic      );
printf(  "boot_clock_4mhz %d\n",boot_clock_4mhz );
printf(  "ROM_VERSION     %c\n",ROM_VERSION     );
printf(  "baudrate_sw     %d\n",baudrate_sw     );
#endif
}

static	Q8tkWidget	*menu_reset_current( void )
{
  Q8tkWidget	*label;
  char wk[128];
  const char *(*s) = str_reset[ menu_lang ];
  int	i, j;
  const char *basic, *clock;
  int	version;

  j = get_reset_basic_initval();
  for( i=0; i<countof(reset_basic_data); i++ ){
    if( reset_basic_data[i].boot_basic == j ) break;
  }
  basic = str_reset_basic[ menu_lang ][ reset_basic_data[i].str_num ];

  j = get_reset_clock_initval();
  for( i=0; i<countof(reset_clock_data); i++ ){
    if( reset_clock_data[i].boot_clock == j ) break;
  }
  clock = str_reset_clock[ menu_lang ][ reset_clock_data[i].str_num ];

  version = get_reset_version_initval() & 0xff;
  if( !isprint(version) ) version = '?';

  sprintf( wk, s[ STR_RESET_CURRENT ], basic, clock, version );

  label = q8tk_label_new( wk );
  q8tk_widget_show( label );

  return label;
}








/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_dipsw( void );
static	Q8tkWidget	*menu_reset( void )
{
  Q8tkWidget *hbox, *vbox, *frame, *widget;
  const char *(*s) = str_reset[ menu_lang ];

  dipsw_dialog = menu_dipsw();	/* ディップスイッチ設定ウィジットを生成 */
				/* (メインのメニューには乗せない )	*/

hbox = q8tk_hbox_new();
{
  vbox = q8tk_vbox_new();
  {

    frame = q8tk_frame_new( "" );
    {
      widget = menu_reset_current();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
    q8tk_frame_set_shadow_type( frame, Q8TK_SHADOW_ETCHED_OUT );

    frame = q8tk_frame_new( s[ STR_RESET_BASIC ] );
    {
      widget = menu_reset_basic();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    frame = q8tk_frame_new( s[ STR_RESET_CLOCK ] );
    {
      widget = menu_reset_clock();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    frame = q8tk_frame_new( s[ STR_RESET_VERSION ] );
    {
      widget = menu_reset_version();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    {
      widget = menu_reset_dipsw();
    }
    q8tk_widget_show( widget );
    q8tk_box_pack_start( vbox, widget );
  }
  q8tk_box_pack_start( hbox, vbox );
  q8tk_widget_show( vbox );
}
q8tk_widget_show( hbox );

    {
      widget = q8tk_button_new_with_label( s[ STR_RESET_NOW ] );
      q8tk_signal_connect( widget, "clicked", cb_reset_now, NULL );
      q8tk_misc_set_placement( widget, 0, Q8TK_PLACEMENT_Y_BOTTOM );
    }
    q8tk_widget_show( widget );
    q8tk_box_pack_start( hbox, widget );


  return hbox;
}








/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*					+--------+
 *	メイン画面 -- DIP-SW		| DIP-SW |
 *					|        +------------
 *
 */

/*
 *  ディップスイッチの切替え(ラジオボタン) 処理
 */
static	struct{
  int		type;
  const char	*(*str)[2];
  int		shift;
} dipsw_b_data[] =
{
  { DIPSW_B_TERM, str_dipsw_b_term, 1,  },
  { DIPSW_B_CH80, str_dipsw_b_ch80, 2,  },
  { DIPSW_B_LN25, str_dipsw_b_ln25, 3,  },
  { DIPSW_B_BOOT, str_dipsw_b_boot, -1, },
},
dipsw_r_data[] =
{
  { DIPSW_R_HDPX,  str_dipsw_r_hdpx,  5 +8, },
  { DIPSW_R_XPRM,  str_dipsw_r_xprm,  4 +8, },
  { DIPSW_R_ST2B,  str_dipsw_r_st2b,  3 +8, },
  { DIPSW_R_DT8B,  str_dipsw_r_dt8b,  2 +8, },
  { DIPSW_R_SPRM,  str_dipsw_r_sprm,  4,    },
  { DIPSW_R_PDEL,  str_dipsw_r_pdel,  5,    },
  { DIPSW_R_ENPTY, str_dipsw_r_enpty, 0 +8, },
  { DIPSW_R_EVPTY, str_dipsw_r_evpty, 1 +8, },
};

static	void	cb_dipsw_b( Q8tkWidget *dummy, void *p )
{
  int	shift = dipsw_b_data[ (int)p>>1 ].shift;
  int	on    = (int)p & 1;

  if( shift < 0 ) {
    if( on ) menu_boot_from_rom = TRUE;
    else     menu_boot_from_rom = FALSE;
  }else{
    if( on ) menu_boot_dipsw |=  (1 << shift );
    else     menu_boot_dipsw &= ~(1 << shift );
  }
}

static	int	get_dipsw_b_initval( int p )
{
  int	s = dipsw_b_data[p].shift;
  if( s<0 ) return   menu_boot_from_rom ? DIPSW_B_BOOT_ROM : DIPSW_B_BOOT_DISK;
  else      return ( menu_boot_dipsw >> s ) & 1;
}

static	Q8tkWidget	*menu_dipsw_b( void )
{
  int	i, j;
  Q8tkWidget	*vbox, *hbox, *label, *button;
  Q8List	*list;
  const char	*(*s) = str_dipsw_b[ menu_lang ];
  const char	*(*ss);

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(dipsw_b_data); i++ ){

    hbox = q8tk_hbox_new();

    {
      label = q8tk_label_new( s[ dipsw_b_data[i].type ] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );
    }

    list = NULL;
    ss = (dipsw_b_data[i].str)[menu_lang];

    for( j=0; j<DIPSW_B_XXXX_END; j++ ){

      button = q8tk_radio_button_new_with_label( list, ss[j] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox, button );
      q8tk_signal_connect( button, "clicked", cb_dipsw_b, (void*)((i<<1)|j));
      if( get_dipsw_b_initval(i) == j ){
	q8tk_toggle_button_set_state( button, TRUE );
      }
      list = q8tk_radio_button_group( button );

      if( dipsw_b_data[i].type==DIPSW_B_BOOT ){
	widget_dipsw_b_boot[j] = button;
      }

    }

    q8tk_box_pack_start( vbox, hbox );
    q8tk_widget_show( hbox );
  }

  q8tk_widget_show( vbox );

  return vbox;
}




static	void	cb_dipsw_r( Q8tkWidget *dummy, void *p )
{
  int	shift = dipsw_r_data[ (int)p>>1 ].shift;
  int	on    = (int)p & 1;

  if( on ) menu_boot_dipsw |=  (1 << shift );
  else     menu_boot_dipsw &= ~(1 << shift );
}

static	int	get_dipsw_r_initval( int p )
{
  int	s = dipsw_r_data[p].shift;

  return ( menu_boot_dipsw >> s ) & 1;
}




/*
 *  ディップスイッチの切替え(コンボ) 処理
 */

static	const	char	*dipsw_r_baudrate_data[] =
{
  "75",  "150",  "300",  "600",  "1200", "2400",  "4800",  "9600",  "19200",
};

static	void	cb_dipsw_r_baudrate( Q8tkWidget *widget, void *dummy )
{
  int	i;
  for( i=0; i<countof(dipsw_r_baudrate_data); i++ ){
    if( strcmp( dipsw_r_baudrate_data[i], q8tk_combo_get_text(widget) ) == 0 ){
      menu_boot_baudrate = i;
      return;
    }
  }
}

static	int	get_dipsw_r_baudrate_initval( void )
{
  return menu_boot_baudrate;
}


static	Q8tkWidget	*menu_dipsw_r( void )
{
  int	i, j;
  Q8tkWidget	*vbox, *hbox, *label, *button, *combo;
  Q8List	*list = NULL;
  const char	*(*s) = str_dipsw_r[ menu_lang ];
  const char	*(*ss);

  vbox = q8tk_vbox_new();

  {
    hbox = q8tk_hbox_new();

    {
      label = q8tk_label_new( s[ DIPSW_R_BAUDRATE ] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );
    }
    {
      combo = q8tk_combo_new();

      for( i=0; i<countof(dipsw_r_baudrate_data); i++ ){
	list = q8_list_append( list, (void *)dipsw_r_baudrate_data[i] );
      }
      q8tk_combo_popdown_strings( combo, list );
      q8_list_free( list );
  
      i = get_dipsw_r_baudrate_initval();
      if( i < countof(dipsw_r_baudrate_data) ){
	q8tk_combo_set_text( combo, dipsw_r_baudrate_data[i] );
      }else{
	q8tk_combo_set_text( combo, "???" );
      }
      q8tk_misc_set_size( combo, 8, 0 );

      q8tk_signal_connect( combo, "changed", cb_dipsw_r_baudrate, NULL );

      q8tk_box_pack_start( hbox, combo );
      q8tk_widget_show( combo );
    }
    q8tk_box_pack_start( vbox, hbox );
    q8tk_widget_show( hbox );
  }


  for( i=0; i<countof(dipsw_r_data); i++ ){

    hbox = q8tk_hbox_new();

    {
      label = q8tk_label_new( s[ dipsw_r_data[i].type ] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );
    }

    list = NULL;
    ss = (dipsw_r_data[i].str)[menu_lang];

    for( j=0; j<DIPSW_R_XXXX_END; j++ ){

      button = q8tk_radio_button_new_with_label( list, ss[j] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox, button );
      q8tk_signal_connect( button, "clicked", cb_dipsw_r, (void*)((i<<1)|j));
      if( get_dipsw_r_initval(i) == j ){
	q8tk_toggle_button_set_state( button, TRUE );
      }
      list = q8tk_radio_button_group( button );
    }

    q8tk_box_pack_start( vbox, hbox );
    q8tk_widget_show( hbox );
  }

  q8tk_widget_show( vbox );

  return vbox;
}








/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_dipsw( void )
{
  Q8tkWidget *vbox, *frame, *widget;
  const char *(*s) = str_dipsw[ menu_lang ];

  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_DIPSW_B ] );
    {
      widget = menu_dipsw_b();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    frame = q8tk_frame_new( s[ STR_DIPSW_R ] );
    {
      widget = menu_dipsw_r();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
  }
  q8tk_widget_show( vbox );

  return vbox;
}








/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*					+------+
 *	メイン画面 -- 音量		| 音量 |
 *					|      +------------
 *
 */

/*
 *  ボリュームの切替え(スケール) 処理
 */

static	struct{
  int		type;
  int		min;
  int		max;
} volume_level_data[] =
{
  { VOLUME_LEVEL_FM,   FMVOL_MIN,   FMVOL_MAX   },
  { VOLUME_LEVEL_PSG,  PSGVOL_MIN,  PSGVOL_MAX  },
  { VOLUME_LEVEL_BEEP, BEEPVOL_MIN, BEEPVOL_MAX },
},
volume_level2_data[] =
{
  { VOLUME_LEVEL_RHYTHM, RHYTHMVOL_MIN, RHYTHMVOL_MAX },
  { VOLUME_LEVEL_ADPCM,  ADPCMVOL_MIN,  ADPCMVOL_MAX  },
},
volume_level3_data[] =
{
  { VOLUME_LEVEL_FMPSG,FMVOL_MIN,   FMVOL_MAX   },
  { VOLUME_LEVEL_BEEP, BEEPVOL_MIN, BEEPVOL_MAX },
};

static	void	cb_volume( Q8tkWidget *widget, void *p )
{
  int	vol = Q8TK_ADJUSTMENT( widget )->value;

  switch( (int)p ){
  case VOLUME_LEVEL_TOTAL:
    xmame_set_sound_volume( vol );
    break;
  case VOLUME_LEVEL_FM:
    xmame_set_mixer_volume( XMAME_MIXER_FM, vol );
    break;
  case VOLUME_LEVEL_PSG:
    xmame_set_mixer_volume( XMAME_MIXER_PSG, vol );
    break;
  case VOLUME_LEVEL_BEEP:
    xmame_set_mixer_volume( XMAME_MIXER_BEEP, vol );
    break;
  case VOLUME_LEVEL_RHYTHM:
    xmame_set_mixer_volume( XMAME_MIXER_RHYTHM, vol );
    break;
  case VOLUME_LEVEL_ADPCM:
    xmame_set_mixer_volume( XMAME_MIXER_ADPCM, vol );
    break;
  case VOLUME_LEVEL_FMPSG:
    xmame_set_mixer_volume( XMAME_MIXER_FMPSG, vol );
    break;
  }
}

static	int	get_volume_initval( int type )
{
  switch( type ){
  case VOLUME_LEVEL_TOTAL: return xmame_get_sound_volume();
  case VOLUME_LEVEL_FM:	   return xmame_get_mixer_volume( XMAME_MIXER_FM );
  case VOLUME_LEVEL_PSG:   return xmame_get_mixer_volume( XMAME_MIXER_PSG );
  case VOLUME_LEVEL_BEEP:  return xmame_get_mixer_volume( XMAME_MIXER_BEEP );
  case VOLUME_LEVEL_RHYTHM:return xmame_get_mixer_volume( XMAME_MIXER_RHYTHM );
  case VOLUME_LEVEL_ADPCM: return xmame_get_mixer_volume( XMAME_MIXER_ADPCM );
  case VOLUME_LEVEL_FMPSG: return xmame_get_mixer_volume( XMAME_MIXER_FMPSG );
  }
  return 0;
}

static	Q8tkWidget	*menu_volume_volume( void )
{
  Q8tkWidget	*hbox, *label, *adj, *scale;
  const char	*(*s) = str_volume_level[ menu_lang ];

  hbox = q8tk_hbox_new();

  {
    label = q8tk_label_new( s[ VOLUME_LEVEL_TOTAL ] );
    q8tk_widget_show( label );
    q8tk_box_pack_start( hbox, label );
  }

  {
    adj = q8tk_adjustment_new( get_volume_initval(VOLUME_LEVEL_TOTAL),
			       VOL_MIN, VOL_MAX, 1, 4 );
    q8tk_signal_connect( adj, "value_changed",
			 cb_volume, (void*)VOLUME_LEVEL_TOTAL );

    scale = q8tk_hscale_new( adj );
    q8tk_adjustment_set_arrow( scale->stat.scale.adj, TRUE );
    /*q8tk_adjustment_set_length( scale->stat.scale.adj, 11 );*/
    q8tk_scale_set_draw_value( scale, TRUE );
    q8tk_scale_set_value_pos( scale, Q8TK_POS_LEFT );

    q8tk_widget_show( scale );
    q8tk_box_pack_start( hbox, scale );
  }

  q8tk_widget_show( hbox );

  return hbox;
}




static	Q8tkWidget	*menu_volume_level( void )
{
  int	i;
  Q8tkWidget	*vbox, *hbox, *label, *adj, *scale;
  const char	*(*s) = str_volume_level[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(volume_level_data); i++ ){

    hbox = q8tk_hbox_new();

    {
      label = q8tk_label_new( s[ volume_level_data[i].type ] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );
    }

    {
      adj = q8tk_adjustment_new( get_volume_initval(volume_level_data[i].type),
				 volume_level_data[i].min, 
				 volume_level_data[i].max, 1, 10 );
      q8tk_signal_connect( adj, "value_changed",
			   cb_volume, (void*)volume_level_data[i].type );

      scale = q8tk_hscale_new( adj );
      q8tk_adjustment_set_arrow( scale->stat.scale.adj, TRUE );
      /*q8tk_adjustment_set_length( scale->stat.scale.adj, 11 );*/
      q8tk_scale_set_draw_value( scale, TRUE );
      q8tk_scale_set_value_pos( scale, Q8TK_POS_LEFT );

      q8tk_widget_show( scale );
      q8tk_box_pack_start( hbox, scale );
    }
    q8tk_box_pack_start( vbox, hbox );
    q8tk_widget_show( hbox );

  }

  q8tk_widget_show( vbox );

  return vbox;
}




static	Q8tkWidget	*menu_volume_level2( void )
{
  int	i;
  Q8tkWidget	*vbox, *hbox, *label, *adj, *scale;
  const char	*(*s) = str_volume_level[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(volume_level2_data); i++ ){

    hbox = q8tk_hbox_new();

    {
      label = q8tk_label_new( s[ volume_level2_data[i].type ] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );
    }

    {
      adj = q8tk_adjustment_new(get_volume_initval(volume_level2_data[i].type),
				volume_level2_data[i].min,
				volume_level2_data[i].max, 1, 10 );
      q8tk_signal_connect( adj, "value_changed",
			   cb_volume, (void*)volume_level2_data[i].type );

      scale = q8tk_hscale_new( adj );
      q8tk_adjustment_set_arrow( scale->stat.scale.adj, TRUE );
      /*q8tk_adjustment_set_length( scale->stat.scale.adj, 11 );*/
      q8tk_scale_set_draw_value( scale, TRUE );
      q8tk_scale_set_value_pos( scale, Q8TK_POS_LEFT );

      q8tk_widget_show( scale );
      q8tk_box_pack_start( hbox, scale );
    }
    q8tk_box_pack_start( vbox, hbox );
    q8tk_widget_show( hbox );

  }

  q8tk_widget_show( vbox );

  return vbox;
}




static	Q8tkWidget	*menu_volume_level3( void )
{
  int	i;
  Q8tkWidget	*vbox, *hbox, *label, *adj, *scale;
  const char	*(*s) = str_volume_level[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(volume_level3_data); i++ ){

    hbox = q8tk_hbox_new();

    {
      label = q8tk_label_new( s[ volume_level3_data[i].type ] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );
    }

    {
      adj = q8tk_adjustment_new( get_volume_initval(volume_level3_data[i].type),
				 volume_level3_data[i].min, 
				 volume_level3_data[i].max, 1, 10 );
      q8tk_signal_connect( adj, "value_changed",
			   cb_volume, (void*)volume_level3_data[i].type );

      scale = q8tk_hscale_new( adj );
      q8tk_adjustment_set_arrow( scale->stat.scale.adj, TRUE );
      /*q8tk_adjustment_set_length( scale->stat.scale.adj, 11 );*/
      q8tk_scale_set_draw_value( scale, TRUE );
      q8tk_scale_set_value_pos( scale, Q8TK_POS_LEFT );

      q8tk_widget_show( scale );
      q8tk_box_pack_start( hbox, scale );
    }
    q8tk_box_pack_start( vbox, hbox );
    q8tk_widget_show( hbox );

  }

  q8tk_widget_show( vbox );

  return vbox;
}




static	Q8tkWidget	*menu_volume_no_available( void )
{
  Q8tkWidget	*label;
  const char *(*s)  = str_volume_no[ menu_lang ];

#ifdef	USE_SOUND
  label = q8tk_label_new( s[ STR_VOLUME_NO_AVAILABEL ] );
#else
  label = q8tk_label_new( s[ STR_VOLUME_NO_SUPPORT ] );
#endif
  q8tk_widget_show( label );

  return label;
}








/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_volume( void )
{
  Q8tkWidget *vbox, *frame, *widget;
  const char *(*s) = str_volume[ menu_lang ];

  if( xmame_sound_is_enable()==FALSE ){
  
    frame = q8tk_frame_new( "" );
    {
      widget = menu_volume_no_available();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_frame_set_shadow_type( frame, Q8TK_SHADOW_ETCHED_OUT );
    return frame;
  }


  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_VOLUME_VOLUME ] );
    {
      widget = menu_volume_volume();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    frame = q8tk_frame_new( s[ STR_VOLUME_LEVEL ] );
    {
#if	defined(USE_SOUND) && defined(USE_FMGEN)
      if( use_fmgen ){
	widget = menu_volume_level3();
      }
      else
#endif
      {
	widget = menu_volume_level();
      }
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

#if	defined(USE_SOUND) && defined(USE_FMGEN)
    if( use_fmgen ){
    }
    else
#endif
    if( sound_board==SOUND_II ){
      frame = q8tk_frame_new( s[ STR_VOLUME_LEVEL2 ] );
      {
	widget = menu_volume_level2();
	q8tk_container_add( frame, widget );
      }
      q8tk_widget_show( frame );
      q8tk_box_pack_start( vbox, frame );
    }
  }
  q8tk_widget_show( vbox );

  return vbox;
}








/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*					+------+
 *	メイン画面 -- ディスク		| DISK |
 *					|      +------------
 *
 */

typedef struct{
  Q8tkWidget	*list;
  Q8tkWidget	*button[2];
  Q8tkWidget	*label[2];
  int		func[2];
  Q8tkWidget	*stat_label;
  Q8tkWidget	*attr_label;
  Q8tkWidget	*num_label;
} T_DISK_INFO;

static	T_DISK_INFO	disk_info[2];

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static	int		dialog_drv;
static	int		dialog_img;
static	int		create_flg;
static	void	set_disk_widget( void );


/* BOOT from DISK で、DISK を CLOSE した時や、
   BOOT from ROM  で、DISK を OPEN した時は、 DIP-SW 設定を強制変更 */
static	void	disk_update_dipsw_b_boot( void )
{
  if( disk_image_exist(0) || disk_image_exist(1) ){
    q8tk_toggle_button_set_state( widget_dipsw_b_boot[DIPSW_B_BOOT_DISK],TRUE);
  }else{
    q8tk_toggle_button_set_state( widget_dipsw_b_boot[DIPSW_B_BOOT_ROM ],TRUE);
  }

  /* リセットしないでメニューモードを抜けると設定が保存されないので・・・ */
  boot_from_rom = menu_boot_from_rom;			/* thanks floi ! */
}




/*
 * 「エラーメッセージダイアログ」
 */
static	void	cb_disk_error_dialog_ok( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dialog_destroy();
}

static	void	cb_disk_error_dialog( int result )
{
  char wk[128];
  const char *(*s) = create_flg ? str_err2[ menu_lang ]
				: str_err[ menu_lang ];

  if( result==ERR_NO ) return;
  if( create_flg ) sprintf( wk, s[ result ] );
  else             sprintf( wk, s[ result ], dialog_drv+1 );

  dialog_create();
  {
    dialog_set_title( wk );
    dialog_set_separator();
    dialog_set_button( s[ ERR_NO ],
		       cb_disk_error_dialog_ok, NULL );
  }
  dialog_start();
}



/*
 * 属性変更の各種処理
 */

enum{
  ATTR_RENAME,
  ATTR_PROTECT,
  ATTR_FORMAT,
  ATTR_UNFORMAT,
  ATTR_APPEND,
  ATTR_CREATE
};

static	void	cb_disk_attr_file_ctrl( int cmd, char *c )
{
  int	drv = dialog_drv;
  int	img = dialog_img;

  int	busy, result = -1;
  FILE *fp;

  create_flg = ( cmd==ATTR_CREATE ) ? TRUE : FALSE;


		/* コマンド別に、ファイルの開け方が異なる */

  switch( cmd ){
  case ATTR_RENAME:
  case ATTR_PROTECT:
  case ATTR_FORMAT:
  case ATTR_UNFORMAT:
    fp = fopen_image_edit( drive[drv].filename, "r+b", &busy );	break;
  case ATTR_APPEND:
    fp = fopen_image_edit( drive[drv].filename, "ab", &busy );	break;
  case ATTR_CREATE:
    fp = fopen_image_edit( c, "ab", &busy );   			break;
  default:
    return;
  }

		/* ファイルは無事開けれたかな？ */

  if( fp==NULL ){			/* ファイルが開けなかった…         */
    if( busy==-1 ) result = ERR_CANT_OPEN;	/* 新規オープンに失敗       */
    else           result = ERR_READ_ONLY;	/* DRIVE[1:][2:]ファイル失敗*/
    cb_disk_error_dialog( result );
    return;
  }else{				/* ファイルは開いた                 */
    if( busy==-1 ) ;				/* 新規オープンしたファイル */
    else{					/* DRIVE[1:][2:]のファイル  */
      if( drive[ busy ].detect_broken_image ){
	result = ERR_MAYBE_BROKEN;
	cb_disk_error_dialog( result );
	return;
      }
    }
  }

#if 0
  if( cmd==ATTR_CREATE || cmd==ATTR_APPEND ){
    /* この処理に時間がかかるような場合、メッセージをだす？？ */
    /* この処理がそんなに時間がかかることはない？？ */
  }
#endif

		/* 開いたファイルに対して、処理 */

  switch( cmd ){
  case ATTR_RENAME:	result = fwrite_name( fp, img, c );	break;
  case ATTR_PROTECT:	result = fwrite_protect( fp, img, c );	break;
  case ATTR_FORMAT:	result = fwrite_format( fp, img );	break;
  case ATTR_UNFORMAT:	result = fwrite_unformat( fp, img );	break;
  case ATTR_CREATE:
  case ATTR_APPEND:	result = append_blank( fp );		break;
  }

		/* その結果 */

  switch( result ){
  case -1:  result = ERR_MAYBE_BROKEN;		break;
  case 0:   result = ERR_NO;			break;
  case 1:   result = ERR_MAYBE_BROKEN;		break;
  case 2:   result = ERR_MAYBE_BROKEN;		break;
  case 3:   result = ERR_SEEK;			break;
  case 4:   result = ERR_WRITE;			break;
  case 5:   result = ERR_UNEXPECTED;		break;
  default:  result = ERR_UNEXPECTED;		break;
  }

		/* 終了処理。なお、エラー時はメッセージを出す */

  if( busy==-1 ){		/* 新規オープンの場合 */
    ex_fclose(fp);
    if( result ){
      cb_disk_error_dialog( result );
    }
    return;
  }else{			/* DRIVE[1:][2:]ファイルの場合 (UPDATE) */
    if( result==0 ){
      switch( cmd ){
      case ATTR_RENAME:    update_after_fwrite_name( busy, img, c );	break;
      case ATTR_PROTECT:   update_after_fwrite_protect( busy, img, c );	break;
      case ATTR_FORMAT:    update_after_fwrite_format( busy, img );	break;
      case ATTR_UNFORMAT:  /* not update */				break;
      case ATTR_APPEND:
      case ATTR_CREATE:    update_after_append_blank( busy );		break;
      }
      set_disk_widget();
      if( cmd != ATTR_CREATE ) disk_update_dipsw_b_boot();
      return;
    }else{
      cb_disk_error_dialog( result );
      return;
    }
  }
}





/*
 * 「リネーム」ダイアログ
 */

static	void	cb_disk_attr_rename_activate( Q8tkWidget *dummy, void *p )
{
  char	wk[32];

  if( (int)p ){			/* dialog_destroy() の前にエントリをゲット */
    strncpy( wk, dialog_get_entry(), 31 );
    wk[31] = '\0';
  }

  dialog_destroy();

  if( (int)p ){
    cb_disk_attr_file_ctrl( ATTR_RENAME, wk );
  }
}
static	void	cb_disk_attr_rename( void )
{
  char wk[128], name[32];
  const char *(*s) = str_disk_attr_rename[ menu_lang ];

  sjis2euc( name, drive[dialog_drv].image[dialog_img].name );
  sprintf( wk, s[ STR_DISK_ATTR_RENAME_TITLE ], name, dialog_drv+1 );

  dialog_create();
  {
    dialog_set_title( wk );
    dialog_set_separator();
    dialog_set_entry( drive[dialog_drv].image[dialog_img].name, /* SJIS ! */
		      15,
		      cb_disk_attr_rename_activate, (void*)TRUE );
    dialog_set_button( s[ STR_DISK_ATTR_RENAME_OK ],
		       cb_disk_attr_rename_activate, (void*)TRUE );
    dialog_set_button( s[ STR_DISK_ATTR_RENAME_CANCEL ],
		       cb_disk_attr_rename_activate, (void*)FALSE );
    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*
 * 「プロテクト」ダイアログ
 */

static	void	cb_disk_attr_protect_clicked( Q8tkWidget *dummy, void *p )
{
  char	c;

  dialog_destroy();

  if( (int)p ){
    if( (int)p == 1 ) c = DISK_PROTECT_TRUE;
    else              c = DISK_PROTECT_FALSE;

    cb_disk_attr_file_ctrl( ATTR_PROTECT, &c );
  }
}
static	void	cb_disk_attr_protect( void )
{
  char wk[128], name[32];
  const char *(*s) = str_disk_attr_protect[ menu_lang ];


  sjis2euc( name, drive[dialog_drv].image[dialog_img].name );
  sprintf( wk, s[ STR_DISK_ATTR_PROTECT_TITLE ], name, dialog_drv+1 );

  dialog_create();
  {
    dialog_set_title( wk );
    dialog_set_separator();
    dialog_set_button( s[ STR_DISK_ATTR_PROTECT_SET ],
		       cb_disk_attr_protect_clicked, (void*)1 );
    dialog_set_button( s[ STR_DISK_ATTR_PROTECT_UNSET ],
		       cb_disk_attr_protect_clicked, (void*)2 );
    dialog_set_button( s[ STR_DISK_ATTR_PROTECT_CANCEL ],
		       cb_disk_attr_protect_clicked, (void*)0 );
    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*
 * 「フォーマット」ダイアログ
 */

static	void	cb_disk_attr_format_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    if( (int)p == 1 ) cb_disk_attr_file_ctrl( ATTR_FORMAT,   NULL );
    else              cb_disk_attr_file_ctrl( ATTR_UNFORMAT, NULL );
  }
}
static	void	cb_disk_attr_format( void )
{
  char wk[128], name[32];
  const char *(*s) = str_disk_attr_format[ menu_lang ];

  sjis2euc( name, drive[dialog_drv].image[dialog_img].name );
  sprintf( wk, s[ STR_DISK_ATTR_FORMAT_TITLE ], name, dialog_drv+1 );

  dialog_create();
  {
    dialog_set_title( wk );
    dialog_set_title( s[ STR_DISK_ATTR_FORMAT_WARNING ] );
    dialog_set_separator();
    dialog_set_button( s[ STR_DISK_ATTR_FORMAT_DO ],
		       cb_disk_attr_format_clicked, (void*)1 );
    dialog_set_button( s[ STR_DISK_ATTR_FORMAT_NOT ],
		       cb_disk_attr_format_clicked, (void*)2 );
    dialog_set_button( s[ STR_DISK_ATTR_FORMAT_CANCEL ],
		       cb_disk_attr_format_clicked, (void*)0 );
    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}
/*
 * 「ブランクディスク」ダイアログ
 */
static	void	cb_disk_attr_blank_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    cb_disk_attr_file_ctrl( ATTR_APPEND, NULL );
  }

}
static	void	cb_disk_attr_blank( void )
{
  char wk[128];
  const char *(*s) = str_disk_attr_blank[ menu_lang ];

  sprintf( wk, s[ STR_DISK_ATTR_BLANK_TITLE ], dialog_drv+1 );

  dialog_create();
  {
    dialog_set_title( wk );
    dialog_set_separator();
    dialog_set_button( s[ STR_DISK_ATTR_BLANK_OK ],
		       cb_disk_attr_blank_clicked, (void*)TRUE );
    dialog_set_button( s[ STR_DISK_ATTR_BLANK_CANCEL ],
		       cb_disk_attr_blank_clicked, (void*)FALSE );
    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}



/*
 * 「」「」「」「」「」ダイアログ
 */
enum {
  FUNC_DISK_ATTR_NONE,
  FUNC_DISK_ATTR_RENAME,
  FUNC_DISK_ATTR_PROTECT,
  FUNC_DISK_ATTR_FORMAT,
  FUNC_DISK_ATTR_BLANK
};
static	void	cb_disk_attr_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  switch( (int)p ){
  case FUNC_DISK_ATTR_RENAME:	cb_disk_attr_rename();	break;
  case FUNC_DISK_ATTR_PROTECT:	cb_disk_attr_protect();	break;
  case FUNC_DISK_ATTR_FORMAT:	cb_disk_attr_format();	break;
  case FUNC_DISK_ATTR_BLANK:	cb_disk_attr_blank();	break;
  }
}


static void cb_disk_attr( void )
{
  char wk[128], name[32];
  const char *(*s) = str_disk_attr[ menu_lang ];

  sjis2euc( name, drive[dialog_drv].image[dialog_img].name );
  sprintf( wk, s[ STR_DISK_ATTR_TITLE ], name, dialog_drv+1 );

  dialog_create();
  {
    dialog_set_title( wk );
    dialog_set_separator();
    dialog_set_button( s[ STR_DISK_ATTR_RENAME ],
		       cb_disk_attr_clicked, (void*)FUNC_DISK_ATTR_RENAME );
    dialog_set_button( s[ STR_DISK_ATTR_PROTECT ],
		       cb_disk_attr_clicked, (void*)FUNC_DISK_ATTR_PROTECT );
    dialog_set_button( s[ STR_DISK_ATTR_FORMAT ],
		       cb_disk_attr_clicked, (void*)FUNC_DISK_ATTR_FORMAT );
    dialog_set_button( s[ STR_DISK_ATTR_BLANK ],
		       cb_disk_attr_clicked, (void*)FUNC_DISK_ATTR_BLANK );
    dialog_set_button( s[ STR_DISK_ATTR_CANCEL ],
		       cb_disk_attr_clicked, (void*)FUNC_DISK_ATTR_NONE );
    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}




/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 *	「イメージファイルを開く」
 */

static	int	cb_disk_cmd;
static void cb_disk_open_ok( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  char	wk[ Q8TK_MAX_FILENAME ];

  strcpy( wk, q8tk_file_selection_get_filename(fsel) );

  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );

  if( cb_disk_cmd == IMG_OPEN ){

    if( disk_image_exist( dialog_drv ) ) disk_eject( dialog_drv );
    if( disk_insert( dialog_drv, wk, 0 ) ){
      cb_disk_error_dialog( ERR_CANT_OPEN );
    }
    else{
      set_menu_dirname();
    }

  }else{	/*   IMG_BOTH */

    if( disk_image_exist( 0 ) ) disk_eject( 0 );
    if( disk_image_exist( 1 ) ) disk_eject( 1 );
    
    if( disk_insert( 0, wk, 0 ) ){
      dialog_drv = 0;
      cb_disk_error_dialog( ERR_CANT_OPEN );
    }else{
      int img = ( drive[0].image_nr > 1 ) ?1 :0;
      if( disk_insert( 1, wk, img ) ){
	dialog_drv = 1;
	cb_disk_error_dialog( ERR_CANT_OPEN );
      }
      set_menu_dirname();
    }

  }
  set_disk_widget();
  disk_update_dipsw_b_boot();
}
static void cb_disk_open_cancel( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static void cb_disk_open( int cmd )
{
  Q8tkWidget *fselect;
  int	num;
  const char *(*s)[NR_DRIVE] = str_disk_open[ menu_lang ];

  cb_disk_cmd = cmd;
  num = (cmd==IMG_OPEN) ? STR_DISK_OPEN_OPEN : STR_DISK_OPEN_BOTH;


  fselect = q8tk_file_selection_new( s[ num ][ dialog_drv ] );

  q8tk_widget_show( fselect );
  q8tk_grab_add( fselect );


  q8tk_file_selection_set_filename( fselect, menu_pathname );


  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->ok_button,
		      "clicked", cb_disk_open_ok, fselect );
  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->cancel_button,
		      "clicked", cb_disk_open_cancel, fselect );
  q8tk_widget_grab_default( Q8TK_FILE_SELECTION( fselect )->cancel_button);


  misc_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( misc_accel, fselect );
  q8tk_accel_group_add( misc_accel, Q8TK_KEY_ESC,
		        Q8TK_FILE_SELECTION( fselect )->cancel_button,
		        "clicked" );
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 *	「イメージファイルを閉じる」
 */

static void cb_disk_close( void )
{
  if( disk_image_exist( dialog_drv ) ) disk_eject( dialog_drv );
  set_disk_widget();
  disk_update_dipsw_b_boot();
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 *	「反対のドライブにセットしているのと同じイメージファイルを開く」
 */

static void cb_disk_copy( void )
{
  int	img;

  if( !disk_image_exist( dialog_drv^1 ) ) return;


  img = dialog_drv;
  if( drive[ dialog_drv^1 ].selected_image == img )   img^=1;
  if( drive[ dialog_drv^1 ].image_nr <= 1 && img==1 ) img = 0;


  if( disk_image_exist( dialog_drv ) ) disk_eject( dialog_drv );
  if( disk_insert( dialog_drv, drive[ dialog_drv^1 ].filename, img ) ){
    cb_disk_error_dialog( ERR_CANT_OPEN );
  }

  set_disk_widget();
  disk_update_dipsw_b_boot();
}





/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 *	イメージの変更 or ボタンが押された
 */

static	void	cb_disk_image( Q8tkWidget *dummy, void *p )
{
  int	drv = ( (int)p ) & 0xff;
  int	img = ( (int)p ) >> 8;

  if( img < 0 ){			/* img==-1 で <<なし>> */
    drive_set_empty( drv );
  }else{				/* img>=0 なら イメージ番号 */
    drive_unset_empty( drv );
    disk_change_image( drv, img );
  }
}
static	void	cb_disk_button( Q8tkWidget *dummy, void *p )
{
  int	drv    = ( (int)p ) & 0xff;
  int	button = ( (int)p ) >> 8;

  dialog_drv = drv;
  dialog_img = drive[dialog_drv].selected_image;

  switch( disk_info[drv].func[button] ){
  case IMG_OPEN:
  case IMG_BOTH:
    cb_disk_open( disk_info[drv].func[button] );
    break;
  case IMG_CLOSE:
    cb_disk_close();
    break;
  case IMG_COPY:
    cb_disk_copy();
    break;
  case IMG_ATTR:
    if( ! drive_check_empty( drv ) ){	     /* イメージ<<なし>>選択時は無効 */
      cb_disk_attr();
    }
    break;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/*
 *	イメージのリストを生成する
 */

static	void	set_disk_widget( void )
{
  int	i, drv;
  Q8tkWidget *item;
  const char *(*sbutton)[NR_DRIVE] = str_disk_button[ menu_lang ];
  const char *(*sinfo)             = str_disk_info[ menu_lang ];
  const char *(*simage)            = str_disk_image[ menu_lang ];


  for( drv=0; drv<2; drv++ ){

    T_DISK_INFO	*w = &disk_info[drv];

    q8tk_list_clear_items( w->list, 0, -1 );
 

    item = q8tk_list_item_new_with_label( simage[ STR_DISK_IMAGE_ENPTY ] );
    q8tk_widget_show( item );
    q8tk_container_add( w->list, item );		/* <<なし>> ITEM */
    q8tk_signal_connect( item, "select",
			 cb_disk_image, (void *)( (-1 <<8) +drv ) );


    if( disk_image_exist( drv ) ){		/* ディスク挿入済み */

      for( i=0; i<disk_image_num_of_drive(drv); i++ ){
	char wk[80], euc_name[40];

	sjis2euc( euc_name, drive[drv].image[i].name );

	/* TODO : 半角のイメージ名の場合の長さ問題 */

	sprintf( wk, "%3d  %-16s %s ",
		 i+1, euc_name, (drive[drv].image[i].protect)?" RO":" RW" );

	item = q8tk_list_item_new_with_label( wk );
	q8tk_widget_show( item );
	q8tk_container_add( w->list, item );
	q8tk_signal_connect( item, "select",
			     cb_disk_image, (void *)( (i<<8) +drv ) );
      }

      if( drive_check_empty( drv ) ){			/* <<なし>> の時は   */
	q8tk_list_select_item( w->list, 0 );		/*      0 番目のITEM */
      }else{						/* そうでなければ    */
	q8tk_list_select_item( w->list,			/*  選択image の ITEM*/
			       drive[drv].selected_image + 1 );
      }


      w->func[0] = IMG_CLOSE;
      w->func[1] = IMG_ATTR;
      q8tk_label_set( w->label[0], sbutton[ w->func[0] ][drv] );
      q8tk_label_set( w->label[1], sbutton[ w->func[1] ][drv] );

      if( drive[drv].read_only ){
	q8tk_label_set( w->attr_label, sinfo[ STR_DISK_INFO_ATTR_RO ] );
      }else{
	q8tk_label_set( w->attr_label, sinfo[ STR_DISK_INFO_ATTR_RW ] );
      }

      {
	char	wk[32];
	char	wk2[20];
	const char *s;

	if( drive[drv].detect_broken_image ){		/* 破損あり */
	  s = sinfo[ STR_DISK_INFO_NR_BROKEN ];
	}else
	if( drive[drv].over_image ||			/* イメージ多過ぎ */
	    drive[drv].image_nr > 99 ){
	  s = sinfo[ STR_DISK_INFO_NR_OVER ];
	}else{
	  s = "";
	}

	sprintf( wk, "%2d%s", 
		 (drive[drv].image_nr>99) ? 99 : drive[drv].image_nr, s );

	sprintf( wk2, "%9.9s", wk );			/* 9文字右詰めに変換 */
	q8tk_label_set( w->num_label,  wk2 );
      }

    }else{					/* ドライブ空っぽ */

      q8tk_list_select_item( w->list, 0 );		    /* <<なし>> ITEM */

      if( disk_image_exist( drv^1 ) ) w->func[0] = IMG_COPY;
      else			      w->func[0] = IMG_BOTH;
      w->func[1] = IMG_OPEN;
      q8tk_label_set( w->label[0], sbutton[ w->func[0] ][drv] );
      q8tk_label_set( w->label[1], sbutton[ w->func[1] ][drv] );

      q8tk_label_set( w->attr_label, sinfo[ STR_DISK_INFO_NODISK ] );
      q8tk_label_set( w->num_label,  sinfo[ STR_DISK_INFO_NODISK ] );

    }


    if( get_drive_ready( drv ) ){
      q8tk_label_set( w->stat_label, sinfo[ STR_DISK_INFO_STAT_READY ] );
    }else{
      q8tk_label_set( w->stat_label, sinfo[ STR_DISK_INFO_STAT_BUSY ] );
    }
  }

}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/*
 *  ブランクイメージファイルの新規作成
 */
static	char	disk_blank_filename[ Q8TK_MAX_FILENAME ];


/*	ファイル選択後の処理。ファイルが存在すれば、追記確認のダイアログ */
static	void	cb_disk_blank_warn_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    cb_disk_attr_file_ctrl( ATTR_CREATE, disk_blank_filename );
  }
}
static void cb_disk_blank_ok( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  const char	*(*s) = str_disk_blank[ menu_lang ];

  strncpy( disk_blank_filename, q8tk_file_selection_get_filename(fsel),
	   Q8TK_MAX_FILENAME-1 );
  disk_blank_filename[ Q8TK_MAX_FILENAME-1 ] = '\0';

  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );

  if( osd_file_stat( disk_blank_filename ) == FILE_STAT_FILE ){
    /* すでにファイルが存在します。ブランクを追加しますか？ */
    dialog_create();
    {
      dialog_set_title( s[ STR_DISK_BLANK_WARN_0 ] );
      dialog_set_title( s[ STR_DISK_BLANK_WARN_1 ] );
      dialog_set_separator();
      dialog_set_button( s[ STR_DISK_BLANK_WARN_APPEND ],
			 cb_disk_blank_warn_clicked, (void*)TRUE );
      dialog_set_button( s[ STR_DISK_BLANK_WARN_CANCEL ],
			 cb_disk_blank_warn_clicked, (void*)FALSE );
      dialog_accel_key( Q8TK_KEY_ESC );
    }
    dialog_start();
  }else{
    cb_disk_attr_file_ctrl( ATTR_CREATE, disk_blank_filename );
  }
}

/*	ファイル選択処理。ファイルセレクションを使用	*/
static void cb_disk_blank_cancel( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static	void	cb_disk_blank( Q8tkWidget *dummy_0, void *dummy_1 )
{
  Q8tkWidget *fselect;
  const char	*(*s) = str_disk_blank[ menu_lang ];

  fselect = q8tk_file_selection_new( s[ STR_DISK_BLANK_FSEL ] );

  q8tk_widget_show( fselect );
  q8tk_grab_add( fselect );


  /* ブランクイメージ生成の際の、ファイル選択時のディレクトリは？ */
  /* 以下の設定では、現在ドライブに挿入しているイメージと同じディレクトリ
     となる。(空なら、QUASI88_DISK_DIR、だめなら カレント)
     以下の設定をしない (#if 0) 場合は、常にカレントになる。*/
#if 1
  q8tk_file_selection_set_filename( fselect, menu_pathname );
#endif


  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->ok_button,
		      "clicked", cb_disk_blank_ok, fselect );
  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->cancel_button,
		      "clicked", cb_disk_blank_cancel, fselect );
  q8tk_widget_grab_default( Q8TK_FILE_SELECTION( fselect )->cancel_button);


  misc_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( misc_accel, fselect );
  q8tk_accel_group_add( misc_accel, Q8TK_KEY_ESC,
		        Q8TK_FILE_SELECTION( fselect )->cancel_button,
		        "clicked" );
}


















static	Q8tkWidget	*menu_disk( void )
{
  Q8tkWidget	*hbox;
  int	i,j;
  const char *(*sinfo)             = str_disk_info[ menu_lang ];
  const char *(*simage)            = str_disk_image[ menu_lang ];


  hbox = q8tk_hbox_new();

  for( i=0; i<countof(disk_info); i++ ){
    T_DISK_INFO	*w = &disk_info[i];
    Q8tkWidget	*vbox, *label, *swin, *vsep;

    vbox = q8tk_vbox_new();

    {
      if( i==0 ) label = q8tk_label_new( simage[ STR_DISK_IMAGE_DRIVE_1 ] );
      else       label = q8tk_label_new( simage[ STR_DISK_IMAGE_DRIVE_2 ] );
      q8tk_widget_show( label );

      swin  = q8tk_scrolled_window_new( NULL, NULL );
      q8tk_widget_show( swin );
      q8tk_scrolled_window_set_policy( swin, Q8TK_POLICY_NEVER,
					     Q8TK_POLICY_AUTOMATIC );
      q8tk_misc_set_size( swin, 29, 11 );

      w->list = q8tk_list_new();
      q8tk_widget_show( w->list );
      q8tk_container_add( swin, w->list );

      for( j=0; j<2; j++ ){
	w->label[j] = q8tk_label_new( "" );	/* 空ラベルのウィジット確保 */
	q8tk_widget_show( w->label[j] );
	w->button[j] = q8tk_button_new();
	q8tk_widget_show( w->button[j] );
	q8tk_container_add( w->button[j], w->label[j] );
	q8tk_signal_connect( w->button[j], "clicked",
			     cb_disk_button, (void *)( (j<<8) + i ) );
      }
    }

    q8tk_box_pack_start( vbox, label );
    q8tk_box_pack_start( vbox, swin  );
    q8tk_box_pack_start( vbox, w->button[0] );
    q8tk_box_pack_start( vbox, w->button[1] );

    q8tk_widget_show( vbox );
    q8tk_box_pack_start( hbox, vbox );

    vsep = q8tk_vseparator_new();
    q8tk_widget_show( vsep );
    q8tk_box_pack_start( hbox, vsep );
  }

  {
    Q8tkWidget	*vbox = q8tk_vbox_new();

    for( i=0; i<countof(disk_info); i++ ){
      T_DISK_INFO	*w = &disk_info[i];
      Q8tkWidget	*frame, *lvbox, *lhbox, *label;

      if( i==0 ) frame = q8tk_frame_new( sinfo[ STR_DISK_INFO_DRIVE_1 ] );
      else       frame = q8tk_frame_new( sinfo[ STR_DISK_INFO_DRIVE_2 ] );
      q8tk_widget_show( frame );
/*    q8tk_frame_set_shadow_type( frame, Q8TK_SHADOW_ETCHED_OUT );*/
      q8tk_frame_set_shadow_type( frame, Q8TK_SHADOW_IN );

      lvbox = q8tk_vbox_new();
      q8tk_widget_show( lvbox );

      {
	lhbox = q8tk_hbox_new();
	q8tk_widget_show( lhbox );

	label = q8tk_label_new( sinfo[ STR_DISK_INFO_STAT ] );
	q8tk_widget_show( label );
	q8tk_box_pack_start( lhbox, label );

	w->stat_label = q8tk_label_new( "" );	/* 空ラベルのウィジット確保 */
	q8tk_widget_show( w->stat_label );
	q8tk_box_pack_start( lhbox, w->stat_label );

	q8tk_box_pack_start( lvbox, lhbox );
      }
      {
	lhbox = q8tk_hbox_new();
	q8tk_widget_show( lhbox );

	label = q8tk_label_new( sinfo[ STR_DISK_INFO_ATTR ] );
	q8tk_widget_show( label );
	q8tk_box_pack_start( lhbox, label );

	w->attr_label = q8tk_label_new( "" );	/* 空ラベルのウィジット確保 */
	q8tk_widget_show( w->attr_label );
	q8tk_box_pack_start( lhbox, w->attr_label );

	q8tk_box_pack_start( lvbox, lhbox );
      }
      {
	lhbox = q8tk_hbox_new();
	q8tk_widget_show( lhbox );

	label = q8tk_label_new( sinfo[ STR_DISK_INFO_NR ] );
	q8tk_widget_show( label );
	q8tk_box_pack_start( lhbox, label );

	w->num_label = q8tk_label_new( "" );	/* 空ラベルのウィジット確保 */
	q8tk_widget_show( w->num_label );
	q8tk_box_pack_start( lhbox, w->num_label );
	q8tk_misc_set_placement( w->num_label, Q8TK_PLACEMENT_X_RIGHT, 0 );

	q8tk_box_pack_start( lvbox, lhbox );
      }

      q8tk_container_add( frame, lvbox );

      q8tk_box_pack_start( vbox, frame );
    }

    {
      int i;
      Q8tkWidget *padding, *button;

      for( i=0; i<5; i++ ){			/* 位置調整のためダミーを何個か */
	padding = q8tk_label_new( "" );
	q8tk_widget_show( padding );
	q8tk_box_pack_start( vbox, padding );
      }

      button = q8tk_button_new_with_label( simage[STR_DISK_IMAGE_BLANK] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( vbox, button );

      q8tk_signal_connect( button, "clicked", cb_disk_blank, NULL );
    }

    q8tk_widget_show( vbox );
    q8tk_box_pack_start( hbox, vbox );
  }

  q8tk_widget_show( hbox );


  set_disk_widget();

  return	hbox;
}



/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/*					+-------+
 *	メイン画面 -- キー設定		|  Key  |
 *					|       +------------
 *
 */

/*
 *  ファンクションキー割り当ての変更(コンボ) 処理
 */

static	struct{
  int		str_num;
} key_fkey_data[] =
{
  { FN_FUNC,		},	/* この並びは str_key_fkey_fn[][] と */
  { FN_FRATE_UP,	},	/* あわせること！		     */
  { FN_FRATE_DOWN,	},	/* FN_XXX の番号順に並べる必要はない */
  { FN_VOLUME_UP,	},
  { FN_VOLUME_DOWN,	},
  { FN_PAUSE,		},
  { FN_RESIZE,		},
  { FN_NOWAIT,		},
  { FN_SPEED_UP,	},
  { FN_SPEED_DOWN,	},
  { FN_MOUSE_HIDE,	},
  { FN_DGA,		},
  { FN_SNAPSHOT,	},
  { FN_SUSPEND,		},
  { FN_IMAGE_NEXT1,	},
  { FN_IMAGE_PREV1,	},
  { FN_IMAGE_NEXT2,	},
  { FN_IMAGE_PREV2,	},
  { FN_NUMLOCK,         },
  { FN_RESET,           },
  { FN_KANA,		},
  { FN_ROMAJI,		},
  { FN_CAPS,		},
  { FN_KETTEI,		},
  { FN_HENKAN,		},
  { FN_ZENKAKU,		},
  { FN_PC,		},
};

static	void	cb_key_fkey( Q8tkWidget *widget, void *fn_key )
{
  int	i;
  const char	*(*s) = str_key_fkey_fn[ menu_lang ];

  for( i=0; i<countof(key_fkey_data); i++ ){
    if( strcmp( s[i], q8tk_combo_get_text(widget) )==0 ){
      switch( (int)fn_key ){
      case 0:	function_f6  = key_fkey_data[i].str_num;	break;
      case 1:	function_f7  = key_fkey_data[i].str_num;	break;
      case 2:	function_f8  = key_fkey_data[i].str_num;	break;
      case 3:	function_f9  = key_fkey_data[i].str_num;	break;
      case 4:	function_f10 = key_fkey_data[i].str_num;	break;
      }
      return;
    }
  }
}

static	int	get_key_fkey_initval( int fn_key )
{
  switch( fn_key ){
  case 0:	return function_f6;
  case 1:	return function_f7;
  case 2:	return function_f8;
  case 3:	return function_f9;
  case 4:	return function_f10;
  }
  return	FN_FUNC;
}

static	Q8tkWidget	*menu_key_fkey( void )
{
  int	i, j, k;
  Q8tkWidget	*vbox, *hbox, *combo, *label;
  Q8List	*list;
  const char	*(*s1) = str_key_fkey[ menu_lang ];
  const char	*(*s2) = str_key_fkey_fn[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( j=0; j<5; j++ ){

    list = NULL;
    hbox  = q8tk_hbox_new();

    {
      label = q8tk_label_new( s1[j] );
      q8tk_box_pack_start( hbox, label );
      q8tk_widget_show( label );
    }
    {
      combo = q8tk_combo_new();

      for( i=0; i<countof(key_fkey_data); i++ ){
	list = q8_list_append( list, (void *)s2[ i ] );
      }
      q8tk_combo_popdown_strings( combo, list );
      q8_list_free( list );
  
      k = get_key_fkey_initval(j);
      for( i=0; i<countof(key_fkey_data); i++ ){
	if( k == key_fkey_data[ i ].str_num ) break;
      }
      if( i<countof(key_fkey_data) ) ;
      else i=0;

      q8tk_combo_set_text( combo, s2[i] );
      q8tk_misc_set_size( combo, 45, 0 );

      q8tk_signal_connect( combo, "changed", cb_key_fkey, (void*)j );

      q8tk_box_pack_start( hbox, combo );
      q8tk_widget_show( combo );
    }

    q8tk_box_pack_start( vbox, hbox );
    q8tk_widget_show( hbox );

  }
  q8tk_widget_show( vbox );

  return vbox;
}

/*
 *  キー設定の変更(チェックボタン) 処理
 */
static	struct{
  int		str_num;
} key_cfg_data[] =
{
  { STR_KEY_CFG_TENKEY,  },
  { STR_KEY_CFG_CURSOR,  },
  { STR_KEY_CFG_NUMLOCK, },
};
static	void	cb_key_cfg( Q8tkWidget *widget, void *p )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  switch( (int)p ){
  case STR_KEY_CFG_TENKEY:
    if( key ) set_tenkey_emu_effective();
    else      set_tenkey_emu_invalid();
    return;
  case STR_KEY_CFG_CURSOR:
    if( key ) set_cursor_emu_effective();
    else      set_cursor_emu_invalid();
    return;
  case STR_KEY_CFG_NUMLOCK:
    if( key ) set_numlock_emu_effective();
    else      set_numlock_emu_invalid();
    return;
  }
}
static	int	get_key_cfg_initval( int type )
{
  switch( type ){
  case STR_KEY_CFG_TENKEY:	return	tenkey_emu;
  case STR_KEY_CFG_CURSOR:	return	cursor_emu;
  case STR_KEY_CFG_NUMLOCK:	return	numlock_emu;
  }
  return FALSE;
}
static	Q8tkWidget	*menu_key_cfg( void )
{
  int	i;
  Q8tkWidget	*vbox, *button;
  const char	*(*s) = str_key_cfg[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(key_cfg_data); i++ ){

    button = q8tk_check_button_new_with_label( s[key_cfg_data[i].str_num] );

    q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );

    if( get_key_cfg_initval(key_cfg_data[i].str_num) ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    q8tk_signal_connect( button, "toggled",
			 cb_key_cfg, (void *)key_cfg_data[i].str_num );
  }

  q8tk_widget_show( vbox );

  return vbox;
}


/*
 *  キーボードダイアログ
 */

typedef struct{
  int	type;		/* 0 … 終端 / 1 … ボタン / 2 … 区切りラベル	*/
  char	*top;		/* キートップの文字 				*/
  int	port;		/* キースキャンポート				*/
  int	bit;		/* キースキャンビット				*/
} t_keymap;

static	void	keymap_create( int model,
			       int (*initval)( t_keymap * ),
			       Q8tkSignalFunc cb_toggled );
static	void	keymap_add_button( Q8tkWidget *button, int accel_esc );
static	void	keymap_start( void );
static	void	keymap_finish( void );





static t_keymap keymap_old0[] =
{
  { 1, "STOP",      9, 0, },
  { 1, "COPY",     10, 4, },
  { 2, " ",                  0, 0, },
  { 1, "   f1   ",  9, 1, },
  { 1, "   f2   ",  9, 2, },
  { 1, "   f3   ",  9, 3, },
  { 1, "   f4   ",  9, 4, },
  { 1, "   f5   ",  9, 5, },
  { 2, "   ",                0, 0, },
  { 1, "R-UP",     11, 0, },
  { 1, "R-DW",     11, 2, },
  { 2, "   ",                0, 0, },
  { 1, " \036 ",    8, 1, },/*↑*/
  { 1, " \037 ",   10, 1, },/*↓*/
  { 1, " \035 ",   10, 2, },/*←*/
  { 1, " \034 ",    8, 2, },/*→*/
  { 0, 0, 0, 0, },
};
static t_keymap keymap_old1[] =
{
  { 1, " ESC ", 9, 7, },
  { 1, " 1 ",   6, 1, },
  { 1, " 2 ",   6, 2, },
  { 1, " 3 ",   6, 3, },
  { 1, " 4 ",   6, 4, },
  { 1, " 5 ",   6, 5, },
  { 1, " 6 ",   6, 6, },
  { 1, " 7 ",   6, 7, },
  { 1, " 8 ",   7, 0, },
  { 1, " 9 ",   7, 1, },
  { 1, " 0 ",   6, 0, },
  { 1, " - ",   5, 7, },
  { 1, " ^ ",   5, 6, },
  { 1, " \\ ",  5, 4, },
  { 1, " BS ",  8, 3, },
  { 2, "   ",            0, 0, },
  { 1, "CLR",   8, 0, },
  { 1, "HLP",  10, 3, },
  { 1, " - ",  10, 5, },
  { 1, " / ",  10, 6, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_old2[] =
{
  { 1, "  TAB  ",10, 0, },
  { 1, " Q ",     4, 1, },
  { 1, " W ",     4, 7, },
  { 1, " E ",     2, 5, },
  { 1, " R ",     4, 2, },
  { 1, " T ",     4, 4, },
  { 1, " Y ",     5, 1, },
  { 1, " U ",     4, 5, },
  { 1, " I ",     3, 1, },
  { 1, " O ",     3, 7, },
  { 1, " P ",     4, 0, },
  { 1, " @ ",     2, 0, },
  { 1, " [ ",     5, 3, },
  { 1, "RETURN ",14, 0, },
  { 2, "   ",             0, 0, },
  { 1, " 7 ",     0, 7, },
  { 1, " 8 ",     1, 0, },
  { 1, " 9 ",     1, 1, },
  { 1, " * ",     1, 2, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_old3[] =
{
  { 1, "CTRL",  8, 7, },
  { 1, "CAPS", 10, 7, },
  { 1, " A ",   2, 1, },
  { 1, " S ",   4, 3, },
  { 1, " D ",   2, 4, },
  { 1, " F ",   2, 6, },
  { 1, " G ",   2, 7, },
  { 1, " H ",   3, 0, },
  { 1, " J ",   3, 2, },
  { 1, " K ",   3, 3, },
  { 1, " L ",   3, 4, },
  { 1, " ; ",   7, 3, },
  { 1, " : ",   7, 2, },
  { 1, " ] ",   5, 5, },
  { 2, "         ",     0, 0, },
  { 1, " 4 ",   0, 4, },
  { 1, " 5 ",   0, 5, },
  { 1, " 6 ",   0, 6, },
  { 1, " + ",   1, 3, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_old4[] =
{
  { 1, "    SHIFT   ",14, 2, },
  { 1, " Z ",          5, 2, },
  { 1, " X ",          5, 0, },
  { 1, " C ",          2, 3, },
  { 1, " V ",          4, 6, },
  { 1, " B ",          2, 2, },
  { 1, " N ",          3, 6, },
  { 1, " M ",          3, 5, },
  { 1, " , ",          7, 4, },
  { 1, " . ",          7, 5, },
  { 1, " / ",          7, 6, },
  { 1, " _ ",          7, 7, },
  { 1, " SHIFT ",     14, 3, },
  { 2, "   ",                  0, 0, },
  { 1, " 1 ",          0, 1, },
  { 1, " 2 ",          0, 2, },
  { 1, " 3 ",          0, 3, },
  { 1, " = ",          1, 4, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_old5[] =
{
  { 2, "       ",           0, 0, },
  { 1, "KANA",      8, 5, },
  { 1, "GRPH",      8, 4, },
  { 1, "                                           ", 9, 6, },
  { 2, "                 ", 0, 0, },
  { 1, " 0 ",       0, 0, },
  { 1, " , ",       1, 5, },
  { 1, " . ",       1, 6, },
  { 1, "RET",      14, 1, },
  { 0, 0, 0, 0, },
};

static t_keymap keymap_new0[] =
{
  { 1, "STOP",    9, 0, },
  { 1, "COPY",   10, 4, },
  { 2, "  ",           0, 0, },
  { 1, " f1 ",    9, 1, },
  { 1, " f2 ",    9, 2, },
  { 1, " f3 ",    9, 3, },
  { 1, " f4 ",    9, 4, },
  { 1, " f5 ",    9, 5, },
  { 2, "   ",          0, 0, },
  { 1, " f6 ",   12, 0, },
  { 1, " f7 ",   12, 1, },
  { 1, " f8 ",   12, 2, },
  { 1, " f9 ",   12, 3, },
  { 1, " f10 ",  12, 4, },
  { 2, "   ",          0, 0, },
  { 1, "ROLUP",  11, 0, },
  { 1, "ROLDW",  11, 2, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_new1[] =
{
  { 1, " ESC ",  9, 7, },
  { 1, " 1 ",    6, 1, },
  { 1, " 2 ",    6, 2, },
  { 1, " 3 ",    6, 3, },
  { 1, " 4 ",    6, 4, },
  { 1, " 5 ",    6, 5, },
  { 1, " 6 ",    6, 6, },
  { 1, " 7 ",    6, 7, },
  { 1, " 8 ",    7, 0, },
  { 1, " 9 ",    7, 1, },
  { 1, " 0 ",    6, 0, },
  { 1, " - ",    5, 7, },
  { 1, " ^ ",    5, 6, },
  { 1, " \\ ",   5, 4, },
  { 1, " BS ",  12, 5, },
  { 2, "   ",         0, 0, },
  { 1, " DEL ", 12, 6, },
  { 1, " INS ", 12, 7, },
  { 2, "  ",          0, 0, },
  { 1, "CLR",    8, 0, },
  { 1, "HLP",   10, 3, },
  { 1, " - ",   10, 5, },
  { 1, " / ",   10, 6, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_new2[] =
{
  { 1, "  TAB  ",10, 0, },
  { 1, " Q ",     4, 1, },
  { 1, " W ",     4, 7, },
  { 1, " E ",     2, 5, },
  { 1, " R ",     4, 2, },
  { 1, " T ",     4, 4, },
  { 1, " Y ",     5, 1, },
  { 1, " U ",     4, 5, },
  { 1, " I ",     3, 1, },
  { 1, " O ",     3, 7, },
  { 1, " P ",     4, 0, },
  { 1, " @ ",     2, 0, },
  { 1, " [ ",     5, 3, },
  { 1, "RETURN ",14, 0, },
  { 2, "                   ",             0, 0, },
  { 1, " 7 ",     0, 7, },
  { 1, " 8 ",     1, 0, },
  { 1, " 9 ",     1, 1, },
  { 1, " * ",     1, 2, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_new3[] =
{
  { 1, "CTRL",  8, 7, },
  { 1, "CAPS", 10, 7, },
  { 1, " A ",   2, 1, },
  { 1, " S ",   4, 3, },
  { 1, " D ",   2, 4, },
  { 1, " F ",   2, 6, },
  { 1, " G ",   2, 7, },
  { 1, " H ",   3, 0, },
  { 1, " J ",   3, 2, },
  { 1, " K ",   3, 3, },
  { 1, " L ",   3, 4, },
  { 1, " ; ",   7, 3, },
  { 1, " : ",   7, 2, },
  { 1, " ] ",   5, 5, },
  { 2, "             ", 0, 0, },
  { 1, " \036 ",8, 1, },/*↑*/
  { 2, "       ",     0, 0, },
  { 1, " 4 ",   0, 4, },
  { 1, " 5 ",   0, 5, },
  { 1, " 6 ",   0, 6, },
  { 1, " + ",   1, 3, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_new4[] =
{
  { 1, "    SHIFT   ",14, 2, },
  { 1, " Z ",          5, 2, },
  { 1, " X ",          5, 0, },
  { 1, " C ",          2, 3, },
  { 1, " V ",          4, 6, },
  { 1, " B ",          2, 2, },
  { 1, " N ",          3, 6, },
  { 1, " M ",          3, 5, },
  { 1, " , ",          7, 4, },
  { 1, " . ",          7, 5, },
  { 1, " / ",          7, 6, },
  { 1, " _ ",          7, 7, },
  { 1, " SHIFT ",     14, 3, },
  { 2, "  ",                 0, 0, },
  { 1, " \035 ",      10, 2, },/*←*/
  { 1, " \037 ",      10, 1, },/*↓*/
  { 1, " \034 ",       8, 2, },/*→*/
  { 2, "  ",                 0, 0, },
  { 1, " 1 ",          0, 1, },
  { 1, " 2 ",          0, 2, },
  { 1, " 3 ",          0, 3, },
  { 1, " = ",          1, 4, },
  { 0, 0, 0, 0, },
};
static t_keymap keymap_new5[] =
{
  { 2, "       ",           0, 0, },
  { 1, "KANA",         8, 5, },
  { 1, "GRPH",         8, 4, },
  { 1, " KETTEI ",    13, 1, },
  { 1, "           ",  9, 6, },
  { 1, "  HENKAN  ",  13, 0, },
  { 1, "PC ",        13, 2, },
  { 1, "ZEN",        13, 3, },
  { 2, "                                 ", 0, 0, },
  { 1, " 0 ",          0, 0, },
  { 1, " , ",          1, 5, },
  { 1, " . ",          1, 6, },
  { 1, "RET",         14, 1, },
  { 0, 0, 0, 0, },
};

static t_keymap *keymap_line[2][6] =
{
  {
    keymap_old0,
    keymap_old1,
    keymap_old2,
    keymap_old3,
    keymap_old4,
    keymap_old5,
  },
  {
    keymap_new0,
    keymap_new1,
    keymap_new2,
    keymap_new3,
    keymap_new4,
    keymap_new5,
  },
};

static	Q8tkWidget	*keymap[127 +2];
static	int		keymap_num;
static	Q8tkWidget	*keymap_accel;

enum {			/* keymap[] は以下のウィジットに使う	*/
  KEYMAP_WIN,

  KEYMAP_VBOX,
  KEYMAP_SCRL,
  KEYMAP_SEP,
  KEYMAP_HBOX,

  KEYMAP_VBOX_1,
  KEYMAP_HBOX_1,
  KEYMAP_HBOX_2,
  KEYMAP_HBOX_3,
  KEYMAP_HBOX_4,
  KEYMAP_HBOX_5,
  KEYMAP_HBOX_6,

  KEYMAP_KEY
};



/*
 *	キーマップダイアログの生成
 */
static	void	keymap_create( int model,
			       int (*initval)( t_keymap * ),
			       Q8tkSignalFunc cb_toggled )
{
  int i,j;
  for( i=0; i<countof(keymap); i++ ) keymap[i] = NULL;

  keymap[ KEYMAP_WIN ] = q8tk_window_new( Q8TK_WINDOW_DIALOG );

  keymap_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( keymap_accel, keymap[ KEYMAP_WIN ] );


  keymap[ KEYMAP_VBOX ] = q8tk_vbox_new();
  q8tk_container_add( keymap[ KEYMAP_WIN ], keymap[ KEYMAP_VBOX ] );
  q8tk_widget_show( keymap[ KEYMAP_VBOX ] );
  {
    keymap[ KEYMAP_SCRL ] = q8tk_scrolled_window_new( NULL, NULL );
    q8tk_box_pack_start( keymap[ KEYMAP_VBOX ], keymap[ KEYMAP_SCRL ] );
    q8tk_misc_set_size( keymap[ KEYMAP_SCRL ], 80, 21 );
    q8tk_scrolled_window_set_policy( keymap[ KEYMAP_SCRL ],
				     Q8TK_POLICY_AUTOMATIC,
				     Q8TK_POLICY_NEVER );
    q8tk_widget_show( keymap[ KEYMAP_SCRL ] );

    keymap[ KEYMAP_SEP ] = q8tk_label_new( "" );	/* 空行 */
    q8tk_box_pack_start( keymap[ KEYMAP_VBOX ], keymap[ KEYMAP_SEP ] );
    q8tk_widget_show( keymap[ KEYMAP_SEP ] );

    keymap[ KEYMAP_HBOX ] = q8tk_hbox_new();
    q8tk_box_pack_start( keymap[ KEYMAP_VBOX ], keymap[ KEYMAP_HBOX ] );
    q8tk_misc_set_placement( keymap[ KEYMAP_HBOX ], 
			     Q8TK_PLACEMENT_X_CENTER, 0 );
    q8tk_widget_show( keymap[ KEYMAP_HBOX ] );
   
    /* keymap_add_button() で、この VBOX にボタンを並べることが出来る */
  }


  /* 以下、キーボードもどきををボタンで配置する */

  keymap[ KEYMAP_VBOX_1 ] = q8tk_vbox_new();
  q8tk_container_add( keymap[ KEYMAP_SCRL ], keymap[ KEYMAP_VBOX_1 ] );
  q8tk_widget_show( keymap[ KEYMAP_VBOX_1 ] );

  keymap_num = KEYMAP_KEY;

  for( j=0; j<6; j++ ){

    t_keymap *p = keymap_line[ model ][ j ];

    keymap[ j + KEYMAP_HBOX_1 ] = q8tk_hbox_new();
    q8tk_box_pack_start( keymap[ KEYMAP_VBOX_1 ], keymap[ j +KEYMAP_HBOX_1 ] );
    q8tk_widget_show( keymap[ j + KEYMAP_HBOX_1 ] );

    for( i=0; p[ i ].type; i++ ){

      if( keymap_num >= countof( keymap ) ){	/* トラップ */
	fprintf( stderr, "%s %d\n", __FILE__, __LINE__ ); break;
      }

      if( p[i].type == 1 ){
	keymap[ keymap_num ] = q8tk_toggle_button_new_with_label( p[i].top );
	if( initval ){
	  if( (*initval)( &p[i] ) ){
	    q8tk_toggle_button_set_state( keymap[ keymap_num ], TRUE );
	  }
	}
	q8tk_signal_connect( keymap[ keymap_num ], "toggled", 
			     cb_toggled, &p[ i ] );
      }else if( p[i].type == 2 ){
	keymap[ keymap_num ] = q8tk_label_new( p[i].top );
      }
      q8tk_box_pack_start( keymap[ j + KEYMAP_HBOX_1 ], keymap[ keymap_num ] );
      q8tk_widget_show( keymap[ keymap_num ] );

      keymap_num++;
    }
  }
}

/*
 *	ソフトウェアキーボードダイアログにボタンを追加
 *		button (ボタン) ウィジットは生成済みであること
 *		必要なシグナル設定も終わっていること。
 *		引数 accel_esc が真なら、ESCキー押下で "clocked" シグナルが
 *			button に通知される (アクセラレータキー)
 */
static	void	keymap_add_button( Q8tkWidget *button, int accel_esc )
{
  if( keymap_num >= countof( keymap ) ){	/* トラップ */
    fprintf( stderr, "%s %d\n", __FILE__, __LINE__ ); return;
  }

  keymap[ keymap_num ] = button;
  q8tk_box_pack_start( keymap[ KEYMAP_HBOX ], keymap[ keymap_num ] );
  q8tk_widget_show( keymap[ keymap_num ] );
  if( accel_esc ){
    q8tk_accel_group_add( keymap_accel, Q8TK_KEY_ESC, 
			  keymap[ keymap_num ], "clicked" );
  }
  keymap_num++;
}

/*
 *	キーマップダイアログの表示
 */
static	void	keymap_start( void )
{
  q8tk_widget_show( keymap[ KEYMAP_WIN ] );
  q8tk_grab_add( keymap[ KEYMAP_WIN ] );

  q8tk_widget_grab_default( keymap[ keymap_num-1 ] );
}

/*
 *	キーマップダイアログの終了・削除
 */
static	void	keymap_finish( void )
{
  int	i;
  for( i=keymap_num-1; i; i-- ){
    if( keymap[i] ){
      q8tk_widget_destroy( keymap[i] );
    }
  }

  q8tk_grab_remove( keymap[ KEYMAP_WIN ] );
  q8tk_widget_destroy( keymap[ KEYMAP_WIN ] );
  q8tk_widget_destroy( keymap_accel );
}




/*
 *  ソフトウェアキーボード
 */

/*	ボタントグル時のコールバック	*/
static	void	cb_key_softkey( Q8tkWidget *button, void *keymap )
{
  t_keymap   *p = (t_keymap *)keymap;

  if( Q8TK_TOGGLE_BUTTON(button)->active ) key_scan[ p->port ] &= ~(1<<p->bit);
  else                                     key_scan[ p->port ] |=  (1<<p->bit);
}

/*	ボタンの初期トグル状態		*/
static	int	get_key_softkey_initval( t_keymap *p )
{
  return (key_scan[ p->port ] & (1<<p->bit)) ? FALSE : TRUE;
}

/*	全てオフにして戻るボタン押下	*/
static	void	cb_key_softkey_release( Q8tkWidget *dummy_0, void *dummy_1 )
{
  int i;
  for( i=0; i<0x10; i++ ) key_scan[i] = 0xff;
  keymap_finish();
}

/*	戻るボタン押下			*/
static	void	cb_key_softkey_end( Q8tkWidget *dummy_0, void *dummy_1 )
{
  int	 my_port, your_port;
  byte my_val,  your_val,  save_val;


  /* 左右の RET をポートに反映 */
  if( (key_scan[ 0x0e ] & 0x03)==0x03 ) key_scan[ 0x01 ] |=  (0x80);
  else                                  key_scan[ 0x01 ] &= ~(0x80);

  /* 左右の SHIFT をポートに反映 */
  if( (key_scan[ 0x0e ] & 0x0c)==0x0c ) key_scan[ 0x08 ] |=  (0x40);
  else                                  key_scan[ 0x08 ] &= ~(0x40);

  if( ROM_VERSION < '8' ){

    /* 前期型キーボードは、左右の区別なし */
    key_scan[ 0x0e ] = 0xff;
    
  }else{

    /* 後期型キーボードは、F6～F10 や INS、DELが拡張されている */
    if( (key_scan[ 0x0c ] & 0x01)==0x00 ){
      key_scan[ 0x09 ] &= ~(0x02);
      key_scan[ 0x08 ] &= ~(0x40);
    }
    if( (key_scan[ 0x0c ] & 0x02)==0x00 ){
      key_scan[ 0x09 ] &= ~(0x04);
      key_scan[ 0x08 ] &= ~(0x40);
    }
    if( (key_scan[ 0x0c ] & 0x04)==0x00 ){
      key_scan[ 0x09 ] &= ~(0x08);
      key_scan[ 0x08 ] &= ~(0x40);
    }
    if( (key_scan[ 0x0c ] & 0x08)==0x00 ){
      key_scan[ 0x09 ] &= ~(0x10);
      key_scan[ 0x08 ] &= ~(0x40);
    }
    if( (key_scan[ 0x0c ] & 0x10)==0x00 ){
      key_scan[ 0x09 ] &= ~(0x20);
      key_scan[ 0x08 ] &= ~(0x40);
    }
    if( (key_scan[ 0x0c ] & 0x20)==0x00 ){
      key_scan[ 0x08 ] &= ~(0x08);
    }
    if( (key_scan[ 0x0c ] & 0x40)==0x00 ){
      key_scan[ 0x08 ] &= ~(0x08);
      key_scan[ 0x08 ] &= ~(0x40);
    }
    if( (key_scan[ 0x0c ] & 0x80)==0x00 ){
      key_scan[ 0x08 ] &= ~(0x08);
    }
    if( (key_scan[ 0x0d ] & 0x01)==0x00 ){
      key_scan[ 0x09 ] &= ~(0x40);
    }
    if( (key_scan[ 0x0d ] & 0x02)==0x00 ){
      key_scan[ 0x09 ] &= ~(0x40);
    }
  }


  /* 複数のキーを同時押下した時の、ハードバグを再現する */

  save_val = key_scan[8] & 0xf0;	/* port 8 の 上位 4bit は対象外 */
  key_scan[8] |= 0xf0;

  for( my_port=0; my_port<12; my_port++ ){
    for( your_port=0; your_port<12; your_port++ ){

      if( my_port==your_port ) continue;

      my_val   = key_scan[ my_port ];
      your_val = key_scan[ your_port ];
	
      if( ( my_val | your_val ) != 0xff ){
	key_scan[ my_port ]   =
	  key_scan[ your_port ] = my_val & your_val;
      }

    }
  }

  key_scan[8] &= ~0xf0;
  key_scan[8] |= save_val;


  /* ウィジットを消滅 */
  keymap_finish();
}


/*
 *	ソフトウェアキーボード処理
 */
static	void	cb_key_softkeyboard( Q8tkWidget *dummy_0, void *dummy_1 )
{
  const char	*(*s) = str_skey_set[ menu_lang ];
  Q8tkWidget	*button;


  if( ROM_VERSION < '8' ){

    /* RET を 左右 RET ポートに反映 */
    if( (key_scan[ 0x01 ] & 0x80)==0x80 ) key_scan[ 0x0e ] |=  (0x03);
    else                                  key_scan[ 0x0e ] &= ~(0x03);

    /* SHIFT を 左右SHIFT をポートに反映 */
    if( (key_scan[ 0x08 ] & 0x40)==0x40 ) key_scan[ 0x0e ] |=  (0x0c);
    else                                  key_scan[ 0x0e ] &= ~(0x0c);

  }else{
    /* F6～F10をSHIFTに反映させてもいいけど……やめとく? */
  }


  /* キーボードダイアログ作成 */

  keymap_create( ( ROM_VERSION < '8' ) ? 0 : 1 ,
		 get_key_softkey_initval, cb_key_softkey );


  /* ボタンを2個追加 */

  button = q8tk_button_new_with_label( s[ STR_SKEY_CAN ]);
  q8tk_signal_connect( button, "clicked", cb_key_softkey_release, 0 );
  keymap_add_button( button, FALSE );

  button = q8tk_button_new_with_label( s[ STR_SKEY_QUIT ] );
  q8tk_signal_connect( button, "clicked", cb_key_softkey_end, 0 );
  keymap_add_button( button, TRUE );		/* アクセラレータキーは ESC */


  /* ソフトウェアキーボード 開始 */

  keymap_start();

}
static	Q8tkWidget	*menu_key_softkeyboard( void )
{
  Q8tkWidget	*button;
  const char	*(*s) = str_skey_set[ menu_lang ];

  button = q8tk_button_new_with_label( s[ STR_SKEY_SET ] );
  q8tk_widget_show( button );

  q8tk_toggle_button_set_state( button, TRUE );
  q8tk_signal_connect( button, "clicked", cb_key_softkeyboard, 0 );
  return button;
}


/*
 *  カーソルキーカスタマイズ 処理
 *				この処理は、floiさんにより提供されました。
 */
static	void	cb_key_cassign( Q8tkWidget *widget, void *p )
{
  int	key;
  char	buf[2];

  switch( (int)p ){
  case STR_KEY_CASSIGN_UP:
    strncpy( buf, q8tk_entry_get_text( widget ), 1 );
    buf[1] = '\0';
    q8tk_entry_set_text( widget, buf );
    set_cursor_assign_key( CURSOR_ASSIGN_UP, (byte)(buf[0]) );
    return;
  case STR_KEY_CASSIGN_LEFT:
    strncpy( buf, q8tk_entry_get_text( widget ), 1 );
    buf[1] = '\0';
    q8tk_entry_set_text( widget, buf );
    set_cursor_assign_key( CURSOR_ASSIGN_LEFT, (byte)(buf[0]) );
    return;
  case STR_KEY_CASSIGN_RIGHT:
    strncpy( buf, q8tk_entry_get_text( widget ), 1 );
    buf[1] = '\0';
    q8tk_entry_set_text( widget, buf );
    set_cursor_assign_key( CURSOR_ASSIGN_RIGHT, (byte)(buf[0]) );
    return;
  case STR_KEY_CASSIGN_DOWN:
    strncpy( buf, q8tk_entry_get_text( widget ), 1 );
    buf[1] = '\0';
    q8tk_entry_set_text( widget, buf );
    set_cursor_assign_key( CURSOR_ASSIGN_DOWN, (byte)(buf[0]) );
    return;
  case STR_KEY_CASSIGN_EFFECTIVE:
    key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
    if( key ) set_cursor_assign_effective();
    else      set_cursor_assign_invalid();
    return;
  }
}
static	int	get_key_cassign_initval( int type )
{
  switch( type ){
  case STR_KEY_CASSIGN_UP:	  return (int)get_cursor_assign_key( CURSOR_ASSIGN_UP );
  case STR_KEY_CASSIGN_LEFT:	  return (int)get_cursor_assign_key( CURSOR_ASSIGN_LEFT );
  case STR_KEY_CASSIGN_RIGHT:	  return (int)get_cursor_assign_key( CURSOR_ASSIGN_RIGHT );
  case STR_KEY_CASSIGN_DOWN:	  return (int)get_cursor_assign_key( CURSOR_ASSIGN_DOWN );
  case STR_KEY_CASSIGN_EFFECTIVE: return cursor_assign;
  }
  return 0;
}
static	Q8tkWidget	*menu_key_cassign( void )
{
  char		buf[2];
  int		code;
  Q8tkWidget	*vbox, *hbox, *label, *button, *entry;
  const char	*(*s) = str_key_cassign[ menu_lang ];

  buf[1] = '\0';

  vbox = q8tk_vbox_new();
  {
    hbox = q8tk_hbox_new();
    {
      label = q8tk_label_new( s[STR_KEY_CASSIGN_UP] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );

      entry = q8tk_entry_new();
      q8tk_misc_set_size( entry, 2, 1 );
      q8tk_widget_show( entry );
      q8tk_box_pack_start( hbox, entry );
      code = get_key_cassign_initval( STR_KEY_CASSIGN_UP );
      if( code != 0 ) {
	buf[0] = (char)code;
        q8tk_entry_set_text( entry, buf );
      }
      q8tk_signal_connect( entry,"changed", cb_key_cassign, (void *)STR_KEY_CASSIGN_UP );
    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );

    hbox = q8tk_hbox_new();
    {
      label = q8tk_label_new( s[STR_KEY_CASSIGN_LEFT] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );

      entry = q8tk_entry_new();
      q8tk_misc_set_size( entry, 2, 1 );
      q8tk_widget_show( entry );
      q8tk_box_pack_start( hbox, entry );
      code = get_key_cassign_initval( STR_KEY_CASSIGN_LEFT );
      if( code != 0 ) {
	buf[0] = (char)code;
        q8tk_entry_set_text( entry, buf );
      }
      q8tk_signal_connect( entry,"changed", cb_key_cassign, (void *)STR_KEY_CASSIGN_LEFT );

      label = q8tk_label_new( s[STR_KEY_CASSIGN_RIGHT] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );

      entry = q8tk_entry_new();
      q8tk_misc_set_size( entry, 2, 1 );
      q8tk_widget_show( entry );
      q8tk_box_pack_start( hbox, entry );
      code = get_key_cassign_initval( STR_KEY_CASSIGN_RIGHT );
      if( code != 0 ) {
	buf[0] = (char)code;
        q8tk_entry_set_text( entry, buf );
      }
      q8tk_signal_connect( entry,"changed", cb_key_cassign, (void *)STR_KEY_CASSIGN_RIGHT );

    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );

    hbox = q8tk_hbox_new();
    {
      label = q8tk_label_new( s[STR_KEY_CASSIGN_DOWN] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );

      entry = q8tk_entry_new();
      q8tk_misc_set_size( entry, 2, 1 );
      q8tk_widget_show( entry );
      q8tk_box_pack_start( hbox, entry );
      code = get_key_cassign_initval( STR_KEY_CASSIGN_DOWN );
      if( code != 0 ) {
	buf[0] = (char)code;
        q8tk_entry_set_text( entry, buf );
      }
      q8tk_signal_connect( entry,"changed", cb_key_cassign, (void *)STR_KEY_CASSIGN_DOWN );
    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );

    button = q8tk_check_button_new_with_label( s[STR_KEY_CASSIGN_EFFECTIVE] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );

    if( get_key_cassign_initval( STR_KEY_CASSIGN_EFFECTIVE ) ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    q8tk_signal_connect( button, "toggled",
			 cb_key_cassign, (void *)STR_KEY_CASSIGN_EFFECTIVE );
  }

  q8tk_widget_show( vbox );

  return vbox;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_key( void )
{
  Q8tkWidget *vbox, *hbox, *frame, *widget;
  const char *(*s) = str_key[ menu_lang ];

  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_KEY_FKEY ] );
    {
      widget = menu_key_fkey();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    hbox = q8tk_hbox_new();
    {
      frame = q8tk_frame_new( s[ STR_KEY_CFG ] );
      {
	widget = menu_key_cfg();
	q8tk_container_add( frame, widget );
      }
      q8tk_widget_show( frame );
      q8tk_box_pack_start( hbox, frame );
      frame = q8tk_frame_new( s[ STR_KEY_SKEY ] );
      {
	widget = menu_key_softkeyboard();
	q8tk_container_add( frame, widget );
      }
      q8tk_widget_show( frame );
      q8tk_box_pack_start( hbox, frame );
    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );

    frame = q8tk_frame_new( s[ STR_KEY_CASSIGN ] );
    {
      widget = menu_key_cassign();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
  }
  q8tk_widget_show( vbox );

  return vbox;
}








/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/*					+-------+
 *	メイン画面 -- マウス設定	| Mouse |
 *					|       +------------
 *
 */

/*
 *  マウスモード切替え(ラジオボタン) 処理
 */

static	struct{
  int		str_num;
  int		mouse_mode;
} mouse_mode_data[] =
{
  { STR_MOUSE_MODE_NOMOUSE,   0, },
  { STR_MOUSE_MODE_MOUSE,     1, },
  { STR_MOUSE_MODE_JOYMOUSE,  2, },
  { STR_MOUSE_MODE_JOYSTICK,  3, },
  { STR_MOUSE_MODE_JOYKEY,    4, },
};

static	void	cb_mouse_mode( Q8tkWidget *dummy, void *p )
{
  mouse_mode = (int)p;
}

static	int	get_mouse_mode_initval( void )
{
  return mouse_mode;
}

static	Q8tkWidget	*menu_mouse_mode( void )
{
  int	i;
  Q8tkWidget	*vbox, *button;
  Q8List	*list = NULL;
  const char	*(*s) = str_mouse_mode[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(mouse_mode_data); i++ ){

    button = q8tk_radio_button_new_with_label( list,
					       s[mouse_mode_data[i].str_num] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );
    q8tk_signal_connect( button, "clicked",
			 cb_mouse_mode, (void *)mouse_mode_data[i].mouse_mode );
    if( get_mouse_mode_initval()==mouse_mode_data[i].mouse_mode ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    list = q8tk_radio_button_group( button );

  }
  q8tk_widget_show( vbox );

  return vbox;
}


/*
 *  ジョイスティック設定変更処理
 */
static	struct{
  int		str_num;
} joystick_setting_data[] =
{
  { STR_MOUSE_JOYSWAP,        },
  { STR_MOUSE_JOYKEYASSIGN,   },
  { STR_MOUSE_JOYKEYASSIGN_A, },
  { STR_MOUSE_JOYKEYASSIGN_B, },
};

static	struct{
  int		str_num;
  int		joy88_key;
} joystick_assign_data[] =
{
  { STR_MOUSE_JOY_ASSIGN_NONE,  JOY88_KEY_NONE,  },
  { STR_MOUSE_JOY_ASSIGN_X,     JOY88_KEY_X,     },
  { STR_MOUSE_JOY_ASSIGN_Z,     JOY88_KEY_Z,     },
  { STR_MOUSE_JOY_ASSIGN_SPACE, JOY88_KEY_SPACE, },
  { STR_MOUSE_JOY_ASSIGN_RET,   JOY88_KEY_RET,   },
  { STR_MOUSE_JOY_ASSIGN_SHIFT, JOY88_KEY_SHIFT, },
  { STR_MOUSE_JOY_ASSIGN_ESC,   JOY88_KEY_ESC,   },
};


static	void	cb_joystick_swap( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  joy_swap_button = key;
}
static	int	get_joystick_swap_initval( void )
{
  return joy_swap_button;
}


static	void	cb_joystick_assign( Q8tkWidget *widget, void *button )
{
  int	i;
  const char	*(*s) = str_mouse_joy_assign[ menu_lang ];

  for( i=0; i<countof(joystick_assign_data); i++ ){
    if( strcmp( s[i], q8tk_combo_get_text(widget) ) == 0 ){
      joy_key_assign[ (int)button ] = i;
      return;
    }
  }
}

static	int	get_joystick_assign_initval( int button )
{
  return joy_key_assign[ button ];
}



static	Q8tkWidget	*menu_mouse_joystick( void )
{
  int i, j, k;
  Q8tkWidget	*vbox, *hbox, *label, *button, *combo;
  Q8List	*list;
  const char	*(*s) = str_mouse_joy[ menu_lang ];
  const char	*(*s2) = str_mouse_joy_assign[ menu_lang ];

  vbox = q8tk_vbox_new();

  button = q8tk_check_button_new_with_label( s[joystick_setting_data[0].str_num] );
  q8tk_widget_show( button );
  q8tk_box_pack_start( vbox, button );

  if( get_joystick_swap_initval() ){
    q8tk_toggle_button_set_state( button, TRUE );
  }
  q8tk_signal_connect( button, "toggled", cb_joystick_swap, 0 );

  label = q8tk_label_new( "" );
  q8tk_widget_show( label );
  q8tk_box_pack_start( vbox, label );

  button = q8tk_label_new( s[joystick_setting_data[1].str_num] );
  q8tk_widget_show( button );
  q8tk_box_pack_start( vbox, button );


  hbox = q8tk_hbox_new();
  {
    for( i=0; i<2; i++ ){
      label = q8tk_label_new( s[joystick_setting_data[2 + i].str_num] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );


      list = NULL;
      combo = q8tk_combo_new();

      for( j=0; j<countof(joystick_assign_data); j++ ){
	list = q8_list_append( list, 
			       (void *)s2[ joystick_assign_data[j].str_num ] );
      }
      q8tk_combo_popdown_strings( combo, list );
      q8_list_free( list );
  
      k = get_joystick_assign_initval( i );
      for( j=0; j<countof(joystick_assign_data); j++ ){
	if( k == joystick_assign_data[ j ].joy88_key ) break;
      }
      if( j<countof(joystick_assign_data) ) ;
      else j=0;

      q8tk_combo_set_text( combo, s2[ joystick_assign_data[j].str_num ] );
      q8tk_misc_set_size( combo, 7, 0 );

      q8tk_signal_connect( combo, "changed", cb_joystick_assign, (void*)i );

      q8tk_widget_show( combo );
      q8tk_box_pack_start( hbox, combo );
    }

  }
  q8tk_widget_show( hbox );
  q8tk_box_pack_start( vbox, hbox );


  q8tk_widget_show( vbox );
  return vbox;
}





/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_mouse( void )
{
  Q8tkWidget *vbox, *frame, *widget;
  const char *(*s) = str_mouse[ menu_lang ];

  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_MOUSE_MODE ] );
    {
      widget = menu_mouse_mode();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

#ifdef	USE_JOY
    frame = q8tk_frame_new( s[ STR_MOUSE_JOYSTICK ] );
    {
      widget = menu_mouse_joystick();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
#endif

  }
  q8tk_widget_show( vbox );

  return vbox;
}








/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*					+--------+
 *	メイン画面 -- テープ		| テープ |
 *					|        +------------
 *
 */

/*
 *  ロードイメージ
 */
static	char	tape_load_filename[ Q8TK_MAX_FILENAME ] = { '\0', };
static	Q8tkWidget	*tape_load_label;
static	char		 tape_load_label_name[ 62 ];
static	Q8tkWidget	*tape_load_ratio;
static	char		 tape_load_ratio_str[ 8 ];

static	void	set_load_label_name( void )
{
  size_t i;
  const char	*(*s) = str_tape_load[ menu_lang ];
  const char	*src;

  if( tape_load_filename[0]=='\0' ) src = s[ STR_TAPE_LOAD_NOFILE ];
  else                              src = tape_load_filename;

  strncpy( tape_load_label_name, src, sizeof(tape_load_label_name) );

  for( i=strlen(src); i < sizeof(tape_load_label_name); i++ ){
    tape_load_label_name[ i ] = ' ';
  }
  tape_load_label_name[ sizeof(tape_load_label_name)-1 ] = '\0';
}

static	void	set_load_ratio_str( void )
{
  char buf[8] = "      %";
  int percent;
  long cur, end;

  if( sio_tape_pos( &cur, &end ) ){
    if( end == 0 ){
      buf[3] = 'E';
      buf[4] = 'N';
      buf[5] = 'D';
      buf[6] = ' ';
    }else{
      percent = cur * 100 / end;
      buf[3] = (percent>=100) ? (((percent / 100) % 10) + '0') : ' ';
      buf[4] = (percent>= 10) ? (((percent % 100) / 10) + '0') : ' ';
      buf[5] =                    (percent %  10)       + '0';
      buf[6] = '%';
    }
  }else{
      buf[3] = '-';
      buf[4] = '-';
      buf[5] = '-';
      buf[6] = '%';
  }

  strncpy( tape_load_ratio_str, buf, sizeof(tape_load_ratio_str) );
}


/*	取り出し処理 (「EJECT」クリック時 )				  */
static	void	cb_tape_load_eject_do( void )
{
  sio_close_tapeload();

  tape_load_filename[0] = '\0';	    /* ファイル名を消す */
  set_load_label_name();
  q8tk_label_set( tape_load_label, tape_load_label_name );

  set_load_ratio_str();
  q8tk_label_set( tape_load_ratio, tape_load_ratio_str );
}

/*	巻き戻し処理 (「REW」クリック時 )				  */
static	void	cb_tape_load_rew_do( void )
{
  if( sio_tape_rewind() ){					/* 成功 */
    ;
  }else{							/* 失敗 */
    tape_load_filename[0] = '\0';	    /* ファイル名を消す */
    set_load_label_name();
    q8tk_label_set( tape_load_label, tape_load_label_name );
  }
  set_load_ratio_str();
  q8tk_label_set( tape_load_ratio, tape_load_ratio_str );
}

/*	ファイル選択処理。ファイルセレクションを使用	*/
static void cb_tape_load_change( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  strncpy( tape_load_filename, q8tk_file_selection_get_filename(fsel),
	   Q8TK_MAX_FILENAME-1 );
  tape_load_filename[Q8TK_MAX_FILENAME-1] = '\0';

  /* テープを開く */
  if( sio_open_tapeload() ){					/* 成功 */
    ;
  }else{							/* 失敗 */
    tape_load_filename[0] = '\0';
  }
  set_load_label_name();
  q8tk_label_set( tape_load_label, tape_load_label_name );
 
  set_load_ratio_str();
  q8tk_label_set( tape_load_ratio, tape_load_ratio_str );

  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static void cb_tape_load_cancel( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static	void	cb_tape_load_fsel( Q8tkWidget *dummy_0, void *dummy_1 )
{
  Q8tkWidget *fselect;
  const char	*(*s) = str_tape_load[ menu_lang ];

  fselect = q8tk_file_selection_new( s[ STR_TAPE_LOAD_FSEL ] );

  q8tk_widget_show( fselect );
  q8tk_grab_add( fselect );

  /* テープのイメージファイル選択時の、ディレクトリは？ */
  /* tape_load_filename のディレクトリにしよう。
     つまり、エントリーに入力されたディレクトリである。
     それがセットされてない場合は、dir_tape のディレクトリ。
     それもなければカレントになるはず */
  if( tape_load_filename[0] != '\0' || dir_tape == NULL ){
    q8tk_file_selection_set_filename( fselect, tape_load_filename );
  }else{
    q8tk_file_selection_set_filename( fselect, dir_tape );
  }

  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->ok_button,
		      "clicked", cb_tape_load_change, fselect );
  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->cancel_button,
		      "clicked", cb_tape_load_cancel, fselect );
  q8tk_widget_grab_default( Q8TK_FILE_SELECTION( fselect )->cancel_button);


  misc_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( misc_accel, fselect );
  q8tk_accel_group_add( misc_accel, Q8TK_KEY_ESC,
		        Q8TK_FILE_SELECTION( fselect )->cancel_button,
		        "clicked" );
}
static	Q8tkWidget	*menu_tape_load( void )
{
  Q8tkWidget	*vbox, *hbox, *label, *button, *hsep, *vsep;
  const char	*(*s) = str_tape_load[ menu_lang ];

  /* 変数 file_tapeload を置き換え */
  if( tape_load_filename[0] == '\0' ){		/* 初めてメニューに入った時 */

    strncpy( tape_load_filename, file_tapeload ? file_tapeload : "",
	     Q8TK_MAX_FILENAME-1 );
    tape_load_filename[ Q8TK_MAX_FILENAME-1 ] = '\0';

    file_tapeload = tape_load_filename;
  }

  vbox = q8tk_vbox_new();
  {
    hbox = q8tk_hbox_new();
    {
      label = q8tk_label_new( s[STR_TAPE_LOAD_FILENAME] );
      q8tk_misc_set_placement( label, Q8TK_PLACEMENT_X_CENTER,
				      Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );

      set_load_label_name();
      tape_load_label = q8tk_label_new( tape_load_label_name );
      q8tk_misc_set_placement( tape_load_label, Q8TK_PLACEMENT_X_CENTER,
						Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( tape_load_label );
      q8tk_box_pack_start( hbox, tape_load_label );

    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );
/*
    hsep = q8tk_hseparator_new();
    q8tk_widget_show( hsep );
    q8tk_box_pack_start( vbox, hsep );
*/
    hbox = q8tk_hbox_new();
    {
      button = q8tk_button_new_with_label( s[STR_TAPE_LOAD_CHANGE] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox, button );
      q8tk_signal_connect( button, "clicked", cb_tape_load_fsel, NULL );

      vsep = q8tk_vseparator_new();
      q8tk_widget_show( vsep );
      q8tk_box_pack_start( hbox, vsep );

      button = q8tk_button_new_with_label( s[STR_TAPE_LOAD_EJECT] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox, button );
      q8tk_signal_connect( button, "clicked", cb_tape_load_eject_do, NULL );

      button = q8tk_button_new_with_label( s[STR_TAPE_LOAD_REW] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox, button );
      q8tk_signal_connect( button, "clicked", cb_tape_load_rew_do, NULL );

      set_load_ratio_str();
      tape_load_ratio = q8tk_label_new( tape_load_ratio_str );
      q8tk_misc_set_placement( tape_load_ratio, Q8TK_PLACEMENT_X_CENTER,
						Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( tape_load_ratio );
      q8tk_box_pack_start( hbox, tape_load_ratio );
    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );
  }

  q8tk_widget_show( vbox );
  return vbox;
}


/*
 *  セーブイメージ
 */
static	char	tape_save_filename[ Q8TK_MAX_FILENAME ] = { '\0', };
static	Q8tkWidget	*tape_save_label;
static	char		 tape_save_label_name[ 62 ];

static	void	set_save_label_name( void )
{
  size_t i;
  const char	*(*s) = str_tape_save[ menu_lang ];
  const char	*src;

  if( tape_save_filename[0]=='\0' ) src = s[ STR_TAPE_SAVE_NOFILE ];
  else                              src = tape_save_filename;

  strncpy( tape_save_label_name, src, sizeof(tape_save_label_name) );

  for( i=strlen(src); i < sizeof(tape_save_label_name); i++ ){
    tape_save_label_name[ i ] = ' ';
  }
  tape_save_label_name[ sizeof(tape_save_label_name)-1 ] = '\0';
}

/*	取り出し処理 (「EJECT」クリック時 )				  */
static	void	cb_tape_save_eject_do( void )
{
  sio_close_tapesave();

  tape_save_filename[0] = '\0';	    /* ファイル名を消す */
  set_save_label_name();
  q8tk_label_set( tape_save_label, tape_save_label_name );
}

/*	ファイル選択処理。ファイルセレクションを使用	*/
static void cb_tape_save_change( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  strncpy( tape_save_filename, q8tk_file_selection_get_filename(fsel),
	   Q8TK_MAX_FILENAME-1 );
  tape_save_filename[Q8TK_MAX_FILENAME-1] = '\0';

  /* テープを開く */
  if( sio_open_tapesave() ){					/* 成功 */
    ;
  }else{							/* 失敗 */
    tape_save_filename[0] = '\0';
  }
  set_save_label_name();
  q8tk_label_set( tape_save_label, tape_save_label_name );
 
  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static void cb_tape_save_cancel( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static	void	cb_tape_save_fsel( Q8tkWidget *dummy_0, void *dummy_1 )
{
  Q8tkWidget *fselect;
  const char	*(*s) = str_tape_save[ menu_lang ];

  fselect = q8tk_file_selection_new( s[ STR_TAPE_SAVE_FSEL ] );

  q8tk_widget_show( fselect );
  q8tk_grab_add( fselect );

  /* テープのイメージファイル選択時の、ディレクトリは？ */
  /* tape_save_filename のディレクトリにしよう。
     つまり、エントリーに入力されたディレクトリである。
     それがセットされてない場合は、dir_tape のディレクトリ。
     それもなければカレントになるはず */
  if( tape_save_filename[0] != '\0' || dir_tape == NULL ){
    q8tk_file_selection_set_filename( fselect, tape_save_filename );
  }else{
    q8tk_file_selection_set_filename( fselect, dir_tape );
  }

  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->ok_button,
		      "clicked", cb_tape_save_change, fselect );
  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->cancel_button,
		      "clicked", cb_tape_save_cancel, fselect );
  q8tk_widget_grab_default( Q8TK_FILE_SELECTION( fselect )->cancel_button);


  misc_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( misc_accel, fselect );
  q8tk_accel_group_add( misc_accel, Q8TK_KEY_ESC,
		        Q8TK_FILE_SELECTION( fselect )->cancel_button,
		        "clicked" );
}
static	Q8tkWidget	*menu_tape_save( void )
{
  Q8tkWidget	*vbox, *hbox, *label, *button, *hsep, *vsep;
  const char	*(*s) = str_tape_save[ menu_lang ];

  /* 変数 file_tapesave を置き換え */
  if( tape_save_filename[0] == '\0' ){		/* 初めてメニューに入った時 */

    strncpy( tape_save_filename, file_tapesave ? file_tapesave : "",
	     Q8TK_MAX_FILENAME-1 );
    tape_save_filename[ Q8TK_MAX_FILENAME-1 ] = '\0';

    file_tapesave = tape_save_filename;
  }

  vbox = q8tk_vbox_new();
  {
    hbox = q8tk_hbox_new();
    {
      label = q8tk_label_new( s[STR_TAPE_SAVE_FILENAME] );
      q8tk_misc_set_placement( label, Q8TK_PLACEMENT_X_CENTER,
				      Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox, label );

      set_save_label_name();
      tape_save_label = q8tk_label_new( tape_save_label_name );
      q8tk_misc_set_placement( tape_save_label, Q8TK_PLACEMENT_X_CENTER,
						Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( tape_save_label );
      q8tk_box_pack_start( hbox, tape_save_label );

    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );
/*
    hsep = q8tk_hseparator_new();
    q8tk_widget_show( hsep );
    q8tk_box_pack_start( vbox, hsep );
*/
    hbox = q8tk_hbox_new();
    {
      button = q8tk_button_new_with_label( s[STR_TAPE_SAVE_CHANGE] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox, button );
      q8tk_signal_connect( button, "clicked", cb_tape_save_fsel, NULL );

      vsep = q8tk_vseparator_new();
      q8tk_widget_show( vsep );
      q8tk_box_pack_start( hbox, vsep );

      button = q8tk_button_new_with_label( s[STR_TAPE_SAVE_EJECT] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox, button );
      q8tk_signal_connect( button, "clicked", cb_tape_save_eject_do, NULL );
    }
    q8tk_widget_show( hbox );
    q8tk_box_pack_start( vbox, hbox );
  }

  q8tk_widget_show( vbox );
  return vbox;
}


/*
 *  テープ処理モード切替え(ラジオボタン) 処理
 */

static	struct{
  int		str_num;
  int		cmt_intr;
} tape_intr_data[] =
{
  { STR_TAPE_INTR_YES, TRUE , },
  { STR_TAPE_INTR_NO,  FALSE, },
};

static	void	cb_tape_intr( Q8tkWidget *dummy, void *p )
{
  if( (int)p == TRUE ) cmt_intr = TRUE;
  else                 cmt_intr = FALSE;
}

static	int	get_tape_intr_initval( void )
{
  if( cmt_intr ) return TRUE;
  else           return FALSE;
}

static	Q8tkWidget	*menu_tape_intr( void )
{
  int	i;
  Q8tkWidget	*vbox, *button;
  Q8List	*list = NULL;
  const char	*(*s) = str_tape_intr[ menu_lang ];

  vbox = q8tk_vbox_new();

  for( i=0; i<countof(tape_intr_data); i++ ){

    button = q8tk_radio_button_new_with_label( list,
					       s[tape_intr_data[i].str_num] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( vbox, button );
    q8tk_signal_connect( button, "clicked",
			 cb_tape_intr, (void *)tape_intr_data[i].cmt_intr );
    if( get_tape_intr_initval()==tape_intr_data[i].cmt_intr ){
      q8tk_toggle_button_set_state( button, TRUE );
    }
    list = q8tk_radio_button_group( button );

  }
  q8tk_widget_show( vbox );

  return vbox;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_tape( void )
{
  Q8tkWidget *vbox, *frame, *widget, *vbox2, *hsep;
  const char *(*s) = str_tape[ menu_lang ];

  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_TAPE_IMAGE ] );
    {
      vbox2 = q8tk_vbox_new();
      {
	widget = menu_tape_load();
	q8tk_box_pack_start( vbox2, widget );
      } 
      {
	hsep = q8tk_hseparator_new();
	q8tk_widget_show( hsep );
	q8tk_box_pack_start( vbox2, hsep );
      }
      {
	widget = menu_tape_save();
	q8tk_box_pack_start( vbox2, widget );
      }
      q8tk_widget_show( vbox2 );
    }
    q8tk_container_add( frame, vbox2 );
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );

    frame = q8tk_frame_new( s[ STR_TAPE_INTR ] );
    {
      widget = menu_tape_intr();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
  }
  q8tk_widget_show( vbox );

  return vbox;
}












/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*					+--------+
 *	メイン画面 -- その他		| その他 |
 *					|        +------------
 *
 */

/*
 *  サスペンド
 */
static	char	misc_suspend_filename[ Q8TK_MAX_FILENAME ] = { '\0', };
static	Q8tkWidget	*misc_suspend_entry;

/*	サスペンド時のメッセージダイアログを消す			  */
static	void	cb_misc_suspend_dialog_ok( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dialog_destroy();
}

/*	サスペンド成功後に、終了する					  */
static	void	cb_misc_suspend_dialog_quit( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dialog_destroy();

  emu_mode = QUIT;
  q8tk_main_quit();
}

/*	サスペンド時のメッセージダイアログ				  */
static	void	cb_misc_suspend_dialog( int result )
{
  const char *(*s) = str_misc_suspend_err[ menu_lang ];
  char filename[72];
  int drv;

  dialog_create();
  {
    dialog_set_title( s[ result ] );
    if( result==STR_MISC_SUSPEND_ERR_OK ){
      dialog_set_title( s[ STR_MISC_SUSPEND_ERR_LINE ] );
      dialog_set_title( s[ STR_MISC_SUSPEND_ERR_INFO ] );
      for( drv=0; drv<NR_DRIVE; drv++ ){
	sprintf( filename, "[DRIVE %d] ", drv+1 );
	if( disk_image_exist( drv ) ){
	  strncat( filename, drive[drv].filename, 
		   sizeof(filename)-strlen(filename)-1 );
	}else{
	  strcat( filename, s[ STR_MISC_SUSPEND_ERR_NOTHING ] );
	}
	dialog_set_title( filename );
      }
    }
    dialog_set_separator();

    if( result==STR_MISC_SUSPEND_ERR_OK ){
      dialog_set_button( s[ STR_MISC_SUSPEND_ERR_QUIT ],
			 cb_misc_suspend_dialog_quit, NULL );
      dialog_set_button( s[ STR_MISC_SUSPEND_ERR_CONTINUE ],
			 cb_misc_suspend_dialog_ok, NULL );
    }else{
      dialog_set_button( s[ STR_MISC_SUSPEND_ERR_DISMIS ],
			 cb_misc_suspend_dialog_ok, NULL );
    }
  }
  dialog_start();
}

/*	サスペンド処理 (「実行」クリック時 )				  */
static	void	cb_misc_suspend_do( void )
{
  if( suspend( misc_suspend_filename ) ){
    cb_misc_suspend_dialog( STR_MISC_SUSPEND_ERR_OK );		/* 成功 */
  }else{
    cb_misc_suspend_dialog( STR_MISC_SUSPEND_ERR_NG );		/* 失敗 */
  }

  /*file_resume = misc_suspend_filename;*/
}

/*	ファイル名前変更。エントリー changed (入力)時に呼ばれる。  */
/*		(ファイルセレクションでの変更時はこれは呼ばれない) */
static void cb_misc_suspend_entry_change( Q8tkWidget *widget, void *dummy )
{
  strncpy( misc_suspend_filename, q8tk_entry_get_text( widget ), 
	   Q8TK_MAX_FILENAME-1 );
  misc_suspend_filename[Q8TK_MAX_FILENAME-1] = '\0';
}

/*	ファイル選択処理。ファイルセレクションを使用	*/
static void cb_misc_suspend_change( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  strncpy( misc_suspend_filename, q8tk_file_selection_get_filename(fsel),
	   Q8TK_MAX_FILENAME-1 );
  misc_suspend_filename[Q8TK_MAX_FILENAME-1] = '\0';
  q8tk_entry_set_text( misc_suspend_entry, misc_suspend_filename );

  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static void cb_misc_suspend_cancel( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static	void	cb_misc_suspend_fsel( Q8tkWidget *dummy_0, void *dummy_1 )
{
  Q8tkWidget *fselect;
  const char	*(*s) = str_misc_suspend[ menu_lang ];

  fselect = q8tk_file_selection_new( s[ STR_MISC_SUSPEND_FSEL ] );

  q8tk_widget_show( fselect );
  q8tk_grab_add( fselect );

  /* サスペンドのイメージファイル選択時の、ディレクトリは？ */
  /* misc_suspend_filename のディレクトリにしよう。
     つまり、エントリーに入力されたディレクトリである。
     初期値は、${HOME} になっているはず */
  q8tk_file_selection_set_filename( fselect, misc_suspend_filename );

  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->ok_button,
		      "clicked", cb_misc_suspend_change, fselect );
  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->cancel_button,
		      "clicked", cb_misc_suspend_cancel, fselect );
  q8tk_widget_grab_default( Q8TK_FILE_SELECTION( fselect )->cancel_button);


  misc_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( misc_accel, fselect );
  q8tk_accel_group_add( misc_accel, Q8TK_KEY_ESC,
		        Q8TK_FILE_SELECTION( fselect )->cancel_button,
		        "clicked" );
}
static	Q8tkWidget	*menu_misc_suspend( void )
{
  Q8tkWidget	*hbox, *label, *button, *vsep;
  const char	*(*s) = str_misc_suspend[ menu_lang ];

  /* 変数 file_resume を置き換え */
  if( misc_suspend_filename[0] == '\0' ){	/* 初めてメニューに入った時 */

    strncpy( misc_suspend_filename, file_resume ? file_resume : "",
	     Q8TK_MAX_FILENAME-1 );
    misc_suspend_filename[ Q8TK_MAX_FILENAME-1 ] = '\0';

    file_resume = misc_suspend_filename;
  }

  hbox = q8tk_hbox_new();
  {
    button = q8tk_button_new_with_label( s[STR_MISC_SUSPEND_BUTTON] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( hbox, button );

    q8tk_signal_connect( button, "clicked", cb_misc_suspend_do, NULL );

    vsep = q8tk_vseparator_new();
    q8tk_widget_show( vsep );
    q8tk_box_pack_start( hbox, vsep );

    label = q8tk_label_new( s[STR_MISC_SUSPEND_FILENAME] );
    q8tk_misc_set_placement( label, Q8TK_PLACEMENT_X_CENTER,
			            Q8TK_PLACEMENT_Y_CENTER);
    q8tk_widget_show( label );
    q8tk_box_pack_start( hbox, label );

    misc_suspend_entry = q8tk_entry_new();
    q8tk_entry_set_text( misc_suspend_entry, misc_suspend_filename );
    q8tk_entry_set_position( misc_suspend_entry, 0 );
    q8tk_misc_set_size( misc_suspend_entry, 41, 1 );
    q8tk_misc_set_placement( misc_suspend_entry,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
    q8tk_widget_show( misc_suspend_entry );
    q8tk_box_pack_start( hbox, misc_suspend_entry );
    q8tk_signal_connect( misc_suspend_entry, "changed",
			 cb_misc_suspend_entry_change, NULL );

    button = q8tk_button_new_with_label( s[STR_MISC_SUSPEND_CHANGE] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( hbox, button );

    q8tk_signal_connect( button, "clicked", cb_misc_suspend_fsel, NULL );

  }

  q8tk_widget_show( hbox );
  return hbox;
}


/*
 *  スクリーン スナップショット
 */
static	char	misc_snapshot_filename[ Q8TK_MAX_FILENAME ] = { '\0', };
static	char	misc_snapshot_command[ Q8TK_MAX_FILENAME ]  = { '\0', };
static	Q8tkWidget	*misc_snapshot_f_entry;
static	Q8tkWidget	*misc_snapshot_c_entry;


/*	スナップショット セーブ (「実行」クリック時 )		  */
static	void	cb_misc_snapshot_do( void )
{
  save_screen_snapshot( );
}

/*	画像フォーマット切り替え (ラジオボタン) 処理 */
static	struct{
  int		str_num;
  int		type;
} misc_snapshot_format_data[] =
{
  { STR_MISC_SNAPSHOT_FORMAT_RAW, 0,   },
  { STR_MISC_SNAPSHOT_FORMAT_ASC, 1,   },
  { STR_MISC_SNAPSHOT_FORMAT_XPM, 2,   },
};

static	void	cb_misc_snapshot_format( Q8tkWidget *dummy, void *p )
{
  snapshot_format = (int)p;
}

static	int	get_misc_snapshot_format_initval( void )
{
  return snapshot_format;
}

/*	ファイル名前変更。エントリー changed (入力)時に呼ばれる。  */
/*		(ファイルセレクションでの変更時はこれは呼ばれない) */
static void cb_misc_snapshot_f_entry_change( Q8tkWidget *widget, void *dummy )
{
  strncpy( misc_snapshot_filename, q8tk_entry_get_text( widget ), 
	   Q8TK_MAX_FILENAME-1 );
  misc_snapshot_filename[Q8TK_MAX_FILENAME-1] = '\0';
}

/*	ファイル選択処理。ファイルセレクションを使用	*/
static void cb_misc_snapshot_change( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  strncpy( misc_snapshot_filename, q8tk_file_selection_get_filename(fsel),
	   Q8TK_MAX_FILENAME-1 );
  misc_snapshot_filename[ Q8TK_MAX_FILENAME-1 ] = '\0';

  q8tk_entry_set_text( misc_snapshot_f_entry, misc_snapshot_filename );

  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static void cb_misc_snapshot_cancel( Q8tkWidget *dummy, Q8tkWidget *fsel )
{
  q8tk_grab_remove( fsel );
  q8tk_widget_destroy( fsel );
  q8tk_widget_destroy( misc_accel );
}
static	void	cb_misc_snapshot_fsel( Q8tkWidget *dummy_0, void *dummy_1 )
{
  Q8tkWidget *fselect;
  const char	*(*s) = str_misc_snapshot[ menu_lang ];

  fselect = q8tk_file_selection_new( s[ STR_MISC_SNAPSHOT_FSEL ] );

  q8tk_widget_show( fselect );
  q8tk_grab_add( fselect );

  q8tk_file_selection_set_filename( fselect, misc_snapshot_filename );

  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->ok_button,
		      "clicked", cb_misc_snapshot_change, fselect );
  q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->cancel_button,
		      "clicked", cb_misc_snapshot_cancel, fselect );
  q8tk_widget_grab_default( Q8TK_FILE_SELECTION( fselect )->cancel_button);


  misc_accel = q8k_accel_group_new();
  q8tk_accel_group_attach( misc_accel, fselect );
  q8tk_accel_group_add( misc_accel, Q8TK_KEY_ESC,
		        Q8TK_FILE_SELECTION( fselect )->cancel_button,
		        "clicked" );
}

/*	コマンド実行状態変更 */
static	int	get_misc_snapshot_c_do_initval( void )
{
  return snapshot_cmd_do;
}
static	void	cb_misc_snapshot_c_do( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
  snapshot_cmd_do = key;
}

/*	コマンド変更。エントリー changed (入力)時に呼ばれる。  */
static void cb_misc_snapshot_c_entry_change( Q8tkWidget *widget, void *dummy )
{
  strncpy( misc_snapshot_command, q8tk_entry_get_text( widget ), 
	   Q8TK_MAX_FILENAME-1 );
  misc_snapshot_command[Q8TK_MAX_FILENAME-1] = '\0';
}



static	Q8tkWidget	*menu_misc_snapshot( void )
{
  int	i;
  Q8tkWidget	*hbox, *vbox, *hbox2, *label, *button, *vsep, *hbox3;
  const char	*(*s) = str_misc_snapshot[ menu_lang ];
  const char	*(*ss) = str_misc_snapshot_format[ menu_lang ];
  Q8List	*list = NULL;

  /* 変数 file_snapshot を置き換え */
  if( misc_snapshot_filename[0] == '\0' ){	/* 初めてメニューに入った時 */

    strncpy( misc_snapshot_filename, file_snapshot ? file_snapshot : "",
	     Q8TK_MAX_FILENAME-1 );
    misc_snapshot_filename[Q8TK_MAX_FILENAME-1] = '\0';

    file_snapshot = misc_snapshot_filename;
  }

  /* 変数 snapshot_cmd を置き換え */
  if( misc_snapshot_command[0] == '\0' ){	/* 初めてメニューに入った時 */

    strncpy( misc_snapshot_command, snapshot_cmd ? snapshot_cmd : "",
	     Q8TK_MAX_FILENAME-1 );
    misc_snapshot_command[ Q8TK_MAX_FILENAME-1 ] = '\0';

    snapshot_cmd = misc_snapshot_command;
  }



  hbox = q8tk_hbox_new();
  {
    button = q8tk_button_new_with_label( s[STR_MISC_SNAPSHOT_BUTTON] );
    q8tk_widget_show( button );
    q8tk_box_pack_start( hbox, button );

    q8tk_signal_connect( button, "clicked", cb_misc_snapshot_do, NULL );


    vsep = q8tk_vseparator_new();
    q8tk_widget_show( vsep );
    q8tk_box_pack_start( hbox, vsep );


    vbox = q8tk_vbox_new();
    {
      hbox2 = q8tk_hbox_new();

      label = q8tk_label_new( s[STR_MISC_SNAPSHOT_FILENAME] );
      q8tk_misc_set_placement( label, Q8TK_PLACEMENT_X_CENTER,
			       Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox2, label );

      misc_snapshot_f_entry = q8tk_entry_new();
      q8tk_entry_set_text( misc_snapshot_f_entry, misc_snapshot_filename );
      q8tk_entry_set_position( misc_snapshot_f_entry, 0 );
      q8tk_misc_set_size( misc_snapshot_f_entry, 41, 1 );
      q8tk_misc_set_placement( misc_snapshot_f_entry,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( misc_snapshot_f_entry );
      q8tk_box_pack_start( hbox2, misc_snapshot_f_entry );
      q8tk_signal_connect( misc_snapshot_f_entry, "changed",
			   cb_misc_snapshot_f_entry_change, NULL );

      button = q8tk_button_new_with_label( s[STR_MISC_SNAPSHOT_CHANGE] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox2, button );

      q8tk_signal_connect( button, "clicked", cb_misc_snapshot_fsel, NULL );


      q8tk_widget_show( hbox2 );
      q8tk_box_pack_start( vbox, hbox2 );
    }
    {
      hbox2 = q8tk_hbox_new();

      label = q8tk_label_new( s[STR_MISC_SNAPSHOT_FORMAT] );
      q8tk_widget_show( label );
      q8tk_box_pack_start( hbox2, label );

      {
	hbox3 = q8tk_hbox_new();

	for( i=0; i<countof(misc_snapshot_format_data); i++ ){

	  button = q8tk_radio_button_new_with_label( list,
				    ss[misc_snapshot_format_data[i].str_num] );
	  q8tk_widget_show( button );
	  q8tk_box_pack_start( hbox3, button );
	  q8tk_signal_connect( button, "clicked",
			 cb_misc_snapshot_format, 
			 (void *)misc_snapshot_format_data[i].type );
	  if( get_misc_snapshot_format_initval() == 
	  				   misc_snapshot_format_data[i].type ){
	    q8tk_toggle_button_set_state( button, TRUE );
	  }
	  list = q8tk_radio_button_group( button );

	}
	q8tk_widget_show( hbox3 );
	q8tk_box_pack_start( hbox2, hbox3 );
      }

      q8tk_widget_show( hbox2 );
      q8tk_box_pack_start( vbox, hbox2 );
    }

#ifdef	USE_SSS_CMD
    {
      label = q8tk_label_new( "" );
      q8tk_widget_show( label );
      q8tk_box_pack_start( vbox, label );
    }
    {
      hbox2 = q8tk_hbox_new();

      button = q8tk_check_button_new_with_label( s[STR_MISC_SNAPSHOT_CMD] );
      q8tk_widget_show( button );
      q8tk_box_pack_start( hbox2, button );

      if( get_misc_snapshot_c_do_initval() ){
	q8tk_toggle_button_set_state( button, TRUE );
      }
      q8tk_signal_connect( button, "toggled", cb_misc_snapshot_c_do, 0 );

      misc_snapshot_c_entry = q8tk_entry_new();
      q8tk_entry_set_text( misc_snapshot_c_entry, misc_snapshot_command );
      q8tk_entry_set_position( misc_snapshot_c_entry, 0 );
      q8tk_misc_set_size( misc_snapshot_c_entry, 41, 1 );
      q8tk_misc_set_placement( misc_snapshot_c_entry,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
      q8tk_widget_show( misc_snapshot_c_entry );
      q8tk_box_pack_start( hbox2, misc_snapshot_c_entry );
      q8tk_signal_connect( misc_snapshot_c_entry, "changed",
			   cb_misc_snapshot_c_entry_change, NULL );

      q8tk_widget_show( hbox2 );
      q8tk_box_pack_start( vbox, hbox2 );
    }
#endif	/* USE_SSS_CMD */

  }
  q8tk_widget_show( vbox );
  q8tk_box_pack_start( hbox, vbox );


  q8tk_widget_show( hbox );

  return hbox;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_misc( void )
{
  Q8tkWidget *vbox, *frame, *widget;
  const char *(*s) = str_misc[ menu_lang ];

  vbox = q8tk_vbox_new();
  {
    frame = q8tk_frame_new( s[ STR_MISC_SUSPEND ] );
    {
      widget = menu_misc_suspend();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
    frame = q8tk_frame_new( s[ STR_MISC_SNAPSHOT ] );
    {
      widget = menu_misc_snapshot();
      q8tk_container_add( frame, widget );
    }
    q8tk_widget_show( frame );
    q8tk_box_pack_start( vbox, frame );
  }
  q8tk_widget_show( vbox );

  return vbox;
}












/*---------------------------------------------------------------------------*/

/*					+-+
 *	メイン画面 -- バージョン情報	|*|
 *					| +------------
 *
 */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static	Q8tkWidget	*menu_info( void )
{
  int i;
  Q8tkWidget *vbox, *swin, *label;
  const char *(*s) = (menu_lang==0) ? menu_ver_info_en : menu_ver_info_jp;

  swin  = q8tk_scrolled_window_new( NULL, NULL );
  {
    vbox = q8tk_vbox_new();
    {
      for( i=0; s[i]; i++ ){
	label = q8tk_label_new( s[i] );
	q8tk_widget_show( label );
	q8tk_box_pack_start( vbox, label );
      }
    }
    q8tk_container_add( swin, vbox );
    q8tk_widget_show( vbox );
  }

  q8tk_scrolled_window_set_policy( swin, Q8TK_POLICY_AUTOMATIC,
				         Q8TK_POLICY_AUTOMATIC );
  q8tk_misc_set_size( swin, 78, 18 );
  q8tk_widget_show( swin );

  return swin;
}










/*--------------------------------------------------------------*/
/* メニューモードの起点のディレクトリ				*/
/*--------------------------------------------------------------*/
static	void	set_menu_dirname( void )
{
  int	drv;

  for( drv=DRIVE_1; drv<NR_DRIVE; drv ++ ){	/* Drv. 1→2 の順にファイル */
						/* の有無をチェック	    */
    if( disk_image_exist( drv ) ){

      osd_path_separate( menu_pathname, drive[drv].filename,
			 Q8TK_MAX_FILENAME );

      /*
	通常は、ファイル名に '/' が含まれるので、パス分離に成功
	するはず → menu_pathname に文字列がセットされる。

	ファイル名に '/' が含まれない場合は、パス分離できない。
	→ menu_pathname[0] = '\0'、つまり空文字列 "" になる
      */

      break;
    }

  }


  if( drv==NR_DRIVE ){			/* ディスクはセットされていない */

    if( menu_pathname[0] == '\0' ){		/* 未定義時のみセット */
						/* (定義済みなら継続) */
      if( osd_get_disk_dir() )
	strncpy( menu_pathname, osd_get_disk_dir(), Q8TK_MAX_FILENAME-1 );
      else
	strcpy( menu_pathname, "" );

      menu_pathname[ Q8TK_MAX_FILENAME-1 ] = '\0';
    }

  }
}



/****************************************************************/
/* メニューモード メイン処理					*/
/****************************************************************/

void	menu_init( void )
{
  int	i;

  /*screen_buf_init();*/	/* screen_buf[] を黒 (0) でクリア */

  menu_event_init();		/* メニュー用にイベントを初期化 */

  set_mouse_visible();
  fflush(NULL);

  for( i=0; i<0x10; i++ ){			/* キースキャンワーク初期化 */
    if     ( i==0x08 ) key_scan[i] |= 0xdf;		/* カナは残す */
    else if( i==0x0a ) key_scan[i] |= 0x7f;		/* CAPSも残す */
    else               key_scan[i]  = 0xff;
  }
  romaji_clear();				/* ローマ字入力バッファ消す */
  indicator_flag = 0;				/* 各種表示は消す	    */
  clear_indicator();

  xmame_sound_suspend();


  set_menu_dirname();

  menu_boot_dipsw      = boot_dipsw;
  menu_boot_from_rom   = boot_from_rom;
  menu_boot_basic      = boot_basic;
  menu_boot_clock_4mhz = boot_clock_4mhz;
  menu_boot_version    = ROM_VERSION;
  menu_boot_baudrate   = baudrate_sw;

  dialog_init();
}



/*
 *	NOTEBOOK に張り付ける、各ページ
 */
static	struct{
  int		str_num;
  Q8tkWidget	*(*menu_func)(void);
} menu_data[] =
{
  { STR_BASE_RESET,	menu_reset,	},
/*{ STR_BASE_DIPSW,	menu_dipsw,	}, */ /* DIPSW は RESET に移動 */
  { STR_BASE_CPU,	menu_cpu,	},
  { STR_BASE_GRAPH,	menu_graph,	},
  { STR_BASE_VOLUME,	menu_volume,	},
  { STR_BASE_DISK,	menu_disk,	},
  { STR_BASE_KEY,	menu_key,	},
  { STR_BASE_MOUSE,	menu_mouse,	},
  { STR_BASE_TAPE,	menu_tape,	},
  { STR_BASE_MISC,	menu_misc,	},
  { STR_BASE_INFO,	menu_info,	},
};


/*
 *	NOTEBOOK の各ページを、ファンクションキーで選択出来るように、
 *	アクセラレータキーを設定する。そのため、ダミーウィジット利用。
 */
#define	cb_note_fake(fn,n)						     \
static	void	cb_note_fake_##fn( Q8tkWidget *dummy, Q8tkWidget *notebook ){\
  q8tk_notebook_set_page( notebook, n );				     \
}
cb_note_fake(f1,0)
cb_note_fake(f2,1)
cb_note_fake(f3,2)
cb_note_fake(f4,3)
cb_note_fake(f5,4)
cb_note_fake(f6,5)
cb_note_fake(f7,6)
cb_note_fake(f8,7)
cb_note_fake(f9,8)
cb_note_fake(f10,9)

static	struct{
  int	key;
  void	(*cb_func)(Q8tkWidget *, Q8tkWidget *);
} menu_fkey_data[] =
{
  { Q8TK_KEY_F1,  cb_note_fake_f1,  },
  { Q8TK_KEY_F2,  cb_note_fake_f2,  },
  { Q8TK_KEY_F3,  cb_note_fake_f3,  },
  { Q8TK_KEY_F4,  cb_note_fake_f4,  },
  { Q8TK_KEY_F5,  cb_note_fake_f5,  },
  { Q8TK_KEY_F6,  cb_note_fake_f6,  },
  { Q8TK_KEY_F7,  cb_note_fake_f7,  },
  { Q8TK_KEY_F8,  cb_note_fake_f8,  },
  { Q8TK_KEY_F9,  cb_note_fake_f9,  },
  { Q8TK_KEY_F10, cb_note_fake_f10, },
};




void	menu_main( void )
{
  int	i;
  const char *(*s) = str_base[ menu_lang ];
  Q8tkWidget	*note_fake[ countof(menu_fkey_data) ];

  q8tk_init();
  {
    Q8tkWidget *window, *vbox, *notebook, *base_button, *page;

    window = q8tk_window_new( Q8TK_WINDOW_TOPLEVEL );
    menu_accel = q8k_accel_group_new();
    q8tk_accel_group_attach( menu_accel, window );
    {
      vbox = q8tk_vbox_new();
      {
	notebook = q8tk_notebook_new();

	for( i=0; i<countof(menu_data); i++ ){

	  page = (*menu_data[i].menu_func)();
	  q8tk_notebook_append( notebook, page, s[ menu_data[i].str_num ] );

	  if( i<countof(menu_fkey_data) ){
	    note_fake[i] = q8tk_button_new();
	    q8tk_signal_connect( note_fake[i], "clicked",
				 menu_fkey_data[i].cb_func, notebook );
	    q8tk_accel_group_add( menu_accel, menu_fkey_data[i].key,
				  note_fake[i], "clicked" );
	  }

	}

	q8tk_widget_show( notebook );
	q8tk_box_pack_start( vbox, notebook );

	base_button = base_menu();
	q8tk_box_pack_start( vbox, base_button );
      }
      q8tk_widget_show( vbox );
      q8tk_container_add( window, vbox );
    }
    q8tk_widget_show( window );

    q8tk_notebook_set_page( notebook, menu_last_page );

    q8tk_main();

    menu_last_page = q8tk_notebook_current_page( notebook );
  }
  q8tk_term();





  if( emu_mode == MENU_MAIN ||
      emu_mode == MONITOR   ){

    menu_event_term();			/* エミュレータ用にイベントを戻す */

    /*screen_buf_init();*/
    redraw_screen( FALSE );
    if( emu_mode==MENU_MAIN ){
      if( check_break_point_PC() ) emu_mode = EXECUTE_BP;
      else                         emu_mode = EXECUTE;
      if( hide_mouse ) set_mouse_invisible();
      else	       set_mouse_visible();
    }
    if( emu_mode!=MONITOR ) xmame_sound_resume();

    joystick_restart();
  }

}

/****************************************************************/
/* Menu Mode PSP				*/
/****************************************************************/

char	path_main[MAXPATH];
char	*Drive1 = " ";
char	*Drive2 = " ";
char	SetDisk_path[2][MAXPATH];
char	dir_path[2][MAXPATH];
int		CAPS = 0;
int		KANA = 0;
int		SHIFT = 0;
int		CAPS_KEY = 0;
int		KANA_KEY = 0;
int		BS_KEY = 0;
int		DEL_KEY = 0;
int		INS_KEY = 0;
int		F6_KEY = 0;
int		F7_KEY = 0;
int		F8_KEY = 0;
int		F9_KEY = 0;
int		F10_KEY = 0;
int		SHIFT_KEY = 0;
static char tmp_dir[MAXPATH];

_init Menu;
_cfg cfg_data;

int KeySetting[] = {
	KeyCol * 1 + 18,//	Key_UP,
	KeyCol * 3 + 18,//	Key_DOWN,
	KeyCol * 2 + 17,//	Key_LEFT,
	KeyCol * 2 + 19,//	Key_RIGHT,
	KeyCol * 5 +  5,//	Key_CIRCLE,
	KeyCol * 2 + 13,//	Key_CROSS,
	KeyCol * 1 + 14,//	Key_TRIANGLE,
	KeyCol * 3 +  3,//	Key_SQUARE,
	-1             ,//	Key_LTRIGGER,
#ifdef SoftwareKeybord
	-1             ,//	Key_RTRIGGGER,
#else
	KeyCol * 4     ,//	Key_RTRIGGGER,
#endif
				  2,//	Key_START,
	KeyCol * 1  +16,//	Key_SELECT,
};

char ConfigFile[] = "q88p.cfg";
char StateFile[] = "q88p.sta";
char ConfigVer[] = "V2";
void menu_init_psp( void )
{
	char path_cfg[MAXPATH];
	int fd,i;
	strcpy(tmp_dir,path_main);//ファイラー用に(一応)メインパスを拾っておく･･･ディレクトリ移動対応用
//	strcpy(tmp_dir,"ms0:/PSP/");//ms0:/PSPのルートフォルダにしてみたらどうか。
	strcpy(path_cfg,path_main);
	strcat(path_cfg,ConfigFile);
	fd = sceIoOpen(path_cfg,SCE_O_RDONLY,0777);
	if(fd >= 0){
		sceIoRead(fd,&cfg_data,sizeof(cfg_data));
		sceIoClose(fd);
		//cfg Initialize
		if((cfg_data.CfgVer[0] == ConfigVer[0]) && (cfg_data.CfgVer[1] == ConfigVer[1])){
			Menu.PSPClock = cfg_data.PSPClock;
			scePowerSetClockFrequency(Clock_main[Menu.PSPClock],Clock_main[Menu.PSPClock],Clock_sub[Menu.PSPClock]);
			Menu.Clock = cfg_data.Clock;
			switch(Menu.Clock){
			case 0:
				boot_clock_4mhz = TRUE;
				cpu_clock_mhz   = CONST_4MHZ_CLOCK;
				break;
			case 1:
				boot_clock_4mhz = FALSE;
				cpu_clock_mhz   = CONST_8MHZ_CLOCK;
				break;
			}
			Menu.Mode = cfg_data.Mode;
			Menu.Subcpu = cfg_data.Subcpu;
			if(Menu.Subcpu > 2) Menu.Subcpu = 0;
			cpu_timing = Menu.Subcpu;
			Menu.Palette = cfg_data.Palette;
			if(Menu.Palette > 1) Menu.Palette = 0;
			if(Menu.Palette == 0) monitor_analog = TRUE;//Analog
			else monitor_analog = FALSE;//Digital
			Menu.Size = cfg_data.Size;
			if(Menu.Size == 0) screen_size = SCREEN_SIZE_HALF;//HALF
			else screen_size = SCREEN_SIZE_FULL;//FULL
			Menu.Half = cfg_data.Half;
			if(Menu.Half == 1) use_half_interp = TRUE;
			else Menu.Half = 0;


			switch(Menu.Mode){
			case 0:	boot_basic = BASIC_N;		break;
			case 1:	boot_basic = BASIC_V1S;		break;
			case 2:	boot_basic = BASIC_V1H;		break;
			case 3:	boot_basic = BASIC_V2;		break;
			}
			Menu.Image[0] = cfg_data.Image[0];
			Menu.Image[1] = cfg_data.Image[1];

			for(i = 0;i < Control_End;i++){
				KeySetting[i] = cfg_data.KeySetting[i];
				SetKeyConfig(i,KeyInput[KeySetting[i]].Port,KeyInput[KeySetting[i]].Bit);
				if(KeySetting[i] == KeyCol * 3 + 1 ) CAPS_KEY = i + 1;
				if(KeySetting[i] == KeyCol * 5     ) KANA_KEY = i + 1;
				if(KeySetting[i] == KeyCol     + 15) INS_KEY  = i + 1;
				if(KeySetting[i] ==               7) F6_KEY = i + 1;
				if(KeySetting[i] ==               8) F7_KEY = i + 1;
				if(KeySetting[i] ==               9) F8_KEY = i + 1;
				if(KeySetting[i] ==              10) F9_KEY = i + 1;
				if(KeySetting[i] ==              11) F10_KEY = i + 1;
				if(KeySetting[i] == KeyCol * 4     ) SHIFT_KEY = i + 1;
			}
			Drive1 = cfg_data.File[0];
			DiskIn[0] = cfg_data.DiskIn[0];
			if(DiskIn[0]) add_diskimage(Drive1,Menu.Image[0],0);
			Drive2 = cfg_data.File[1];
			DiskIn[1] = cfg_data.DiskIn[1];
			if(DiskIn[1]) add_diskimage(Drive2,Menu.Image[1],1);
		}
	}
}

void cfg_write(void)
{
	char path_cfg[MAXPATH];
	int fd,i;

	strcpy(cfg_data.CfgVer,ConfigVer);
	cfg_data.PSPClock = Menu.PSPClock;
	cfg_data.Clock = Menu.Clock;
	cfg_data.Mode = Menu.Mode;
	cfg_data.Subcpu = Menu.Subcpu;
	cfg_data.Palette = Menu.Palette;
	cfg_data.Size = Menu.Size;
	cfg_data.Half = Menu.Half;
	cfg_data.Image[0] = Menu.Image[0];
	cfg_data.Image[1] = Menu.Image[1];
	for(i = 0;i < Control_End;i++){
		cfg_data.KeySetting[i] = KeySetting[i];
	}
	strcpy(cfg_data.File[0],Drive1);
	strcpy(cfg_data.File[1],Drive2);
	cfg_data.DiskIn[0] = DiskIn[0];
	cfg_data.DiskIn[1] = DiskIn[1];

	strcpy(path_cfg,path_main);
	strcat(path_cfg,ConfigFile);
	fd = sceIoOpen(path_cfg,SCE_O_CREAT|SCE_O_WRONLY|SCE_O_TRUNC,0777);
	if(fd >= 0){
		sceIoWrite(fd,&cfg_data, sizeof(cfg_data));
		sceIoClose(fd);
	}
}

void menu_draw(int Power){
	int i;
	char *DiskName1,*DiskName2;

    for(i = 0;i<Menu_exit + 1;i++){
		pgPrint_drawbg(Menu_x[0],Menu_y[i],0xFFFF,0,"  ");
    }
	pgPrint_drawbg(Menu_x[0],Menu_y[MenuMode],0xFFFF,0,">>");
	pgPrint_drawbg(0,0,0x001F,0,"Quasi88 for PSP Configuration");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_pspclock],0xFFFF,0,"PSP Clock");
	pgPrint_drawbg(Menu_x[2],Menu_y[Menu_pspclock],0xFFFF - (0x07FF * (Menu.PSPClock == 0)),0,"222MHz");
	pgPrint_drawbg(Menu_x[3],Menu_y[Menu_pspclock],0xFFFF - (0x07FF * (Menu.PSPClock == 1)),0,"266MHz");
	pgPrint_drawbg(Menu_x[4],Menu_y[Menu_pspclock],0xFFFF - (0x07FF * (Menu.PSPClock == 2)),0,"333MHz");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_keyconfig],0xFFFF,0,"KeyConfig");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_clock],0xFFFF,0,"Clock");
	pgPrint_drawbg(Menu_x[2],Menu_y[Menu_clock],0xFFFF - (0x07FF * (Menu.Clock == 0)),0,"4MHz");
	pgPrint_drawbg(Menu_x[3],Menu_y[Menu_clock],0xFFFF - (0x07FF * (Menu.Clock == 1)),0,"8MHz");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_mode],0xFFFF,0,"Mode");
	pgPrint_drawbg(Menu_x[2],Menu_y[Menu_mode],0xFFFF - (0x07FF * (Menu.Mode == 0)),0,"N  ");
	pgPrint_drawbg(Menu_x[3],Menu_y[Menu_mode],0xFFFF - (0x07FF * (Menu.Mode == 1)),0,"V1S");
	pgPrint_drawbg(Menu_x[4],Menu_y[Menu_mode],0xFFFF - (0x07FF * (Menu.Mode == 2)),0,"V1H");
	pgPrint_drawbg(Menu_x[5],Menu_y[Menu_mode],0xFFFF - (0x07FF * (Menu.Mode == 3)),0,"V2 ");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_subcpu],0xFFFF,0,"SubCPU");
	pgPrint_drawbg(Menu_x[2],Menu_y[Menu_subcpu],0xFFFF - (0x07FF * (Menu.Subcpu == 0)),0,"0");
	pgPrint_drawbg(Menu_x[3],Menu_y[Menu_subcpu],0xFFFF - (0x07FF * (Menu.Subcpu == 1)),0,"1");
	pgPrint_drawbg(Menu_x[4],Menu_y[Menu_subcpu],0xFFFF - (0x07FF * (Menu.Subcpu == 2)),0,"2");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_palette],0xFFFF,0,"Palette");
	pgPrint_drawbg(Menu_x[2],Menu_y[Menu_palette],0xFFFF - (0x07FF * (Menu.Palette == 0)),0,"Analog");
	pgPrint_drawbg(Menu_x[3],Menu_y[Menu_palette],0xFFFF - (0x07FF * (Menu.Palette == 1)),0,"Digital");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_size],0xFFFF,0,"Screen");
	pgPrint_drawbg(Menu_x[2],Menu_y[Menu_size],0xFFFF - (0x07FF * (Menu.Size == 0)),0,"HALF");
	pgPrint_drawbg(Menu_x[3],Menu_y[Menu_size],0xFFFF - (0x07FF * (Menu.Size == 1)),0,"FULL");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_half_interp],0xFFFF,0,"Half Int.");
	pgPrint_drawbg(Menu_x[2],Menu_y[Menu_half_interp],0xFFFF - (0x07FF * (Menu.Half == 0)),0,"FALSE");
	pgPrint_drawbg(Menu_x[3],Menu_y[Menu_half_interp],0xFFFF - (0x07FF * (Menu.Half == 1)),0,"TRUE");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_drive1],0xFFFF,0,"Drive_1");
	if(DiskIn[0] == 1){
		pgPrint_drawbg(Menu_x[2],Menu_y[Menu_drive1],0xFFFF,0,Drive1);
		pgPrint_drawbg(Menu_x[5],Menu_y[Menu_drive1] + 1,0xFFFF,0,Number[Menu.Image[0]]);
	}else{
		pgPrint_drawbg(Menu_x[2],Menu_y[Menu_drive1],0xFFFF,0,"                                    ");
		pgPrint_drawbg(Menu_x[5],Menu_y[Menu_drive1] + 1,0xFFFF,0,"  ");
	}
	pgPrint_drawbg(Menu_x[4],Menu_y[Menu_drive1] + 1,0xFFFF,0,"Image No.");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_drive2],0xFFFF,0,"Drive_2");
	if(DiskIn[1] == 1){
		pgPrint_drawbg(Menu_x[2],Menu_y[Menu_drive2],0xFFFF,0,Drive2);
		pgPrint_drawbg(Menu_x[5],Menu_y[Menu_drive2] + 1,0xFFFF,0,Number[Menu.Image[1]]);
	}else{
		pgPrint_drawbg(Menu_x[2],Menu_y[Menu_drive2],0xFFFF,0,"                                    ");
		pgPrint_drawbg(Menu_x[5],Menu_y[Menu_drive2] + 1,0xFFFF,0,"  ");
	}
	pgPrint_drawbg(Menu_x[4],Menu_y[Menu_drive2] + 1,0xFFFF,0,"Image No.");
	if(Power == 0){
		pgPrint_drawbg(Menu_x[1],Menu_y[Menu_power],0xFFFF,0,"Power On");
	}else{
		pgPrint_drawbg(Menu_x[1],Menu_y[Menu_power],0xFFFF,0,"Reset");
	}
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_continue],0xFFFF,0,"Continue(PowerOn at First)");
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_state],0xFFFF,0,"State");
	if(Menu.state == 0){
		pgPrint_drawbg(Menu_x[1] + 6,Menu_y[Menu_state],0xFFFF,0,"Load");
	}else{
		pgPrint_drawbg(Menu_x[1] + 6,Menu_y[Menu_state],0xFFFF,0,"Save");
	}
	pgPrint_drawbg(Menu_x[1],Menu_y[Menu_exit],0xFFFF,0,"Exit");

}

extern int out_wave; // true if enable pcm playback

void wait_release_key(void)
{
	readpad_menu();
	while(now_pad)
	{
		pgWaitV();
		readpad_menu();
	}
}

int menu_psp(int Power){
	int old_pcmflag;

	char *p,*open;
	int x,y,i,old_size = screen_size;
//	sound_board = SOUND_II;//サウンドボード2有効
	old_pcmflag = out_wave;
	out_wave = 0; // stop pcm playback

    pgScreenFlipV();
   	pgFillvram(0);
	MenuMode = 0;

	menu_draw(Power); // メニューキーが離されるまで待つ
	wait_release_key();

	while(1){
		menu_draw(Power);
		while(!now_pad){
			readpad_menu();
			pgWaitV();
		}
		x = 0;
		y = 0;
		if (now_pad & CTRL_UP) y = -1;
		if (now_pad & CTRL_DOWN) y = 1;
		if (now_pad & CTRL_LEFT) x = -1;
		if (now_pad & CTRL_RIGHT) x = 1;
		//×かL-Triggerが押されたらcontinue
		if (now_pad & CTRL_CROSS || now_pad & CTRL_LTRIGGER) {
			MenuMode = Menu_exit;
			goto GOBACK;
		}
		MenuMode += y;
		if(MenuMode < Menu_pspclock) MenuMode = Menu_exit;
		if(MenuMode > Menu_exit) MenuMode = Menu_pspclock;
		
		switch(MenuMode){
		case Menu_pspclock:
			Menu.PSPClock += x;
			if(Menu.PSPClock < 0) Menu.PSPClock = 0;
			if(Menu.PSPClock > 2) Menu.PSPClock = 2;
			scePowerSetClockFrequency(Clock_main[Menu.PSPClock],Clock_main[Menu.PSPClock],Clock_sub[Menu.PSPClock]);
			break;
		case Menu_keyconfig:
			//KeyConfig
			if(now_pad & CTRL_CIRCLE){
				KeyConfigMenu();
			}
			break;
		case Menu_clock:
			Menu.Clock += x;
			if(Menu.Clock < 0) Menu.Clock = 0;
			if(Menu.Clock > 1) Menu.Clock = 1;
			switch(Menu.Clock){
			case 0:
				boot_clock_4mhz = TRUE;
				cpu_clock_mhz   = CONST_4MHZ_CLOCK;
//				sound_clock_mhz = CONST_4MHZ_CLOCK;
				break;
			case 1:
				boot_clock_4mhz = FALSE;
				cpu_clock_mhz   = CONST_8MHZ_CLOCK;
//				sound_clock_mhz = CONST_8MHZ_CLOCK;
				break;
			}
			break;
		case Menu_mode:
			Menu.Mode += x;
			if(Menu.Mode < 0) Menu.Mode = 0;
			if(Menu.Mode > 3) Menu.Mode = 3;
			switch(Menu.Mode){
			case 0:	boot_basic = BASIC_N;		break;
			case 1:	boot_basic = BASIC_V1S;		break;
			case 2:	boot_basic = BASIC_V1H;		break;
			case 3:	boot_basic = BASIC_V2;		break;
			}
			break;
		case Menu_subcpu:
//			if(Power) continue;//電源投入後は変更不可
			Menu.Subcpu += x;
			if(Menu.Subcpu < 0) Menu.Subcpu = 0;
			if(Menu.Subcpu > 2) Menu.Subcpu = 2;
			cpu_timing = Menu.Subcpu;
			break;
		case Menu_palette:
			Menu.Palette += x;
			if(Menu.Palette < 0) Menu.Palette = 0;
			if(Menu.Palette > 1) Menu.Palette = 1;
			if(Menu.Palette == 0) monitor_analog = TRUE;//Analog
			else monitor_analog = FALSE;//Digital
			break;
		case Menu_size:
			Menu.Size += x;
			if(Menu.Size < 0) Menu.Size = 0;
			if(Menu.Size > 1) Menu.Size = 1;
			if(Menu.Size == 0) screen_size = SCREEN_SIZE_HALF;//HALF
			else screen_size = SCREEN_SIZE_FULL;//FULL
			break;
		case Menu_half_interp:
			Menu.Half += x;
			if(Menu.Half < 1){
				Menu.Half = 0;
				use_half_interp = FALSE;
			}else if(Menu.Half > 0){
				Menu.Half = 1;
				use_half_interp = TRUE;
			}
			break;
		case Menu_drive1:
			if(DiskIn[0] == 1){//イメージが複数ある場合
				Menu.Image[0] += x;
				if(Menu.Image[0] < 0) Menu.Image[0] = 0;
				if(Menu.Image[0] > 15) Menu.Image[0] = 15;
				if(Power == 0){
					add_diskimage(Drive1,Menu.Image[0],0);
					boot_from_rom = FALSE;
				}else{
					switch(disk_change_image(DRIVE_1,Menu.Image[0])){
						case 0://OK
							break;
						case 1://No File
							DiskIn[0] = 0;
							Drive1 = "";
							Menu.Image[0] = 0;
							boot_from_rom = TRUE;
							break;
						case 2://No Image
							if(Menu.Image[0] != 0) Menu.Image[0] -= x;
							break;
					}
				}
			}
			if(now_pad & CTRL_CIRCLE || now_pad & CTRL_TRIANGLE){
				DiskIn[0] = 0;
				Drive1 = "";
				Menu.Image[0] = 0;
				boot_from_rom = TRUE;
				if(disk_image_exist(0)){
					disk_eject(0);
				}
			}
			if(now_pad & CTRL_CIRCLE){
				if(GetFileName(0) == 0){
					Drive1 = SetDisk_path[0];
					switch(Power){
					case 0:
						add_diskimage(Drive1,0,0);
						DiskIn[0] = 1;
						Menu.Image[0] = 0;
						boot_from_rom = FALSE;
						break;
					case 1:
						if(disk_insert( DRIVE_1, Drive1, 0 ) == 1){
							//FileOpen Failed
							DiskIn[0] = 0;
						}else{
							DiskIn[0] = 1;
							Menu.Image[0] = 0;
							boot_from_rom = FALSE;
							//イメージファイルが複数あってDrive2があいている場合に同じファイルの次のイメージをセットする
							if(DiskIn[1] == 0){
								DiskIn[1] = 1;
								strcpy(Drive2,Drive1);
								Menu.Image[1] = Menu.Image[0] + 1;
								if(disk_insert( DRIVE_2, Drive1, Menu.Image[1] ) == 1){
									DiskIn[1] = 0;
									Drive2 = "";
									Menu.Image[1] = 0;
								}
							}

						}
						break;
					}
				}
			}
			break;
		case Menu_drive2:
			if(DiskIn[1] == 1){//イメージが複数ある場合
				Menu.Image[1] += x;
				if(Menu.Image[1] < 0) Menu.Image[1] = 0;
				if(Menu.Image[1] > 15) Menu.Image[1] = 15;
				if(Power == 0){
					add_diskimage(Drive2,Menu.Image[1],1);
				}else{
					switch(disk_change_image(DRIVE_2,Menu.Image[1])){
						case 0://OK
							break;
						case 1://No File
							DiskIn[1] = 0;
							Drive2 = "";
							Menu.Image[1] = 0;
							boot_from_rom = TRUE;
							break;
						case 2://No Image
							if(Menu.Image[1] != 0) Menu.Image[1] -= x;
							break;
					}
				}
			}
			if(now_pad & CTRL_CIRCLE || now_pad & CTRL_TRIANGLE){
				DiskIn[1] = 0;
				Drive2 = "";
				Menu.Image[1] = 0;
				if(disk_image_exist(1)){
					disk_eject(1);
				}
			}
			if(now_pad & CTRL_CIRCLE){
				if(GetFileName(1) == 0){
					Drive2 = SetDisk_path[1];
					switch(Power){
					case 0:
						add_diskimage(Drive2,0,1);
						DiskIn[1] = 1;
						Menu.Image[1] = 0;
						boot_from_rom = FALSE;
						break;
					case 1:
						if(disk_insert( DRIVE_2, Drive2, 0 ) == 1){
							//FileOpen Failed
							DiskIn[1] = 0;
						}else{
							DiskIn[1] = 1;
							boot_from_rom = FALSE;
							Menu.Image[1] = 0;
						}
						break;
					}
				}
			}
			break;
		case Menu_power:
			if(now_pad & CTRL_CIRCLE){
				if(Power == 1){
					main_reset();
				}
				goto GOBACK;
			}
		case Menu_continue:
		case Menu_exit:
			if(now_pad & CTRL_CIRCLE){
				if (MenuMode == Menu_exit)
				{
					pgPrint_drawbg(7,30,0xFFFF,0,"Exit Now(& Save Config)? O:OK Other:Cancel");
					now_pad = 0;
					while(!now_pad){
						readpad_menu();
						pgWaitV();
					}
					if(now_pad & CTRL_CIRCLE){
						cfg_write();
						pgFillvram(0);
						//一応クロックを222MHzに戻す
						scePowerSetClockFrequency(222,222,111);
					    sceKernelExitGame();
						return (MenuMode);
					}else{
						pgPrint_drawbg(7,30,0x0000,0,"                                          ");
						break;
					}
				}
				goto GOBACK;
			}
			break;
		case Menu_state:
			if(Power == 0) continue;
			Menu.state += x;
			if(Menu.state < 1) Menu.state = 0;
			if(Menu.state > 0) Menu.state = 1;
			if(now_pad & CTRL_CIRCLE){
				char path_cfg[MAXPATH];
				int i;

				cfg_data.PSPClock = Menu.PSPClock;
				cfg_data.Clock = Menu.Clock;
				cfg_data.Mode = Menu.Mode;
				cfg_data.Subcpu = Menu.Subcpu;
				cfg_data.Palette = Menu.Palette;
				cfg_data.Half = Menu.Half;
				cfg_data.Image[0] = Menu.Image[0];
				cfg_data.Image[1] = Menu.Image[1];
				for(i = 0;i < Control_End;i++){
					cfg_data.KeySetting[i] = KeySetting[i];
				}
				strcpy(cfg_data.File[0],Drive1);
				strcpy(cfg_data.File[1],Drive2);
				cfg_data.DiskIn[0] = DiskIn[0];
				cfg_data.DiskIn[1] = DiskIn[1];
				strcpy(path_cfg,path_main);

				if(Menu.state == 1){
					strcpy(StateFile,Drive1);
					i = sizeof(StateFile);
					while(1){
						if(StateFile[i++] == '.'){
							StateFile[i++] = 's';
							StateFile[i++] = 't';
							StateFile[i++] = 'a';
							break;
						}else if(StateFile[i] == '\0'){
							strcpy(StateFile,"q88p.sta");
							break;
						}
					}
					if(TRUE == suspend(StateFile)){
						pgPrint_drawbg(Menu_x[1] + 11,Menu_y[Menu_state],0xFFFF,0,"OK!   ");
					}else{
						pgPrint_drawbg(Menu_x[1] + 11,Menu_y[Menu_state],0xFFFF,0,"Error!");
					}
				}else{
					strcpy(StateFile,Drive1);
					i = sizeof(StateFile);
					while(1){
						if(StateFile[i++] == '.'){
							StateFile[i++] = 's';
							StateFile[i++] = 't';
							StateFile[i++] = 'a';
							break;
						}else if(StateFile[i] == '\0'){
							strcpy(StateFile,"q88p.sta");
							break;
						}
					}
					if(TRUE == resume(StateFile)){
					    pgScreenFlipV();
						redraw_screen( FALSE );
					    pgScreenFlipV();
						pgPrint_drawbg(Menu_x[1] + 11,Menu_y[Menu_state],0xFFFF,0,"OK!   ");
					}else{
						pgPrint_drawbg(Menu_x[1] + 11,Menu_y[Menu_state],0xFFFF,0,"Error!");
					}
				}
				Menu.PSPClock = cfg_data.PSPClock;
				Menu.Clock = cfg_data.Clock;
				Menu.Mode = cfg_data.Mode;
				Menu.Subcpu = cfg_data.Subcpu;
				Menu.Palette = cfg_data.Palette;
				Menu.Size = cfg_data.Size;
				Menu.Half = cfg_data.Half;
				Menu.Image[0] = cfg_data.Image[0];
				Menu.Image[1] = cfg_data.Image[1];
				for(i = 0;i < Control_End;i++){
					KeySetting[i] = cfg_data.KeySetting[i];
				}
				strcpy(Drive1,cfg_data.File[0]);
				strcpy(Drive2,cfg_data.File[1]);
				DiskIn[0] = cfg_data.DiskIn[0];
				DiskIn[1] = cfg_data.DiskIn[1];
				strcpy(path_main,path_cfg);
				strcpy(ConfigFile,"q88p.cfg");
			}
			break;

		}
		now_pad = 0;
	}

	GOBACK: // 暫定的
	wait_release_key();
	out_wave = old_pcmflag;
    pgScreenFlipV();
	if(old_size != screen_size && Power){
		pgFillvram(0);
	    graphic_system_restart(TRUE);
	    q8tk_misc_redraw();
	    if(screen_size == SCREEN_SIZE_HALF){
		  pgPrint_drawbg(0,0,0xFFFF,0,"QUASI88 --- started ---");
		  pgPrint_drawbg(0,1,0xFFFF,0,"QUASI88 memory clear");
		  pgPrint_drawbg(0,2,0xFFFF,0,"Running");
		}
	}
	return (MenuMode);
}

void KeyConfigMenu(void)
{
	int x,y,Mode = 0,k;
	pgFillvram(0);
	KeySet = 0;
	now_pad = 0;
	while(1){
		KeyConfigDraw(Mode);
		KeyConfigKBDraw(0);
		while(!now_pad){
			readpad_menu();
			pgWaitV();
		}
		x = 0;
		y = 0;
		if (now_pad & CTRL_UP) y = -1;
		if (now_pad & CTRL_DOWN) y = 1;
		if (now_pad & CTRL_LEFT) x = -1;
		if (now_pad & CTRL_RIGHT) x = 1;
		if(Mode == 0){
			KeySet += y;
			if(KeySet < 0) KeySet = Control_End;
			if(KeySet > Control_End) KeySet = 0;
		}else{
			KeySetting[KeySet] += (x + (y * KeyCol));
			while(KeyBorad[KeyboradMap() + KeySetting[KeySet]] == ""){
				KeySetting[KeySet] += (x + (y * KeyCol));
				if(KeySetting[KeySet] < 0) KeySetting[KeySet] = 0;
				if(KeySetting[KeySet] > KeyCol * 5 + 9) KeySetting[KeySet] = KeyCol * 5 + 9;
			}
			if(KeySetting[KeySet] < 0) KeySetting[KeySet] = 0;
			if(KeySetting[KeySet] > KeyCol * 5 + 9) KeySetting[KeySet] = KeyCol * 5 + 9;
		}
		if(KeySet == Control_L) continue;//Lボタンはメニュー固定
#ifdef SoftwareKeybord
		if(KeySet == Control_R) continue;//Rボタンはソフトウェアキーボード固定
#endif
		if(now_pad & CTRL_CIRCLE){
			switch(KeySet){
			case Control_Up:
			case Control_Down:
			case Control_Left:
			case Control_Right:
			case Control_Circle:
			case Control_Cross:
			case Control_Triangle:
			case Control_Square:
			case Control_Start:
			case Control_Select:
				Mode ^= 1;
				break;
			case Control_L:
			case Control_R:
				break;
			default:
				goto exit;
			}
		}
		if(now_pad & CTRL_CROSS){
			if(Mode != 0){
				Mode = 0;
			}else{
				goto exit;
			}
		}
		now_pad = 0;
	}
exit:
	CAPS_KEY = 0;
	KANA_KEY = 0;
	BS_KEY = 0;
	INS_KEY = 0;
	DEL_KEY = 0;
	F6_KEY = 0;
	F7_KEY = 0;
	F8_KEY = 0;
	F9_KEY = 0;
	F10_KEY = 0;
	SHIFT_KEY = 0;
	for(k = 0;k < Control_End;k++){
		SetKeyConfig(k,KeyInput[KeySetting[k]].Port,KeyInput[KeySetting[k]].Bit);
		if(KeySetting[k] == KeyCol * 3 + 1 ) CAPS_KEY = k + 1;
		if(KeySetting[k] == KeyCol * 5     ) KANA_KEY = k + 1;
//		if(KeySetting[k] == KeyCol     + 14) BS_KEY   = k + 1;
		if(KeySetting[k] == KeyCol     + 15) INS_KEY  = k + 1;
//		if(KeySetting[k] == KeyCol     + 16) DEL_KEY  = k + 1;
		if(KeySetting[k] ==               7) F6_KEY = k + 1;
		if(KeySetting[k] ==               8) F7_KEY = k + 1;
		if(KeySetting[k] ==               9) F8_KEY = k + 1;
		if(KeySetting[k] ==              10) F9_KEY = k + 1;
		if(KeySetting[k] ==              11) F10_KEY = k + 1;
		if(KeySetting[k] == KeyCol * 4     ) SHIFT_KEY = k + 1;
	}
	pgFillvram(0);
}

void KeyConfigDraw(int Mode)
{
	int i,color = 0xFFFF;
	static char *Key[] = {
		"UP",
		"DOWN",
		"LEFT",
		"RIGHT",
		"CIRCLE",
		"CROSS",
		"TRIANGLE",
		"SQUARE",
		"L-TRIGGER",
		"R-TRIGGER",
		"START",
		"SELECT",
	};

	pgPrint_drawbg(0,0,0x001F,0,"Quasi88 for PSP Key Setting");
	for(i = 0;i < Control_End;i++){
		pgPrint_drawbg(Menu_x[1],Menu_y[i],0xFFFF - (0x07FF * (KeySet == i)) + ((Mode == 1 * 0x00FF)) ,0,Key[i]);
		if(KeySetting[i] >= 0){
			pgPrint_drawbg(Menu_x[3],Menu_y[i],0xFFFF - (0x07FF * (Mode == 1)) * (KeySet == i),0,KeyBorad[KeyboradMap() + KeySetting[i]]);
		}else if(i == 8){
			pgPrint_drawbg(Menu_x[3],Menu_y[i],0xFFFF,0,"MENU");
		}else if(i == 9){
			pgPrint_drawbg(Menu_x[3],Menu_y[i],0xFFFF,0,"SOFT KB");
		}
	}
	pgPrint_drawbg(Menu_x[1],Menu_y[i],0xFFFF - (0x07FF * (KeySet == i)),0,"EXIT");
}

int SoftKBSet = 0;

void KeyConfigKBDraw(int Mode)
{
	int i,j,k,color = 0xFFFF;

	for(i = 0;i < 6;i++){
		for(j = 0;j < KeyCol;j++){
			color = 0xFFFF;
			if(Mode == 0){//configから呼び出した場合は色つけ
				for(k = 0;k < Control_End;k++){
					if(KeySetting[k] == ((i * KeyCol) + j)) color = (k + 1) * (0x0FF0 / Control_End);
				}
			}else{//SoftwareKeyBoardでは選択してあるところに色つけ
				if(SoftKBSet == ((i * KeyCol) + j)) color = 0xF800;
			}
			if(KeyBorad[KeyboradMap() + (i * KeyCol) + j] != ""){
				pgPrint_drawbg(9 + j * 2,28 + i,color,0,KeyBorad[KeyboradMap() + (i * KeyCol) + j]);
			}else if(Mode == 1){
				pgPrint_drawbg(9 + j * 2,28 + i,color,0,"  ");
			}
		}
	}
	color = 0xFFFF;
	if(Mode == 0){//configから呼び出した場合は色つけ
		for(k = 0;k < Control_End;k++){
			if(KeySetting[k] >= ((5 * KeyCol) + 5) - 1 && KeySetting[k] <= ((5 * KeyCol) + 5) + 1) color = (k + 1) * (0x0FF0 / Control_End);
		}
	}else{//SoftwareKeyBoardでは選択してあるところに色つけ
		if(SoftKBSet >= ((5 * KeyCol) + 5) - 1 && SoftKBSet <= ((5 * KeyCol) + 5) + 1) color = 0xF800;
	}
	pgPrint_drawbg(17,33,color,0,"Space ");

}

void KeyConfigKBErace(void)
{
	int i,j;
	for(i = 0;i < 6;i++){
		for(j = 0;j < KeyCol;j++){
			pgPrint_drawbg(9 + j * 2,28 + i,0xFFFF,0,"  ");
		}
	}
	pgPrint_drawbg(17,33,0xFFFF,0,"     ");
	redraw_screen( FALSE );
}


psp_dirent dlist[0x108];
int dircount = 0;
char pdir[MAXPATH] = "";

int GetFileName(int Drive)
{
	int ret,fd,Num;
	int i,w,y,Coursol;
	char p[MAXPATH];
	int ViewCol[0x108];

getdir:
	w = 0;
	Coursol = 0;
	pgFillvram(0);
	Num = 0;
	strcpy(p,tmp_dir);
	strcat(p,pdir);
	fd = sceIoDopen(p);
	ret = 1;
	pgPrint_drawbg(Menu_x[1],0,0xF800,0,"FileList");
	while((ret>0) && (Num<0x108)) {
		ret = sceIoDread(fd, &dlist[Num]);
		if (ret>0){
			switch(CheckName(dlist[Num].name)){
			case 0://.d88以外のファイル
				if(Num > 0)Num--;
				break;
			case 1://.d88
				ViewCol[Num] = 0;
				break;
			case 2://Directory
				ViewCol[Num] = 1;
				break;
			}
			Num++;
		}
	}
	sceIoDclose(fd);
	strcpy(dlist[0].name,"..");//最初は必ず上にあがるで

	while(1){
		if(Num < 29) w = 0;
		for(i = 0;i < Num - ((29 < Num) * (Num - 29));i++){
				pgPrint_drawbg(Menu_x[1],i + 2,0xFFFF - (0x07FF * (Coursol == (w + i))),0,"                    ");//適当にスペースで消しているので,ファイル名が長いファイルはごみが残るね…
				pgPrint_drawbg(Menu_x[1],i + 2,0xFFFF - (0x07FF * (Coursol == (w + i))) - ((ViewCol[w + i] * 0xFE1F) * (Coursol != (w + i))),0,dlist[i + w].name);
		}
		now_pad = 0;
		while(!now_pad){
			readpad_menu();
			pgWaitV();
		}
		y = 0;
		if (now_pad & CTRL_UP) y = -1;
		if (now_pad & CTRL_DOWN) y = 1;
		if((Coursol - w) == 15) w += y;			//Cursolが画面上15行目ならいっしょにスクロールしちゃう
		Coursol += y;
		if(w < 0) w = 0;						//上端ではスクロールしない
		if(w > (Num - 29)) w = Num - 29;		//下端でもスクロールしない
		if(Coursol < 0) Coursol = 0;
		if(Coursol > (Num - 1))  Coursol = Num - 1;

		if(now_pad & CTRL_CIRCLE) {
			//フォルダの場合
			if(CheckName(dlist[Coursol].name) != 1){
				//ディレクトリ移動する予定。ちなみに".."はひとつあがるで。
				if(Coursol == 0){
					goto dirup;
				}else{
					dircount++;
					strcat(pdir,dlist[Coursol].name);
					strcat(pdir,"/");
					goto getdir;
				}
			}else{
				//ファイルの場合
				strcpy(p,pdir);
				strcat(p,dlist[Coursol].name);
				pgFillvram(0);
				strcpy(SetDisk_path[Drive],p);
				return 0;
			}
		}
		if(now_pad & CTRL_TRIANGLE){
dirup:
			if(dircount > 1){
				for(i = 0;i < MAXPATH;i++) {
					if (pdir[i]==0) break;
				}
				i--;
				while(i > 4) {
					if (pdir[i - 1] == '/') {
						pdir[i] = 0;
						break;
					}
					i--;
				}
				dircount--;
			}else{
				if(dircount == 1) dircount--;
				strcpy(pdir,"");
			}
			goto getdir;
		}
		if(now_pad & CTRL_CROSS){
			pgFillvram(0);
			return 1;
		}
	}
	return 1;
}

int CheckName(char *p)
{
	char *d88;
	d88 = p;
	while(*d88){
		if(*d88++ == '.')break;
	}
	if(!*d88)return 2;
	d88=p;
	while(*d88){
		if(d88[0] == '.' && (d88[1] == 'd' || d88[1] == 'D') && (d88[2] == '8') && (d88[3] == '8') && (d88[4] == 0)) return 1;
		d88++;
	}
	return 0;
}

int KeyboradMap(void)
{
	int Number = 0;
	//None = 0,CAPS = 1,KANA = 2,SHIFT = 3,CAPS + SHIFT = 4,KANA + SHIFT = 5
	if(KANA == 1)
		Number = 2 + (3 * SHIFT);
	else if(CAPS == 1)
		Number = 1 + (3 * SHIFT);
	else
		Number = (3 * SHIFT);
	
	Number *= (KeyCol * 6 + 1);
	return Number;
}
