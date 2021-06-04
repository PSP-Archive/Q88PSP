/************************************************************************/
/*									*/
/* �W���C�X�e�B�b�N����							*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "keyboard.h"	/* mouse_mode */
#include "joystick.h"


int	use_joystick      = FALSE;	/* �W���C�X�e�B�b�N�̎g�p�̗L�� */
int	joy_swap_button   = FALSE;	/* �{�^����AB�����ւ���  	*/
int	joy_key_assign[2] =		/* AB�{�^�����L�[�Ɋ��蓖�Ă�	*/
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


#if	defined( USE_JOY_LINUX_USB )	/* --------------------------------- */

/*
   Linux�p USB joystick �h���C�o
	�ȉ��̃R�[�h�́A���n���� [eiichi@licorp.co.jp] �ɂ��񋟂���܂����B
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef NAKAMIZO
unsigned char joy_code[][2][4] = {
    {{0x01, 0x80, 0x02, 0x01}, {0xef, 0x53, 0x02, 0x01}},    /* �� */
    {{0xff, 0x7f, 0x02, 0x01}, {0xef, 0x53, 0x02, 0x01}},    /* �� */
    {{0xff, 0x7f, 0x02, 0x00}, {0xef, 0x53, 0x02, 0x00}},    /* �E */
    {{0x01, 0x80, 0x02, 0x00}, {0xef, 0x53, 0x02, 0x00}},    /* �� */
    {{0x01, 0x00, 0x01, 0x0b}, {0x00, 0x00, 0x01, 0x0b}},    /* start */
    {{0x01, 0x00, 0x01, 0x0a}, {0x00, 0x00, 0x01, 0x0a}},    /* select */
    {{0x01, 0x00, 0x01, 0x00}, {0x00, 0x00, 0x01, 0x00}},    /* A */
    {{0x01, 0x00, 0x01, 0x01}, {0x00, 0x00, 0x01, 0x01}},    /* B */
    {{0x01, 0x00, 0x01, 0x02}, {0x00, 0x00, 0x01, 0x02}},    /* C */
    {{0x01, 0x00, 0x01, 0x03}, {0x00, 0x00, 0x01, 0x03}},    /* X */
    {{0x01, 0x00, 0x01, 0x04}, {0x00, 0x00, 0x01, 0x04}},    /* Y */
    {{0x01, 0x00, 0x01, 0x05}, {0x00, 0x00, 0x01, 0x05}},    /* Z */
    {{0x01, 0x00, 0x01, 0x06}, {0x00, 0x00, 0x01, 0x06}},    /* R1 */
    {{0x01, 0x00, 0x01, 0x08}, {0x00, 0x00, 0x01, 0x08}},    /* R2 */
    {{0x01, 0x00, 0x01, 0x07}, {0x00, 0x00, 0x01, 0x07}},    /* L1 */
    {{0x01, 0x00, 0x01, 0x09}, {0x00, 0x00, 0x01, 0x09}}     /* L2 */
};
#endif /* NAKAMIZO */

unsigned char joy_code[][2][4] = {
    {{0x01, 0x80, 0x02, 0x01}, {0x00, 0x00, 0x02, 0x01}},    /* �� */
    {{0xff, 0x7f, 0x02, 0x01}, {0x00, 0x00, 0x02, 0x01}},    /* �� */
    {{0xff, 0x7f, 0x02, 0x00}, {0x00, 0x00, 0x02, 0x00}},    /* �E */
    {{0x01, 0x80, 0x02, 0x00}, {0x00, 0x00, 0x02, 0x00}},    /* �� */
    {{0x01, 0x00, 0x01, 0x0b}, {0x00, 0x00, 0x01, 0x0b}},    /* start */
    {{0x01, 0x00, 0x01, 0x0a}, {0x00, 0x00, 0x01, 0x0a}},    /* select */
    {{0x01, 0x00, 0x01, 0x00}, {0x00, 0x00, 0x01, 0x00}},    /* A */
    {{0x01, 0x00, 0x01, 0x01}, {0x00, 0x00, 0x01, 0x01}},    /* B */
    {{0x01, 0x00, 0x01, 0x02}, {0x00, 0x00, 0x01, 0x02}},    /* C */
    {{0x01, 0x00, 0x01, 0x03}, {0x00, 0x00, 0x01, 0x03}},    /* X */
    {{0x01, 0x00, 0x01, 0x04}, {0x00, 0x00, 0x01, 0x04}},    /* Y */
    {{0x01, 0x00, 0x01, 0x05}, {0x00, 0x00, 0x01, 0x05}},    /* Z */
    {{0x01, 0x00, 0x01, 0x06}, {0x00, 0x00, 0x01, 0x06}},    /* R1 */
    {{0x01, 0x00, 0x01, 0x08}, {0x00, 0x00, 0x01, 0x08}},    /* R2 */
    {{0x01, 0x00, 0x01, 0x07}, {0x00, 0x00, 0x01, 0x07}},    /* L1 */
    {{0x01, 0x00, 0x01, 0x09}, {0x00, 0x00, 0x01, 0x09}}     /* L2 */
};

static JOY_BUTTON joy_button[10];       /* �ω��̂����� Joystick �̃{�^�� */
static FILE*   joystick_device;         /* Joystick �f�o�C�X */
static int	joystick_state;

static int press_up_down = 0;           /* �R���g���[���㉺�{�^��������� */
static int press_right_left = 0;        /* �R���g���[�����E�{�^��������� */

int joy_init(void)
{
    int   fd;

    /*
    if( mouse_mode != 3 ){
      joystick_device = NULL;
      return;
    }
    */

    fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (fd != -1) {
	joystick_device = fdopen(fd, "r");
    } else {
	joystick_device = NULL;
    }

    if( joystick_device ) return TRUE;
    else                  return FALSE;
}

void joy_term(void)
{
    if (joystick_device) {
	ex_fclose(joystick_device);
	joystick_device = NULL;
    }
}

void scan_joy(void)
{
    int i;
    int j;
    int len;
    unsigned char joydata[8];
    joystick_state = 0;

    j = 0;
    for (i = 0; i < 10; i ++) {
	joy_button[i].button = -1;
	joy_button[i].is_press = -1;
    }
    if (joystick_device != NULL) {
	while ((len =ex_fread(joydata, 1, sizeof(joydata), joystick_device)) == 8) {
	    if (len == 8) {
		for (i = 0; i <= 16; i ++) {
		    if (!memcmp(joydata + 4, joy_code[i][0], 4)) {
			joy_button[j].button = i;
			joy_button[j].is_press = 1;
			j ++;
			break;
		    }
		    if (!memcmp(joydata + 4, joy_code[i][1], 4)) {
			joy_button[j].button = i;
			joy_button[j].is_press = 0;
			j ++;
			break;
		    }
		}
		if (j == 10) {
		    break;
		}
	    }
	}
    }

    if( joystick_device ){

      for (i = 0; i < 10; i ++) {
	if (joy_button[i].button == -1) {
	  break;
	}
	if (joy_button[i].button == JOY_BUTTON_A) {
	  if (joy_button[i].is_press) {
	    joystick_state |= JOY88_A;
	  } else {
	    joystick_state &= ~JOY88_A;
	  }
	} else if (joy_button[i].button == JOY_BUTTON_B) {
	  if (joy_button[i].is_press) {
	    joystick_state |= JOY88_B;
	  } else {
	    joystick_state &= ~JOY88_B;
	  }
	} else if (joy_button[i].button == JOY_BUTTON_UP) {
	  if (joy_button[i].is_press) {
	      press_up_down = 1;
	  } else {
	      press_up_down = 0;
	  }
	} else if (joy_button[i].button == JOY_BUTTON_DOWN) {
	  if (joy_button[i].is_press) {
	      press_up_down = 2;
	  } else {
	      press_up_down = 0;
	  }
	} else if (joy_button[i].button == JOY_BUTTON_RIGHT) {
	  if (joy_button[i].is_press) {
	      press_right_left = 1;
	  } else {
	      press_right_left = 0;
	  }
	} else if (joy_button[i].button == JOY_BUTTON_LEFT) {
	  if (joy_button[i].is_press) {
	      press_right_left = 2;
	  } else {
	      press_right_left = 0;
	  }
	}
      }

      /* �㉺�{�^�� */
      joystick_state &= ~(JOY88_UP|JOY88_DOWN);
      if (press_up_down == 1) {
	joystick_state |= JOY88_UP;
      } else if (press_up_down == 2) {
	joystick_state |= JOY88_DOWN;
      }

      /* ���E�{�^�� */
      joystick_state &= ~(JOY88_LEFT|JOY88_RIGHT);
      if (press_right_left == 1) {
	joystick_state |= JOY88_RIGHT;
      } else if (press_right_left == 2) {
	joystick_state |= JOY88_LEFT;
      }

    }

    return;
}
	
int joystick_get_state(void)
{
  if( joystick_device ) return joystick_state;
  else                  return 0;
}

#elif	defined( USE_JOY_SDL )		/* --------------------------------- */


#include <SDL.h>

static	SDL_Joystick *joy = NULL;
static	int	      joy_state;

int joy_init(void)
{
  joy = NULL;
  joy_state = 0;

  if( ! SDL_WasInit( SDL_INIT_JOYSTICK ) ){
    if( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) ){
      return FALSE;
    }
  }

  if( SDL_NumJoysticks() >= 1 ){	/* �W���C�X�e�B�b�N�����邩�`�F�b�N */

    joy = SDL_JoystickOpen(0);		/* �W���C�X�e�B�b�N���I�[�v�� */

    if( joy )
      SDL_JoystickEventState( SDL_IGNORE );
  }

  if( joy ) return TRUE;
  else      return FALSE;
}

void joy_term(void)
{
  if( joy ){
    SDL_JoystickClose( joy );
  }
  joy = NULL;
}

void scan_joy(void)
{
  joy_state = 0;

  if( joy ){
    Uint8 button1, button2;
    Sint16 x,y;

    SDL_JoystickUpdate();

    /* Uint8 pad = SDL_JoystickGetHat( joy, SDL_HAT_UP   | SDL_HAT_RIGHT | 
					    SDL_HAT_DOWN | SDL_HAT_LEFT ); */

    x = SDL_JoystickGetAxis( joy, 0 );
    y = SDL_JoystickGetAxis( joy, 1 );

    button1 = SDL_JoystickGetButton( joy, 0 );
    button2 = SDL_JoystickGetButton( joy, 1 );

    if( y < -0x4000 ) joy_state |= JOY88_UP;
    if( y >  0x4000 ) joy_state |= JOY88_DOWN;
    if( x < -0x4000 ) joy_state |= JOY88_LEFT;
    if( x >  0x4000 ) joy_state |= JOY88_RIGHT;
    if( button1 )     joy_state |= JOY88_A;
    if( button2 )     joy_state |= JOY88_B;
  }
}

int joystick_get_state(void)
{
  if( joy ) return joy_state;
  else      return 0;
}

#else					/*----------------------------------*/

/* opps !*/

#endif					/*----------------------------------*/

#endif			/*##################################################*/

