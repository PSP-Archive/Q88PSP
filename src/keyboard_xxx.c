/************************************************************************/
/*									*/
/* �L�[���́A�}�E�X���́A���̑��̓��� (OS�ˑ�)				*/
/*									*/
/* �y�֐��z								*/
/*									*/
/* void	scan_keyboard( int key_check_only )				*/
/*	�L�[���͂�ǂ݂Ƃ�Akey_scan[] ���[�N��ݒ肷��B		*/
/*	���� key_scan[] ���[�N�́A���̂܂� PC8801�� I/O �|�[�g 00H�`0FH	*/
/*	�ɑΉ�����B							*/
/*									*/
/* void scan_mouse( void )						*/
/*	�}�E�X���͂�ǂ݂Ƃ�A���W�� mouse_x�Amouse_y �ɁA�{�^���̉���	*/
/*	�󋵂� sound_reg[ 0EH ]�`[ 0FH ] �ɐݒ肷��B			*/
/*	�W���C�X�e�B�b�N���͂� sound_reg[ 0EH ]�`[ 0FH ] �ɐݒ肷��B	*/
/*									*/
/* void	check_mouse( void )						*/
/*	�����OS��ˑ��B�}�E�X�̈ړ��ʂ� sound_reg[ 0FH ] �ɐݒ肷��B	*/
/*									*/
/* void check_focus( void )						*/
/*	���̑��̓��͂�(�K�v�ɉ�����)�`�F�b�N����B			*/
/*		X �� �t�H�[�J�X���`�F�b�N���A�t�H�[�J�X���O�ꂽ��A	*/
/*		�ꎞ��~��������A�E�C���h�E�}�l�[�W���[�ɂ����	*/
/*		�E�C���h�E�����ꂽ��A�����I�������肵�Ă���B	*/
/*									*/
/* void check_expose( void )						*/
/*	��ʘI�o�`�F�b�N						*/
/*									*/
/* void	set_tenkey_emu_effective( void );				*/
/* void	set_tenkey_emu_invalid( void );					*/
/* void	set_cursor_emu_effective( void );				*/
/* void	set_cursor_emu_invalid( void );					*/
/* void	set_numlock_emu_effective( void );				*/
/* void	set_numlock_emu_invalid( void );				*/
/*	�L�[�̊��蓖�Ă�ύX����֐��Ɩ߂��֐�				*/
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

byte	key_scan[0x10];			/* IN[00-0F] �L�[�X�L����	*/

int	tenkey_emu      = FALSE;	/* 10 KEY �̓��͂𐶐�	*/
int	cursor_emu      = FALSE;	/* Cursor KEY -> 10 KEY (by funa) */
int	numlock_emu     = FALSE;	/* software NUM lock	*/

int	mouse_mode	= 0;		/* �}�E�X 0:No 1:Yes 2:Joy  */

int	cursor_assign	= FALSE;	/* Cursor Key -> �C�ӂ̃L�[ */

typedef	struct
{
  byte	key_code;		/* �u��������L�[�R�[�h */
  int	shift_key_on;		/* Shift���K�v��? */
}CURSOR_ASSIGN_TABLE;
CURSOR_ASSIGN_TABLE	cursor_assign_table[CURSOR_ASSIGN_End] =
{
  { 0x00, FALSE, },		/* Cursor Up */
  { 0x00, FALSE, },		/* Cursor Left */
  { 0x00, FALSE, },		/* Cursor Right */
  { 0x00, FALSE, },		/* Cursor Down */
};

int	function_f6  = FN_FUNC;		/* F6-F10 KEY �̋@�\	*/
int	function_f7  = FN_FUNC;
int	function_f8  = FN_FUNC;
int	function_f9  = FN_FUNC;
int	function_f10 = FN_FUNC;

int	romaji_type = 0;		/* ���[�}���ϊ��̃^�C�v	*/

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

char	*file_rec	= NULL;		/* �L�[���͋L�^�̃t�@�C���� */
char	*file_pb	= NULL;		/* �L�[���͍Đ��̃t�@�C���� */

static	FILE *fp_rec;
static	FILE *fp_pb;

static struct {				/* �L�[���͋L�^�\����		*/
  Uchar	key[16];			/*	I/O 00H�`0FH 		*/
   char	mouse_xh;			/*	�}�E�X x ���		*/
  Uchar	mouse_xl;			/*	�}�E�X x ����		*/
   char	mouse_yh;			/*	�}�E�X y ���		*/
  Uchar	mouse_yl;			/*	�}�E�X y ����		*/
  Uchar	joy[2];				/*	SOUND SSG 0EH�`0FH	*/
   char	image[2];			/*	�C���[�WNo -1��,0��,1�`	*/
} key_record;

/*----------------------------------------------------------------------*/
/* �L�[�R�[�h�ϊ��\							*/
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


/* miscellany_keys[n][2] �� n�̒l */

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

/* ���E�V�t�g�A���E���^�[�� �͌��ݖ��Ή�	*/
/* �Ƃ��ɁA�V�t�g�A���^�[�� �Ɋ���U����	*/




/* 0xff?? �n�� (����L�[) for SDL */

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
/*20                     ����     �ϊ�                                �J�i  */
     { 0,  0},{ 0,  0},{Pd,Bt1},{Pd,Bt0},{ 0,  0},{ 0,  0},{ 0,  0},{P8,Bt5},
/*28                     �S�p                                               */
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

/* 0x00?? �n��(��ʃL�[) */

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

/* 0x04?? �n�� (�J�i�L�[) */

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
/*A0             �B       �u       �v       �A       �E       ��       �@   */
     { 0,  0},{P7,Bt5},{P5,Bt3},{P5,Bt5},{P7,Bt4},{P7,Bt6},{P6,Bt0},{P3,Bt3},
/*A8    �B       �D       �F       �H       ��       ��       ��       �b   */
     {P2,Bt5},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P6,Bt7},{P7,Bt0},{P7,Bt1},{P5,Bt2},
/*B0    �[       �A       �C       �E       �G       �I       �J       �L   */
     {P5,Bt4},{P6,Bt3},{P2,Bt5},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P4,Bt4},{P2,Bt7},
/*B8    �N       �P       �R       �T       �V       �X       �Z       �\   */
     {P3,Bt0},{P7,Bt2},{P2,Bt2},{P5,Bt0},{P2,Bt4},{P4,Bt2},{P4,Bt0},{P2,Bt3},
/*C0    �^       �`       �c       �e       �g       �i       �j       �k   */
     {P4,Bt1},{P2,Bt1},{P5,Bt2},{P4,Bt7},{P4,Bt3},{P4,Bt5},{P3,Bt1},{P6,Bt1},
/*C8    �l       �m       �n       �q       �t       �w       �z       �}   */
     {P7,Bt4},{P3,Bt3},{P2,Bt6},{P4,Bt6},{P6,Bt2},{P5,Bt6},{P5,Bt7},{P3,Bt2},
/*D0    �~       ��       ��       ��       ��       ��       ��       ��   */
     {P3,Bt6},{P5,Bt5},{P7,Bt6},{P3,Bt5},{P6,Bt7},{P7,Bt0},{P7,Bt1},{P3,Bt7},
/*D8    ��       ��       ��       ��       ��       ��       �J       �K   */
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
/* �J�[�\���L�[�� �e���L�[[2][4][6][8] �Ɋ��蓖�Ă�֐� �� ��������֐�	*/
/*----------------------------------------------------------------------*/
/*
   �ȉ��̃R�[�h�́A�J�[�\���L�[���e���L�[�Ɍ����Ă郂�[�h
     (-cursor �I�v�V���������ċN��������) �̏����ŁA
     �M�� �[���� [funa@am.ics.keio.ac.jp] �ɂ����̂ł��B
  	DynaBook SS3010�̗l�ȃm�[�g�̂��߂�,
	[�E Shift] -> [9]
	[\]        -> [7] �Ɋ��蓖�ĂĂ��܂�.
	�m�[�g�ł̎g�p���l���Ă���̂�,�K���ȃL�[�̏ꏊ������
	[1],[3] �̃L�[�ւ̊��蓖�Ă͍l���Ă��܂���.
	"\" �L�[�ւ̊��蓖�Ă� latin_keys[0][0]�ɑ΂���
	�s���Ă���̂�,���̃L�[���e�����󂯂Ă��܂���������܂���.

   �����́Avoid	keyboard_init( void ) �֐��̒��ɑg�ݍ��܂�Ă��܂������A
   ���j���[���[�h�⃂�j�^�[���[�h�ȂǂŕύX�o����悤�ɁA
   �����̊��蓖�Ċ֐��A�����֐��Ƃ����`�ɓƗ������܂����B
   ( ver 0.2 �ȍ~��� )
*/

void	set_cursor_emu_effective( void )
{
}

void	set_cursor_emu_invalid( void )
{
}


/*----------------------------------------------------------------------*/
/* �t���L�[��[0]�`[9] �������ꂽ�ꍇ�ɁA�e���L�[�ɑΉ�������^�߂��֐�	*/
/*----------------------------------------------------------------------*/
void	set_tenkey_emu_effective( void )
{
}

void	set_tenkey_emu_invalid( void )
{

}



/*----------------------------------------------------------------------*/
/* �\�t�g�E�F�A NUL Lock ��L���ɂ���֐��^�����ɂ���֐�		*/
/*----------------------------------------------------------------------*/
void	set_numlock_emu_effective( void )
{
}

void	set_numlock_emu_invalid( void )
{
}


/*----------------------------------------------------------------------*/
/* �J�[�\���L�[��C�ӂ̃L�[�ɒu��������֐� / ���ɖ߂��֐�		*/
/*			�����R�[�h�́Afloi����ɂ��񋟂���܂����B	*/
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
/* ���[�}������								*/
/*----------------------------------------------------------------------*/
#include "romaji.c"

/*----------------------------------------------------------------------*/
/* �t�@���N�V�����L�[ f6�`f10 �������ꂽ���ɁA����ȓ��������@�\�֐�	*/
/*----------------------------------------------------------------------*/
static	int	function_key( int fkey, int on )
{
  return 0;
}



/************************************************************************/
/* �L�[�X�L�����֐�	(1/60sec���ɃX�L��������)	OS�ˑ�		*/
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
		key_scan[KeyInput[SoftKBSet].Port] |=  KeyInput[SoftKBSet].Bit;//�L�[�I������O��OFF�ɂ��Ă���
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

		KeyConfigKBDraw(1);//SoftKB�`��
		SHIFT = 0;
		shift_off();//�Ƃ肠����SHIFT OFF
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
		shift_off();//�Ƃ肠����SHIFT OFF
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
/* �}�E�X�X�L�����֐�							*/
/************************************************************************/

int	enable_b2menu = FALSE;		/* ���N���b�N�Ń��j���[���[�h��	*/

	int	mouse_x = 0;		/* ���ۂ̃}�E�X���W x	     */
	int	mouse_y = 0;		/*         �V       y	     */
static	int	mouse_image_x = 0;	/* ���z�I�ȃ}�E�X���W x	     */
static	int	mouse_image_y = 0;	/*          �V        y	     */
	int	mouse_dx = 0;		/* �}�E�X���W����(0�`�}127)x */
	int	mouse_dy = 0;		/*          �V             y */


#define	MOUSE88_KEEP	0
#define	MOUSE88_PRESS	1
#define	MOUSE88_RELEASE	2

static struct{

  int	moved;		/* TRUE �ړ����� FALSE �ړ��Ȃ�			*/
  int	x, y;		/* moved == TRUE ���́A�}�E�X�|�C���^�ʒu	*/

  int	button[3];	/* �{�^�������� [0]�`[2] = ���E���E�E�{�^��	*/

} mouse_stat;


/************************************************************************/
/* �}�E�X�X�L�����֐�	1/60sec���ɌĂ΂��		OS �ˑ�		*/
/*									*/
/*	mouse_mode == 0		�������Ȃ�				*/
/*	  sound_reg[ E ] = 0xff;					*/
/*	  sound_reg[ F ] = 0xff;					*/
/*	mouse_mode == 1		�}�E�X�̃{�^�������m����		*/
/*	  sound_reg[ F ] = �{�^���������				*/
/*	  mouse_dx       = �}�E�X x�ړ���				*/
/*	  mouse_dy       = �}�E�X y�ړ���				*/
/*	mouse_mode == 2		�}�E�X�̃{�^���ƁA�ړ����������m����	*/
/*	  sound_reg[ F ] = �{�^���������				*/
/*	  sound_reg[ E ] = �}�E�X�̈ړ����� �㉺���E			*/
/*	mouse_mode == 3		�W���C�X�e�B�b�N���͂����m����		*/
/*	  sound_reg[ F ] = AB�{�^���������				*/
/*	  sound_reg[ E ] = �㉺���E�{�^���������			*/
/*	mouse_mode == 4		�W���C�X�e�B�b�N���͂��L�[�ɔ��f����	*/
/*	  sound_reg[ E ] = 0xff;					*/
/*	  sound_reg[ F ] = 0xff;					*/
/*	 (key_scan[xx] = xx �c �Z�b�g�ς�)				*/
/************************************************************************/

void	scan_mouse( void )
{
}




/************************************************************************/
/* �}�E�X���W�v�Z�֐�					OS ��ˑ�	*/
/*	JOP1 �� �X�g���[�u���ŏ��ɗ����オ�������ɌĂ΂��֐�		*/
/*									*/
/*	mouse_x, mouse_y, mouse_image_x, mouse_image_y �����ƂɁA	*/
/*	�}�E�X�̈ړ��� mouse_dx, mouse_dy ���v�Z����			*/
/************************************************************************/

void	check_mouse( void )
{
}




/************************************************************************/
/* �t�H�[�J�X�`�F�b�N�֐�				OS�ˑ�		*/
/*	1/60sec���Ƀ`�F�b�N����						*/
/*	�t�H�[�J�X�𓾂�ƃI�[�g���s�[�gOFF�A�����ƃI�[�g���s�[�gON	*/
/*	need_focus ���^�̏ꍇ�́A����� PAUSE �̈ڍs���`�F�b�N		*/
/*									*/
/*	-focus �I�v�V�������T�|�[�g���Ȃ��ꍇ�̓_�~�[�֐��ł��\��Ȃ��B	*/
/************************************************************************/

int	need_focus = FALSE;			/* �t�H�[�J�X�A�E�g��~���� */

void	scan_focus( void )
{
}



/************************************************************************/
/* ��ʘI�o�`�F�b�N�`�F�b�N�֐�				OS�ˑ�		*/
/*	1/60sec���Ƀ`�F�b�N����						*/
/*	���A�`�F�b�N���邾���ŁA�����ŕ`�悷��킯�ł͂Ȃ�		*/
/*									*/
/*	�Ⴆ�� -frameskip 600 �Ƃ��w�肷��� 10�b���ɂ����\�����s��Ȃ�	*/
/*	�̂ŁA���̊Ԃɑ��̃E�C���h�E�ɉB����čēx�I�o�����ꍇ�ȂǁA	*/
/*	�E�C���h�E���^�����ɂȂ��Ă��܂��B���������邽�߂ɉ�ʘI�o	*/
/*	�������͋����I�ɍĕ`�悷��悤�Ɏw������B			*/
/*									*/
/*	�����Ƃ��A����� X��ŕK�v�Ȃ����ŁA���̃V�X�e�����g���ꍇ��	*/
/*	�Ȃɂ����Ȃ��_�~�[�֐��Ƃ��Ă��\��Ȃ�������Ȃ��B		*/
/*	( �Ⴆ�΁A��ʘI�o���̓V�X�e��������ɍĕ`�悷��ꍇ�Ȃ� )	*/
/************************************************************************/
void	scan_expose( void )
{
}



/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
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
  keyboard_init();		/* �L�[�ݒ�������� */
}
