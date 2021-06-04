/************************************************************************/
/*									*/
/* ヘルプメッセージ (OS依存)						*/
/*	Q_TITLE, Q_VERSION は コンパイル時に(Makefileで)定義		*/
/*	それ以外のマクロは、全て initval.h に				*/
/*									*/
/*		getconf.c ファイル内から、include されている。		*/
/************************************************************************/

static	void	help_mess( void )
{
  fprintf
  (
   stdout,
   Q_TITLE " ver " Q_VERSION "  ---  PC-8801 emulator with SDL\n"
   "\n"
   "Usage: %s [-option1 [-option2...]] [ filename1 [n][filename2][m] ]\n"
   "  [filename1] = name of file to load as DRIVE 1:\n"
   "  [filename2] = name of file to load as DRIVE 2:\n"
   "  [n]         = image number of DRIVE 1:\n"
   "  [m]         = image number of DRIVE 2:\n"
   "  [-option]   =\n"
   "  ** BOOT UP CONFIG **\n"
   "    -n/-v1s/-v1h/-v2     - Select BASIC mode [%s]\n"
   "    -dipsw <num>         - Dip switch setting [0x%04x]\n"
   "    -baudrate <bps>      - Baud rate setting [%d]\n"
   "    -romboot/-diskboot   - Boot Select ROM or DISK [%s]\n"
   "    -4mhz/-8mhz          - Select CPU clock [%s]\n"
   "    -sd/-sd2             - Soundboard type is standard or II [%s]\n"
   "    -extram [<cards>]    - Number of extend ram card (0..8, 128KB/card) [%d]\n"
   "    -noextram            - Not use extram ( mean -extram 0 )\n"
   "    -jisho/-nojisho      - Use/Not use JISHO-ROM [%s]\n"
   "    -mouse/-nomouse/-joymouse/-joystick/-joykey\n"
   "                         - Use mouse / Not use / Use for joystick-mode /\n"
   "                           Use joystick (if compiled in) / Use joystick \n"
   "                           as key (if compiled in) [-nomouse]\n"
   "    -24k/-15k            - Set monitor frequency [-24k]\n"
   "    -analog/-digital     - Set monitor RGB analog or digital [-analog]\n"
   "  ** TAPE **\n"
   "    -tapeload <filename> - Set tape image for load (T88,CMT)\n"
   "    -tapesave <filename> - Set tape image for save (CMT)\n"
   "    -cmt_intr/-cmt_poll  - Use/Not use interrupt for tape-loading\n"
   "  ** GRAPHIC **\n"
   "    -frameskip <period>  - Number of frame skip [%d]\n"
   "    -autoskip            - Use auto frame skip\n"
   "    -full/-half          - Screen size is full or half [-full]\n"
   "    -width <x>/-height <y> - Set window size <x>*<y>\n"
   "    -interp/-nointerp    - Use / Not use reduce interpolation [-interp] \n"
   "    -interlace/-nointerlace  - Use / Not use interlace display [-nointerlace] \n"
   "    -dga/-nodga          - Fullscreen mode / Window mode [-nodga]\n"
   "    -hide_mouse          - Hide mouse cursor\n"
   "    -grab_mouse          - Grab mouse cursor\n"
   "  ** MISC **\n"
   "    -romdir <path>       - Set ROM image file path\n"
   "    -diskdir <path>      - Set DISK image file path\n"
   "    -compatrom <filename>- Specify ROM image file of P88SR.EXE\n"
   "    -setver <num>        - Set V1 mode version as <num>\n"
   "                           0..2=88/3=mkII/4=SR/5..7=FR/8=FH/9=FA..\n"
   "    -tenkey              - Convert from full-key 0-9 to ten-key 0-9\n"
   "    -cursor              - Covert from cursor-key to ten-key 2,4,6,8\n"
   "    -numlock             - Set software NumLock to ON\n"
   "    -f6 <func>/-f7 <func>/-f8 <func>/-f9 <func>/-f10 <func>\n"
   "                         - Assign F6..F10 key to function of <func>\n"
   "                           <func> = FRATE-UP/FRATE-DOWN/VOLUME-UP/VOLUME-DOWN/\n"
   "                                    PAUSE/RESIZE/NOWAIT/SPEED-UP/SPEED-DOWN/\n"
   "                                    MOUSE-HIDE/DGA/SNAPSHOT/SUSPEND/\n"
   "                                    IMAGE-NEXT1/IMAGE-PREV1/IMAGE-NEXT2/\n"
   "                                    IMAGE-PREV2/NUMLOCK/RESET/KANA/ROMAJI/\n"
   "                                    CAPS/KETTEI/HENKAN/ZENKAKU/PC\n"
   "    -romaji <type>       - Set ROMAJI-HENKAN type (0:egg/1:MS-IME/2:ATOK)\n"
   "                             need option '-f6..-f10 ROMAJI' for ROMAJI input\n"
   "    -menukey             - Assign F6-F10 Key for QUASI88 control\n"
   "    -kanjikey            - Assign F6-F10 Key for KANJI-input\n"
   "    -joyswap             - Swap Joystick Button A<-->B (with -joystick)\n"
   "    -joyassign <n>       - Key Asssign of joystick button (with -joykey) [1]\n"
   "                           <n> =  0: none             / 1: Z and X\n"
   "                                  2: space and return / 3: space and shift\n"
   "                                  4: Z and shift\n"
   "    -subload <wait>      - Set Sub-CPU load (0:no wait/(slow)1..60..(fast)) [6]\n"
   "    -timestop            - Freeze RealTimeClock\n"
   "    -record <filename>   - Write all key inputs to the file <filename>\n"
   "    -playback <filename> - Play back all key inputs from the file <filename>\n"
   "    -exchange            - Send a fake signal when exchange disks\n"
   "  ** EMULATOR **\n"
   "    -cpu <0/1/2/3>       - Main-Sub CPU control timing [%d]\n"
   "    -clock <rate>        - CPU clock MHz (0.1..100.0) [%6.4f]\n"
   "    -soundclock <rate>   - Soundchip clock MHz (0.1..100.0) [%6.4f]\n"
   "    -vsync <rate>        - VSYNC freq Hz (10.0..240.0) [%6.2f]\n"
   "    -waitfreq            - Set frequency Hz for wait (10.0..240.0) [%6.2f]\n"
   "    -nowait              - No wait ( ignore option '-waitfreq' )\n"
   "    -fdc_wait            - FDC wait mode effective\n"
   "    -load <wait>         - Set wait (0:no wait/(slow)1..1000..(fast)) [0]\n"
   "    -hsbasic             - High-speed basic mode\n"
   "  ** SYSTEM**\n"
   "    -help                - Print this help page\n"
   "    -verbose <level>     - Select debugging messages [0x%02x]\n"
   "                             0 - Silent           1 - Startup messages\n"
   "                             2 - Undef Z80 op     4 - Undecorded I/O\n" 
   "                             8 - PIO warning     16 - FDC disk error\n"
   "                            32 - wait error      64 - suspend info\n"
   "                           128 - sound info\n"
   "    -logo/-nologo        - display/not display TITLE LOGO\n"
   "    -english             - Menu mode display in English\n"
   "    -sleep/-nosleep      - Sleep / Not sleep during idle [-nosleep] \n"
   "    -button2menu         - press mouse-button-2 to menu mode\n"
   "    -menu                - start in menu mode\n"
#ifdef	USE_MONITOR
   "    -monitor             - start in monitor mode\n"
   "    -debug               - enable to go to monitor mode\n"
   "    -fdcdebug            - print FDC status\n"
#endif
   "    -resume              - resume QUASI88\n"
   "                                  ( state file is " QUASI88STATE_FILE " )\n"
   "    -resumefile <filename>  - resume QUASI88 ( state file is <filename> )\n"
   "    -resumeforce <filename> - resume QUASI88 ( state file is <filename> )\n"
   "                              resume if state-file-version mismatch\n"
   "\n"
   "Configuration:\n"
#ifdef	UNIX88
   "  <Configuration file>\n"
   "                        ... ${HOME}/" QUASI88RC_FILE "\n"
   "  <Suspend state file>\n"
   "                        ... ${HOME}/" QUASI88STATE_FILE "\n"
   "  <Directory of ROM IMAGE FILE>\n"
   "                        ... \"%s\"\n"
   "                              or ${QUASI88_ROM_DIR}\n"
   "                              or -romdir <path>\n"
   "  <Directory of DISK IMAGE FILE>\n"
   "                        ... \"%s\"\n"
   "                              or ${QUASI88_DISK_DIR}\n"
   "                              or -diskdir <path>\n"
   "                              or current directory\n"
#else	/* WIN88, other */
   "  <Configuration file>\n"
   "                        ... " QUASI88RC_FILE "\n"
   "  <Suspend state file>\n"
   "                        ... " QUASI88STATE_FILE "\n"
#endif
   "  <Max number of images in one file>\n"
   "                        ... %d images\n"
   ,

   command,
   (DEFAULT_BASIC==BASIC_AUTO)
     ? "AUTO"
     : (DEFAULT_BASIC==BASIC_N)
       ? "-n"
       : (DEFAULT_BASIC==BASIC_V1S)
         ? "-v1s"
         : (DEFAULT_BASIC==BASIC_V1H)
            ? "-v1h"
            : "-v2",
   DEFAULT_DIPSW,
   (DEFAULT_BAUDRATE==BAUDRATE_75 )
     ? 75
     : (DEFAULT_BAUDRATE==BAUDRATE_150 )
       ? 150
       : (DEFAULT_BAUDRATE==BAUDRATE_300 )
         ? 300
         : (DEFAULT_BAUDRATE==BAUDRATE_600 )
           ? 600
           : (DEFAULT_BAUDRATE==BAUDRATE_1200 )
             ? 1200
             : (DEFAULT_BAUDRATE==BAUDRATE_2400 )
               ? 2400
               : (DEFAULT_BAUDRATE==BAUDRATE_4800 )
                 ? 4800
                 : (DEFAULT_BAUDRATE==BAUDRATE_9600 )
                   ? 9600
                   : 19200,
   (DEFAULT_BOOT==BOOT_AUTO)
     ? "AUTO"
     : (DEFAULT_BOOT==BOOT_DISK)
       ? "-diskboot"
       : "-romboot",
   (DEFAULT_CLOCK==CLOCK_8MHZ) ? "-8mhz" : "-4mhz",
   (DEFAULT_SOUND==SOUND_I) ? "-sd" : "-sd2",
   DEFAULT_EXTRAM,
   (DEFAULT_JISHO) ? "-jisho" : "-nojisho",
   DEFAULT_FRAMESKIP, 
   DEFAULT_CPU,
   DEFAULT_CPU_CLOCK_MHZ,
   DEFAULT_SOUND_CLOCK_MHZ,
   DEFAULT_VSYNC_FREQ_HZ,
   DEFAULT_WAIT_FREQ_HZ,

   DEFAULT_VERBOSE,
#ifdef	UNIX88
   (strlen(ROM_DIR)==0) ? "current directory" : ROM_DIR,
   (strlen(DISK_DIR)==0)? "current directory" : DISK_DIR,
#endif
   MAX_NR_IMAGE
   );
}
