/************************************************************************/
/*                                                                      */
/* Warning! This file contains some Japanese characters written in      */
/*            EUC-Japan code. So, don't modify the cording type !       */
/*            ^^^^^^^^^           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^         */
/*                                                                      */
/* （注意）このファイル中の日本語は、EUC コードで書かれています。       */
/*         直接日本語の文字列を処理している箇所があるので、他の         */
/*         漢字コードに変換したりしないで下さい。                       */
/*                                                                      */
/************************************************************************/

/*----------------------------------------------------------------------*/
/* このファイルは、menu.c ファイル内にて include されています。		*/
/*----------------------------------------------------------------------*/
/*									*/
/* QUASI88 の タイトル表示およびメニュー画面表示にて表示される文字列は	*/
/* 全てこのファイルで定義されています。					*/
/* 処理の都合上、使用可能な日本語文字列は EUC-Japan のみです。		*/
/*									*/
/*----------------------------------------------------------------------*/

/*
#ifndef	NR_LANG
#define	NR_LANG		2
#endif
#ifndef	NR_DRIVE
#define	NR_DRIVE	2
#endif
*/

/***************************************************************
 * QUASI88 の タイトルにて使用する文字列
 ****************************************************************/

const	T_INDI_VER	indi_ver[] =
{
/*{ 16+192+16, 4+48-16, "Ver. " Q_VERSION },*/

  { 16+192+16, 4+20-16, "Ver " Q_VERSION },
  { 16+192,    4+48-16, "(SDL)" },
  { 0, 0, NULL },				/* 終端 */
};


const	char	*indi_guide[ NR_LANG ] = 
{
  "      Press F12 to MENU screen        ",
  "F12 キーを押すとメニュー画面になります",
};



/***************************************************************
 * QUASI88 の メニューにて使用する文字列
 ****************************************************************/

/*--------------------------------------------------------------
 *	メニューの '*' タグにて表示される情報
 *--------------------------------------------------------------*/

/*
 *	-english 時のメッセージ - - - - - - - - - - - - - - - - -
 */
static const char *menu_ver_info_en[] =
{
  Q_TITLE "  ver. " Q_VERSION  "    - PC-8801 Emulator "
#if	defined(UNIX88)
  "for UNIX "
#elif	defined(WIN88)
  "for Win32 "
#endif
  "(SDL version) -",
  "",

#ifndef	USE_JOY
  " JOYSTICK : ... Not available",
#endif  

#ifdef	USE_SOUND
  "",
  " SOUND ... Available",
  "    SOUND ported from xmame-0.71.1",
  "    (c) Nicola Salmoria, The MAME Team and xmame project",
#ifdef	USE_FMGEN
  "",
  " FM Sound Generator ... Available",
  "     fmgen008-current (c) cisc",
#endif
#else
  " SOUND ... Not available",
#endif  


  NULL,	/* 終端 */
};


/*
 *	日本語表示可能時のメッセージ - - - - - - - - - - - - - - - - -
 */
static const char *menu_ver_info_jp[] =
{
  Q_TITLE "  ver. " Q_VERSION  "    〜 PC-8801 エミュレータ "
#if	defined(UNIX88)
  "for UNIX "
#elif	defined(WIN88)
  "for Win32 "
#endif
  "(SDL版) 〜",
  "",

#ifndef	USE_JOY
  "  ジョイスティックは無効です",
#endif  

#ifdef	USE_SOUND
  "",
  "  サウンド出力が組み込まれています",
  "     サウンド出力は、xmame-0.71.1 から移植されました",
  "     (c) Nicola Salmoria, The MAME Team and xmame project",
#ifdef	USE_FMGEN
  "",
  "  FM Sound Generator が組み込まれています",
  "     fmgen008-current (c) cisc",
#endif
  "",
  "  (注意) 音量変更機能は現在サポートされていません ",
#else
  "  サウンド出力 は組み込まれていません",
#endif

#ifdef	UNIX88
#ifdef	UNIX_WAIT
  "",
  "  ウェイト制御に UNIX のシステムコールを使用します",
#else
  "  ウェイト制御に SDL のタイマ機能を使用します",
#endif
#endif

  NULL,	/* 終端 */
};





/*--------------------------------------------------------------
 *	メインメニュー画面
 *--------------------------------------------------------------*/
enum {
  STR_BASE_RESET,
  STR_BASE_DIPSW,
  STR_BASE_CPU,
  STR_BASE_GRAPH,
  STR_BASE_VOLUME,
  STR_BASE_DISK,
  STR_BASE_KEY,
  STR_BASE_MOUSE,
  STR_BASE_TAPE,
  STR_BASE_MISC,
  STR_BASE_INFO,

  STR_BASE_MONITOR,
  STR_BASE_QUIT,
  STR_BASE_EXIT,
  STR_BASE_END
};
static	const	char	*str_base[ NR_LANG ][ STR_BASE_END ] =
{
  {
    " RESET ",
    " DIP-SW ",
    " CPU ",
    " SCREEN ",
    " VOLUME ",
    " DISK ",
    " KEY ",
    " MOUSE ",
    " TAPE ",
    " MISC ",
    "*",

    "  MONITOR  ",
    " QUIT (F12) ",
    " EXIT (ESC) ",
  },
  {
    " リセット ",
    " DIP-SW ",
    " CPU  ",
    " 画面 ",
    " 音量 ",
    " ディスク ",
    " キー ",
    " マウス ",
    " テープ ",
    "  他  ",
    "*",

    " モニターモード ",
    " 終了 (F12) ",
    " 戻る (ESC) ",
  },
};


enum {
  STR_BASE_QUIT_TITLE,
  STR_BASE_QUIT_OK,
  STR_BASE_QUIT_CANCEL,
  STR_BASE_QUIT_END
};
static	const	char	*str_base_quit[ NR_LANG ][ STR_BASE_QUIT_END ] =
{
  {
    " *** QUIT NOW, REALLY ? *** ",
    "   OK   (F12) ",
    " CANCEL (ESC) ",
  },
  {
    "本当に終了して、よろしいか？",
    " 終了 (F12) ",
    " 取消 (ESC) ",
  },
};



/*--------------------------------------------------------------
 *	「グラフィック」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_GRAPH_FRATE,
  STR_GRAPH_SIZE,
  STR_GRAPH_END
};
static	const	char	*str_graph[ NR_LANG ][ STR_GRAPH_END ] =
{
  {
    " <<< FRAME RATE >>> ",
    " <<< RESIZE >>> ",
  },
  {
    " フレームレート ",
    " 画面サイズ ",
  },
};

enum {
  STR_GRAPH_FRATE_60,
  STR_GRAPH_FRATE_30,
  STR_GRAPH_FRATE_20,
  STR_GRAPH_FRATE_15,
  STR_GRAPH_FRATE_12,
  STR_GRAPH_FRATE_10,
  STR_GRAPH_FRATE_6,
  STR_GRAPH_FRATE_5,
  STR_GRAPH_FRATE_4,
  STR_GRAPH_FRATE_3,
  STR_GRAPH_FRATE_2,
  STR_GRAPH_FRATE_1,
  STR_GRAPH_FRATE_END
};
enum {
  STR_GRAPH_FRATE_FPS,
  STR_GRAPH_FRATE_SCREEN,
  STR_GRAPH_FRATE_END2
};
static	const	char	*str_graph_frate[ NR_LANG ][ STR_GRAPH_FRATE_END ]
							[STR_GRAPH_FRATE_END2]=
{
  {
    { "60", " fps (-frameskip 1) ", },
    { "30", " fps (-frameskip 2) ", },
    { "20", " fps (-frameskip 3) ", },
    { "15", " fps (-frameskip 4) ", },
    { "12", " fps (-frameskip 5) ", },
    { "10", " fps (-frameskip 6) ", },
    { "6",  " fps (-frameskip 10)", },
    { "5",  " fps (-frameskip 12)", },
    { "4",  " fps (-frameskip 15)", },
    { "3",  " fps (-frameskip 20)", },
    { "2",  " fps (-frameskip 30)", },
    { "1",  " fps (-frameskip 60)", },
  },
  {
    { "60", " fps (-frameskip 1) ", },
    { "30", " fps (-frameskip 2) ", },
    { "20", " fps (-frameskip 3) ", },
    { "15", " fps (-frameskip 4) ", },
    { "12", " fps (-frameskip 5) ", },
    { "10", " fps (-frameskip 6) ", },
    { "6",  " fps (-frameskip 10)", },
    { "5",  " fps (-frameskip 12)", },
    { "4",  " fps (-frameskip 15)", },
    { "3",  " fps (-frameskip 20)", },
    { "2",  " fps (-frameskip 30)", },
    { "1",  " fps (-frameskip 60)", },
  },
};

static	const	char	*str_graph_autoskip[ NR_LANG ] =
{
  "Auto frame skip (-autoskip) ",
  "オートスキップを有効にする (-autoskip) ",
};

enum {
  STR_GRAPH_SIZE_HALF,
  STR_GRAPH_SIZE_FULL,
  STR_GRAPH_SIZE_DOUBLE,
  STR_GRAPH_SIZE_END
};
static	const	char	*str_graph_size[ NR_LANG ][ STR_GRAPH_SIZE_END ] =
{
  {
    " HALF SIZE (-half) ",
    " FULL SIZE (-full) ",
    " DOUBLE SIZE (-double) ",
  },
  {
    " 半分サイズ (-half) ",
    " 標準サイズ (-full) ",
    " 倍サイズ (-double) ",
  },
};



enum {
  STR_GRAPH_MISC_HIDE_MOUSE,
  STR_GRAPH_MISC_15K,
  STR_GRAPH_MISC_DIGITAL,
  STR_GRAPH_MISC_NOINTERP,
  STR_GRAPH_MISC_INTERLACE,
  STR_GRAPH_MISC_END
};
static	const	char	*str_graph_misc[ NR_LANG ][ STR_GRAPH_MISC_END ] =
{
  {
    "Hide Mouse-cursor       (-hide_mouse)",
    "Monitor Freq. 15k       (-15k)",
    "Digital Monitor         (-digital)",
    "No reduce interpolation (-nointerp)",
    "Interlace Display       (-interlace)",
  },
  {
    "マウスを表示しない           (-hide_mouse)",
    "モニタ周波数を15kに設定      (-15k)",
    "デジタルモニタに設定         (-digital)",
    "半分サイズ時に縮小補間しない (-nointerp)",
    "インターレース表示する       (-interlace)",
  },
};


/*--------------------------------------------------------------
 *	「CPU設定」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_CPU_CPU,
  STR_CPU_CLOCK,
  STR_CPU_WAIT,
  STR_CPU_END
};
static	const	char	*str_cpu[ NR_LANG ][ STR_CPU_END ] =
{
  {
    " <<< SUB-CPU MODE >>> ",
    " << CLOCK >>",
    " << WAIT >>",
  },
  {
    " SUB-CPU駆動 <変更時はリセットを推奨> (-cpu) ",
    " CPU クロック (-clock) ",
    " ウエイト (-waitfreq, -nowait) ",
  },
};

enum {
  STR_CPU_CPU__1,
  STR_CPU_CPU_0,
  STR_CPU_CPU_1,
  STR_CPU_CPU_2,
  STR_CPU_CPU_3,
  STR_CPU_CPU_END
};
static	const	char	*str_cpu_cpu[ NR_LANG ][ STR_CPU_CPU_END ] =
{
  {
    "  -1  Compatible mode for old version.            (-cpu -1) ",
    "   0  Change when watch flags.                    (-cpu 0)  ",
    "   1  Change fixed period from PIO accessed.      (-cpu 1)  ",
    "   2  Change when same cycles.                    (-cpu 2)  ",
    "   3  Change when same cycles & repeat send/recv. (-cpu 3)  ",
  },
  {
    "  -1  旧来との互換モード                          (-cpu -1) ",
    "   0  フラグチェック時に切り替え                  (-cpu 0)  ",
    "   1  PIOアクセスから一定期間、交互処理           (-cpu 1)  ",
    "   2  同サイクル処理毎に切り替え                  (-cpu 2)  ",
    "   3  同サイクル処理毎と、連続送受信時に切り替え  (-cpu 3)  ",
  },
};

enum {
  STR_CPU_CLOCK_4MHZ,
  STR_CPU_CLOCK_8MHZ,
  STR_CPU_CLOCK_ANNOUNCE,
  STR_CPU_CLOCK_END
};
static	const	char	*str_cpu_clock[ NR_LANG ][ STR_CPU_CLOCK_END ] =
{
  {
    " Set 4MHz ",
    " Set 8MHz ",
    "(Range= 0.1-100.0)",
  },
  {
    " 4MHzに設定 ",
    " 8MHzに設定 ",
    "（範囲は 0.1〜100.0、変更は直ちに反映されます）",
  },
};

enum {
  STR_CPU_WAIT_NOWAIT,
  STR_CPU_WAIT_FREQ,
  STR_CPU_WAIT_ANNOUNCE,
  STR_CPU_WAIT_END
};
static	const	char	*str_cpu_wait[ NR_LANG ][ STR_CPU_WAIT_END ] =
{
  {
    " No Wait ",
    " Frequency for wait ",
    "(Range= 10-240) ",
  },
  {
    " ウエイトなしにする ",
    " ウエイト用の周期 ",
    "（範囲は 10〜240、変更は直ちに反映されます）",
  },
};

enum {
  STR_CPU_MISC_DUMMY,
  STR_CPU_MISC_FDCWAIT,
  STR_CPU_MISC_FDCWAIT_X,
  STR_CPU_MISC_HSBASIC,
  STR_CPU_MISC_HSBASIC_X,
  STR_CPU_MISC_END
};
static	const	char	*str_cpu_misc[ NR_LANG ][ STR_CPU_MISC_END ] =
{
  {
    "",
    "FDC Wait ON",
    "(-fdc_wait)",
    "HighSpeed BASIC ON",
    "(-hsbasic)",
  },
  {
    "",
    "FDCウエイトありにする ",
    "(-fdc_wait)",
    "高速BASIC処理 有効 ",
    "(-hsbasic)",
  },
};






/*--------------------------------------------------------------
 *	「リセット」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_RESET_CURRENT,
  STR_RESET_BASIC,
  STR_RESET_CLOCK,
  STR_RESET_VERSION,
  STR_RESET_DIPSW,
  STR_RESET_DIPSW_SET,
  STR_RESET_DIPSW_QUIT,
  STR_RESET_NOW,
  STR_RESET_END
};
static	const	char	*str_reset[ NR_LANG ][ STR_RESET_END ] =
{
  {
    " Current Mode ->%s/%s/ ROM ver 1.%c ",
    " BASIC MODE ",
    " CPU CLOCK ",
    " ROM VERSION ",
    " DIP-SW Setting ",
    " <<< DIP-SW Setting >>> ",
    " EXIT ",
    " RESET imidiately! ",
  },
  {
    " 現在のモード →%s/%s/ ROM ver 1.%c ",
    " BASIC モード ",
    " CPU クロック ",
    " ROM バージョン ",
    " ディップスイッチ設定 ",
    " <<< ディップスイッチ設定 >>> ",
    " 戻る ",
    " この設定でリセットする ",
  },
};


enum {
  STR_RESET_BASIC_V1S,
  STR_RESET_BASIC_V1H,
  STR_RESET_BASIC_V2,
  STR_RESET_BASIC_N,
  STR_RESET_BASIC_END
};
static	const	char	*str_reset_basic[ NR_LANG ][ STR_RESET_BASIC_END ] =
{
  {
    " N88 V1S ",
    " N88 V1H ",
    " N88 V2 ",
    " N ",
  },
  {
    " N88 V1S ",
    " N88 V1H ",
    " N88 V2 ",
    " N ",
  },
};


enum {
  STR_RESET_CLOCK_4MHZ,
  STR_RESET_CLOCK_8MHZ,
  STR_RESET_CLOCK_END
};
static	const	char	*str_reset_clock[ NR_LANG ][ STR_RESET_CLOCK_END ] =
{
  {
    " 4MHz ",
    " 8MHz ",
  },
  {
    " 4MHz ",
    " 8MHz ",
  },
};

/*--------------------------------------------------------------
 *	「音量」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_VOLUME_VOLUME,
  STR_VOLUME_LEVEL,
  STR_VOLUME_LEVEL2,
  STR_VOLUME_END
};
static	const	char	*str_volume[ NR_LANG ][ STR_VOLUME_END ] =
{
  {
    " Volume ",
    " Level ",
    " depend on FM-level ",
  },
  {
    " 音量 ",
    " レベル ",
    " 以下はＦＭ音量に依存します ",
  },
};

enum {
  STR_VOLUME_NO_SUPPORT,
  STR_VOLUME_NO_AVAILABEL,
  STR_VOLUME_NO_END
};
static	const	char	*str_volume_no[NR_LANG][ STR_VOLUME_NO_END ]=
{
  {
    " Sound is not supported. ",
    " Sound is disable. ",
  },
  {
    " サウンドは使用不可になっています。",
    " サウンド出力が無効になっています。",
  },
};


enum {
  VOLUME_LEVEL_TOTAL,
  VOLUME_LEVEL_FM,
  VOLUME_LEVEL_PSG,
  VOLUME_LEVEL_BEEP,
  VOLUME_LEVEL_RHYTHM,
  VOLUME_LEVEL_ADPCM,
  VOLUME_LEVEL_FMPSG,
  VOLUME_LEVEL_END
};
static	const	char	*str_volume_level[ NR_LANG ][ VOLUME_LEVEL_END ] =
{
  {
    " VOLUME [db]    :",
    " FM sound   [%] :",
    " PSG sound  [%] :",
    " BEEP sound [%] :",
    " RHYTHM     [%] :",
    " ADPCM      [%] :",
    " FM & PSG   [%] :",
  },
  {
    " 音量 [ｄｂ]    ：",
    " ＦＭ音量   [％]：",
    " ＰＳＧ音量 [％]：",
    " ＢＥＥＰ音 [％]：",
    " リズム音量 [％]：",
    " ADPCM 音量 [％]：",
    " FM/PSG音量 [％]：",
  },
};



/*--------------------------------------------------------------
 *	「DIP-SW」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_DIPSW_B,
  STR_DIPSW_R,
  STR_DIPSW_END
};
static	const	char	*str_dipsw[ NR_LANG ][ STR_DIPSW_END ] =
{
  {
    " Boot up ",
    " RC232C ",
  },
  {
    " 初期設定 ",
    " RS232C 設定 ",
  },
};
enum {
  DIPSW_B_TERM,
  DIPSW_B_CH80,
  DIPSW_B_LN25,
  DIPSW_B_BOOT,
  DIPSW_B_END
};
static	const	char	*str_dipsw_b[ NR_LANG ][ DIPSW_B_END ] =
{
  {
    "BOOT MODE           :",
    "Chars per Line      :",
    "Lines per screen    :",
    "Boot Up from        :",
  },
  {
    "立ち上げモード     ：",
    "１行あたりの文字数 ：",
    "１画面あたりの行数 ：",
    "システムの立ち上げ ：",
  }
};
enum {
  DIPSW_R_BAUDRATE,
  DIPSW_R_HDPX,
  DIPSW_R_XPRM,
  DIPSW_R_ST2B,
  DIPSW_R_DT8B,
  DIPSW_R_SPRM,
  DIPSW_R_PDEL,
  DIPSW_R_ENPTY,
  DIPSW_R_EVPTY,
  DIPSW_R_END
};
static	const	char	*str_dipsw_r[ NR_LANG ][ DIPSW_R_END ] =
{
  {
    "Baud Rate (BPS)     :",
    "Duplex              :",
    "X parameter         :",
    "Stop Bit            :",
    "Data Bit            :",
    "S parameter         :",
    "DEL code            :",
    "Patiry Check        :",
    "Patiry              :",
  },
  {
    "通信速度［ボー］   ：",
    "通  信  方  式     ：",
    "Ｘパラメータ       ：",
    "ストップビット長   ：",
    "データビット長     ：",
    "Ｓパラメータ       ：",
    "ＤＥＬコード       ：",
    "パリティチェック   ：",
    "パ  リ  ティ       ：",
  },
};

enum {	DIPSW_B_TERM_TERM,	DIPSW_B_TERM_BASIC,	DIPSW_B_XXXX_END };
enum {	DIPSW_B_CH80_CH80,	DIPSW_B_CH80_CH40	};
enum {	DIPSW_B_LN25_LN25,	DIPSW_B_LN25_LN20	};
enum {	DIPSW_B_BOOT_DISK,	DIPSW_B_BOOT_ROM	};

enum {	DIPSW_R_HDPX_HALF,	DIPSW_R_HDPX_FULL,	DIPSW_R_XXXX_END };
enum {	DIPSW_R_XPRM_EN,	DIPSW_R_XPRM_DIS	};
enum {	DIPSW_R_ST2B_2B,	DIPSW_R_ST2B_1B		};
enum {	DIPSW_R_DT8B_8B,	DIPSW_R_DT8B_7B		};
enum {	DIPSW_R_SPRM_EN,	DIPSW_R_SPRM_DIS	};
enum {	DIPSW_R_PDEL_EN,	DIPSW_R_PDEL_DIS	};
enum {	DIPSW_R_ENPTY_EN,	DIPSW_R_ENPTY_DIS	};
enum {	DIPSW_R_EVPTY_EV,	DIPSW_R_EVPTY_ODD	};

static	const	char	*str_dipsw_b_term[ NR_LANG ][ DIPSW_B_XXXX_END ] =
{
  { "TERMINAL   ", "BASIC      ", },{ "ターミナル ", "ＢＡＳＩＣ ", },
};
static	const	char	*str_dipsw_b_ch80[ NR_LANG ][ DIPSW_B_XXXX_END ] =
{
  { "80ch / line", "40ch / line", },{ "８０字     ", "４０字     ", },
};
static	const	char	*str_dipsw_b_ln25[ NR_LANG ][ DIPSW_B_XXXX_END ] =
{
  { "25line/scrn", "20line/scrn", },{ "２５行     ", "２０行     ", },
};
static	const	char	*str_dipsw_b_boot[ NR_LANG ][ DIPSW_B_XXXX_END ] =
{
  { "DISK       ", "ROM        ", },{ "ディスク   ", "ＲＯＭ     ", },
};

static	const	char	*str_dipsw_r_hdpx[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "HALF       ", "FULL       ", },{ "半二重     ", "全二重     ", },
};
static	const	char	*str_dipsw_r_xprm[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "Enable     ", "Disable    ", },{ "有  効     ", "無  効     ", },
};
static	const	char	*str_dipsw_r_st2b[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "2 bit      ", "1 bit      ", },{ "２ bit     ", "１ bit     ", },
};
static	const	char	*str_dipsw_r_dt8b[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "8 bit      ", "7 bit      ", },{ "８ bit     ", "７ bit     ", },
};
static	const	char	*str_dipsw_r_sprm[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "Enable     ", "Disable    ", },{ "有  効     ", "無  効     ", },
};
static	const	char	*str_dipsw_r_pdel[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "Enable     ", "Disable    ", },{ "有  効     ", "無  効     ", },
};
static	const	char	*str_dipsw_r_enpty[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "Yes        ", "No         ", },{ "有  り     ", "無  し     ", },
};
static	const	char	*str_dipsw_r_evpty[ NR_LANG ][ DIPSW_R_XXXX_END ] =
{
  { "Even       ", "Odd        ", },{ "偶  数     ", "奇  数     ", },
};






/*--------------------------------------------------------------
 *	「ディスク」 タグ
 *--------------------------------------------------------------*/
enum {
  ERR_NO,
  ERR_CANT_OPEN,
  ERR_READ_ONLY,
  ERR_MAYBE_BROKEN,
  ERR_SEEK,
  ERR_WRITE,
  ERR_OVERFLOW,
  ERR_UNEXPECTED,
  ERR_END
};
static	const	char	*str_err[ NR_LANG ][ ERR_END ] =
{
  {
    " OK ",
    "File in DRIVE %d: / can't open.",
    "File in DRIVE %d: / read only.",
    "File in DRIVE %d: / maybe broken.",
    "File in DRIVE %d: / SEEK Error.",
    "File in DRIVE %d: / WRITE Error.",
    "File in DRIVE %d: / strings too long.",
    "File in DRIVE %d: / UNEXPECTED Error.",
  },
  {
    " 確認 ",
    "ドライブ %d:／ファイルが開けません。",
    "ドライブ %d:／読み込み専用ファイルです。",
    "ドライブ %d:／ファイルが(多分)壊れています。",
    "ドライブ %d:／シークエラーが発生しました。",
    "ドライブ %d:／書き込みエラーが発生しました。",
    "ドライブ %d:／入力文字列が長過ぎます。",
    "ドライブ %d:／予期せぬエラーが発生しました。",
  },
};
static	const	char	*str_err2[ NR_LANG ][ ERR_END ] =
{
  {
    " OK ",
    "Error / can't open.",
    "Error / read only.",
    "Error / maybe broken.",
    "Error / SEEK Error.",
    "Error / WRITE Error.",
    "Error / strings too long.",
    "Error / UNEXPECTED Error.",
  },
  {
    " 確認 ",
    "エラー／ファイルが開けません。",
    "エラー／読み込み専用ファイルです。",
    "エラー／ファイルが(多分)壊れています。",
    "エラー／シークエラーが発生しました。",
    "エラー／書き込みエラーが発生しました。",
    "エラー／入力文字列が長過ぎます。",
    "エラー／予期せぬエラーが発生しました。",
  },
};

enum {
  STR_DISK_ATTR_RENAME_TITLE,
  STR_DISK_ATTR_RENAME_OK,
  STR_DISK_ATTR_RENAME_CANCEL,
  STR_DISK_ATTR_RENAME_END
};
static	const	char *str_disk_attr_rename[NR_LANG][STR_DISK_ATTR_RENAME_END] =
{
  {
    " Change image name '%s' at drive [%d:]. ",
    "  OK  ",
    "CANCEL",
  },
  {
    " イメージ 「%s」（ドライブ [%d:]）の名前を変更します ",
    " 変更 ",
    " 取消 ",
  },
};

enum {
  STR_DISK_ATTR_PROTECT_TITLE,
  STR_DISK_ATTR_PROTECT_SET,
  STR_DISK_ATTR_PROTECT_UNSET,
  STR_DISK_ATTR_PROTECT_CANCEL,
  STR_DISK_ATTR_PROTECT_END
};
static  const  char *str_disk_attr_protect[NR_LANG][STR_DISK_ATTR_PROTECT_END]=
{
  {
    " (Un)Set Protect image '%s' at drive [%d:]. ",
    " SET PROTECT ",
    " UNSET PROTECT ",
    " CANCEL ",
  },
  {
    " イメージ 「%s」（ドライブ [%d:]）のプロテクト状態を変更します",
    " プロテクト状態にする ",
    " プロテクトを解除する ",
    " 取消 ",
  },
};

enum {
  STR_DISK_ATTR_FORMAT_TITLE,
  STR_DISK_ATTR_FORMAT_WARNING,
  STR_DISK_ATTR_FORMAT_DO,
  STR_DISK_ATTR_FORMAT_NOT,
  STR_DISK_ATTR_FORMAT_CANCEL,
  STR_DISK_ATTR_FORMAT_END
};
static	const	char *str_disk_attr_format[NR_LANG][STR_DISK_ATTR_FORMAT_END] =
{
  {
    " (Un)Format image '%s' at drive [%d:]. ",
    "(WARNING :data in the image will lost!)",
    " FORMAT ",
    " UNFORMAT ",
    " CANCEL ",
  },
  {
    " イメージ 「%s」（ドライブ [%d:]）を(アン)フォーマットします ",
    "(注意:イメージ内のデータは消去されます)",
    " フォーマットする ",
    " アンフォーマットする ",
    " 取消 ",
  },
};

enum {
  STR_DISK_ATTR_BLANK_TITLE,
  STR_DISK_ATTR_BLANK_OK,
  STR_DISK_ATTR_BLANK_CANCEL,
  STR_DISK_ATTR_BLANK_END
};
static	const	char *str_disk_attr_blank[NR_LANG][STR_DISK_ATTR_BLANK_END] =
{
  {
    " Append Blank image at drive [%d:]. ",
    " APPEND ",
    " CANCEL ",
  },
  {
    " ドライブ [%d:] の ファイルに、ブランクイメージを追加します",
    " ブランクイメージの追加 ",
    " 取消 ",
  },
};

enum {
  STR_DISK_ATTR_TITLE,
  STR_DISK_ATTR_RENAME,
  STR_DISK_ATTR_PROTECT,
  STR_DISK_ATTR_FORMAT,
  STR_DISK_ATTR_BLANK,
  STR_DISK_ATTR_CANCEL,
  STR_DISK_ATTR_END
};
static	const	char *str_disk_attr[ NR_LANG ][ STR_DISK_ATTR_END ] =
{
  {
    " Change Attribute of image '%s' at drive [%d:]. ",
    "RENAME",
    "PROTECT",
    "(UN)FORMAT",
    "APPEND BLANK",
    "CANCEL",
  },
  {
    " イメージ 「%s」（ドライブ [%d:]）の属性などを変更します ",
    "名前変更",
    "属性変更",
    "(アン)フォーマット",
    "ブランクの追加",
    " 取消 ",
  },
};


enum {
  STR_DISK_OPEN_OPEN,
  STR_DISK_OPEN_BOTH,
  STR_DISK_OPEN_END
};
static	const	char *str_disk_open[ NR_LANG ][ STR_DISK_OPEN_END ][NR_DRIVE] =
{
  {
    {
      " OPEN FILE in DRIVE [1:] ",
      " OPEN FILE in DRIVE [2:] ",
    },
    {
      " OPEN FILE in DRIVE [1:] & [2:] ",
      " OPEN FILE in DRIVE [1:] & [2:] ",
    }
  },
  {
    {
      " DRIVE [1:] に、ファイルを開けます ",
      " DRIVE [2:] に、ファイルを開けます ",
    },
    {
      " DRIVE [1:] と [2:] に、ファイルを開けます ",
      " DRIVE [1:] と [2:] に、ファイルを開けます ",
    }
  },
};


enum {
  STR_DISK_IMAGE_DRIVE_1,
  STR_DISK_IMAGE_DRIVE_2,
  STR_DISK_IMAGE_ENPTY,
  STR_DISK_IMAGE_BLANK,
  STR_DISK_IMAGE_END
};
static	const	char *str_disk_image[ NR_LANG ][ STR_DISK_IMAGE_END ] =
{
  {
    " <<< DRIVE [1:] >>>",
    " <<< DRIVE [2:] >>>",
    "< EMPTY >                 ",
    "  Create Blank  "
  },
  {
    " <<< DRIVE [1:] >>>",
    " <<< DRIVE [2:] >>>",
    "< なし >                  ",
    " ブランクの作成 "
  },
};
enum {
  STR_DISK_INFO_DRIVE_1,
  STR_DISK_INFO_DRIVE_2,
  STR_DISK_INFO_STAT,
  STR_DISK_INFO_STAT_READY,
  STR_DISK_INFO_STAT_BUSY,
  STR_DISK_INFO_ATTR,
  STR_DISK_INFO_ATTR_RW,
  STR_DISK_INFO_ATTR_RO,
  STR_DISK_INFO_NR,
  STR_DISK_INFO_NR_BROKEN,
  STR_DISK_INFO_NR_OVER,
  STR_DISK_INFO_NODISK,
  STR_DISK_INFO_END
};
static	const	char *str_disk_info[ NR_LANG ][ STR_DISK_INFO_END ] =
{
  {
    " DRIVE [1:] ",
    " DRIVE [2:] ",
    "STATUS     ",		/* "STATUS     READY" で16文字 */
    "READY",
    " BUSY",
    "ATTR  ",			/* "ATTR  READ/WRITE" で16文字 */
    "READ/WRITE",
    " READ-ONLY",
    "IMAGE  ",			/* "IMAGE  xxxxxxxxx" で16文字、 x は 9文字 */
    "+BROKEN",
    " OVER",
    "",
  },
  {
    " DRIVE [1:] ",
    " DRIVE [2:] ",
    "状態       ",
    "READY",
    " BUSY",
    "属性  ",
    "READ/WRITE",
    " READ-ONLY",
    "総数   ",
    " +破損",
    " 以上 ",
    "",
  },
};



enum {
  IMG_OPEN,
  IMG_CLOSE,
  IMG_BOTH,
  IMG_COPY,
  IMG_ATTR,
  IMG_END
};
static	const	char *str_disk_button[ NR_LANG ][ IMG_END ][ NR_DRIVE ] =
{
  {
    {
      " DRIVE [1:]           OPEN ",
      " DRIVE [2:]           OPEN ",
    },
    {
      " DRIVE [1:]          CLOSE ",
      " DRIVE [2:]          CLOSE ",
    },
    {
      " DRIVE [1:][2:] BOTH  OPEN ",
      " DRIVE [1:][2:] BOTH  OPEN ",
    },
    {
      " DRIVE [1:] <= [2:]   OPEN ",
      " DRIVE [1:] => [2:]   OPEN ",
    },
    {
      " CHANGE ATTRIBUTE of IMAGE ",
      " CHANGE ATTRIBUTE of IMAGE ",
    },
  },
  {
    {
      " DRIVE [1:]           開く ",
      " DRIVE [2:]           開く ",
    },
    {
      " DRIVE [1:]         閉じる ",
      " DRIVE [2:]         閉じる ",
    },
    {
      " DRIVE [1:][2:] 両方に開く ",
      " DRIVE [1:][2:] 両方に開く ",
    },
    {
      " DRIVE [1:] ← [2:]   開く ",
      " DRIVE [1:] → [2:]   開く ",
    },
    {
      " イメージの 属性を変更する ",
      " イメージの 属性を変更する ",
    },
  },
};

enum {
  STR_DISK_BLANK_FSEL,
  STR_DISK_BLANK_WARN_0,
  STR_DISK_BLANK_WARN_1,
  STR_DISK_BLANK_WARN_APPEND,
  STR_DISK_BLANK_WARN_CANCEL,
  STR_DISK_BLANK_END
};
static	const	char	*str_disk_blank[ NR_LANG ][ STR_DISK_BLANK_END ] =
{
  {
    " Create a new file as blank image file.",
    " This File Already Exist. ",
    " Append a blank image ? ",
    " APPEND ",
    " CANCEL ",
  },
  {
    " ブランクイメージファイルを新規作成します ",
    " 指定したファイルはすでに存在します。 ",
    " ブランクイメージを追加しますか？ ",
    " 追加する ",
    " 取消 ",
  },
};



/*--------------------------------------------------------------
 *	「その他」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_MISC_SUSPEND,
  STR_MISC_BLANK,
  STR_MISC_SNAPSHOT,
  STR_MISC_END
};
static	const	char	*str_misc[ NR_LANG ][ STR_MISC_END ] =
{
  {
    " Suspend ",
    " Blank Image ",
    " Screen SnapShot ",
  },
  {
    " サスペンド ",
    " ブランクの作成 ",
    " スクリーン スナップショット ",
  },
};

enum {
  STR_MISC_SUSPEND_FILENAME,
  STR_MISC_SUSPEND_CHANGE,
  STR_MISC_SUSPEND_BUTTON,
  STR_MISC_SUSPEND_FSEL,
  STR_MISC_SUSPEND_END
};
static	const	char	*str_misc_suspend[ NR_LANG ][ STR_MISC_SUSPEND_END ] =
{
  {
    " State file       ",
    "Change",
    " EXEC ",
    " Input (Select) a suspend-image filename. ",
  },
  {
    " ステートファイル ",
    " 変更 ",
    " 実行 ",
    " サスペンド情報を書き込むファイルを入力して下さい ",
  },
};

enum {
  STR_MISC_SUSPEND_ERR_DISMIS,
  STR_MISC_SUSPEND_ERR_QUIT,
  STR_MISC_SUSPEND_ERR_CONTINUE,
  STR_MISC_SUSPEND_ERR_OK,
  STR_MISC_SUSPEND_ERR_NG,
  STR_MISC_SUSPEND_ERR_LINE,
  STR_MISC_SUSPEND_ERR_INFO,
  STR_MISC_SUSPEND_ERR_NOTHING,
  STR_MISC_SUSPEND_ERR_END
};
static	const	char	*str_misc_suspend_err[ NR_LANG ][ STR_MISC_SUSPEND_ERR_END ] =
{
  {
    " OK ",
    " QUIT ",
    " CONTINUE ",
    "Suspend Finished.",
    "Error / Suspend not finished.",
    "----------------------------------------------",
    "  ( Currently image file(s) in drive 1 & 2 )  ",
    " Nothing ",
  },
  {
    "確認",
    " 終了 ",
    " 継続 ",
    "サスペンド情報を保存しました。",
    "エラー／サスペンド情報は保存されませんでした。",
    "----------------------------------------------",
    "  ( 現在ドライブにセットされているファイル )  ",
    " なし ",
  },
};


enum {
  STR_MISC_SNAPSHOT_FORMAT,
  STR_MISC_SNAPSHOT_FILENAME,
  STR_MISC_SNAPSHOT_CHANGE,
  STR_MISC_SNAPSHOT_BUTTON,
  STR_MISC_SNAPSHOT_FSEL,
  STR_MISC_SNAPSHOT_CMD,
  STR_MISC_SNAPSHOT_END
};
static	const	char   *str_misc_snapshot[ NR_LANG ][ STR_MISC_SNAPSHOT_END ] =
{
  {
    " Format          ",
    " File basename    ",
    "Change",
    " SAVE ",
    " Input (Select) a snapshot base-filename. ",
    "Exec following Command ",
  },
  {
    " 画像形式        ",
    " ファイルベース名 ",
    " 変更 ",
    " 保存 ",
    " スナップショットのファイル名を入力して下さい ",
    "次のコマンドを実行する ",
  },
};

enum {
  STR_MISC_SNAPSHOT_FORMAT_RAW,
  STR_MISC_SNAPSHOT_FORMAT_ASC,
  STR_MISC_SNAPSHOT_FORMAT_XPM,
  STR_MISC_SNAPSHOT_FORMAT_END
};
static	const	char	*str_misc_snapshot_format[ NR_LANG ][ STR_MISC_SNAPSHOT_FORMAT_END ] =
{
  {
    " PPM (raw) ",
    " PPM (ascii) ",
    " XPM ",
  },
  {
    " PPM (raw) ",
    " PPM (ascii) ",
    " XPM ",
  },
};

/*--------------------------------------------------------------
 *	「キー」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_KEY_CFG,
  STR_KEY_FKEY,
  STR_KEY_SKEY,
  STR_KEY_CASSIGN,
  STR_KEY_END
};
static	const	char	*str_key[ NR_LANG ][ STR_KEY_END ] =
{
  {
    " Key Config ",
    " Function key Config ",
    " Software Keyboard ",
    " Curosr Key Config ",
  },
  {
    " キー設定 ",
    " ファンクションキー設定 ",
    "ソフトウェアキーボード",
    " カーソルキー設定 ",
  },
};

enum {
  STR_KEY_CFG_TENKEY,
  STR_KEY_CFG_CURSOR,
  STR_KEY_CFG_NUMLOCK,
  STR_KEY_CFG_END
};
static	const	char	*str_key_cfg[ NR_LANG ][ STR_KEY_CFG_END ] =
{
  {
    "Set 1-9 key to TEN-key    (-tenkey)  ",
    "Set CUSROR-key to TEN-key (-cursor)  ",
    "software NUM-Lock ON      (-numlock) ",
  },
  {
    "数字入力をキーテンキーに割り当てる (-tenkey)  ",
    "カーソルキーをテンキーに割り当てる (-cursor)  ",
    "ソフトウェア NUM Lock をオンする   (-numlock) ",
  },
};

enum {
  STR_KEY_FKEY_F6,
  STR_KEY_FKEY_F7,
  STR_KEY_FKEY_F8,
  STR_KEY_FKEY_F9,
  STR_KEY_FKEY_F10,
  STR_KEY_FKEY_END
};
static	const	char	*str_key_fkey[ NR_LANG ][ STR_KEY_FKEY_END ] =
{
  {
    "   f6  key ",
    "   f7  key ",
    "   f8  key ",
    "   f9  key ",
    "   f10 key ",
  },
  {
    "   f6  キー ",
    "   f7  キー ",
    "   f8  キー ",
    "   f9  キー ",
    "   f10 キー ",
  },
};



static	const	char	*str_key_fkey_fn[ NR_LANG ][ FN_end ] =
{
  {
    "default       (function key)",
    "FRATE-UP    : Frame Rate  Up",
    "FRATE-DOWN  : Frame Rate  Down",
    "VOLUME-UP   : Volume  Up   (if supported)",
    "VOLUME-DOWN : Volume  Down (if supported)",
    "PAUSE       : Pause",
    "RESIZE      : Resize",
    "NOWAIT      : No-Wait",
    "SPEED-UP    : Speed Up",
    "SPEED-DOWN  : Speed Down",
    "MOUSE-HIDE  : Hide Mouse",
    "DGA         : Full Screen Mode (if supported)",
    "SNAPSHOT    : Save Screen Snapshot",
    "SUSPEND     : Suspend QUASI88",
    "IMAGE-NEXT1 : Drive 1:  Next Image",
    "IMAGE-PREV1 : Drive 1:  Prev Image",
    "IMAGE-NEXT2 : Drive 2:  Next Image",
    "IMAGE-PREV2 : Drive 2:  Prev Image",
    "NUMLOCK     : Software NUM Lock",
    "RESET       : Reset switch",
    "KANA        : KANA key",
    "ROMAJI      : KANA(ROMAJI) Key",
    "CAPS        : CAPS Key",
    "KETTEI      : KETTEI Key",
    "HENKAN      : HENKAN Key",
    "ZENKAKU     : ZENKAKU Key",
    "PC          : PC Key",
  },
  {
    "default       標準 (ファンクションキー)",
    "FRATE-UP    : フレームレート 上げる ",
    "FRATE-DOWN  : フレームレート 下げる ",
    "VOLUME-UP   : 音量 上げる (対応時のみ)",
    "VOLUME-DOWN : 音量 下げる (対応時のみ)",
    "PAUSE       : 一時停止",
    "RESIZE      : 画面サイズ変更",
    "NOWAIT      : ウエイトなし",
    "SPEED-UP    : 速度 上げる ",
    "SPEED-DOWN  : 速度 下げる ",
    "MOUSE-HIDE  : マウスを隠す",
    "DGA         : フルスクリーン切替 (対応時のみ)",
    "SNAPSHOT    : スクリーンスナップショット",
    "SUSPEND     : サスペンド",
    "IMAGE-NEXT1 : Drive 1:  次イメージ",
    "IMAGE-PREV1 : Drive 1:  前イメージ",
    "IMAGE-NEXT2 : Drive 2:  次イメージ",
    "IMAGE-PREV2 : Drive 2:  前イメージ",
    "NUMLOCK     : ソフトウェア NUM Lock",
    "RESET       : リセット スイッチ",
    "KANA        : カナ キー",
    "ROMAJI      : カナ(ローマ字入力) キー",
    "CAPS        : CAPS キー",
    "KETTEI      : 決定 キー",
    "HENKAN      : 変換 キー",
    "ZENKAKU     : 全角 キー",
    "PC          : PC キー",
  },
};

enum {
  STR_SKEY_SET,
  STR_SKEY_CAN,
  STR_SKEY_QUIT,
  STR_SKEY_SET_END
};
static	const	char	*str_skey_set[ NR_LANG ][ STR_SKEY_SET_END ] =
{
  {
    "Setting",
    "All key release & QUIT",
    " QUIT ",
  },
  {
    " 設定 ",
    " 全てオフにして戻る ",
    " 戻る ",
  },
};

enum {
  STR_KEY_CASSIGN_UP,
  STR_KEY_CASSIGN_LEFT,
  STR_KEY_CASSIGN_RIGHT,
  STR_KEY_CASSIGN_DOWN,
  STR_KEY_CASSIGN_EFFECTIVE,
  STR_KEY_CASSIGN_END
};
static	const	char	*str_key_cassign[ NR_LANG ][ STR_KEY_CASSIGN_END ] =
{
  {
    "      \036",		/* Up */
    "  \035",			/* Left */
    "     \034",		/* Right */
    "      \037",		/* Down */
    "Config Effective ",
  },
  {
    "      \036",		/* Up */
    "  \035",			/* Left */
    "     \034",		/* Right */
    "      \037",		/* Down */
    "設定を有効にする ",
  },
};

/*--------------------------------------------------------------
 *	「マウス」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_MOUSE_MODE,
  STR_MOUSE_JOYSTICK,
  STR_MOUSE_END
};
static	const	char	*str_mouse[ NR_LANG ][ STR_MOUSE_END ] =
{
  {
    " mouse setting ",
    " joystick setting ",
  },
  {
    " マウス設定 ",
    " ジョイスティック設定 ",
  },
};

enum {
  STR_MOUSE_MODE_NOMOUSE,
  STR_MOUSE_MODE_MOUSE,
  STR_MOUSE_MODE_JOYMOUSE,
  STR_MOUSE_MODE_JOYSTICK,
  STR_MOUSE_MODE_JOYKEY,
  STR_MOUSE_MODE_END
};
static	const	char	*str_mouse_mode[ NR_LANG ][ STR_MOUSE_MODE_END ] =
{
  {
    " Mouse not available              (-nomouse) ",
    " Mouse available                  (-mouse)   ",
    " Mouse available in joystick-mode (-joymouse) ",
    " Joystick available               (-joystick) (if supported)",
    " Joystick available as key-input  (-joykey)   (if supported)",
  },
  {
    " マウスなし                       (-nomouse) ",
    " マウスあり                       (-mouse) ",
    " マウスを JOYSTICKモード で使用   (-joymouse) ",
    " ジョイスティックあり             (-joystick) 対応時のみ ",
    " ジョイスティックをキーに割り当て (-joykey)   対応時のみ ",
  },
};

enum {
  STR_MOUSE_JOYSWAP,
  STR_MOUSE_JOYKEYASSIGN,
  STR_MOUSE_JOYKEYASSIGN_A,
  STR_MOUSE_JOYKEYASSIGN_B,
  STR_MOUSE_JOYEND
};
static	const	char	*str_mouse_joy[ NR_LANG ][ STR_MOUSE_JOYEND ] =
{
  {
    "Swap Joystick-button A and B (-joy_swap)",
    " joy-button key assign (under setting -joykey) ",
    "   Button-A ",
    "   Button-B ",
  },
  {
    "ジョイスティックボタンを入れ替え (-joyswap)",
    " キー割り当て設定 (キー割り当て有効時のみ) ",
    "   ボタン A ",
    "   ボタン B ",
  },
};

enum {
  STR_MOUSE_JOY_ASSIGN_NONE,
  STR_MOUSE_JOY_ASSIGN_X,
  STR_MOUSE_JOY_ASSIGN_Z,
  STR_MOUSE_JOY_ASSIGN_SPACE,
  STR_MOUSE_JOY_ASSIGN_RET,
  STR_MOUSE_JOY_ASSIGN_SHIFT,
  STR_MOUSE_JOY_ASSIGN_ESC,
  STR_MOUSE_JOY_ASSIGN_END
};
static	const	char	*str_mouse_joy_assign[ NR_LANG ][ STR_MOUSE_JOY_ASSIGN_END ] =
{
  {
    " none ",
    " X    ",
    " Z    ",
    " SPACE",
    " RET  ",
    " SHIFT",
    " ESC  ",
  },
  {
    " なし ",
    " X    ",
    " Z    ",
    " SPACE",
    " RET  ",
    " SHIFT",
    " ESC  ",
  },
};

/*--------------------------------------------------------------
 *	「テープ」 タグ
 *--------------------------------------------------------------*/
enum {
  STR_TAPE_IMAGE,
  STR_TAPE_INTR,
  STR_TAPE_END
};
static	const	char	*str_tape[ NR_LANG ][ STR_TAPE_END ] =
{
  {
    " Tape image ",
    " Tape Load Timing "
  },
  {
    " テープイメージ ",
    " テープロードの処理方法 "
  },
};

enum {
  STR_TAPE_LOAD_FILENAME,
  STR_TAPE_LOAD_CHANGE,
  STR_TAPE_LOAD_EJECT,
  STR_TAPE_LOAD_REW,
  STR_TAPE_LOAD_FSEL,
  STR_TAPE_LOAD_NOFILE,
  STR_TAPE_LOAD_END
};
static	const	char	*str_tape_load[ NR_LANG ][ STR_TAPE_LOAD_END ] =
{
  {
    " for Lave :",
    " Change File ",
    " Eject  ",
    " Rewind ",
    " Input (Select) a tape-load-image filename. (CMT/T88)",
    "(Nothing)",
  },
  {
    " ロード用：",
    " ファイル変更 ",
    " 取出し ",
    " 巻戻し ",
    " ロード用テープイメージ(CMT/T88)を入力して下さい",
    "（ファイルなし）",
  },
};

enum {
  STR_TAPE_SAVE_FILENAME,
  STR_TAPE_SAVE_CHANGE,
  STR_TAPE_SAVE_EJECT,
  STR_TAPE_SAVE_FSEL,
  STR_TAPE_SAVE_NOFILE,
  STR_TAPE_SAVE_END
};
static	const	char	*str_tape_save[ NR_LANG ][ STR_TAPE_SAVE_END ] =
{
  {
    " for Save :",
    " Change File ",
    " Eject  ",
    " Input (Select) a tape-save-image filename. (CMT)",
    "(Nothing)",
  },
  {
    " セーブ用：",
    " ファイル変更 ",
    " 取出し ",
    " セーブ用テープイメージ(CMT)を入力して下さい",
    "（ファイルなし）",
  },
};


enum {
  STR_TAPE_INTR_YES,
  STR_TAPE_INTR_NO,
  STR_TAPE_INTR_END,
};
static	const	char	*str_tape_intr[ NR_LANG ][ STR_TAPE_INTR_END ] =
{
  {
    " Use Interrupt     (Choose in N88-BASIC mode) ",
    " Not Use Interrupt (Choose in N-BASIC mode for LOAD speed-up) ",
  },
  {
    " 割り込みを使う     (N88-BASIC では、必ずこちらを選択してください) ",
    " 割り込みを使わない (N-BASIC は、こちらでも可。ロードが速くなります) ",
  },
};

