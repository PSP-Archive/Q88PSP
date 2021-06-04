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

#include <SDL.h>

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

#include "exmem.h"

byte	key_scan[0x10];			/* IN[00-0F] �L�[�X�L����	*/

int	tenkey_emu      = FALSE;	/* 10 KEY �̓��͂𐶐�	*/
int	cursor_emu      = FALSE;	/* Cursor KEY -> 10 KEY (by funa) */
int	numlock_emu     = FALSE;	/* software NUM lock	*/

int	mouse_mode	= 0;		/* �}�E�X 0:No 1:Yes 2:Joy  */

int	cursor_assign	= FALSE;	/* Cursor Key -> �C�ӂ̃L�[ */

int meta_flag = 0;

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
#if 0
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
#endif

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
  cursor_emu = TRUE;

  miscellany_keys[SDLK_LEFT][0] = P0;	/* LEFT -> 4 */
  miscellany_keys[SDLK_LEFT][1] = Bt4;

  miscellany_keys[SDLK_UP][0] = P1;	/* UP -> 8 */
  miscellany_keys[SDLK_UP][1] = Bt0;

  miscellany_keys[SDLK_RIGHT][0] = P0;	/* RIGHT -> 6 */
  miscellany_keys[SDLK_RIGHT][1] = Bt6;

  miscellany_keys[SDLK_DOWN][0] = P0;	/* DOWN -> 2 */
  miscellany_keys[SDLK_DOWN][1] = Bt2;

  miscellany_keys[SDLK_RSHIFT][0] = P1;	/* R-Shift -> 9 */
  miscellany_keys[SDLK_RSHIFT][1] = Bt1;

  latin1_keys[SDLK_BACKSLASH][0] = P0;		/* \ -> 7 */
  latin1_keys[SDLK_BACKSLASH][1] = Bt7;
}

void	set_cursor_emu_invalid( void )
{
  cursor_emu = FALSE;

  if( cursor_assign ) {
    set_cursor_assign_effective();
  } else {

  miscellany_keys[SDLK_LEFT][0] = Pa;	/* LEFT -> 4 */
  miscellany_keys[SDLK_LEFT][1] = Bt2;

  miscellany_keys[SDLK_UP][0] = P8;	/* UP -> 8 */
  miscellany_keys[SDLK_UP][1] = Bt1;

  miscellany_keys[SDLK_RIGHT][0] = P8;	/* RIGHT -> 6 */
  miscellany_keys[SDLK_RIGHT][1] = Bt2;

  miscellany_keys[SDLK_DOWN][0] = Pa;	/* DOWN -> 2 */
  miscellany_keys[SDLK_DOWN][1] = Bt1;

  miscellany_keys[SDLK_RSHIFT][0] = P8;	/* R-Shift -> 9 */
  miscellany_keys[SDLK_RSHIFT][1] = Bt6;

  latin1_keys[SDLK_BACKSLASH][0] = 0;		/* \ -> 7 */
  latin1_keys[SDLK_BACKSLASH][1] = 0;

  }
}


/*----------------------------------------------------------------------*/
/* �t���L�[��[0]�`[9] �������ꂽ�ꍇ�ɁA�e���L�[�ɑΉ�������^�߂��֐�	*/
/*----------------------------------------------------------------------*/
void	set_tenkey_emu_effective( void )
{
  tenkey_emu = TRUE;

  latin1_keys[0x30][0] = P0;
  latin1_keys[0x30][1] = Bt0;

  latin1_keys[0x31][0] = P0;
  latin1_keys[0x31][1] = Bt1;

  latin1_keys[0x32][0] = P0;
  latin1_keys[0x32][1] = Bt2;

  latin1_keys[0x33][0] = P0;
  latin1_keys[0x33][1] = Bt3;

  latin1_keys[0x34][0] = P0;
  latin1_keys[0x34][1] = Bt4;

  latin1_keys[0x35][0] = P0;
  latin1_keys[0x35][1] = Bt5;

  latin1_keys[0x36][0] = P0;
  latin1_keys[0x36][1] = Bt6;

  latin1_keys[0x37][0] = P0;
  latin1_keys[0x37][1] = Bt7;

  latin1_keys[0x38][0] = P1;
  latin1_keys[0x38][1] = Bt0;

  latin1_keys[0x39][0] = P1;
  latin1_keys[0x39][1] = Bt1;
}

void	set_tenkey_emu_invalid( void )
{
  tenkey_emu = FALSE;

  latin1_keys[0x30][0] = P6;
  latin1_keys[0x30][1] = Bt0;

  latin1_keys[0x31][0] = P6;
  latin1_keys[0x31][1] = Bt1;

  latin1_keys[0x32][0] = P6;
  latin1_keys[0x32][1] = Bt2;

  latin1_keys[0x33][0] = P6;
  latin1_keys[0x33][1] = Bt3;

  latin1_keys[0x34][0] = P6;
  latin1_keys[0x34][1] = Bt4;

  latin1_keys[0x35][0] = P6;
  latin1_keys[0x35][1] = Bt5;

  latin1_keys[0x36][0] = P6;
  latin1_keys[0x36][1] = Bt6;

  latin1_keys[0x37][0] = P6;
  latin1_keys[0x37][1] = Bt7;

  latin1_keys[0x38][0] = P7;
  latin1_keys[0x38][1] = Bt0;

  latin1_keys[0x39][0] = P7;
  latin1_keys[0x39][1] = Bt1;
}



/*----------------------------------------------------------------------*/
/* �\�t�g�E�F�A NUL Lock ��L���ɂ���֐��^�����ɂ���֐�		*/
/*----------------------------------------------------------------------*/
void	set_numlock_emu_effective( void )
{
  numlock_emu = TRUE;

  latin1_keys[0x6d][0] = P0;		/* m -> 0[10] */
  latin1_keys[0x6d][1] = Bt0;
  latin1_keys[0x4d][0] = P0;		/* M -> 0[10] */
  latin1_keys[0x4d][1] = Bt0;

  latin1_keys[0x2c][0] = P1;		/* , -> ,[10] */
  latin1_keys[0x2c][1] = Bt5;
  latin1_keys[0x3c][0] = P1;		/* < -> ,[10] */
  latin1_keys[0x3c][1] = Bt5;

  latin1_keys[0x2e][0] = P1;		/* . -> .[10] */
  latin1_keys[0x2e][1] = Bt6;
  latin1_keys[0x3e][0] = P1;		/* > -> .[10] */
  latin1_keys[0x3e][1] = Bt6;

  latin1_keys[0x2f][0] = P1;		/* / -> ret */
  latin1_keys[0x2f][1] = Bt7;
  latin1_keys[0x3f][0] = P1;		/* ? -> ret */
  latin1_keys[0x3f][1] = Bt7;


  latin1_keys[0x6a][0] = P0;		/* j -> 1[10] */
  latin1_keys[0x6a][1] = Bt1;
  latin1_keys[0x4a][0] = P0;		/* J -> 1[10] */
  latin1_keys[0x4a][1] = Bt1;

  latin1_keys[0x6b][0] = P0;		/* k -> 2[10] */
  latin1_keys[0x6b][1] = Bt2;
  latin1_keys[0x4b][0] = P0;		/* K -> 2[10] */
  latin1_keys[0x4b][1] = Bt2;

  latin1_keys[0x6c][0] = P0;		/* l -> 3[10] */
  latin1_keys[0x6c][1] = Bt3;
  latin1_keys[0x4c][0] = P0;		/* L -> 3[10] */
  latin1_keys[0x4c][1] = Bt3;

  latin1_keys[0x3b][0] = P1;		/* ; -> =[10] */
  latin1_keys[0x3b][1] = Bt4;
  latin1_keys[0x2b][0] = P1;		/* + -> =[10] */
  latin1_keys[0x2b][1] = Bt4;


  latin1_keys[0x75][0] = P0;		/* u -> 4[10] */
  latin1_keys[0x75][1] = Bt4;
  latin1_keys[0x55][0] = P0;		/* U -> 4[10] */
  latin1_keys[0x55][1] = Bt4;

  latin1_keys[0x69][0] = P0;		/* i -> 5[10] */
  latin1_keys[0x69][1] = Bt5;
  latin1_keys[0x49][0] = P0;		/* I -> 5[10] */
  latin1_keys[0x49][1] = Bt5;

  latin1_keys[0x6f][0] = P0;		/* o -> 6[10] */
  latin1_keys[0x6f][1] = Bt6;
  latin1_keys[0x4f][0] = P0;		/* O -> 6[10] */
  latin1_keys[0x4f][1] = Bt6;

  latin1_keys[0x70][0] = P1;		/* p -> +[10] */
  latin1_keys[0x70][1] = Bt3;
  latin1_keys[0x50][0] = P1;		/* P -> +[10] */
  latin1_keys[0x50][1] = Bt3;


  latin1_keys[0x37][0] = P0;		/* 7 -> 7[10] */
  latin1_keys[0x37][1] = Bt7;
  latin1_keys[0x27][0] = P0;		/* ' -> 7[10] */
  latin1_keys[0x27][1] = Bt7;

  latin1_keys[0x38][0] = P1;		/* 8 -> 8[10] */
  latin1_keys[0x38][1] = Bt0;
  latin1_keys[0x28][0] = P1;		/* ( -> 8[10] */
  latin1_keys[0x28][1] = Bt0;

  latin1_keys[0x39][0] = P1;		/* 9 -> 9[10] */
  latin1_keys[0x39][1] = Bt1;
  latin1_keys[0x29][0] = P1;		/* ) -> 9[10] */
  latin1_keys[0x29][1] = Bt1;

  latin1_keys[0x30][0] = P1;		/* 0 -> *[10] */
  latin1_keys[0x30][1] = Bt2;
  latin1_keys[0x7e][0] = P1;		/* ~ -> *[10] */
  latin1_keys[0x7e][1] = Bt2;
}

void	set_numlock_emu_invalid( void )
{
  numlock_emu = FALSE;

  latin1_keys[0x6d][0] = P3;		/* m */
  latin1_keys[0x6d][1] = Bt5;
  latin1_keys[0x4d][0] = P3;		/* M */
  latin1_keys[0x4d][1] = Bt5;

  latin1_keys[0x2c][0] = P7;		/* , */
  latin1_keys[0x2c][1] = Bt4;
  latin1_keys[0x3c][0] = P7;		/* < */
  latin1_keys[0x3c][1] = Bt4;

  latin1_keys[0x2e][0] = P7;		/* . */
  latin1_keys[0x2e][1] = Bt5;
  latin1_keys[0x3e][0] = P7;		/* > */
  latin1_keys[0x3e][1] = Bt5;

  latin1_keys[0x2f][0] = P7;		/* / */
  latin1_keys[0x2f][1] = Bt6;
  latin1_keys[0x3f][0] = P7;		/* ? */
  latin1_keys[0x3f][1] = Bt6;


  latin1_keys[0x6a][0] = P3;		/* j */
  latin1_keys[0x6a][1] = Bt2;
  latin1_keys[0x4a][0] = P3;		/* J */
  latin1_keys[0x4a][1] = Bt2;

  latin1_keys[0x6b][0] = P3;		/* k */
  latin1_keys[0x6b][1] = Bt3;
  latin1_keys[0x4b][0] = P3;		/* K */
  latin1_keys[0x4b][1] = Bt3;

  latin1_keys[0x6c][0] = P3;		/* l */
  latin1_keys[0x6c][1] = Bt4;
  latin1_keys[0x4c][0] = P3;		/* L */
  latin1_keys[0x4c][1] = Bt4;

  latin1_keys[0x3b][0] = P7;		/* ; */
  latin1_keys[0x3b][1] = Bt3;
  latin1_keys[0x2b][0] = P7;		/* + */
  latin1_keys[0x2b][1] = Bt3;


  latin1_keys[0x75][0] = P4;		/* u */
  latin1_keys[0x75][1] = Bt5;
  latin1_keys[0x55][0] = P4;		/* U */
  latin1_keys[0x55][1] = Bt5;

  latin1_keys[0x69][0] = P3;		/* i */
  latin1_keys[0x69][1] = Bt1;
  latin1_keys[0x49][0] = P3;		/* I */
  latin1_keys[0x49][1] = Bt1;

  latin1_keys[0x6f][0] = P3;		/* o */
  latin1_keys[0x6f][1] = Bt7;
  latin1_keys[0x4f][0] = P3;		/* O */
  latin1_keys[0x4f][1] = Bt7;

  latin1_keys[0x70][0] = P4;		/* p */
  latin1_keys[0x70][1] = Bt0;
  latin1_keys[0x50][0] = P4;		/* P */
  latin1_keys[0x50][1] = Bt0;


  latin1_keys[0x37][0] = P6;		/* 7 */
  latin1_keys[0x37][1] = Bt7;
  latin1_keys[0x27][0] = P6;		/* ' */
  latin1_keys[0x27][1] = Bt7;

  latin1_keys[0x38][0] = P7;		/* 8 */
  latin1_keys[0x38][1] = Bt0;
  latin1_keys[0x28][0] = P7;		/* ( */
  latin1_keys[0x28][1] = Bt0;

  latin1_keys[0x39][0] = P7;		/* 9 */
  latin1_keys[0x39][1] = Bt1;
  latin1_keys[0x29][0] = P7;		/* ) */
  latin1_keys[0x29][1] = Bt1;

  latin1_keys[0x30][0] = P6;		/* 0 */
  latin1_keys[0x30][1] = Bt0;
  latin1_keys[0x7e][0] = P5;		/* ~ */
  latin1_keys[0x7e][1] = Bt6;
}


/*----------------------------------------------------------------------*/
/* �J�[�\���L�[��C�ӂ̃L�[�ɒu��������֐� / ���ɖ߂��֐�		*/
/*			�����R�[�h�́Afloi����ɂ��񋟂���܂����B	*/
/*----------------------------------------------------------------------*/
void	set_cursor_assign_effective( void )
{
  byte	code;

  cursor_assign = TRUE;

  code = cursor_assign_table[CURSOR_ASSIGN_LEFT].key_code;	/* Left */
  if( code != 0x00 ) {
    miscellany_keys[SDLK_LEFT][0] = latin1_keys[code][0];
    miscellany_keys[SDLK_LEFT][1] = latin1_keys[code][1];
  } else {
    miscellany_keys[SDLK_LEFT][0] = Pa;
    miscellany_keys[SDLK_LEFT][1] = Bt2;
  }

  code = cursor_assign_table[CURSOR_ASSIGN_UP].key_code;	/* Up */
  if( code != 0x00 ) {
    miscellany_keys[SDLK_UP][0] = latin1_keys[code][0];
    miscellany_keys[SDLK_UP][1] = latin1_keys[code][1];
  } else {
    miscellany_keys[SDLK_UP][0] = P8;
    miscellany_keys[SDLK_UP][1] = Bt1;
  }

  code = cursor_assign_table[CURSOR_ASSIGN_RIGHT].key_code;	/* Right */
  if( code != 0x00 ) {
    miscellany_keys[SDLK_RIGHT][0] = latin1_keys[code][0];
    miscellany_keys[SDLK_RIGHT][1] = latin1_keys[code][1];
  } else {
    miscellany_keys[SDLK_RIGHT][0] = P8;
    miscellany_keys[SDLK_RIGHT][1] = Bt2;
  }

  code = cursor_assign_table[CURSOR_ASSIGN_DOWN].key_code;	/* Down */
  if( code != 0x00 ) {
    miscellany_keys[SDLK_DOWN][0] = latin1_keys[code][0];
    miscellany_keys[SDLK_DOWN][1] = latin1_keys[code][1];
  } else {
    miscellany_keys[SDLK_DOWN][0] = Pa;
    miscellany_keys[SDLK_DOWN][1] = Bt1;
  }

}

void	set_cursor_assign_invalid( void )
{
  cursor_assign = FALSE;

  if( cursor_emu ) {
    set_cursor_emu_effective();
  } else {
    miscellany_keys[SDLK_LEFT][0] = Pa;	/* Left */
    miscellany_keys[SDLK_LEFT][1] = Bt2;

    miscellany_keys[SDLK_UP][0] = P8;	/* Up */
    miscellany_keys[SDLK_UP][1] = Bt1;

    miscellany_keys[SDLK_RIGHT][0] = P8;	/* Right */
    miscellany_keys[SDLK_RIGHT][1] = Bt2;

    miscellany_keys[SDLK_DOWN][0] = Pa;	/* Down */
    miscellany_keys[SDLK_DOWN][1] = Bt1;
  }
}

void	set_cursor_assign_key( int index, byte new_code )
{
  cursor_assign_table[index].key_code = new_code;

	/* Shift�L�[���K�v���`�F�b�N */
  if( 0x21 <= new_code && new_code <= 0x2b ) {		/* ! <---> + */
    cursor_assign_table[index].shift_key_on = TRUE;
  } else if( 0x3c <= new_code && new_code <= 0x3f ) {	/* < <---> ? */
    cursor_assign_table[index].shift_key_on = TRUE;
  } else if( 0x41 <= new_code && new_code <= 0x5a ) {	/* A <---> Z */
    cursor_assign_table[index].shift_key_on = TRUE;
  } else if( 0x5f <= new_code && new_code <= 0x60 ) {	/* _ <---> ` */
    cursor_assign_table[index].shift_key_on = TRUE;
  } else if( 0x7b <= new_code && new_code <= 0x7e ) {	/* { <---> ~ */
    cursor_assign_table[index].shift_key_on = TRUE;
  } else {
    cursor_assign_table[index].shift_key_on = FALSE;
  }

  if( cursor_assign )
    set_cursor_assign_effective();
}

byte	get_cursor_assign_key( int index )
{
  return cursor_assign_table[index].key_code;
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
  switch( fkey ){
  case FN_FUNC:					/* �@�\�Ȃ� */
    /* �֐��Ăт����֎~ �I assert(FALSE) */
    return 0;

  case FN_FRATE_UP:				/* �t���[�� */
    if( on ) indicate_change_frame(+1);
    return 0;
  case FN_FRATE_DOWN:				/* �t���[�� */
    if( on ) indicate_change_frame(-1);
    return 0;

  case FN_VOLUME_UP:				/* ���� */
    if( on ) indicate_change_volume(-1);
    return 0;
  case FN_VOLUME_DOWN:				/* ���� */
    if( on ) indicate_change_volume(+1);
    return 0;

  case FN_PAUSE:				/* �ꎞ��~ */
    if( on ) 
      if( emu_mode != PAUSE_MAIN ) emu_mode = PAUSE;  
    return 0;

  case FN_RESIZE:				/* ���T�C�Y */
    if( on ){
      if( ++screen_size > screen_size_max ) screen_size = 0;
      graphic_system_restart(TRUE);
    }
    return 0;

  case FN_NOWAIT:				/* �E�G�C�g */
    if( on ) indicate_change_wait(0);
    return 0;
  case FN_SPEED_UP:
    if( on ) indicate_change_wait(+1);
    return 0;
  case FN_SPEED_DOWN:
    if( on ) indicate_change_wait(-1);
    return 0;

  case FN_MOUSE_HIDE:				/* �}�E�X�\�� */
    if( on ){
      hide_mouse ^= 1;
      if( hide_mouse ) set_mouse_invisible();
      else	       set_mouse_visible();
    }
    return 0;

  case FN_DGA:					/* DGA <--> noDGA �؂�ւ� */
    if( on ){
      if( use_DGA ) use_DGA = FALSE;
      else          use_DGA = TRUE;
      graphic_system_restart(TRUE);
    }
    return 0;

  case FN_SNAPSHOT:				/* �X�N���[���X�i�b�v�V���b�g*/
    if( on ) save_screen_snapshot( );
    return 0;

  case FN_SUSPEND:				/* �T�X�y���h */
    if( on ){
      if( suspend( file_resume ) )
	indicate_change_suspend();
    }
    return 0;

  case FN_IMAGE_NEXT1:				/* DRIVE1: �C���[�W�ύX */
    if( on ) indicate_change_image_empty( 0 );
    else     indicate_change_image_change( 0, +1 );
    return 0;
  case FN_IMAGE_PREV1:
    if( on ) indicate_change_image_empty( 0 );
    else     indicate_change_image_change( 0, -1 );
    return 0;
  case FN_IMAGE_NEXT2:				/* DRIVE2: �C���[�W�ύX */
    if( on ) indicate_change_image_empty( 1 );
    else     indicate_change_image_change( 1, +1 );
    return 0;
  case FN_IMAGE_PREV2:
    if( on ) indicate_change_image_empty( 1 );
    else     indicate_change_image_change( 1, -1 );
    return 0;

  case FN_NUMLOCK:				/* NUM lock */
    if( on ){
      if( numlock_emu==0 ) set_numlock_emu_effective();
      else                 set_numlock_emu_invalid();
    }
    return 0;

  case FN_RESET:				/* ���Z�b�g */
    if( on ){
      main_reset();
    }
    return 0;

  case FN_KANA:					/* �J�i */
    if( on ){
      key_scan[P8] ^= Bt5;   romaji_input_term();
    }
    return 0;
  case FN_ROMAJI:				/* �J�i(���[�}��) */
    if( on ){
      key_scan[P8] ^= Bt5;					
      if( (key_scan[P8]&Bt5)==0 ) romaji_input_init();
      else                        romaji_input_term();
    }
    return 0;

  case FN_CAPS:					/* CAPS */
    if( on ){
      key_scan[Pa] ^= Bt7;
    }
    return 0;

  case FN_KETTEI:				/* ���� */
    if( on ) space_on();
    else     space_off();	return code_kettei;
  case FN_HENKAN:				/* �ϊ� */
    if( on ) space_on();
    else     space_off();	return code_henkan;
  case FN_ZENKAKU:				/* �S�p */
  				return code_zenkaku;
  case FN_PC:					/* �o�b */
				return code_pc;
  }
  return 0;
}



/************************************************************************/
/* �L�[�X�L�����֐�	(1/60sec���ɃX�L��������)	OS�ˑ�		*/
/************************************************************************/

void	keyboard_init( void )
{
  int	i;

  romaji_make_list();

  for( i=0; i<0x10; i++ )  key_scan[i]=0xff;

  if( cursor_emu )  set_cursor_emu_effective();
  if( tenkey_emu )  set_tenkey_emu_effective();
  if( numlock_emu ) set_numlock_emu_effective();



  fp_pb  = NULL;
  fp_rec = NULL;

  if( file_pb ){				/* �Đ��p�t�@�C�����I�[�v�� */
    if( !(fp_pb=ex_fopen( file_pb, "rb")) ){
      fprintf( stderr, "Can't open PlayBack file <%s>\n", file_pb );
      main_exit(1);
    }
  }
  if( file_rec ){				/* �L�^�p�t�@�C�����I�[�v�� */
    if( !(fp_rec=ex_fopen( file_rec, "wb")) ){
      fprintf( stderr, "Can't open Record file <%s>\n", file_rec );
      main_exit(1);
    }
							/* ���[�N�������� */
    for( i=0; i<16; i++ ) key_record.key[i]     = 0xff;
    key_record.mouse_xh = 0;
    key_record.mouse_xl = 0;
    key_record.mouse_yh = 0;
    key_record.mouse_yl = 0;
    for( i=0; i<2; i++ ){
      int img;
      if( disk_image_exist( i ) &&
	  drive_check_empty( i ) == FALSE )
	img = disk_image_now_selected(i) + 1;
      else
	img = -1;
      key_record.image[i] = img;
    }
  }
}

static void scan_key_sub(int type, int key)
{
    int i = key, ii = key;

	if( type==SDL_KEYDOWN ){		/* ======== �L�[ ON ======== */
		if (meta_flag) 
		{
			switch( ii ) {
				case SDLK_l;
				emu_mode = MENU;
				break;
			}
		}
      if (ii < 0x20 || ii > 122) { /* NOT ASCII */
	switch( ii ){
	case SDLK_ESCAPE:
#ifdef	USE_ZAURUS
	  if ((key_scan[P8] & Bt6) == 0)
	    emu_mode = QUIT;
#endif
	  if( romaji_input_mode ) romaji_clear();
				break;
	case SDLK_LMETA:
			meta_flag = 1;
	break;
				
	case SDLK_F11:					/* FDD�X�e�[�^�X�\�� */
	  indicate_change_fdd_stat();
	  break;
	case SDLK_F12:					/* ���j���[���[�h */
	  emu_mode = MENU;
	  break;

	case SDLK_F6:
	  if ( function_f6==FN_FUNC ) shift_on();
	  else                        i = function_key( function_f6, TRUE );
	  break;
	case SDLK_F7:
	  if ( function_f7==FN_FUNC ) shift_on();
	  else                        i = function_key( function_f7, TRUE );
	  break;
	case SDLK_F8:
	  if ( function_f8==FN_FUNC ) shift_on();
	  else                        i = function_key( function_f8, TRUE );
	  break;
	case SDLK_F9:
	  if ( function_f9==FN_FUNC ) shift_on();
	  else                        i = function_key( function_f9, TRUE );
	  break;
	case SDLK_F10:
	  if ( function_f10==FN_FUNC ) shift_on();
	  else                         i = function_key( function_f10, TRUE );
	  break;

	case SDLK_INSERT:
	  shift_on();
	case SDLK_DELETE:
	case SDLK_BACKSPACE:
	  bs_on();
	  break;
	}
	if( cursor_assign ) {	/* �J�[�\���L�[�ɔC�ӂ̃L�[�����蓖�ĂĂ���Ȃ� */
	  int	index = -1;
	  switch( i ) {
	    case SDLK_LEFT:  index = CURSOR_ASSIGN_LEFT;	break;
	    case SDLK_UP:    index = CURSOR_ASSIGN_UP;		break;
	    case SDLK_RIGHT: index = CURSOR_ASSIGN_RIGHT;	break;
	    case SDLK_DOWN:  index = CURSOR_ASSIGN_DOWN;	break;
	    default:	break;
	  }
	  if( index != -1 ){
	    if( cursor_assign_table[index].shift_key_on )
	      shift_on();	/* Shift�L�[��L���ɂ��� */
	  }
	}
	key_scan[miscellany_keys[i][0]] &= ~miscellany_keys[i][1];
      } else {
	if( romaji_input_mode==0 ){
	  key_scan[    latin1_keys[i][0]] &= ~latin1_keys[    i][1];
	}else{
	  if( ii <= 0xff && isgraph(ii) ){
	    romaji_check( ii );
	  }
	}
#if 0
      case 0x0400:				/* ���������L�[ */
	key_scan[  katakana_keys[i][0]] &= ~katakana_keys[  i][1];
	break;
#endif
      }
    }else{				/* ======== �L�[ OFF ======== */
      if (ii < 0x20 || ii > 122) { /* NOT ASCII */
			switch( ii ){
	case SDLK_LMETA:
		meta_flag = 0;
	break;

	case SDLK_F6:
	  if ( function_f6==FN_FUNC ) shift_off();
	  else                        i = function_key( function_f6, FALSE );
	  break;
	case SDLK_F7:
	  if ( function_f7==FN_FUNC ) shift_off();
	  else                        i = function_key( function_f7, FALSE );
	  break;
	case SDLK_F8:
	  if ( function_f8==FN_FUNC ) shift_off();
	  else                        i = function_key( function_f8, FALSE );
	  break;
	case SDLK_F9:
	  if ( function_f9==FN_FUNC ) shift_off();
	  else                        i = function_key( function_f9, FALSE );
	  break;
	case SDLK_F10:
	  if ( function_f10==FN_FUNC ) shift_off();
	  else                        i = function_key( function_f10, FALSE );
	  break;

	case SDLK_INSERT:
	  shift_off();
	case SDLK_DELETE:
	case SDLK_BACKSPACE:
	  bs_off();
	  break;
	}
	if( cursor_assign ) {	/* �J�[�\���L�[�ɔC�ӂ̃L�[�����蓖�ĂĂ���Ȃ� */
	  int	index = -1;
	  switch( i ) {
	    case SDLK_LEFT:  index = CURSOR_ASSIGN_LEFT;	break;
	    case SDLK_UP:    index = CURSOR_ASSIGN_UP;		break;
	    case SDLK_RIGHT: index = CURSOR_ASSIGN_RIGHT;	break;
	    case SDLK_DOWN:  index = CURSOR_ASSIGN_DOWN;	break;
	    default:	break;
	  }
	  if( index != -1 ) {
	    if( cursor_assign_table[index].shift_key_on )
	      shift_off();	/* �V�t�g�L�[�𖳌��ɂ��� */
	  }
	}
	key_scan[miscellany_keys[i][0]] |=  miscellany_keys[i][1];
      } else {
	if( romaji_input_mode==0 ){
	  key_scan[    latin1_keys[i][0]] |=  latin1_keys[    i][1];
	}
#if 0
      case 0x0400:
	key_scan[  katakana_keys[i][0]] |=  katakana_keys[  i][1];
	break;
#endif
      }
    }
}

void	scan_keyboard( int key_check_only )
{
  SDL_Event E;
  SDLKey ii;
  int i;

  SDL_PumpEvents();
  if( SDL_PeepEvents(&E, 1, SDL_GETEVENT,
		     SDL_EVENTMASK(SDL_QUIT)) ){
      if (E.type == SDL_QUIT) {
	  emu_mode = QUIT;
      }
  }
  while
    ( SDL_PeepEvents(&E, 1, SDL_GETEVENT,
		     SDL_EVENTMASK(SDL_KEYDOWN)|SDL_EVENTMASK(SDL_KEYUP)) ){
      ii = E.key.keysym.sym;
      scan_key_sub(E.type, ii);
  }


  /* �������� �֐��̏I���܂ŁA OS��ˑ��̂͂� */

  if( romaji_input_mode ) romaji_out_put();

  if( use_joystick ){
    if( mouse_mode==4 ){
      int state = joystick_get_state();
      int i, on[2], port, mask;

      if( joy_swap_button==FALSE ){
	on[0] = state & JOY88_A;
	on[1] = state & JOY88_B;
      }else{
	on[0] = state & JOY88_B;
	on[1] = state & JOY88_A;
      }

      if( joy_key_assign[0] == joy_key_assign[1] ){
	if( on[0] || on[1] ){
	  on[0] = on[1] = TRUE;
	}else{
	  on[0] = on[1] = FALSE;
	}
      }

      for( i=0; i<2; i++ ){
	switch( joy_key_assign[i] ){
	case JOY88_KEY_X:	port = P5, mask = Bt0;	break;
	case JOY88_KEY_Z:	port = P5, mask = Bt2;	break;
	case JOY88_KEY_SPACE:	port = P9, mask = Bt6;	break;
	case JOY88_KEY_RET:	port = P1, mask = Bt7;	break;
	case JOY88_KEY_SHIFT:	port = P8, mask = Bt6;	break;
	case JOY88_KEY_ESC:	port = P9, mask = Bt7;	break;
	default:		continue;
	}
	if( on[i] ) key_scan[ port ] &= ~mask;
	else        key_scan[ port ] |=  mask;
      }

      if( state & JOY88_UP )	key_scan[ P1 ] &= ~Bt0;		/* 8 */
      else			key_scan[ P1 ] |=  Bt0;

      if( state & JOY88_DOWN )	key_scan[ P0 ] &= ~Bt2;		/* 2 */
      else			key_scan[ P0 ] |=  Bt2;

      if( state & JOY88_RIGHT )	key_scan[ P0 ] &= ~Bt6;		/* 6 */
      else			key_scan[ P0 ] |=  Bt6;

      if( state & JOY88_LEFT )	key_scan[ P0 ] &= ~Bt4;		/* 4 */
      else			key_scan[ P0 ] |=  Bt4;

    }
  }

  if( key_check_only ) return;

  if( fp_rec ){
    for( i=0; i<0x10; i++ )
      key_record.key[i] = key_scan[i];
  }
  if( fp_pb ){
    if(ex_fread( &key_record, sizeof(char), sizeof(key_record), fp_pb )
							== sizeof(key_record)){
      for( i=0; i<0x10; i++ )
	key_scan[i] = key_record.key[i];

    }else{
      if( verbose_proc ) printf( "End of PlayBack file <%s>\n", file_pb );
      ex_fclose( fp_pb );
      fp_pb = NULL;
    }
  }
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
  int	i, x, y;

  {
    SDL_Event E;
    static int fn = -1;
    static int sdl_x = 0, sdl_y = 0;

    mouse_stat.moved = FALSE;

  SDL_PumpEvents();
  while( SDL_PeepEvents(&E, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEMOTION)) ){

#if 0
      sdl_x = E.motion.x;
      sdl_y = E.motion.y;
#else
      mouse_coord_trans(E.motion.x, E.motion.y, &sdl_x, &sdl_y);
#endif

#if 0	/* for QUASI88z */
	if( verbose_proc ) printf("Mouse %d,%d\n", sdl_x, sdl_y);
#endif

      if( mouse_mode == 0 ||		/* �}�E�X���� */
	  mouse_mode == 3 ||		/* �W���C�X�e�B�b�N���� */
	  mouse_mode == 4 ){		/* �W���C�X�e�B�b�N���L�[���[�h */

	mouse_stat.moved = FALSE;	/* ��L�̃��[�h�ł̓}�E�X�ړ��͕s�v */

      }else{				/* �}�E�X���� */
					/* �}�E�X���W���C�X�e�B�b�N���[�h */
	mouse_stat.moved = TRUE;
	mouse_stat.x = sdl_x;
	mouse_stat.y = sdl_y;
      }
    }



    mouse_stat.button[0] = MOUSE88_KEEP;
    mouse_stat.button[1] = MOUSE88_KEEP;
    mouse_stat.button[2] = MOUSE88_KEEP;
  if( SDL_PeepEvents(&E, 1, SDL_GETEVENT,
		     SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)|
		     SDL_EVENTMASK(SDL_MOUSEBUTTONUP)) ){

#if 0	/* for QUASI88z */
      if( verbose_proc ) printf("MouseB %d\n", E.type);
#endif

      if( E.type==SDL_MOUSEBUTTONDOWN ){
	if( E.button.button == SDL_BUTTON_LEFT ) {
	  fn = function_mouse_pos(sdl_x, sdl_y);
	    if (fn >= 0) {
		scan_key_sub(SDL_KEYDOWN, SDLK_F1+fn);
	    }else{
		mouse_stat.button[0] = MOUSE88_PRESS;
	    }
	}
	if( E.button.button == SDL_BUTTON_MIDDLE) mouse_stat.button[1] = MOUSE88_PRESS;
	if( E.button.button == SDL_BUTTON_RIGHT)  mouse_stat.button[2] = MOUSE88_PRESS;
      }else{   /* ButtonRelease */
	if( E.button.button == SDL_BUTTON_LEFT ) {
	    if (fn >= 0){
		scan_key_sub(SDL_KEYUP, SDLK_F1+fn);
	    }else{
		mouse_stat.button[0] = MOUSE88_RELEASE;
	    }
	}
	if( E.button.button == SDL_BUTTON_MIDDLE) mouse_stat.button[1] = MOUSE88_RELEASE;
	if( E.button.button == SDL_BUTTON_RIGHT)  mouse_stat.button[2] = MOUSE88_RELEASE;
      }
    }
  }


  /* �������� �֐��̏I���܂ŁA OS��ˑ��̂͂� */

  if( enable_b2menu ){
    if( mouse_stat.button[1] == MOUSE88_PRESS ) emu_mode = MENU;
  }


  switch( mouse_mode ){
  case 0:			/* --- �}�E�X�Ȃ� ----------------------- */
  case 4:			/* --- �W���C�X�e�B�b�N���L�[���[�h ----- */

    sound_reg[ 0x0e ] = 0xff;
    sound_reg[ 0x0f ] = 0xff;
    break;


  case 1:			/* --- �}�E�X���� ----------------------- */
  case 2:			/* --- �}�E�X���W���C�X�e�B�b�N���[�h --- */

    sound_reg[ 0x0f ] |= 0xfc;
    if( mouse_stat.button[0] == MOUSE88_PRESS   ) sound_reg[ 0x0f ] &= ~0x01;
    if( mouse_stat.button[0] == MOUSE88_RELEASE ) sound_reg[ 0x0f ] |=  0x01;
    if( mouse_stat.button[2] == MOUSE88_PRESS   ) sound_reg[ 0x0f ] &= ~0x02;
    if( mouse_stat.button[2] == MOUSE88_RELEASE ) sound_reg[ 0x0f ] |=  0x02;

    if( mouse_stat.moved ){
      x = mouse_stat.x;
      y = mouse_stat.y;
    }


    if( mouse_mode==1 ){	/* --- �}�E�X���� ----------------------- */

      if( mouse_stat.moved ){
	if( screen_size == SCREEN_SIZE_HALF ){	/* ��ʃT�C�Y�����̎��́A */
	  x *= 2;				/* �}�E�X�̍��W��{�ɂ��� */
	  y *= 2;
	}else if( screen_size == SCREEN_SIZE_DOUBLE ){
	  x /= 2;
	  y /= 2;
	}
      }

      if( fp_rec ){
	if( mouse_stat.moved ){
	  key_record.mouse_xh = (x>>8) & 0xff;
	  key_record.mouse_xl =  x     & 0xff;
	  key_record.mouse_yh = (y>>8) & 0xff;
	  key_record.mouse_yl =  y     & 0xff;
	}else{
	  key_record.mouse_xh = (char)0x80;
	  key_record.mouse_xl =       0x00;
	  key_record.mouse_yh = (char)0x80;
	  key_record.mouse_yl =       0x00;
	}
      }
      if( fp_pb ){
	x  = (int)key_record.mouse_xh << 8;
	x |=      key_record.mouse_xl;
	y  = (int)key_record.mouse_yh << 8;
	y |=      key_record.mouse_yl;

	if( (x&0xffff)==0x8000 && 
	    (y&0xffff)==0x8000  ) mouse_stat.moved = FALSE;
	else                      mouse_stat.moved = TRUE;
      }

      if( mouse_stat.moved ){
	mouse_x = x;
	mouse_y = y;
      }

    }else{			/* --- �}�E�X���W���C�X�e�B�b�N���[�h --- */

      if( mouse_stat.moved ){		    /* �}�E�X���ړ����Ă�����A     */
	int flag;
	int dx = x - mouse_x;			/* �O��Ƃ̍������㉺���E�� */
	int dy = y - mouse_y;			/* �u��������		    */
	mouse_x = x;
	mouse_y = y;
	if( dx==0 ){
	  if     ( dy==0  ) flag = 0xff;		/* ---- */
	  else if( dy > 0 ) flag = ~0x02;		/* ��   */
	  else              flag = ~0x01;		/* ��   */
	}else if( dx > 0 ){
	  int a = dy*100/dx;
	  if     ( a >  241 ) flag = ~0x02;		/* ��   */
	  else if( a >   41 ) flag = ~0x02 & ~0x08;	/* ���E */
	  else if( a >  -41 ) flag =         ~0x08;	/*   �E */
	  else if( a > -241 ) flag = ~0x01 & ~0x08;	/* ��E */
	  else                flag = ~0x01;		/* ��   */
	}else{
	  int a = -dy*100/dx;
	  if     ( a >  241 ) flag = ~0x02;		/* ��   */
	  else if( a >   41 ) flag = ~0x02 & ~0x04;	/* ���� */
	  else if( a >  -41 ) flag =         ~0x04;	/*   �� */
	  else if( a > -241 ) flag = ~0x01 & ~0x04;	/* �㍶ */
	  else                flag = ~0x01;		/* ��   */
	}
	sound_reg[ 0x0e ] = flag;
      }else{				    /* �}�E�X���ړ����ĂȂ���΁A   */
	sound_reg[ 0x0e ] = 0xff;		/* �㉺���E�����Ȃ�         */
      }
    }
    break;

  case 3:			/* --- �W���C�X�e�B�b�N���� ------------- */
    sound_reg[ 0x0e ] = 0xff;
    sound_reg[ 0x0f ] = 0xff;

    if( use_joystick ){
      int state = joystick_get_state();

      if( joy_swap_button==FALSE ){
	if( state & JOY88_A )   sound_reg[ 0x0f ] &= ~0x01;
	if( state & JOY88_B )   sound_reg[ 0x0f ] &= ~0x02;
      }else{
	if( state & JOY88_A )   sound_reg[ 0x0f ] &= ~0x02;
	if( state & JOY88_B )   sound_reg[ 0x0f ] &= ~0x01;
      }
      if( state & JOY88_UP )    sound_reg[ 0x0e ] &= ~0x01;
      if( state & JOY88_DOWN )  sound_reg[ 0x0e ] &= ~0x02;
      if( state & JOY88_LEFT )  sound_reg[ 0x0e ] &= ~0x04;
      if( state & JOY88_RIGHT ) sound_reg[ 0x0e ] &= ~0x08;
    }
    break;

  }



  if( fp_rec ){
    key_record.joy[0] = sound_reg[ 0x0e ];
    key_record.joy[1] = sound_reg[ 0x0f ];

    for( i=0; i<2; i++ ){
      int img;
      if( disk_image_exist( i ) &&
	  drive_check_empty( i ) == FALSE )
	img = disk_image_now_selected(i) + 1;
      else
	img = -1;
      if( key_record.image[i] != img ) key_record.image[i] = img;
      else                             key_record.image[i] = 0;
    }

    if(ex_fwrite( &key_record, sizeof(char), sizeof(key_record), fp_rec )
							== sizeof(key_record)){
      ;
    }else{
      if( verbose_proc ) printf("Can't Write Recording file <%s>\n", file_rec);
      ex_fclose( fp_rec );
      fp_rec = NULL;
    }
  }
  if( fp_pb ){
    sound_reg[ 0x0e ] = key_record.joy[0];
    sound_reg[ 0x0f ] = key_record.joy[1];

    for( i=0; i<2; i++ ){
      if( key_record.image[i]==-1 ){
	drive_set_empty( i );
      }else if( disk_image_exist( i ) &&
		key_record.image[i] > 0 &&
		key_record.image[i] <= disk_image_num_of_drive( i ) ){
	drive_unset_empty( i );
	disk_change_image( i, key_record.image[i]-1 );
      }
    }
  }



  if( sound_reg[ 0x07 ] & 0x80 ){
    sound_reg[ 0x0e ] = 0xff;
    sound_reg[ 0x0f ] = 0xff;
  }
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
  int	flag;
  int	x = mouse_x;
  int	y = mouse_y;


#if 1			/* X�̃}�E�X�𓮂����Ă���Ԃ����A88�ɉ��������� */
  if( mouse_stat.moved == FALSE ){
    mouse_image_x = x;
    mouse_image_y = y;
    mouse_dx = 0;
    mouse_dy = 0;

  }else
#endif			/* X�̃}�E�X���~�܂��Ă��A�ړ��ʕ�88�ɉ��������� */
  {

		/* �O��Ƃ̍������v�Z�B�������A-127�`127�͈̔͂Ɍ��� */


/* ���̌v�Z�Ŏg���}�N���B���Ȃ݂ɁAABS(x),SGN(x) �Ƃ��ɁAx!=0 ���O�� */
#define	ABS( x )	( ((x) >= 0)? x : -(x) )
#define	SGN( x )	( ((x) >= 0)? 1 : -1   )

  flag = 0;
  mouse_dx = x - mouse_image_x;
  mouse_dy = y - mouse_image_y;

  if( mouse_dx < -127 || mouse_dx > 127 ) flag |= 0x01;		/* �}127 �� */
  if( mouse_dy < -127 || mouse_dy > 127 ) flag |= 0x02;		/* �z������ */
  if( flag==0x03 ){						/* �̏���   */
    if( ABS(mouse_dx) > ABS(mouse_dy) ) flag = 0x01;
    else                                flag = 0x02;
  }
  switch( flag ){
  case 0x01:
    mouse_dy = 127*SGN(mouse_dx)*mouse_dy / mouse_dx;
    mouse_dx = 127*SGN(mouse_dx);
    break;
  case 0x02:
    mouse_dx = 127*SGN(mouse_dy)*mouse_dx / mouse_dy;
    mouse_dy = 127*SGN(mouse_dy);
    break;
  }

  mouse_image_x += mouse_dx;
  mouse_image_y += mouse_dy;

#undef	ABS
#undef	SGN
  }

  if( mouse_image_x < 0 ){
    if     ( mouse_image_y < 0   ){ mouse_image_x = 0;   mouse_image_y = 0;   }
    else if( mouse_image_y < 400 ){ mouse_image_x = 0;                        }
    else                          { mouse_image_x = 0;   mouse_image_y = 399; }
  }else if( mouse_image_x < 640 ){
    if     ( mouse_image_y < 0   ){                      mouse_image_y = 0;   }
    else if( mouse_image_y < 400 ){                                           }
    else                          {                      mouse_image_y = 399; }
  }else{
    if     ( mouse_image_y < 0   ){ mouse_image_x = 639; mouse_image_y = 0;   }
    else if( mouse_image_y < 400 ){ mouse_image_x = 639;                      }
    else                          { mouse_image_x = 639; mouse_image_y = 399; }
  }
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
  int	ret = suspend_work( fp, offset, 
			    suspend_keyboard_work, 
			    countof(suspend_keyboard_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->keyboard ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_keyboard(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_keyboard_work, 
			   countof(suspend_keyboard_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}



void	keyboard_init_at_resume( void )
{
  keyboard_init();		/* �L�[�ݒ�������� */
}
