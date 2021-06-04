/*
 *	Joystick ヘッダファイル
 */

/*
   Linux用 USB joystick は引地さん [eiichi@licorp.co.jp] により提供されました。
*/

#ifndef _JOYSTICK_H_

extern	int	use_joystick;		/* ジョイスティックの使用の有無 */
extern	int	joy_swap_button;	/* ボタンのABを入れ替える  	*/
extern	int	joy_key_assign[2];	/* ABボタンをキーに割り当てる	*/


/* ジョイスティックのボタンとキーの割り当て対応 */

#define	JOY88_KEY_NONE		0
#define	JOY88_KEY_X		1
#define	JOY88_KEY_Z		2
#define	JOY88_KEY_SPACE		3
#define	JOY88_KEY_RET		4
#define	JOY88_KEY_SHIFT		5
#define	JOY88_KEY_ESC		6


/* int  joystick_get_state(void) の戻り値	*/
/* ジョイスティック未実装時は、0 を返すこと	*/

#define	JOY88_UP	0x0001
#define	JOY88_DOWN	0x0002
#define	JOY88_LEFT	0x0004
#define	JOY88_RIGHT	0x0008
#define	JOY88_A		0x0010
#define	JOY88_B		0x0020



#ifdef	USE_JOY				/* --------------------------------- */

void joystick_init(void);
void joystick_term(void);
void scan_joystick(void);
void joystick_restart( void );
int  joystick_get_state(void);

#else	/* !USE_JOY */			/* --------------------------------- */

#define	joystick_init()		use_joystick = FALSE;
#define	joystick_term()
#define	scan_joystick()
#define	joystick_restart()
#define	joystick_get_state()	(0)

#endif	/* !USE_JOY */



#if	defined( USE_JOY_LINUX_USB )	/* --------------------------------- */

#include <stdio.h>

#define JOY_BUTTON_UP      0
#define JOY_BUTTON_DOWN    1
#define JOY_BUTTON_RIGHT   2
#define JOY_BUTTON_LEFT    3
#define JOY_BUTTON_START   4
#define JOY_BUTTON_SELECT  5
#define JOY_BUTTON_A       6
#define JOY_BUTTON_B       7
#define JOY_BUTTON_C       8
#define JOY_BUTTON_X       9
#define JOY_BUTTON_Y       10
#define JOY_BUTTON_Z       11
#define JOY_BUTTON_R1      12
#define JOY_BUTTON_R2      13
#define JOY_BUTTON_L1      14
#define JOY_BUTTON_L2      15

typedef struct {
    int button;
    int is_press;
} JOY_BUTTON;

extern JOY_BUTTON joy_button[10];        /* 変化のあった Joystick のボタン */
extern FILE*   joystick_device;          /* Joystick デバイス */

int  joy_init(void);
void joy_term(void);
void scan_joy(void);

#elif	defined( USE_JOY_SDL )		/* --------------------------------- */

int  joy_init(void);
void joy_term(void);
void scan_joy(void);

#endif					/* --------------------------------- */

#endif /*_JOYSTICK_H_*/
