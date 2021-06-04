/************************************************************************/
/*									*/
/* モニターモード開始／終了時の設定 (OS依存)				*/
/*									*/
/* 【関数】								*/
/* void	monitor_event_init( void )					*/
/*	・モニターモードに入った時に必要な初期化を、行なう		*/
/*									*/
/* void monitor_event_term( void )					*/
/*	・モニターモードを終える時に、monitor_event_init() で初期化した	*/
/*	  設定を破棄する。						*/
/*	  また、同時に QUASI88 のエミュレートモードに応じた設定を	*/
/*	  ここで再度行なう。						*/
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
	/* オートリピート ON */

  XAutoRepeatOn( display );

	/* グラブの解除 */

  XUngrabPointer (display, CurrentTime);

  XFlush( display );
}

void	monitor_event_term( void )
{
	/* オートリピート OFF にはしない！ */


	/* グラブの設定	*/

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
