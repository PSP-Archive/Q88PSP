/************************************************************************/
/*									*/
/* QUASI88 ���j���[�p Tool Kit						*/
/*				Core lib				*/
/*									*/
/*	GTK+ �� API ��^���č��܂����B��₱�������āA�h�L�������g��	*/
/*	�����܂���c�c�c�B						*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"

#include "q8tk.h"
#include "q8tk-glib.h"

#include "menu-event.h"
#include "menu-screen.h"

#include "file-op.h"
#include "wait.h"


/* �R���{�{�b�N�X�ɐݒ肷�镶�����A�R���{�̃��X�g�ȊO�ł��F�߂� */
#define	ENABLE_COMBO_SET_TEXT_ALL_WORD

/* �f�o�b�O�p */
#define	Q8TK_ASSERT_DEBUG


/************************************************************************/
/* �f�o�b�O								*/
/************************************************************************/

/*----------------------------------------------------------------------*/

#ifdef	Q8TK_ASSERT_DEBUG

#define	Q8tkAssert(e,s)						\
	((e) ? (void)0 : _Q8tkAssert(__FILE__, __LINE__, #e, s ))
static	void	_Q8tkAssert( char *file, int line, char *exp, const char *s )
{
  fprintf( stderr, "Fatal Error | %s <%s:%d>\n", exp, file, line );
  fprintf( stderr, "message = %s\n",(s)?s:"---");
  main_exit(1);
}
#else

#define	Q8tkAssert(e,s)		((void)0)

#endif

/*----------------------------------------------------------------------*/

#ifdef	Q8TK_ASSERT_DEBUG

static const char *debug_type(int type){
  switch( type ){
  case Q8TK_TYPE_WINDOW:	return "window  :";
  case Q8TK_TYPE_BUTTON:	return "button  :";
  case Q8TK_TYPE_TOGGLE_BUTTON:	return "t-button:";
  case Q8TK_TYPE_CHECK_BUTTON:	return "c-button:";
  case Q8TK_TYPE_RADIO_BUTTON:	return "r-button:";
  case Q8TK_TYPE_FRAME:		return "frame   :";
  case Q8TK_TYPE_LABEL:		return "label   :";
  case Q8TK_TYPE_NOTEBOOK:	return "notebook:";
  case Q8TK_TYPE_NOTEPAGE:	return "page    :";
  case Q8TK_TYPE_VBOX:		return "vbox    :";
  case Q8TK_TYPE_HBOX:		return "hbox    :";
  case Q8TK_TYPE_VSEPARATOR:	return "vsep    :";
  case Q8TK_TYPE_HSEPARATOR:	return "hsep    :";
  case Q8TK_TYPE_COMBO:		return "combo   :";
  case Q8TK_TYPE_LIST:		return "list    :";
  case Q8TK_TYPE_LIST_ITEM:	return "listitem:";
  case Q8TK_TYPE_ADJUSTMENT:	return "adjust  :";
  case Q8TK_TYPE_HSCALE:	return "hscale  :";
  case Q8TK_TYPE_VSCALE:	return "vscale  :";
  case Q8TK_TYPE_SCROLLED_WINDOW:return "scrolled:";
  case Q8TK_TYPE_ENTRY:		return "entry   :";
  case Q8TK_TYPE_OPTION_MENU:	return "option  :";
  case Q8TK_TYPE_MENU:		return "menu    :";
  case Q8TK_TYPE_RADIO_MENU_ITEM:return "r-menuI:";
  case Q8TK_TYPE_MENU_LIST:	return "menu-lst:";
  case Q8TK_TYPE_DIALOG:	return "dialog  :";
  case Q8TK_TYPE_FILE_SELECTION:return "f-select:";
  }
  return "UNDEF TYPE:";
}

#else

#define	debug_type(t)		((void)0)

#endif


/************************************************************************/
/* �G���[����								*/
/************************************************************************/
#define	CHECK_MALLOC_OK(e,s)	((e) ? (void)0 : _CHECK_MALLOC_OK(s))

static	void	_CHECK_MALLOC_OK( const char *s )
{
  fprintf( stderr, "Fatal Error : %s exhaused!\n", s );
  main_exit(1);
}


/************************************************************************/
/* ���[�N								*/
/************************************************************************/

#define	MAX_WIDGET		(4096)
static	Q8tkWidget	*widget_table[ MAX_WIDGET ];


#define	MAX_LIST		(1024)
static	Q8List		*list_table[ MAX_LIST ];


#define	MAX_WINDOW_LEVEL	(8)
static	Q8tkWidget	*window_level[ MAX_WINDOW_LEVEL ];
static	int		window_level_now;
static	Q8tkWidget	*event_widget[ MAX_WINDOW_LEVEL ];

#define	set_event_widget(w)	event_widget[ window_level_now ] = (w)
#define	get_event_widget()	event_widget[ window_level_now ]


static	char	file_selection_pathname[ Q8TK_MAX_FILENAME ];
static	char	file_selection_filename[ Q8TK_MAX_FILENAME ];


static	struct{
  int	x, y;
  int	x_old, y_old;
  int	button, button_old, button_on, button_off;
} mouse;


/*----------------------------------------------------------------------*/

static	void	widget_map( Q8tkWidget *widget );
static	void	widget_construct( void );
static	void	widget_signal_do( Q8tkWidget *widget, const char *name );
static	void	widget_destroy_child( Q8tkWidget *widget );
static	void	widget_destroy_all( Q8tkWidget *widget );
static	void	widget_redraw_now( void );



/************************************************************************/
/* ���I���[�N�̊m�ہ^�J��						*/
/************************************************************************/
/*--------------------------------------------------------------
 *	Widget
 *--------------------------------------------------------------*/
static	Q8tkWidget	*malloc_widget( void )
{
  int		i;
  Q8tkWidget	*w;

  for( i=0; i<MAX_WIDGET; i++ ){
    if( widget_table[i] == NULL ){
      w = (Q8tkWidget *)calloc( 1, sizeof(Q8tkWidget ) );
      if( w ){
	widget_table[i] = w;
	return w;
      }else{
	Q8tkAssert(FALSE,"memory exhoused");
	CHECK_MALLOC_OK( FALSE, "memory" );
	break;
      }
    }
  }
  Q8tkAssert(FALSE,"work 'widget' exhoused");
  CHECK_MALLOC_OK( FALSE, "work 'widget'" );
  return NULL;
}
static	void	free_widget( Q8tkWidget *w )
{
  int	i;
  for( i=0; i<MAX_WIDGET; i++ ){
    if( widget_table[i] == w ){
      ex_free( w );
      widget_table[i] = NULL;
      return;
    }
  }
  Q8tkAssert(FALSE,"pointer is not malloced widget");
}
/*--------------------------------------------------------------
 *	List
 *--------------------------------------------------------------*/
static	Q8List	*malloc_list( void )
{
  int		i;
  Q8List	*l;

  for( i=0; i<MAX_LIST; i++ ){
    if( list_table[i] == NULL ){
      l = (Q8List *)calloc( 1, sizeof(Q8List ) );
      if( l ){
	list_table[i] = l;
	return l;
      }else{
	Q8tkAssert(FALSE,"memory exhoused");
	CHECK_MALLOC_OK( FALSE, "memory" );
	break;
      }
    }
  }
  Q8tkAssert(FALSE,"work 'list' exhoused");
  CHECK_MALLOC_OK( FALSE, "work 'list'" );
  return NULL;
}
static	void	free_list( Q8List *l )
{
  int	i;
  for( i=0; i<MAX_LIST; i++ ){
    if( list_table[i] == l ){
      ex_free( l );
      list_table[i] = NULL;
      return;
    }
  }
  Q8tkAssert(FALSE,"pointer is not malloced list");
}




/************************************************************************/
/* Q8TK ��p �����񏈗�							*/
/*	q8_strncpy( s, ct, n )						*/
/*		������ ct �� ������ s �� �R�s�[����B			*/
/*		s �̕�����I�[�́A�K�� '\0' �ƂȂ�As �̒����� n-1 ����	*/
/*		�ȉ��Ɏ��܂�B						*/
/*	q8_strncat( s, ct, n )						*/
/*		������ ct �� ������ s �� �t������B			*/
/*		s �̕�����I�[�́A�K�� '\0' �ƂȂ�As �̒����� n-1 ����	*/
/*		�ȉ��Ɏ��܂�B						*/
/************************************************************************/
static	void	q8_strncpy( char *s, const char *ct, size_t n )
{
  strncpy( s, ct, n-1 );
  s[ n-1 ] = '\0';
}
static	void	q8_strncat( char *s, const char *ct, size_t n )
{
  if( n > strlen(s) + 1 )
    strncat( s, ct, n - strlen(s) -1 );
}


/************************************************************************/
/* ���X�g����								*/
/************************************************************************/
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
Q8List	*q8_list_append( Q8List *list, void *ptr )
{
  Q8List	*new_list;

  new_list  = malloc_list();

  if( list ){
    list->next     = new_list;
    new_list->prev = list;
    new_list->next = NULL;
  }else{
    new_list->prev = NULL;
    new_list->next = NULL;
  }
  new_list->ptr    = ptr;

  return new_list;
}
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8_list_free( Q8List *list )
{
  Q8List *l;

  list = q8_list_first( list );

  while( list ){
    l = list->next;
    free_list( list );
    list = l;
  }
}
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
Q8List	*q8_list_first( Q8List *list )
{
  while( list->prev ){
    list = list->prev;
  }
  return list;
}	
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
Q8List	*q8_list_last( Q8List *list )
{
  while( list->next ){
    list = list->next;
  }
  return list;
}	


/************************************************************************/
/* �������ƏI��								*/
/************************************************************************/

static	int		q8tk_main_loop_flag;
static	int		q8tk_construct_flag;
static	int		q8tk_draw_flag;

#define	set_main_loop_flag(f)	q8tk_main_loop_flag = (f)
#define	get_main_loop_flag()	q8tk_main_loop_flag
#define	set_construct_flag(f)	q8tk_construct_flag = (f)
#define	get_construct_flag()	q8tk_construct_flag
#define	set_draw_flag(f)	q8tk_draw_flag = (f)
#define	get_draw_flag()		q8tk_draw_flag

static	Q8tkWidget	*q8tk_drag_widget;
#define	set_drag_widget(w)	q8tk_drag_widget = (w)
#define	get_drag_widget()	q8tk_drag_widget

static	int		active_widget_flag;
#define	check_active( widget )						\
		do{							\
		  if( (widget)==get_event_widget() ) active_widget_flag=TRUE; \
		}while(0)
#define	check_active_finish()	active_widget_flag=FALSE
#define	is_active_widget()	(active_widget_flag)


static	Q8tkWidget	*q8tk_tab_top_widget;
#define	set_tab_top_widget(w)	q8tk_tab_top_widget = (w)
#define	get_tab_top_widget()	q8tk_tab_top_widget


#define	NR_SCROLL_ADJ	(8)
static	struct{
  int		drawn;
  Q8tkWidget	*widget;
} q8tk_scroll_adj[ NR_SCROLL_ADJ ];

static	void	init_scroll_adj_widget( void )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    q8tk_scroll_adj[i].drawn    = FALSE;
    q8tk_scroll_adj[i].widget   = NULL;
  }
}

static	void	set_scroll_adj_widget( Q8tkWidget *w )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    if( q8tk_scroll_adj[i].widget==w ) return;
  }
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    if( q8tk_scroll_adj[i].widget==NULL ){
      q8tk_scroll_adj[i].drawn  = FALSE;
      q8tk_scroll_adj[i].widget = w;
      return;
    }
  }
}
static	void	check_scroll_adj_widget( Q8tkWidget *w )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    if( q8tk_scroll_adj[i].widget==w ){
      q8tk_scroll_adj[i].drawn = TRUE;
      return;
    }
  }
}




/*--------------------------------------------------------------
 *	������
 *--------------------------------------------------------------*/
void	q8tk_init( void )
{
  int	i;
  for( i=0; i<MAX_WIDGET; i++ ){
    widget_table[i] = NULL;
  }

  for( i=0; i<MAX_LIST; i++ ){
    list_table[i] = NULL;
  }

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){
    window_level[i] = NULL;
    event_widget[i] = NULL;
  }
  window_level_now = -1;

  set_main_loop_flag( TRUE );
  set_construct_flag( TRUE );
  set_draw_flag     ( TRUE );

  set_drag_widget( NULL );

  init_scroll_adj_widget();


  q8gr_clear_screen();

  strcpy( file_selection_pathname, "" );
  strcpy( file_selection_filename, "" );

  memset( &mouse, 0x00, sizeof(mouse) );
  menu_init_event( &mouse.x, &mouse.y );

  wait_vsync_reset();
}
/*--------------------------------------------------------------
 *	�I��
 *--------------------------------------------------------------*/
void	q8tk_term( void )
{
  int	i;
  for( i=0; i<MAX_WIDGET; i++ ){
    if( widget_table[i] ){
      if( widget_table[i]->name ) ex_free( widget_table[i]->name );
      ex_free( widget_table[i] );
    }
  }
  for( i=0; i<MAX_LIST; i++ ){
    if( list_table[i] ){
      ex_free( list_table[i] );
    }
  }
}



/************************************************************************/
/* ���[�_���̐ݒ�							*/
/************************************************************************/
/*--------------------------------------------------------------
 *	���[�_���ݒ�
 *--------------------------------------------------------------*/
void	q8tk_grab_add( Q8tkWidget *widget )
{
  int	i;
  Q8tkAssert(widget->type==Q8TK_TYPE_WINDOW,"grab add not window");

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){
    if( window_level[i] == NULL ){
      window_level[i] = widget;
      window_level_now= i;
      set_construct_flag( TRUE );
      return;
    }
  }
  Q8tkAssert(FALSE,"overflow window level");
}
/*--------------------------------------------------------------
 *	���[�_������
 *--------------------------------------------------------------*/
void	q8tk_grab_remove( Q8tkWidget *widget )
{
  int	i;

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){
    if( window_level[i] == widget ){
      break;
    }
  }
  Q8tkAssert(i<MAX_WINDOW_LEVEL,"grab remove not widget");
  for(    ; i<MAX_WINDOW_LEVEL-1; i++ ){
    window_level[i] = window_level[i+1];
    event_widget[i] = event_widget[i+1];
  }
  window_level[i] = NULL;
  event_widget[i] = NULL;
  window_level_now --;
  set_construct_flag( TRUE );
}



/********************************************************/
/* �E�B�W�b�g�̍쐬					*/
/********************************************************/

/*--------------------------------------------------------------
 * WINDOW
 *	�E�S�ẴE�B�W�b�g�̍ł���c�ɂȂ�B
 *	�E�q������Ă�B
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	�E���̃E�B�W�b�g��\������ɂ́Aq8tk_grab_add()�ɂāA
 *	  �����I�ɕ\�����w������B
 *	�EWINDOW �͍ő�AMAX_WINDOW_LEVEL�쐬�ł��邪�A�C�x���g
 *	  ���󂯕t����̂́A�Ō�� q8tk_grab_add() �𔭍s����
 *	  WINDOW �̎q���݂̂ł���B
 *	�EWINDOW �́A3�Ɏ�ނ킯����Anew()�̈����Ō��܂�
 *		Q8TK_WINDOW_TOPLEVEL �c ��Ԋ�b�� WINDOW
 *					�B��̂ݐ����\
 *					�E�C���h�E�g�������Ȃ�
 *		Q8TK_WINDOW_DIALOG   �c �o���ς����E�C���h�E�g������
 *		Q8TK_WINDOW_POPUP    �c �P���ȃE�C���h�E�g������
 *	�E�V�O�i�� �c �Ȃ�
 *	--------------------------------------------------------
 *	<TOP> - WINDOW - xxx
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_window_new( int window_type )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_WINDOW;
  w->attr   = Q8TK_ATTR_CONTAINER;

  switch( window_type ){
  case Q8TK_WINDOW_TOPLEVEL:
    Q8tkAssert(window_level[0]==NULL,"Redefine TOP window");
    q8tk_grab_add( w );
    w->stat.window.no_frame = TRUE;
    break;

  case Q8TK_WINDOW_DIALOG:
    w->stat.window.no_frame = FALSE;
    w->stat.window.shadow_type = Q8TK_SHADOW_OUT;
    break;

  case Q8TK_WINDOW_POPUP:
    w->stat.window.no_frame = FALSE;
    w->stat.window.shadow_type = Q8TK_SHADOW_ETCHED_OUT;
    break;
  }

  return	w;
}

/*--------------------------------------------------------------
 * BUTTON
 *	�E�W���I�ȃ{�^���B�{�^���������ƈ������݁A�����Ɩ߂�B
 *	�E�q������Ă�B
 *	  ���A�����I�ɂ́ALABEL �ȊO�������Ƃ͂Ȃ����낤�B
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	�E�V�O�i��
 *		"clicked"	�{�^���������ꂽ���ɔ���
 *	--------------------------------------------------------
 *	- BUTTON - xxx		(���Axxx ��LABELL )
 *
 *--------------------------------------------------------------*/
static	void	button_event_button_on( Q8tkWidget *widget )
{
  widget->stat.button.active = Q8TK_BUTTON_ON;

  widget_redraw_now();		/* ��U�ĕ`�� */
  widget->stat.button.active = Q8TK_BUTTON_OFF;
  widget_signal_do( widget, "clicked" );

  set_construct_flag( TRUE );
}
static	void	button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    button_event_button_on( widget );
  }
}
Q8tkWidget	*q8tk_button_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on  = button_event_button_on;
  w->event_key_on     = button_event_key_on;

  return	w;
}
Q8tkWidget	*q8tk_button_new_with_label( const char *label )
{
  Q8tkWidget	*b = q8tk_button_new();
  Q8tkWidget	*l  = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}

/*--------------------------------------------------------------
 * TOGGLE BUTTON
 *	�E�g�O���{�^���B�{�^���������ƈ������ށB������x�{�^����
 *	  �����Ɩ߂�B
 *	�E�q������Ă�B
 *	  ���A�����I�ɂ́ALABEL �ȊO�������Ƃ͂Ȃ����낤�B
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	�E�V�O�i��
 *		"clicked"	�{�^���������ꂽ���ɔ���
 *		"toggled"	�{�^���̏�Ԃ��ω��������ɔ���
 *	--------------------------------------------------------
 *	- TOGGLE BUTTON - xxx		(���Axxx ��LABELL )
 *
 *--------------------------------------------------------------*/
static	void	toggle_button_event_button_on( Q8tkWidget *widget )
{
  if( widget->stat.button.active == Q8TK_BUTTON_ON ){
    widget->stat.button.active = Q8TK_BUTTON_OFF;
    widget_signal_do( widget, "toggled" );
  }else{
    widget->stat.button.active = Q8TK_BUTTON_ON;
    widget_signal_do( widget, "clicked" );
    widget_signal_do( widget, "toggled" );
  }
  set_construct_flag( TRUE );
}
static	void	toggle_button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    toggle_button_event_button_on( widget );
  }
}
Q8tkWidget	*q8tk_toggle_button_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_TOGGLE_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on = toggle_button_event_button_on;
  w->event_key_on    = toggle_button_event_key_on;

  return	w;
}
Q8tkWidget	*q8tk_toggle_button_new_with_label( const char *label )
{
  Q8tkWidget	*b = q8tk_toggle_button_new();
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}
void	q8tk_toggle_button_set_state( Q8tkWidget *widget, int status )
{
  if( status ){
    if( widget->event_button_on ){
      (*widget->event_button_on)( widget );
    }
  }
}

/*--------------------------------------------------------------
 * CHECK BUTTON
 *	�E�`�F�b�N�{�^���B�{�^���������ƁA�`�F�b�N�{�b�N�X��
 *	  �h��Ԃ����B������x�{�^���������Ɩ߂�B
 *	�E�q������Ă�B
 *	  ���A�����I�ɂ́ALABEL �ȊO�������Ƃ͂Ȃ����낤�B
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	�E�V�O�i��
 *		"clicked"	�{�^���������ꂽ���ɔ���
 *		"toggled"	�{�^���̏�Ԃ��ω��������ɔ���
 *	--------------------------------------------------------
 *	- CHECKE BUTTON - xxx		(���Axxx ��LABELL )
 *
 *--------------------------------------------------------------*/
static	void	check_button_event_button_on( Q8tkWidget *widget )
{
  if( widget->stat.button.active == Q8TK_BUTTON_ON ){
    widget->stat.button.active = Q8TK_BUTTON_OFF;
    widget_signal_do( widget, "toggled" );
  }else{
    widget->stat.button.active = Q8TK_BUTTON_ON;
    widget_signal_do( widget, "clicked" );
    widget_signal_do( widget, "toggled" );
  }
  set_construct_flag( TRUE );
}
static	void	check_button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    check_button_event_button_on( widget );
  }
}
Q8tkWidget	*q8tk_check_button_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_CHECK_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on  = check_button_event_button_on;
  w->event_key_on     = check_button_event_key_on;

  return	w;
}
Q8tkWidget	*q8tk_check_button_new_with_label( const char *label )
{
  Q8tkWidget	*b = q8tk_check_button_new();
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}

/*--------------------------------------------------------------
 * RADIO BUTTON
 *	�E���W�I�{�^��
 *	�E�������̃��W�I�{�^���ŃO���[�s���O�ł���B
 *	�E�{�^���������ƃ`�F�b�N����邪�A�������O���[�s���O
 *	  ���ꂽ���̃��W�I�{�^���́A�`�F�b�N���O���B
 *	�E�q������Ă�B
 *	  ���A�����I�ɂ́ALABEL �ȊO�������Ƃ͂Ȃ����낤�B
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	�E�V�O�i��
 *		"clicked"	�{�^���������ꂽ���ɔ���
 *		"toggled"	�{�^���̏�Ԃ��ω��������ɔ���
 *	--------------------------------------------------------
 *	- RADIO BUTTON - xxx		(���Axxx ��LABELL )
 *		    :
 *		    :
 *		    :�c�c LIST -> PREV
 *			       -> NEXT
 *			  LIST �̓O���[�s���O���BPREV�ANEXT ��
 *			  �����O���[�v�̑��� RADIO BUTTON
 *
 *--------------------------------------------------------------*/
static	void	radio_button_event_button_on( Q8tkWidget *widget )
{
  Q8List	*list;

  widget_signal_do( widget, "clicked" );
  if( widget->stat.button.active == Q8TK_BUTTON_ON ) return;

  list = widget->stat.button.list;
  while( list->prev ){
    list = list->prev;
    ((Q8tkWidget *)(list->ptr))->stat.button.active = Q8TK_BUTTON_OFF;
    widget_signal_do( (Q8tkWidget *)(list->ptr), "toggled" );
  }
  list = widget->stat.button.list;
  while( list->next ){
    list = list->next;
    ((Q8tkWidget *)(list->ptr))->stat.button.active = Q8TK_BUTTON_OFF;
    widget_signal_do( (Q8tkWidget *)(list->ptr), "toggled" );
  }

  widget->stat.button.active = Q8TK_BUTTON_ON;
  widget_signal_do( widget, "toggled" );

  set_construct_flag( TRUE );
}
static	void	radio_button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    radio_button_event_button_on( widget );
  }
}
Q8tkWidget	*q8tk_radio_button_new( Q8List *list )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_RADIO_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on  = radio_button_event_button_on;
  w->event_key_on     = radio_button_event_key_on;

  if( list ) w->stat.button.active = FALSE;
  else       w->stat.button.active = TRUE;
  w->stat.button.list = q8_list_append( list, w );

  return	w;
}
Q8tkWidget	*q8tk_radio_button_new_with_label( Q8List *list,
						   const char *label )
{
  Q8tkWidget	*b = q8tk_radio_button_new( list );
  Q8tkWidget	*l  = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}
Q8List		*q8tk_radio_button_group( Q8tkWidget *radio_button )
{
  return	radio_button->stat.button.list;
}



/*--------------------------------------------------------------
 * COMBO BOX
 *	�E�R���{�{�b�N�X���ǂ��B���������͕s�\�ł���B
 *	�E�q�͎��ĂȂ�
 *	�Eq8tk_combo_popdown_strings() �����s���ꂽ���A���̃��X�g
 *	  �Ɠ�������������� LIST ITEM - LABEL �̃E�B�W�b�g��
 *	  ��������B
 *	�E�V�O�i��
 *		"changed" ���e�ɕύX�����������ɔ���
 *	--------------------------------------------------------
 *	- COMBO BOX
 *		:
 *		:�c�c LIST   ��  LIST ITEM - LABEL
 *			:
 *		      LIST   ��  LIST ITEM - LABEL
 *			:
 *		      LIST   ��  LIST ITEM - LABEL
 *			:
 *
 *	��COMBO BOX �� �}�E�X�ŃN���b�N�����ƁACOMBO BOX �́A
 *	  WINDOW (POPUP) �������������A�ȉ��̂悤�Ȑe�q�\�������B
 *	  �Ȃ��A�q������ list item - label �́A��� COMBO BOX �� LIST
 *	  �ɓo�^���ꂽ�A�E�B�W�b�g�ł���B
 *
 *	WINDOW - LIST - list item - label
 *	  :		    |
 *	  :		list item - label
 *	  :		    |
 *	  :		list item - label
 *	  :		    |
 *	  :
 *	  :
 *	  :�c�c ACCEL GROUP - ACCEL KEY  �� BUTTON(dummy)
 *
 *	���K���� list item ���N���b�N�����ƁACOMBO BOX ��
 *	  �w�����������񂪕ς��d�g�݂ɂȂ��Ă���B
 *		
 *	��ESC �L�[���������ƁAWINDOW (POPUP) ���������A����
 *	  ��Ԃɖ߂��B������������邽�߂ɁA�A�N�Z�����[�^�L�[��
 *	  ���p���Ă���B�A�N�Z�����[�^�L�[�́A�E�B�W�b�g�ɃV�O�i��
 *	  �𑗂邱�Ƃ����o���Ȃ��̂ŁA�_�~�[�̃{�^��(��\��) ��
 *	  �������AWINDOW (POPUP)�̏��������́A���̃E�B�W�b�g��
 *	  �R�[���o�b�N�֐��Ƃ��āA�o�^���Ă���B
 *
 *	��COMBO BOX �̃��X�g�������Ȃ������ɁA��ʂɎ��܂�Ȃ�
 *	  �ꍇ���łĂ����B�����ŁASCROLLED_WINDOW �� WINDOW(POPUP)��
 *	  ��������悤�ɂ��Ă݂��B
 *
 *	WINDOW - SCROLLED WINDOW - LIST - list item - label
 *	  :	  :�c�c ADJUSTMENT	    |
 *	  :	  :�c�c ADJUSTMENT	  list item - label
 *	  :				    |
 *	  :				  list item - label
 *	  :				    |
 *	  :
 *	  :
 *	  :�c�c ACCEL GROUP - ACCEL KEY  �� BUTTON(dummy)
 *
 *	  �ł��A����ADJUSTMENT ���o��̂͂����Ƃ������̂ŁA
 *	  ��ʂ���͂ݏo�������Ȏ��̂݁ASCROLLED WINDOW �ɂ������B
 *	  ���A�͂ݏo���������ǂ����̔���𐫊i�ɍs�Ȃ��ɂ́A���\�ʓ|����
 *	  �Ȃ̂ŁA�Ƃ肠���� COMBO �̕\���ʒu���x�[�X�ɁA�K���ɂ͂ݏo��
 *	  �����Ɣ��f�����Ƃ��̂݁ASCROLLED WINDOW �ɂ��Ă݂��B
 *--------------------------------------------------------------*/
static	Q8tkWidget *combo_list, *combo_window, *combo_scrolled_window;
static	Q8tkWidget *combo_fake, *combo_accel_group;
static	void	combo_event_list_callback( Q8tkWidget *list_item,
					   Q8tkWidget *parent )
{
  Q8List *l;

#ifdef	ENABLE_COMBO_SET_TEXT_ALL_WORD
  if( parent->name ){
    ex_free( parent->name );
    parent->name = NULL;
    parent->stat.combo.selected = list_item;
    widget_signal_do( parent, "changed" );
  }else
#endif

  if( parent->stat.combo.selected != list_item ){
    parent->stat.combo.selected = list_item;
    widget_signal_do( parent, "changed" );
  }

  l = parent->stat.combo.list;
  while( l ){
    q8tk_signal_handlers_destroy( (Q8tkWidget *)l->ptr );
    l = l->next;
  }

  q8tk_grab_remove( combo_window );

  if( combo_scrolled_window )
    q8tk_widget_destroy( combo_scrolled_window );

  q8tk_widget_destroy( combo_list );		/* LIST �͔j���B�������A�q�� */
  q8tk_widget_destroy( combo_window );		/* LIST ITEM �͎c���B*/
  q8tk_widget_destroy( combo_fake );
  q8tk_widget_destroy( combo_accel_group );
}
static	void	combo_fake_callback( Q8tkWidget *dummy, Q8tkWidget *parent )
{
  Q8List *l;

  l = parent->stat.combo.list;
  while( l ){
    q8tk_signal_handlers_destroy( (Q8tkWidget *)l->ptr );
    l = l->next;
  }

  q8tk_grab_remove( combo_window );

  if( combo_scrolled_window )
    q8tk_widget_destroy( combo_scrolled_window );

  q8tk_widget_destroy( combo_list );		/* LIST �͔j���B�������A�q�� */
  q8tk_widget_destroy( combo_window );		/* LIST ITEM �͎c���B*/
  q8tk_widget_destroy( combo_fake );
  q8tk_widget_destroy( combo_accel_group );
}

static	void	combo_event_button_on( Q8tkWidget *widget )
{
  int	i;
  Q8List     *l;
  combo_list   = q8tk_list_new();
  combo_window = q8tk_window_new( Q8TK_WINDOW_POPUP );
  combo_scrolled_window = NULL;

  i = 0;
  l = widget->stat.combo.list;
  while( l ){
    q8tk_container_add( combo_list, (Q8tkWidget *)(l->ptr) );
    l = l->next;
    i ++;
  }

  q8tk_list_select_child( combo_list, widget->stat.combo.selected );

  l = widget->stat.combo.list;
  while( l ){
    q8tk_signal_connect( (Q8tkWidget *)(l->ptr), "select",
			 combo_event_list_callback, widget );
    l = l->next;
  }

  q8tk_widget_show( combo_list );

  if( widget->y + i + 2 > 24 ){		/* ��ʂ���͂ݏo�����Ȃ玞 */
					/* SCROLLED WINDOW �𐶐�   */
    int height = 24 - 2 - widget->y;
    if( height < 3 ) height = 3;

    combo_scrolled_window = q8tk_scrolled_window_new( NULL, NULL );
    q8tk_container_add( combo_scrolled_window, combo_list );

    q8tk_scrolled_window_set_policy( combo_scrolled_window,
				   Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS );

    if( widget->stat.combo.width ){
      q8tk_misc_set_size( combo_scrolled_window, 
			  widget->stat.combo.width +3, height );
    }else{
      q8tk_misc_set_size( combo_scrolled_window, 
			  widget->stat.combo.length +3, height );
    }

    q8tk_widget_show( combo_scrolled_window );
    q8tk_container_add( combo_window, combo_scrolled_window );

  }else{				/* �ʏ�͂������� */

    q8tk_container_add( combo_window, combo_list );

  }

  q8tk_widget_show( combo_window );
  q8tk_grab_add( combo_window );
  q8tk_widget_grab_default( combo_list );

  if( widget->stat.combo.width ){
    q8tk_misc_set_size( combo_list, widget->stat.combo.width, 0 );
  }

  combo_window->stat.window.set_position = TRUE;
  combo_window->stat.window.x = widget->x -1;
  combo_window->stat.window.y = widget->y +1;


	/* ESC �L�[�������������X�g���������邽�߂́A�_�~�[�𐶐� */

  combo_fake = q8tk_button_new();
  q8tk_signal_connect( combo_fake, "clicked", combo_fake_callback, widget );

  combo_accel_group = q8k_accel_group_new();
  q8tk_accel_group_attach( combo_accel_group, combo_window );
  q8tk_accel_group_add( combo_accel_group, Q8TK_KEY_ESC,
		        combo_fake, "clicked" );
}
static	void	combo_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    combo_event_button_on( widget );
  }
}
Q8tkWidget	*q8tk_combo_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_COMBO;

  w->event_button_on  = combo_event_button_on;
  w->event_key_on     = combo_event_key_on;

  return	w;
}
void		q8tk_combo_popdown_strings( Q8tkWidget *combo, Q8List *list )
{
  Q8List *new_list = NULL;
  Q8tkAssert(list,NULL);

  if( combo->stat.combo.list ){		/* �����̃��X�g��j��(�J��) */
    Q8List *l = combo->stat.combo.list;			/* TODO : ���`�F�b�N */
    while( l ){
      widget_destroy_child( (Q8tkWidget *)l->ptr );
      l = l->next;
    }
    q8_list_free( combo->stat.combo.list );
  }

  list = q8_list_first( list );
  combo->stat.combo.length = 0;		/* �Œ������񐔂��L�����Ă��� */

  while( list ){			/* �Slist�� LIST_ITEM ������ */
    Q8tkWidget *list_item;
    int len = strlen( (const char *)list->ptr );
    combo->stat.combo.length = Q8TKMAX( combo->stat.combo.length, len );

    list_item = q8tk_list_item_new_with_label( (const char *)(list->ptr) );
    q8tk_widget_show( list_item );

    new_list = q8_list_append( new_list, list_item );
    list = list->next;
  }

  new_list = q8_list_first( new_list );
  combo->stat.combo.list     = new_list;
  combo->stat.combo.selected = (Q8tkWidget *)(new_list->ptr);

  set_construct_flag( TRUE );
}
const	char	*q8tk_combo_get_text( Q8tkWidget *combo )
{
  Q8List *list = combo->stat.combo.list;

#ifdef	ENABLE_COMBO_SET_TEXT_ALL_WORD
  if( combo->name ) return combo->name;
#endif

  if( list ){
    while( list ){
      if( list->ptr == combo->stat.combo.selected ){
	Q8tkAssert(((Q8tkWidget*)(list->ptr))->type==Q8TK_TYPE_LIST_ITEM,NULL);
	if( ((Q8tkWidget*)(list->ptr))->child ){
	  return (const char *) ((Q8tkWidget*)(list->ptr))->child->name;
	}else{
	  return NULL;
	}
      }
      list = list->next;
    }
    return NULL;
  }else{
    return	NULL;
  }
}
void		q8tk_combo_set_text( Q8tkWidget *combo, const char *text )
{
  Q8List *list = combo->stat.combo.list;

#ifdef	ENABLE_COMBO_SET_TEXT_ALL_WORD
  if( combo->name ){
    ex_free( combo->name );
    combo->name = NULL;
  }
#endif

  while( list ){
    Q8tkAssert((((Q8tkWidget*)(list->ptr))->type==Q8TK_TYPE_LIST_ITEM),NULL);
    if( ((Q8tkWidget*)(list->ptr))->child ){
      if( strcmp( text, ((Q8tkWidget*)(list->ptr))->child->name )==0 ){
	if( combo->stat.combo.selected != list->ptr ){
	  combo->stat.combo.selected = (Q8tkWidget *)(list->ptr);
	  widget_signal_do( combo, "changed" );
	  set_construct_flag( TRUE );
	}
	return;
      }
    }
    list = list->next;
  }

#ifdef	ENABLE_COMBO_SET_TEXT_ALL_WORD
  combo->name  = (char *)ex_malloc( strlen(text)+1 );
  Q8tkAssert(combo->name,"memory exhoused");
  CHECK_MALLOC_OK( combo->name, "memory" );
  strcpy( combo->name, text );
  combo->stat.combo.length = strlen(text);
  set_construct_flag( TRUE );
#endif
}
#if 0
void	q8tk_combo_set_width( Q8tkWidget *combo, int width )
{
  if( width > 0 ) combo->stat.combo.width = width;
  else            combo->stat.combo.width = 0;
  set_construct_flag( TRUE );
}
#endif


/*--------------------------------------------------------------
 * LIST
 *	�E���X�g�{�b�N�X�B�I����@�́ASELECTION_BROWSE �̂�
 *	�E�����̎q�����Ă�B
 *	  �������ALIST ITEM �ȊO�͎q�ɂ��Ȃ�����
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	  �q�́A���Ԃ� LIST �ɂȂ��ł����B
 *	�E�V�O�i��
 *		"selection_change"	�I��Ώۂ� LIST ITEM ���ς������
 *	--------------------------------------------------------
 *	- LIST - LIST ITEM - LABEL
 *		     |
 *		 LIST ITEM - LABEL
 *		     |
 *		 LIST ITEM - LABEL
 *		     |
 *
 *--------------------------------------------------------------*/
static	void	list_event_key_on( Q8tkWidget *widget, int key )
{
  Q8tkWidget *w = widget->stat.list.active;

  if( w ){
    if( key <= 0xff && isgraph(key) ){ /* �X�y�[�X�ȊO�̕��������͂��ꂽ�ꍇ */

      while( (w = w->next) ){
	if( w->child ){
	  Q8tkAssert(w->child->type==Q8TK_TYPE_LABEL,NULL);
	  if( w->child->name[0] == key ){

	    widget->stat.list.active = w;

	    if( widget->parent &&
	        widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW ){
	      int i;
	      Q8tkWidget *c = widget->child;
	      for( i=0; c ;i++ ){
		if( c==widget->stat.list.active ) break;
		c = c->next;
	      }
	      if( i >=   widget->parent->stat.scrolled.child_y0
		       + widget->parent->stat.scrolled.child_sy ){
		Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value
				= i -widget->parent->stat.scrolled.child_sy +1;
	      }

	      (*w->event_button_on)( w );	/* �c�c ���������������Ȃ� */
	    }
	    set_construct_flag( TRUE );
	    break;
	  }
	}
      }

    }else{			/* �X�y�[�X��A���䕶�������͂��ꂽ�ꍇ */
      switch( key ){

      case Q8TK_KEY_RET:
      case Q8TK_KEY_SPACE:
	(*w->event_button_on)( w );
	break;

      case Q8TK_KEY_UP:
	if( w->prev ){
	  widget->stat.list.active = w->prev;

	  if( widget->parent &&
	      widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW ){
	    int i;
	    Q8tkWidget *c = widget->child;
	    for( i=0; c ;i++ ){
	      if( c==widget->stat.list.active ) break;
	      c = c->next;
	    }
	    if( i < widget->parent->stat.scrolled.child_y0 ){
	      Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value = i;
	    }
	  }

	  set_construct_flag( TRUE );
	}
	break;
      case Q8TK_KEY_DOWN:
	if( w->next ){
	  widget->stat.list.active = w->next;

	  if( widget->parent &&
	      widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW ){
	    int i;
	    Q8tkWidget *c = widget->child;
	    for( i=0; c ;i++ ){
	      if( c==widget->stat.list.active ) break;
	      c = c->next;
	    }
	    if( i >=   widget->parent->stat.scrolled.child_y0
		     + widget->parent->stat.scrolled.child_sy ){
	      Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value
				= i -widget->parent->stat.scrolled.child_sy +1;
	    }
	  }

	  set_construct_flag( TRUE );
	}
	break;

      case Q8TK_KEY_PAGE_UP:
	if( widget->parent &&
	    widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW &&
	    widget->parent->stat.scrolled.vscrollbar ){
	  int i;
	  Q8tkWidget *c = widget->child;
	  int h = widget->parent->stat.scrolled.height -1;

	  if( widget->stat.scrolled.hscrollbar ) h -= 3;
	  else                                   h -= 2;
	  for( ; h>0; h-- ){
	    if( w->prev==NULL ) break;
	    w = w->prev;
	  }

	  widget->stat.list.active = w;

	  for( i=0; c ;i++ ){
	    if( c==widget->stat.list.active ) break;
	    c = c->next;
	  }
	  if( i < widget->parent->stat.scrolled.child_y0 ){
	    Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value = i;
	  }
	  set_construct_flag( TRUE );
	}
	break;
      case Q8TK_KEY_PAGE_DOWN:
	if( widget->parent &&
	    widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW &&
	    widget->parent->stat.scrolled.vscrollbar ){
	  int i;
	  int h = widget->parent->stat.scrolled.height -1;
	  Q8tkWidget *c = widget->child;

	  if( widget->stat.scrolled.hscrollbar ) h -= 3;
	  else                                   h -= 2;
	  for( ; h>0; h-- ){
	    if( w->next==NULL ) break;
	    w = w->next;
	  }

	  widget->stat.list.active = w;

	  for( i=0; c ;i++ ){
	    if( c==widget->stat.list.active ) break;
	    c = c->next;
	  }
	  if( i >=   widget->parent->stat.scrolled.child_y0
		   + widget->parent->stat.scrolled.child_sy ){
	    Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value
				= i -widget->parent->stat.scrolled.child_sy +1;
	  }
	  set_construct_flag( TRUE );
	}
	break;
      }
    }
  }
}
Q8tkWidget	*q8tk_list_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_LIST;
  w->attr  = Q8TK_ATTR_CONTAINER;

  w->event_key_on = list_event_key_on;

  return	w;
}
void	q8tk_list_append_items( Q8tkWidget *wlist, Q8List *list )
{
  Q8tkAssert(list,NULL);

  list = q8_list_first( list );

  while( list ){
    q8tk_container_add( wlist, (Q8tkWidget*)list->ptr );
    list = list->next;
  }

  set_construct_flag( TRUE );
}
void	q8tk_list_clear_items( Q8tkWidget *wlist, int start, int end )
{
  int	i, rep;
  Q8tkWidget *wk, *c = wlist->child;

  if( c==NULL ) return;
  if( start<0 ) return;

  if( end < start ) rep = MAX_WIDGET;
  else              rep = end-start;

  for( i=0; i<start; i++ ){
    if( (c = c->next) == NULL ) return;
  }
  for( i=0; i<=rep; i++ ){
    q8tk_container_remove( wlist, c );
    wk = c->next;
    widget_destroy_child( c );
    if( (c = wk) == NULL ) return;
  }

  set_construct_flag( TRUE );
}
void	q8tk_list_select_item( Q8tkWidget *wlist, int item )
{
  int	i;
  Q8tkWidget *c = wlist->child;

  if( c==NULL ) return;
  for( i=0; i<item; i++ ){
    if( (c = c->next) == NULL ) return;
  }
  if( wlist->stat.list.selected != c ){
    wlist->stat.list.selected = c;
    wlist->stat.list.active   = c;
    widget_signal_do( wlist, "selection_changed" );
    set_construct_flag( TRUE );
  }
  widget_signal_do( c, "select" );
  set_scroll_adj_widget( c );
}
void	q8tk_list_select_child( Q8tkWidget *wlist, Q8tkWidget *child )
{
  if( wlist->stat.list.selected != child ){
    wlist->stat.list.selected = child;
    wlist->stat.list.active   = child;
    widget_signal_do( wlist, "selection_changed" );
    set_construct_flag( TRUE );
  }
  widget_signal_do( child, "select" );
  set_scroll_adj_widget( child );
}
#if 0
void	q8tk_list_set_width( Q8tkWidget *wlist, int width )
{
  if( width > 0 )  wlist->stat.list.width  = width;
  else             wlist->stat.list.width  = 0;
  set_construct_flag( TRUE );
}
#endif

/*
 * ����͂�����Ɠ��ꏈ���B
 *	LIST �� SCROLLED WINDOW �̎q�̏ꍇ�ŁASCROLLED WINDOW �̃X�N���[��
 *	�o�[(�c����) ���������ꂽ�Ƃ��A���̊֐����Ă΂��B
 *	�����ł́ASCROLLED WINDOW �̕\���͈͂ɉ����āALIST �� active
 *	�E�B�W�b�g��ύX���Ă���B
 */
static	void	list_event_window_scrolled( Q8tkWidget *swin, int sy )
{
  Q8tkWidget	*widget = swin->child;	/* == LIST */

  Q8tkWidget	*c = widget->child;
  int		nth = 0;

  if( c==NULL ) return;

  while( c ){
    if( c==widget->stat.list.active ) break;
    nth ++;
    c = c->next;
  }
  nth = nth - swin->stat.scrolled.vadj->stat.adj.value;

  if( 0 <= nth && nth < sy ){
    /* Ok, No Adjust */
  }else{

    if( nth < 0 ) nth = swin->stat.scrolled.vadj->stat.adj.value;
    else          nth = swin->stat.scrolled.vadj->stat.adj.value + sy -1;

    c = widget->child;
    while( nth-- ){
      if( (c = c->next) == NULL ) return;
    }
    widget->stat.list.active   = c;
    set_construct_flag( TRUE );

  }
}




/*--------------------------------------------------------------
 * LIST ITEM
 *	�E���X�g�A�C�e���B
 *	�ELIST �̎q�ɂȂ��
 *	�E�q�����Ă邪�ALABEL �Ɍ���
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	�E�V�O�i��
 *		"select"	�N���b�N������ (���I����Ԃł�����)
 *	--------------------------------------------------------
 *	- LIST ITEM - LABEL
 *
 *--------------------------------------------------------------*/
static	void	list_item_event_button_on( Q8tkWidget *widget )
{
  q8tk_widget_grab_default( widget->parent );

  if( widget->parent->stat.list.selected != widget ){
    widget->parent->stat.list.selected = widget;
    widget->parent->stat.list.active   = widget;
    widget_redraw_now();	/* ��U�ĕ`�� */
    widget_signal_do( widget->parent, "selection_changed" );
  }
  widget_signal_do( widget, "select" );
  set_construct_flag( TRUE );
}
Q8tkWidget	*q8tk_list_item_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_LIST_ITEM;
  w->attr   = Q8TK_ATTR_CONTAINER | Q8TK_ATTR_LABEL_CONTAINER;

  w->event_button_on  = list_item_event_button_on;

  return	w;
}
Q8tkWidget	*q8tk_list_item_new_with_label( const char *label )
{
  Q8tkWidget	*i = q8tk_list_item_new();
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( i, l );

  i->with_label = TRUE;

  return	i;
}



/*--------------------------------------------------------------
 * LABEL
 *	�E���x��
 *	�E(�\���p��)�������ێ��ł���
 *	�E�V�O�i�� �c ����
 *	--------------------------------------------------------
 *	- LABEL
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_label_new( const char *label )
{
  Q8tkWidget	*w;

  if( label==NULL ) label="";

  w = malloc_widget();
  w->type  = Q8TK_TYPE_LABEL;
  w->name  = (char *)ex_malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );

  w->stat.label.foreground = Q8GR_PALETTE_FOREGROUND;
  w->stat.label.background = Q8GR_PALETTE_BACKGROUND;

  return	w;
}
void		q8tk_label_set( Q8tkWidget *w, const char *label )
{
  if( label==NULL ) label="";

  if( w->name ) ex_free( w->name );
  w->name  = (char *)ex_malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );
  set_construct_flag( TRUE );
}


/*--------------------------------------------------------------
 * FRAME
 *	�E�t���[��
 *	�E�q���ЂƂ��Ă�B
 *	�E�q�����ɂ́Aq8tk_container_add() ���g�p����B
 *	�E(���o����)�������ێ��ł���B
 *	�E�V�O�i�� �c ����
 *	--------------------------------------------------------
 *	- FRAME - xxx
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_frame_new( const char *label )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_FRAME;
  w->attr  = Q8TK_ATTR_CONTAINER;

  w->stat.frame.shadow_type = Q8TK_SHADOW_OUT;

  w->name  = (char *)ex_malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );

  return	w;
}
void	q8tk_frame_set_shadow_type( Q8tkWidget *frame, int shadow_type )
{
  frame->stat.frame.shadow_type = shadow_type;
}





/*--------------------------------------------------------------
 * HBOX
 *	�E�����{�b�N�X
 *	�E�����̎q�����Ă�B
 *	�E�q�����ɂ́Aq8tk_box_pack_XXX() ���g�p����B
 *	�E�V�O�i�� �c ����
 *	--------------------------------------------------------
 *	- HBOX - xxx
 *		  |
 *		 xxx
 *		  |
 *		 xxx
 *		  |
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_hbox_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_HBOX;
  w->attr   = Q8TK_ATTR_CONTAINER;

  return	w;
}
/*--------------------------------------------------------------
 * VBOX
 *	�E�����{�b�N�X
 *	�E�����̎q�����Ă�B
 *	�E�q�����ɂ́Aq8tk_box_pack_XXX() ���g�p����B
 *	�E�V�O�i�� �c ����
 *	--------------------------------------------------------
 *	- HBOX - xxx
 *		  |
 *		 xxx
 *		  |
 *		 xxx
 *		  |
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_vbox_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_VBOX;
  w->attr   = Q8TK_ATTR_CONTAINER;

  return	w;
}


/*--------------------------------------------------------------
 * NOTE BOOK ( NOTE PAGE )
 *	�E�m�[�g�u�b�N
 *	�E�����̃y�[�W�����Ă�B�e�y�[�W�̓R���e�i�ł���A�q������Ă�B
 *	�Eq8tk_notebook_append() �ŁA�q�������A���̓x�ɓ�����
 *	  NOTE PAGE �𐶐����A���ꂪ�q�������ƂɂȂ�B
 *	�ENOTE PAGE �́A(���o����)�������ێ��ł���B
 *	�E�V�O�i��
 *		"switch_page"	�ʂ̃y�[�W�ɐ؂�ւ�������ɔ���
 *	--------------------------------------------------------
 *	- NOTE BOOX - NOTE PAGE - xxx
 *			|
 *		      NOTE PAGE - xxx
 *			|
 *		      NOTE PAGE - xxx
 *			|
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_notebook_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_NOTEBOOK;
  w->stat.notebook.page = NULL;

  return	w;
}
static	void	notepage_event_button_on( Q8tkWidget *widget )
{
  if( (widget->parent)->stat.notebook.page != widget ){
    (widget->parent)->stat.notebook.page = widget;
    widget_signal_do( widget->parent, "switch_page" );
    set_construct_flag( TRUE );
  }
}
static	void	notepage_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    notepage_event_button_on( widget );
  }
}
void		q8tk_notebook_append( Q8tkWidget *notebook,
				      Q8tkWidget *widget, const char *label )
{
  Q8tkWidget	*w, *c;

  Q8tkAssert(notebook->type==Q8TK_TYPE_NOTEBOOK,NULL);

  w = malloc_widget();
  w->type   = Q8TK_TYPE_NOTEPAGE;
  w->attr   = Q8TK_ATTR_CONTAINER;
  w->parent = notebook;
  w->name  = (char *)ex_malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );

  if( notebook->child ){
    c = notebook->child;
    while( c->next ){
      c = c->next;
    }
    c->next = w;
    w->prev = c;
    w->next = NULL;
  }else{
    w->prev = NULL;
    w->next = NULL;
    notebook->child              = w;
    notebook->stat.notebook.page = w;
  }

  q8tk_container_add( w, widget );
  q8tk_widget_show(w);

  w->event_button_on  = notepage_event_button_on;
  w->event_key_on     = notepage_event_key_on;
}
int	q8tk_notebook_current_page( Q8tkWidget *notebook )
{
  Q8tkWidget *child = notebook->child;
  int	i = 0;

  while( child ){
    if( child == notebook->stat.notebook.page ){
      return i;
    }
    child = child->next;
    i++;
  }
  return -1;
}
void	q8tk_notebook_set_page( Q8tkWidget *notebook, int page_num )
{
  Q8tkWidget *child = notebook->child;
  int	i = 0;

  while( child ){
    if( i==page_num ){
      if( notebook->stat.notebook.page != child ){
	notebook->stat.notebook.page = child;
	widget_signal_do( notebook, "switch_page" );
	set_construct_flag( TRUE );
      }
      break;
    }else{
      child = child->next;
      i++;
    }
  }
}
void	q8tk_notebook_next_page( Q8tkWidget *notebook )
{
  if( notebook->child ){
    Q8tkWidget *page = notebook->stat.notebook.page;
    if( page && page->next ){
      notebook->stat.notebook.page = page->next;
      set_construct_flag( TRUE );
    }
  }
}
void	q8tk_notebook_prev_page( Q8tkWidget *notebook )
{
  if( notebook->child ){
    Q8tkWidget *page = notebook->stat.notebook.page;
    if( page && page->prev ){
      notebook->stat.notebook.page = page->prev;
      set_construct_flag( TRUE );
    }
  }
}


/*--------------------------------------------------------------
 * VSEPARATOR
 *	�E�����Z�p���[�^
 *	�E�q�͎��ĂȂ�
 *	�E�������A�e�E�B�W�b�g�̑傫���ɂ��A���I�ɕς��B
 *	�E�V�O�i�� �c �Ȃ�
 *	--------------------------------------------------------
 *	- VSEPARATOR
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_vseparator_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_VSEPARATOR;

  return	w;
}
/*--------------------------------------------------------------
 * HSEPARATOR
 *	�E�����Z�p���[�^
 *	�E�q�͎��ĂȂ�
 *	�E�������A�e�E�B�W�b�g�̑傫���ɂ��A���I�ɕς��B
 *	�E�V�O�i�� �c �Ȃ�
 *	--------------------------------------------------------
 *	- VSEPARATOR
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_hseparator_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_HSEPARATOR;

  return	w;
}



/*--------------------------------------------------------------
 * ADJUSTMENT
 *	�E�A�W���X�g�����g
 *	�E�����W(�l�͈̔�)�Ƒ���(�����������A�傫��������2���)
 *	  �������A������������Ɍ���B
 *	�E�X�P�[���E�B�W�b�g��X�N���[���h�E�C���h�E�𐶐�����
 *	  �ۂɁA�K�v�ƂȂ�B�P�Ƃł͎g�p���邱�Ƃ͂Ȃ��B
 *	�E�X�P�[���𐶐�����ۂɁA�O�����ăA�W���X�g�����g��
 *	  �������Ă����A���̃A�W���X�g�����g�������ăX�P�[����
 *	  ��������̂���ʓI�B�t�ɁA�X�N���[���h�E�C���h�E��
 *	  ���̐������ɃA�W���X�g�����g��������������̂���ʓI
 *	  �Ȃ��߁A�O�����ăA�W���X�g�����g�𐶐����邱�Ƃ͖����B
 *	�E�q�͎��ĂȂ��B�q�ɂȂ邱�Ƃ��ł��Ȃ��B
 *	�E�V�O�i��
 *		"value_changed"		�l���ς�������ɔ���
 *	--------------------------------------------------------
 *	ADJUSTMENT
 *
 *--------------------------------------------------------------*/
enum{ ADJ_STEP_DEC, ADJ_STEP_INC, ADJ_PAGE_DEC, ADJ_PAGE_INC, ADJ_SLIDER_DRAG};
static	void	adjustment_do( Q8tkWidget *widget, int mode )
{
  Q8Adjust	*adj = &widget->stat.adj;
  int	value_old = adj->value;

  switch( mode ){
  case ADJ_STEP_DEC:
    adj->value -= adj->step_increment;
    if( adj->value < adj->lower ) adj->value = adj->lower;
    break;
  case ADJ_STEP_INC:
    adj->value += adj->step_increment;
    if( adj->value > adj->upper ) adj->value = adj->upper;
    break;
  case ADJ_PAGE_DEC:
    adj->value -= adj->page_increment;
    if( adj->value < adj->lower ) adj->value = adj->lower;
    break;
  case ADJ_PAGE_INC:
    adj->value += adj->page_increment;
    if( adj->value > adj->upper ) adj->value = adj->upper;
    break;
  case ADJ_SLIDER_DRAG:
    set_drag_widget( widget );
    return;
  }

  if( value_old != adj->value ){
    widget_signal_do( widget, "value_changed" );
  }
  set_construct_flag( TRUE );
}
static	void	adjustment_event_button_on( Q8tkWidget *widget )
{
  Q8Adjust	*adj = &widget->stat.adj;
  int	slider_x, slider_y;
  int	arrow_lower_x, arrow_lower_y;
  int	arrow_upper_x, arrow_upper_y;
  int	mouse_x = mouse.x / 8;
  int	mouse_y = mouse.y / 16;

  if( adj->horizontal ){			/* HORIZONTAL ADJUSTMENT */

    slider_x = adj->x + adj->pos;
    slider_y = adj->y;

    if( mouse_y == slider_y ){

      if( adj->arrow ){
	arrow_lower_x = adj->x;		slider_x ++;
	arrow_upper_x = adj->x + adj->length +1;

	if      ( mouse_x == arrow_lower_x ){
	  adjustment_do( widget, ADJ_STEP_DEC );	return;
	}else if( mouse_x == arrow_upper_x ){
	  adjustment_do( widget, ADJ_STEP_INC );	return;
	}
      }

      if      ( mouse_x == slider_x ){
	adjustment_do( widget, ADJ_SLIDER_DRAG );	return;
      }else if( mouse_x < slider_x ){
	adjustment_do( widget, ADJ_PAGE_DEC );		return;
      }else{
	adjustment_do( widget, ADJ_PAGE_INC );		return;
      }

    }else{
      return;
    }

  }else{					/* VIRTICAL ADJUSTMENT */

    slider_x = adj->x;
    slider_y = adj->y + adj->pos;

    if( mouse_x == slider_x ){

      if( adj->arrow ){
	arrow_lower_y = adj->y;		slider_y ++;
	arrow_upper_y = adj->y + adj->length +1;
	if      ( mouse_y == arrow_lower_y ){
	  adjustment_do( widget, ADJ_STEP_DEC );	return;
	}else if( mouse_y == arrow_upper_y ){
	  adjustment_do( widget, ADJ_STEP_INC );	return;
	}
      }

      if     ( mouse_y == slider_y ){
	adjustment_do( widget, ADJ_SLIDER_DRAG );	return;
      }else if( mouse_y < slider_y ){
	adjustment_do( widget, ADJ_PAGE_DEC );		return;
      }else{
	adjustment_do( widget, ADJ_PAGE_INC );		return;
      }

    }else{
      return;
    }

  }
}
static	void	adjustment_event_dragging( Q8tkWidget *widget )
{
  Q8Adjust	*adj = &widget->stat.adj;
  int	mouse_x = mouse.x / 8;
  int	mouse_y = mouse.y / 16;
  int	adj_x = adj->x;
  int	adj_y = adj->y;
  int	slider = -1;

  if( adj->upper <= adj->lower ) return;

  if( adj->horizontal ){
    if( adj->arrow ) adj_x ++;
    if( adj_x <= mouse_x && mouse_x < adj_x+adj->length ){
      slider = mouse_x - adj_x;
    }
  }else{
    if( adj->arrow ) adj_y ++;
    if( adj_y <= mouse_y && mouse_y < adj_y+adj->length ){
      slider = mouse_y - adj_y;
    }
  }

  if( slider >= 0 ){
    float val0 = ( slider -1 ) * adj->scale   + adj->lower;
    float val1 = ( slider    ) * adj->scale   + adj->lower;
    int   val;

    if     ( slider <= 0 )             val = adj->lower;
    else if( slider >= adj->length-1 ) val = adj->upper;
    else{
      float base = (float)(adj->upper-adj->lower) / (adj->length-1);
      int   val2 = (int)( val1 / base ) * base;
      if( val0<val2 && val2<=val1 ) val = val2;
      else                          val = ( val0 + val1 ) / 2;
    }

    if( adj->value != val ){
      adj->value = val;
      widget_signal_do( widget, "value_changed" );
    }
    set_construct_flag( TRUE );
  }
}
static	void	adjustment_event_key_on( Q8tkWidget *widget, int key )
{
  Q8Adjust	*adj = &widget->stat.adj;

  if( adj->horizontal ){			/* HORIZONTAL ADJUSTMENT */
    if     ( key==Q8TK_KEY_LEFT  ) adjustment_do( widget, ADJ_STEP_DEC );
    else if( key==Q8TK_KEY_RIGHT ) adjustment_do( widget, ADJ_STEP_INC );
  }else{					/* VIRTICAL ADJUSTMENT */
    if     ( key==Q8TK_KEY_UP  )  adjustment_do( widget, ADJ_STEP_DEC );
    else if( key==Q8TK_KEY_DOWN ) adjustment_do( widget, ADJ_STEP_INC );
    else if( key==Q8TK_KEY_PAGE_UP )   adjustment_do( widget, ADJ_PAGE_DEC );
    else if( key==Q8TK_KEY_PAGE_DOWN ) adjustment_do( widget, ADJ_PAGE_INC );
  }
}
Q8tkWidget	*q8tk_adjustment_new( int value, int lower, int upper,
				      int step_increment, int page_increment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_ADJUSTMENT;

  w->event_button_on = adjustment_event_button_on;
  w->event_dragging  = adjustment_event_dragging;
  w->event_key_on    = adjustment_event_key_on;

  w->stat.adj.value = value;
  w->stat.adj.lower = lower;
  w->stat.adj.upper = upper;
  w->stat.adj.step_increment = step_increment;
  w->stat.adj.page_increment = page_increment;

  return	w;
}
void	q8tk_adjustment_set_value( Q8tkWidget *adj, int value )
{
  if( value < adj->stat.adj.lower ) value = adj->stat.adj.lower;
  if( value > adj->stat.adj.value ) value = adj->stat.adj.upper;

  if( adj->stat.adj.value != value ){
    adj->stat.adj.value = value;

    /* �V�O�i���͔��������Ȃ� */
    /*
    widget_signal_do( adj, "value_changed" );
    */

    set_construct_flag( TRUE );
  }
}
void	q8tk_adjustment_clamp_page( Q8tkWidget *adj, int lower, int upper )
{
  if( upper < lower ) upper = lower;

  adj->stat.adj.lower = lower;
  adj->stat.adj.upper = upper;
  if( adj->stat.adj.value < adj->stat.adj.lower )
				adj->stat.adj.value = adj->stat.adj.lower;
  if( adj->stat.adj.value > adj->stat.adj.upper )
				adj->stat.adj.value = adj->stat.adj.upper;

  /* �V�O�i���͔��������Ȃ� */

  set_construct_flag( TRUE );
}

void		q8tk_adjustment_set_arrow( Q8tkWidget *adj, int arrow )
{
  if( adj->stat.adj.arrow != arrow ){
    adj->stat.adj.arrow = arrow;
    set_construct_flag( TRUE );
  }
}
void		q8tk_adjustment_set_length( Q8tkWidget *adj, int length )
{
  if( adj->stat.adj.max_length != length ){
    adj->stat.adj.max_length = length;
    set_construct_flag( TRUE );
  }
}



/*--------------------------------------------------------------
 * HSCALE
 *	�E�����X�P�[��
 *	�Enew()���ɁA�����ŃA�W���X�g�����g���w�肷��B�X�P�[����
 *	  �����W(�͈�)�⑝���́A���̃A�W���X�g�����g�Ɉˑ�����B
 *	�Enew()���̈����� NULL �̏ꍇ�́A�����I�ɃA�W���X�g�����g
 *	  ����������邪�A���̎��̃����W�� 0�`10 �A������ 1 �� 2
 *	  �ɌŒ�ł���B(�ύX�\)
 *	�E�q�͎��ĂȂ�
 *	�E�V�O�i�� �c �Ȃ��B
 *		      �������A�A�W���X�g�����g�̓V�O�i�����󂯂�B
 *	--------------------------------------------------------
 *	- HSCALE
 *	     :�c�c ADJUSTMENT
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_hscale_new( Q8tkWidget *adjustment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_HSCALE;

  if( adjustment ){
    w->stat.scale.adj = adjustment;
  }else{
    w->stat.scale.adj = q8tk_adjustment_new( 0, 0, 10, 1, 2 );
  }

  w->stat.scale.adj->stat.adj.horizontal = TRUE;
  w->stat.scale.adj->parent = w;

  return	w;
}
/*--------------------------------------------------------------
 * HSCALE
 *	�E�����X�P�[��
 *	�Enew()���ɁA�����ŃA�W���X�g�����g���w�肷��B�X�P�[����
 *	  �����W(�͈�)�⑝���́A���̃A�W���X�g�����g�Ɉˑ�����B
 *	�Enew()���̈����� NULL �̏ꍇ�́A�����I�ɃA�W���X�g�����g
 *	  ����������邪�A���̎��̃����W�� 0�`10 �A������ 1 �� 2
 *	  �ɌŒ�ł���B(�ύX�\)
 *	�E�q�͎��ĂȂ�
 *	�E�V�O�i�� �c �Ȃ��B
 *		      �������A�A�W���X�g�����g�̓V�O�i�����󂯂�B
 *	--------------------------------------------------------
 *	- VSCALE
 *	     :�c�c ADJUSTMENT
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_vscale_new( Q8tkWidget *adjustment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_VSCALE;

  if( adjustment ){
    w->stat.scale.adj = adjustment;
  }else{
    w->stat.scale.adj = q8tk_adjustment_new( 0, 0, 10, 1, 2 );
  }

  w->stat.scale.adj->stat.adj.horizontal = FALSE;
  w->stat.scale.adj->parent = w;

  return	w;
}

void		q8tk_scale_set_value_pos( Q8tkWidget *scale, int pos )
{
  scale->stat.scale.value_pos = pos;
  set_construct_flag( TRUE );
}
void		q8tk_scale_set_draw_value( Q8tkWidget *scale, int draw_value )
{
  scale->stat.scale.draw_value = draw_value;
  set_construct_flag( TRUE );
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * �\�����̃T�C�Y�v�Z (widget_size()������Ă΂��)
 */
static	void	adjustment_size( Q8Adjust *adj, int *sx, int *sy )
{
  int i, limit;
  int range = ( adj->upper - adj->lower + 1 );

  if( adj->horizontal ) limit = Q8GR_SCREEN_X * 0.8;
  else                  limit = Q8GR_SCREEN_Y * 0.8;

  if( adj->max_length <= 2 ){
    for( i=1; ; i++ ) if( range / i < limit ) break;
    adj->length = range / i;
  }else{
    adj->length = adj->max_length;
  }
  adj->scale = (float)( range - 1 ) / (adj->length - 2 );
  if     ( adj->value == adj->lower ) adj->pos = 0;
  else if( adj->value == adj->upper ) adj->pos = adj->length-1;
  else{
    adj->pos = (adj->value-adj->lower) / adj->scale + 1;
    if( adj->pos >= adj->length-1 ) adj->pos = adj->length-2;
  }

  if( adj->horizontal ){
    *sx = adj->length + ((adj->arrow) ? +2: 0 );
    *sy = 1;
  }else{
    *sx = 1;
    *sy = adj->length + ((adj->arrow) ? +2: 0 );
  }
}






/*--------------------------------------------------------------
 * SCROLLED WINDOW
 *	�E�X�N���[���h�E�C���h�E
 *	�Enew()���ɁA�����ŃA�W���X�g�����g���w�肷�邪�A�X�P�[����
 *	  �����W(�͈�)�͂��̃X�N���[���h�E�C���h�E�̎q�̑傫����
 *	  ����āA���I�ɕω�����B(�����͈����p�����)
 *	�Enew()���̈����� NULL �̏ꍇ�́A�����I�ɃA�W���X�g�����g
 *	  �����������B���̎��̑����� 1 �� 10 �ł���B���ɗ��R��
 *	  �Ȃ���΁ANULL �ɂ�鎩�������̕����ȒP�ŕ֗��B
 *	�E�q������Ă�B�������A�q�⑷�� SCROLLED WINDOW ��
 *	  ���悤�ȏꍇ�̓���͖����؂ł���B
 *	�E�V�O�i�� �c �Ȃ�
 *	--------------------------------------------------------
 *	- SCROLLED WINDOW     - xxx
 *	     :�c�c ADJUSTMENT
 *	     :�c�c ADJUSTMENT
 *
 *--------------------------------------------------------------*/
static	void	scrolled_window_event_button_on( Q8tkWidget *widget )
{
  if( widget->child && widget->child->type==Q8TK_TYPE_LIST ){
    q8tk_widget_grab_default( widget->child );
    set_construct_flag( TRUE );
  }
}
Q8tkWidget	*q8tk_scrolled_window_new( Q8tkWidget *hadjustment,
					   Q8tkWidget *vadjustment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_SCROLLED_WINDOW;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->stat.scrolled.width  = 10;
  w->stat.scrolled.height = 10;
  w->stat.scrolled.hpolicy = Q8TK_POLICY_ALWAYS;
  w->stat.scrolled.vpolicy = Q8TK_POLICY_ALWAYS;
  w->stat.scrolled.hscrollbar = TRUE;
  w->stat.scrolled.vscrollbar = TRUE;

  if( hadjustment ) w->stat.scrolled.hadj = hadjustment;
  else{             w->stat.scrolled.hadj = q8tk_adjustment_new( 0, 0,7, 1,10);
                    w->with_label = TRUE; 
  }
  q8tk_adjustment_set_arrow( w->stat.scrolled.hadj, TRUE );
  q8tk_adjustment_set_length( w->stat.scrolled.hadj, 7 );
  w->stat.scrolled.hadj->stat.adj.horizontal = TRUE;
  w->stat.scrolled.hadj->parent = w;

  if( vadjustment ) w->stat.scrolled.vadj = vadjustment;
  else{             w->stat.scrolled.vadj = q8tk_adjustment_new( 0, 0,7, 1,10);
		    w->with_label = TRUE;
  }
  q8tk_adjustment_set_arrow( w->stat.scrolled.vadj, TRUE );
  q8tk_adjustment_set_length( w->stat.scrolled.vadj, 7 );
  w->stat.scrolled.vadj->stat.adj.horizontal = FALSE;
  w->stat.scrolled.vadj->parent = w;

  w->event_button_on  = scrolled_window_event_button_on;

  return	w;
}
void		q8tk_scrolled_window_set_policy( Q8tkWidget *scrolledw,
						  int hscrollbar_policy,
						  int vscrollbar_policy )
{
  scrolledw->stat.scrolled.hpolicy = hscrollbar_policy;
  scrolledw->stat.scrolled.vpolicy = vscrollbar_policy;

  if      ( scrolledw->stat.scrolled.hpolicy==Q8TK_POLICY_ALWAYS ){
    scrolledw->stat.scrolled.hscrollbar = TRUE;
  }else if( scrolledw->stat.scrolled.hpolicy==Q8TK_POLICY_NEVER  ){
    scrolledw->stat.scrolled.hscrollbar = FALSE;
  }

  if      ( scrolledw->stat.scrolled.vpolicy==Q8TK_POLICY_ALWAYS ){
    scrolledw->stat.scrolled.vscrollbar = TRUE;
  }else if( scrolledw->stat.scrolled.vpolicy==Q8TK_POLICY_NEVER  ){
    scrolledw->stat.scrolled.vscrollbar = FALSE;
  }

  set_construct_flag( TRUE );
}
#if 0
void		q8tk_scrolled_window_set_width_height( Q8tkWidget *w, 
						       int width, int height )
{
  w->stat.scrolled.width  = width;
  w->stat.scrolled.height = height;

  set_construct_flag( TRUE );
}
#endif






/*--------------------------------------------------------------
 * ENTRY
 *	�E�G���g���[
 *	�E�����̓��͂��\�B�������A���̃E�B�W�b�g���u�����v��
 *	  ���ׂ� EUC-JAPAN ��O��ɂ��Ă���̂ɑ΂��A�G���g���[
 *	  �ł́APC-8801 �Ŏg�p����Ă��� ANK���� ��O��ɂ��Ă���B
 *	  (�܂�A�����͋֎~�B���p�J�i��O���t�B�b�N�����͉� !)
 *	�E�q�͎��ĂȂ�
 *	�E�V�O�i��
 *		"activate"	���^�[���L�[���͂����������ɔ���
 *		"changed"	�������́A�����폜�����������ɔ���
 *	--------------------------------------------------------
 *	- ENTRY
 *
 *--------------------------------------------------------------*/
static	void	entry_event_button_on( Q8tkWidget *widget )
{
  int	mouse_x = mouse.x / 8;

  q8tk_entry_set_position( widget,
			   mouse_x - widget->x + widget->stat.entry.disp_pos );
  set_construct_flag( TRUE );
}
static	void	entry_event_key_on( Q8tkWidget *widget, int key )
{
  switch( key ){
  case Q8TK_KEY_LEFT:
    if( widget->stat.entry.cursor_pos ){
      widget->stat.entry.cursor_pos --;
      if( widget->stat.entry.cursor_pos < widget->stat.entry.disp_pos ){
	widget->stat.entry.disp_pos --;
      }
      set_construct_flag( TRUE );
    }
    break;
  case Q8TK_KEY_RIGHT:
    if( widget->stat.entry.cursor_pos < (int)strlen( widget->name ) ){
      widget->stat.entry.cursor_pos ++;
      if( widget->stat.entry.cursor_pos - widget->stat.entry.disp_pos +1
					   > widget->stat.entry.disp_length ){
	widget->stat.entry.disp_pos ++;
      }
      set_construct_flag( TRUE );
    }
    break;
  case Q8TK_KEY_RET:
    widget_signal_do( widget, "activate" );
    break;
  case Q8TK_KEY_BS:
    if( widget->stat.entry.cursor_pos ){
      memcpy( &widget->name[ widget->stat.entry.cursor_pos-1 ],
	      &widget->name[ widget->stat.entry.cursor_pos ],
	      strlen( &widget->name[ widget->stat.entry.cursor_pos ])+1 );
      widget->stat.entry.cursor_pos --;
      if( widget->stat.entry.cursor_pos < widget->stat.entry.disp_pos ){
	widget->stat.entry.disp_pos --;
      }
      widget_signal_do( widget, "changed" );
      set_construct_flag( TRUE );
    }
    break;
  default:
    if( key <= 0xff && isprint(key) ){
      if( (int)strlen( widget->name ) < widget->stat.entry.max_length ){
	memmove( &widget->name[ widget->stat.entry.cursor_pos+1 ],
		 &widget->name[ widget->stat.entry.cursor_pos ],
		 strlen( &widget->name[ widget->stat.entry.cursor_pos ])+1 );
	widget->name[ widget->stat.entry.cursor_pos ] = key;
	widget->stat.entry.cursor_pos ++;
	if( widget->stat.entry.cursor_pos - widget->stat.entry.disp_pos +1
					   > widget->stat.entry.disp_length ){
	  widget->stat.entry.disp_pos ++;
	}
	widget_signal_do( widget, "changed" );
	set_construct_flag( TRUE );
      }
    }
  }
}
Q8tkWidget	*q8tk_entry_new( void )
{
  return	q8tk_entry_new_with_max_length( 1024 );
}
Q8tkWidget	*q8tk_entry_new_with_max_length( int max )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_ENTRY;
  w->name  = (char *)ex_malloc( max+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  w->name[0] = '\0';

  w->stat.entry.max_length    = max;
  w->stat.entry.malloc_length = max;
  w->stat.entry.disp_pos      = 0;
  w->stat.entry.cursor_pos    = 0;
  w->stat.entry.disp_length   = 10;

  w->event_button_on  = entry_event_button_on;
  w->event_key_on     = entry_event_key_on;

  return	w;
}
const	char	*q8tk_entry_get_text( Q8tkWidget *entry )
{
  return	entry->name;
}
void		q8tk_entry_set_text( Q8tkWidget *entry, const char *text )
{
  q8_strncpy( entry->name, text, entry->stat.entry.max_length+1 );
  entry->stat.entry.cursor_pos = strlen(entry->name);
  entry->stat.entry.disp_pos   =
		entry->stat.entry.cursor_pos -entry->stat.entry.disp_length +1;
  if( entry->stat.entry.disp_pos<0 ) entry->stat.entry.disp_pos = 0;

  set_construct_flag( TRUE );
}
void		q8tk_entry_set_position( Q8tkWidget *entry, int position )
{
  if( position < 0 ) position = 0;
  if( (int)strlen( entry->name ) < position ) position = strlen( entry->name );

  if( position < entry->stat.entry.disp_pos ){

    entry->stat.entry.disp_pos   = position;
    entry->stat.entry.cursor_pos = position;

  }else if( position - entry->stat.entry.disp_pos
				>= entry->stat.entry.disp_length ){
    entry->stat.entry.disp_pos   = position - entry->stat.entry.disp_length +1;
    entry->stat.entry.cursor_pos = position;

  }else{

    entry->stat.entry.cursor_pos = position;

  }
  set_construct_flag( TRUE );
}
void	q8tk_entry_set_max_length( Q8tkWidget *entry, int max )
{
  if( max <= entry->stat.entry.malloc_length ){
    entry->stat.entry.max_length = max;
  }else{
    entry->name = (char *)realloc( entry->name, max+1 );
    Q8tkAssert(entry->name,"memory exhoused");
    CHECK_MALLOC_OK( entry->name, "memory" );
    entry->stat.entry.max_length    = max;
    entry->stat.entry.malloc_length = max;
  }
}
#if 0
void	q8tk_entry_set_width( Q8tkWidget *entry, int max )
{
  entry->stat.entry.disp_length = max;
  set_construct_flag( TRUE );
}
#endif



/*--------------------------------------------------------------
 * OPTION MENU
 *	�EMENU �� �y��
 *	�E�q�͎��ĂȂ����AMENU ���q�̂悤�Ɉ�����B
 * MENU
 *	�EOPTION MENU �̎q�ɂȂ��
 *	�E�V�O�i��
 *		"changed"	���e���ς�������ɔ���
 * RADIO MENU ITEM
 *	�E�ꉞ�R���e�i�����ALABEL�����悹��Ȃ�
 *	�E�V�O�i��
 *		"activate"	�I�����ꂽ���ɔ���
 *		"activate-list"	�����Ŏg�p��������ȃV�O�i��
 *				��ʂ̎g�p�͋֎~�B
 * MENU LIST
 *	�EOPTION MENU �����̓����ł̂ݎg�p��������ȃE�B�W�b�g
 *	�E��ʂɎg�p�͕s��
 *	--------------------------------------------------------
 *
 *	OPTION MENU - MENU
 *			:
 *			:�c�c RADIO MENU ITEM - LABEL
 *				     |
 *			      RADIO MENU ITEM - LABEL
 *				     |
 *			      RADIO MENU ITEM - LABEL
 *				     |
 *
 *	  �EOPTION MENU �̓R���e�i�����AMENU �����悹��Ȃ��B
 *	    ( q8tk_container_add()�֐��ł͂Ȃ��Aq8tk_option_menu_set_menu()
 *	      �֐����g�p���āAMENU ���悹��B)
 *	  �EMENU �́A�R���e�i�ł͂Ȃ��B
 *	    RADIO MENU ITEM �𕡐��A���j���[�Ƃ��ēo�^���邱�Ƃ��o����B
 *	    ( q8tk_menu_append()�֐����g���āA�o�^���� )
 *	  �ERADIO MENU ITEM �́A�R���e�i�����ALABEL �����悹��Ȃ��B
 *
 *	��OPTION MENU �� �}�E�X�ŃN���b�N�����ƁAOPTION MENU �́A
 *	  WINDOW (DIALOG) �������������A�ȉ��̂悤�Ȑe�q�\�������B
 *	  �Ȃ��A�q������ radio menu item - label �́A��� MENU �ɓo�^
 *	  ���ꂽ�A�E�B�W�b�g�ł���B
 *
 *	WINDOW - MENU LIST - radio menu item - label
 *				    |
 *			     radio menu item - label
 *				    |
 *			     radio menu item - label
 *				    |
 *
 *	�����̂Ƃ��Aradio menu item �͓����ŁA�V�O�i�� "activate-list" ��
 *	�o�^�����B(���[�U�́A���̃V�O�i���̎g�p�֎~)
 *
 *	���K���� radio menu item ���N���b�N�����ƁA"activate-list"
 *	�V�O�i�����������A���������ARADIO MENU ITEM �� "activate"�A
 *	MENU �ɁA"selection_changed" �V�O�i����������B
 *
 *	GTK �������ɐ^���������ɁA�����ɓ��B
 *--------------------------------------------------------------*/
static	Q8tkWidget	*q8tk_menu_list_new( void );
static	void	q8tk_menu_list_select_child( Q8tkWidget *mlist,
					     Q8tkWidget *child );
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *	OPTION MENU
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*
 * OPTION MENU �ɂ��A�����������ꂽ���j���[���X�g�̊e�A�C�e���̃R�[���o�b�N�B
 *	RADIO MENU ITEM �� "activate"�AMENU �� "changed" �𑗂�B
 */
static	Q8tkWidget *option_menu_list, *option_menu_window;
static	void	option_menu_event_list_callback( Q8tkWidget *list_item,
						 Q8tkWidget *parent )
{
  Q8List *l;

  if( parent->stat.menu.selected != list_item ){
    parent->stat.menu.selected = list_item;
    widget_signal_do( parent, "changed" );
  }

  l = parent->stat.menu.list;
  while( l ){
    q8tk_signal_connect( (Q8tkWidget *)(l->ptr), "activate-list", NULL, 0 );
    if( (Q8tkWidget *)(l->ptr) == parent->stat.menu.selected ){
      widget_signal_do(  (Q8tkWidget *)(l->ptr), "activate" );
    }
    l = l->next;
  }

  q8tk_grab_remove( option_menu_window );

  q8tk_widget_destroy( option_menu_list );	/* MENU LIST �͔j���B�A���q��*/
  q8tk_widget_destroy( option_menu_window );	/* RADIO MENU ITEM �͎c���B*/
}

/*
 * OPTION MENU ���N���b�N���ꂽ���̃R�[���o�b�N�B
 *	WINDOW �` MENU LIST ��������������
 */
static	void	option_menu_event_button_on( Q8tkWidget *widget )
{
  Q8List     *l;
  Q8tkWidget *w = widget->child;

  if( w ){
    option_menu_list   = q8tk_menu_list_new();
    option_menu_window = q8tk_window_new( Q8TK_WINDOW_DIALOG );

    l = w->stat.menu.list;
    while( l ){
      q8tk_container_add( option_menu_list, (Q8tkWidget *)(l->ptr) );
      l = l->next;
    }

    q8tk_menu_list_select_child( option_menu_list, w->stat.menu.selected );

    l = w->stat.menu.list;
    while( l ){
      q8tk_signal_connect( (Q8tkWidget *)(l->ptr), "activate-list",
			   option_menu_event_list_callback, w );
      l = l->next;
    }

    q8tk_widget_show( option_menu_list );
    q8tk_container_add( option_menu_window, option_menu_list );

    q8tk_widget_show( option_menu_window );
    q8tk_grab_add( option_menu_window );
    q8tk_widget_grab_default( option_menu_list );

    option_menu_window->stat.window.set_position = TRUE;
    option_menu_window->stat.window.x = w->x -1;
    option_menu_window->stat.window.y = w->y -1;

    {
      int nth = 0;
      l = w->stat.menu.list;
      while( l ){
	if( l->ptr == w->stat.menu.selected ) break;
	nth ++;
	l = l->next;
      }
      option_menu_window->stat.window.y -= nth;
    }
  }
}
static	void	option_menu_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    option_menu_event_button_on( widget );
  }
}

Q8tkWidget	*q8tk_option_menu_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_OPTION_MENU;
  w->attr   = Q8TK_ATTR_CONTAINER | Q8TK_ATTR_MENU_CONTAINER;

  w->stat.option.button = TRUE;

  w->event_button_on  = option_menu_event_button_on;
  w->event_key_on     = option_menu_event_key_on;

  return	w;
}
void		q8tk_option_menu_set_menu( Q8tkWidget *option_menu,
					   Q8tkWidget *menu )
{
  Q8tkAssert(menu->type==Q8TK_TYPE_MENU,NULL);
  q8tk_container_add( option_menu, menu );
}
void		q8tk_option_menu_remove_menu( Q8tkWidget *option_menu )
{
  if( option_menu->child ){
    q8tk_container_remove( option_menu, option_menu->child );
  }
}
void		q8tk_option_menu_set_button( Q8tkWidget *option_menu,
					     int button )
{
  if( option_menu->stat.option.button != button ){
    option_menu->stat.option.button = button;
    set_construct_flag( TRUE );
  }
}
#if 0
Q8tkWidget 	*q8tk_option_menu_get_menu( Q8tkWidget *option_menu )
{
  if( option_menu->child ){
    return (option_menu->child)->stat.menu.selected;
  }
  return NULL;
}
void		q8tk_option_menu_set_menu( Q8tkWidget *option_menu,
					   Q8tkWidget *menu )
{
  if( option_menu->child ){
    (option_menu->child)->stat.menu.selected = menu;
  }
}
#endif


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *	MENU
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
Q8tkWidget	*q8tk_menu_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_MENU;

  return	w;
}
void		q8tk_menu_append( Q8tkWidget *menu, Q8tkWidget *widget )
{
  Q8tkAssert(widget->type==Q8TK_TYPE_RADIO_MENU_ITEM,NULL);

  if( menu->stat.menu.list ){
    q8_list_append( q8_list_last( menu->stat.menu.list ), widget );
  }else{
    menu->stat.menu.list     = q8_list_append( NULL, widget );
    menu->stat.menu.selected = widget;
  }
}
Q8tkWidget	*q8tk_menu_get_active( Q8tkWidget *menu )
{
  return	menu->stat.menu.selected;
}
void		q8tk_menu_set_active( Q8tkWidget *menu, int index )
{
  int	i;
  Q8List *l = menu->stat.menu.list;

  if( l==NULL ) return;
  for( i=0; i<index; i++ ){
    if( (l = l->next) == NULL ) return;
  }

  if( menu->stat.menu.selected != l->ptr ){
    menu->stat.menu.selected = (Q8tkWidget *)(l->ptr);
    widget_signal_do( menu, "changed" );
    set_construct_flag( TRUE );
  }
  widget_signal_do( (Q8tkWidget *)(l->ptr), "activate" );
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *	RADIO MENU ITEM
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static	void	radio_menu_item_event_button_on( Q8tkWidget *widget )
{
  Q8List	*list;

  if( widget->stat.button.active == Q8TK_BUTTON_ON ){
    widget_signal_do( widget, "activate-list" );
    return;
  }

  list = widget->stat.button.list;
  while( list->prev ){
    list = list->prev;
    ((Q8tkWidget *)(list->ptr))->stat.button.active = Q8TK_BUTTON_OFF;
  }
  list = widget->stat.button.list;
  while( list->next ){
    list = list->next;
    ((Q8tkWidget *)(list->ptr))->stat.button.active = Q8TK_BUTTON_OFF;
  }

  widget->stat.button.active = Q8TK_BUTTON_ON;

  {
    if( widget->parent &&
        widget->parent->type == Q8TK_TYPE_MENU_LIST )
      widget->parent->stat.menulist.active = widget;
    widget_redraw_now();	/* ��U�ĕ`�� */
  }

  widget_signal_do( widget, "activate-list" );


  set_construct_flag( TRUE );
}
static	void	radio_menu_item_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    radio_menu_item_event_button_on( widget );
  }
}
Q8tkWidget	*q8tk_radio_menu_item_new( Q8List *list )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_RADIO_MENU_ITEM;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on  = radio_menu_item_event_button_on;
  w->event_key_on     = radio_menu_item_event_key_on;

  if( list ) w->stat.button.active = FALSE;
  else       w->stat.button.active = TRUE;
  w->stat.button.list = q8_list_append( list, w );

  return	w;
}
Q8tkWidget	*q8tk_radio_menu_item_new_with_label( Q8List *list,
						      const char *label )
{
  Q8tkWidget	*b = q8tk_radio_menu_item_new( list );
  Q8tkWidget	*l  = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}
Q8List*		q8tk_radio_menu_item_group( Q8tkWidget *radio_menu_item )
{
  return	radio_menu_item->stat.button.list;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *	MENU LIST
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static	void	menu_list_event_key_on( Q8tkWidget *widget, int key )
{
  Q8tkWidget *w = widget->stat.menulist.active;

  if( w ){
    switch( key ){

    case Q8TK_KEY_RET:
    case Q8TK_KEY_SPACE:
      (*w->event_button_on)( w );
      break;

    case Q8TK_KEY_UP:
      if( w->prev ){
	widget->stat.list.active = w->prev;
	set_construct_flag( TRUE );
      }
      break;
    case Q8TK_KEY_DOWN:
      if( w->next ){
	widget->stat.list.active = w->next;
	set_construct_flag( TRUE );
      }
      break;
    }
  }
}
static	Q8tkWidget	*q8tk_menu_list_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_MENU_LIST;
  w->attr  = Q8TK_ATTR_CONTAINER;

  w->event_key_on = menu_list_event_key_on;

  return	w;
}
static	void	q8tk_menu_list_select_child( Q8tkWidget *mlist,
					     Q8tkWidget *child )
{
  if( mlist->stat.menulist.selected != child ){
    mlist->stat.menulist.selected = child;
    mlist->stat.menulist.active   = child;
    set_construct_flag( TRUE );
    /* �V�O�i���͑���Ȃ�  ���̎��_�ő���Ƃ�₱�����Ȃ�B
       widget_signal_do( child, "activate" );
    */
  }
}




/*--------------------------------------------------------------
 * ACCEL GROUP
 *	�E�A�N�Z�����[�^�[�L�[��`�̓y�� (�e)
 *	�E�q�͎��ĂȂ����A�\�����ł��Ȃ��B
 *	�E�����I�ɂ́AACCEL KEY ���q�Ɏ��`�Ԃ��Ƃ�
 *	�E�E�C���h�E�Ɋ֘A�Â��邱�ƂŁA�q�� ACCEL KEY ��
 *	  ���̃E�C���h�E�ŗL���ɂ��邱�Ƃ��ł���B
 * ACCEL KEY
 *	�E�A�N�Z�����[�^�[�L�[��`
 *	�E�q�͎��ĂȂ����A�\�����ł��Ȃ��B
 *	�E�����I�ɂ́AACCEL GROUP �̎q�̌`�Ԃ��Ƃ�
 *	�E�e�� ACCEL GROUP �Ɗ֘A�t����ꂽ�E�C���h�E�ɂāA
 *	  �L�[�{�[�h���͂��󂯂Ƃ�ƁA��`�����E�B�W�b�g�ɃV�O�i����
 *	  ����B
 *	--------------------------------------------------------
 *	 WINDOW
 *	    :
 *	    :�c�c ACCEL GROUP - ACCEL KEY
 *				    |
 *				ACCEL KEY
 *				    |
 *				ACCEL KEY
 *				    |
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8k_accel_group_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_ACCEL_GROUP;

  return w;
}
void	q8tk_accel_group_attach( Q8tkWidget *accel_group, Q8tkWidget *window )
{
  Q8tkAssert(window->type==Q8TK_TYPE_WINDOW,NULL);
  window->stat.window.accel = accel_group;
}
void	q8tk_accel_group_detach( Q8tkWidget *accel_group, Q8tkWidget *window )
{
  Q8tkAssert(window->type==Q8TK_TYPE_WINDOW,NULL);

  if( window->stat.window.accel == accel_group ){
    window->stat.window.accel = NULL;
  }
}
void	q8tk_accel_group_add( Q8tkWidget *accel_group, int accel_key,
			      Q8tkWidget *widget,      const char *signal )
{
  Q8tkWidget	*w;
  w = malloc_widget();

  if( accel_group->child ){
    Q8tkWidget *c = accel_group->child;
    while( c->next ){
      c = c->next;
    }
    c->next      = w;
    w->prev = c;
    w->next = NULL;
  }else{
    accel_group->child = w;
    w->prev     = NULL;
    w->next     = NULL;
  }
  w->parent = accel_group;

  w->type  = Q8TK_TYPE_ACCEL_KEY;
  w->name  = (char *)ex_malloc( strlen(signal)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, signal );

  w->stat.accel.key    = accel_key;
  w->stat.accel.widget = widget;
}



/*--------------------------------------------------------------
 * DIALOG
 *	�E�_�C�A���O
 *	--------------------------------------------------------
 *	WINDOW - FRAME - VBOX - VBOX(vbox)
 *	  :		 	 |
 *	  :			HBOX(action_area)
 *	DIALOG
 *
 *	q8tk_dialog_new()�̕Ԃ�l�́AWINDOW �ɂȂ�B
 *
 *	DIALOG �� ���[�N�ɁAVBOX (vbox) �ƁAHBOX (action_area) ��
 *	�ݒ肳���̂ŁA�������ă��[�U�͐ݒ���s�Ȃ��B
 *
 *	q8tk_widget_destroy() �̍ۂ́A����4�̃E�C�W�b�g��
 *	�j�󂷂�B
 *
 *--------------------------------------------------------------*/
Q8tkWidget	*q8tk_dialog_new( void )
{
  Q8tkWidget	*dialog, *window, *vbox, *frame;

  dialog = malloc_widget();
  dialog->type = Q8TK_TYPE_DIALOG;

  window = q8tk_window_new( Q8TK_WINDOW_DIALOG );
  window->stat.window.work = dialog;

  frame = q8tk_frame_new("");
  q8tk_container_add( window, frame );
  q8tk_widget_show( frame );
  q8tk_frame_set_shadow_type( frame, Q8TK_SHADOW_NONE );

  vbox = q8tk_vbox_new();
  q8tk_container_add( frame, vbox );
  q8tk_widget_show( vbox );

  dialog->stat.dialog.vbox = q8tk_vbox_new();
  q8tk_box_pack_start( vbox, dialog->stat.dialog.vbox );
  q8tk_widget_show( dialog->stat.dialog.vbox );

  dialog->stat.dialog.action_area = q8tk_hbox_new();
  q8tk_box_pack_start( vbox, dialog->stat.dialog.action_area );
  q8tk_widget_show( dialog->stat.dialog.action_area );

  return window;
}




/*--------------------------------------------------------------
 * FILE SELECTION
 *	�E�t�@�C���Z���N�V����
 *	�E������t�@�C�����̒����́A�p�X���܂߂čő��
 *	  Q8TK_MAX_FILENAME �܂łł���B(������I�[��\0���܂�)
 *	--------------------------------------------------------
 *	WINDOW - VBOX - LABEL (q8tk_fileselection_new()�̈���)
 *	  :		  |
 *	  :		HSEPARATOR
 *	FSELECT		  |
 *			LABEL (�f�B���N�g����)
 *			  |
 *			LABEL (�t�@�C����)
 *			  |
 *			SCROLLED - LIST (file_list) - LIST ITEM
 *			  |   -+- ADJ			| +- LABEL
 *			LABEL  +- ADJ		      LIST ITEM
 *			  |				| +- LABEL
 *			ENTRY (selection_entry)	      LIST ITEM
 *			  |				| +- LABEL
 *			HVOX - BUTTON (cancel_button)   :
 *				 | +- LABEL		:
 *			       BUTTON (ok_button)
 *				   +- LABEL
 *
 *	q8tk_fileselection_new()�̕Ԃ�l�́AWINDOW �ɂȂ�B
 *
 *	FILESELECTION �� ���[�N�ɁALIST(file_list) �ƁAENTRY
 *	(selection_entry) �ƁABUTTON (cancel_button/ok_button) ��
 *	�ݒ肳���̂ŁA�������ă��[�U�͐ݒ���s�Ȃ��B
 *
 *	q8tk_widget_destroy() �̍ۂ́A���ׂẴE�B�W�b�g( 18��
 *	�E�B�W�b�g �y�� �s�萔�� LIST ITEM �E�C�W�b�g)��j�󂷂�B
 *
 *	�EENTRY �� changed ���󂯂��ꍇ�A���͂����������̂Ƃ݂Ȃ��B
 *	�ELIST ITEM �� select ���󂯂��ꍇ�A
 *		LIST �� selection_changed ���󂯂��ꍇ�́A
 *			ENTRY �� LIST ITEM �̕�������Z�b�g
 *		LIST �� selection_changed ���󂯂Ă��Ȃ��ꍇ�́A
 *			���͂����������̂Ƃ݂Ȃ��B
 *
 *	�E���͂��������ꍇ�A�ȉ��̏������s�Ȃ��B
 *		���͂��f�B���N�g���̏ꍇ
 *			LIST ITEM ��S�čX�V
 *			ENTRY �̓N���A
 *		���͂��t�@�C���̏ꍇ�A
 *			���̃t�@�C������ filename �Ƃ���B
 *			ok_button �� �V�O�i���𑗂�
 *
 *--------------------------------------------------------------*/

static	int	fsel_set_file_list( Q8tkWidget *fsel, const char *filename );
static	int	fsel_open_file( Q8tkWidget *fselect, const char *filename );

/* �t�@�C�����X�g�ŁA�I������Ă���A�C�e�����A���������̃R�[���o�b�N */
static	void	fsel_list_selection_changed( Q8tkWidget *dummy, void *fselect )
{
  Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = TRUE;
}

/* �t�@�C�����X�g�ŁA�A�C�e�����I�����ꂽ���̃R�[���o�b�N */
static	void	fsel_item_selected( Q8tkWidget *item, void *fselect )
{
  char	name[ Q8TK_MAX_FILENAME ];
  char	wk[ Q8TK_MAX_FILENAME ];
  T_DIR_ENTRY dir;


  /* LIST ITEM �ɓo�^���ꂽ�t�@�C�������A�A�N�Z�X�\�ȃt�@�C�����ɕϊ� */

  dir.type = item->stat.misc.attr;
  dir.name = item->child->name;
  osd_readdir_realname( name, &dir, Q8TK_MAX_FILENAME );


	/* ���ݑI������Ă���̂ƁA�ʂ̃t�@�C�������I�΂ꂽ���A
	   ���̃t�@�C�������A�G���g���[�ɃZ�b�g����B*/

  if( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed ){

    Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = FALSE;

    ((Q8tkWidget*)fselect)->stat.fselect.selection_changed = FALSE;

    q8tk_entry_set_text( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)
				->selection_entry, name );

	/* ���ݑI������Ă���̂ƁA�����t�@�C�������I�΂ꂽ���A
	   ���̃t�@�C�����J���B*/

  }else{

				/* ���݂̃p�X (file_selection_pathname) �ƁA
				   �I�������t�@�C���� (item->child->name) ����
				   �t���p�X�̃t�@�C�����𐶐����� */
    osd_path_connect( wk, file_selection_pathname, name, Q8TK_MAX_FILENAME );

    if( fsel_open_file( (Q8tkWidget*)fselect, wk ) ){
      /*widget_redraw_now();*/	/* ��U�ĕ`�� */
      widget_signal_do( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->ok_button,
		        "clicked" );
    }

  }

}

/* �t�@�C�����X�g�ŁA�G���g���[�ɓ��͂��ꂽ���̃R�[���o�b�N */
static	void	fsel_entry_activate( Q8tkWidget *entry, void *fselect )
{
  char	wk[ Q8TK_MAX_FILENAME ];

			/* ���݂̃p�X (file_selection_pathname) �ƁA
			   ���͂����t�@�C���� (q8tk_entry_get_text(entry)) ����
			   �t���p�X�̃t�@�C�����𐶐����� */
  osd_path_connect( wk, file_selection_pathname,
		    q8tk_entry_get_text(entry), Q8TK_MAX_FILENAME );


  if( fsel_open_file( (Q8tkWidget*)fselect, wk ) ){
    widget_signal_do( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->ok_button,
		      "clicked" );
  }
}


/* �f�B���N�g�����J���āA�t�@�C�����X�g�𐶐�����֐� (�R�[���o�b�N����Ȃ�) */
static	int	fsel_set_file_list( Q8tkWidget *fsel, const char *filename )
{
  T_DIR_INFO	*dirp;
  T_DIR_ENTRY	*dirent;
  char		wk[ Q8TK_MAX_FILENAME ];
  int		nr = 0, i;
  Q8tkWidget	*c;

		/* ������ LIST ITEM �폜 */

  while( ( c = Q8TK_FILE_SELECTION(fsel)->file_list->child ) ){
    q8tk_container_remove( Q8TK_FILE_SELECTION(fsel)->file_list, c );
    q8tk_widget_destroy( c );
  }
  q8tk_adjustment_set_value( Q8TK_FILE_SELECTION(fsel)->scrolled_window
						    ->stat.scrolled.hadj, 0 );
  q8tk_adjustment_set_value( Q8TK_FILE_SELECTION(fsel)->scrolled_window
						    ->stat.scrolled.vadj, 0 );


		/* �f�B���N�g���𒲂ׁA�t�@�C������ LIST ITEM �Ƃ��ēo�^ */

  if( (dirp = osd_opendir( filename )) ){

    while( (dirent = osd_readdir( dirp )) ){
      Q8tkWidget *item;

      q8_strncpy( wk, dirent->name, Q8TK_MAX_FILENAME );

      item = q8tk_list_item_new_with_label( wk );
      q8tk_container_add( Q8TK_FILE_SELECTION(fsel)->file_list, item );
      q8tk_widget_show( item );
      q8tk_signal_connect( item, "select", fsel_item_selected, fsel );

      item->stat.misc.attr = dirent->type;

      nr ++;
    }
    osd_closedir( dirp );


    i = strlen(filename);
    if( i + 6 > Q8GR_SCREEN_X * 0.5 ){		/* 6 == sizeof("DIR = ") */
      i = i - (Q8GR_SCREEN_X * 0.5 - 6 -3);	/* 3 == sizeof("...")    */
    }else{
      i = 0;
    }
    q8_strncpy( wk, "DIR = ", Q8TK_MAX_FILENAME );
    if( i ){
      q8_strncat( wk, "...", Q8TK_MAX_FILENAME );
    }
    q8_strncat( wk, &filename[i], Q8TK_MAX_FILENAME );
    q8tk_label_set( Q8TK_FILE_SELECTION(fsel)->dir_name, wk );


    sprintf( wk, "%d file(s)", nr );
    q8tk_label_set( Q8TK_FILE_SELECTION(fsel)->nr_files, wk );
  }

  return	nr;
}

static	int	fsel_open_file( Q8tkWidget *fselect, const char *filename )
{
  const char *child;
  char	fname[ Q8TK_MAX_FILENAME ];
  char	parent[ Q8TK_MAX_FILENAME ];

  if( filename   ==NULL ||
      filename[0]=='\0' ){		/* filename�������̏ꍇ�́A	*/
    filename = osd_get_current_dir();	/* �J�����g�f�B���N�g���Ƃ���	*/
    if( filename==NULL ) filename = "";
  }

  q8_strncpy( fname, filename, Q8TK_MAX_FILENAME );
  osd_path_regularize( fname, Q8TK_MAX_FILENAME );	/* ���K�� */


	/**** �f�B���N�g�����ǂ����𒲂ׂ� ****/

  if( osd_file_stat( fname )==FILE_STAT_DIR ){

    fsel_set_file_list( fselect, fname );
    q8_strncpy( file_selection_pathname, fname, Q8TK_MAX_FILENAME );
    q8tk_entry_set_text( Q8TK_FILE_SELECTION(fselect)->selection_entry, "" );

    return 0;			/* filename �̓f�B���N�g���ł��� */


	/**** �f�B���N�g������Ȃ������ꍇ ****/
  }else{

		/* �t�@�C������O�� '/' ���������A���̃f�B���N�g�����Ђ炭 */

    child = osd_path_separate( parent, fname, Q8TK_MAX_FILENAME );

    if( parent[0] != '\0' ){			/* �p�X��������		*/

      if( fsel_set_file_list( fselect, parent ) ){  /*�f�B���N�g���J������ */

						    /*�p�X��ێ����Ă����� */
	q8_strncpy( file_selection_pathname, parent, Q8TK_MAX_FILENAME );

	if( child ) filename = child;		    /*�p�X�������         */
	else        filename = "";		    /*�t�@�C�������w��     */

      }else{					    /* �f�B���N�g���J���Ȃ� */

	q8tk_label_set( Q8TK_FILE_SELECTION(fselect)->dir_name, 
							"DIR = non existant" );
	q8tk_label_set( Q8TK_FILE_SELECTION(fselect)->nr_files, 
							"0 file(s)" );
      }
    }

    q8tk_entry_set_text( Q8TK_FILE_SELECTION(fselect)->selection_entry,
			 filename );

    return 1;			/* filename �̓t�@�C���ł��� */
  }
}





Q8tkWidget	*q8tk_file_selection_new( const char *title )
{
  Q8tkWidget	*fselect, *window, *vbox, *wk;

  fselect = malloc_widget();
  fselect->type = Q8TK_TYPE_FILE_SELECTION;

  window = q8tk_window_new( Q8TK_WINDOW_DIALOG );
  window->stat.window.work = fselect;

  vbox = q8tk_vbox_new();
  q8tk_container_add( window, vbox );
  q8tk_widget_show( vbox );

  wk = q8tk_label_new( title );
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
  q8tk_misc_set_placement( wk, Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP );
  
  wk = q8tk_hseparator_new();
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );

  fselect->stat.fselect.dir_name = q8tk_label_new( "DIR =" );
  q8tk_box_pack_start( vbox, fselect->stat.fselect.dir_name );
  q8tk_widget_show( fselect->stat.fselect.dir_name );
  q8tk_misc_set_placement( fselect->stat.fselect.dir_name,
			   Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP );

  fselect->stat.fselect.nr_files = q8tk_label_new( "0 file(s)" );
  q8tk_box_pack_start( vbox, fselect->stat.fselect.nr_files );
  q8tk_widget_show( fselect->stat.fselect.nr_files );
  q8tk_misc_set_placement( fselect->stat.fselect.nr_files,
			   Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP );

  fselect->stat.fselect.scrolled_window=q8tk_scrolled_window_new( NULL, NULL );
  q8tk_box_pack_start( vbox, fselect->stat.fselect.scrolled_window );
  q8tk_widget_show( fselect->stat.fselect.scrolled_window );

  fselect->stat.fselect.file_list = q8tk_list_new();
  q8tk_container_add( fselect->stat.fselect.scrolled_window,
		      fselect->stat.fselect.file_list );
  q8tk_widget_show( fselect->stat.fselect.file_list );
  q8tk_signal_connect( fselect->stat.fselect.file_list, "selection_changed",
		       fsel_list_selection_changed, window );

  q8tk_misc_set_placement( fselect->stat.fselect.scrolled_window,
			   Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
  q8tk_scrolled_window_set_policy( fselect->stat.fselect.scrolled_window,
				   Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS );
  q8tk_misc_set_size( fselect->stat.fselect.scrolled_window, 32, 14 );
  q8tk_misc_set_size( fselect->stat.fselect.file_list, 32-2, 0 );

  wk = q8tk_label_new( "File name:" );
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
  q8tk_misc_set_placement( wk, Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP );

  fselect->stat.fselect.selection_entry = q8tk_entry_new();
  q8tk_box_pack_start( vbox, fselect->stat.fselect.selection_entry );
  q8tk_widget_show( fselect->stat.fselect.selection_entry );
  q8tk_misc_set_size( fselect->stat.fselect.selection_entry, 40, 0 );
  q8tk_signal_connect( fselect->stat.fselect.selection_entry, "activate",
		       fsel_entry_activate, window );

  wk = q8tk_hbox_new();
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
  q8tk_misc_set_placement( wk, Q8TK_PLACEMENT_X_RIGHT, Q8TK_PLACEMENT_Y_TOP );

  fselect->stat.fselect.ok_button = q8tk_button_new_with_label("  OK  ");
  q8tk_box_pack_start( wk, fselect->stat.fselect.ok_button );
  q8tk_widget_show( fselect->stat.fselect.ok_button );

  fselect->stat.fselect.cancel_button = q8tk_button_new_with_label("CANCEL");
  q8tk_box_pack_start( wk, fselect->stat.fselect.cancel_button );
  q8tk_widget_show( fselect->stat.fselect.cancel_button );

  /*q8tk_file_selection_set_filename( window, file_selection_pathname );*/

  return window;
}
const	char	*q8tk_file_selection_get_filename( Q8tkWidget *fselect )
{
  osd_path_connect( file_selection_filename, file_selection_pathname,
		    q8tk_entry_get_text(Q8TK_FILE_SELECTION(fselect)
							->selection_entry),
		     Q8TK_MAX_FILENAME );

  return file_selection_filename;
}

/* �Ԃ�l�́Afilename �� �t�@�C���Ȃ�^�A�f�B���N�g���Ȃ�U */
int		q8tk_file_selection_set_filename( Q8tkWidget *fselect,
						  const char *filename )
{
  return fsel_open_file( fselect, filename );
}






/************************************************************************/
/* �G���ȏ���								*/
/************************************************************************/

/*--------------------------------------------------------------
 * �\���ʒu�̕ύX (�ꕔ�̃E�B�W�b�g�̂݉\)
 *--------------------------------------------------------------*/
void	q8tk_misc_set_placement( Q8tkWidget *widget,
				 int placement_x, int placement_y )
{
  widget->placement_x = placement_x;
  widget->placement_y = placement_y;
}

/*--------------------------------------------------------------
 * �\���T�C�Y�̕ύX (�ꕔ�̃E�B�W�b�g�̂݉\)
 *	�ECOMBO		  �c �����񕔕��̕\����
 *	�ELIST		  �c ������̕\����
 *	�ESCROLLED WINDOW �c �E�C���h�E�̕��A����
 *	�EENTRY		  �c ������̕\����
 *--------------------------------------------------------------*/
void	q8tk_misc_set_size( Q8tkWidget *widget, int width, int height )
{
  switch( widget->type ){

  case Q8TK_TYPE_COMBO:
    if( width > 0 ) widget->stat.combo.width = width;
    else            widget->stat.combo.width = 0;
    break;

  case Q8TK_TYPE_LIST:
    if( width > 0 )  widget->stat.list.width  = width;
    else             widget->stat.list.width  = 0;
    break;

  case Q8TK_TYPE_SCROLLED_WINDOW:
    widget->stat.scrolled.width  = width;
    widget->stat.scrolled.height = height;
    break;

  case Q8TK_TYPE_ENTRY:
    widget->stat.entry.disp_length = width;
    break;

  default:
#ifdef	Q8TK_ASSERT_DEBUG
    fprintf( stderr, "Cant resize widget=%s\n", debug_type(widget->type));
#endif
    Q8tkAssert(FALSE,NULL);
    return;
  }
  set_construct_flag( TRUE );
}


/*--------------------------------------------------------------
 * �����ĕ`��
 *--------------------------------------------------------------*/
void	q8tk_misc_redraw( void )
{
  set_construct_flag( TRUE );
}

/************************************************************************/
/* �R���e�i�֌W								*/
/************************************************************************/
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_container_add( Q8tkWidget *container, Q8tkWidget *widget )
{
  Q8tkAssert(container->attr&Q8TK_ATTR_CONTAINER,NULL);
  if( container->attr&Q8TK_ATTR_LABEL_CONTAINER ){
    Q8tkAssert(widget->type==Q8TK_TYPE_LABEL,NULL);
  }
  if( container->attr&Q8TK_ATTR_MENU_CONTAINER ){
    Q8tkAssert(widget->type==Q8TK_TYPE_MENU,NULL);
  }

  switch( container->type ){
  case Q8TK_TYPE_LIST:				/* LIST BOX ��O���� */
    if( container->child ){
      Q8tkWidget *c = container->child;
      while( c->next ){
	c = c->next;
      }
      c->next      = widget;
      widget->prev = c;
      widget->next = NULL;
    }else{
      container->child = widget;
      widget->prev     = NULL;
      widget->next     = NULL;
      container->stat.list.selected = widget;
      container->stat.list.active   = widget;
    }
    widget->parent    = container;
    break;

  case Q8TK_TYPE_MENU_LIST:			/* MENU LIST ��O���� */
    if( container->child ){
      Q8tkWidget *c = container->child;
      while( c->next ){
	c = c->next;
      }
      c->next      = widget;
      widget->prev = c;
      widget->next = NULL;
    }else{
      container->child = widget;
      widget->prev     = NULL;
      widget->next     = NULL;
      container->stat.menulist.selected = widget;
      container->stat.menulist.active   = widget;
    }
    widget->parent    = container;
    break;

  default:					/* �ʏ�̏��� */
    container->child = widget;
    widget->parent   = container;
    widget->prev     = NULL;
    widget->next     = NULL;
    break;
  }

  if( widget->visible ){
    widget_map( widget );
  }
}
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_box_pack_start( Q8tkWidget *box, Q8tkWidget *widget )
{
  Q8tkAssert(box->attr&Q8TK_ATTR_CONTAINER,NULL);

  if( box->child==NULL ){
    box->child     = widget;
    widget->prev   = NULL;
    widget->next   = NULL;
  }else{
    Q8tkWidget *c = box->child;
    while( c->next ){
      c = c->next;
    }
    c->next      = widget;
    widget->prev = c;
    widget->next = NULL;
  }
  widget->parent = box;

  if( widget->visible ){
    widget_map( widget );
  }
}
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_box_pack_end( Q8tkWidget *box, Q8tkWidget *widget )
{
  Q8tkAssert(box->attr&Q8TK_ATTR_CONTAINER,NULL);

  if( box->child==NULL ){
    widget->prev   = NULL;
    widget->next   = NULL;
  }else{
    Q8tkWidget *c = box->child;
    Q8tkAssert(c->prev==NULL,NULL);
    c->prev      = widget;
    widget->next = c;
    widget->prev = NULL;
  }
  box->child     = widget;
  widget->parent = box;

  if( widget->visible ){
    widget_map( widget );
  }
}



/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_container_remove( Q8tkWidget *container, Q8tkWidget *widget )
{
  Q8tkAssert(container->attr&Q8TK_ATTR_CONTAINER,NULL);
  Q8tkAssert(widget->parent==container,NULL);

  if( widget->prev == NULL ){			/* �������e�̒����̎� */
    Q8tkWidget *n = widget->next;
    if( n ) n->prev = NULL;
    container->child = n;

  }else{					/* ��������Ȃ��� */
    Q8tkWidget *p = widget->prev;
    Q8tkWidget *n = widget->next;
    if( n ) n->prev = p;
    p->next = n;
  }

  switch( container->type ){
  case Q8TK_TYPE_LIST:				/* LIST BOX ��O���� */
    if( container->stat.list.selected == widget ){
      container->stat.list.selected = container->child;
      container->stat.list.active   = container->child;
    }
    break;
  case Q8TK_TYPE_MENU_LIST:			/* MENU LIST ��O���� */
    if( container->stat.menulist.selected == widget ){
      container->stat.menulist.selected = container->child;
      container->stat.menulist.active   = container->child;
    }
    break;
  }

  if( container->visible ){
    widget_map( container );
  }
}


/************************************************************************/
/* �\���֌W								*/
/************************************************************************/
void	q8tk_widget_show( Q8tkWidget *widget )
{
  widget->visible = TRUE;
  widget_map( widget );
}
void	q8tk_widget_hide( Q8tkWidget *widget )
{
  widget->visible = FALSE;
  widget_map( widget );
}


/************************************************************************/
/* �����֌W								*/
/*	���g������							*/
/*	���g�Ǝq������							*/
/*	���g�Ǝq�ƌZ�������						*/
/*									*/
/*	TODO : ���jCOMBO �̘A��͂ǂ�����H				*/
/************************************************************************/
void	q8tk_widget_destroy( Q8tkWidget *widget )
{
  Q8tkWidget *work;

  if( widget->type == Q8TK_TYPE_WINDOW &&	/* DIALOG �� FILE SELECTION */
      (work = widget->stat.window.work) ){	/* �̗�O����		    */
    switch( work->type ){
    case Q8TK_TYPE_DIALOG:
      Q8tkAssert(widget->child->type==Q8TK_TYPE_FRAME,NULL);
      Q8tkAssert(widget->child->child->type==Q8TK_TYPE_VBOX,NULL);
      q8tk_widget_destroy( Q8TK_DIALOG(widget)->action_area );
      q8tk_widget_destroy( Q8TK_DIALOG(widget)->vbox );
      q8tk_widget_destroy( widget->child->child );
      q8tk_widget_destroy( widget->child );
      q8tk_widget_destroy( work );
      break;
    case Q8TK_TYPE_FILE_SELECTION:
      {
	q8tk_widget_destroy( (Q8TK_FILE_SELECTION(widget)->scrolled_window)
							->stat.scrolled.hadj );
	q8tk_widget_destroy( (Q8TK_FILE_SELECTION(widget)->scrolled_window)
							->stat.scrolled.vadj );
	widget_destroy_all( widget->child );
      }
      q8tk_widget_destroy( work );
      break;
    }
  }

  if( widget->type == Q8TK_TYPE_ACCEL_GROUP &&	/* ACCEL �̎q�͑S�ď��� */
      widget->child ){
    widget_destroy_all( widget->child );
  }


  if( widget->with_label &&			/* XXX_new_with_label()�� */
      widget->child      &&			/* �������ꂽ LABEL�̏��� */
      widget->child->type == Q8TK_TYPE_LABEL ){
    q8tk_widget_destroy( widget->child );
  }
  if( widget->with_label &&			/* ���� NULL �Ő������ꂽ */
      widget->type==Q8TK_TYPE_SCROLLED_WINDOW ){/* SCROLLED WINDOW �̏��� */
    q8tk_widget_destroy( widget->stat.scrolled.hadj );
    q8tk_widget_destroy( widget->stat.scrolled.vadj );
  }

  if( widget->name ){
    ex_free( widget->name );
    widget->name = NULL;
  }
  free_widget( widget );
}
static	void	widget_destroy_child( Q8tkWidget *widget )
{
  if( widget->child ) widget_destroy_child( widget->child );

  if( widget->name ){
    ex_free( widget->name );
    widget->name = NULL;
  }
  free_widget( widget );
}
static	void	widget_destroy_all( Q8tkWidget *widget )
{
  if( widget->next ) widget_destroy_all( widget->next );
  if( widget->child ) widget_destroy_all( widget->child );

  if( widget->name ){
    ex_free( widget->name );
    widget->name = NULL;
  }
  free_widget( widget );
}




/************************************************************************/
/* ����̃E�B�W�b�g�Ƀt�H�[�J�X�𓖂Ă�B				*/
/*	�ʏ� �t�H�[�J�X�́A���O�Ƀ{�^����L�[���͂��Ȃ��ꂽ�E�B�W�b�g	*/
/*	�ɂȂ�ATAB �ɂ��ؑւ��\�����A���̊֐��œ���̃E�B�W�b�g	*/
/*	�Ƀt�H�[�J�X��ݒ肷�邱�Ƃ��ł���B				*/
/*	�������A���̃E�B�W�b�g�̈�Ԑ�c�� WINDOW ���Aq8tk_grab_add()��	*/
/*	�������Ȃ��ꂽ���ƂłȂ���Ζ����ł���B			*/
/************************************************************************/
void	q8tk_widget_grab_default( Q8tkWidget *widget )
{
  set_event_widget( widget );
  set_construct_flag( TRUE );
}


/************************************************************************/
/* �V�O�i���֌W								*/
/************************************************************************/
/*
 *	�C�ӂ̃E�B�W�b�g�ɁA�C�ӂ̃V�O�i���𑗂�
 */
static	void	widget_signal_do( Q8tkWidget *widget, const char *name )
{
  switch( widget->type ){
  case Q8TK_TYPE_BUTTON:			/* �{�^��		*/
    if( strcmp( name, "clicked" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;
  case Q8TK_TYPE_TOGGLE_BUTTON:			/* �g�O���{�^��		*/
  case Q8TK_TYPE_CHECK_BUTTON:			/* �`�F�b�N�{�^��	*/
  case Q8TK_TYPE_RADIO_BUTTON:			/* ���W�I�{�^��		*/
    if      ( strcmp( name, "clicked" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }else if( strcmp( name, "toggled" )==0 ){
      if( widget->user_event_1 ){
	(*widget->user_event_1)( widget, widget->user_event_1_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_NOTEBOOK:			/* �m�[�g�u�b�N		*/
    if( strcmp( name, "switch_page" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_COMBO:				/* �R���{�{�b�N�X���ǂ� */
    if( strcmp( name, "changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_LIST:				/* ���X�g�{�b�N�X	*/
    if( strcmp( name, "selection_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_LIST_ITEM:			/* ���X�g�A�C�e��	*/
    if( strcmp( name, "select" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_ADJUSTMENT:			/* �A�W���X�g�����g */
    if( strcmp( name, "value_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_ENTRY:				/* �G���g���[		*/
    if      ( strcmp( name, "activate" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }else if( strcmp( name, "changed" )==0 ){
      if( widget->user_event_1 ){
	(*widget->user_event_1)( widget, widget->user_event_1_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_MENU:				/* ���j���[ */
    if( strcmp( name, "changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_MENU_LIST:			/* ���j���[���X�g	*/
    if( strcmp( name, "selection_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_RADIO_MENU_ITEM:		/* ���W�I���j���[�A�C�e�� */
    if      ( strcmp( name, "activate" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }else if( strcmp( name, "activate-list" )==0 ){
      if( widget->user_event_1 ){
	(*widget->user_event_1)( widget, widget->user_event_1_parm );
      }
      return;
    }
    break;

  }

#ifdef	Q8TK_ASSERT_DEBUG
  fprintf( stderr, "BAD signal %s '%s'\n", debug_type(widget->type),name);
#endif
  Q8tkAssert(FALSE,NULL);
  return;
}


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_signal_handlers_destroy( Q8tkWidget *widget )
{
  widget->user_event_0 = NULL;
  widget->user_event_1 = NULL;
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
/* �Ԃ�l�́A���� (�K�� 0 ) */
int	q8tk_signal_connect( Q8tkWidget *widget, const char *name,
			     Q8tkSignalFunc func, void *func_data )
{
  switch( widget->type ){
  case Q8TK_TYPE_BUTTON:			/* �{�^��		*/
    if( strcmp( name, "clicked" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;
  case Q8TK_TYPE_TOGGLE_BUTTON:			/* �g�O���{�^��		*/
  case Q8TK_TYPE_CHECK_BUTTON:			/* �`�F�b�N�{�^��	*/
  case Q8TK_TYPE_RADIO_BUTTON:			/* ���W�I�{�^��		*/
    if      ( strcmp( name, "clicked" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }else if( strcmp( name, "toggled" )==0 ){
      widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_1_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_NOTEBOOK:			/* �m�[�g�u�b�N		*/
    if( strcmp( name, "switch_page" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_COMBO:				/* �R���{�{�b�N�X���ǂ� */
    if( strcmp( name, "changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_LIST:				/* ���X�g�{�b�N�X	*/
    if( strcmp( name, "selection_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_LIST_ITEM:			/* ���X�g�A�C�e��	*/
    if( strcmp( name, "select" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_ADJUSTMENT:			/* �A�W���X�g�����g */
    if( strcmp( name, "value_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_ENTRY:				/* �G���g��		*/
    if      ( strcmp( name, "activate" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }else if( strcmp( name, "changed" )==0 ){
      widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_1_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_MENU:				/* ���j���[		*/
    if( strcmp( name, "changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_MENU_LIST:			/* ���j���[���X�g	*/
    if( strcmp( name, "selection_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_RADIO_MENU_ITEM:		/* ���W�I���j���[�A�C�e�� */
    if      ( strcmp( name, "activate" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }else if( strcmp( name, "activate-list" )==0 ){
      widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_1_parm = func_data;
      return 0;
    }
    break;

  }

#ifdef	Q8TK_ASSERT_DEBUG
  fprintf( stderr, "Undefined signal %s '%s'\n",debug_type(widget->type),name);
#endif
  Q8tkAssert(FALSE,NULL);
  return 0;
}



/************************************************************************/
/* �C�x���g�̃`�F�b�N							*/
/************************************************************************/
/*
 *	DRAGGING
 */
static	void	widget_dragging( Q8tkWidget *focus )
{
  if( focus && focus->event_dragging ){
    (*focus->event_dragging)( focus );
  }
}
/*
 *	DRAG OFF
 */
static	void	widget_drag_off( Q8tkWidget *focus )
{
  if( focus && focus->event_drag_off ){
    (*focus->event_drag_off)( focus );
  }
}

/*
 *	MOUSE BUTTON ON
 */
static	void	widget_button_on( Q8tkWidget *focus )
{
  if( focus && focus->event_button_on ){
    (*focus->event_button_on)( focus );
  }
}
/*
 *	KEY ON
 */
static	void		widget_key_on( Q8tkWidget *focus, int key )
{
  if( focus && focus->event_key_on ){
    (*focus->event_key_on)( focus, key );
  }
}



/************************************************************************/
/* ���C��								*/
/************************************************************************/
/*
 *	�����I�ɁA�����ɍĕ`��
 */
static	void	widget_redraw_now( void )
{
  widget_construct();
  menu_draw_screen();
  menu_clear_event();
}


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_main( void )
{
  int	type, button, x, y;

  while( get_main_loop_flag() ){

    if( get_construct_flag() ){
      widget_construct();
      set_draw_flag( TRUE );
      set_construct_flag( FALSE );
    }
    if( get_draw_flag() ){
      menu_draw_screen();
      menu_clear_event();
      set_draw_flag( FALSE );
    }

    type = MENU_EVENT_NOTHING;
    menu_check_event( &type, &button, &x, &y );	/* �C�x���g�҂�	*/

    switch( type ){

    case MENU_EVENT_KEY_ON:
      if( !get_drag_widget() ){			/* �h���b�O������Ȃ�	*/
	if( button==Q8TK_KEY_TAB ){			/*	[TAB]�L�[ */
	  if( get_event_widget()==NULL )
	    set_event_widget( q8tk_tab_top_widget );
	  else
	    set_event_widget( get_event_widget()->tab_next );
	  {
	    Q8tkWidget *w = get_event_widget();
	    if( w && w->type != Q8TK_TYPE_ADJUSTMENT		/* �X�N���[��*/
	          && w->type != Q8TK_TYPE_LIST			/* �␳��    */
	          && w->type != Q8TK_TYPE_DIALOG		/* �Ȃ���Ȃ�*/
	          && w->type != Q8TK_TYPE_FILE_SELECTION ){	/* �E�B�W�b�g*/
	      set_scroll_adj_widget( w );
	    }
	  }
	  set_construct_flag( TRUE );
	}else{						/*	���̃L�[ */

	  int grab_flag = FALSE;
	  if( window_level_now >= 0 ){				/* �A�N�Z�� */
	    Q8tkWidget *w = window_level[ window_level_now ];	/* ���[�^�[ */
	    Q8tkAssert(w->type==Q8TK_TYPE_WINDOW,NULL);		/* �L�[���A */
	    if( w->stat.window.accel ){				/* �ݒ肳�� */
	      w = (w->stat.window.accel)->child;		/* �Ă���� */
	      while( w ){					/* �������� */
		if( w->stat.accel.key == button ){
		  widget_signal_do( w->stat.accel.widget, w->name );
		  break;
		}
		w = w->next;
	      }
	    }
	  }
	  if( grab_flag == FALSE ){
	    widget_key_on( get_event_widget(), button );
	  }
	}
      }
      break;

    case MENU_EVENT_MOUSE_ON:
      if( button & Q8TK_BUTTON_L ){		/* ���{�^�� ON		*/
	if( !get_drag_widget() ){		/* �h���b�O������Ȃ�	*/
	  Q8tkWidget	*focus;
	  focus = (Q8tkWidget *)q8gr_get_focus_screen( mouse.x/8, mouse.y/16 );
	  set_event_widget( focus );
	  widget_button_on( get_event_widget() );
	}
      }else if( button & Q8TK_BUTTON_U ){	/* �z�C�[�� UP		*/
	widget_key_on( get_event_widget(), Q8TK_KEY_PAGE_UP );
      }else if( button & Q8TK_BUTTON_D ){	/* �z�C�[�� DOWN	*/
	widget_key_on( get_event_widget(), Q8TK_KEY_PAGE_DOWN );
      }
      break;

    case MENU_EVENT_MOUSE_OFF:
      if( button & Q8TK_BUTTON_L ){		/* ���{�^�� OFF		*/
	if( get_drag_widget() ){		/* ���� �h���b�O��	*/
	  widget_drag_off( get_drag_widget() );
	  set_drag_widget( NULL );
	}
      }
      break;

    case MENU_EVENT_MOUSE_MOVE:
      mouse.x_old = mouse.x;
      mouse.y_old = mouse.y;
      mouse.x     = x;
      mouse.y     = y;
      if( get_drag_widget() ){			/* ���� �h���b�O��	*/
	if( mouse.x/8  != mouse.x_old/8  ||	/* �}�E�X 8dot�ȏ� �ړ�	*/
	    mouse.y/16 != mouse.y_old/16 ){
	  widget_dragging( get_drag_widget() );
	}
      }
      break;

    case MENU_EVENT_QUIT:
      q8tk_main_quit();
      break;
    }

  }
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_main_quit( void )
{
  set_main_loop_flag( FALSE );
}







/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/*------------------------------------------------------*/
/* �E�B�W�b�g�̕\���ݒ�
	�e�E�B�W�b�g�������`�F�b�N���A
	�E�S�Ă̐e�E�B�W�b�g�������
	�E��c�E�B�W�b�g�� WINDOW
	�̏ꍇ�A���� WINDOW �ȉ����Čv�Z���ĕ\������
	�悤�ɁA�t���O�����Ă�B
	���ۂ̍Čv�Z�A�\���́Aq8tk_main() �ōs�Ȃ��B	*/
/*------------------------------------------------------*/
static	void	widget_map( Q8tkWidget *widget )
{
  Q8tkWidget *ancestor, *parent;
  int	     size_calc = TRUE;

  if( widget->visible ){
    ancestor  = widget;
    parent    = widget->parent;
    while( parent ){
      if( parent->visible ){
	ancestor  = parent;
	parent    = parent->parent;
      }else{
	size_calc = FALSE;
	break;
      }
    }
    if( size_calc && ancestor->type == Q8TK_TYPE_WINDOW ){
      set_construct_flag( TRUE );
    }
  }
}


/*------------------------------------------------------*/
/* �q�E�B�W�b�g�̃T�C�Y�����ɁA�������g�̃T�C�Y���v�Z����B
   �ċA�I�ɁA�S�Ă̎q�E�B�W�F�b�g�������Ɍv�Z����B

   *widget �c ��Ԑe�̃E�B�W�b�g
   *max_sx �c �S�Ă̎q�E�B�W�b�g�̂Ȃ��ōő�T�C�Y x
   *max_sy �c �S�Ă̎q�E�B�W�b�g�̂Ȃ��ōő�T�C�Y y
   *sum_sx �c �q�̒��ԃE�B�W�b�g�̃T�C�Y���a x
   *sum_sy �c �q�̒��ԃE�B�W�b�g�̃T�C�Y���a y          */
/*------------------------------------------------------*/
static	void	widget_resize( Q8tkWidget *widget, int max_sx, int max_sy );
static	void	widget_size( Q8tkWidget *widget, int *max_sx, int *max_sy,
						 int *sum_sx, int *sum_sy )
{
  int	n_msx, n_msy, n_ssx, n_ssy;

/*printf("%d \n",widget->type);fflush(stdout);*/


		/* �������g�̒��� (next) �����݂���΁A�ċA�I�Ɍv�Z */
  
  if( widget->next ){
    widget_size( widget->next, &n_msx, &n_msy, &n_ssx, &n_ssy );
  }else{
    n_msx = n_msy = n_ssx = n_ssy = 0;
  }


  if( widget->visible ){

    int	c_msx, c_msy, c_ssx, c_ssy;

		/* �q�E�B�W�b�g�̃T�C�Y�v�Z(�ċA) */

    if( widget->child ){
      widget_size( widget->child, &c_msx, &c_msy, &c_ssx, &c_ssy );
    }else{
      c_msx = c_msy = c_ssx = c_ssy = 0;
    }

		/* �q�E�B�W�b�g�����ɁA���g�̃T�C�Y�v�Z */

    switch( widget->type ){
    case Q8TK_TYPE_WINDOW:			/* �E�C���h�E		*/
      if( widget->stat.window.no_frame ){
	widget->sx = c_msx;
	widget->sy = c_msy;
      }else{
	widget->sx = c_msx +2;
	widget->sy = c_msy +2;
      }
      break;
    case Q8TK_TYPE_BUTTON:			/* �{�^��		*/
    case Q8TK_TYPE_TOGGLE_BUTTON:		/* �g�O���{�^��		*/
      widget->sx = c_msx +2;	/* �T�C�Y�ύX 4 �� 2 */
      widget->sy = c_msy +2;
      break;
    case Q8TK_TYPE_CHECK_BUTTON:		/* �`�F�b�N�{�^��	*/
    case Q8TK_TYPE_RADIO_BUTTON:		/* ���W�I�{�^��		*/
      widget->sx = c_msx +3;
      widget->sy = (c_msy==0) ? 1 : c_msy;
      break;
    case Q8TK_TYPE_FRAME:			/* �t���[��		*/
      widget->sx = ( ( widget->name )
			? Q8TKMAX( c_msx, euclen( widget->name ) )
			: c_msx )   +2;
      widget->sy = c_msy +2;
      break;
    case Q8TK_TYPE_LABEL:			/* ���x��		*/
      widget->sx = ( widget->name ) ? euclen( widget->name ) : 0;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_NOTEBOOK:			/* �m�[�g�u�b�N		*/
      {
	int	xx = 0;
	Q8tkWidget *n = widget->child;
	while(n){
	  xx += ( n->name ) ? euclen(n->name) : 0;
	  xx += 1;
	  n = n->next;
	}
	xx += 1;
	widget->sx = Q8TKMAX( c_msx +2, xx );
      }
      widget->sy = c_msy + 4;
      break;
    case Q8TK_TYPE_NOTEPAGE:			/* �m�[�g�u�b�N�̃y�[�W	*/
      widget->sx = c_msx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_VBOX:			/* �c�{�b�N�X		*/
      widget->sx = c_msx;
      widget->sy = c_ssy;
      break;
    case Q8TK_TYPE_HBOX:			/* ���{�b�N�X		*/
      widget->sx = c_ssx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_VSEPARATOR:			/* �c��؂��		*/
      widget->sx = 1;
      widget->sy = 1;
      break;
    case Q8TK_TYPE_HSEPARATOR:			/* ����؂��		*/
      widget->sx = 1;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_COMBO:			/* �R���{�{�b�N�X���ǂ� */
      if( widget->stat.combo.width ){
	widget->sx = widget->stat.combo.width;
      }else{
	if( widget->stat.combo.list ){
	  widget->sx = widget->stat.combo.length;
	}else{
	  widget->sx = 8;
	}
      }
      widget->sx += 3;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_LIST:			/* ���X�g�{�b�N�X	*/
      widget->sx = Q8TKMAX( c_msx, widget->stat.list.width );
      widget->sy = c_ssy;
      break;

    case Q8TK_TYPE_LIST_ITEM:			/* ���X�g�A�C�e��	*/
      widget->sx = c_msx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_ADJUSTMENT:
      Q8tkAssert(FALSE,NULL);
      break;

    case Q8TK_TYPE_HSCALE:
    case Q8TK_TYPE_VSCALE:
      if( widget->stat.scale.adj ){
	int sx, sy;
	adjustment_size( &widget->stat.scale.adj->stat.adj, &sx, &sy );

	if( widget->stat.scale.draw_value ){
	  if( widget->stat.scale.value_pos == Q8TK_POS_LEFT ||
	      widget->stat.scale.value_pos == Q8TK_POS_RIGHT ){
	    widget->sx = sx + 4;
	    widget->sy = Q8TKMAX( sy, 1 );
	  }else{			/* Q8TK_POS_UP||Q8TK_POS_BOTTOM*/
	    widget->sx = Q8TKMAX( sx, 3 );
	    widget->sy = sy + 1;
	  }
	}else{
	  widget->sx = sx;
	  widget->sy = sy;
	}
      }else{
	widget->sx = 0;
	widget->sy = 0;
      }
      break;

    case Q8TK_TYPE_SCROLLED_WINDOW:
      if( widget->child ){

	/* �c���X�N���[���o�[��\�����邩�ǂ����́A
	   child �̃T�C�Y�Ascrolled �̃T�C�Y�Ascrolled �� policy�A
	   �̑g�ݍ��킹�ɂ��A�ȉ��� 9 �p�^�[���ɕ������B

			   0)              1)              2)
			c_msx < w-2	c_msx = w-2	c_msx > w-2
			(NEVER)				(ALLWAYS)
	0)
	  c_msy < h-2				  
	  (NEVER)					  ---
	1)
	  c_msy = h-2					    |
							  --+
	2)
	  c_msy > h-2	    |		    |		    |
	  (ALLWAYS)			  --+		  --+

	*/

	int	w, h, tmp;

	switch( widget->stat.scrolled.hpolicy ){
	case Q8TK_POLICY_NEVER:		w = 0;		break;
	case Q8TK_POLICY_ALWAYS:	w = 2;		break;
	default: /* AUTOMATIC */
	  if     ( c_msx <  widget->stat.scrolled.width - 2 ) w = 0;
	  else if( c_msx == widget->stat.scrolled.width - 2 ) w = 1;
	  else                                                w = 2;
	}

	switch( widget->stat.scrolled.vpolicy ){
	case Q8TK_POLICY_NEVER:		h = 0;		break;
	case Q8TK_POLICY_ALWAYS:	h = 2;		break;
	default: /* AUTOMATIC */
	  if     ( c_msy <  widget->stat.scrolled.height - 2 ) h = 0;
	  else if( c_msy == widget->stat.scrolled.height - 2 ) h = 1;
	  else                                                 h = 2;
	}

	if      ( (w==0||w==1) && (h==0||h==1) ){
	  widget->stat.scrolled.hscrollbar = FALSE;
	  widget->stat.scrolled.vscrollbar = FALSE;
	}else if( (w==2) && (h==0) ){
	  widget->stat.scrolled.hscrollbar = TRUE;
	  widget->stat.scrolled.vscrollbar = FALSE;
	}else if( (w==0) && (h==2) ){
	  widget->stat.scrolled.hscrollbar = FALSE;
	  widget->stat.scrolled.vscrollbar = TRUE;
	}else{
	  widget->stat.scrolled.hscrollbar = TRUE;
	  widget->stat.scrolled.vscrollbar = TRUE;
	}

	w = (widget->stat.scrolled.vscrollbar)
					? widget->stat.scrolled.width - 3
					: widget->stat.scrolled.width - 2;
	h = (widget->stat.scrolled.hscrollbar)
					? widget->stat.scrolled.height - 3
					: widget->stat.scrolled.height - 2;

	q8tk_adjustment_set_length( widget->stat.scrolled.hadj, w );
	q8tk_adjustment_clamp_page( widget->stat.scrolled.hadj, 0, c_msx - w );
	adjustment_size( &widget->stat.scrolled.hadj->stat.adj, &tmp, &tmp );

	q8tk_adjustment_set_length( widget->stat.scrolled.vadj, h );
	q8tk_adjustment_clamp_page( widget->stat.scrolled.vadj, 0, c_msy - h );
	adjustment_size( &widget->stat.scrolled.vadj->stat.adj, &tmp, &tmp );

	if( widget->stat.scrolled.vadj_value
			!= widget->stat.scrolled.vadj->stat.adj.value &&
	    widget->child->type == Q8TK_TYPE_LIST ){
	  list_event_window_scrolled( widget, h );
	}
      }
      widget->stat.scrolled.vadj_value
			=  widget->stat.scrolled.vadj->stat.adj.value;
      widget->sx = widget->stat.scrolled.width;
      widget->sy = widget->stat.scrolled.height;
      break;

    case Q8TK_TYPE_ENTRY:
      widget->sx = widget->stat.entry.disp_length;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_OPTION_MENU:			/* �I�v�V�������j���[	*/
      widget->sx = c_msx + 4 + ((widget->stat.option.button) ?2 :0);
      widget->sy = c_msy +     ((widget->stat.option.button) ?2 :0);
      break;
    case Q8TK_TYPE_MENU:			/* ���j���[ */
      if( widget->stat.menu.list ){
	int	max = 8;
	Q8List	*list = widget->stat.menu.list;
	while( list ){
	  if( ((Q8tkWidget *)(list->ptr))->child ){
	    int l = strlen( ((Q8tkWidget *)(list->ptr))->child->name );
	    max = Q8TKMAX( max, l );
	  }
	  list = list->next;
	}
	widget->sx = max;
	widget->sy = 1;
      }else{
	widget->sx = 0;
	widget->sy = 0;
      }
      break;
    case Q8TK_TYPE_RADIO_MENU_ITEM:		/* ���W�I���j���[�A�C�e��*/
      widget->sx = c_msx;
      widget->sy = c_msy;
      break;
    case Q8TK_TYPE_MENU_LIST:			/* ���j���[���X�g	*/
      widget->sx = c_msx;
      widget->sy = c_ssy;
      break;

    case Q8TK_TYPE_DIALOG:
      Q8tkAssert(FALSE,NULL);
      break;
    case Q8TK_TYPE_FILE_SELECTION:
      Q8tkAssert(FALSE,NULL);
      break;

    default:
      Q8tkAssert(FALSE,"Undefined type");
    }

  }else{
    widget->sx = 0;
    widget->sy = 0;
  }


		/* �T�C�Y���X�V */

  *max_sx = Q8TKMAX( widget->sx, n_msx );
  *max_sy = Q8TKMAX( widget->sy, n_msy );
  *sum_sx = widget->sx + n_ssx;
  *sum_sy = widget->sy + n_ssy;


		/* �q�E�B�W�b�g�ɃZ�p���[�^���܂܂��ꍇ�́A�T�C�Y���� */

  widget_resize( widget, widget->sx, widget->sy );


		/* ���X�g�{�b�N�X�Ȃǂ̏ꍇ�A�q�E�B�W�b�g�̃T�C�Y�𒲐� */

  if( widget->type == Q8TK_TYPE_LIST ||
      widget->type == Q8TK_TYPE_MENU_LIST ){
    Q8tkWidget *child = widget->child;
    while( child ){
      Q8tkAssert( child->type==Q8TK_TYPE_LIST_ITEM||
		  child->type==Q8TK_TYPE_RADIO_MENU_ITEM,NULL );
      child->sx = widget->sx;
      if( child->next ) child = child->next;
      else              break;
    }
  }


/*printf("%s (%02d,%02d) max{ %02d,%02d } sum{ %02d,%02d }\n",debug_type(widget->type),widget->sx,widget->sy,*max_sx,*max_sy,*sum_sx,*sum_sy);fflush(stdout);*/
}


/*
 * �Z�p���[�^�ȂǁA�e�̑傫���Ɉˑ�����E�B�W�b�g�̃T�C�Y���Čv�Z����
 */
static	void	widget_resize( Q8tkWidget *widget, int max_sx, int max_sy )
{
  if( widget->type == Q8TK_TYPE_WINDOW   ||
      widget->type == Q8TK_TYPE_NOTEPAGE ||
      widget->type == Q8TK_TYPE_VBOX     ||
      widget->type == Q8TK_TYPE_HBOX     ){

    Q8tkWidget *child = widget->child;
    
    if( widget->type == Q8TK_TYPE_WINDOW &&
	! widget->stat.window.no_frame ){
      max_sx -= 2;
      max_sy -= 2;
    }
    if( child ) widget_resize( child, max_sx, max_sy );

    while( child ){
      switch( child->type ){

      case Q8TK_TYPE_HSEPARATOR:
	if( widget->type != Q8TK_TYPE_HBOX ){
	  if( child->sx < max_sx ) child->sx = max_sx;
	}
	break;

      case Q8TK_TYPE_VSEPARATOR:
	if( widget->type != Q8TK_TYPE_VBOX ){
	  if( child->sy < max_sy ) child->sy = max_sy;
	}
	break;

      case Q8TK_TYPE_VBOX:
	if( widget->type == Q8TK_TYPE_VBOX ){
	  if( child->sx < max_sx ) child->sx = max_sx;
	}
	break;

      case Q8TK_TYPE_HBOX:
	if( widget->type == Q8TK_TYPE_HBOX ){
	  if( child->sy < max_sy ) child->sy = max_sy;
	}
	break;
      }

      if( child->next ) child = child->next;
      else              break;
    }
  }
}



/*------------------------------------------------------*/
/* �X�N���[���h�E�C���h�E�Ɋ܂܂��E�B�W�b�g�̕\������

   �X�N���[���h�E�C���h�E�Ɋ܂܂��E�B�W�b�g�ŁA�E�C���h�E�O��
   ������̂Ɋւ��āA����̃t���O���ݒ肳��Ă���΁A�����I��
   �X�N���[���ʒu�𒲐����A�\�Ȍ���E�B�W�b�g�S�̂�\������
   �������s�Ȃ��B���̂��߂ɂ��낢��ƃ��[�N�̏������s�Ȃ��B

   �X�N���[���h�E�C���h�E�ɂ���E�B�W�b�g���\�����ꂽ���ǂ����́A
   ���ۂɕ\������܂ł킩��Ȃ��̂ŁA���ۂɕ\�����Ȃ��烏�[�N��
   �ݒ肵�A���[�N�̓��e����ŁA�ĕ\���Ƃ������ƂɂȂ�B

   �܂�A

	for( i=0; i<2; i++ ){
	  widget_size();
	  widget_scroll_adjust_init();
	  widget_draw();
	  if( ! widget_scroll_adjust() ) break;
	}

   �ƂȂ�B

  �Ȃ��A�������s�Ȃ��̂́A�ŏ��Ɍ������� �e�� SCROLLED WINDOW �̂�
  �ɑ΂��Ă����Ȃ̂ŁASCROLLED WINDOW �̓���q�̏ꍇ�͂ǂ��Ȃ邩
  �킩��Ȃ��B						*/
/*------------------------------------------------------*/

#define	widget_scroll_adj_check_widget( w )	check_scroll_adj_widget( w )


static	void	widget_scroll_adjust_init( void )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    q8tk_scroll_adj[i].drawn = FALSE;
  }
}

static	int	widget_scroll_adjust( void )
{
  int	i, result = 0;

  for( i=0; i<NR_SCROLL_ADJ; i++ ){

/*printf("%d %d %s\n",i,q8tk_scroll_adj[i].drawn,(q8tk_scroll_adj[i].widget)?debug_type(q8tk_scroll_adj[i].widget->type):"NULL");*/

    if( q8tk_scroll_adj[i].widget ){

      if( q8tk_scroll_adj[i].drawn ){

	Q8tkWidget *widget = q8tk_scroll_adj[i].widget;
	Q8tkWidget *p = widget->parent;
	/*int	x1 = widget->x;*/
	int	x1 = widget->x + widget->sx -1;
	int	y1 = widget->y + widget->sy -1;

	/*
	   SCROLLED WINDOW ���ɏ悹��E�B�W�b�g�S�̂��A��������_�Ƃ����A
	   ���Έʒu real_y

	   y1 = widget->y + widget->sy -1;
	   p->y + 1 - p->stat.scrolled.child_y0 + [ real_y ] = y1;
	*/

	while( p ){
	  if( p->type==Q8TK_TYPE_SCROLLED_WINDOW ){

	    if( p->x +1 <= x1  &&  x1 < p->x +1 + p->stat.scrolled.child_sx ){
	      /* Ok, Expose */
	    }else{
	      Q8TK_ADJUSTMENT(p->stat.scrolled.hadj)->value
			= x1 - ( p->x+1 - p->stat.scrolled.child_x0 )
					- p->stat.scrolled.child_sx + 1;
	      result = 1;
	    }

	    if( p->y +1 <= y1  &&  y1 < p->y +1 + p->stat.scrolled.child_sy ){
	      /* Ok, Expose */
	    }else{
	      Q8TK_ADJUSTMENT(p->stat.scrolled.vadj)->value
			= y1 - ( p->y+1 - p->stat.scrolled.child_y0 )
					- p->stat.scrolled.child_sy + 1;
	      result = 1;
	    }

	    /*
	    printf("  %s %d %d %d %d  %d %d %d %d\n",debug_type(p->type),
		   p->x, p->y, p->sx, p->sy,
		   p->stat.scrolled.child_x0, p->stat.scrolled.child_y0,
		   p->stat.scrolled.child_sx, p->stat.scrolled.child_sy );
	    printf("  %s %d %d %d %d\n",debug_type(widget->type),
		   widget->x, widget->y, widget->sx, widget->sy );
	    printf("  %d\n",y1);
	    if( result ) printf("ADJ %d,%d\n",
				Q8TK_ADJUSTMENT(p->stat.scrolled.vadj)->value,
				Q8TK_ADJUSTMENT(p->stat.scrolled.hadj)->value);
	    */


	    break;
	  }
	  p = p->parent;
	}
	q8tk_scroll_adj[i].drawn  = FALSE;
	q8tk_scroll_adj[i].widget = NULL;
      }
    }
  }

  return result;
}






/*------------------------------------------------------*/
/* �������g�̕\���ʒu�����ƂɁA�`�悵�A
   �ċA�I�ɁA�S�Ă̎q�E�B�W�F�b�g���`�悷��B		*/
/*------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*
   �E�B�W�b�g��TAB�֘A�Â�
	TAB�L�[�ɂ��A�t�H�[�J�X�̈ړ����������邽�߂ɁA
	�`��O�A�`�撆�A�`���Ɋe��ݒ���s�Ȃ��B

		widget_tab_pre()  �c TAB�Â��̏�����
		widget_draw()     �c �`��B������TAB�Â����s�Ȃ�
		widget_tab_post() �c TAB�Â��̌㏈��

	widget_draw() �̓����ł́ATAB�Â����s�Ȃ������E�B�W�b�g�̕`���
	�ۂɁA�wwidget_tab_set()�x �֐����ĂԁB

	�܂��ATAB�t�����ꂽ�E�B�W�b�g���A�A�N�e�B�u��Ԃ��ǂ�����m��ɂ́A
	�`��̑O����A�wcheck_active()�x�Ɓwcheck_active_finish()�x�֐��ň͂ށB

	�y��ʓI�ȗ�z

	    widget_tab_set( widget );
	    check_active( widget );
	    q8gr_draw_xxx( x, y, is_active_widget(), widget );
	    if( widget->child ){
	      widget_draw();
	    }
	    check_active_finish();

	is_active_widget() �́A���g���A�N�e�B�u��Ԃ��ǂ����𒲂ׂ�֐���
	����Acheck_active()�`check_active_finish() �̊ԂŁA�L���ł���B

	��̏ꍇ�A�A�N�e�B�u��Ԃ͎q�E�B�W�b�g�ɓ`�d����B
	�Ⴆ�΁A�{�^���E�B�W�b�g�̏ꍇ�A�{�^���̎q�E�B�W�b�g�ł��郉�x���ɁA
	�i�A�N�e�B�u�ȏꍇ�́j�A���_�[���C���������A�Ƃ������ɂ�����B
*/

static	Q8tkWidget	*tab_widget;
static	void	widget_tab_pre( void )
{
  tab_widget          = NULL;
  q8tk_tab_top_widget = NULL;
}
static	void	widget_tab_post( void )
{
  if( tab_widget ){
    tab_widget->tab_next = q8tk_tab_top_widget;
  }
}

#define	widget_tab_set( widget )					\
		do{							\
		  if( tab_widget==NULL ) q8tk_tab_top_widget =(widget);	\
		  else                   tab_widget->tab_next=(widget);	\
		  tab_widget = widget;					\
		}while(0)
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */





static	void	widget_draw( Q8tkWidget *widget )
{
  int		x = widget->x;
  int		y = widget->y;
  Q8tkWidget	*child = widget->child;


  widget_scroll_adj_check_widget( widget );


		/* HBOX�AVBOX �̎q�̏ꍇ�Ɍ���A*/
		/* �z�u���኱�ύX�ł���B	*/

  if( widget->parent ){
    switch( (widget->parent)->type ){
    case Q8TK_TYPE_VBOX:
      if      ( widget->placement_x == Q8TK_PLACEMENT_X_CENTER ){
	x += ( (widget->parent)->sx - widget->sx ) / 2;
      }else if( widget->placement_x == Q8TK_PLACEMENT_X_RIGHT ){
	x += ( (widget->parent)->sx - widget->sx );
      }
      break;
    case Q8TK_TYPE_HBOX:
      if      ( widget->placement_y == Q8TK_PLACEMENT_Y_CENTER ){
	y += ( (widget->parent)->sy - widget->sy ) / 2;
      }else if( widget->placement_y == Q8TK_PLACEMENT_Y_BOTTOM ){
	y += ( (widget->parent)->sy - widget->sy );
      }
      break;
    }
  }


		/* �������g�� type�����Ƃɘg�Ȃǂ������B*/
		/* �q������΁Ax,y ������B		*/
		/* �q�̒���(next)�́Ax,y �����߂�B	*/
		/* �����̎q�ɑ΂��Ă̂ݍċA�I�ɏ����B	*/

/*printf("%s (%d,%d) %d %d\n",debug_type(widget->type),widget->sx,widget->sy,widget->x,widget->y);fflush(stdout);*/

  if( widget->visible ){

    switch( widget->type ){

    case Q8TK_TYPE_WINDOW:		/* �E�C���h�E		*/
      if( widget->stat.window.no_frame ){
	/* no frame */
      }else{
	q8gr_draw_window( x, y, widget->sx, widget->sy,
			  widget->stat.window.shadow_type );
      }
      if( child ){
	child->x = x  + (widget->stat.window.no_frame ? 0 : 1);
	child->y = y  + (widget->stat.window.no_frame ? 0 : 1);
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_BUTTON:		/* �{�^��		*/
    case Q8TK_TYPE_TOGGLE_BUTTON:	/* �g�O���{�^��		*/
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_button( x, y, widget->sx, widget->sy,
		        widget->stat.button.active, widget );
      if( child ){
	child->x = x + 1;	/* �T�C�Y�ύX 2 �� 1 */
	child->y = y + 1;
	widget_draw( child );
      }
      check_active_finish();
      break;
    case Q8TK_TYPE_CHECK_BUTTON:	/* �`�F�b�N�{�^��		*/
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_check_button( x, y, widget->stat.button.active, widget );
      if( child ){
	child->x = x + 3;
	child->y = y;
	widget_draw( child );
      }
      check_active_finish();
      break;
    case Q8TK_TYPE_RADIO_BUTTON:	/* ���W�I�{�^��		*/
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_radio_button( x, y, widget->stat.button.active, widget );
      if( child ){
	child->x = x + 3;
	child->y = y;
	widget_draw( child );
      }
      check_active_finish();
      break;

    case Q8TK_TYPE_FRAME:		/* �t���[��		*/
      q8gr_draw_frame( x, y, widget->sx, widget->sy,
		       widget->name, widget->stat.frame.shadow_type );
      if( child ){
	child->x = x + 1;
	child->y = y + 1;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_LABEL:			/* ���x��		*/
      q8gr_euc_puts( x, y, widget->stat.label.foreground,
		           widget->stat.label.background,
		     FALSE, is_active_widget(),
		     (widget->name) ?(widget->name) :"" );
      break;

    case Q8TK_TYPE_NOTEBOOK:		/* �m�[�g�u�b�N		*/
      q8gr_draw_notebook( x, y, widget->sx, widget->sy );
      if( child ){
	child->x = x;
	child->y = y;
	widget_draw( child );
      }
      break;
    case Q8TK_TYPE_NOTEPAGE:		/* �m�[�g�u�b�N�̃y�[�W	*/
      {
	int select_flag = (widget==(widget->parent)->stat.notebook.page);
	check_active( widget );
	q8gr_draw_notepage( (widget->name) ? widget->name : "",
			    select_flag,
			    (select_flag) ? FALSE : is_active_widget(),
			    (select_flag) ? NULL : widget );
	check_active_finish();
	if( child && select_flag ){
	  child->x = ((widget->parent)->x) + 1;
	  child->y = ((widget->parent)->y) + 3;
	  widget_draw( child );
	}else{
	  widget_tab_set( widget );
	}
      }
      break;

    case Q8TK_TYPE_VBOX:			/* �c�{�b�N�X		*/
      if( child ){
	child->x = x;	x += 0;
	child->y = y;	y += child->sy;
	while( child->next ){
	  child = child->next;
	  child->x = x;	x += 0;
	  child->y = y;	y += child->sy;
	}
	child = widget->child;
	widget_draw( child );
      }
      break;
    case Q8TK_TYPE_HBOX:			/* ���{�b�N�X		*/
      if( child ){
	child->x = x;	x += child->sx;
	child->y = y;	y += 0;
	while( child->next ){
	  child = child->next;
	  child->x = x;	x += child->sx;
	  child->y = y;	y += 0;
	}
	child = widget->child;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_VSEPARATOR:			/* �c��؂��		*/
      q8gr_draw_vseparator( x, y, widget->sy );
      break;
    case Q8TK_TYPE_HSEPARATOR:			/* ����؂��		*/
      q8gr_draw_hseparator( x, y, widget->sx );
      break;

    case Q8TK_TYPE_COMBO:			/* �R���{�{�b�N�X���ǂ� */
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_combo( x, y, widget->sx -3, is_active_widget(),
		       q8tk_combo_get_text( widget ), widget );
      check_active_finish();
      break;

    case Q8TK_TYPE_LIST:			/* ���X�g�{�b�N�X	*/
      widget_tab_set( widget );
      if( child ){
	child->x = x;	x += 0;
	child->y = y;	y += child->sy;
	while( child->next ){
	  child = child->next;
	  child->x = x;	x += 0;
	  child->y = y;	y += child->sy;
	}
	child = widget->child;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_LIST_ITEM:			/* ���X�g�A�C�e��	*/
      check_active( widget->parent );
      if( child ){
	int rev   = (widget->parent->stat.list.selected==widget) ?TRUE :FALSE;
	int under = (widget->parent->stat.list.active  ==widget) ?TRUE :FALSE;
	if( rev && under ) under = FALSE;

	q8gr_draw_list_item( x, y, widget->sx, is_active_widget(), rev, under,
			     child->name, widget );
      }
      check_active_finish();
      break;

    case Q8TK_TYPE_ADJUSTMENT:
      Q8tkAssert(FALSE,NULL);
      break;

    case Q8TK_TYPE_HSCALE:
      if( widget->stat.scale.adj ){
	Q8Adjust *adj = &(widget->stat.scale.adj->stat.adj);
	widget_tab_set( widget->stat.scale.adj );
	widget->stat.scale.adj->x = x;
	widget->stat.scale.adj->y = y;
	check_active( widget->stat.scale.adj );
	q8gr_draw_hscale( x, y, adj, is_active_widget(),
			  widget->stat.scale.draw_value,
			  widget->stat.scale.value_pos,
			  widget->stat.scale.adj );
	check_active_finish();
      }
      break;
    case Q8TK_TYPE_VSCALE:
      if( widget->stat.scale.adj ){
	Q8Adjust *adj = &(widget->stat.scale.adj->stat.adj);
	widget->stat.scale.adj->x = x;
	widget->stat.scale.adj->y = y;
	widget_tab_set( widget->stat.scale.adj );
	check_active( widget->stat.scale.adj );
	q8gr_draw_vscale( x, y, adj, is_active_widget(),
			  widget->stat.scale.draw_value,
			  widget->stat.scale.value_pos,
			  widget->stat.scale.adj );
	check_active_finish();
      }
      break;

    case Q8TK_TYPE_SCROLLED_WINDOW:
      if( child ){
	int sx = widget->sx;
	int sy = widget->sy;

	if( widget->stat.scrolled.hscrollbar ){
	  widget_tab_set( widget->stat.scrolled.hadj );
	  check_active( widget->stat.scrolled.hadj );
	  q8gr_draw_hscale( x, y + widget->sy -1,
			    &(widget->stat.scrolled.hadj->stat.adj),
			    is_active_widget(),
			    FALSE, 0, widget->stat.scrolled.hadj );
	  check_active_finish();
	  sy --;
	}
	if( widget->stat.scrolled.vscrollbar ){
	  widget_tab_set( widget->stat.scrolled.vadj );
	  check_active( widget->stat.scrolled.vadj );
	  q8gr_draw_hscale( x + widget->sx -1, y, 
			    &(widget->stat.scrolled.vadj->stat.adj),
			    is_active_widget(),
			    FALSE, 0, widget->stat.scrolled.vadj );
	  check_active_finish();
	  sx --;
	}
	q8gr_draw_scrolled_window( x,y, sx,sy, Q8TK_SHADOW_ETCHED_OUT, widget);
	q8gr_set_screen_mask( x+1, y+1, sx-2, sy-2 );
	widget->stat.scrolled.child_x0
				= widget->stat.scrolled.hadj->stat.adj.value;
	widget->stat.scrolled.child_y0
				= widget->stat.scrolled.vadj->stat.adj.value;
	widget->stat.scrolled.child_sx = sx -2;
	widget->stat.scrolled.child_sy = sy -2;

	child->x = x - widget->stat.scrolled.child_x0 +1;
	child->y = y - widget->stat.scrolled.child_y0 +1;

	widget_draw( child );
	q8gr_reset_screen_mask();
      }else{
	q8gr_draw_window( x,y, widget->sx,widget->sy, Q8TK_SHADOW_ETCHED_OUT );
      }
      break;

    case Q8TK_TYPE_ENTRY:
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_entry( x, y, widget->sx, widget->name,
		       widget->stat.entry.disp_pos,
		       (is_active_widget())? widget->stat.entry.cursor_pos: -1,
		       widget );
      check_active_finish();
      break;

    case Q8TK_TYPE_OPTION_MENU:
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_option_menu( x, y, widget->sx, widget->sy,
			     widget->stat.option.button, widget );
      if( child ){
	child->x = x + ((widget->stat.option.button) ? 1 : 0);
	child->y = y + ((widget->stat.option.button) ? 1 : 0);
	widget_draw( child );
      }
      check_active_finish();
      break;
    case Q8TK_TYPE_MENU:
      {
	Q8tkWidget *c = widget->stat.menu.selected;
	if( c && c->child ){
	  q8gr_euc_puts( x, y, Q8GR_PALETTE_FOREGROUND,
			       Q8GR_PALETTE_BACKGROUND,
			 FALSE, is_active_widget(),
			(c->child->name) ?(c->child->name) :"" );
	}
      }
      break;
    case Q8TK_TYPE_RADIO_MENU_ITEM:
      if( child ){
	int under = (widget->parent->stat.menulist.active==widget) ?TRUE:FALSE;
	q8gr_draw_radio_menu_item( x, y, widget->sx, under,
				   child->name, widget );
      }
      break;
    case Q8TK_TYPE_MENU_LIST:
      widget_tab_set( widget );
      if( child ){
	child->x = x;	x += 0;
	child->y = y;	y += child->sy;
	while( child->next ){
	  child = child->next;
	  child->x = x;	x += 0;
	  child->y = y;	y += child->sy;
	}
	child = widget->child;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_DIALOG:
      Q8tkAssert(FALSE,NULL);
      break;
    case Q8TK_TYPE_FILE_SELECTION:
      Q8tkAssert(FALSE,NULL);
      break;

    default:
      Q8tkAssert(FALSE,"Undefined type");
    }
  }


	/* �������g�̒��� (next) �����݂���΁A�ċA�I�ɏ��� */
  
  if( widget->next ){
    widget = widget->next;
    widget_draw( widget );
  }
}














/*------------------------------------------------------*/
/* �X�N���[����ʂ��쐬�B
   q8tk_grab_add() �Őݒ肳�ꂽ WINDOW ���g�b�v�Ƃ��āA
   �S�Ă̎q�E�B�W�F�b�g�̑傫���A�ʒu���v�Z���A
   menu_screen[][]�ɁA�\�����e��ݒ肷��B
   �����ɁATAB �L�[�������ꂽ���́A�t�H�[�J�X�̕ύX��
   �菇�����߂Ă����B					*/
/*------------------------------------------------------*/

static	void	widget_construct( void )
{
  int		i, j, tmp;
  Q8tkWidget	*widget;

  q8gr_clear_screen();

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){

    widget = window_level[i];

    if( widget ){
      Q8tkAssert(widget->type==Q8TK_TYPE_WINDOW,NULL);

      for( j=0; j<2; j++ ){

	q8gr_clear_focus_screen();

	widget_size( widget, &tmp, &tmp, &tmp, &tmp );

	if( widget->stat.window.set_position==FALSE ){
	  widget->x = ( Q8GR_SCREEN_X - widget->sx ) / 2;
	  widget->y = ( Q8GR_SCREEN_Y - widget->sy ) / 2;
	}else{
	  widget->x = widget->stat.window.x;
	  widget->y = widget->stat.window.y;
	}

	widget_scroll_adjust_init();

	widget_tab_pre();
	widget_draw( widget );
	widget_tab_post();

	if( widget_scroll_adjust() ){
	  /* Redraw! */
	}else{
	  break;
	}

      }

    }else{
      break;
    }
  }


  if( get_drag_widget() ){
    /* none */
  }else{
    Q8tkWidget *w;
    int exist;

    w = q8tk_tab_top_widget;

    if( w ){

      do{				/* TAB TOP �� NOTEPAGE �ȊO�ɐݒ� */
	if( w->type != Q8TK_TYPE_NOTEPAGE ) break;
	w = w->tab_next;
      } while( w != q8tk_tab_top_widget );
      q8tk_tab_top_widget = w;

      exist = FALSE;			/* event_widget �����݂��邩�`�F�b�N */
      do{			
	if( w==get_event_widget() ){
	  exist = TRUE;
	  break;
	}
	w = w->tab_next;
      } while( w != q8tk_tab_top_widget );
      if( !exist ){			/*    ���݂��Ȃ���� NULL �ɂ��Ă��� */
	set_event_widget( NULL );
      }
    }
  }




#if 0
for(tmp=0,i=0; i<MAX_WIDGET; i++ ){
  if( widget_table[i] ) tmp++;
}
printf("[TOTAL WIDGET] %d\n",tmp);
#endif

/*
{
  Q8tkWidget *w = q8tk_tab_top_widget;
  while( w ){
    printf("%s\n",debug_type(w->type));
    w = w->tab_next;
    if( w == q8tk_tab_top_widget ) break;
  }
}
*/
}




/************************************************************************/
/* Q8TK ���g����ŕ֗��ȁA�G���Ȋ֐�					*/
/************************************************************************/

/****************************************************************/
/* SJIS �� EUC �ɕϊ� (���Ȃ�K��)				*/
/*	*sjis_p �̕������ EUC �ɕϊ����āA*euc_p �Ɋi�[����B	*/
/*	���ӁI�j���̊֐��́A�o�b�t�@���ӂ���`�F�b�N���Ă��Ȃ��B*/
/*		*euc_p �́A*sjis_p �̔{�ȏ�̒������Ȃ��Ɗ댯	*/
/****************************************************************/

void	sjis2euc( char *euc_p, const char *sjis_p )
{
  int	h,l, h2, l2;

  while( ( h = (unsigned char)*sjis_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      *euc_p ++ = h;

    }else if( 0xa1 <= h && h <= 0xdf ){		/* ���p�J�i */

      *euc_p ++ = (char)0x8e;
      *euc_p ++ = h;

    }else{					/* �S�p���� */

      if( ( l = (unsigned char)*sjis_p++ ) ){

	if( l <= 0x9e ){
	  if( h <= 0x9f ) h2 = (h - 0x71) *2 +1;
	  else            h2 = (h - 0xb1) *2 +1;
	  if( l >= 0x80 ) l2 = l - 0x1f -1;
	  else            l2 = l - 0x1f;
	}else{
	  if( h <= 0x9f ) h2 = (h - 0x70) *2;
	  else            h2 = (h - 0xb0) *2;
	  l2 = l - 0x7e;
	}
	*euc_p++ = 0x80 | h2;
	*euc_p++ = 0x80 | l2;

      }else{
	break;
      }

    }
  }

  *euc_p = '\0';
}


/****************************************************************/
/* EUC �� SJIS �ɕϊ� (���Ȃ�K��)				*/
/*	*euc_p �̕������ SJIS �ɕϊ����āA*sjis_p �Ɋi�[����B	*/
/*	���ӁI�j���̊֐��́A�o�b�t�@���ӂ���`�F�b�N���Ă��Ȃ��B*/
/*		*sjis_p �́A*euc_p �Ɠ����ȏ�̒������Ȃ��Ɗ댯	*/
/****************************************************************/

void	euc2sjis( char *sjis_p, const char *euc_p )
{
  int	h,l;

  while( ( h = (unsigned char)*euc_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      *sjis_p ++ = h;

    }else if( h==0x8e ){			/* ���p�J�i */

      if( ( h = (unsigned char)*euc_p++ ) ){

	if( 0xa1 <= h && h <= 0xdf )
	  *sjis_p ++ = h;

      }else{
	break;
      }

    }else if( h & 0x80 ){			/* �S�p���� */

      if( ( l = (unsigned char)*euc_p++ ) ){

	if( l & 0x80 ){

	  h = (h & 0x7f) - 0x21;
	  l = (l & 0x7f) - 0x21;

	  if( h & 0x01 ) l += 0x9e;
	  else           l += 0x40;
	  if( l >= 0x7f ) l += 1;

	  h = (h>>1) + 0x81;

	  if( h >= 0xa0 ) h += 0x40;

	  *sjis_p++ = h;
	  *sjis_p++ = l;

	}

      }else{
	break;
      }

    }
  }

  *sjis_p = '\0';
}


/****************************************************************/
/* EUC������̒������v�Z (���������K��)				*/
/*	ASCII�E���p�J�i��1�����A�S�p������2�����Ƃ���B		*/
/*	�����񖖂́A\0 �͒����Ɋ܂߂Ȃ��B			*/
/****************************************************************/

int	euclen( const char *euc_p )
{
  int	i = 0, h;

  while( ( h = (unsigned char)*euc_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      i++;

    }else if( h == 0x8e ){			/* ���p�J�i */

      euc_p ++;
      i++;

    }else{					/* ���� */

      euc_p ++;
      i += 2;

    }
  }

  return i;
}
