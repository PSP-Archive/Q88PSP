#ifndef MENU_EVENT_H_INCLUDED
#define MENU_EVENT_H_INCLUDED

/************************************************************************/
/* メニューモードので、イベントをチェックする関数			*/
/************************************************************************/

void	menu_event_init( void );
void	menu_event_term( void );


enum{
  MENU_EVENT_NOTHING,
  MENU_EVENT_KEY_ON,
  MENU_EVENT_MOUSE_ON,
  MENU_EVENT_MOUSE_OFF,
  MENU_EVENT_MOUSE_MOVE,
  MENU_EVENT_QUIT,
  MENU_EVENT_END
};

void	menu_check_event( int *type, int *button, int *x, int *y );
void	menu_init_event( int *win_x, int *win_y );
void	menu_clear_event( void );

void	menu_quit_signal( void );




#endif	/* MENU_EVENT_H_INCLUDED */
