/************************************************************************/
/*									*/
/* 起動直後の引数の処理と、ワークの初期化 (OS依存)			*/
/*									*/
/*									*/
/* 【関数】								*/
/* int	config_init( int argc, char *argv[] )				*/
/* int	exist_rcfile( void )						*/
/*									*/
/* 【関数 (OS非依存) 】							*/
/* void	disk_set_args_file( void )					*/
/* void	bootup_work_init( void )					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "getconf.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "joystick.h"
#include "memory.h"
#include "screen.h"
#include "sound.h"
#include "fdc.h"

#include "emu.h"
#include "file-op.h"
#include "drive.h"
#include "menu.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "indicator.h"
#include "suspend.h"


#ifdef EXIST_IGNORE_OPT_C

#include "snddrv/ignore-opt.c"

#else

T_IGNORE_OPT ignore_opt[] = {  { 0, 0 }, };

#endif


static char *conf_dir_rom  = NULL;	/* -romdir  で指定したパス名	*/
static char *conf_dir_disk = NULL;	/* -diskdir で指定したパス名	*/

char		*dir_tape = NULL;	/* -tapedir で指定したパス名	*/



/*----------------------------------------------------------------------*/

/* エラー表示のマクロ
 *		ERR_MES1( "Opps, Bad data" );
 *		ERR_MES2( "%d is too large", val );
 *	コマンド名と改行が勝手に付加されて、stderr に出力される
 */

static	char	*command = NULL;

#define	ERR_MES1( fmt )						\
		do{						\
		  fprintf( stderr, "%s : %s\n", command, fmt );	\
		}while(0)

#define	ERR_MES2( fmt, arg )					\
		do{						\
		  fprintf( stderr, "%s : ", command );		\
		  fprintf( stderr, fmt, arg );			\
		  fprintf( stderr, "\n" );			\
		}while(0)



/*----------------------------------------------------------------------*/
/* static void help_mess( void )					*/
/*			ヘルプ表示関数					*/
/*----------------------------------------------------------------------*/
#include "help.c"


/*----------------------------------------------------------------------*/
enum {
  OPT_N, OPT_V1S, OPT_V1H, OPT_V2,
  OPT_DIPSW, OPT_BAUDRATE, OPT_ROMBOOT, OPT_DISKBOOT,
  OPT_4MHZ, OPT_8MHZ,
  OPT_SD, OPT_SD2,  
  OPT_EXTRAM, OPT_NOEXTRAM, OPT_JISHO, OPT_NOJISHO,
  OPT_TAPELOAD, OPT_TAPESAVE,
  OPT_PRINTER, OPT_SERIALOUT, OPT_SERIALIN,
  OPT_MOUSE, OPT_NOMOUSE, OPT_JOYMOUSE, OPT_JOYSTICK, OPT_JOYKEY,
  OPT_24K, OPT_15K, OPT_ANALOG, OPT_DIGITAL,
  OPT_FRAMESKIP, OPT_AUTOSKIP, OPT_NOAUTOSKIP, 
  OPT_HALF, OPT_FULL, OPT_DOUBLE, OPT_CMAP, 
  OPT_WIDTH, OPT_HEIGHT,
  OPT_INTERP, OPT_NOINTERP, OPT_INTERLACE, OPT_NOINTERLACE,
  OPT_ROMDIR, OPT_DISKDIR, OPT_TAPEDIR,
  OPT_SHM, OPT_NOSHM, OPT_DGA, OPT_NODGA,
  OPT_HIDE_MOUSE, OPT_GRAB_MOUSE,
  OPT_SHOW_MOUSE, OPT_UNGRAB_MOUSE,
  OPT_COMPATROM, OPT_SETVER,
  OPT_TENKEY, OPT_CURSOR, OPT_NUMLOCK, OPT_MENUKEY, OPT_KANJIKEY,
  OPT_NOTENKEY, OPT_NOCURSOR, OPT_NONUMLOCK,
  OPT_JOYSWAP, OPT_JOYASSIGN,
  OPT_SUBLOAD, OPT_TIMESTOP, OPT_FDC_WAIT, OPT_FDC_NOWAIT,
  OPT_CPU, OPT_CPU1COUNT,
  OPT_CLOCK, OPT_SOUNDCLOCK, OPT_VSYNC,
  OPT_WAITFREQ, OPT_NOWAIT, OPT_LOAD, 
  OPT_HELP, OPT_VERBOSE, OPT_LOGO, OPT_NOLOGO, OPT_ENGLISH, OPT_JAPANESE,
  OPT_FOCUS, OPT_NOFOCUS,
  OPT_SLEEP, OPT_NOSLEEP, OPT_SLEEPPARM, OPT_B2MENU, OPT_NOB2MENU,
  OPT_MENU, OPT_MONITOR, OPT_DEBUG,
  OPT_F6, OPT_F7, OPT_F8, OPT_F9, OPT_F10, OPT_ROMAJI,
  OPT_RESUME, OPT_RESUMEFILE, OPT_RESUMEFORCE,
  OPT_RECORD, OPT_PLAYBACK,
  OPT_FDCDEBUG,
  OPT_EXCHANGE, OPT_NOEXCHANGE, OPT_HSBASIC, OPT_NOHSBASIC,
  OPT_CMT_SPEED, OPT_CMT_INTR, OPT_CMT_POLL, OPT_CMT_WAIT, OPT_CMT_NOWAIT,
  OPT_PCG, OPT_NOPCG,
  EndofOPT
};

static struct {
  int		num;
  const	char	*str;
} opt_index[] =
{
  { OPT_N,         "n",         },
  { OPT_V1S,       "v1s",       },
  { OPT_V1H,       "v1h",       },
  { OPT_V2,        "v2",        },
  { OPT_DIPSW,     "dipsw",     },
  { OPT_BAUDRATE,  "baudrate",	},
  { OPT_ROMBOOT,   "romboot",   },
  { OPT_DISKBOOT,  "diskboot",  },
  { OPT_4MHZ,      "4mhz",      },
  { OPT_8MHZ,      "8mhz",      },
  { OPT_SD,        "sd",        },
  { OPT_SD2,       "sd2",       },
  { OPT_EXTRAM,    "extram",    },
  { OPT_NOEXTRAM,  "noextram",  },
  { OPT_JISHO,     "jisho",     },
  { OPT_NOJISHO,   "nojisho",   },
  { OPT_TAPELOAD,  "tapeload",  },
  { OPT_TAPESAVE,  "tapesave",  },
  { OPT_PRINTER,   "printer",   },
  { OPT_SERIALOUT, "serialout", },
  { OPT_SERIALIN,  "serialin",	},
  { OPT_MOUSE,     "mouse",     },
  { OPT_NOMOUSE,   "nomouse",   },
  { OPT_JOYMOUSE,  "joymouse",  },
  { OPT_JOYSTICK,  "joystick",  },
  { OPT_JOYKEY,    "joykey",    },
  { OPT_24K,       "24k",       },
  { OPT_15K,       "15k",       },
  { OPT_ANALOG,    "analog",    },
  { OPT_DIGITAL,   "digital",   },
  { OPT_FRAMESKIP, "frameskip", },
  { OPT_AUTOSKIP,  "autoskip",  },
  { OPT_NOAUTOSKIP,"noautoskip",},
  { OPT_HALF,      "half",      },
  { OPT_FULL,      "full",      },
  { OPT_DOUBLE,    "double",    },
  { OPT_CMAP,      "cmap",      },
  { OPT_WIDTH,     "width",     },
  { OPT_HEIGHT,    "height",    },
  { OPT_INTERP,    "interp",    },
  { OPT_NOINTERP,  "nointerp",  },
  { OPT_INTERLACE, "interlace",   },
  { OPT_NOINTERLACE,"nointerlace",},
  { OPT_ROMDIR,    "romdir",    },
  { OPT_DISKDIR,   "diskdir",   },
  { OPT_TAPEDIR,   "tapedir",   },
  { OPT_SHM,       "shm",       },
  { OPT_NOSHM,     "noshm",     },
  { OPT_DGA,       "dga",       },
  { OPT_NODGA,     "nodga",     },
  { OPT_HIDE_MOUSE,"hide_mouse",},
  { OPT_SHOW_MOUSE,"show_mouse",},
  { OPT_GRAB_MOUSE,"grab_mouse",},
  { OPT_UNGRAB_MOUSE,"ungrab_mouse",},
  { OPT_COMPATROM, "compatrom", },
  { OPT_SETVER,    "setver",    },
  { OPT_TENKEY,    "tenkey",    },
  { OPT_CURSOR,    "cursor",    },
  { OPT_NUMLOCK,   "numlock",   },
  { OPT_MENUKEY,   "menukey",   },
  { OPT_KANJIKEY,  "kanjikey",	},
  { OPT_NOTENKEY,  "notenkey",  },
  { OPT_NOCURSOR,  "nocursor",  },
  { OPT_NONUMLOCK, "nonumlock", },
  { OPT_JOYSWAP,   "joyswap",   },
  { OPT_JOYASSIGN, "joyassign", },
  { OPT_SUBLOAD,   "subload",   },
  { OPT_TIMESTOP,  "timestop",  },
  { OPT_FDC_WAIT,  "fdc_wait",  },
  { OPT_FDC_NOWAIT,"fdc_nowait",},
  { OPT_CPU,       "cpu",       },
  { OPT_CPU1COUNT, "cpu1count",	},
  { OPT_CLOCK,     "clock",	},
  { OPT_SOUNDCLOCK,"soundclock",},
  { OPT_VSYNC,     "vsync",	},
  { OPT_WAITFREQ,  "waitfreq",	},
  { OPT_NOWAIT,    "nowait",    },
  { OPT_LOAD,      "load",      },
  { OPT_HELP,	   "help",      },
  { OPT_VERBOSE,   "verbose",   },
  { OPT_LOGO,      "logo",      },
  { OPT_NOLOGO,    "nologo",    },
  { OPT_ENGLISH,   "english"    },
  { OPT_JAPANESE,  "japanese"   },
  { OPT_FOCUS,     "focus"      },
  { OPT_NOFOCUS,   "nofocus"    },
  { OPT_SLEEP,     "sleep"      },
  { OPT_NOSLEEP,   "nosleep"    },
  { OPT_SLEEPPARM, "sleepparm"  },
  { OPT_B2MENU,    "button2menu",},
  { OPT_NOB2MENU,  "nobutton2menu",},
  { OPT_MENU,      "menu"       },
  { OPT_MONITOR,   "monitor",   },
  { OPT_DEBUG,     "debug",	},
  { OPT_F6,        "f6",	},
  { OPT_F7,        "f7",	},
  { OPT_F8,        "f8",	},
  { OPT_F9,        "f9",	},
  { OPT_F10,       "f10",	},
  { OPT_ROMAJI,    "romaji",	},
  { OPT_RESUME,    "resume",	},
  { OPT_RESUMEFILE,"resumefile",},
  { OPT_RESUMEFORCE,"resumeforce",},
  { OPT_RECORD,    "record",	},
  { OPT_PLAYBACK,  "playback",	},
  { OPT_FDCDEBUG,  "fdcdebug",  },
  { OPT_EXCHANGE,  "exchange",  },
  { OPT_NOEXCHANGE,"noexchange",},
  { OPT_HSBASIC,   "hsbasic",   },
  { OPT_NOHSBASIC, "nohsbasic", },
  { OPT_CMT_SPEED, "cmt_speed", },
  { OPT_CMT_INTR,  "cmt_intr",  },
  { OPT_CMT_POLL,  "cmt_poll",  },
  { OPT_CMT_WAIT,  "cmt_wait",  },
  { OPT_CMT_NOWAIT,"cmt_nowait",},
  { OPT_PCG,       "pcg",       },
  { OPT_NOPCG,     "nopcg",     },
};



/*----------------------------------------------------------------------*/

	/* ブート時の状態 */

static	int	boot_ver	= -1;		/* Version強制変更 0～9	*/


	/* XMAME サウンドオプションのためのワーク */
static	int	xmame_option_priority;


/*----------------------------------------------------------------------
 * static	int	check_option( char *opt1, char *opt2 )
 *	オプションの処理を行なう
 *		opt1 … 最初の引数 ( '-' で始まる文字列 )
 *		opt2 … 次の引数   ( なければ NULl )
 *	戻り値
 *		0   … 構文エラー
 *		1/2 … 成功(処理した引数の個数)
 *		-1  … オプションじゃない
 *----------------------------------------------------------------------*/

/* 文字列を整数(または実数)に変換。成功かどうかの判定マクロ */

#define	CONV_TO_INT(opt2)	strtol( opt2, &conv_end, 0 )
#define	CONV_TO_DBL(opt2)	strtod( opt2, &conv_end )
#define CONV_ERR()		(*conv_end!='\0')
#define CONV_OK()		(*conv_end=='\0')


/* 文字列を整数に変換し、ワークに格納。エラー時はメッセージを表示し戻る */

#define	SET_OPT2_NUM_TO_WORK( work, low, high, mes_overflow, mes_no_opt )\
		do{							\
		  if( opt2 ){						\
		    ret_val ++;						\
		    work = CONV_TO_INT(opt2);				\
		    if( CONV_ERR() || work < low || high < work ){	\
		      ERR_MES1( mes_overflow );				\
		      return 0;						\
		    }							\
		  }else{						\
		    ERR_MES1( mes_no_opt );				\
		    return 0;						\
		  }							\
		}while(0)

/* 文字列を実数に変換し、ワークに格納。エラー時はメッセージを表示し戻る */

#define	SET_OPT2_DBL_TO_WORK( work, low, high, mes_overflow, mes_no_opt )\
		do{							\
		  if( opt2 ){						\
		    ret_val ++;						\
		    work = CONV_TO_DBL(opt2);				\
		    if( CONV_ERR() || work < low || high < work ){	\
		      ERR_MES1( mes_overflow );				\
		      return 0;						\
		    }							\
		  }else{						\
		    ERR_MES1( mes_no_opt );				\
		    return 0;						\
		  }							\
		}while(0)

/* 文字列用にメモリを確保し、ワークに格納。エラー時はメッセージを表示し戻る */

#define	SET_OPT2_STR_TO_WORK( work, mes_no_opt )			\
		do{							\
		  if( opt2 ){						\
		    ret_val ++;						\
		    if( work ) ex_free( work );				\
		    if( !( work = (char*)ex_malloc( strlen(opt2)+1 ) ) ){	\
		      ERR_MES1( "Malloc Failed!" );			\
		      return 0;						\
		    }else{						\
		      strcpy( work, opt2 );				\
		    }							\
		  }else{						\
		    ERR_MES1( mes_no_opt );				\
		    return 0;						\
		  }							\
		}while(0)

/* -fx オプションの文字列を規定の数値にに変換し、ワークに格納。
   エラー時はメッセージを表示し戻る */

#define	SET_OPT2_FKEY_TO_WORK( work, mes_bad_opt, mes_no_opt )		\
		do{							\
		  if( opt2 ){						\
		    int	zzz;						\
		    ret_val ++;						\
		    for( zzz=1; zzz < FN_end; zzz++ ){			\
		      if( strcmp( opt2, fkey_table[zzz].str ) == 0 ||	\
			  strcmp( opt2, fkey_table[zzz].str2 ) == 0 ){	\
			work = fkey_table[zzz].num;			\
			break;						\
		      }							\
		    }							\
		    if( zzz==FN_end ){					\
		      ERR_MES1( mes_bad_opt );				\
		      return 0;						\
		    }							\
		  }else{						\
		    ERR_MES1( mes_no_opt );				\
		    return 0;						\
		  }							\
		}while(0)

/* オプションを読み飛ばす (サウンドなし時の、サウンドオプションなど)。
   なお、オプションの値の範囲などは見ない */

#define	DUMMY_OPT1( op )						\
		do{							\
		  ERR_MES2( "\"-%s\" is ignored", op );			\
		}while(0)

#define	DUMMY_OPT2( op )						\
		do{							\
		  if( opt2 ){						\
		    ERR_MES2( "\"-%s\" is ignored", op );		\
		    ret_val ++;						\
		  }else{						\
		    ERR_MES2( "\"-%s\" requires an argument", op );	\
		    return 0;						\
		  }							\
		}while(0)



static	int	check_option( char *opt1, char *opt2 )
{
  int	j,  ret_val = 1;
  char *conv_end;


  if( opt1==NULL )     return 0;
  if( opt1[0] != '-' ) return -1;


  for( j=0; j<countof(opt_index); j++ ){
    if( strcmp( &opt1[1], opt_index[j].str )==0 ) break;
  }

  if( j==countof(opt_index) ){

    j = xmame_check_option( opt1, opt2, xmame_option_priority );
    if( j==0 ){

      T_IGNORE_OPT *opt = ignore_opt;

      while( opt->str ){
	if( strcmp( &opt1[1], opt->str )==0 ) break;
	opt ++;
      }
      if( opt->str == 0 ){
	ERR_MES2( "%s ... Wrong option!", opt1 );
	return 0;
      }else{
	if( opt->argc == 1 ){
	  DUMMY_OPT1( opt->str );
	  return 1;
	}
	if( opt->argc == 2 ){
	  DUMMY_OPT2( opt->str );
	  return 2;
	}
	return 0;
      }

    }
    return j;

  }else{

    switch( opt_index[j].num ){

    case OPT_N:		boot_basic = BASIC_N;		break;
    case OPT_V1S:	boot_basic = BASIC_V1S;		break;
    case OPT_V1H:	boot_basic = BASIC_V1H;		break;
    case OPT_V2:	boot_basic = BASIC_V2;		break;

    case OPT_DIPSW:
      SET_OPT2_NUM_TO_WORK( boot_dipsw, 0x0000, 0xffff,
			   "-dipsw <num> ... num = 0..0xffff",
			   "-dipsw ... No dipsw setting supplied" );
      break;
    case OPT_BAUDRATE:
      {
	int i, rate;
	static const int table[] = {
	  75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
	};
	SET_OPT2_NUM_TO_WORK( rate, 75, 19200,
			      "-boudrate <bps> ... bps = "	\
			      "75/150/300/600/1200/2400/4800/9600/19200",
			     "-baudrate ... No baud rate supplied" );
	for( i=0; i<countof(table); i++ ){
	  if( rate == table[i] ){
	    baudrate_sw = i;
	    break;
	  }
	}
	if( i==countof(table) ){
	  ERR_MES1( "-boudrate <bps> ... bps = "	\
		    "75/150/300/600/1200/2400/4800/9600/19200" );
	  return 0;
	}
      }
      break;

    case OPT_ROMBOOT:	boot_from_rom = TRUE;		break;
    case OPT_DISKBOOT:	boot_from_rom = FALSE;		break;

    case OPT_4MHZ:
      boot_clock_4mhz = TRUE;
      cpu_clock_mhz   = CONST_4MHZ_CLOCK;
      break;
    case OPT_8MHZ:
      boot_clock_4mhz = FALSE;
      cpu_clock_mhz   = CONST_8MHZ_CLOCK;
      break;

    case OPT_SD:	sound_board = SOUND_I;		break;
    case OPT_SD2:	sound_board = SOUND_II;		break;

    case OPT_NOEXTRAM:	use_extram = 0;			break;
    case OPT_EXTRAM:
      SET_OPT2_NUM_TO_WORK( use_extram, 0, 8,
			   "-extram <x> ... x = 0..8",
			   "-extram ... No ram-card number supplied" );
      break;

    case OPT_JISHO:	use_jisho_rom = 1;		break;
    case OPT_NOJISHO:	use_jisho_rom = 0;		break;

    case OPT_TAPELOAD:
      SET_OPT2_STR_TO_WORK( file_tapeload, 
			   "-tapeload ... No filename for tape load image");
      break;
    case OPT_TAPESAVE:
      SET_OPT2_STR_TO_WORK( file_tapesave, 
			   "-tapesave ... No filename for tape save image");
      break;

    case OPT_PRINTER:
      SET_OPT2_STR_TO_WORK( file_printer, 
			   "-printer ... No filename for printer output");
      break;
    case OPT_SERIALOUT:
      SET_OPT2_STR_TO_WORK( file_serialout,
			   "-serialout ... No filename for serial output");
      break;
    case OPT_SERIALIN:
      SET_OPT2_STR_TO_WORK( file_serialin,
			   "-serialin ... No filename for serial input");
      break;

    case OPT_MOUSE:	mouse_mode = 1;			break;
    case OPT_NOMOUSE:	mouse_mode = 0;			break;
    case OPT_JOYMOUSE:	mouse_mode = 2;			break;
    case OPT_JOYSTICK:	mouse_mode = 3;			break;
    case OPT_JOYKEY:	mouse_mode = 4;			break;

    case OPT_24K:	monitor_15k = 0x00;		break;
    case OPT_15K:	monitor_15k = 0x02;		break;
    case OPT_ANALOG:	monitor_analog = TRUE;		break;
    case OPT_DIGITAL:	monitor_analog = FALSE;		break;


    case OPT_FRAMESKIP:
      SET_OPT2_NUM_TO_WORK( frameskip_rate, 1, 65536,
			   "-frameskip <period> ... period >= 1",
			   "-frameskip ... No frameskip period supplied" );
      break;

    case OPT_AUTOSKIP:		use_auto_skip = TRUE;		break;
    case OPT_NOAUTOSKIP:	use_auto_skip = FALSE;		break;

    case OPT_HALF:	screen_size = SCREEN_SIZE_HALF;		break;
    case OPT_FULL:	screen_size = SCREEN_SIZE_FULL;		break;
    case OPT_DOUBLE:	screen_size = SCREEN_SIZE_DOUBLE;	break;

    case OPT_CMAP:
      SET_OPT2_NUM_TO_WORK( colormap_type, -1, 2,
			   "-cmap <x> ... x = 0/1/2",
			   "-cmap ... No colormap type supplied" );
      break;

    case OPT_WIDTH:
      SET_OPT2_NUM_TO_WORK( WIDTH, 1, 65536,
			   "-width <w> ... w >= 1",
			   "-width ... No width supplied" );
      WIDTH &= ~7;
      break;
    case OPT_HEIGHT:
      SET_OPT2_NUM_TO_WORK( HEIGHT, 1, 65536,
			   "-width <h> ... h >= 1",
			   "-width ... No height supplied" );
      HEIGHT &= ~1;
      break;

    case OPT_INTERP:	use_half_interp = TRUE;		break;
    case OPT_NOINTERP:	use_half_interp = FALSE;	break;

    case OPT_INTERLACE:		use_interlace = TRUE;	break;
    case OPT_NOINTERLACE:	use_interlace = FALSE;	break;


    case OPT_ROMDIR:
      SET_OPT2_STR_TO_WORK( conf_dir_rom,
			   "-romdir ... No path of rom image file");
      break;
    case OPT_DISKDIR:
      SET_OPT2_STR_TO_WORK( conf_dir_disk,
			   "-diskdir ... No path of disk image file");
      break;
    case OPT_TAPEDIR:
      SET_OPT2_STR_TO_WORK( dir_tape,
			   "-tapedir ... No path of disk image file");
      break;


#ifdef MITSHM
    case OPT_SHM:	use_SHM = TRUE;			break;
    case OPT_NOSHM:	use_SHM = FALSE;		break;
#else
    case OPT_SHM:	DUMMY_OPT1( "-shm" );		break;
    case OPT_NOSHM:	DUMMY_OPT1( "-noshm" );		break;
#endif

#ifdef USE_DGA
    case OPT_DGA:	use_DGA = TRUE;			break;
    case OPT_NODGA:	use_DGA = FALSE;		break;
#else
    case OPT_DGA:	DUMMY_OPT1( "-dga" );		break;
    case OPT_NODGA:	DUMMY_OPT1( "-nodga" );		break;
#endif

    case OPT_HIDE_MOUSE:	hide_mouse = TRUE;	break;
    case OPT_SHOW_MOUSE:	hide_mouse = FALSE;	break;

    case OPT_GRAB_MOUSE:	grab_mouse = TRUE;	break;
    case OPT_UNGRAB_MOUSE:	grab_mouse = FALSE;	break;


    case OPT_COMPATROM:
      SET_OPT2_STR_TO_WORK( file_compatrom,
			   "-compatrom ... No file supplied for compat rom" );
      break;
    case OPT_SETVER:
      SET_OPT2_NUM_TO_WORK( boot_ver, 0, 9, 
			   "-setver <x> ... x = 0..9",
			   "-setver ... No interrupt timing supplied" );
      break;

    case OPT_TENKEY:	tenkey_emu = TRUE;		break;
    case OPT_NOTENKEY:	tenkey_emu = FALSE;		break;

    case OPT_CURSOR:	cursor_emu = TRUE;		break;
    case OPT_NOCURSOR:	cursor_emu = FALSE;		break;

    case OPT_NUMLOCK:	numlock_emu = TRUE;		break;
    case OPT_NONUMLOCK:	numlock_emu = FALSE;		break;

    case OPT_MENUKEY:
      function_f6  = FN_FRATE_UP;
      function_f7  = FN_FRATE_DOWN;
      function_f8  = FN_VOLUME_UP;
      function_f9  = FN_VOLUME_DOWN;
      function_f10 = FN_PAUSE;				break;
    case OPT_KANJIKEY:
      function_f6  = FN_KANA;
      function_f7  = FN_KETTEI;
      function_f8  = FN_HENKAN;
      function_f9  = FN_ZENKAKU;
      function_f10 = FN_ROMAJI;				break;
    case OPT_F6:
      SET_OPT2_FKEY_TO_WORK( function_f6,
			     "-f6 <FUNCTION> ... BAD Function",
			     "-f6 ... No function supplied" );
      break;
    case OPT_F7:
      SET_OPT2_FKEY_TO_WORK( function_f7,
			     "-f7 <FUNCTION> ... BAD Function",
			     "-f7 ... No function supplied" );
      break;
    case OPT_F8:
      SET_OPT2_FKEY_TO_WORK( function_f8,
			     "-f8 <FUNCTION> ... BAD Function",
			     "-f8 ... No function supplied" );
      break;
    case OPT_F9:
      SET_OPT2_FKEY_TO_WORK( function_f9,
			     "-f9 <FUNCTION> ... BAD Function",
			     "-f9 ... No function supplied" );
      break;
    case OPT_F10:
      SET_OPT2_FKEY_TO_WORK( function_f10,
			     "-f10 <FUNCTION> ... BAD Function",
			     "-f10 ... No function supplied" );
      break;

    case OPT_ROMAJI:
      SET_OPT2_NUM_TO_WORK( romaji_type, 0, 2,
			   "-romaji <x> ... x = 0(egg)/1(MS-IME)/2(ATOK)",
			   "-romaji ... No romaji-henkan type supplied" );
      break;

    case OPT_JOYSWAP:	joy_swap_button = TRUE;		break;
    case OPT_JOYASSIGN:
      {
	int tmp;
	SET_OPT2_NUM_TO_WORK( tmp, 0, 4,
			      "-joyassign <x> ... x = 0..4",
			      "-joyassign ... No value supplied" );
	switch( tmp ){
	case 1:	joy_key_assign[0] = JOY88_KEY_X;
		joy_key_assign[1] = JOY88_KEY_Z;	break;
	case 2:	joy_key_assign[0] = JOY88_KEY_SPACE;
		joy_key_assign[1] = JOY88_KEY_RET;	break;
	case 3:	joy_key_assign[0] = JOY88_KEY_SPACE;
		joy_key_assign[1] = JOY88_KEY_SHIFT;	break;
	case 4:	joy_key_assign[0] = JOY88_KEY_SHIFT;
		joy_key_assign[1] = JOY88_KEY_Z;	break;
	default:joy_key_assign[0] = JOY88_KEY_NONE;
		joy_key_assign[1] = JOY88_KEY_NONE;	break;
	}
      }
      break;

    case OPT_SUBLOAD:
      SET_OPT2_NUM_TO_WORK( sub_load_rate, 0, 65536,
			   "-subload <x> ... x = 0..",
			   "-subload ... No value supplied" );
      break;

    case OPT_TIMESTOP:	calendar_stop = TRUE;		break;

    case OPT_FDC_WAIT:	fdc_wait = 1;			break;
    case OPT_FDC_NOWAIT:fdc_wait = 0;			break;

    case OPT_CPU:
      SET_OPT2_NUM_TO_WORK( cpu_timing, -1, 3,
			   "-cpu <x> ... x = 0/1/2/3",
			   "-cpu ... No cpu timing supplied" );
      break;
    case OPT_CPU1COUNT:
      SET_OPT2_NUM_TO_WORK( CPU_1_COUNT, 1, 65536,
			   "-cpu1count <x> ... x = 1..",
			   "-cpu1count ... No -cpu 1 change count supplied" );
      break;

    case OPT_CLOCK:
      SET_OPT2_DBL_TO_WORK( cpu_clock_mhz, 0.001, 65536.0,
			   "-clock <x> ... x[MHz] = 0.1000 .. 100.0000",
			   "-clock ... No cpu frequency rate supplied");
      break;
    case OPT_SOUNDCLOCK:
      SET_OPT2_DBL_TO_WORK( sound_clock_mhz, 0.001, 65536.0,
			   "-soundclock <x> ... x[MHz] = 0.1000 .. 100.0000",
			   "-soundclock ... No cpu frequency rate supplied");
      break;
    case OPT_VSYNC:
      SET_OPT2_DBL_TO_WORK( vsync_freq_hz, 1.0, 240.0,
			   "-vsync <x> ... x[Hz] = 10.0 .. 240.0",
			   "-vsync ... No vsync frequency rate supplied");
      break;

    case OPT_WAITFREQ:
      SET_OPT2_DBL_TO_WORK( wait_freq_hz, 1.0, 240.0,
			   "-waitfreq <x> ... x[Hz] = 10.0 .. 240.0",
			   "-waitfreq ... No frequency supplied" );
      break;
    case OPT_NOWAIT:	no_wait = TRUE;			break;

    case OPT_LOAD:
      SET_OPT2_NUM_TO_WORK( cpu_load, 0, 0xffffff,
			   "-load <x> ... x = 0 .. number of -iperiod(max)",
			   "-load ... No wait supplied");
      break;

    case OPT_HELP:
      help_mess();
      xmame_show_option();
      main_exit(0);

    case OPT_VERBOSE:
      SET_OPT2_NUM_TO_WORK( verbose_level, 0x00, 0xff, 
			   "-verbose <num> ... num = 0..0xff",
			   "-verbose ... No verbose level supplied" );
      break;

    case OPT_LOGO:	bootup_logo = TRUE;		break;
    case OPT_NOLOGO:	bootup_logo = FALSE;		break;

    case OPT_ENGLISH:	menu_lang = LANG_ENGLISH;	break;
    case OPT_JAPANESE:	menu_lang = LANG_JAPAN;		break;

    case OPT_FOCUS:	need_focus = TRUE;		break;
    case OPT_NOFOCUS:	need_focus = FALSE;		break;

    case OPT_SLEEP:	wait_by_sleep = TRUE;		break;
    case OPT_NOSLEEP:	wait_by_sleep = FALSE;		break;
    case OPT_SLEEPPARM:
      SET_OPT2_NUM_TO_WORK( wait_sleep_min_us, 0, 1000000,
			   "-sleepparm <num> ... num = 0..1000000",
			   "-sleepparm ... No sleep-parameter supplied" );

    case OPT_B2MENU:	enable_b2menu = TRUE;		break;
    case OPT_NOB2MENU:	enable_b2menu = FALSE;		break;

    case OPT_MENU:	emu_mode = MENU;		break;

#ifdef	USE_MONITOR
    case OPT_MONITOR:	emu_mode = MONITOR;
			debug_mode = TRUE;		break;
    case OPT_DEBUG:	debug_mode = TRUE;		break;
#else
    case OPT_MONITOR:
    case OPT_DEBUG:					break;
#endif


    case OPT_RESUME:	resume_flag = TRUE;		break;
    case OPT_RESUMEFILE:
      resume_flag = TRUE;
      SET_OPT2_STR_TO_WORK( file_resume,
			   "-resumefile ... No filename of suspended data");
      break;
    case OPT_RESUMEFORCE:
      resume_force = TRUE;
      resume_flag = TRUE;
      SET_OPT2_STR_TO_WORK( file_resume,
			   "-resumeforce ... No filename of suspended data");
      break;

    case OPT_RECORD:
      SET_OPT2_STR_TO_WORK( file_rec,
			   "-record ... No filename for recording");
      break;

    case OPT_PLAYBACK:
      SET_OPT2_STR_TO_WORK( file_pb,
			   "-playback ... No filename for play-back");
      break;

    case OPT_FDCDEBUG:	fdc_debug_mode = TRUE;		break;

    case OPT_EXCHANGE:	disk_exchange = TRUE;		break;
    case OPT_NOEXCHANGE:disk_exchange = FALSE;		break;

    case OPT_HSBASIC:	highspeed_mode = TRUE;		break;
    case OPT_NOHSBASIC:	highspeed_mode = FALSE;		break;

    case OPT_CMT_SPEED:
      SET_OPT2_NUM_TO_WORK( cmt_speed, 0, 0xffff,
			   "-cmt_speed <num> ... num = 0..0xffff",
			   "-cmt_speed ... No speed supplied" );
      break;
    case OPT_CMT_INTR:	cmt_intr = TRUE;		break;
    case OPT_CMT_POLL:	cmt_intr = FALSE;		break;

    case OPT_CMT_WAIT:	cmt_wait = TRUE;		break;
    case OPT_CMT_NOWAIT:cmt_wait = FALSE;		break;

    case OPT_PCG:	use_pcg = TRUE;			break;
    case OPT_NOPCG:	use_pcg = FALSE;		break;

    default:
      ERR_MES1( "Internal Error !" );
      return 0;
    }
  }

  return ret_val;
}







/************************************************************************
 *	エンディアンネスの判定
 *		返り値	TRUE  … 成功
 *			FALSE … 失敗
 *************************************************************************/
static	int	check_endianness( void )
{
  const char *judged = "This machine is %s.";
  const char *please = "Compile again %s in Makefile.\n";

  int	x = 1;

#ifdef LSB_FIRST
  if( *(char *)&x != 1 ){
    ERR_MES1( "!!! CAN'T EXCUTE !!!" );
    ERR_MES2( judged, "Big-Endian" );
    ERR_MES2( please, "comment-out 'LSB = -DLSB_FIRST'" );
    return FALSE;
  }
#else
  if( *(char *)&x == 1 ){
    ERR_MES1( "!!! CAN'T EXCUTE !!!" );
    ERR_MES2( judged, "Little-Endian" );
    ERR_MES2( please, "comment-in 'LSB = -DLSB_FIRST'");
    return FALSE;
  }
#endif

  return TRUE;
}





/************************************************************************
 *	起動時のオプション(引数)の解析
 *		返り値	TRUE  … 成功
 *			FALSE … 失敗
 *		-help オプションをつけると、強制的に終了します。(main_exit())
 *************************************************************************/
static	struct{
  char	*filename;
  int	image_num;
} disk[ NR_DRIVE ];


/* 起動時の引数に、ディスクイメージファイルがあったかどうかの判定マクロ */
#define	exist_argsfile()	(( disk[DRIVE_1].filename ) ? TRUE : FALSE)
/* → 結局、使ってない */

void add_diskimage(char *filename,int image,int Drive)
{
	disk[Drive].filename = filename;
	disk[Drive].image_num = image;
}

char *DiskName(int Drive)
{
	return disk[Drive].filename;
}

static	int	get_option( int argc, char *argv[] )
{
  int	i;
  char	*conv_end;
  int	img, disk_count;


	/* disk[] ワーク初期化 */

  disk_count = 0;
  for( i=0; i<NR_DRIVE; i++ ){
    disk[i].filename  = NULL;
    disk[i].image_num = -1;
  }

	/* 引数を1個づつ順に解析 */

  for( i=1; i<argc; i++ ){

    if( *argv[i]!='-' ){	/* '-' 以外で始まるオプションは、ファイル名 */

      switch( disk_count ){
      case 0:
      case 1:
	disk[ disk_count ].filename = argv[i];
	if( i+1 < argc ){
	  img = CONV_TO_INT( argv[i+1] );
	  if( CONV_OK() ){
	    if( img<1 || img>MAX_NR_IMAGE ){
	      ERR_MES2( "image number wrong ( 1..%d )\n", MAX_NR_IMAGE+1 );
	      return FALSE;
	    }
	    disk[ disk_count ].image_num = img - 1;

	    if( i+2 < argc ){
	      img = CONV_TO_INT( argv[i+2] );
	      if( CONV_OK() ){
		if( img<1 || img>MAX_NR_IMAGE ){
		  ERR_MES2( "image number wrong ( 1..%d )\n", MAX_NR_IMAGE+1 );
		  return FALSE;
		}
		if( disk_count+1 >= NR_DRIVE ){
		  ERR_MES1( " Excessive file image\n" );
		  return FALSE;
		}
		disk[ disk_count+1 ].filename  = argv[i];
		disk[ disk_count+1 ].image_num = img - 1;
		disk_count ++;
		i ++;
	      }

	    }
	    i ++;
	  }
	}
	disk_count ++;
	break;

      default:
	ERR_MES2( "Excessive filename '%s'\n", argv[i] );
	/* return FALSE; */	/* 3個以上ファイル指定しても継続させる */
      }

    }else{			/* '-' で始まる引数は、オプション */

      int cnt  = check_option( argv[i], (i+1<argc) ? argv[i+1] : NULL );
      if( cnt<=0 ) return FALSE;
      else         i += cnt-1;

    }
  }

  return TRUE;
}






/************************************************************************
 *	環境ファイルの解析
 *		返り値	TRUE  … 成功
 *			FALSE … 失敗
 *************************************************************************/

/* 環境ファイル1行あたりの最大文字数 */
#define	MAX_RCFILE_LINE	(256)

static	FILE	*fp_rc = NULL;


/* 環境ファイルの読み込みに成功したかどうかの判定関数 */
int	exist_rcfile( void ){ return ( fp_rc ) ? TRUE : FALSE; }


static	int	get_rcfile( void )
{
  int  line_cnt;
  char line[ MAX_RCFILE_LINE ];
  char parm[ MAX_RCFILE_LINE ];
  int	status;


		/* ${HOME}/.quasi88rc を開く */
		/* なお、エラーが出ても、正常終了する */

  fp_rc = osd_fopen_rcfile( QUASI88RC_FILE, &status );

  if( fp_rc==NULL ){
    if( status==HOME_F_NO_MEM ){
      ERR_MES1( "<warning> Malloc Failed (get rc-file)" );
    }
    return TRUE;
  }



		/* ${HOME}/.quasi88rcファイルを1行づつ解析 */

  line_cnt = 0;

  while( fgets( line, MAX_RCFILE_LINE, fp_rc ) ){
    char *p = &line[0];
    char *q = &parm[0];
    int   argcnt = 0;			/* パラメータは1行あたり、*/
    char *argval[2];			/* 最大で、2個まで        */
    int   result;
    int   esc = FALSE;			/* エスケープシーケンス処理中 */

    line_cnt++;
    *q = '\0';

    while(1){
      while(1){
	if     ( *p=='#' || *p=='\n' || *p=='\0' ) goto BREAK;
	else if( *p==' ' || *p=='\t' ) p++;
	else{
	  if( argcnt==2 ){ argcnt++;              goto BREAK; }
	  else           { argval[argcnt++] = q;  break;      }
	}
      }
      while(1){
	if      (*p=='\n' || *p=='\0')         { *q++ = '\0'; goto BREAK; }
	else if (*p=='#'              && !esc ){ *q++ = '\0'; goto BREAK; }
	else if((*p==' ' || *p=='\t') && !esc ){ *q++ = '\0'; p++; break; }
	else if (*p==QUASI88RC_ESC    && !esc ){         p++; esc=TRUE;   }
	else                                   { *q++ = *p++; esc=FALSE;  }
      }
    }
  BREAK:;


	/* パラメータがなければ次の行へ、あれば解析処理 */

    if      ( argcnt==0 ){			/* パラメータなし    */
      continue;
    }else if( argcnt>2 ){			/* パラメータ3個以上 */
      result = 0;
    }else{					/* パラメータ1～2個  */

#if 0
      if( strncmp( argval[0], "-resume", 7 )==0 ){
	ERR_MES1( "'" QUASI88RC_FILE "' detect option '-resume*'" );
	result = 0;
      }else{
	result = check_option( argval[0], (argcnt==2) ? argval[1] : NULL );
      }
#else				/* 環境ファイルでも -resume 指定可としよう */

	result = check_option( argval[0], (argcnt==2) ? argval[1] : NULL );
#endif
    }

    if( result != argcnt ){		/* エラー時は、エラー行を表示*/
      ERR_MES2( "'" QUASI88RC_FILE "' syntax error in line %d.", line_cnt );
      return FALSE;
    }

  }

  ex_fclose( fp_rc );
  return TRUE;
}















/************************************************************************/
/* 引数の処理								*/
/************************************************************************/
int	config_init( int argc, char *argv[] )
{
  command = argv[0];

  if( ! check_endianness() )       return FALSE;
  xmame_option_priority = 1;				/* XMAME 依存 */
  if( ! get_rcfile() )             return FALSE;
  xmame_option_priority = 2;				/* XMAME 依存 */
  if( ! get_option( argc, argv ) ) return FALSE;

#if 0
  if( resume_flag ){
    if( ! get_resume_file() )      return FALSE;
  }else{
    if( ! get_rcfile() )           return FALSE;
  }

  if( ! get_option( argc, argv ) ) return FALSE;	/* 再度評価 */
#endif


  osd_set_image_dir( conf_dir_rom, conf_dir_disk );	/*イメージパス設定 */

  if( dir_tape == NULL ){
    dir_tape = getenv( "QUASI88_TAPE_DIR" );
  }

  return TRUE;
}








/************************************************************************/
/* 引数で指定されたディスクイメージをセットする				*/
/************************************************************************/
void	disk_set_args_file( void )
{
  int	i;

	/* ディスクの指定無し */

  if( ! disk[DRIVE_1].filename ) return;


	/* DRIVE 1 に対してのみ、ディスクの指定あり */

  if( ! disk[DRIVE_2].filename ){

		/* % quasi88 file          */
		/* % quasi88 file num      */

    if( disk[0].image_num==-1 ){

      disk_insert( DRIVE_1, disk[DRIVE_1].filename, 0 );
      if( disk_image_num_of_drive( DRIVE_1 ) >= 2 ){
	disk_insert( DRIVE_2, disk[DRIVE_1].filename, 1 );
      }

    }else{
      disk_insert( DRIVE_1, disk[DRIVE_1].filename, disk[DRIVE_1].image_num );
    }

  }else{

	/* DRIVE 1 / DRIVE 2 ともに、ディスクの指定あり */

		/* % quasi88 file m m      */
		/* % quasi88 file n file   */
		/* % quasi88 file file m   */
		/* % quasi88 file n file m */

    for( i=0; i<NR_DRIVE; i++ ){
      if( disk[i].image_num < 0 ) disk[i].image_num = 0;
      disk_insert( i, disk[i].filename, disk[i].image_num );
    }

  }
}





/************************************************************************/
/* 各種変数初期化 (引数やPC8801のバージョンによって、変わるもの)	*/
/************************************************************************/
void	bootup_work_init( void )
{

	/* V1モードのバージョンの小数点以下を強制変更する */

  if( boot_ver != -1 ) ROM_VERSION = boot_ver+'0';



	/* 起動デバイス(ROM/DISK)未定の時 */

  if( boot_from_rom==BOOT_AUTO ){		
    if( disk_image_exist(0) ) boot_from_rom = FALSE; /* ディスク挿入時はDISK */
    else                      boot_from_rom = TRUE;  /* それ以外は、    ROM  */
  }


	/* 起動時の BASICモード未定の時	  */

  if( boot_basic==BASIC_AUTO ){			
    if( ROM_VERSION >= '4' )			/* SR 以降は、V2	  */
      boot_basic = BASIC_V2;
    else					/* それ以前は、V1S	  */
      boot_basic = BASIC_V1S;
  }


	/* サウンド(I/II)のポートを設定	 */

  if( sound_board == SOUND_II ){

    if     ( ROM_VERSION >= '8' )		/* FH/MH 以降は、44～47H */
      sound_port = SD_PORT_44_45 | SD_PORT_46_47;
    else if( ROM_VERSION >= '4' )		/* SR 以降は、44～45,A8～ADH */
      sound_port = SD_PORT_44_45 | SD_PORT_A8_AD;
    else					/* それ以前は、  A8～ADH */
      sound_port = SD_PORT_A8_AD;

  }else{

    if( ROM_VERSION >= '4' )			/* SR以降は、44～45H	 */
      sound_port = SD_PORT_44_45;
    else					/* それ以前は、なし	 */
      sound_port = 0;
  }


}
