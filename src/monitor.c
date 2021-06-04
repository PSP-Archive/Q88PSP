/************************************************************************/
/*									*/
/* モニターモード							*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>

#if 0	/* UNIX */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "quasi88.h"
#include "initval.h"
#include "monitor.h"

#include "pc88cpu.h"
#include "pc88main.h"
#include "pc88sub.h"
#include "crtcdmac.h"
#include "memory.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "pio.h"
#include "sound.h"
#include "screen.h"
#include "fdc.h"
#include "joystick.h"

#include "emu.h"
#include "drive.h"
#include "image.h"
#include "file-op.h"
#include "indicator.h"
#include "menu.h"
#include "menu-event.h"
#include "pause.h"
#include "snddrv.h"
#include "wait.h"
#include "suspend.h"
#include "snapshot.h"

#include "basic.h"

#include "monitor-event.h"

#include "exmem.h"

#ifdef USE_GNU_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#ifdef USE_LOCALE
#include <locale.h>
#include <langinfo.h>
#endif /* USE_LOCALE */



/****************************************************************/
/* Ctrl-C で、モニターモードへ移るように設定			*/
/*	ただし、起動時に -debug オプションを指定した時のみ	*/
/****************************************************************/

int	debug_mode	= FALSE;		/* デバッグ機能(モニター)  */

char	alt_char	= 'X';			/* 代替文字 */



/* モニターモード以外の時に SIGINT(Ctrl-C)を受け取ったらモニターモードに移行 */

static	void	sigint_handler( int dummy )
{
  if( emu_mode == PAUSE      ||
      emu_mode == PAUSE_MAIN ){
    pause_quit_signal();
  }
  if( emu_mode == MENU      ||
      emu_mode == MENU_MAIN ){
    menu_quit_signal();
  }
  if( emu_mode != MONITOR      &&
      emu_mode != MONITOR_MAIN ){
    emu_mode = MONITOR;
    set_cpu_dormant( z80main_cpu );
  }
  signal( SIGINT, sigint_handler );
}

/* SIGTERM を受けとったら、終了する */

static	void	sigterm_handler( int dummy )
{
  if( emu_mode == PAUSE      ||
      emu_mode == PAUSE_MAIN ){
    pause_quit_signal();
  }
  if( emu_mode == MENU      ||
      emu_mode == MENU_MAIN ){
    menu_quit_signal();
  }
  emu_mode = QUIT;
  set_cpu_dormant( z80main_cpu );
  set_cpu_dormant( z80sub_cpu );
}

/*-------- 割り込み設定 -------- */

void	set_signal( void )
{
  if( debug_mode ){
    signal( SIGINT,  sigint_handler );
    signal( SIGTERM, sigterm_handler );
  }else{
    signal( SIGINT,  sigterm_handler );
    signal( SIGTERM, sigterm_handler );
  }
}



/****************************************************************/

/*
 *	モニタモードが無効の場合のダミー関数
 */

#ifndef	USE_MONITOR

void	monitor_init( void )
{
  /* この関数は呼ばれないはず */
}

void	monitor_main( void )
{
  /* この関数は呼ばれないはず */
  emu_mode = STEP;
}


/****************************************************************/

#else	/* USE_MONITOR */



/*
 * loadbas、savebas 処理 (peach氏提供)
 */
#include "basic.c"


/*
   help [<cmd>]
   menu
   quit

   go
   trace <steps>|#<steps>|change
   step [call][jp][rep]
   break [<cpu>] [<action>] <addr>|<port> [#<No>]

   read [<bank>] <addr>
   write [<bank>] <addr> <data>
   dump [[<bank>] <addr> [<addr>|#<size>]]
   fill [<bank>] <addr> <addr>|#<size> <data>
   move [<bank>] <addr> <addr>|#<size> [<bank>] <addr>
   search <value> [<bank>] <addr> <addr>|#<size>
   in  [<cpu>] <port>
   out [<cpu>] <port> <data>
   loadmem <filename> <bank> <addr> [<addr|#size>]
   savemem <filename> <bank> <addr> <addr|#size>

   reset [<mode>] [<dipsw>]
   reg [all|<cpu>] [<name> [<value>]]
   disasm [[<cpu>] [addr] [#steps]]
   set  [<variable> [value]]
   show [<variable>]
   redraw
   resize [<size>]

   drive
   drive show
   drive empty [1|2]
   drive eject [1|2]
   drive set <drive_no> <filename> <image_no>

   file show <filename>
   file create <filename>
   file protect <filename> <image_no>
   file unprotect <filename> <image_no>
   file format <filename> <image_no>
   file unformat <filename> <image_no>
   file rename <filename> <image_no> <image_name>
*/


#if	defined( PIO_DISP ) || defined( PIO_FILE )
int	pio_debug = 0;
#endif
#if	defined( FDC_DISP ) || defined( FDC_FILE )
int	fdc_debug = 0;
#endif
#if	defined( MAIN_DISP ) || defined( MAIN_FILE )
int	main_debug = 0;
#endif
#if	defined( SUB_DISP ) || defined( SUB_FILE )
int	sub_debug = 0;
#endif




enum MonitorJob
{
  MONITOR_LINE_INPUT,

  MONITOR_HELP,
  MONITOR_MENU,
  MONITOR_QUIT,

  MONITOR_GO,
  MONITOR_TRACE,
  MONITOR_STEP,
  MONITOR_STEPALL,
  MONITOR_BREAK,

  MONITOR_READ,
  MONITOR_WRITE,
  MONITOR_DUMP,
  MONITOR_DUMPEXT,
  MONITOR_FILL,
  MONITOR_MOVE,
  MONITOR_SEARCH,
  MONITOR_IN,
  MONITOR_OUT,
  MONITOR_LOADMEM,
  MONITOR_SAVEMEM,

  MONITOR_RESET,
  MONITOR_REG,
  MONITOR_DISASM,

  MONITOR_SET,
  MONITOR_SHOW,
  MONITOR_REDRAW,
  MONITOR_RESIZE,
  MONITOR_DRIVE,
  MONITOR_FILE,

  MONITOR_SUSPEND,
  MONITOR_SNAPSHOT,
  MONITOR_LOADFONT,

  MONITOR_MISC,

  MONITOR_FBREAK,
  MONITOR_TEXTSCR,
  MONITOR_LOADBAS,
  MONITOR_SAVEBAS,

  EndofMONITOR
};



/****************************************************************/
/* ヘルプメッセージ表示関数					*/
/****************************************************************/
static	void	help_help( void )
{
  printf
  (
   "  help [<cmd>]\n"
   "    print help\n"
   "    <cmd> ... command for help\n"
   "              [omit]... print short help for all commands.\n"
   );
}
static	void	help_menu( void )
{
  printf
  (
   "  menu\n"
   "    enter menu-mode.\n"
   );
}
static	void	help_quit( void )
{
  printf
  (
   "  quit\n"
   "    quit QUASI88.\n"
   );
}
static	void	help_go( void )
{
  printf
  (
   "  go\n"
   "    execute MAIN and|or SUB program\n"
   );
}
static	void	help_trace( void )
{
  printf
  (
   "  trace [#<steps>|<steps>|change]\n"
   "    execute MAIN ane|or SUB program specityes times\n"
   "    [all omit]        ... trace some steps (previous steps)\n"
   "    #<steps>, <steps> ... step counts of trace  ( you can omit '#' )\n"
   "	change            ... trace while change CPU job. ( main<->sub )\n"
   "                          this is work under condition -cpu 0 or -cpu 1\n"
   );
}
static	void	help_step( void )
{
  printf
  (
   "  step [call][jp][rep]\n"
   "    execute MAIN and|or SUB program 1 time\n"
   "    [all omit] ... execute 1 step\n"
   "    call       ... not trace CALL instruction\n"
   "    jp         ... not trace DJNZ instruction\n"
   "    rep        ... not trace LD*R/CP*R/IN*R/OT*R instruction\n"
   "    CAUTION)\n"
   "         call/jp/rep are work under condition -cpu 0 or -cpu 1\n"
   "         call/jp/rep are use break-point #10.\n"
   );
}
static	void	help_stepall( void )
{
  printf
  (
   "  S\n"
   "    mean 'step all'   (see. step)\n"
   );
}
static	void	help_break( void )
{
  printf
  (
   "  break [<cpu>] [<action>] <addr|port> [#<No>]\n"
   "  break [<cpu>] CLEAR [#<No>]\n"
   "  break\n"
   "    set break point\n"
   "    [all omit]  ... show all break points\n"
   "    <cpu>       ... CPU select MAIN|SUB\n"
   "                    [omit]... select MAIN\n"
   "    <action>    ... set action of conditon PC|READ|WRITE|IN|OUT or CLEAR\n"
   "                    PC    ... break if PC reach addr\n"
   "                    READ  ... break if data is read\n"
   "                    WRITE ... break if data is written\n"
   "                    IN    ... break if data is input\n"
   "                    OUT   ... break if data is output\n"
   "                    CLEAR ... clear all break point\n"
   "                    [omit]... select PC\n"
   "	<addr|port> ... specify address or port\n"
   "                    if <action> is CLEAR, this argument is invalid\n"
   "    #<No>       ... number of break point. (#1..#10)\n"
   "                    #0    ... all break point when <action> is CLEAR\n"
   "                    [omit]... select #1\n"
   "                    CAUTION).. #10 is used by system\n"
   );
}  



static	void	help_read( void )
{
  printf
  (
   "  read [<bank>] <addr>\n"
   "    read memory.\n"
   "    <bank> ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "               [omit]... current memory bank of MAIN.\n"
   "    <addr> ... specify address\n");
}
static	void	help_write( void )
{
  printf
  (
   "  write [<bank>] <addr> <data>\n"
   "    write memory.\n"
   "    <bank> ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "               [omit]... current memory bank of MAIN.\n"
   "    <addr> ... specify address\n"
   "    <data> ... write data\n"
   );
}
static	void	help_dump( void )
{
  printf
  (
   "  dump [<bank>] <start-addr> [<end-addr>]\n"
   "  dump [<bank>] <start-addr> [#<size>]\n"
   "    dump memory.\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                     [omit]... current memory bank of MAIN.\n"
   "    <start-addr> ... dump start address\n"
   "    <end-addr>   ... dump end address\n"
   "                     [omit]... <start-address>+256\n"
   "    #<size>      ... dump size\n"
   "                     [omit]... 256 byte\n"
   );
}
static	void	help_dumpext( void )
{
  printf
  (
   "  dumpext [<bank>] [#<board>] <start-addr> [<end-addr>]\n"
   "  dumpext [<bank>] [#<board>] <start-addr> [#<size>]\n"
   "    dump external ram memory.\n"
   "    <bank>       ... memory bank EXT0|EXT1|EXT2|EXT3\n"
   "                     [omit]... current memory bank of EXT0.\n"
   "    #<board>     ... board number (1..8).\n"
   "                     [omit]... board #1.\n"
   "    <start-addr> ... dump start address(0x0000..0x7fff)\n"
   "    <end-addr>   ... dump end address(0x0000..0x7fff)\n"
   "                     [omit]... <start-address>+256\n"
   "    #<size>      ... dump size\n"
   "                     [omit]... 256 byte\n"
   );
}
static	void	help_fill( void )
{
  printf
  (
   "  fill [<bank>] <start-addr> <end-addr> <value>\n"
   "  fill [<bank>] <start-addr> #<size>    <value>\n"
   "    fill memory by specify value. \n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                     [omit]... current memory bank of MAIN.\n"
   "    <start-addr> ... fill start address\n"
   "    <end-addr>   ... fill end address\n"
   "    #<size>      ... fill size\n"
   "    <value>      ... fill value\n"
   );
}
static	void	help_move( void )
{
  printf
  (
   "  move [<src-bank>] <src-addr> <end-addr> [<dist-bank>] <dist-addr>\n"
   "  move [<src-bank>] <src-addr> #<size>    [<dist-bank>] <dist-addr>\n"
   "    move memory. \n"
   "    <src-bank>  ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                    [omit]... current memory bank of MAIN.\n"
   "    <src-addr>  ... move source start address\n"
   "    <end-addr>  ... move source end   address\n"
   "    #<size>     ... move size\n"
   "    <dist-bank> ... memory bank\n"
   "                    [omit]... same as <src-bank>\n"
   "    <dist-addr> ... move distination address\n"
   );
}
static	void	help_search( void )
{
  printf
  (
   "  search [<value> [[<bank>] <start-addr> <end-addr>]]\n"
   "    search memory. \n"
   "    <value>      ... search value\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                     [omit]... current memory bank of MAIN.\n"
   "    <start-addr> ... search start address\n"
   "    <end-addr>   ... search end address\n"
   "    [omit-all]   ... search previous value or strings\n"
   );
}
static	void	help_in( void )
{
  printf
  (
   "  in [<cpu>] <port>\n"
   "    input I/O port.\n"
   "    <cpu>  ... CPU select MAIN|SUB\n"
   "               [omit]... select MAIN\n"
   "    <port> ... in port address\n"
   );
}
static	void	help_out( void )
{
  printf
  (
   "  out [<cpu>] <port> <data>\n"
   "    output I/O port.\n"
   "    <cpu>  ... CPU select MAIN|SUB\n"
   "               [omit]... select MAIN\n"
   "    <port> ... out port address\n"
   "    <data> ... output data\n"
   );
}
static	void	help_loadmem( void )
{
  printf
  (
   "  loadmem <filename> <bank> <start-addr> [<end-addr>]\n"
   "  loadmem <filename> <bank> <start-addr> [#<size>]\n"
   "    load memory from binary file.\n"
   "    <filename>   ... binary filename.\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "    <start-addr> ... load start addr\n"
   "    <end-addr>   ... load end addr\n"
   "    #<size>      ... load size\n"
   "                     [omit] set filesize as binary size\n"
   );
}
static	void	help_savemem( void )
{
  printf
  (
   "  savemem <filename> <bank> <start-addr> <end-addr>\n"
   "  savemem <filename> <bank> <start-addr> #<size>\n"
   "    save memory image to file.\n"
   "    <filename>   ... filename.\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "    <start-addr> ... save start addr\n"
   "    <end-addr>   ... save end addr\n"
   "    #<size>      ... save size\n"
   );
}
static	void	help_reset( void )
{
  printf
  (
   "  reset [<mode>] [<dipsw>]\n"
   "    reset PC8800 and execute from address 0000H.\n"
   "	<mode>  ... BASIC mode N|V1S|V1H|V2\n"
   "                [omit] select current BASIC mode\n"
   "    <dipsw> ... dip-switch setting \n"
   "                [omit] select current dip-switch setting\n"
   );
}
static	void	help_reg( void )
{
  printf
  (
   "  reg [[<cpu>] [<name> <value>]]\n"
   "    show & set register.\n"
   "    [all omit] ... show all register (MAIN and|or SUB).\n"
   "    <cpu>      ... CPU select MAIN|SUB\n"
   "                   [omit]... select MAIN\n"
   "    <name>     ... specity register name.\n"
   "                   AF|BC|DE|HL|AF'|BC'|DE'|HL'|IX|IY|SP|PC|I|R|IFF|IM\n"
   "    <value>    ... set value\n"
   "                   [omit]... show value of register\n"
   );
}
static	void	help_disasm( void )
{
  printf
  (
   "  disasm [[<cpu>] [<start-addr>][#<steps>]]\n"
   "    disassemble.\n"
   "    [all omit]   ... disasmble 16 steps from MAIN CPU PC address.\n"
   "    <cpu>        ... CPU select MAIN|SUB\n"
   "                     [omit]... select MAIN\n"
   "    <start-addr> ... disassemble start address\n"
   "                     [omit]... reg PC address\n"
   "    #<steps>     ... disassemble steps\n"
   "                     [omit]... 16 steps\n"
   );
}
static	void	help_set( void )
{
  printf
  (
   "  set [[<variabe-name> [<value>]]]\n"
   "    show & set variables.\n"
   "    [all omit]     ... show all variable.\n"
   "    <variabe-name> ... specify variable name.\n"
   "    <value>        ... set value\n"
   "                       [omit]... show value of variable\n"
   );
}
static	void	help_show( void )
{
  printf
  (
   "  show [<variabe-name>]\n"
   "    show variables.\n"
   "    [all omit]     ... show all variable.\n"
   "    <variabe-name> ... specify variable name.\n"
   );
}
static	void	help_redraw( void )
{
  printf
  (
   "  redraw\n"
   "    redraw QUASI88 screen.\n"
   );
}
static	void	help_resize( void )
{
  printf
  (
   "  resize [<screen-size>]\n"
   "    resize screen.\n"
   "    <screen_size> ... screen size FULL|HALF|DOUBLE\n"
   "                      [omit]... change screen size HALF,FULL,DOUBLE...\n"
   );
}
static	void	help_drive( void )
{
  printf
  (
   "  drive\n"
   "  drive show\n"
   "  drive empty [<drive_no>]\n"
   "  drive eject [<drive_no>]\n"
   "  drive set <drive_no> <filename> [<image_no>]\n"
   "    Show drive information, Eject Disk, Set Disk.\n"
   "      drive [show] ... Show now drive information.\n"
   "      drive empty  ... Set/Unset drive <drive_no> empty.\n"
   "      drive eject  ... Eject disk from drive <drive_no>\n"
   "                       <drive_no> omit, eject all disk.\n"
   "      drive set    ... Eject disk and insert new disk\n"
   "        <drive_no> ... 1 | 2  mean  DRIVE 1: | DRIVE 2:\n"
   "        <filename> ... if filename is '-' , disk not change\n"
   "        <image_no> ... image number (1..%d max)\n"
   "                       <image_no> omit, set image number 1.\n"
   , MAX_NR_IMAGE
   );
}
static	void	help_file( void )
{
  printf
  (
   "  file show <filename>\n"
   "  file create <filename>\n"
   "  file protect <filename> <image_no>\n"
   "  file unprotect <filename> <image_no>\n"
   "  file format <filename> <image_no>\n"
   "  file unformat <filename> <image_no>\n"
   "  file rename <filename> <image_no> <image_name>\n"
   "    Disk image file utility.\n"
   "      file show      ... Show file information.\n"
   "      file create    ... Create / Append blank disk image in file.\n"
   "      file protect   ... Set protect.\n"
   "      file unprotect ... Unset protect.\n"
   "      file format    ... format image by N88DISK-BASIC DATA DISK format.\n"
   "      file unformat  ... Unformat image.\n"
   "      file rename    ... Rename disk image.\n"
   "        <filename>   ... filename\n"
   "        <image_no>   ... image number (1..%d max)\n"
   "        <image_name> ... image name (MAX 16chars)\n"
   , MAX_NR_IMAGE
   );
}

static	void	help_suspend( void )
{
  printf
  (
   "  suspend <filename>\n"
   "    suspend QUASI88\n"
   "    <filename> ... specify suspend-image-file filename.\n"
   );
}

static	void	help_snapshot( void )
{
  printf
  (
   "  snapshot\n"
   "    save screen snapshot\n"
   );
}

static	void	help_loadfont( void )
{
  printf
  (
   "  loadfont <filename>\n"
   "    load text-font file\n"
   "    <filename> ... specify text-font-file filename.\n"
   );
}

static	void help_fbreak( void )
{
  printf
  (
   "  fbreak [<action>] <drive> <track> [<sector>] [#<No>]\n"
   "  fbreak CLEAR [#<No>]\n"
   "  fbreak\n"
   "    set fdc break point\n"
   "    [all omit] ... show all break points\n"
   "    <action>   ... set action of conditon READ|WRITE|DIAG or CLEAR\n"
   "                   READ  ... break if fdc command is read\n"
   "                   WRITE ... break if fdc command is write\n"
   "                   DIAG  ... break if fdc command is diag\n"
   "                   CLEAR ... clear all break point\n"
   "                   [omit]... select READ\n"
   "    <drive>    ... specify drive (1 or 2)\n"
   "                   if <action> is CLEAR, this argument is invalid\n"
   "    <track>    ... specify track (0...)\n"
   "    <sector>   ... specify sector (1...)\n"
   "                   [omit]... select all sector\n"
   "    #<No>      ... number of break point. (#1..#10)\n"
   "                   #0    ... all break point when <action> is CLEAR\n"
   "                   [omit]... select #1\n"
   );
}  

static	void help_textscr(void)
{
  printf
  (
   "  textscr [<char>]\n"
   "    print text screen in the console screen\n"
   "    <char> ... alternative character to unprintable one.\n"
   "               [omit] ... use 'X'\n"
   );
}  

static	void help_loadbas(void)
{
  printf
  (
   "  loadbas <filename> [<type>]\n"
   "    load basic list\n"
   "    <filename> ... filename of basic list.\n"
   "    <type>     ... set type of basic list ASCII or BINARY\n"
   "                   ASCII  ... load as text list\n"
   "                   BINARY ... load as intermediate code\n"
   "                   [omit] ... select ASCII\n"
   );
}  

static	void help_savebas(void)
{
  printf
  (
   "  savebas [<filename> [<type>]]\n"
   "    print or save basic list\n"
   "    <filename> ... filename of basic list.\n"
   "    <type>     ... set type of basic list ASCII or BINARY\n"
   "                   ASCII  ... save as text list\n"
   "                   BINARY ... save as intermediate code\n"
   "                   [omit] ... select ASCII\n"
   );
}  

static	void	help_misc(void)
{printf("  misc ... this is for debug. don't mind!\n");}


/****************************************************************/
/* 命令の種類判定テーブル					*/
/****************************************************************/
static struct{
  int	job;
  char	*cmd;
  void	(*help)(void);
  char	*help_mes;
}monitor_cmd[]=
{
{ MONITOR_HELP,    "help",    help_help,    "print help",                  },
{ MONITOR_HELP,    "?",       help_help,    "     ''   ",                  },
{ MONITOR_MENU,    "menu",    help_menu,    "enter menu-mode",             },
{ MONITOR_MENU,    "m",       help_menu,    "     ''        ",             },
{ MONITOR_QUIT,    "quit",    help_quit,    "quit quasi88",                },
{ MONITOR_QUIT,    "q",       help_quit,    "     ''     ",                },
{ MONITOR_GO,      "go",      help_go,      "exec emu",                    },
{ MONITOR_GO,      "g",       help_go,      "    ''  ",                    },
{ MONITOR_TRACE,   "trace",   help_trace,   "trace emu",                   },
{ MONITOR_TRACE,   "t",       help_trace,   "    ''   ",                   },
{ MONITOR_STEP,    "step",    help_step,    "step emu",                    },
{ MONITOR_STEP,    "s",       help_step,    "    ''   ",                   },
{ MONITOR_STEPALL, "S",       help_stepall, "    ''   ",                   },
{ MONITOR_BREAK,   "break",   help_break,   "set break point",             },
{ MONITOR_READ,    "read",    help_read,    "read memory",                 },
{ MONITOR_WRITE,   "write",   help_write,   "write memory ",               },
{ MONITOR_DUMP,    "dump",    help_dump,    "dump memory",                 },
{ MONITOR_DUMPEXT, "dumpext", help_dumpext, "dump external ram memory",    },
{ MONITOR_FILL,    "fill",    help_fill,    "fill memory",                 },
{ MONITOR_MOVE,    "move",    help_move,    "move memory",                 },
{ MONITOR_SEARCH,  "search",  help_search,  "search memory",               },
{ MONITOR_IN,      "in",      help_in,      "input port",                  },
{ MONITOR_OUT,     "out",     help_out,     "output port",                 },
{ MONITOR_LOADMEM, "loadmem", help_loadmem, "load memory from file",       },
{ MONITOR_SAVEMEM, "savemem", help_savemem, "save memory to file",         },
{ MONITOR_RESET,   "reset",   help_reset,   "reset PC8800 system",         },
{ MONITOR_REG,     "reg",     help_reg,     "show/set CPU register",       },
{ MONITOR_DISASM,  "disasm",  help_disasm,  "disassemble",                 },
{ MONITOR_SET,     "set",     help_set,     "show variable",               },
{ MONITOR_SHOW,    "show",    help_show,    "show/set variable",           },
{ MONITOR_REDRAW,  "redraw",  help_redraw,  "redraw screen",               },
{ MONITOR_RESIZE,  "resize",  help_resize,  "resize screen",               },
{ MONITOR_DRIVE,   "drive",   help_drive,   "operate disk drive",          },
{ MONITOR_FILE,    "file",    help_file,    "disk image file utility",     },
{ MONITOR_SUSPEND, "suspend", help_suspend, "suspend QUASI88",             },
{ MONITOR_SNAPSHOT,"snapshot",help_snapshot,"save screen snapshot",        },
{ MONITOR_LOADFONT,"loadfont",help_loadfont,"load text-font file",         },
{ MONITOR_MISC,    "misc",    help_misc,    "for debug",                   },
{ MONITOR_FBREAK,  "fbreak",  help_fbreak,  "set fdc break point",         },
{ MONITOR_TEXTSCR, "textscr", help_textscr, "print text screen",           },
{ MONITOR_LOADBAS, "loadbas", help_loadbas, "load basic list",             },
{ MONITOR_SAVEBAS, "savebas", help_savebas, "save basic list",             },
};



/****************************************************************/
/* 引数の種類判定テーブル					*/
/****************************************************************/

enum ArgvType{
  ARGV_END    = 0x00000,
  ARGV_STR    = 0x00001,		/* strings			*/
  ARGV_PORT   = 0x00002,		/* 0～0xff			*/
  ARGV_ADDR   = 0x00004,		/* 0～0xffff			*/
  ARGV_NUM    = 0x00008,		/* 0～0x7fffffff		*/
  ARGV_INT    = 0x00010,		/* -0x7fffffff～0x7fffffff	*/
  ARGV_DRV    = 0x00020,		/* 1～2				*/
  ARGV_IMG    = 0x00040,		/* 1～MAX_NR_IMAGE		*/
  ARGV_SIZE   = 0x00080,		/* #1～#0x7fffffff		*/
  ARGV_CPU    = 0x00100,		/* CpuName			*/
  ARGV_BANK   = 0x00200,		/* MemoryName			*/
  ARGV_REG    = 0x00400,		/* RegisterName			*/
  ARGV_BREAK  = 0x00800,		/* BreakAction			*/
  ARGV_MODE   = 0x01000,		/* ResetCommand			*/
  ARGV_CHANGE = 0x02000,		/* TraceCommand			*/
  ARGV_STEP   = 0x04000,		/* StepCommand			*/
  ARGV_ALL    = 0x08000,		/* RegCommand			*/
  ARGV_RESIZE = 0x10000,		/* ResizeCommand		*/
  ARGV_FILE   = 0x20000,		/* FileCommand			*/
  ARGV_DRIVE  = 0x40000,		/* DriveCommand			*/
  ARGV_FBREAK = 0x80000,		/* FBreakAction			*/
  ARGV_BASIC  = 0x100000,		/* BasicCodeType		*/

  EndofArgvType
};

enum ArgvName{

  ARG_MAIN,							/* <cpu> */
  ARG_SUB,

  /*ARG_MAIN,*/							/* <bank> */
  ARG_ROM,	ARG_RAM,	ARG_N,		ARG_HIGH,
  ARG_EXT0,	ARG_EXT1,	ARG_EXT2,	ARG_EXT3,
  ARG_B,	ARG_R,		ARG_G,
  /*ARG_SUB*/

  ARG_AF,	ARG_BC,		ARG_DE,		ARG_HL,		/* <reg> */
  ARG_IX,	ARG_IY,		ARG_SP,		ARG_PC,
  ARG_AF1,	ARG_BC1,	ARG_DE1,	ARG_HL1,
  ARG_I,	/*ARG_R,*/
  ARG_IFF,	ARG_IM,		ARG_HALT,

  /*ARG_PC,*/	ARG_READ,	ARG_WRITE,	ARG_IN,		/* <action>*/
  ARG_OUT,	ARG_DIAG,	ARG_CLEAR,

  ARG_V2,	ARG_V1H,	ARG_V1S,	/*ARG_N,*/	/* <mode> */

  ARG_CHANGE,						/* trace change */

  /*ARG_ALL*/
  ARG_CALL,	ARG_JP,		ARG_REP,		/* step <cmd>   */

  ARG_ALL,						/* reg all      */

  ARG_FULL,	ARG_HALF,	ARG_DOUBLE,		/* resize <arg> */

  ARG_SHOW,	ARG_EJECT,				/* drive <cmd>  */
  ARG_EMPTY,	ARG_SET,

  /*ARG_SHOW,*/						/* file <cmd>   */
  ARG_CREATE,	ARG_RENAME,
  ARG_PROTECT,	ARG_UNPROTECT,
  ARG_FORMAT,	ARG_UNFORMAT,

  ARG_BINARY,	ARG_ASCII,				/* savebas <type> */
  
  EndofArgName
};


static	struct{
  char	*str_l;
  char	*str_u;
  int	type;
  int	val;
}monitor_argv[]=
{
  { "main",	"MAIN",		ARGV_CPU,	ARG_MAIN,	}, /* <cpu> */
  { "sub",	"SUB",		ARGV_CPU,	ARG_SUB,	},

  { "main",	"MAIN",		ARGV_BANK,	ARG_MAIN,	}, /* <bank> */
  { "rom",	"ROM",		ARGV_BANK,	ARG_ROM,	},
  { "ram",	"RAM",		ARGV_BANK,	ARG_RAM,	},
  { "n",	"N",		ARGV_BANK,	ARG_N,		},
  { "high",	"HIGH",		ARGV_BANK,	ARG_HIGH,	},
  { "ext0",	"EXT0",		ARGV_BANK,	ARG_EXT0,	},
  { "ext1",	"EXT1",		ARGV_BANK,	ARG_EXT1,	},
  { "ext2",	"EXT2",		ARGV_BANK,	ARG_EXT2,	},
  { "ext3",	"EXT3",		ARGV_BANK,	ARG_EXT3,	},
  { "b",	"B",		ARGV_BANK,	ARG_B,		},
  { "r",	"R",		ARGV_BANK,	ARG_R,		},
  { "g",	"G",		ARGV_BANK,	ARG_G,		},
  { "sub",	"SUB",		ARGV_BANK,	ARG_SUB,	},

  { "af",	"AF",		ARGV_REG,	ARG_AF,		}, /* <reg> */
  { "bc",	"BC",		ARGV_REG,	ARG_BC,		},
  { "de",	"DE",		ARGV_REG,	ARG_DE,		},
  { "hl",	"HL",		ARGV_REG,	ARG_HL,		},
  { "ix",	"IX",		ARGV_REG,	ARG_IX,		},
  { "iy",	"IY",		ARGV_REG,	ARG_IY,		},
  { "sp",	"SP",		ARGV_REG,	ARG_SP,		},
  { "pc",	"PC",		ARGV_REG,	ARG_PC,		},
  { "af'",	"AF'",		ARGV_REG,	ARG_AF1,	},
  { "bc'",	"BC'",		ARGV_REG,	ARG_BC1,	},
  { "de'",	"DE'",		ARGV_REG,	ARG_DE1,	},
  { "hl'",	"HL'",		ARGV_REG,	ARG_HL1,	},
  { "i",	"I",		ARGV_REG,	ARG_I,		},
  { "r",	"R",		ARGV_REG,	ARG_R,		},
  { "iff",	"IFF",		ARGV_REG,	ARG_IFF,	},
  { "im",	"IM",		ARGV_REG,	ARG_IM,		},
  { "halt",	"HALT",		ARGV_REG,	ARG_HALT,	},

  { "pc",	"PC",		ARGV_BREAK,	ARG_PC,		}, /*<action>*/
  { "read",	"READ",		ARGV_BREAK,	ARG_READ,	},
  { "write",	"WRITE",	ARGV_BREAK,	ARG_WRITE,	},
  { "in",	"IN",		ARGV_BREAK,	ARG_IN,		},
  { "out",	"OUT",		ARGV_BREAK,	ARG_OUT,	},
  { "clear",	"CLEAR",	ARGV_BREAK,	ARG_CLEAR,	},

  { "read",	"READ",		ARGV_FBREAK,	ARG_READ,	}, /*<action>*/
  { "write",	"WRITE",	ARGV_FBREAK,	ARG_WRITE,	},
  { "diag",	"DIAG",		ARGV_FBREAK,	ARG_DIAG,	},
  { "clear",	"CLEAR",	ARGV_FBREAK,	ARG_CLEAR,	},

  { "v2",	"V2",		ARGV_MODE,	ARG_V2,		}, /* <mode> */
  { "v1h",	"V1H",		ARGV_MODE,	ARG_V1H,	},
  { "v1s",	"V1S",		ARGV_MODE,	ARG_V1S,	},
  { "n",	"N",		ARGV_MODE,	ARG_N,		},

  { "change",	"CHANGE",	ARGV_CHANGE,	ARG_CHANGE,	}, /* trace */

  { "call",	"CALL",		ARGV_STEP,	ARG_CALL,	}, /* step  */
  { "jp",	"JP",		ARGV_STEP,	ARG_JP,		},
  { "rep",	"REP",		ARGV_STEP,	ARG_REP,	},
  { "all",	"ALL",		ARGV_STEP,	ARG_ALL,	},

  { "all",	"ALL",		ARGV_ALL,	ARG_ALL,	}, /* reg   */

  { "full",	"FULL",		ARGV_RESIZE,	ARG_FULL,	}, /* resize*/
  { "half",	"HALF",		ARGV_RESIZE,	ARG_HALF,	},
  { "double",	"DOUBLE",	ARGV_RESIZE,	ARG_DOUBLE,	},

  { "show",	"SHOW",		ARGV_DRIVE,	ARG_SHOW,	}, /* drive */
  { "eject",	"EJECT",	ARGV_DRIVE,	ARG_EJECT,	},
  { "empty",	"EMPTY",	ARGV_DRIVE,	ARG_EMPTY,	},
  { "set",	"SET",		ARGV_DRIVE,	ARG_SET,	},

  { "show",	"SHOW",		ARGV_FILE,	ARG_SHOW,	}, /* file  */
  { "create",	"CREATE",	ARGV_FILE,	ARG_CREATE,	},
  { "rename",	"RENAME",	ARGV_FILE,	ARG_RENAME,	},
  { "protect",	"PROTECT",	ARGV_FILE,	ARG_PROTECT,	},
  { "unprotect","UNPROTECT",	ARGV_FILE,	ARG_UNPROTECT,	},
  { "format",	"FORMAT",	ARGV_FILE,	ARG_FORMAT,	},
  { "unformat",	"UNFORMAT",	ARGV_FILE,	ARG_UNFORMAT,	},

  { "binary",	"BINARY",	ARGV_BASIC,	ARG_BINARY,	}, /*savebas*/
  { "ascii",	"ASCII",	ARGV_BASIC,	ARG_ASCII,	},
};



enum SetType
{
  MTYPE_NEWLINE,	MTYPE_INT,	MTYPE_BYTE,	MTYPE_WORD,
  MTYPE_KEY,		MTYPE_PALETTE,	MTYPE_CRTC,	MTYPE_PIO,
  MTYPE_CURSOR,		MTYPE_SIZE,	MTYPE_MOUSE,	MTYPE_FRAMESKIP,
  MTYPE_DOUBLE,		MTYPE_CLOCK,
  MTYPE_VOLUME,		MTYPE_FMMIXER,	MTYPE_PSGMIXER,	MTYPE_BEEPMIXER,
  MTYPE_RHYTHMMIXER,	MTYPE_ADPCMMIXER,	MTYPE_MIXER,
  EndofTYPE
};
static struct{
  char	*var_name;
  char	*port_mes;
  int	var_type;
  void	*var_ptr;
}monitor_variable[]=
{
{ "boot_dipsw",		"(boot:3031)",	MTYPE_INT,	&boot_dipsw,	    },
{ "boot_from_rom",	"(boot:40>>3)",	MTYPE_INT,	&boot_from_rom,	    },
{ "boot_clock_4mhz",	"(boot:6E>>7)",	MTYPE_INT,	&boot_clock_4mhz,   },
{ "boot_basic",		"(boot:3031)",	MTYPE_INT,	&boot_basic,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "dipsw_1",		"(IN:30)",	MTYPE_BYTE,	&dipsw_1,	    },
{ "dipsw_2",		"(IN:31)",	MTYPE_BYTE,	&dipsw_2,	    },
{ "ctrl_boot",		"(IN:40&08)",	MTYPE_BYTE,	&ctrl_boot,	    },
{ "ctrl_vrtc",		"(IN:40>>5)",	MTYPE_INT,	&ctrl_vrtc,	    },
{ "memory_bank",	"(IO:5C-5F)",	MTYPE_INT,	&memory_bank,       },
{ "cpu_clock",		"(IN:6E)",	MTYPE_BYTE,	&cpu_clock,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "sys_ctrl",		"(OUT:30)",	MTYPE_BYTE,	&sys_ctrl,	    },
{ "grph_ctrl",		"(OUT:31)",	MTYPE_BYTE,	&grph_ctrl,	    },
{ "misc_ctrl",		"(IO:32)",	MTYPE_BYTE,	&misc_ctrl,	    },
{ "ALU1_ctrl",		"(OUT:34)",	MTYPE_BYTE,	&ALU1_ctrl,	    },
{ "ALU2_ctrl",		"(OUT:35)",	MTYPE_BYTE,	&ALU2_ctrl,	    },
{ "grph_pile",		"(OUT:53)",	MTYPE_BYTE,	&grph_pile,	    },
{ "baudrate_sw",	"(IO:6F)",	MTYPE_BYTE,	&baudrate_sw,	    },
{ "window_offset",      "(OUT:70)",     MTYPE_WORD,	&window_offset,     },
{ "ext_rom_bank",	"(IO:71)",	MTYPE_BYTE,	&ext_rom_bank,	    },
{ "ext_ram_ctrl",	"(IO:E2)",	MTYPE_BYTE,	&ext_ram_ctrl,	    },
{ "ext_ram_bank",	"(IO:E3)",	MTYPE_BYTE,	&ext_ram_bank,	    },
{ "jisho_rom_bank",	"(OUT:F0)",	MTYPE_BYTE,	&jisho_rom_bank,    },
{ "jisho_rom_ctrl",	"(OUT:F1)",	MTYPE_BYTE,	&jisho_rom_ctrl,    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
  
{ "intr_level",		"(OUT:E4&07)",	MTYPE_INT,	&intr_level,	    },
{ "intr_priority",	"(OUT:E4&08)",	MTYPE_INT,	&intr_priority,	    },
{ "intr_sio_enable",	"(OUT:E6&04)",	MTYPE_INT,	&intr_sio_enable,   },
{ "intr_vsync_enable",	"(OUT:E6&02)",	MTYPE_INT,	&intr_vsync_enable, },
{ "intr_rtc_enable",	"(OUT:E6&01)",	MTYPE_INT,	&intr_rtc_enable,   },
{ "intr_sound_enable",	"(IO:32AA&08)",	MTYPE_INT,	&intr_sound_enable, },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "text_display",	"",		MTYPE_INT,	&text_display,      },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "sound_ENABLE_A",	"(sd[27],[29])",MTYPE_INT,	&sound_ENABLE_A,    },
{ "sound_ENABLE_B",	"(sd[27],[29])",MTYPE_INT,	&sound_ENABLE_B,    },
{ "sound_TIMER_A",	"(sd[2425])",	MTYPE_INT,	&sound_TIMER_A,	    },
{ "sound_TIMER_B",	"(sd[26])",	MTYPE_INT,	&sound_TIMER_B,	    },
{ "sound_reg[27]",	"",		MTYPE_BYTE,	&sound_reg[0x27],   },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "sound2_EN_EOS",	"(sd[29])",	MTYPE_INT,	&sound2_EN_EOS,     },
{ "sound2_EN_BRDY",	"(sd[29])",	MTYPE_INT,	&sound2_EN_BRDY,    },
{ "sound2_EN_ZERO",	"(sd[29])",	MTYPE_INT,	&sound2_EN_ZERO,    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "fdc_wait",		"",		MTYPE_INT,	&fdc_wait,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "key",		"(IN:00..0F)",	MTYPE_KEY,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "palette",		"(OUT:5254..5B)",MTYPE_PALETTE,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "crtc",		"",		MTYPE_CRTC,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "pio",		"(IO:FC..FF)",	MTYPE_PIO,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

#ifdef	USE_SOUND
{ "volume",		"(-vol)",	MTYPE_VOLUME,	NULL,		    },
{ "fm-mixer",		"(-fmvol)",	MTYPE_FMMIXER,	NULL,		    },
{ "psg-mixer",		"(-psgvol)",	MTYPE_PSGMIXER,	NULL,		    },
{ "beep-mixer",		"(-beepvol)",	MTYPE_BEEPMIXER,NULL,		    },
{ "rhythm-mixer",	"(-rhythmvol)",	MTYPE_RHYTHMMIXER, NULL,	    },
{ "adpcm-mixer",	"(-adpcmvol)",	MTYPE_ADPCMMIXER,  NULL,	    },
{ "",			"",		MTYPE_MIXER,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
#endif

{ "frameskip_rate",	"(-frameskip)",	MTYPE_FRAMESKIP,&frameskip_rate,    },
{ "tenkey_emu",		"(-tenkey)",	MTYPE_INT,	&tenkey_emu,	    },
{ "cursor_emu",		"(-cursor)",	MTYPE_CURSOR,	&cursor_emu,	    },
{ "screen_size",	"(-full/...)",	MTYPE_SIZE,	&screen_size,	    },
{ "hide_mouse",		"(-hide_mouse)",MTYPE_MOUSE,	&hide_mouse         },
{ "sound_board",	"(-sd)",	MTYPE_INT,	&sound_board,	    },
{ "mouse_mode",		"(-mouse)",	MTYPE_INT,	&mouse_mode,	    },
{ "cpu_timing",		"(-cpu)",	MTYPE_INT,	&cpu_timing,	    },
{ "menu_lang",		"(-english)",	MTYPE_INT,	&menu_lang,	    },
{ "function_f6",	"(-f6)",	MTYPE_INT,	&function_f6,       },
{ "function_f7",	"(-f7)",	MTYPE_INT,	&function_f7,       },
{ "function_f8",	"(-f8)",	MTYPE_INT,	&function_f8,       },
{ "function_f9",	"(-f9)",	MTYPE_INT,	&function_f9,       },
{ "function_f10",	"(-f10)",	MTYPE_INT,	&function_f10,      },
{ "cpu_load",		"(-load)",	MTYPE_INT,	&cpu_load,	    },
{ "sub_load_rate",	"(-subload)",	MTYPE_INT,	&sub_load_rate,	    },
{ "select_main_cpu",	"",		MTYPE_INT,	&select_main_cpu,   },
{ "dual_cpu_count",	"",		MTYPE_INT,	&dual_cpu_count,    },
{ "CPU_1_COUNT",	"",		MTYPE_INT,	&CPU_1_COUNT,	    },
{ "wait_by_sleep",	"(-sleep)",	MTYPE_INT,	&wait_by_sleep,	    },
{ "wait_sleep_min_us",	"(-sleepparm)",	MTYPE_INT,	&wait_sleep_min_us, },
{ "wait_freq_hz",	"(-waitfreq)",	MTYPE_DOUBLE,	&wait_freq_hz,	    },
{ "no_wait",		"(-no_wait)",	MTYPE_INT,	&no_wait,	    },
{ "cpu_clock_mhz",	"(-clock)",	MTYPE_CLOCK,	&cpu_clock_mhz,	    },
{ "sound_clock_mhz",	"(-soundclock)",MTYPE_CLOCK,	&sound_clock_mhz,   },
{ "vsync_freq_hz",	"(-vsync)",	MTYPE_CLOCK,	&vsync_freq_hz,     },
{ "verbose_proc",	"(-verbose&01)",MTYPE_INT,	&verbose_proc,	    },
{ "verbose_z80",	"(-verbose&02)",MTYPE_INT,	&verbose_z80,       },
{ "verbose_io",		"(-verbose&04)",MTYPE_INT,	&verbose_io,	    },
{ "verbose_pio",	"(-verbose&08)",MTYPE_INT,	&verbose_pio,	    },
{ "verbose_fdc",	"(-verbose&10)",MTYPE_INT,	&verbose_fdc,	    },
{ "verbose_wait",	"(-verbose&20)",MTYPE_INT,	&verbose_wait,	    },
{ "verbose_suspend",	"(-verbose&40)",MTYPE_INT,	&verbose_suspend,   },
{ "verbose_snd",	"(-verbose&80)",MTYPE_INT,	&verbose_snd,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

#if	defined( PIO_DISP ) || defined( PIO_FILE )
{ "pio_debug",		"for debug",    MTYPE_INT,	&pio_debug,	    },
#endif
#if	defined( FDC_DISP ) || defined( FDC_FILE )
{ "fdc_debug",		"for debug",    MTYPE_INT,	&fdc_debug,	    },
#endif
#if	defined( MAIN_DISP ) || defined( MAIN_FILE )
{ "main_debug",		"for debug",    MTYPE_INT,	&main_debug,	    },
#endif
#if	defined( SUB_DISP ) || defined( SUB_FILE )
{ "sub_debug",		"for debug",    MTYPE_INT,	&sub_debug,	    },
#endif

};



/*--------------------------------------------------------------*/
/* メモリ READ/WRITE 関数					*/
/*--------------------------------------------------------------*/
static	byte	peek_memory( int bank, word addr )
{
  int	verbose_save;
  byte	wk;

  switch( bank ){
  case ARG_MAIN:
    return main_mem_read(addr);
  case ARG_ROM:
    if( addr<0x8000 ) return main_rom[addr];
    else              return main_mem_read(addr);
  case ARG_RAM:
    if( 0xf000<=addr && high_mode ) return main_high_ram[addr-0xf000];
    else                            return main_ram[addr];
  case ARG_N:
    if( addr<0x8000 ) return main_rom_n[addr];
    else              return main_mem_read(addr);
  case ARG_EXT0:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[0][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_EXT1:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[1][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_EXT2:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[2][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_EXT3:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[3][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_B:
    if( 0xc000<=addr ) return main_vram[addr-0xc000][0];
    else               return main_mem_read(addr);
  case ARG_R:
    if( 0xc000<=addr ) return main_vram[addr-0xc000][1];
    else               return main_mem_read(addr);
  case ARG_G:
    if( 0xc000<=addr ) return main_vram[addr-0xc000][2];
    else               return main_mem_read(addr);
  case ARG_HIGH:
    if( 0xf000<=addr ){
      if( high_mode )  return main_ram[addr];
      else             return main_high_ram[addr-0xf000];
    }else              return main_mem_read(addr);
  case ARG_SUB:
    verbose_save = verbose_io;
    verbose_io = 0;
    wk = sub_mem_read(addr);
    verbose_io = verbose_io;
    return wk;
  }
  return 0xff;
}
static	void	poke_memory( int bank, word addr, byte data )
{
  int	verbose_save;

  switch( bank ){
  case ARG_MAIN:
    main_mem_write(addr,data);
    return;
  case ARG_ROM:
    if( addr<0x8000 ) main_rom[addr] = data;
    else              main_mem_write(addr,data);
    return;
  case ARG_RAM:
    if( 0xf000<=addr && high_mode ) main_high_ram[addr-0xf000] = data;
    else                            main_ram[addr] = data;
    return;
  case ARG_N:
    if( addr<0x8000 ) main_rom_n[addr] = data;
    else              main_mem_write(addr,data);
    return;
  case ARG_EXT0:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[0][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_EXT1:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[1][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_EXT2:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[2][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_EXT3:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[3][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_B:
    if( 0xc000<=addr ) main_vram[addr-0xc000][0] = data;
    else               main_mem_write(addr,data);
    return;
  case ARG_R:
    if( 0xc000<=addr ) main_vram[addr-0xc000][1] = data;
    else               main_mem_write(addr,data);
    return;
  case ARG_G:
    if( 0xc000<=addr ) main_vram[addr-0xc000][2] = data;
    else               main_mem_write(addr,data);
    return;
  case ARG_HIGH:
    if( 0xf000<=addr ){
      if( high_mode )  main_ram[addr] = data;
      else             main_high_ram[addr-0xf000] = data;
    }else              main_mem_write(addr,data);
    return;
  case ARG_SUB:
    verbose_save = verbose_io;
    verbose_io = 0;
    sub_mem_write(addr,data);
    verbose_io = verbose_io;
    return;
  }
}








/*==============================================================*/
/* 引数処理							*/
/*==============================================================*/

/*
 * buf[] の文字列から、単語を取り出す。区切りは、SPC と TAB。
 * 取り出した各単語の先頭アドレスが、*d_argv[] に格納される。
 * 単語は最大 MAX_ARGS 個取り出す。単語の数は、d_argc にセット。
 * 単語の数が MAX_ARGS よりも多い時は、d_argc に MAX_ARGS+1 をセット、
 * この時、MAX_ARGS 個までは、*d_argv[] が格納されている。
 */

#define MAX_ARGS	(8)
#define	MAX_CHRS	(256)

static	char	buf[MAX_CHRS];
static	int	d_argc;
static	char	*d_argv[MAX_ARGS];

static	int	argv_counter;

static	void	getarg( void )
{
  char	*p = &buf[0];

  argv_counter = 1;

  d_argc = 0;
  while(1){

    while(1){
      if     ( *p=='\n' || *p=='\0' ) goto EXIT_GETARG;
      else if( *p==' '  || *p=='\t' ) p++;
      else {
	if( d_argc==MAX_ARGS )       { d_argc++;            goto EXIT_GETARG; }
	else                         { d_argv[d_argc++]=p;  break; }
      }
    }
    while(1){
      if     ( *p=='\n' || *p=='\0' ){ *p   = '\0';  goto EXIT_GETARG; }
      else if( *p==' '  || *p=='\t' ){ *p++ = '\0';  break; }
      else                           {  p++; }
    }

  }
 EXIT_GETARG:;

  return;
}


/*
 * getarg() により、main()の引数と同じような形式で、int d_argc, char *d_argv[]
 * が設定されるが、これをもう少し簡単に処理したいので、shift() 関数を用意した。
 *
 * shift() 関数を呼ぶと、一番最初の引数が解析され、その結果が argv ワークに
 * 格納される。この後で、argv.type をチェックすれば、その引数の種類が、
 * argv.val をチェックすれば、その引数の値がわかる。
 *
 * shift() 関数により、引数が見かけ上一つずつ前にずれていく。
 * ゆえに、shift() 関数を連続して呼べば、常に次の引数が解析される。
 *
 *   shift();
 *   if( argv.type == XXX ){ 処理() };
 *   shift();
 *   if( argv.type == YYY ){ 処理() };
 *   ...
 */

static struct{
  int	type;			/* 引数の種類	ARGV_xxx	   */
  int	val;			/* 引数の値	ARG_xxx または、数 */
  char	*str;			/* 引数の文字列	d_argv[xxx]と同じ  */
} argv;


static	void	shift( void )
{
  int	i, size = FALSE;
  char	*p, *chk;


  if( argv_counter > MAX_ARGS ||		/* これ以上引数が無い */
      argv_counter >= d_argc ){

    argv.type = ARGV_END;

  }else{					/* まだ引数があるので解析 */

    p = d_argv[ argv_counter ];
    if( *p == '#' ){ size = TRUE; p++; }

    argv.type = 0;
    argv.val  = strtol( p, &chk, 0 );
    argv.str  = d_argv[ argv_counter ];

    if( p!=chk && *chk=='\0' ){				/* 数値の場合 */

      if( size ){						/* #で始まる */
	if( argv.val <= 0 ) argv.type = ARGV_STR;
	else                argv.type = ARGV_SIZE;
      }else{							/*数で始まる */
	argv.type |= ARGV_INT;
	if( argv.val >= 0 )      argv.type |= ARGV_NUM;
	if( argv.val <= 0xff )   argv.type |= ARGV_PORT;
	if( argv.val <= 0xffff ) argv.type |= ARGV_ADDR;
	if( 1<= argv.val && argv.val <= 2 )            argv.type |= ARGV_DRV;
	if( 1<= argv.val && argv.val <= MAX_NR_IMAGE ) argv.type |= ARGV_IMG;
      }

    }else{						/* 文字列の場合 */

      if( size ){						/* #で始まる */
	argv.type = ARGV_STR;
      }else{							/*字で始まる */
	for( i=0; i<countof( monitor_argv ); i++ ){
	  if( strcmp( p, monitor_argv[i].str_l )==0 ||
	      strcmp( p, monitor_argv[i].str_u )==0 ){
	    argv.type |= monitor_argv[i].type;
	    argv.val   = monitor_argv[i].val;
	  }
	}
	if( argv.type == 0 ) argv.type = ARGV_STR;
      }

    }

    argv_counter ++;

  }
}


/* shift() した結果、引数が設定されたかどうかをチェック */

#define	exist_argv()	(argv.type)

/* shift() した結果、処理された引数の種類をチェック */

#define	argv_is( x )	(argv.type & (x))


/* 引数の値 (ARG_xxx) から、引数の文字列 (大文字) を得る */

INLINE	char	*argv2str( int argv_val )
{
  int	i;

  for( i=0; i<countof( monitor_argv ); i++ ){
    if( argv_val == monitor_argv[i].val ) return monitor_argv[i].str_u;
  }
  return "";
}




/*==============================================================*/
/* エラー関連							*/
/*==============================================================*/
#define error()							\
	do{							\
	  printf("Invalid argument (arg %d)\n",argv_counter);	\
	  return;						\
	} while(0)




/*==============================================================*/
/* テキストスクリーン関連			by peach	*/
/*==============================================================*/
#define PUT_JIS_IN(fp)	fprintf(fp, "%c%c%c", 0x1b, 0x28, 0x49);
#define PUT_JIS_OUT(fp)	fprintf(fp, "%c%c%c", 0x1b, 0x28, 0x42);

enum {LANG_EUC, LANG_JIS, LANG_SJIS};

int lang = -1;

static void set_lang(void)
{
    /*char *p;*/

    if (lang >= 0) return;

#ifdef USE_LOCALE
    setlocale(LC_ALL, "");
    p = nl_langinfo(CODESET);
    if (strncmp(p, "JIS_", 4) == 0) {
	lang = LANG_JIS;
    } else if (strncmp(p, "SHIFT_JIS", 9) == 0) {
	lang = LANG_SJIS;
    } else {			/* EUC_JP (default)*/
	lang = LANG_EUC;
    }
#else
    lang = LANG_EUC;
#endif
}

void print_hankaku(FILE *fp, Uchar *str, char ach)
{
    Uchar *ptr;

    /* 標準出力じゃないならそのまま */
    if (fp != stdout) {
	fprintf(fp, "%s", str);
	return;
    }

    ptr = str;
    set_lang();

    if (lang == LANG_JIS) PUT_JIS_IN(fp);
    while (*ptr != '\0') {
	if (*ptr == '\n') {
	    if (lang == LANG_JIS) {
		PUT_JIS_OUT(fp);
		fputc('\n', fp);
		PUT_JIS_IN(fp);
	    } else {
		fputc('\n', fp);
	    }
	} else if (0xa1u <= *ptr && *ptr <= 0xdfu) {
	    switch (lang) {
	    case LANG_EUC:
		fputc(0x8eu, fp);	fputc(*ptr, fp);	break;
	    case LANG_JIS:
		fputc(*ptr - 0x80u, fp);	break;
	    case LANG_SJIS:
		fputc(*ptr, fp);	break;
	    }
	} else if (isprint(*ptr)) {
	    fputc(*ptr, fp);
	} else {		/* 表示不能 */
	    fputc(ach, fp);
	    /*fprintf(fp, "0x%x", *ptr);*/
	}
	ptr++;
    }
    if (lang == LANG_JIS) PUT_JIS_OUT(fp);
}


/****************************************************************/
/* 命令別処理							*/
/****************************************************************/

/*--------------------------------------------------------------*/
/* help [<cmd>]							*/
/*	ヘルプを表示する					*/
/*--------------------------------------------------------------*/
static	void	monitor_help( void )
{
  int	i;
  char *cmd = NULL;

  if( exist_argv() ){					/* [cmd] */
    cmd = argv.str;
    shift();
  }
  if( exist_argv() ) error();



  if( cmd==NULL ){			/* 引数なし。全ヘルプ表示 */

    printf("help\n");
    for( i=0; i<countof(monitor_cmd); i++ ){
      printf("  %-7s %s\n", monitor_cmd[i].cmd, monitor_cmd[i].help_mes);
    }
    printf("     Note: type \"help <command-name>\" for more details.\n");

  }else{				/* 引数のコマンドのヘルプ表示 */

    for( i=0; i<countof(monitor_cmd); i++ ){
      if( strcmp( cmd, monitor_cmd[i].cmd )==0 ) break;
    }
    if( i==countof(monitor_cmd) ) error();
    (monitor_cmd[i].help)();

  }

}


/*--------------------------------------------------------------*/
/* dump [<bank>] <start-addr> [<end-addr>]			*/
/* dump [<bank>] <start-addr> [#<size>]				*/
/*	メモリダンプを表示する					*/
/*--------------------------------------------------------------*/
static	int	save_dump_addr = -1;
static	int	save_dump_bank = ARG_MAIN;
static	void	monitor_dump( void )
{
  int	i, j;
  byte	c;
  int	bank  = save_dump_bank;
  int	start = save_dump_addr;
  int	size  = 256;


  if( !exist_argv() ){
    if( save_dump_addr == -1 ) error();
    /* else   skip */
  }else{

    if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
      bank = argv.val;
      shift();
    }

    if( !argv_is( ARGV_ADDR ) ) error();		/* <addr> */
    start = argv.val;
    shift();						

    if     ( !exist_argv() )	    /* skip */;		/* [<addr|#size>] */
    else if( argv_is( ARGV_SIZE ) ) size = argv.val;
    else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
    else                            error();
    shift();
  }

  if( exist_argv() ) error();


	/*================*/

  save_dump_addr = start + size;		/* 毎回ダンプしたアドレスを */
  save_dump_bank = bank;			/* 覚えておく (連続ダンプ用)*/

  size = ( size+15 ) /16;

  printf("addr : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");
  printf("---- : -----------------------------------------------\n");
  for( i=0; i<size; i++ ){
    printf("%04X : ",(start+i*16)&0xffff);
    for( j=0; j<16; j++ ){
      printf("%02X ", peek_memory( bank, start+i*16+j ) );
    }
    printf("|");
    for( j=0; j<16; j++ ){
      c = peek_memory( bank, start+i*16+j );
      if( !isprint( c ) ) c = '.';
      printf("%c", c );
    }
    printf("|\n");
  }
  printf("\n");

  return;
}

/*----------------------------------------------------------------------*/
/* dumpext [<bank>] [<board>] <start-addr> [<end-addr>]			*/
/* dumpext [<bank>] [<board>] <start-addr> [#<size>]			*/
/*	拡張RAMのメモリダンプを表示する					*/
/*				この機能は peach氏により実装されました	*/
/*----------------------------------------------------------------------*/
static	int	save_dumpext_addr = -1;
static	int	save_dumpext_bank = ARG_EXT0;
static	int	save_dumpext_board = 1;
static	void	monitor_dumpext( void )
{
  int	i, j;
  byte	c;
  int	bank  = save_dumpext_bank;
  int	start = save_dumpext_addr;
  int	board = save_dumpext_board;
  int	size  = 256;


  if( !exist_argv() ){
    if( save_dumpext_addr == -1 ) error();
    /* else   skip */
  }else{

    if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
      bank = argv.val;
      if ( bank < ARG_EXT0 || ARG_EXT3 < bank ) error();
      shift();
    }

    if( argv_is( ARGV_SIZE ) ) {			/* [#<board>] */
      board = argv.val;
      if ( board < 1 || use_extram < board ) error();
      shift();
    }

    if( !argv_is( ARGV_ADDR ) ) error();		/* <addr> */
    start = argv.val;
    if ( start >= 0x8000 ) error();
    shift();						

    if     ( !exist_argv() )	    /* skip */;		/* [<addr|#size>] */
    else if( argv_is( ARGV_SIZE ) ) size = argv.val;
    else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
    else                            error();
    if ( start + size >= 0x8000 ) error();
    shift();
  }

  if( exist_argv() ) error();


	/*================*/

  save_dumpext_addr  = start + size;		/* 毎回ダンプしたアドレスを */
  save_dumpext_bank  = bank;			/* 覚えておく (連続ダンプ用)*/
  save_dumpext_board = board;

  size = ( size+15 ) /16;
  bank = bank - ARG_EXT0 + (board - 1) * 4;

  printf("addr : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");
  printf("---- : -----------------------------------------------\n");
  for( i=0; i<size; i++ ){
    printf("%04X : ",(start+i*16)&0xffff);
    for( j=0; j<16; j++ ){
      printf("%02X ", ext_ram[bank][start+i*16+j]) ;
    }
    printf("|");
    for( j=0; j<16; j++ ){
      c = ext_ram[bank][start+i*16+j];
      if( !isprint( c ) ) c = '.';
      printf("%c", c );
    }
    printf("|\n");
  }
  printf("\n");

  return;
}



/*--------------------------------------------------------------*/
/* fill [<bank>] <start-addr> <end-addr> <value>		*/
/* fill [<bank>] <start-addr> #<size>	 <value>		*/
/*	メモリを埋める						*/
/*--------------------------------------------------------------*/
static	void	monitor_fill( void )
{
  int	i;
  int	bank  = ARG_MAIN;
  int	start, size, value;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
    bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR ) ) error();			/* <addr> */
  start = argv.val;
  shift();						

  if     ( argv_is( ARGV_SIZE ) ) size = argv.val;	/* [<addr|#size>] */
  else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
  else                            error();
  shift();

  if( !argv_is( ARGV_INT )) error();			/* <data> */
  value = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  for( i=0; i<size; i++ ){
    poke_memory( bank, start+i, value );
  }

  return;
}



/*--------------------------------------------------------------*/
/* move [<bank>] <src-addr> <end-addr> [<bank>] <dist-addr>	*/
/* move [<bank>] <src-addr> #size      [<bank>] <dist-addr>	*/
/*	メモリ転送						*/
/*--------------------------------------------------------------*/
static	void	monitor_move( void )
{
  int	i;
  int	s_bank  = ARG_MAIN;
  int	d_bank  = ARG_MAIN;
  int	start, size, dist;
  byte	data;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
    s_bank = argv.val;
    d_bank = s_bank;
    shift();
  }

  if( !argv_is( ARGV_ADDR ) ) error();			/* <addr> */
  start = argv.val;
  shift();						

  if     ( argv_is( ARGV_SIZE ) ) size = argv.val;	/* [<addr|#size>] */
  else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
  else                            error();
  shift();

  if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
    d_bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR ) ) error();			/* <addr> */
  dist = argv.val;
  shift();						

  if( exist_argv() ) error();


	/*================*/

  if( start+size <= dist ){			/* 転送元-転送先が 重ならない*/
    for( i=0; i<size; i++ ){
      data = peek_memory( s_bank, start+i );
      poke_memory( d_bank, dist+i, data );
    }
  }else{					/* 転送元-転送先が 重なる */
    for( i=size-1; i>=0; i-- ){
      data = peek_memory( s_bank, start+i );
      poke_memory( d_bank, dist+i, data );
    }
  }

  return;
}



/*--------------------------------------------------------------*/
/* search [<value> [[<bank>] <start-addr> <end-addr>]]		*/
/*	特定の定数 (1バイト) をサーチ				*/
/*--------------------------------------------------------------*/
static	int	save_search_addr = -1;
static	int	save_search_size = -1;
static	int	save_search_bank = ARG_MAIN;
static	byte	save_search_data;
static	void	monitor_search( void )
{
  int	i, j;

  if( !exist_argv() ){
    if( save_search_addr == -1 ) error();
    /* else   skip */
  }else{

    if( !argv_is( ARGV_INT )) error();			/* <value> */
    save_search_data = argv.val;
    shift();

    if( !exist_argv() ){
      if( save_search_addr == -1 ) error();
      /* else   skip */
    }else{

      if( argv_is( ARGV_BANK ) ){			/* [<bank>] */
	save_search_bank = argv.val;
	shift();
      }

      if( !argv_is( ARGV_ADDR ) ) error();		/* <addr> */
      save_search_addr = argv.val;
      shift();						

      if     ( argv_is( ARGV_SIZE ) )			/* <end-addr|#size>*/
			save_search_size = argv.val;
      else if( argv_is( ARGV_ADDR ) )
			save_search_size = argv.val - save_search_addr +1;
      else error();
      shift();
    }

  }

  if( exist_argv() ) error();


	/*================*/

  j=0;						/* 一致したアドレスを列挙 */
  for( i=0; i<save_search_size; i++ ){
    if( peek_memory( save_search_bank, save_search_addr+i )
        == save_search_data ){
      printf("[%04X] ",save_search_addr+i);
      if( ++j == 11 ){ printf("\n"); j=0; }
    }
  }
  if( j!=0 ) printf("\n");

  return;
}



/*--------------------------------------------------------------*/
/* read [<bank>] <addr>						*/
/*	特定のアドレスをリード					*/
/*--------------------------------------------------------------*/
static	void	monitor_read( void )
{
  int	i, j;
  int	addr;
  byte	data;
  int	bank = ARG_MAIN;



  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK )){				/* [<bank>] */
    bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR )) error();			/* <addr> */
  addr = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  data = peek_memory( bank, addr );

  printf("READ memory %s[ %04XH ] -> %02X  (= %d | %+d | ",argv2str(bank),
	 addr,(Uint)data,(Uint)data,(int)((char)data));
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf("B )\n");

  return;
}
/*--------------------------------------------------------------*/
/* write [<bank>] <addr> <data>					*/
/*	特定のアドレスにライト					*/
/*--------------------------------------------------------------*/
static	void	monitor_write( void )
{
  int	i, j;
  int	addr;
  byte	data;
  int	bank = ARG_MAIN;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK )){				/* [<bank>] */
    bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR )) error();			/* <addr> */
  addr = argv.val;
  shift();

  if( !argv_is( ARGV_INT )) error();			/* <data> */
  data = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  poke_memory( bank, addr, data );

  printf("WRITE memory %s[ %04XH ] <- %02X  (= %d | %+d | ",argv2str(bank),
	 addr,(Uint)data,(Uint)data,(int)((char)data));
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf("B )\n");

  return;
}



/*--------------------------------------------------------------*/
/* in [<cpu>] <port>						*/
/*	特定のポートから入力					*/
/*--------------------------------------------------------------*/
static	void	monitor_in( void )
{
  int	i, j;
  int	cpu = ARG_MAIN, port;
  byte	data;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_CPU )){				/* [<cpu>] */
    cpu = argv.val;
    shift();
  }

  if( !argv_is( ARGV_PORT )) error();			/* <port> */
  port = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  if( cpu==ARG_MAIN ) data = main_io_in(port) & 0xff;
  else                data = sub_io_in(port) & 0xff;

  printf("IN port %s[ %02X ] -> %02X (",argv2str(cpu),port,data);
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf(")\n");

  return;
}



/*--------------------------------------------------------------*/
/* out [<cpu>] <port> <data>					*/
/*	特定のポートに出力					*/
/*--------------------------------------------------------------*/
static	void	monitor_out( void )
{
  int	i, j;
  int	cpu = ARG_MAIN, port;
  byte	data;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_CPU )){				/* [<cpu>] */
    cpu = argv.val;
    shift();
  }

  if( !argv_is( ARGV_PORT )) error();			/* <port> */
  port = argv.val;
  shift();

  if( !argv_is( ARGV_INT )) error();			/* <data> */
  data = argv.val & 0xff;
  shift();

  if( exist_argv() ) error();


	/*================*/

  if( cpu==ARG_MAIN ) main_io_out(port,data);
  else                sub_io_out(port,data);

  printf("OUT port %s[ %02X ] <- %02X (",argv2str(cpu),port,data);
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf(")\n");

  return;
}


/*--------------------------------------------------------------*/
/* reset [<mode>] [<dipsw>]					*/
/*	リセット。モードとディップを設定できる			*/
/*--------------------------------------------------------------*/
static	void	monitor_reset( void )
{
  int	dipsw = -1, mode = -1;


  while( exist_argv() ){
    if( argv_is( ARGV_MODE )){
      if( mode != -1 ) error();				/* <mode> */
      mode = argv.val;
      shift();
    }else if( argv_is( ARGV_NUM )){			/* <dipsw> */
      if( dipsw != -1 ) error();
      dipsw = argv.val & 0xffff;
      shift();
    }else{
      error();
    }
  }

  if( exist_argv() ) error();


	/*================*/

  switch( mode ){
  case ARG_N:	boot_basic = BASIC_N;		break;
  case ARG_V1S:	boot_basic = BASIC_V1S;		break;
  case ARG_V1H:	boot_basic = BASIC_V1H;		break;
  case ARG_V2:	boot_basic = BASIC_V2;		break;
  }
  if( dipsw!=-1 ) boot_dipsw = dipsw;

  main_reset();

  if( check_break_point_PC() ) emu_mode = EXECUTE_BP;
  else                         emu_mode = EXECUTE;
}

/*--------------------------------------------------------------*/
/*  reg [[<cpu>] [<name> <value>]]				*/
/*  reg all							*/
/*	レジスタの内容を表示／変更				*/
/*--------------------------------------------------------------*/
static	void	monitor_reg( void )
{
  int	all = FALSE;
  int	cpu = -1, reg = -1, value=0;
  z80arch	*z80;


  if( exist_argv() ){

    if( argv_is( ARGV_ALL )){				/* all */
      all = TRUE;
      shift();
    }else{

      if( argv_is( ARGV_CPU )){				/* [<cpu>] */
	cpu = argv.val;
	shift();
      }

      if( exist_argv() ){
	if( !argv_is( ARGV_REG )) error();		/* [<name>] */
	reg = argv.val;
	shift();
	if( !argv_is( ARGV_INT )) error();		/* [<value>] */
	value = argv.val;
	shift();
      }
    }
  }

  if( exist_argv() ) error();


	/*================*/

  if( reg==-1 ){				/* レジスタ表示 */
    if( !all  &&  cpu==-1 ){
      if( cpu_timing >= 2 )   all = TRUE;
      else{
	if( select_main_cpu ) cpu = ARG_MAIN;
	else                  cpu = ARG_SUB;
      }
    }
    if( all ){
      z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      z80_debug( &z80sub_cpu,  "[SUB CPU]\n"  );
    }else{
      if( cpu==ARG_MAIN ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      else                z80_debug( &z80sub_cpu,  "[SUB CPU]\n"  );
    }
    return;
  }

  						/* レジスタ代入 */
  if( cpu==-1 ){
    if( cpu_timing >= 2 ){
      cpu = ARG_MAIN;
    }else{
      if( select_main_cpu ) cpu = ARG_MAIN;
      else                  cpu = ARG_SUB;
    }
  }

  if( cpu==ARG_MAIN ) z80 = &z80main_cpu;
  else                z80 = &z80sub_cpu;

  switch( reg ){
  case ARG_AF:	z80->AF.W = value;	break;
  case ARG_BC:	z80->BC.W = value;	break;
  case ARG_DE:	z80->DE.W = value;	break;
  case ARG_HL:	z80->HL.W = value;	break;
  case ARG_IX:	z80->IX.W = value;	break;
  case ARG_IY:	z80->IY.W = value;	break;
  case ARG_SP:	z80->SP.W = value;	break;
  case ARG_PC:	z80->PC.W = value;	break;
  case ARG_AF1:	z80->AF1.W = value;	break;
  case ARG_BC1:	z80->BC1.W = value;	break;
  case ARG_DE1:	z80->DE1.W = value;	break;
  case ARG_HL1:	z80->HL1.W = value;	break;
  case ARG_I:	value &= 0xff;	z80->I = value;		break;
  case ARG_R:	value &= 0xff;	z80->R = value;		break;
  case ARG_IFF:	if(value)  value=1;	z80->IFF  = value;	break;
  case ARG_IM:  if(value>3)value=2;	z80->IM   = value;	break;
  case ARG_HALT:if(value)  value=1;	z80->HALT = value;	break;
  }

  printf("[%s] reg %s <- %04X\n",argv2str(cpu),argv2str(reg),value);
}



/*--------------------------------------------------------------*/
/* disasm [[<cpu>][<start-addr>][#<steps>]]			*/
/*	逆アセンブル						*/
/*--------------------------------------------------------------*/
static	int	save_disasm_addr = -1;
static	void	monitor_disasm( void )
{
  int	i, pc;
  int	addr = save_disasm_addr;
  int	cpu = -1;
  int	step = 16;
  z80arch	*z80;


  if( exist_argv() ){

    if( argv_is( ARGV_CPU )){				/* [<cpu>] */
      cpu = argv.val;
      shift();
    }

    if( argv_is( ARGV_ADDR )){				/* [<addr>] */
      addr = argv.val;
      shift();
    }

    if( argv_is( ARGV_SIZE )){				/* [#<step>] */
      step = argv.val;
      shift();

    }

  }
  if( exist_argv() ) error();


	/*================*/

  if( cpu == -1 ){					/* CPU 未指定時 */
    if( cpu_timing >= 2 ){
      cpu = ARG_MAIN;
    }else{
      if( select_main_cpu ) cpu = ARG_MAIN;
      else                  cpu = ARG_SUB;
    }
  }

  if( cpu==ARG_MAIN ) z80 = &z80main_cpu;
  else                z80 = &z80sub_cpu;

  if( addr == -1 ) addr = z80->PC.W;			/* ADDR 未指定時 */


  pc = 0;
  for( i=0; i<step; i++ ){
    pc += z80_line_disasm( z80, addr+pc );
    printf("\n");
  }

  save_disasm_addr = ( addr + pc ) & 0xffff;
}


/*--------------------------------------------------------------*/
/* go								*/
/*	実行							*/
/*--------------------------------------------------------------*/
static	void	monitor_go( void )
{
  if( exist_argv() ) error();

  if( check_break_point_PC() ) emu_mode = EXECUTE_BP;
  else                         emu_mode = EXECUTE;
}



/*--------------------------------------------------------------*/
/* trace <step>							*/
/* trace #<step>						*/
/* trace change							*/
/*	指定したステップ分またはCPU処理が変わるまで、実行	*/
/*--------------------------------------------------------------*/
static	int	save_trace_change = FALSE;
static	void	monitor_trace( void )
{
  int	change = FALSE, step = trace_counter;

  if( exist_argv() ){

    if     ( argv_is( ARGV_CHANGE )) change = TRUE;		/* [change]  */
    else if( argv_is( ARGV_SIZE )  ) step = argv.val;		/* [<step>]  */
    else if( argv_is( ARGV_NUM )   ) step = argv.val;		/* [#<step>] */
    else                             error();
    shift();

  }else{

    if( save_trace_change ) change = TRUE;
    else                    change = FALSE;

  }

  if( exist_argv() ) error();


	/*================*/

  if( change ){
    save_trace_change = TRUE;
    if( check_break_point_PC() ) emu_mode = TRACE_CHANGE_BP;
    else                         emu_mode = TRACE_CHANGE;
  }else {
    save_trace_change = FALSE;
    trace_counter = step;
    if( check_break_point_PC() ) emu_mode = TRACE_BP;
    else                         emu_mode = TRACE;
  }
}




/*--------------------------------------------------------------*/
/* step								*/
/* step [call] [jp] [rep] [all]					*/
/*	1ステップ、実行						*/
/*	CALL、DJNZ、LDIR etc のスキップが指定可能		*/
/*--------------------------------------------------------------*/
static	void	monitor_step( void )
{
  int	call = FALSE, jp = FALSE, rep = FALSE;
  int	flag = FALSE;
  int	cpu;
  byte	code;
  word	addr;

  while( exist_argv() ){
    if( argv_is( ARGV_STEP )){
      if( argv.val==ARG_CALL ) call = TRUE;
      if( argv.val==ARG_JP )   jp   = TRUE;
      if( argv.val==ARG_REP )  rep  = TRUE;
      if( argv.val==ARG_ALL ){ call = jp = rep = TRUE; }
      shift();
    }else{
      error();
    }
  }

  if( exist_argv() ) error();


	/*================*/

  if( cpu_timing >= 2 ){

    if( check_break_point_PC() ) emu_mode = STEP_BP;
    else                         emu_mode = STEP;
    return;

  }else{

    if( select_main_cpu ){
      cpu  = BP_MAIN;
      addr = z80main_cpu.PC.W;
      code = main_mem_read( addr );
    }else{
      cpu  = BP_SUB;
      addr = z80sub_cpu.PC.W;
      code = sub_mem_read( addr );
    }

    if( call ){
      if(   code      == 0xcd ||		/* CALL nn    = 11001101B */
	  (code&0xc7) == 0xc4 ){		/* CALL cc,nn = 11ccc100B */
	addr += 3;
	flag = TRUE;
      }
    }
    if( jp ){
      if( code == 0x10 ){			/* DJNZ e     = 00010000B */
	addr += 2;
	flag = TRUE;
      }
    }
    if( rep ){
      if( code == 0xed ){			/* LDIR/LDDR/CPIR/CPDR etc */
	if( select_main_cpu ) code = main_mem_read( addr+1 );
	else                  code = sub_mem_read( addr+1  );
	if( (code&0xf4) == 0xb0 ){
	  addr += 2;
	  flag = TRUE;
	}
      }
    }

    if( flag ){
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_PC;
      break_point[cpu][BP_NUM_FOR_SYSTEM].addr = addr;
      emu_mode = EXECUTE_BP;
    }else{
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_NONE;
      emu_mode = STEP;
    }
  }
}



/*--------------------------------------------------------------*/
/* S								*/
/*	step all に同じ						*/
/*--------------------------------------------------------------*/
static	void	monitor_stepall( void )
{
  int	flag = FALSE;
  int	cpu;
  byte	code;
  word	addr;

  if( exist_argv() ) error();


	/*================*/

  if( cpu_timing >= 2 ){
    if( check_break_point_PC() ) emu_mode = STEP_BP;
    else                         emu_mode = STEP;
    return;
  }else{
    if( select_main_cpu ){
      cpu  = BP_MAIN;
      addr = z80main_cpu.PC.W;
      code = main_mem_read( addr );
    }else{
      cpu  = BP_SUB;
      addr = z80sub_cpu.PC.W;
      code = sub_mem_read( addr );
    }
    if(   code      == 0xcd ||		/* CALL nn    = 11001101B */
        (code&0xc7) == 0xc4 ){		/* CALL cc,nn = 11ccc100B */
      addr += 3;
      flag = TRUE;
    }
    if( code == 0x10 ){			/* DJNZ e     = 00010000B */
      addr += 2;
      flag = TRUE;
    }
    if( code == 0xed ){			/* LDIR/LDDR/CPIR/CPDR etc */
      if( select_main_cpu ) code = main_mem_read( addr+1 );
      else                  code = sub_mem_read( addr+1  );
      if( (code&0xf4) == 0xb0 ){
	addr += 2;
	flag = TRUE;
      }
    }
    if( flag ){
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_PC;
      break_point[cpu][BP_NUM_FOR_SYSTEM].addr = addr;
      emu_mode = EXECUTE_BP;
    }else{
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_NONE;
      emu_mode = STEP;
    }
  }

}



/*--------------------------------------------------------------*/
/* break [<cpu>] [PC|READ|WRITE|IN|OUT] <addr|port> [#<No>]	*/
/* break [<cpu>] CLEAR [#<No>]					*/
/* break							*/
/*	ブレークポイントの設定／解除／表示			*/
/*--------------------------------------------------------------*/
static	void	monitor_break( void )
{
  int	show = FALSE, i, j;
  char	*s=NULL;
  int	cpu = BP_MAIN, action = ARG_PC, addr=0, number = 0;


  if( exist_argv() ){

    if( argv_is( ARGV_CPU )){				/* [<cpu>] */
      if( argv.val==ARG_MAIN ) cpu = BP_MAIN;
      else                     cpu = BP_SUB;
      shift();
    }

    if( argv_is( ARGV_BREAK ) ){			/* <action> */
      action = argv.val;
      shift();
    }

    switch( action ){					/* <addr|port> */
    case ARG_IN: case ARG_OUT:
      if( !argv_is( ARGV_PORT )) error();
      addr = argv.val;
      shift();
      break;
    case ARG_PC: case ARG_READ: case ARG_WRITE:
      if( !argv_is( ARGV_ADDR )) error();
      addr = argv.val;
      shift();
      break;
    }

    if( exist_argv() ){					/* [#<No>] */
      if( argv.val < 0 || argv.val > NR_BP ||
	  (action != ARG_CLEAR && argv.val < 1) ) error();
      number = argv.val -1;
      shift();
    }

  }else{

    show = TRUE;

  }

  if( exist_argv() ) error();


	/*================*/

  if( show ){
    for( j=0; j<2; j++ ){
      printf( "  %s:\n", (j==0)?"MAIN":"SUB" );
      for( i=0; i<NR_BP; i++ ){
	printf( "    #%d  ", i+1 );
	if (i < 9) printf(" ");			/* 見やすく by peach */
	addr = break_point[j][i].addr;
	switch( break_point[j][i].type ){
	case BP_NONE:	printf("-- none --\n");				break;
	case BP_PC:	printf("PC   reach %04XH\n",addr&0xffff);	break;
	case BP_READ:	printf("READ  from %04XH\n",addr&0xffff);	break;
	case BP_WRITE:	printf("WRITE   to %04XH\n",addr&0xffff);	break;
	case BP_IN:	printf("INPUT from %02XH\n",addr&0xff);		break;
	case BP_OUT:	printf("OUTPUT  to %04XH\n",addr&0xff);		break;
	}
      }
    }
  }else{
    if( action==ARG_CLEAR ){
      if ( number<0 ) {
	for ( i=0; i<9; i++ ) break_point[cpu][i].type = BP_NONE;
	printf( "clear break point %s - all\n",(cpu==0)?"MAIN":"SUB" );
      } else {
	break_point[cpu][number].type = BP_NONE;
	printf( "clear break point %s - #%d\n",(cpu==0)?"MAIN":"SUB",number+1 );
      }
    }else{
      switch( action ){
      case ARG_PC:
	break_point[cpu][number].type = BP_PC;
	s = "PC : %04XH";
	break;
      case ARG_READ:
	break_point[cpu][number].type = BP_READ;
	s = "READ : %04XH";
	break;
      case ARG_WRITE:
	break_point[cpu][number].type = BP_WRITE;
	s = "WRITE : %04XH";
	break;
      case ARG_IN:
	break_point[cpu][number].type = BP_IN;
	s = "IN : %02XH";
	break;
      case ARG_OUT:
	break_point[cpu][number].type = BP_OUT;
	s = "OUT : %02XH";
	break;
      }
      break_point[cpu][number].addr = addr;
      printf( "set break point %s - #%d [ ",(cpu==0)?"MAIN":"SUB",number+1 );
      printf( s, addr );
      printf( " ]\n" );
    }
    if( cpu==BP_MAIN ) pc88main_break_point();
    else               pc88sub_break_point();
  }

}




/*--------------------------------------------------------------*/
/* loadmem <filename> <bank> <start-addr> [<end-addr>]		*/
/* loadmem <filename> <bank> <start-addr> [#<size>]		*/
/*	ファイルからメモリにロード				*/
/*--------------------------------------------------------------*/
static	void	monitor_loadmem( void )
{
  int	addr, size, bank;
  char	*filename;
  FILE	*fp;
  int	c, i;

  if( !exist_argv() ) error();				/* <filename> */
  filename = argv.str;
  shift();

  if( !argv_is( ARGV_BANK ) ) error();			/* <bank> */
  bank = argv.val;
  shift();

  if( !argv_is( ARGV_ADDR ) ) error();			/* <start-addr> */
  addr = argv.val;
  shift();

  if     ( argv_is( ARGV_SIZE ) ){			/* #<size>|<end-addr>*/
    size = argv.val;
    shift();
  }else if( argv_is( ARGV_ADDR ) ){
    size = argv.val - addr +1;
    shift();
  }else{
    size = -1;
  }

  if( exist_argv() ) error();


	/*================*/

  if( (fp=ex_fopen( filename,"rb")) ){
    if( size<0 ){
#if 0
      struct stat filestats;

      if(fstat(fileno(fp), &filestats)) size = 0;
      else                              size = filestats.st_size;
#else
      if( ex_fseek( fp, 0, SEEK_END )==0 ){
	size = ex_ftell( fp );
	if( size < 0 ) size = 0;
      }
      ex_fseek( fp, 0, SEEK_SET );
#endif
    }
    for( i=0; i<size; i++ ){
      if( (c=getc(fp)) ==  EOF ){
	printf("Warning : loadmem : file size too short (<%d)\n",size);
	break;
      }
      poke_memory( bank, addr+i, c );
    }
    ex_fclose(fp);
    printf("Load [%s] -> %s (size %d )\n",filename,argv2str(bank),i);
  }else{
    printf("file [%s] can't open\n",filename);
  }
}



/*--------------------------------------------------------------*/
/* savemem <filename> <bank> <start-addr> <end-addr>		*/
/* savemem <filename> <bank> <start-addr> #<size>		*/
/*	メモリをファイルにセーブ				*/
/*--------------------------------------------------------------*/
static	void	monitor_savemem( void )
{
  int	addr, size, bank;
  char	*filename;
  FILE	*fp;
  int	c, i;

  if( !exist_argv() ) error();				/* <filename> */
  filename = argv.str;
  shift();

  if( !argv_is( ARGV_BANK ) ) error();			/* <bank> */
  bank = argv.val;
  shift();

  if( !argv_is( ARGV_ADDR ) ) error();			/* <start-addr> */
  addr = argv.val;
  shift();

  if     ( argv_is( ARGV_SIZE ) ) size = argv.val;	/* #<size>|<end-addr>*/
  else if( argv_is( ARGV_ADDR ) ) size = argv.val - addr +1;
  else                            error();
  shift();

  if( exist_argv() ) error();


	/*================*/

  if( (fp=ex_fopen( filename,"wb")) ){
    for( i=0; i<size; i++ ){
      c = peek_memory( bank, addr+i );
      if( putc(c, fp) == EOF ){
	printf("Warning : savemem : file write failed\n");
	break;
      }
    }
    ex_fclose(fp);
    printf("Save [%s] -> %s (size %d )\n",filename,argv2str(bank),i);
  }else{
    printf("file [%s] can't open\n",filename);
  }
}



/*--------------------------------------------------------------*/
/* set [<variable> [<value>] ]					*/
/* show [<variable> ]						*/
/*	内部変数を表示／変更					*/
/*--------------------------------------------------------------*/
static	void	monitor_set_key_printf( void )		/*** set key ***/
{
  int	j;
  printf("  %-23s %-15s","key_scan[0]-[15]","(IN:00..0F)");
  for( j=0; j<0x8; j++ ) printf("%02X ", key_scan[j]);
  printf("\n");
  printf("  %-23s %-15s","","");
  for(    ; j<0x10; j++ ) printf("%02X ", key_scan[j]);
  printf("\n");
}
static	void	monitor_set_palette_printf( void )	/*** set palette ***/
{
  int	j;

  printf("  %-23s %-15sG:R:B = %01X:%01X:%01X\n",
	 "vram_bg_palette",
	 "(OUT:52/54)",
	 vram_bg_palette.green>>5,
	 vram_bg_palette.red  >>5,
	 vram_bg_palette.blue >>5 );

  for( j=0; j<8; j++ ){
    printf("  %-23s (OUT:%02X)       G:R:B = %01X:%01X:%01X\n",
	   (j==0)?"vram_palette":"",
	   j+0x54,
	   vram_palette[j].green>>5,
	   vram_palette[j].red  >>5,
	   vram_palette[j].blue >>5 );
  }
}
static	void	monitor_set_crtc_printf( void )		/*** set crtc ***/
{
  int j;
  printf("  CRTC & DMAC internal variable\n");
  
  printf("  %-23s %-15s %s\n",
	 "  CRTC active",    "", (crtc_active)?"Yes":"No"  );
  printf("  %-23s %-15s %02x\n",
	 "  Interrupt mask", "", crtc_intr_mask );
  printf("  %-23s %-15s %s\n",
	 "  Reverse",        "", (crtc_reverse_display)?"Yes":"No"  );
  printf("  %-23s %-15s %s\n",
	 "  Line Skip",      "", (crtc_skip_line)?"Yes":"No"  );
  printf("  %-23s %-15s %02XH(%d)\n",
	 "  Cursor position",   "[X]", crtc_cursor[0], crtc_cursor[0] );
  printf("  %-23s %-15s %02XH(%d)\n",
	 "  Cursor position",   "[Y]", crtc_cursor[1], crtc_cursor[1] );


  printf("  %-23s %-15s %02XH\n", "  CRTC Format [0]",
	 "CHHHHHHH", crtc_format[0] );
  printf("  %-23s %-25s %s\n", "",
	 "  C:DMA mode", (crtc_format[0])&0x80?"Charactor":"Burst" );
  printf("  %-23s %-25s %d\n", "",
	 "  H:chars/line", (crtc_format[0]&0x7f)+2 );

  printf("  %-23s %-15s %02XH\n", "  CRTC Format [1]",
	 "BBLLLLLL", crtc_format[1] );
  printf("  %-23s %-25s %d\n", "",
	 "  B:blink time", crtc_format[1]>>6 );
  printf("  %-23s %-25s %d\n", "",
	 "  L:lines/screen", (crtc_format[1]&0x3f)+1 );

  printf("  %-23s %-15s %02XH\n", "  CRTC Format [2]",
	 "SCCRRRRR", crtc_format[2] );
  printf("  %-23s %-25s %s\n", "",
	 "  S:skip line", (crtc_format[2]&0x80)?"Skip":"Normal");
  printf("  %-23s %-25s %d\n", "",
	 "  C:cursor type", (crtc_format[2]>>5)&0x03 );
  printf("  %-23s %-25s %d\n", "",
	 "  R:lines/char", (crtc_format[2]&0x1f)+1 );

  printf("  %-23s %-15s %02XH\n", "  CRTC Format [3]",
	 "VVVZZZZZ", crtc_format[3] );
  printf("  %-23s %-25s %d\n", "",
	 "  V:v wide(line)", (crtc_format[3]>>5)+1 );
  printf("  %-23s %-25s %d\n", "",
	 "  Z:h wide(char)", (crtc_format[3]&0x1f)+2 );

  printf("  %-23s %-15s %02XH\n", "  CRTC Format [4]",
	 "TTTAAAAA", crtc_format[4] );
  printf("  %-23s %-25s %d\n", "",
	 "  T:attr type", (crtc_format[4]>>5) );
  printf("  %-23s %-25s %d\n", "",
	 "  A:attr size", (crtc_format[4]&0x1f)+1 );


  for(j=0;j<4;j++ ){
    printf("  %-23s Ch.%d%-11s %04XH .. +%04XH, %s\n",
	   (j==0)?"  DMAC addr/cntr/mode":"", j,"",
	   dmac_address[j].W, dmac_counter[j].W&0x3fff,
	   ((dmac_counter[j].W&0xc000)==0x0000)?"Verify":
	   (((dmac_counter[j].W&0xc000)==0x4000)?"Write":
	    (((dmac_counter[j].W&0xc000)==0x8000)?"Read":"BAD"))
	   );
  }
  printf("  %-23s %-15s %02X (%X%X%X%X%X%X%X%X)\n",
	 "", "Mode",
	 dmac_mode,
	 (dmac_mode>>7)&0x01,	 (dmac_mode>>6)&0x01,
	 (dmac_mode>>5)&0x01,	 (dmac_mode>>4)&0x01,
	 (dmac_mode>>3)&0x01,	 (dmac_mode>>2)&0x01,
	 (dmac_mode>>1)&0x01,	 (dmac_mode>>0)&0x01
	 );
  printf("  %-23s %-15s %s\n",
	 "  text_display",    "", (text_display==TEXT_DISABLE)?"TEXT_DISABLE"
				 :((text_display==TEXT_ENABLE)?"TEXT_ENABLE"
				   :"TEXT_ATTR_ONLY"));
}
static	void	monitor_set_pio_printf( void )		/*** set pio ***/
{
/*
  pio_AB[0][0].type     R    __      __  R   pio_AB[1][0].type   
              .exist    -      \    /    -               .status 
              .data     00H     \  /     00H             .data   
              .chg_cnt  4        \/      4               .chg_cnt
                                 /\
  pio_AB[0][1].type     W       /  \     W   pio_AB[1][1].type   
              .exist    -    __/    \__  -               .exist
              .data     00H              00H             .data   
              .chg_cnt  4                4               .chg_cnt
  
  pio_C[0][0].type      W    ___    ___  W   pio_C[1][0].type    
             .data      00H     \  /     00H            .data    
             .chg_cnt   1        \/      1              .chg_cnt  
                                 /\
  pio_C[0][1].type      R    ___/  \___  R   pio_C[1][1].type    
             .data      00H              00H            .data    
             .chg_cnt   1                1              .chg_cnt
*/

  printf(
     "  pio_AB[0][0].type     %s    __      __  %s   pio_AB[1][0].type\n",
	 (pio_AB[0][0].type==PIO_READ)?"R":"W",
	 (pio_AB[1][0].type==PIO_READ)?"R":"W" );

  printf(
     "              .exist    %s      \\    /    %s               .exist\n",
	 (pio_AB[0][0].exist==PIO_EXIST)?"*":"-",
	 (pio_AB[1][0].exist==PIO_EXIST)?"*":"-" );

  printf(
     "              .data     %02XH     \\  /     %02XH             .data\n",
	 (pio_AB[0][0].data),
	 (pio_AB[1][0].data) );

  printf(
     "              .chg_cnt % 2d       \\/     % 2d              .chg_cnt\n",
	 (pio_AB[0][0].chg_cnt),
	 (pio_AB[1][0].chg_cnt) );

  printf("                                 /\\\n");

  printf(
     "  pio_AB[0][1].type     %s       /  \\     %s   pio_AB[1][1].type\n",
	 (pio_AB[0][1].type==PIO_READ)?"R":"W",
	 (pio_AB[1][1].type==PIO_READ)?"R":"W" );

  printf(
     "              .exist    %s    __/    \\__  %s               .exist\n",
	 (pio_AB[0][1].exist==PIO_EXIST)?"*":"-",
	 (pio_AB[1][1].exist==PIO_EXIST)?"*":"-" );

  printf(
     "              .data     %02XH              %02XH             .data\n",
	 (pio_AB[0][1].data),
	 (pio_AB[1][1].data) );

  printf(
     "              .chg_cnt % 2d              % 2d              .chg_cnt\n",
	 (pio_AB[0][1].chg_cnt),
	 (pio_AB[1][1].chg_cnt) );

  printf("  \n");

  printf(
     "  pio_C[0][0].type      %s    ___    ___  %s   pio_C[1][0].type\n",
	 (pio_C[0][0].type==PIO_READ)?"R":"W",
	 (pio_C[1][0].type==PIO_READ)?"R":"W" );

  printf(
     "             .data      %02XH     \\  /     %02XH            .data\n",
	 (pio_C[0][0].data),
	 (pio_C[1][0].data) );

  printf(
     "             .chg_cnt   % 2d       \\/      % 2d             .chg_cnt\n",
	 (pio_C[0][0].chg_cnt),
	 (pio_C[1][0].chg_cnt) );

  printf("                                 /\\\n");

  printf(
     "  pio_C[0][1].type      %s    ___/  \\___  %s   pio_C[1][1].type\n",
	 (pio_C[0][1].type==PIO_READ)?"R":"W",
	 (pio_C[1][1].type==PIO_READ)?"R":"W" );

  printf(
     "             .data      %02XH              %02XH            .data\n",
	 (pio_C[0][1].data),
	 (pio_C[1][1].data) );

  printf(
     "             .chg_cnt   % 2d               % 2d             .chg_cnt\n",
	 (pio_C[0][1].chg_cnt),
	 (pio_C[1][1].chg_cnt) );
}

/*----------------------------------------------------------------------*/
/* fbreak [<cpu>] [READ|WRITE|DIAG] <drive> <track> [<sector>] [#<No>]	*/
/* fbreak [<cpu>] CLEAR [#<No>]						*/
/* fbreak								*/
/*	FDC ブレークポイントの設定／解除／表示				*/
/*				この機能は peach氏により実装されました	*/
/*----------------------------------------------------------------------*/
static void monitor_fbreak( void )
{
    int show = FALSE, i;
    char *s = NULL;
    int action = ARG_READ, number = 0;
    int	drive = -1, track = -1, sector = -1;

    if(exist_argv()){
	if (argv_is(ARGV_FBREAK)) {			/* <action> */
	    action = argv.val;
	    shift();
	}

	if (action == ARG_READ || action == ARG_WRITE || action == ARG_DIAG) {
	    if(!argv_is(ARGV_DRV)) error();
	    drive = argv.val;
	    shift();
	    if (!argv_is(ARGV_NUM) || argv.val < 0 || argv.val > 163) error();
	    track = argv.val;
	    shift();
	    if (exist_argv() && !argv_is(ARGV_SIZE)) {
		if (!argv_is(ARGV_NUM) || argv.val < 0) error();
		sector = argv.val;
		shift();
	    }
	}

	if(exist_argv()){					/* [#<No>] */
	    if( argv.val < 0 || argv.val > NR_BP ||
		(action != ARG_CLEAR && argv.val < 1) ) error();
	    number = argv.val -1;
	    shift();
	}
    } else {
	show = TRUE;
    }

    if(exist_argv()) error();

    if( show ){
	printf( "  %s:\n", "FDC" );
	for( i=0; i<NR_BP; i++ ){
	    /*if (break_point_fdc[i].type == BP_NONE) continue;*/
	    printf( "    #%d  ", i+1 );
	    if (i < 9) printf(" ");
	    drive = break_point_fdc[i].drive;
	    track = break_point_fdc[i].track;
	    sector = break_point_fdc[i].sector;
	    if (break_point_fdc[i].type == BP_NONE) {
		printf("-- none --\n");
	    } else {
		switch( break_point_fdc[i].type ){
		case BP_READ:	printf("FDC READ from "); break;
		case BP_WRITE:	printf("FDC WRITE  to "); break;
		case BP_DIAG:	printf("FDC DIAG   in "); break;
		}
		printf("D:%d T:%d", drive, track);
		if (sector >= 0) printf(" S:%d", sector);
		printf("\n");
	    }
	}
    } else {
	if( action==ARG_CLEAR ){
	    if ( number<0 ) {
		for ( i=0; i<9; i++ ) break_point_fdc[i].type = BP_NONE;
		printf( "clear break point %s - all\n", "FDC" );
	    } else {
		break_point_fdc[number].type = BP_NONE;
		printf( "clear break point %s - #%d\n", "FDC", number+1);
	    }
	}else{
	    switch( action ){
	    case ARG_READ:
		break_point_fdc[number].type = BP_READ;
		s = "READ";
		break;
	    case ARG_WRITE:
		break_point_fdc[number].type = BP_WRITE;
		s = "WRITE";
		break;
	    case ARG_DIAG:
		break_point_fdc[number].type = BP_DIAG;
		s = "DIAG";
		break;
	    }
	    break_point_fdc[number].drive = drive;
	    break_point_fdc[number].track = track;
	    break_point_fdc[number].sector = sector;
	    printf( "set break point - #%d [ %s : D:%d T:%d ",
		    number+1, s, drive, track);
	    if (sector >= 0) printf("S:%d ", sector);
	    printf("]\n");
	}
    }
	
    pc88fdc_break_point();
}

/*----------------------------------------------------------------------*/
/* textscr								*/
/*	テキスト画面をコンソールに表示					*/
/*				この機能は peach氏により実装されました	*/
/*----------------------------------------------------------------------*/
static void print_text_screen( void )
{
    /*FILE *fp;*/
    int i, j;
    int line;
    int width;
    int end;
    Uchar text_buf[82];		/* 80文字 + '\n' + '\0' */
    
    if (grph_ctrl & 0x20) line = 25;
    else                  line = 20;

    if (sys_ctrl & 0x01) width = 80;
    else                 width = 40;

    for (i = 0, end = 0; i < line; i++) {
	for (j = 0; j < width; j++) {
	    if (width == 80) {
		text_buf[j] =
		    text_attr_buf[ text_attr_flipflop ][i * 80 + j] >> 8;
	    } else {
		text_buf[j] =
		    text_attr_buf[ text_attr_flipflop ][i * 80 + j * 2] >> 8;
	    }
	    if (text_buf[j] == 0) text_buf[j] = ' ';
	    else end = j;
	}
	/* 終端までの空白は入れない */
	text_buf[end + 1] = '\n';
	text_buf[end + 2] = '\0';
	print_hankaku(stdout, text_buf, alt_char);
    }
}

static void monitor_textscr( void )
{
    if( exist_argv() ){					/* <char> */
      alt_char = argv.str[0];
      shift();
    }

    print_text_screen();
}

/*--------------------------------------------------------------*/
/* loadbas <filename> [<type>]					*/
/*	BASIC LIST を読み込む					*/
/*--------------------------------------------------------------*/
#if 1					/* experimental by peach */
static void monitor_loadbas( void )
{
    char *filename;
    int size;
    int type = ARG_ASCII;
    FILE *fp;

    if( !exist_argv() ) error();			/* <filename> */
    filename = argv.str;
    shift();

    if( exist_argv() ) {				/* <type> */
	if(!argv_is(ARGV_BASIC)) error();
	type = argv.val;
	shift();
    }

    if( exist_argv() ) error();

    if ((fp = ex_fopen( filename, "r"))) {
	if (type == ARG_ASCII) {
	    size = basic_encode_list(fp);
	} else {
	    size = basic_load_intermediate_code(fp);	    
	}
	ex_fclose(fp);
	if (size > 0) printf("Load [%s] (size %d)\n", filename, size);
    } else {
	printf("file [%s] can't open\n",filename);
    }
}
#else
static void monitor_loadbas( void ){ printf( "sorry, not support\n" ); }
#endif
/*--------------------------------------------------------------*/
/* savebas [<filename> [<type>]]				*/
/*	BASIC LIST を出力					*/
/*--------------------------------------------------------------*/
#if 1					/* experimental by peach */
static void monitor_savebas( void )
{
    char *filename;
    int size;
    int type = ARG_ASCII;
    FILE *fp;

    if( exist_argv() ) {			/* <filename> */
	filename = argv.str;
	shift();
	if( exist_argv() ) {			/* <type> */
	    if(!argv_is(ARGV_BASIC)) error();
	    type = argv.val;
	    shift();
	}
    } else {
	filename = NULL;
    }

    if( exist_argv() ) error();

    if(filename == NULL) {
	basic_decode_list(stdout);
    } else if ((fp = ex_fopen( filename, "w"))) {
	if (type == ARG_ASCII) {
	    size = basic_decode_list(fp);
	} else {
	    size = basic_save_intermediate_code(fp);
	}
	ex_fclose(fp);
	if (size > 0) printf("Save [%s] (size %d)\n", filename, size);
    } else {
	printf("file [%s] can't open\n",filename);
    }
}
#else
static void monitor_savebas( void ){ printf( "sorry, not support\n" ); }
#endif

/*--------------------------------------------------------------*/
/* サウンド関連							*/
/*								*/
/*--------------------------------------------------------------*/
#ifdef	USE_SOUND
static	void	monitor_set_volume_printf( int index )
{
  printf("  %-23s %-15s %d\n",
	 monitor_variable[index].var_name,
	 monitor_variable[index].port_mes, xmame_get_sound_volume() );
}
static	void	monitor_set_fmmixer_printf( int index )
{
  printf("  %-23s %-15s %d\n",
	 monitor_variable[index].var_name,
	 monitor_variable[index].port_mes, xmame_get_mixer_volume( XMAME_MIXER_FM ) );
}
static	void	monitor_set_psgmixer_printf( int index )
{
  printf("  %-23s %-15s %d\n",
	 monitor_variable[index].var_name,
	 monitor_variable[index].port_mes, xmame_get_mixer_volume( XMAME_MIXER_PSG ) );
}
static	void	monitor_set_beepmixer_printf( int index )
{
  printf("  %-23s %-15s %d\n",
	 monitor_variable[index].var_name,
	 monitor_variable[index].port_mes, xmame_get_mixer_volume( XMAME_MIXER_BEEP ) );
}
static	void	monitor_set_rhythmmixer_printf( int index )
{
  printf("  %-23s %-15s %d\n",
	 monitor_variable[index].var_name,
	 monitor_variable[index].port_mes, xmame_get_mixer_volume( XMAME_MIXER_RHYTHM ) );
}
static	void	monitor_set_adpcmmixer_printf( int index )
{
  printf("  %-23s %-15s %d\n",
	 monitor_variable[index].var_name,
	 monitor_variable[index].port_mes, xmame_get_mixer_volume( XMAME_MIXER_ADPCM ) );
}
static	void	monitor_set_mixer_printf( void )
{
  if( use_sound ){
    printf("\n  Following is mixing level of xmame-sound-driver.\n");
    xmame_set_mixer_volume( -1, -1 );
  }
}

static	void	monitor_set_volume( int vol )
{
  xmame_set_sound_volume( vol );
}
static	void	monitor_set_fmmixer( int vol )
{
  xmame_set_mixer_volume( XMAME_MIXER_FM, vol );
}
static	void	monitor_set_psgmixer( int vol )
{
  xmame_set_mixer_volume( XMAME_MIXER_PSG, vol );
}
static	void	monitor_set_beepmixer( int vol )
{
  xmame_set_mixer_volume( XMAME_MIXER_BEEP, vol );
}
static	void	monitor_set_rhythmmixer( int vol )
{
  xmame_set_mixer_volume( XMAME_MIXER_RHYTHM, vol );
}
static	void	monitor_set_adpcmmixer( int vol )
{
  xmame_set_mixer_volume( XMAME_MIXER_ADPCM, vol );
}
#endif

static void monitor_set_show_printf( int index )	/*** set (print) ***/
{
  int	val;
  switch( monitor_variable[index].var_type ){

  case MTYPE_INT:
  case MTYPE_CURSOR:
  case MTYPE_SIZE:
  case MTYPE_MOUSE:
  case MTYPE_FRAMESKIP:
    val = *((int *)monitor_variable[index].var_ptr);
    goto MTYPE_numeric_variable;

  case MTYPE_BYTE:
    val = *((byte *)monitor_variable[index].var_ptr);
    goto MTYPE_numeric_variable;

  case MTYPE_WORD:
    val = *((word *)monitor_variable[index].var_ptr);
    goto MTYPE_numeric_variable;

  MTYPE_numeric_variable:;
    printf("  %-23s %-15s %08XH (%d)\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes, val, val );
    break;

  case MTYPE_DOUBLE:
  case MTYPE_CLOCK:
    printf("  %-23s %-15s %8.4f\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   *((double *)monitor_variable[index].var_ptr) );
    break;

  case MTYPE_KEY:
    monitor_set_key_printf();
    break;
  case MTYPE_PALETTE:
    monitor_set_palette_printf();
    break;
  case MTYPE_CRTC:
    monitor_set_crtc_printf();
    break;
  case MTYPE_PIO:
    monitor_set_pio_printf();
    break;

#ifdef	USE_SOUND
  case MTYPE_VOLUME:
    monitor_set_volume_printf(index);
    break;
  case MTYPE_FMMIXER:
    monitor_set_fmmixer_printf(index);
    break;
  case MTYPE_PSGMIXER:
    monitor_set_psgmixer_printf(index);
    break;
  case MTYPE_BEEPMIXER:
    monitor_set_beepmixer_printf(index);
    break;
  case MTYPE_RHYTHMMIXER:
    monitor_set_rhythmmixer_printf(index);
    break;
  case MTYPE_ADPCMMIXER:
    monitor_set_adpcmmixer_printf(index);
    break;
  case MTYPE_MIXER:
    monitor_set_mixer_printf();
    break;
#endif

  case MTYPE_NEWLINE:
    printf("\n");
    break;
  }
}






static	void	monitor_set( void )
{
  void	*var_ptr;
  int	set_type, index=0, value=0, i;
  double dvalue = 0.0;
  int	key_flag = 0;

  set_type = 0;
  if( exist_argv() ){
    for( index=0; index<countof(monitor_variable); index++ ){
      if( strcmp( argv.str, monitor_variable[index].var_name )==0 ) break;
    }
    if( index==countof(monitor_variable) ) error();
    shift();
    set_type = 1;
    if( exist_argv() ){
      if     ( argv_is( ARGV_INT ) ){
	value  = argv.val;
	dvalue = (double)argv.val;
      }else if( argv_is( ARGV_STR ) ){
	if( monitor_variable[index].var_type != MTYPE_KEY ){
	  char *chk;
	  dvalue = strtod( argv.str, &chk );
	  if( *chk != '\0' ) error();
	}else{
	  if( argv.str[0] != '~' ){
	    error();
	  }
	  key_flag = 1;
	}
      }else{
	error();
      }
      shift();
      set_type = 2;
    }
  }

  if( exist_argv() ) error();


	/*================*/

  switch( set_type ){
  case 0:
    for( index=0; index<countof(monitor_variable); index++ ){
      monitor_set_show_printf( index );
    }
    break;

  case 1:
    monitor_set_show_printf( index );
    break;

  case 2:
    var_ptr = monitor_variable[index].var_ptr;
    switch( monitor_variable[index].var_type ){

    case MTYPE_INT: 	*((int *)var_ptr)  = value;	break;
    case MTYPE_BYTE:	*((byte *)var_ptr) = value;	break;
    case MTYPE_WORD:	*((word *)var_ptr) = value;	break;

    case MTYPE_KEY:
      if( key_flag==0 ){
	for( i=0; i<0x10; i++ ) key_scan[i] = value;
	printf("     key_scan[0..15] = %d\n",value);
      }else{
	char c_new[16];
	char *p = &argv.str[1];
	int i, j, x;
	for( i=0; i<(int)sizeof(c_new) && *p;i++ ){
	  for( j=0; j<2; j++ ){
	    x = -1;
	    if     ( '0'<=*p && *p<='9' ) x = *p - '0';
	    else if( 'a'<=*p && *p<='f' ) x = *p - 'a' + 10;
	    else if( 'A'<=*p && *p<='F' ) x = *p - 'A' + 10;
	    p++;
	    if( (*p=='\0' && j==0) || x < 0 ){
	      goto set_key_break;
	    }
	    if( j==0 ) c_new[i] = x*16;
	    else       c_new[i] = ( c_new[i] + x );
	  }
	}
	if( *p ) goto set_key_break;

	for( j=0;j<i;j++ ){
	  key_scan[j] = ~c_new[j];
	}
	monitor_set_show_printf( index );
      }
      break;

    set_key_break:
      printf("Invalid parameter %s.\n", argv.str);
      break;

    case MTYPE_PALETTE:
    case MTYPE_CRTC:
    case MTYPE_PIO:
      printf("Sorry! This variable can't set value. "
	     "(palette,crtc,pio, and so on)\n");
      break;

    case MTYPE_CURSOR:
      *((int *)var_ptr) = value;
      if( cursor_emu ) set_cursor_emu_effective();
      else	       set_cursor_emu_invalid();
      break;

    case MTYPE_SIZE:
      graphic_system_restart(TRUE);
      break;

    case MTYPE_MOUSE:
      *((int *)var_ptr) = value;
      if( hide_mouse ) set_mouse_invisible();
      else	       set_mouse_visible();
      break;

    case MTYPE_FRAMESKIP:
      *((int *)var_ptr) = value;
      blink_ctrl_update();
      break;

    case MTYPE_DOUBLE:
      *((double *)var_ptr) = dvalue;
      break;

    case MTYPE_CLOCK:
      *((double *)var_ptr) = dvalue;
      interval_work_init_all();
      break;

#ifdef	USE_SOUND
    case MTYPE_VOLUME:
      monitor_set_volume( value );
      break;
    case MTYPE_FMMIXER:
      monitor_set_fmmixer( value );
      break;
    case MTYPE_PSGMIXER:
      monitor_set_psgmixer( value );
      break;
    case MTYPE_BEEPMIXER:
      monitor_set_beepmixer( value );
      break;
    case MTYPE_RHYTHMMIXER:
      monitor_set_rhythmmixer( value );
      break;
    case MTYPE_ADPCMMIXER:
      monitor_set_adpcmmixer( value );
      break;
    case MTYPE_MIXER:
      break;
#endif

    }
    break;

  }
}

static	void	monitor_show( void )
{
  int	set_type, index=0;

  set_type = 0;
  if( exist_argv() ){
    for( index=0; index<countof(monitor_variable); index++ ){
      if( strcmp( argv.str, monitor_variable[index].var_name )==0 ) break;
    }
    if( index==countof(monitor_variable) ) error();
    shift();
    set_type = 1;
  }

  if( exist_argv() ) error();


  switch( set_type ){
  case 0:
    for( index=0; index<countof(monitor_variable); index++ ){
      monitor_set_show_printf( index );
    }
    break;

  case 1:
    monitor_set_show_printf( index );
    break;
  }
}





/*--------------------------------------------------------------*/
/* resize <screen_size>						*/
/*	画面サイズを変更					*/
/*--------------------------------------------------------------*/
static	void	monitor_resize( void )
{
  int	command = -1;

  if( exist_argv() ){
    if( !argv_is( ARGV_RESIZE ) ) error();
    command = argv.val;
    shift();
  }

  if( exist_argv() ) error();


	/*================*/

  switch( command ){
  case -1:					/* resize */
    if( ++screen_size > screen_size_max ) screen_size = 0;
    break;
  case ARG_FULL:				/* resize full */
    screen_size = SCREEN_SIZE_FULL;
    break;
  case ARG_HALF:				/* resize half */
    screen_size = SCREEN_SIZE_HALF;
    break;
  case ARG_DOUBLE:				/* resize double */
    screen_size = SCREEN_SIZE_DOUBLE;
    break;
  }

  if( screen_size > screen_size_max ) screen_size = screen_size_max;
  graphic_system_restart(TRUE);
  return;
}





/*--------------------------------------------------------------*/
/* drive [show]							*/
/* drive eject [<drive_no>]					*/
/* drive empty <drive_no>					*/
/* drive set <drive_no> <filename >				*/
/*	ドライブ関連処理					*/
/*		ドライブに設定されたファイルの状態を見る	*/
/*		ドライブを空にする				*/
/*		ドライブを一時的に空にする			*/
/*		ドライブにファイルをセット(交換)		*/
/*--------------------------------------------------------------*/
static	void	monitor_drive( void )
{
  int	i, j, command, drv = -1, img = 0;
  char	*filename=NULL;

  if( !exist_argv() ){
    command = ARG_SHOW;
  }else{

    if( !argv_is( ARGV_DRIVE )) error();		/* <command> */
    command = argv.val;
    shift();

    switch( command ){
    case ARG_SHOW:					/*   show */
      break;
    case ARG_EJECT:					/*   eject */
      if( exist_argv() ){
	if( !argv_is( ARGV_DRV )) error();		/*	[<drive_no>] */
	drv = argv.val-1;
	shift();
      }
      break;
    case ARG_EMPTY:					/*   empty */
      if( !argv_is( ARGV_DRV )) error();		/*	<drive_no> */
      drv = argv.val-1;
      shift();
      break;
    case ARG_SET:					/*   set */
      if( !argv_is( ARGV_DRV )) error();		/*	<drive_no> */
      drv = argv.val-1;
      shift();
      if( !exist_argv() ) error();			/*	<filename> */
      filename = argv.str;
      shift();
      if( exist_argv() ){
	if( !argv_is( ARGV_IMG )) error();		/*	[<image_no>] */
	img = argv.val-1;
	shift();
      }
      break;
    }

  }
  if( exist_argv() ) error();


	/*================*/

  switch( command ){
  case ARG_SHOW:				/* drive show */
    for( i=0; i<2; i++ ){
      printf("DRIVE %d: lamp[ %s ]%s\n",
	     i+1, get_drive_ready(i) ? " " : "#",
	     (drive[i].fp && drive[i].empty) ? "  ..... *** Empty ***" : "" );
      if( disk_image_exist( i ) ){
	printf("    filename = %s\n",drive[i].filename );
	printf("    FILE *fp = %p : permission = %s : filesize = %ld\n",
	       (void *)drive[i].fp,
	       drive[i].read_only ? "Read Only" : "Read Write",
	       drive[i].file_size );
	printf("    Selected image No = %d/%d%s : protect = %s : media = %s\n",
	       drive[i].selected_image +1,
	       drive[i].image_nr,
	       drive[i].detect_broken_image ? " + broken"
			: (drive[i].over_image ? " + alpha " : "         "),
	       drive[i].protect==DISK_PROTECT_TRUE ? "RO" : "RW",
	       (drive[i].type==DISK_TYPE_2HD) ? "2HD"
			: ((drive[i].type==DISK_TYPE_2DD) ? "2DD" : "2D") );
	printf("    ------------------------------------------\n");
	for( j=0; j<drive[i].image_nr; j++ ){
	  printf("    %s% 3d %-17s  %s  %s  %ld\n",
		 j==drive[i].selected_image ? "*" : " ",
		 j+1,
		 drive[i].image[j].name,
		 drive[i].image[j].protect==DISK_PROTECT_TRUE ? "RO"
		 :(drive[i].image[j].protect==DISK_PROTECT_FALSE?"RW":"??"),
		 drive[i].image[j].type==DISK_TYPE_2D  ? "2D "
		 :(drive[i].image[j].type==DISK_TYPE_2DD ? "2DD"
		   :(drive[i].image[j].type==DISK_TYPE_2HD ? "2DH":"???")),
		 drive[i].image[j].size);
	}
      }
      printf("\n");
    }
    break;
  case ARG_EMPTY:				/* drive empty n */
    if( disk_image_exist( drv ) ){
      drive_change_empty( drv );
      if( drive_check_empty(drv) ) printf("** Set DRIVE %d: Empty **\n",drv+1);
      else                       printf("** Unset DRIVE %d: Empty **\n",drv+1);
    }
    break;
  case ARG_EJECT:
    switch( drv ){
    case 0:					/* drive eject 0 */
    case 1:					/* drive eject 1 */
      if( disk_image_exist( drv ) ){
	disk_eject( drv );
	printf("-- Disk Eject from DRIVE %d: --\n",drv+1);
      }
      break;
    default:					/* drive eject	*/
      for( i=0; i<2; i++ ){
	if( disk_image_exist( i ) ){
	  disk_eject( i );
	  printf("-- Disk Eject from DRIVE %d: --\n",i+1);
	}
      }
      break;
    }
    break;
  case ARG_SET:					/* drive set x yyy z */
    if( strcmp( filename, "-" )==0 ){
      switch( disk_change_image( drv, img ) ){
      case 0:						/* OK */
	printf("== Image change in DRIVE %d: ==\n", drv+1);
	printf("   image number ->[%d]\n",img+1);
	break;
      case 1:						/* no disk */
	printf("** Disk not exist in DRIVE:%d **\n", drv+1 );
	break;
      case 2:						/* no image */
	printf("** Image number %d is not exist in DRIVE:%d **\n",img+1,drv+1);
	break;
      }
    }else{
      if( disk_image_exist( drv ) ){
	disk_eject( drv );
	printf("-- Disk Eject from DRIVE %d: --\n",drv+1);
      }
      if( disk_insert( drv, filename, img ) ){		/* Faile */
	printf("** Disk %s can't insert **\n",filename);
      }else{						/* Success */
	printf("== Disk insert to DRIVE %d: ==\n", drv+1);
	printf("   file ->[%s] image number ->[%d]\n",filename,img+1);
      }
    }
    break;
  }


  return;
}



/*--------------------------------------------------------------*/
/* file show <filename>						*/
/* file create <filename>					*/
/* file protect <filename> <image_no>				*/
/* file unprotect <filename> <image_no>				*/
/* file format <filename> <image_no>				*/
/* file unformat <filename> <image_no>				*/
/* file rename <filename> <image_no> <image_name>		*/
/*	ファイル関連処理					*/
/*		ファイルを見る					*/
/*		ブランクイメージをファイルに追加(作成)		*/
/*		イメージのライトプロテクトを設定		*/
/*		イメージのライトプロテクトを解除		*/
/*		イメージをフォーマット				*/
/*		イメージをアンフォーマット			*/
/*		イメージ名を変更				*/
/*--------------------------------------------------------------*/
static	void	monitor_file( void )
{
  int	command, num, img = 0, result=-1, busy;
  char	*filename, *imagename=NULL;
  long	offset;
  FILE *fp;
  Uchar	c[32];
  char	*s = NULL;



  if( !argv_is( ARGV_FILE )) error();			/* <command> */
  command = argv.val;
  shift();

  if( !exist_argv() ) error();				/* <filename> */
  filename = argv.str;
  shift();

  switch( command ){
  case ARG_SHOW:					/*   show      */
  case ARG_CREATE:					/*   create    */
    break;
  case ARG_PROTECT:					/*   protect   */
  case ARG_UNPROTECT:					/*   unprotect */
  case ARG_FORMAT:					/*   format    */
  case ARG_UNFORMAT:					/*   unformat  */
    if( !argv_is( ARGV_IMG ) ) error();			/*	<image_no> */
    img = argv.val-1;
    shift();
    break;
  case ARG_RENAME:					/*   rename    */
    if( !argv_is( ARGV_IMG ) ) error();		/*	<image_no> */
    img = argv.val-1;
    shift();
    if( !exist_argv() ) error();
    imagename = argv.str;				/*	<imagename> */
    shift();
    break;
  }

  if( exist_argv() ) error();


	/*================*/

  switch( command ){
  case ARG_SHOW:
    if( (fp = ex_fopen( filename, "rb" ))==NULL ){
      printf("Open error! %s\n",filename);
      break;
    }
    printf("filename = %s   size = %ld\n",
	    filename, osd_file_size( filename ) );
    printf("  -No------Name-----------R/W-Type---Size--\n");
    offset = 0;
    num = 0;
    while( (result=fread_header( fp, offset, c ) ) == 0 ){
      c[16] = '\0';
      printf("  % 3d   %-17s  %s  %s  %ld\n",
	     num+1,
	     c,
	     c[DISK_PROTECT]==DISK_PROTECT_TRUE ? "RO"
	     :(c[DISK_PROTECT]==DISK_PROTECT_FALSE?"RW":"??"),
	     c[DISK_TYPE]==DISK_TYPE_2D  ? "2D "
	     :(c[DISK_TYPE]==DISK_TYPE_2DD ? "2DD"
	       :(c[DISK_TYPE]==DISK_TYPE_2HD ? "2DH":"???")),
	     read_size_in_header( c ) );
      offset += read_size_in_header( c );
      num++;
      if( num > 255 ){ result = -1; break; }
    }
    printf("\n");
    switch( result ){
    case -1:  printf("Image number too many (over 255)\n");	break;
    case 1:							break;
    case 2:   printf("Image is broken in image No. %d\n",num+1);break;
    case 3:   printf("Access Error\n");				break;
    default:  printf("Internal Error\n");
    }
    ex_fclose(fp);
    break;


  case ARG_CREATE:
  case ARG_PROTECT:
  case ARG_UNPROTECT:
  case ARG_FORMAT:
  case ARG_UNFORMAT:
  case ARG_RENAME:
				/* ファイルを開く */
    switch( command ){
    case ARG_CREATE: fp = fopen_image_edit( filename, "ab", &busy );  break;
    default:         fp = fopen_image_edit( filename, "r+b", &busy ); break;
    }
				/* エラーチェック */
    if( fp==NULL ){
      switch( busy ){
      case -1:
	printf("Open error! %s\n",filename);				break;
      case 0: case 1:
	printf("File %s is read only (in DRIVE %d:)\n",filename,busy+1);break;
      }
      break;	/* 終了 */
    }else{
      switch( busy ){
      case 0: case 1:
	if( drive[ busy ].detect_broken_image )
	  printf("Warning! File %s maybe be broken!"
		 " ..... continued, but not update drive status.\n",filename);
      }
    }
				/* コマンド別処理 */
    switch( command ){
    case ARG_CREATE:
      result = append_blank( fp );		s = "Create blank image";
      break;
    case ARG_PROTECT:
      c[0] = DISK_PROTECT_TRUE;
      result = fwrite_protect( fp, img, (char*)c );	s = "Set Protect";
      break;
    case ARG_UNPROTECT:
      c[0] = DISK_PROTECT_FALSE;
      result = fwrite_protect( fp, img, (char*)c );	s = "Unset Protect";
      break;
    case ARG_FORMAT:
      result = fwrite_format( fp, img );	s = "Format";
      break;
    case ARG_UNFORMAT:
      result = fwrite_unformat( fp, img );	s = "Unformat";
      break;
    case ARG_RENAME:
      strncpy( (char *)c, imagename, 17 );
      result = fwrite_name( fp, img, (char*)c );	s = "Rename image";
      break;
    }
				/* エラー表示 */
    switch( result ){
    case -1:  printf("This file maybe broken.\n");	break;
    case 0:   printf("%s complete.\n",s);		break;
    case 1:   printf("Image No. %d not exist.\n",img+1);break;
    case 2:   printf("Image No. %d is broken.\n",img+1);break;
    case 3:   printf("Seek Error\n");			break;
    case 4:   printf("Write error\n");			break;
    case 5:   printf("Image number over\n");		break;
    }
				/* エラー後の処理 */

    switch( busy ){
    case -1:					/* 新規ファイルの場合、終了 */
      ex_fclose(fp);
      break;
    case 0: case 1:				/* ドライブに設定してある */
      if( result==0 ){				/* ファイルの場合、更新   */
	switch( command ){
	case ARG_CREATE:
	  update_after_append_blank( busy );			break;
	case ARG_PROTECT:
	case ARG_UNPROTECT:
	  update_after_fwrite_protect( busy, img, (char*)c );	break;
	case ARG_FORMAT:
	  update_after_fwrite_format( busy, img );		break;
	case ARG_UNFORMAT:
	  /* not update */					break;
	case ARG_RENAME:
	  update_after_fwrite_name( busy, img, (char*)c );	break;
	}
      }else{					/* エラーの時は…困ったな */
	printf("Fatal error in File %s ( in DRIVE %d: )\n", filename, busy+1);
	printf("File %s maybe be broken.\n", filename );
      }
      break;
    }
    break;
  }

  return;
}



/*--------------------------------------------------------------*/
/* suspend <filename>						*/
/*	サスペンド						*/
/*--------------------------------------------------------------*/
static	void	monitor_suspend( void )
{
  char	*filename = NULL;

  if( exist_argv() ){
    filename = argv.str;
    shift();
    if( exist_argv() ) error();
  }else{
    error();
  }

  if( suspend( filename ) ) printf( "suspend succsess\n");
  else                      printf( "suspend failed\n");
  return;
}





/*--------------------------------------------------------------*/
/* snapshot							*/
/*	スクリーンスナップショットの保存			*/
/*--------------------------------------------------------------*/
static	void	monitor_snapshot( void )
{
  if( exist_argv() ){
    error();
  }

  if( save_screen_snapshot( ) == 0 ){
    printf( "save-snapshot failed\n" );
  }
  return;
}






/*--------------------------------------------------------------*/
/* loadfont <filename>						*/
/*	フォントファイルのロード				*/
/*--------------------------------------------------------------*/
static	void	monitor_loadfont( void )
{
  char	*filename = NULL;

  if( exist_argv() ){
    filename = argv.str;
    shift();
    if( exist_argv() ) error();
  }else{
    error();
  }

  if( memory_load_font( filename )==0 ){
    printf( "font-file \"%s\" load failed (...use system font)\n",filename);
  }
  redraw_screen( FALSE );
  return;
}








/*----------------------------------*/
static	void    monitor_misc(void)
{
/*
  int ch;
  extern const char *mixer_get_name(int ch);
  extern mixer_get_mixing_level(int ch);
    for( ch=0; ch<16 ; ch++ ){
      const char *name = mixer_get_name(ch);
      if(name) printf( "%d[ch] %s\t:%d\n", ch,name,mixer_get_mixing_level(ch));
    }
*/
#if 0
  FILE *fp;
  fp = ex_fopen( "log.main","wb");
 ex_fwrite( main_ram,            sizeof(byte),  0x0f000,  fp );
 ex_fwrite( main_high_ram,       sizeof(byte),  0x01000,  fp );
  ex_fclose(fp);

  fp = ex_fopen( "log.high","wb");
 ex_fwrite( &main_ram[0xf000],  sizeof(byte),   0x1000,  fp );
  ex_fclose(fp);

  fp = ex_fopen( "log.sub","wb");
 ex_fwrite( &sub_romram[0x4000], sizeof(byte),   0x4000,  fp );
  ex_fclose(fp);

  fp = ex_fopen( "log.vram","wb");
 ex_fwrite( main_vram,           sizeof(byte), 4*0x4000,  fp );
  ex_fclose(fp);
#endif

/*
  extern void monitor_fdc(void);
  monitor_fdc();
*/
}





#ifdef USE_GNU_READLINE

/*--------------------------------------------------------------*/
/* readline を使ってみよう。					*/
/*--------------------------------------------------------------*/

char *command_generator( char *text, int state );
char *set_arg_generator( char *text, int state );
char **fileman_completion( char *text, int start, int end );

static void initialize_readline( void )
{
  rl_readline_name = "QUASI88";	 /*よくわからんが ~/.inputrc に関係あるらしい*/
  rl_attempted_completion_function = (CPPFunction *)fileman_completion;
}

char **fileman_completion( char *text, int start, int end )
{
  char **matches = NULL;

  int i=0;
  char c;					/* "set " と入力された場合 */
  while( (c = rl_line_buffer[i]) ){
    if( c==' ' || c=='\t' ){ i++; continue; }
    else                   break;
  }
  if (strncmp( &rl_line_buffer[i], "set", 3) == 0  &&  start > (i+3) )
    matches = completion_matches( text, set_arg_generator );
  else

  if (start == 0)				/* 行頭での入力の場合 */
    matches = completion_matches( text, command_generator );

  return matches;
}

char *command_generator( char *text, int state )
{
  static int count, len;
  char *name;

  if( state == 0 ){		/* この関数、最初は state=0 で呼び出される */
    count = 0;			/* らしいので、その時に変数を初期化する。  */
    len = strlen (text);
  }

  while( count < countof(monitor_cmd) ){	/* コマンド名を検索 */

    name = monitor_cmd[count].cmd;
    count ++;

    if( strncmp( name, text, len ) == 0 ){
      char *p = ex_malloc( strlen(name) + 1 );
      if (p){
	strcpy( p, name );
      }
      return p;
    }
  }

  return NULL;
}

char *set_arg_generator( char *text, int state )
{
  static int count, len;	/* set コマンドが入力済みの場合 */
  char *name;

  if( state == 0 ){
    count = 0;
    len = strlen (text);
  }

  while( count < countof(monitor_variable) ){	/* 変数名を検索 */

    name = monitor_variable[count].var_name;
    count ++;

    if( strcmp( name, "" )==0 ) continue;

    if( strncmp( name, text, len ) == 0 ){
      char *p = ex_malloc( strlen(name) + 1 );
      if (p){
	strcpy( p, name );
      }
      return p;
    }
  }

  return NULL;
}
#endif



/****************************************************************/
/* デバッグ メイン処理						*/
/****************************************************************/

static	int	monitor_job = 0;

void	monitor_init( void )
{
  monitor_job = 0;

  save_dump_addr = -1;
  save_dump_bank = ARG_MAIN;
  save_dumpext_addr = -1;
  save_dumpext_bank = ARG_EXT0;
  save_dumpext_board = 1;
  save_disasm_addr = -1;


  xmame_sound_suspend();
  monitor_event_init();


  {
	/* 一番最初にモニターモードに入った時は、メッセージを表示 */

    static int enter_monitor_mode_first = TRUE;
    if( enter_monitor_mode_first ){
      printf("\n"
	     "*******************************************************************************\n"
	     "* QUASI88   - monitor mode -                                                  *\n"
	     "*                                                                             *\n"
	     "*    Enter  go   or g  : Return to emulator                                   *\n"
	     "*    Enter  menu or m  : Enter menu mode                                      *\n"
	     "*    Enter  quit or q  : Quit QUASI88                                         *\n"
	     "*                                                                             *\n"
	     "*    Enter  help or ?  : Display help                                         *\n"
	     "*******************************************************************************\n"
	     "\n" );

      enter_monitor_mode_first = FALSE;

#ifdef USE_GNU_READLINE
      initialize_readline();
#endif
    }
  }

  fflush(NULL);

  indicator_flag = 0;			/* 各種表示は消す	    */
}



void	monitor_main( void )
{
  int	i;

  while( emu_mode == MONITOR_MAIN ){

    switch( monitor_job ){

    case MONITOR_LINE_INPUT:

#ifndef USE_GNU_READLINE

      printf("QUASI88> ");
      if( fgets( buf, MAX_CHRS, stdin ) == NULL ){	/* ^D が入力されたら */
#ifndef IGNORE_CTRL_D					/* 強制的に終了。    */
	emu_mode = QUIT;				/* 回避方法がわからん*/
#else
	emu_mode = MONITOR;				/* IRIX/AIXは大丈夫? */
#endif
	break;
      }

#else
      {
	char *p, *chk;
	HIST_ENTRY *ph;
	p = readline( "QUASI88> " );			/* GNU readline の  */
	if( p==NULL ){					/* 仕様がいまいち   */
	  printf( "\n" );				/* わからん。       */
	  break;					/* man で斜め読み   */
	}else{						/* してみたが、英語 */
							/* は理解しがたい。 */
	  ph = previous_history();
	  if ( *p=='\0' && ph != NULL) {	/* リターンキーで直前の */
	    strncpy( buf, ph->line, MAX_CHRS-1 );	/*コマンドを実行 */
	  } else strncpy( buf, p, MAX_CHRS-1 );
	  buf[ MAX_CHRS-1 ] = '\0';

	  chk = p;	/* 空行じゃなければ履歴に残す */
	  while( *chk ){
	    if( *chk==' ' || *chk=='\t' ){ chk++;		continue; }
	    /* 同じコマンドは履歴に残さない */
	    else if (ph != NULL && strcmp(chk, ph->line) == 0)	break;
	    else			 { add_history( chk );  break;    }
	  }
	}
	ex_free( p );
	/* このあたりの処理は、peachさんにより改良されました */
      }
#endif

      getarg();						/* 引数を分解 */

      if( d_argc==0 ){					/* 空行の場合 */
	monitor_job = 0;
      }else{
	for( i=0; i<countof(monitor_cmd); i++ ){
	  if( strcmp( d_argv[0], monitor_cmd[i].cmd )==0 ) break;
	}
	if( i==countof(monitor_cmd) ){			/* 無効命令の場合 */
	  printf("Invalid command : %s\n",d_argv[0]);
	  monitor_job = 0;
	}else{						/* 引数が ? の場合 */
	  if( d_argc==2 && strcmp( d_argv[1], "?" )==0 ){
	    (monitor_cmd[i].help)();
	    monitor_job = 0;
	  }else{					/* 通常の命令の場合 */
	    monitor_job = monitor_cmd[i].job;
	    shift();
	    fflush(NULL);
	  }
	}
      }
      break;
      


    case MONITOR_HELP:
      monitor_job=0;
      monitor_help();
      break;
    case MONITOR_MENU:
      emu_mode = MENU;
      break;
    case MONITOR_QUIT:
      emu_mode = QUIT;
      break;
      
    case MONITOR_GO:
      monitor_job=0;
      monitor_go();
      break;
    case MONITOR_TRACE:
      monitor_job=0;
      monitor_trace();
      break;
    case MONITOR_STEP:
      monitor_job=0;
      monitor_step();
      break;
    case MONITOR_STEPALL:
      monitor_job=0;
      monitor_stepall();
      break;
    case MONITOR_BREAK:
      monitor_job=0;
      monitor_break();
      break;
      
      
    case MONITOR_READ:
      monitor_job=0;
      monitor_read();
      break;
    case MONITOR_WRITE:
      monitor_job=0;
      monitor_write();
      break;
    case MONITOR_DUMP:
      monitor_job=0;
      monitor_dump();
      break;
    case MONITOR_DUMPEXT:
      monitor_job=0;
      monitor_dumpext();
      break;
    case MONITOR_FILL:
      monitor_job=0;
      monitor_fill();
      break;
    case MONITOR_MOVE:
      monitor_job=0;
      monitor_move();
      break;
    case MONITOR_SEARCH:
      monitor_job=0;
      monitor_search();
      break;
    case MONITOR_IN:
      monitor_job=0;
      monitor_in();
      break;
    case MONITOR_OUT:
      monitor_job=0;
      monitor_out();
      break;
    case MONITOR_LOADMEM:
      monitor_job=0;
      monitor_loadmem();
      break;
    case MONITOR_SAVEMEM:
      monitor_job=0;
      monitor_savemem();
      break;
      
    case MONITOR_RESET:
      monitor_job=0;
      monitor_reset();
      break;
    case MONITOR_REG:
      monitor_job=0;
      monitor_reg();
      break;
    case MONITOR_DISASM:
      monitor_job=0;
      monitor_disasm();
      break;
      
    case MONITOR_SET:
      monitor_job=0;
      monitor_set();
      break;
    case MONITOR_SHOW:
      monitor_job=0;
      monitor_show();
      break;
    case MONITOR_REDRAW:
      monitor_job=0;
      redraw_screen( FALSE );
      break;
    case MONITOR_RESIZE:
      monitor_job=0;
      monitor_resize();
      break;
    case MONITOR_DRIVE:
      monitor_job=0;
      monitor_drive();
      break;
    case MONITOR_FILE:
      monitor_job=0;
      monitor_file();
      break;
    case MONITOR_SUSPEND:
      monitor_job=0;
      monitor_suspend();
      break;
    case MONITOR_SNAPSHOT:
      monitor_job=0;
      monitor_snapshot();
      break;
    case MONITOR_LOADFONT:
      monitor_job=0;
      monitor_loadfont();
      break;
      
      
      
    case MONITOR_MISC:
      monitor_job=0;
      monitor_misc();
      break;

    case MONITOR_FBREAK:
      monitor_job=0;
      monitor_fbreak();
      break;
    case MONITOR_TEXTSCR:
      monitor_job=0;
      monitor_textscr();
      break;
    case MONITOR_LOADBAS:
      monitor_job=0;
      monitor_loadbas();
      break;
    case MONITOR_SAVEBAS:
      monitor_job=0;
      monitor_savebas();
      break;

      
    default:
      monitor_job=0;
      printf("Internal Error\n");
      break;
    }

  }

  monitor_event_term();

  if( ! (emu_mode==QUIT || emu_mode==MONITOR || emu_mode==MENU || 
	 emu_mode==STEP ) ){
    xmame_sound_resume();

    joystick_restart();
  }
}







#if	defined( PIO_DISP ) || defined( PIO_FILE )
void	logpio( const char *format, ... )
{
  va_list  list;
  if( pio_debug==0 ) return;
#ifdef	PIO_DISP
  va_start( list, format );
  vfprintf( stdout, format, list );
  va_end( list );
  fflush(stdout);
#endif
#ifdef	PIO_FILE
  va_start( list, format );
  vfprintf( LOG, format, list );
  va_end( list );
  fflush(LOG);
#endif
}
#endif

#if	defined( FDC_DISP ) || defined( FDC_FILE )
void	logfdc( const char *format, ... )
{
  va_list  list;
  if( fdc_debug==0 ) return;
#ifdef	FDC_DISP
  va_start( list, format );
  vfprintf( stdout, format, list );
  va_end( list );
  fflush(stdout);
#endif
#ifdef	FDC_FILE
  va_start( list, format );
  vfprintf( LOG, format, list );
  va_end( list );
  fflush(LOG);
#endif
}
#endif


#if	defined( MAIN_DISP ) || defined( MAIN_FILE ) || defined( SUB_DISP ) || defined( SUB_FILE )
void	logz80( const char *format, ... )
{
  va_list  list;
#if	defined( MAIN_DISP ) || defined( SUB_DISP )
  va_start( list, format );
  vfprintf( stdout, format, list );
  va_end( list );
  fflush(stdout);
#endif
#if	defined( MAIN_FILE ) || defined( SUB_FILE )
  va_start( list, format );
  vfprintf( LOG, format, list );
  va_end( list );
  fflush(LOG);
#endif
}
#endif


#endif	/* USE_MONITOR */
