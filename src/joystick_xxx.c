/************************************************************************/
/*									*/
/* ジョイスティック入力							*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "keyboard.h"	/* mouse_mode */
#include "joystick.h"


int	use_joystick      = FALSE;	/* ジョイスティックの使用の有無 */
int	joy_swap_button   = FALSE;	/* ボタンのABを入れ替える  	*/
int	joy_key_assign[2] =		/* ABボタンをキーに割り当てる	*/
{
  JOY88_KEY_X, JOY88_KEY_Z,
};


#ifdef	USE_JOY		/*##################################################*/

void	joystick_init( void )
{
  if( mouse_mode==3 || mouse_mode==4 )
    use_joystick = joy_init();
  else
    use_joystick = FALSE;
}

void	joystick_term( void )
{
  if( use_joystick ) joy_term();
}

void	scan_joystick( void )
{
  if( use_joystick ) scan_joy();
}

void	joystick_restart( void )
{
  if( mouse_mode==3 || mouse_mode==4 ){
    if( use_joystick == FALSE ){
      joystick_init();
    }
  }else{
    joystick_term();
    use_joystick = FALSE;
  }
}


int joy_init(void)
{
      return FALSE;
}

void joy_term(void)
{
}

void scan_joy(void)
{
}

int joystick_get_state(void)
{
}

#endif			/*##################################################*/

