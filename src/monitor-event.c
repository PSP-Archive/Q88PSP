/************************************************************************/
/*									*/
/* ���j�^�[���[�h�J�n�^�I�����̐ݒ� (OS�ˑ�)				*/
/*									*/
/* �y�֐��z								*/
/* void	monitor_event_init( void )					*/
/*	�E���j�^�[���[�h�ɓ��������ɕK�v�ȏ��������A�s�Ȃ�		*/
/*									*/
/* void monitor_event_term( void )					*/
/*	�E���j�^�[���[�h���I���鎞�ɁAmonitor_event_init() �ŏ���������	*/
/*	  �ݒ��j������B						*/
/*	  �܂��A������ QUASI88 �̃G�~�����[�g���[�h�ɉ������ݒ��	*/
/*	  �����ōēx�s�Ȃ��B						*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "graph.h"
#include "monitor-event.h"

#include "device.h"



#undef	MENU_EVENT_DEPEND_X11
#ifdef	MENU_EVENT_DEPEND_X11

void	monitor_event_init( void )
{
	/* �I�[�g���s�[�g ON */

  XAutoRepeatOn( display );

	/* �O���u�̉��� */

  XUngrabPointer (display, CurrentTime);

  XFlush( display );
}

void	monitor_event_term( void )
{
	/* �I�[�g���s�[�g OFF �ɂ͂��Ȃ��I */


	/* �O���u�̐ݒ�	*/

  if( grab_mouse ){
    XGrabPointer( display, window, True, 0, GrabModeAsync, GrabModeAsync, 
		  window, None, CurrentTime );
  }

  XFlush( display );
}


#else

void	monitor_event_init( void )
{
}

void	monitor_event_term( void )
{
}

#endif
