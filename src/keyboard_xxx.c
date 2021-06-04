/************************************************************************/
/*									*/
/* キー入力、マウス入力、その他の入力 (OS依存)				*/
/*									*/
/* 【関数】								*/
/*									*/
/* void	scan_keyboard( int key_check_only )				*/
/*	キー入力を読みとり、key_scan[] ワークを設定する。		*/
/*	この key_scan[] ワークは、そのまま PC8801の I/O ポート 00H～0FH	*/
/*	に対応する。							*/
/*									*/
/* void scan_mouse( void )						*/
/*	マウス入力を読みとり、座標を mouse_x、mouse_y に、ボタンの押下	*/
/*	状況を sound_reg[ 0EH ]～[ 0FH ] に設定する。			*/
/*	ジョイスティック入力を sound_reg[ 0EH ]～[ 0FH ] に設定する。	*/
/*									*/
/* void	check_mouse( void )						*/
/*	これはOS非依存。マウスの移動量を sound_reg[ 0FH ] に設定する。	*/
/*									*/
/* void check_focus( void )						*/
/*	その他の入力を(必要に応じて)チェックする。			*/
/*		X の フォーカスをチェックし、フォーカスが外れたら、	*/
/*		一時停止させたり、ウインドウマネージャーによって	*/
/*		ウインドウを閉じられたら、強制終了したりしている。	*/
/*									*/
/* void check_expose( void )						*/
/*	画面露出チェック						*/
/*									*/
/* void	set_tenkey_emu_effective( void );				*/
/* void	set_tenkey_emu_invalid( void );					*/
/* void	set_cursor_emu_effective( void );				*/
/* void	set_cursor_emu_invalid( void );					*/
/* void	set_numlock_emu_effective( void );				*/
/* void	set_numlock_emu_invalid( void );				*/
/*	キーの割り当てを変更する関数と戻す関数				*/
/*									*/
/************************************************************************/

// #include <SDL.h>

#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "keyboard.h"
#include "joystick.h"

#include "sound.h"	/* need by scan_mouse()		*/
#include "graph.h"	/* screen_size			*/

#include "drive.h"

#include "emu.h"
#include "device.h"
#include "indicator.h"
#include "pause.h"
#include "screen.h"
#include "suspend.h"
#include "snapshot.h"

#include "pg.h"

#include "menu.h"

byte	key_scan[0x10];			/* IN[00-0F] キースキャン	*/

int	tenkey_emu      = FALSE;	/* 10 KEY の入力を生成	*/
int	cursor_emu      = FALSE;	/* Cursor KEY -> 10 KEY (by funa) */
int	numlock_emu     = FALSE;	/* software NUM lock	*/

int	mouse_mode	= 0;		/* マウス 0:No 1:Yes 2:Joy  */

int	cursor_assign	= FALSE;	/* Cursor Key -> 任意のキー */

typedef	struct
{
  byte	key_code;		/* 置き換えるキーコード */
  int	shift_key_on;		/* Shiftが必要か? */
}CURSOR_ASSIGN_TABLE;
CURSOR_ASSIGN_TABLE	cursor_assign_table[CURSOR_ASSIGN_End] =
{
  { 0x00, FALSE, },		/* Cursor Up */
  { 0x00, FALSE, },		/* Cursor Left */
  { 0x00, FALSE, },		/* Cursor Right */
  { 0x00, FALSE, },		/* Cursor Down */
};

int	function_f6  = FN_FUNC;		/* F6-F10 KEY の機能	*/
int	function_f7  = FN_FUNC;
int	function_f8  = FN_FUNC;
int	function_f9  = FN_FUNC;
int	function_f10 = FN_FUNC;

int	romaji_type = 0;		/* ローマ字変換のタイプ	*/

FKEY_TABLE	fkey_table[ FN_end ] =
{
  { FN_FUNC,        NULL,	   NULL,          },
  { FN_FRATE_UP,    "FRATE-UP",    "frate-up",    },
  { FN_FRATE_DOWN,  "FRATE-DOWN",  "frate-down",  },
  { FN_VOLUME_UP,   "VOLUME-UP",   "volume-up",   },
  { FN_VOLUME_DOWN, "VOLUME-DOWN", "volume-down", },
  { FN_PAUSE,       "PAUSE",       "pause",       },
  { FN_RESIZE,      "RESIZE",      "resize",      },
  { FN_NOWAIT,      "NOWAIT",      "nowait",      },
  { FN_SPEED_UP,    "SPEED-UP",    "speed-up",    },
  { FN_SPEED_DOWN,  "SPEED-DOWN",  "speed-down",  },
  { FN_MOUSE_HIDE,  "MOUSE-HIDE",  "mouse-hide",  },
  { FN_DGA,         "DGA",         "dga",         },
  { FN_SNAPSHOT,    "SNAPSHOT",    "snapshot",    },
  { FN_SUSPEND,     "SUSPEND",     "suspend",     },
  { FN_IMAGE_NEXT1, "IMAGE-NEXT1", "image-next1", },
  { FN_IMAGE_PREV1, "IMAGE-PREV1", "image-prev1", },
  { FN_IMAGE_NEXT2, "IMAGE-NEXT2", "image-next2", },
  { FN_IMAGE_PREV2, "IMAGE-PREV2", "image-prev2", },
  { FN_NUMLOCK,     "NUMLOCK",     "numlock",     },
  { FN_RESET,       "RESET",       "reset",       },
  { FN_KANA,        "KANA",        "kana",        },
  { FN_ROMAJI,      "ROMAJI",      "romaji",      },
  { FN_CAPS,        "CAPS",        "caps",        },
  { FN_KETTEI,      "KETTEI",      "kettei",      },
  { FN_HENKAN,      "HENKAN",      "henkan",      },
  { FN_ZENKAKU,     "ZENKAKU",     "zenkaku",     },
  { FN_PC,          "PC",          "pc",          },
};

char	*file_rec	= NULL;		/* キー入力記録のファイル名 */
char	*file_pb	= NULL;		/* キー入力再生のファイル名 */

static	FILE *fp_rec;
static	FILE *fp_pb;

static struct {				/* キー入力記録構造体		*/
  Uchar	key[16];			/*	I/O 00H～0FH 		*/
   char	mouse_xh;			/*	マウス x 上位		*/
  Uchar	mouse_xl;			/*	マウス x 下位		*/
   char	mouse_yh;			/*	マウス y 上位		*/
  Uchar	mouse_yl;			/*	マウス y 下位		*/
  Uchar	joy[2];				/*	SOUND SSG 0EH～0FH	*/
   char	image[2];			/*	イメージNo -1空,0同,1～	*/
} key_record;

/*----------------------------------------------------------------------*/
/* キーコード変換表							*/
/*----------------------------------------------------------------------*/

#define Bt7	0x80
#define Bt6	0x40
#define Bt5	0x20
#define Bt4	0x10
#define Bt3	0x08
#define Bt2	0x04
#define Bt1	0x02
#define Bt0	0x01

#define P0	0
#define P1	1
#define P2	2
#define P3	3
#define P4	4
#define P5	5
#define P6	6
#define P7	7
#define P8	8
#define P9	9
#define Pa	10
#define Pb	11
#define Pc	12
#define Pd	13
#define Pe	14


/* miscellany_keys[n][2] の nの値 */

#define	code_kettei	0x0022
#define	code_henkan	0x0023
#define	code_zenkaku	0x002a
#define	code_pc		0x00e8


#define	shift_on()	key_scan[P8] &= ~Bt6
#define	shift_off()	key_scan[P8] |=  Bt6
#define	space_on()	key_scan[P9] &= ~Bt6
#define	space_off()	key_scan[P9] |=  Bt6
#define	bs_on()		key_scan[P8] &= ~Bt3
#define	bs_off()	key_scan[P8] |=  Bt3

/* 左右シフト、左右リターン は現在未対応	*/
/* ともに、シフト、リターン に割り振られる	*/




/* 0xff?? 系列 (特殊キー) for SDL */

byte	miscellany_keys[][2] =
{
/*00                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*08    BS       TAB               HCLR              RET                    */
     {Pc,Bt5},{Pa,Bt0},{ 0,  0},{P8,Bt0},{ 0,  0},{P1,Bt7},{ 0,  0},{ 0,  0},
/*10                               STOP              COPY                   */
     { 0,  0},{ 0,  0},{ 0,  0},{P9,Bt0},{ 0,  0},{Pa,Bt4},{ 0,  0},{ 0,  0},
/*18                               ESC                                      */
     { 0,  0},{ 0,  0},{ 0,  0},{P9,Bt7},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*20                     決定     変換                                カナ  */
     { 0,  0},{ 0,  0},{Pd,Bt1},{Pd,Bt0},{ 0,  0},{ 0,  0},{ 0,  0},{P8,Bt5},
/*28                     全角                                               */
     { 0,  0},{ 0,  0},{Pd,Bt3},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*30                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*38                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*40                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*48                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*50    H CLR    LEFT     UP       RIGHT    DOWN     R UP     R DN     END  */
     {P8,Bt0},{Pa,Bt2},{P8,Bt1},{P8,Bt2},{Pa,Bt1},{Pb,Bt0},{Pb,Bt1},{Pa,Bt3},
/*58                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*60             COPY              INS                                      */
     { 0,  0},{Pa,Bt4},{ 0,  0},{Pc,Bt6},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*68                      HELP     STOP                                     */
     { 0,  0},{ 0,  0},{Pa,Bt3},{P9,Bt0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*70                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*78                                                          kana          */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{Pc,Bt7},
/*80                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*88                                                 RET                    */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{P1,Bt7},{ 0,  0},{ 0,  0},
/*90                                                 7[10]    4[10]    8[10]*/
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{P0,Bt7},{P0,Bt4},{P1,Bt0},
/*98    6[10]    2[10]    9[10]    3[10]    1[10]    5[10]    0[10]    .[10]*/
     {P0,Bt6},{P0,Bt2},{P1,Bt1},{P0,Bt3},{P0,Bt1},{P0,Bt5},{P0,Bt0},{P1,Bt6},
/*A0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*A8                      *[10]    +[10]    ,[10]    -[10]    .[10]    /[10]*/
     { 0,  0},{ 0,  0},{P1,Bt2},{P1,Bt3},{P1,Bt5},{Pa,Bt5},{P1,Bt6},{Pa,Bt6},
/*B0    0[10]    1[10]    2[10]    3[10]    4[10]    5[10]    6[10]    7[10]*/
     {P0,Bt0},{P0,Bt1},{P0,Bt2},{P0,Bt3},{P0,Bt4},{P0,Bt5},{P0,Bt6},{P0,Bt7},
/*B8    8[10]    9[10]                               =[10]    f1       f2   */
     {P1,Bt0},{P1,Bt1},{ 0,  0},{ 0,  0},{ 0,  0},{P1,Bt4},{P9,Bt1},{P9,Bt2},
/*C0    f3       f4       f5       f6       f7       f8       f9       f10  */
     {P9,Bt3},{P9,Bt4},{P9,Bt5},{P9,Bt1},{P9,Bt2},{P9,Bt3},{P9,Bt4},{P9,Bt5},
/*C8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*D0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*D8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*E0           L SHIFT  R SHIFT    CTRL     CTRL     CAPS              GRPH */
     { 0,  0},{P8,Bt6},{P8,Bt6},{P8,Bt7},{P8,Bt7},{Pa,Bt7},{ 0,  0},{P8,Bt4},
/*E8    GRPH     GRPH      PC                                               */
     {P8,Bt4},{P8,Bt4},{Pd,Bt2},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*F0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*F8                                                                   DEL  */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{Pc,Bt7},

/*00    0[10]    1[10]    2[10]    3[10]    4[10]    5[10]    6[10]    7[10]*/
     {P0,Bt0},{P0,Bt1},{P0,Bt2},{P0,Bt3},{P0,Bt4},{P0,Bt5},{P0,Bt6},{P0,Bt7},
/*08    8[10]    9[10]    .[10]    /[10]    *[10]    -[10]    +[10]  ret[10]*/
     {P1,Bt0},{P1,Bt1},{P1,Bt6},{Pa,Bt6},{P1,Bt2},{Pa,Bt5},{P1,Bt3},{ 0,  0},
/*10    =[10]    UP      DOWN    RIGHT      LEFT     INS      HOME     HELP */
     {P1,Bt4},{P8,Bt1},{Pa,Bt1},{P8,Bt2},{Pa,Bt2},{Pc,Bt6},{P8,Bt0},{Pa,Bt3},
/*18    R UP    R DN      f1       f2       f3       f4       f5       f6   */
     {Pb,Bt0},{Pb,Bt1},{P9,Bt1},{P9,Bt2}, {P9,Bt3},{P9,Bt4},{P9,Bt5},{P9,Bt1},
/*20    f7      f8        f9       f10                                      */
     {P9,Bt2},{P9,Bt3},{P9,Bt4},{P9,Bt5},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*28                                                CAPS              SHIFT */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{Pa,Bt7},{ 0,  0},{P8,Bt6},
/*30   SHIFT    CTRL     CTRL      PC      GRPH     GRPH     GRPH           */
     {P8,Bt6},{P8,Bt7},{P8,Bt7},{Pd,Bt2},{P8,Bt4},{P8,Bt4},{P8,Bt4},{ 0,  0},
/*38                              HELP     COPY              STOP           */
     { 0,  0},{ 0,  0},{ 0,  0},{Pa,Bt3},{Pa,Bt4},{ 0,  0},{P9,Bt0},{ 0,  0},
/*                                                                          */
     { 0,  0},{ 0,  0},{ 0,  0},
};

/* 0x00?? 系列(一般キー) */

byte	latin1_keys[][2] =
{
/*00                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*08                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*10                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*18                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*20    SPACE    !        "        #        $        %        &        ^    */
     {P9,Bt6},{P6,Bt1},{P6,Bt2},{P6,Bt3},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P5,Bt6},
/*28    (        )        *        +        ,        -        .        /    */
     {P7,Bt0},{P7,Bt1},{P7,Bt2},{P7,Bt3},{P7,Bt4},{P5,Bt7},{P7,Bt5},{P7,Bt6},
/*30    0        1        2        3        4        5        6        7    */
     {P6,Bt0},{P6,Bt1},{P6,Bt2},{P6,Bt3},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P6,Bt7},

#ifdef	UNIX88
/*38    8        9        :        ;        <        ;        >        ?    */
     {P7,Bt0},{P7,Bt1},{P7,Bt2},{P7,Bt3},{P7,Bt4},{P7,Bt3},{P7,Bt5},{P7,Bt6},
#else
/*38    8        9        ;        :        <        ;        >        ?    */
     {P7,Bt0},{P7,Bt1},{P7,Bt3},{P7,Bt2},{P7,Bt4},{P7,Bt3},{P7,Bt5},{P7,Bt6},
#endif

/*40    @        A        B        C        D        E        F        G    */
     {P2,Bt0},{P2,Bt1},{P2,Bt2},{P2,Bt3},{P2,Bt4},{P2,Bt5},{P2,Bt6},{P2,Bt7},
/*48    H        I        J        K        L        M        N        O    */
     {P3,Bt0},{P3,Bt1},{P3,Bt2},{P3,Bt3},{P3,Bt4},{P3,Bt5},{P3,Bt6},{P3,Bt7},
/*50    P        Q        R        S        T        U        V        W    */
     {P4,Bt0},{P4,Bt1},{P4,Bt2},{P4,Bt3},{P4,Bt4},{P4,Bt5},{P4,Bt6},{P4,Bt7},
/*58    X        Y        Z        [        \        ]        ^        _    */
     {P5,Bt0},{P5,Bt1},{P5,Bt2},{P5,Bt3},{P5,Bt4},{P5,Bt5},{P5,Bt6},{P7,Bt7},
/*60    `        a        b        c        d        e        f        g    */
     {P2,Bt0},{P2,Bt1},{P2,Bt2},{P2,Bt3},{P2,Bt4},{P2,Bt5},{P2,Bt6},{P2,Bt7},
/*68    h        i        j        k        l        m        n        o    */
     {P3,Bt0},{P3,Bt1},{P3,Bt2},{P3,Bt3},{P3,Bt4},{P3,Bt5},{P3,Bt6},{P3,Bt7},
/*70    p        q        r        s        t        u        v        w    */
     {P4,Bt0},{P4,Bt1},{P4,Bt2},{P4,Bt3},{P4,Bt4},{P4,Bt5},{P4,Bt6},{P4,Bt7},
/*78    x        y        z        {        |        }        ~             */
     {P5,Bt0},{P5,Bt1},{P5,Bt2},{P5,Bt3},{P5,Bt4},{P5,Bt5},{P5,Bt6},{ 0,  0},
/*80                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*88                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*90                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*98                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*A0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*A8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*B0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*B8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*C0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*C8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*D0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*D8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*E0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*E8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*F0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*F8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
};

/* 0x04?? 系列 (カナキー) */

byte	katakana_keys[][2] =
{
/*00                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*08                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*10                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*18                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*20                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*28                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*30                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*38                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*40                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*48                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*50                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*58                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*60                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*68                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*70                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*78                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*80                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*88                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*90                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*98                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*A0             。       「       」       、       ・       ヲ       ァ   */
     { 0,  0},{P7,Bt5},{P5,Bt3},{P5,Bt5},{P7,Bt4},{P7,Bt6},{P6,Bt0},{P3,Bt3},
/*A8    ィ       ゥ       ェ       ォ       ャ       ュ       ョ       ッ   */
     {P2,Bt5},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P6,Bt7},{P7,Bt0},{P7,Bt1},{P5,Bt2},
/*B0    ー       ア       イ       ウ       エ       オ       カ       キ   */
     {P5,Bt4},{P6,Bt3},{P2,Bt5},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P4,Bt4},{P2,Bt7},
/*B8    ク       ケ       コ       サ       シ       ス       セ       ソ   */
     {P3,Bt0},{P7,Bt2},{P2,Bt2},{P5,Bt0},{P2,Bt4},{P4,Bt2},{P4,Bt0},{P2,Bt3},
/*C0    タ       チ       ツ       テ       ト       ナ       ニ       ヌ   */
     {P4,Bt1},{P2,Bt1},{P5,Bt2},{P4,Bt7},{P4,Bt3},{P4,Bt5},{P3,Bt1},{P6,Bt1},
/*C8    ネ       ノ       ハ       ヒ       フ       ヘ       ホ       マ   */
     {P7,Bt4},{P3,Bt3},{P2,Bt6},{P4,Bt6},{P6,Bt2},{P5,Bt6},{P5,Bt7},{P3,Bt2},
/*D0    ミ       ム       メ       モ       ヤ       ユ       ヨ       ラ   */
     {P3,Bt6},{P5,Bt5},{P7,Bt6},{P3,Bt5},{P6,Bt7},{P7,Bt0},{P7,Bt1},{P3,Bt7},
/*D8    リ       ル       レ       ロ       ワ       ン       ゛       ゜   */
     {P3,Bt4},{P7,Bt5},{P7,Bt3},{P7,Bt7},{P6,Bt0},{P5,Bt1},{P2,Bt0},{P5,Bt3},
/*E0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*E8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*F0                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
/*F8                                                                        */
     { 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
};



/*----------------------------------------------------------------------*/
/* カーソルキーを テンキー[2][4][6][8] に割り当てる関数 と 解除する関数	*/
/*----------------------------------------------------------------------*/
/*
   以下のコードは、カーソルキーをテンキーに見立てるモード
     (-cursor オプションをつけて起動した時) の処理で、
     舟橋 啓さん [funa@am.ics.keio.ac.jp] によるものです。
  	DynaBook SS3010の様なノートのために,
	[右 Shift] -> [9]
	[\]        -> [7] に割り当てています.
	ノートでの使用を考えているので,適当なキーの場所が無い
	[1],[3] のキーへの割り当ては考えていません.
	"\" キーへの割り当ては latin_keys[0][0]に対して
	行っているので,他のキーも影響を受けてしまうかもしれません.

   当初は、void	keyboard_init( void ) 関数の中に組み込まれていましたが、
   メニューモードやモニターモードなどで変更出来るように、
   処理の割り当て関数、解除関数という形に独立させました。
   ( ver 0.2 以降より )
*/

void	set_cursor_emu_effective( void )
{
}

void	set_cursor_emu_invalid( void )
{
}


/*----------------------------------------------------------------------*/
/* フルキー側[0]～[9] が押された場合に、テンキーに対応させる／戻す関数	*/
/*----------------------------------------------------------------------*/
void	set_tenkey_emu_effective( void )
{
}

void	set_tenkey_emu_invalid( void )
{

}



/*----------------------------------------------------------------------*/
/* ソフトウェア NUL Lock を有効にする関数／無効にする関数		*/
/*----------------------------------------------------------------------*/
void	set_numlock_emu_effective( void )
{
}

void	set_numlock_emu_invalid( void )
{
}


/*----------------------------------------------------------------------*/
/* カーソルキーを任意のキーに置き換える関数 / 元に戻す関数		*/
/*			これらコードは、floiさんにより提供されました。	*/
/*----------------------------------------------------------------------*/
void	set_cursor_assign_effective( void )
{
}

void	set_cursor_assign_invalid( void )
{

}

void	set_cursor_assign_key( int index, byte new_code )
{
}

byte	get_cursor_assign_key( int index )
{

}



/*----------------------------------------------------------------------*/
/* ローマ字処理								*/
/*----------------------------------------------------------------------*/
#include "romaji.c"

/*----------------------------------------------------------------------*/
/* ファンクションキー f6～f10 が押された時に、特殊な動作をする機能関数	*/
/*----------------------------------------------------------------------*/
static	int	function_key( int fkey, int on )
{
  return 0;
}



/************************************************************************/
/* キースキャン関数	(1/60sec毎にスキャンする)	OS依存		*/
/************************************************************************/

void	keyboard_init( void )
{
	int i;
	
	for( i=0; i<0x10; i++ )  key_scan[i]=0xff;

}

static void scan_key_sub(int type, int key)
{
 
}

int Caps_Count = 0;
int Kana_Count = 0;
int Caps_Count_s = 0;
int Kana_Count_s = 0;
int SoftKB = 0;
int SoftKB_Count = 0;
int AnalogX = 0;
int AnalogY = 0;

void	scan_keyboard( int key_check_only )
{
	int i,x,y;
	readpad();
	if(SoftKB){
		key_scan[KeyInput[SoftKBSet].Port] |=  KeyInput[SoftKBSet].Bit;//キー選択する前にOFFにしておく
	}
#ifndef KeyConfig
	if (now_pad & CTRL_UP)
		key_scan[P1] &= ~Bt0;
	else
		key_scan[P1] |=  Bt0;
	
	if (now_pad & CTRL_DOWN)
		key_scan[P0] &= ~Bt2;
	else
		key_scan[P0] |=  Bt2;
		
	if (now_pad & CTRL_LEFT)
		key_scan[P0] &= ~Bt4;
	else
		key_scan[P0] |=  Bt4;
	
	if (now_pad & CTRL_RIGHT)
		key_scan[P0] &= ~Bt6;
	else
		key_scan[P0] |=  Bt6;
				
	
	if (now_pad & CTRL_CIRCLE)
		key_scan[P9] &= ~Bt6;
	else
		key_scan[P9] |=  Bt6;

	if (now_pad & CTRL_CROSS)
		key_scan[P9] &= ~Bt7;
	else
		key_scan[P9] |=  Bt7;

	if (now_pad & CTRL_TRIANGLE)
		key_scan[P1] &= ~Bt7;
	else
		key_scan[P1] |=  Bt7;

//	if (now_pad & CTRL_SQUARE)

	if (now_pad & CTRL_LTRIGGER)
		menu_psp(1);// PoewrOn TRUE

//	if (now_pad & CTRL_RTRIGGER)
#else

	if(!SoftKB){
		if (now_pad & CTRL_UP)
			key_scan[PSPControl[Control_Up].Port] &= ~PSPControl[Control_Up].Bit;
		else
			key_scan[PSPControl[Control_Up].Port] |=  PSPControl[Control_Up].Bit;
		
		if (now_pad & CTRL_DOWN)
			key_scan[PSPControl[Control_Down].Port] &= ~PSPControl[Control_Down].Bit;
		else
			key_scan[PSPControl[Control_Down].Port] |=  PSPControl[Control_Down].Bit;
			
		if (now_pad & CTRL_LEFT)
			key_scan[PSPControl[Control_Left].Port] &= ~PSPControl[Control_Left].Bit;
		else
			key_scan[PSPControl[Control_Left].Port] |=  PSPControl[Control_Left].Bit;
		
		if (now_pad & CTRL_RIGHT)
			key_scan[PSPControl[Control_Right].Port] &= ~PSPControl[Control_Right].Bit;
		else
			key_scan[PSPControl[Control_Right].Port] |=  PSPControl[Control_Right].Bit;
	}
	
	if (now_pad & CTRL_CROSS)
		key_scan[PSPControl[Control_Cross].Port] &= ~PSPControl[Control_Cross].Bit;
	else
		key_scan[PSPControl[Control_Cross].Port] |=  PSPControl[Control_Cross].Bit;

	if (now_pad & CTRL_TRIANGLE)
		key_scan[PSPControl[Control_Triangle].Port] &= ~PSPControl[Control_Triangle].Bit;
	else
		key_scan[PSPControl[Control_Triangle].Port] |=  PSPControl[Control_Triangle].Bit;

	if (now_pad & CTRL_SQUARE)
		key_scan[PSPControl[Control_Square].Port] &= ~PSPControl[Control_Square].Bit;
	else
		key_scan[PSPControl[Control_Square].Port] |=  PSPControl[Control_Square].Bit;

	if (now_pad & CTRL_LTRIGGER)
		menu_psp(1);// PoewrOn TRUE

#ifdef SoftwareKeybord
	if (now_pad & CTRL_RTRIGGER){
		if(SoftKB_Count == 0){
			SoftKB = !SoftKB;
			if(!SoftKB) KeyConfigKBErace();
			SoftKB_Count = 1;
		}
	}else{
		SoftKB_Count = 0;
	}

#else

	if (now_pad & CTRL_RTRIGGER)
		key_scan[PSPControl[Control_R].Port] &= ~PSPControl[Control_R].Bit;
	else
		key_scan[PSPControl[Control_R].Port] |=  PSPControl[Control_R].Bit;

#endif

	if (now_pad & CTRL_START)
		key_scan[PSPControl[Control_Start].Port] &= ~PSPControl[Control_Start].Bit;
	else
		key_scan[PSPControl[Control_Start].Port] |=  PSPControl[Control_Start].Bit;

	if (now_pad & CTRL_SELECT)
		key_scan[PSPControl[Control_Select].Port] &= ~PSPControl[Control_Select].Bit;
	else
		key_scan[PSPControl[Control_Select].Port] |=  PSPControl[Control_Select].Bit;

#ifdef SoftwareKeybord
	if(SoftKB){
		x = 0;
		y = 0;
		if(now_pad & CTRL_UP){
			if(AnalogY == 0) y -= 1;
			AnalogY += 1;
		}else if(now_pad & CTRL_DOWN){
			if(AnalogY == 0) y += 1;
			AnalogY += 1;
		}else{
			AnalogY = 0;
		}
		if(now_pad & CTRL_LEFT){
			if(AnalogX == 0) x -= 1;
			AnalogX += 1;
		}else if(now_pad & CTRL_RIGHT){
			if(AnalogX == 0) x += 1;
			AnalogX += 1;
		}else{
			AnalogX = 0;
		}
		if(AnalogY > 5) AnalogY = 0;
		if(AnalogX > 5) AnalogX = 0;

		SoftKBSet += (x + (y * KeyCol));
		while(KeyBorad[KeyboradMap() + SoftKBSet] == ""){
			SoftKBSet += (x + (y * KeyCol));
			if(SoftKBSet < 0) SoftKBSet = 0;
			if(SoftKBSet > KeyCol * 5 + 9) SoftKBSet = KeyCol * 5 + 9;
		}
		if(SoftKBSet < 0) SoftKBSet = 0;
		if(SoftKBSet > KeyCol * 5 + 9) SoftKBSet = KeyCol * 5 + 9;

		KeyConfigKBDraw(1);//SoftKB描画
		SHIFT = 0;
		shift_off();//とりあえずSHIFT OFF
		if (now_pad & CTRL_CIRCLE){
			key_scan[KeyInput[SoftKBSet].Port] &= ~KeyInput[SoftKBSet].Bit;
			if(SoftKBSet == KeyCol * 3 + 1 ){
				if(Caps_Count_s == 0){
					CAPS = !CAPS;
					CAPSPUT();
					Caps_Count_s++;
				}
			}
			if(SoftKBSet == KeyCol * 5     ){
				if(Kana_Count_s == 0){
					KANA = !KANA;
					KANAPUT();
					Kana_Count_s++;
				}
			}
			if((SoftKBSet == KeyCol + 15) || (SoftKBSet == 7) || (SoftKBSet == 8) || (SoftKBSet == 9) || (SoftKBSet == 10) || (SoftKBSet == 11) || (SoftKBSet == KeyCol * 4)){
				SHIFT = 1;
				shift_on();
			}
		}else{
			Caps_Count_s = 0;
			Kana_Count_s = 0;
		}
	}else{
#endif
		if (now_pad & CTRL_CIRCLE)
			key_scan[PSPControl[Control_Circle].Port] &= ~PSPControl[Control_Circle].Bit;
		else
			key_scan[PSPControl[Control_Circle].Port] |=  PSPControl[Control_Circle].Bit;
#ifdef SoftwareKeybord
	}
#endif

#ifdef SoftwareKeybord
	if(!SoftKB){
#endif
		SHIFT = 0;
		shift_off();//とりあえずSHIFT OFF
#ifdef SoftwareKeybord
	}
#endif
	for(i = 0;i < Control_End;i++){
		if(now_pad & Control_val[i]){
			if(i == (INS_KEY - 1) || i == (F6_KEY - 1) || i == (F7_KEY - 1) || i == (F8_KEY - 1) || i == (F9_KEY - 1) || i == (F10_KEY - 1)){
				SHIFT = 1;
				shift_on();
			}
		}
	}
	if((now_pad & Control_val[SHIFT_KEY - 1]) && ((SHIFT_KEY - 1) >= 0)){
		SHIFT = 1;
		shift_on();
	}
	if((now_pad & Control_val[CAPS_KEY - 1]) &&( (CAPS_KEY - 1) >= 0)){
		if(Caps_Count == 0){
			CAPS = !CAPS;
			CAPSPUT();
			Caps_Count++;
		}
	}else{
		Caps_Count = 0;
	}
	if((now_pad & Control_val[KANA_KEY - 1]) && ((KANA_KEY - 1) >= 0)){
		if(Kana_Count == 0){
			KANA = !KANA;
			KANAPUT();
			Kana_Count++;
		}
	}else{
		Kana_Count = 0;
	}

	CAPSPUT();
	KANAPUT();
	
	
#endif

}

void KANAPUT()
{
	if(KANA == 1){
		key_scan[P8] &= ~Bt5;
		if(screen_size == SCREEN_SIZE_HALF) pgPrint_drawbg(0,5,0xFFFF,0,"KANA");
	}else{
		key_scan[P8] |= Bt5;
		if(screen_size == SCREEN_SIZE_HALF) pgPrint_drawbg(0,5,0xFFFF,0,"    ");
	}
}

void CAPSPUT()
{
	if(CAPS == 1){
		key_scan[Pa] &= ~Bt7;
		if(screen_size == SCREEN_SIZE_HALF) pgPrint_drawbg(0,4,0xFFFF,0,"CAPS");
	}else{
		key_scan[Pa] |= Bt7;
		if(screen_size == SCREEN_SIZE_HALF) pgPrint_drawbg(0,4,0xFFFF,0,"    ");
	}
}

void SetKeyConfig(int Button ,int Port ,int Bit){
	PSPControl[Button].Port = Port;
	PSPControl[Button].Bit = Bit;
}

/************************************************************************/
/* マウススキャン関数							*/
/************************************************************************/

int	enable_b2menu = FALSE;		/* 中クリックでメニューモードへ	*/

	int	mouse_x = 0;		/* 実際のマウス座標 x	     */
	int	mouse_y = 0;		/*         〃       y	     */
static	int	mouse_image_x = 0;	/* 仮想的なマウス座標 x	     */
static	int	mouse_image_y = 0;	/*          〃        y	     */
	int	mouse_dx = 0;		/* マウス座標増分(0～±127)x */
	int	mouse_dy = 0;		/*          〃             y */


#define	MOUSE88_KEEP	0
#define	MOUSE88_PRESS	1
#define	MOUSE88_RELEASE	2

static struct{

  int	moved;		/* TRUE 移動あり FALSE 移動なし			*/
  int	x, y;		/* moved == TRUE 時の、マウスポインタ位置	*/

  int	button[3];	/* ボタン押下状況 [0]～[2] = 左・中・右ボタン	*/

} mouse_stat;


/************************************************************************/
/* マウススキャン関数	1/60sec毎に呼ばれる		OS 依存		*/
/*									*/
/*	mouse_mode == 0		何もしない				*/
/*	  sound_reg[ E ] = 0xff;					*/
/*	  sound_reg[ F ] = 0xff;					*/
/*	mouse_mode == 1		マウスのボタンを検知する		*/
/*	  sound_reg[ F ] = ボタン押下情報				*/
/*	  mouse_dx       = マウス x移動量				*/
/*	  mouse_dy       = マウス y移動量				*/
/*	mouse_mode == 2		マウスのボタンと、移動方向を検知する	*/
/*	  sound_reg[ F ] = ボタン押下情報				*/
/*	  sound_reg[ E ] = マウスの移動方向 上下左右			*/
/*	mouse_mode == 3		ジョイスティック入力を検知する		*/
/*	  sound_reg[ F ] = ABボタン押下情報				*/
/*	  sound_reg[ E ] = 上下左右ボタン押下情報			*/
/*	mouse_mode == 4		ジョイスティック入力をキーに反映する	*/
/*	  sound_reg[ E ] = 0xff;					*/
/*	  sound_reg[ F ] = 0xff;					*/
/*	 (key_scan[xx] = xx … セット済み)				*/
/************************************************************************/

void	scan_mouse( void )
{
}




/************************************************************************/
/* マウス座標計算関数					OS 非依存	*/
/*	JOP1 の ストローブが最初に立ち上がった時に呼ばれる関数		*/
/*									*/
/*	mouse_x, mouse_y, mouse_image_x, mouse_image_y をもとに、	*/
/*	マウスの移動量 mouse_dx, mouse_dy を計算する			*/
/************************************************************************/

void	check_mouse( void )
{
}




/************************************************************************/
/* フォーカスチェック関数				OS依存		*/
/*	1/60sec毎にチェックする						*/
/*	フォーカスを得るとオートリピートOFF、失うとオートリピートON	*/
/*	need_focus が真の場合は、さらに PAUSE の移行をチェック		*/
/*									*/
/*	-focus オプションをサポートしない場合はダミー関数でも構わない。	*/
/************************************************************************/

int	need_focus = FALSE;			/* フォーカスアウト停止あり */

void	scan_focus( void )
{
}



/************************************************************************/
/* 画面露出チェックチェック関数				OS依存		*/
/*	1/60sec毎にチェックする						*/
/*	が、チェックするだけで、ここで描画するわけではない		*/
/*									*/
/*	例えば -frameskip 600 とか指定すると 10秒毎にしか表示を行わない	*/
/*	ので、この間に他のウインドウに隠されて再度露出した場合など、	*/
/*	ウインドウが真っ黒になってしまう。これを避けるために画面露出	*/
/*	した時は強制的に再描画するように指示する。			*/
/*									*/
/*	もっとも、これは X上で必要なだけで、他のシステムを使う場合は	*/
/*	なにもしないダミー関数としても構わないかもれない。		*/
/*	( 例えば、画面露出時はシステムが勝手に再描画する場合など )	*/
/************************************************************************/
void	scan_expose( void )
{
}



/****************************************************************/	
/* サスペンド／レジューム					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_keyboard_work[] =
{
  { TYPE_INT,	&tenkey_emu,	 },
  { TYPE_INT,	&cursor_emu,	 },
  { TYPE_INT,	&numlock_emu,	 },
  { TYPE_INT,	&mouse_mode,	 },
  { TYPE_INT,	&function_f6,	 },
  { TYPE_INT,	&function_f7,	 },
  { TYPE_INT,	&function_f8,	 },
  { TYPE_INT,	&function_f9,	 },
  { TYPE_INT,	&function_f10,	 },
};

int	suspend_keyboard( FILE *fp, long offset )
{
  return TRUE;
}


int	resume_keyboard(  FILE *fp, long offset )
{
  return TRUE;
}



void	keyboard_init_at_resume( void )
{
  keyboard_init();		/* キー設定を初期化 */
}
