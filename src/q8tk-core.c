/************************************************************************/
/*									*/
/* QUASI88 メニュー用 Tool Kit						*/
/*				Core lib				*/
/*									*/
/*	GTK+ の API を真似て作りました。ややこしすぎて、ドキュメントは	*/
/*	書けません………。						*/
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


/* コンボボックスに設定する文字を、コンボのリスト以外でも認める */
#define	ENABLE_COMBO_SET_TEXT_ALL_WORD

/* デバッグ用 */
#define	Q8TK_ASSERT_DEBUG


/************************************************************************/
/* デバッグ								*/
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
/* エラー処理								*/
/************************************************************************/
#define	CHECK_MALLOC_OK(e,s)	((e) ? (void)0 : _CHECK_MALLOC_OK(s))

static	void	_CHECK_MALLOC_OK( const char *s )
{
  fprintf( stderr, "Fatal Error : %s exhaused!\n", s );
  main_exit(1);
}


/************************************************************************/
/* ワーク								*/
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
/* 動的ワークの確保／開放						*/
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
/* Q8TK 専用 文字列処理							*/
/*	q8_strncpy( s, ct, n )						*/
/*		文字列 ct を 文字列 s に コピーする。			*/
/*		s の文字列終端は、必ず '\0' となり、s の長さは n-1 文字	*/
/*		以下に収まる。						*/
/*	q8_strncat( s, ct, n )						*/
/*		文字列 ct を 文字列 s に 付加する。			*/
/*		s の文字列終端は、必ず '\0' となり、s の長さは n-1 文字	*/
/*		以下に収まる。						*/
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
/* リスト処理								*/
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
/* 初期化と終了								*/
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
 *	初期化
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
 *	終了
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
/* モーダルの設定							*/
/************************************************************************/
/*--------------------------------------------------------------
 *	モーダル設定
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
 *	モーダル解除
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
/* ウィジットの作成					*/
/********************************************************/

/*--------------------------------------------------------------
 * WINDOW
 *	・全てのウィジットの最も先祖になる。
 *	・子を一つ持てる。
 *	・子を持つには、q8tk_container_add() を使用する。
 *	・このウィジットを表示するには、q8tk_grab_add()にて、
 *	  明示的に表示を指示する。
 *	・WINDOW は最大、MAX_WINDOW_LEVEL個作成できるが、イベント
 *	  を受け付けるのは、最後に q8tk_grab_add() を発行した
 *	  WINDOW の子孫のみである。
 *	・WINDOW は、3つに種類わけされ、new()の引数で決まる
 *		Q8TK_WINDOW_TOPLEVEL … 一番基礎の WINDOW
 *					唯一つのみ生成可能
 *					ウインドウ枠を持たない
 *		Q8TK_WINDOW_DIALOG   … 出っぱったウインドウ枠をもつ
 *		Q8TK_WINDOW_POPUP    … 単純なウインドウ枠をもつ
 *	・シグナル … なし
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
 *	・標準的なボタン。ボタンを押すと引っ込み、離すと戻る。
 *	・子を一つ持てる。
 *	  が、実質的には、LABEL 以外を持つことはないだろう。
 *	・子を持つには、q8tk_container_add() を使用する。
 *	・シグナル
 *		"clicked"	ボタンが押された時に発生
 *	--------------------------------------------------------
 *	- BUTTON - xxx		(大抵、xxx はLABELL )
 *
 *--------------------------------------------------------------*/
static	void	button_event_button_on( Q8tkWidget *widget )
{
  widget->stat.button.active = Q8TK_BUTTON_ON;

  widget_redraw_now();		/* 一旦再描画 */
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
 *	・トグルボタン。ボタンを押すと引っ込む。もう一度ボタンを
 *	  押すと戻る。
 *	・子を一つ持てる。
 *	  が、実質的には、LABEL 以外を持つことはないだろう。
 *	・子を持つには、q8tk_container_add() を使用する。
 *	・シグナル
 *		"clicked"	ボタンが押された時に発生
 *		"toggled"	ボタンの状態が変化した時に発生
 *	--------------------------------------------------------
 *	- TOGGLE BUTTON - xxx		(大抵、xxx はLABELL )
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
 *	・チェックボタン。ボタンを押すと、チェックボックスが
 *	  塗りつぶされる。もう一度ボタンを押すと戻る。
 *	・子を一つ持てる。
 *	  が、実質的には、LABEL 以外を持つことはないだろう。
 *	・子を持つには、q8tk_container_add() を使用する。
 *	・シグナル
 *		"clicked"	ボタンが押された時に発生
 *		"toggled"	ボタンの状態が変化した時に発生
 *	--------------------------------------------------------
 *	- CHECKE BUTTON - xxx		(大抵、xxx はLABELL )
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
 *	・ラジオボタン
 *	・いくつかのラジオボタンでグルーピングできる。
 *	・ボタンを押すとチェックされるが、同じくグルーピング
 *	  された他のラジオボタンは、チェックが外れる。
 *	・子を一つ持てる。
 *	  が、実質的には、LABEL 以外を持つことはないだろう。
 *	・子を持つには、q8tk_container_add() を使用する。
 *	・シグナル
 *		"clicked"	ボタンが押された時に発生
 *		"toggled"	ボタンの状態が変化した時に発生
 *	--------------------------------------------------------
 *	- RADIO BUTTON - xxx		(大抵、xxx はLABELL )
 *		    :
 *		    :
 *		    :…… LIST -> PREV
 *			       -> NEXT
 *			  LIST はグルーピング情報。PREV、NEXT は
 *			  同じグループの他の RADIO BUTTON
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
 *	・コンボボックスもどき。書き換えは不能である。
 *	・子は持てない
 *	・q8tk_combo_popdown_strings() が発行された時、そのリスト
 *	  と同じ文字列を持つ LIST ITEM - LABEL のウィジットを
 *	  生成する。
 *	・シグナル
 *		"changed" 内容に変更があった時に発生
 *	--------------------------------------------------------
 *	- COMBO BOX
 *		:
 *		:…… LIST   →  LIST ITEM - LABEL
 *			:
 *		      LIST   →  LIST ITEM - LABEL
 *			:
 *		      LIST   →  LIST ITEM - LABEL
 *			:
 *
 *	●COMBO BOX が マウスでクリックされると、COMBO BOX は、
 *	  WINDOW (POPUP) を自動生成し、以下のような親子構造を作る。
 *	  なお、子文字の list item - label は、上の COMBO BOX の LIST
 *	  に登録された、ウィジットである。
 *
 *	WINDOW - LIST - list item - label
 *	  :		    |
 *	  :		list item - label
 *	  :		    |
 *	  :		list item - label
 *	  :		    |
 *	  :
 *	  :
 *	  :…… ACCEL GROUP - ACCEL KEY  → BUTTON(dummy)
 *
 *	○適当な list item がクリックされると、COMBO BOX の
 *	  指し示す文字列が変わる仕組みになっている。
 *		
 *	○ESC キーが押されると、WINDOW (POPUP) を消去し、元の
 *	  状態に戻す。これを実現するために、アクセラレータキーを
 *	  利用している。アクセラレータキーは、ウィジットにシグナル
 *	  を送ることしか出来ないので、ダミーのボタン(非表示) を
 *	  生成し、WINDOW (POPUP)の消去処理は、このウィジットの
 *	  コールバック関数として、登録している。
 *
 *	※COMBO BOX のリストが長くなった時に、画面に収まらない
 *	  場合がでてきた。そこで、SCROLLED_WINDOW で WINDOW(POPUP)を
 *	  生成するようにしてみた。
 *
 *	WINDOW - SCROLLED WINDOW - LIST - list item - label
 *	  :	  :…… ADJUSTMENT	    |
 *	  :	  :…… ADJUSTMENT	  list item - label
 *	  :				    |
 *	  :				  list item - label
 *	  :				    |
 *	  :
 *	  :
 *	  :…… ACCEL GROUP - ACCEL KEY  → BUTTON(dummy)
 *
 *	  でも、いつもADJUSTMENT が出るのはうっとおしいので、
 *	  画面からはみ出しそうな時のみ、SCROLLED WINDOW にしたい。
 *	  が、はみ出しそうかどうかの判定を性格に行なうには、結構面倒そう
 *	  なので、とりあえず COMBO の表示位置をベースに、適当にはみ出し
 *	  そうと判断したときのみ、SCROLLED WINDOW にしてみた。
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

  q8tk_widget_destroy( combo_list );		/* LIST は破棄。ただし、子の */
  q8tk_widget_destroy( combo_window );		/* LIST ITEM は残す。*/
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

  q8tk_widget_destroy( combo_list );		/* LIST は破棄。ただし、子の */
  q8tk_widget_destroy( combo_window );		/* LIST ITEM は残す。*/
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

  if( widget->y + i + 2 > 24 ){		/* 画面からはみ出そうなら時 */
					/* SCROLLED WINDOW を生成   */
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

  }else{				/* 通常はこっちで */

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


	/* ESC キーを押した時リストを消去するための、ダミーを生成 */

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

  if( combo->stat.combo.list ){		/* 既存のリストを破棄(開放) */
    Q8List *l = combo->stat.combo.list;			/* TODO : 未チェック */
    while( l ){
      widget_destroy_child( (Q8tkWidget *)l->ptr );
      l = l->next;
    }
    q8_list_free( combo->stat.combo.list );
  }

  list = q8_list_first( list );
  combo->stat.combo.length = 0;		/* 最長文字列数を記憶しておく */

  while( list ){			/* 全listを LIST_ITEM 化する */
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
 *	・リストボックス。選択方法は、SELECTION_BROWSE のみ
 *	・複数の子をもてる。
 *	  しかし、LIST ITEM 以外は子にしないこと
 *	・子を持つには、q8tk_container_add() を使用する。
 *	  子は、順番に LIST にならんでいく。
 *	・シグナル
 *		"selection_change"	選択対象の LIST ITEM が変わった時
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
    if( key <= 0xff && isgraph(key) ){ /* スペース以外の文字が入力された場合 */

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

	      (*w->event_button_on)( w );	/* …… 無い方がいいかなあ */
	    }
	    set_construct_flag( TRUE );
	    break;
	  }
	}
      }

    }else{			/* スペースや、制御文字が入力された場合 */
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
 * これはちょっと特殊処理。
 *	LIST が SCROLLED WINDOW の子の場合で、SCROLLED WINDOW のスクロール
 *	バー(縦方向) が動かされたとき、この関数が呼ばれる。
 *	ここでは、SCROLLED WINDOW の表示範囲に応じて、LIST の active
 *	ウィジットを変更している。
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
 *	・リストアイテム。
 *	・LIST の子になれる
 *	・子を持てるが、LABEL に限る
 *	・子を持つには、q8tk_container_add() を使用する。
 *	・シグナル
 *		"select"	クリックした時 (既選択状態でも発生)
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
    widget_redraw_now();	/* 一旦再描画 */
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
 *	・ラベル
 *	・(表示用の)文字列を保持できる
 *	・シグナル … 無し
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
 *	・フレーム
 *	・子をひとつもてる。
 *	・子を持つには、q8tk_container_add() を使用する。
 *	・(見出しの)文字列を保持できる。
 *	・シグナル … 無し
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
 *	・水平ボックス
 *	・複数の子をもてる。
 *	・子を持つには、q8tk_box_pack_XXX() を使用する。
 *	・シグナル … 無し
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
 *	・垂直ボックス
 *	・複数の子をもてる。
 *	・子を持つには、q8tk_box_pack_XXX() を使用する。
 *	・シグナル … 無し
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
 *	・ノートブック
 *	・複数のページをもてる。各ページはコンテナであり、子を一つ持てる。
 *	・q8tk_notebook_append() で、子を持つが、その度に内部で
 *	  NOTE PAGE を生成し、これが子を持つことになる。
 *	・NOTE PAGE は、(見出しの)文字列を保持できる。
 *	・シグナル
 *		"switch_page"	別のページに切り替わった時に発生
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
 *	・垂直セパレータ
 *	・子は持てない
 *	・長さが、親ウィジットの大きさにより、動的に変わる。
 *	・シグナル … なし
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
 *	・水平セパレータ
 *	・子は持てない
 *	・長さが、親ウィジットの大きさにより、動的に変わる。
 *	・シグナル … なし
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
 *	・アジャストメント
 *	・レンジ(値の範囲)と増分(小さい増分、大きい増分の2種類)
 *	  を持つが、いずれも整数に限る。
 *	・スケールウィジットやスクロールドウインドウを生成する
 *	  際に、必要となる。単独では使用することはない。
 *	・スケールを生成する際に、前もってアジャストメントを
 *	  生成しておき、このアジャストメントをもってスケールを
 *	  生成するのが一般的。逆に、スクロールドウインドウは
 *	  その生成時にアジャストメントを自動生成するのが一般的
 *	  なため、前もってアジャストメントを生成することは無い。
 *	・子は持てない。子になることもできない。
 *	・シグナル
 *		"value_changed"		値が変わった時に発生
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

    /* シグナルは発生させない */
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

  /* シグナルは発生させない */

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
 *	・水平スケール
 *	・new()時に、引数でアジャストメントを指定する。スケールの
 *	  レンジ(範囲)や増分は、このアジャストメントに依存する。
 *	・new()時の引数が NULL の場合は、自動的にアジャストメント
 *	  が生成されるが、この時のレンジは 0～10 、増分は 1 と 2
 *	  に固定である。(変更可能)
 *	・子は持てない
 *	・シグナル … なし。
 *		      ただし、アジャストメントはシグナルを受ける。
 *	--------------------------------------------------------
 *	- HSCALE
 *	     :…… ADJUSTMENT
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
 *	・垂直スケール
 *	・new()時に、引数でアジャストメントを指定する。スケールの
 *	  レンジ(範囲)や増分は、このアジャストメントに依存する。
 *	・new()時の引数が NULL の場合は、自動的にアジャストメント
 *	  が生成されるが、この時のレンジは 0～10 、増分は 1 と 2
 *	  に固定である。(変更可能)
 *	・子は持てない
 *	・シグナル … なし。
 *		      ただし、アジャストメントはシグナルを受ける。
 *	--------------------------------------------------------
 *	- VSCALE
 *	     :…… ADJUSTMENT
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
 * 表示時のサイズ計算 (widget_size()内から呼ばれる)
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
 *	・スクロールドウインドウ
 *	・new()時に、引数でアジャストメントを指定するが、スケールの
 *	  レンジ(範囲)はこのスクロールドウインドウの子の大きさに
 *	  よって、動的に変化する。(増分は引き継がれる)
 *	・new()時の引数が NULL の場合は、自動的にアジャストメント
 *	  が生成される。この時の増分は 1 と 10 である。特に理由が
 *	  なければ、NULL による自動生成の方が簡単で便利。
 *	・子を一つ持てる。ただし、子や孫が SCROLLED WINDOW を
 *	  持つような場合の動作は未検証である。
 *	・シグナル … なし
 *	--------------------------------------------------------
 *	- SCROLLED WINDOW     - xxx
 *	     :…… ADJUSTMENT
 *	     :…… ADJUSTMENT
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
 *	・エントリー
 *	・文字の入力が可能。ただし、他のウィジットが「文字」は
 *	  すべて EUC-JAPAN を前提にしているのに対し、エントリー
 *	  では、PC-8801 で使用されている ANK文字 を前提にしている。
 *	  (つまり、漢字は禁止。半角カナやグラフィック文字は可 !)
 *	・子は持てない
 *	・シグナル
 *		"activate"	リターンキー入力があった時に発生
 *		"changed"	文字入力、文字削除があった時に発生
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
 *	・MENU の 土台
 *	・子は持てないが、MENU を子のように扱える。
 * MENU
 *	・OPTION MENU の子になれる
 *	・シグナル
 *		"changed"	内容が変わった時に発生
 * RADIO MENU ITEM
 *	・一応コンテナだが、LABELしか乗せれない
 *	・シグナル
 *		"activate"	選択された時に発生
 *		"activate-list"	内部で使用される特殊なシグナル
 *				一般の使用は禁止。
 * MENU LIST
 *	・OPTION MENU 処理の内部でのみ使用される特殊なウィジット
 *	・一般に使用は不可
 *	--------------------------------------------------------
 *
 *	OPTION MENU - MENU
 *			:
 *			:…… RADIO MENU ITEM - LABEL
 *				     |
 *			      RADIO MENU ITEM - LABEL
 *				     |
 *			      RADIO MENU ITEM - LABEL
 *				     |
 *
 *	  ・OPTION MENU はコンテナだが、MENU しか乗せれない。
 *	    ( q8tk_container_add()関数ではなく、q8tk_option_menu_set_menu()
 *	      関数を使用して、MENU を乗せる。)
 *	  ・MENU は、コンテナではない。
 *	    RADIO MENU ITEM を複数個、メニューとして登録することが出来る。
 *	    ( q8tk_menu_append()関数を使って、登録する )
 *	  ・RADIO MENU ITEM は、コンテナだが、LABEL しか乗せれない。
 *
 *	●OPTION MENU が マウスでクリックされると、OPTION MENU は、
 *	  WINDOW (DIALOG) を自動生成し、以下のような親子構造を作る。
 *	  なお、子文字の radio menu item - label は、上の MENU に登録
 *	  された、ウィジットである。
 *
 *	WINDOW - MENU LIST - radio menu item - label
 *				    |
 *			     radio menu item - label
 *				    |
 *			     radio menu item - label
 *				    |
 *
 *	○このとき、radio menu item は内部で、シグナル "activate-list" が
 *	登録される。(ユーザは、このシグナルの使用禁止)
 *
 *	○適当な radio menu item がクリックされると、"activate-list"
 *	シグナルが発生し、引き続き、RADIO MENU ITEM に "activate"、
 *	MENU に、"selection_changed" シグナルが送られる。
 *
 *	GTK を強引に真似ただけに、実装に難あり。
 *--------------------------------------------------------------*/
static	Q8tkWidget	*q8tk_menu_list_new( void );
static	void	q8tk_menu_list_select_child( Q8tkWidget *mlist,
					     Q8tkWidget *child );
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *	OPTION MENU
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*
 * OPTION MENU により、自動生成されたメニューリストの各アイテムのコールバック。
 *	RADIO MENU ITEM に "activate"、MENU に "changed" を送る。
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

  q8tk_widget_destroy( option_menu_list );	/* MENU LIST は破棄。但し子の*/
  q8tk_widget_destroy( option_menu_window );	/* RADIO MENU ITEM は残す。*/
}

/*
 * OPTION MENU がクリックされた時のコールバック。
 *	WINDOW ～ MENU LIST を自動生成する
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
    widget_redraw_now();	/* 一旦再描画 */
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
    /* シグナルは送らない  この時点で送るとややこしくなる。
       widget_signal_do( child, "activate" );
    */
  }
}




/*--------------------------------------------------------------
 * ACCEL GROUP
 *	・アクセラレーターキー定義の土台 (親)
 *	・子は持てないし、表示もできない。
 *	・内部的には、ACCEL KEY を子に持つ形態をとる
 *	・ウインドウに関連づけることで、子の ACCEL KEY を
 *	  そのウインドウで有効にすることができる。
 * ACCEL KEY
 *	・アクセラレーターキー定義
 *	・子は持てないし、表示もできない。
 *	・内部的には、ACCEL GROUP の子の形態をとる
 *	・親の ACCEL GROUP と関連付けられたウインドウにて、
 *	  キーボード入力を受けとると、定義したウィジットにシグナルを
 *	  送る。
 *	--------------------------------------------------------
 *	 WINDOW
 *	    :
 *	    :…… ACCEL GROUP - ACCEL KEY
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
 *	・ダイアログ
 *	--------------------------------------------------------
 *	WINDOW - FRAME - VBOX - VBOX(vbox)
 *	  :		 	 |
 *	  :			HBOX(action_area)
 *	DIALOG
 *
 *	q8tk_dialog_new()の返り値は、WINDOW になる。
 *
 *	DIALOG の ワークに、VBOX (vbox) と、HBOX (action_area) が
 *	設定されるので、これを介してユーザは設定を行なう。
 *
 *	q8tk_widget_destroy() の際は、この4つのウイジットを
 *	破壊する。
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
 *	・ファイルセレクション
 *	・扱えるファイル名の長さは、パスを含めて最大で
 *	  Q8TK_MAX_FILENAME までである。(文字列終端の\0を含む)
 *	--------------------------------------------------------
 *	WINDOW - VBOX - LABEL (q8tk_fileselection_new()の引数)
 *	  :		  |
 *	  :		HSEPARATOR
 *	FSELECT		  |
 *			LABEL (ディレクトリ名)
 *			  |
 *			LABEL (ファイル数)
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
 *	q8tk_fileselection_new()の返り値は、WINDOW になる。
 *
 *	FILESELECTION の ワークに、LIST(file_list) と、ENTRY
 *	(selection_entry) と、BUTTON (cancel_button/ok_button) が
 *	設定されるので、これを介してユーザは設定を行なう。
 *
 *	q8tk_widget_destroy() の際は、すべてのウィジット( 18個の
 *	ウィジット 及び 不定数の LIST ITEM ウイジット)を破壊する。
 *
 *	・ENTRY が changed を受けた場合、入力があったものとみなす。
 *	・LIST ITEM が select を受けた場合、
 *		LIST が selection_changed を受けた場合は、
 *			ENTRY に LIST ITEM の文字列をセット
 *		LIST が selection_changed を受けていない場合は、
 *			入力があったものとみなす。
 *
 *	・入力があった場合、以下の処理を行なう。
 *		入力がディレクトリの場合
 *			LIST ITEM を全て更新
 *			ENTRY はクリア
 *		入力がファイルの場合、
 *			そのファイル名を filename とする。
 *			ok_button に シグナルを送る
 *
 *--------------------------------------------------------------*/

static	int	fsel_set_file_list( Q8tkWidget *fsel, const char *filename );
static	int	fsel_open_file( Q8tkWidget *fselect, const char *filename );

/* ファイルリストで、選択されているアイテムが、代わった時のコールバック */
static	void	fsel_list_selection_changed( Q8tkWidget *dummy, void *fselect )
{
  Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = TRUE;
}

/* ファイルリストで、アイテムが選択された時のコールバック */
static	void	fsel_item_selected( Q8tkWidget *item, void *fselect )
{
  char	name[ Q8TK_MAX_FILENAME ];
  char	wk[ Q8TK_MAX_FILENAME ];
  T_DIR_ENTRY dir;


  /* LIST ITEM に登録されたファイル名を、アクセス可能なファイル名に変換 */

  dir.type = item->stat.misc.attr;
  dir.name = item->child->name;
  osd_readdir_realname( name, &dir, Q8TK_MAX_FILENAME );


	/* 現在選択されているのと、別のファイル名が選ばれた時、
	   そのファイル名を、エントリーにセットする。*/

  if( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed ){

    Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = FALSE;

    ((Q8tkWidget*)fselect)->stat.fselect.selection_changed = FALSE;

    q8tk_entry_set_text( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)
				->selection_entry, name );

	/* 現在選択されているのと、同じファイル名が選ばれた時、
	   そのファイル名開く。*/

  }else{

				/* 現在のパス (file_selection_pathname) と、
				   選択したファイル名 (item->child->name) から
				   フルパスのファイル名を生成する */
    osd_path_connect( wk, file_selection_pathname, name, Q8TK_MAX_FILENAME );

    if( fsel_open_file( (Q8tkWidget*)fselect, wk ) ){
      /*widget_redraw_now();*/	/* 一旦再描画 */
      widget_signal_do( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->ok_button,
		        "clicked" );
    }

  }

}

/* ファイルリストで、エントリーに入力された時のコールバック */
static	void	fsel_entry_activate( Q8tkWidget *entry, void *fselect )
{
  char	wk[ Q8TK_MAX_FILENAME ];

			/* 現在のパス (file_selection_pathname) と、
			   入力したファイル名 (q8tk_entry_get_text(entry)) から
			   フルパスのファイル名を生成する */
  osd_path_connect( wk, file_selection_pathname,
		    q8tk_entry_get_text(entry), Q8TK_MAX_FILENAME );


  if( fsel_open_file( (Q8tkWidget*)fselect, wk ) ){
    widget_signal_do( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->ok_button,
		      "clicked" );
  }
}


/* ディレクトリを開いて、ファイルリストを生成する関数 (コールバックじゃない) */
static	int	fsel_set_file_list( Q8tkWidget *fsel, const char *filename )
{
  T_DIR_INFO	*dirp;
  T_DIR_ENTRY	*dirent;
  char		wk[ Q8TK_MAX_FILENAME ];
  int		nr = 0, i;
  Q8tkWidget	*c;

		/* 既存の LIST ITEM 削除 */

  while( ( c = Q8TK_FILE_SELECTION(fsel)->file_list->child ) ){
    q8tk_container_remove( Q8TK_FILE_SELECTION(fsel)->file_list, c );
    q8tk_widget_destroy( c );
  }
  q8tk_adjustment_set_value( Q8TK_FILE_SELECTION(fsel)->scrolled_window
						    ->stat.scrolled.hadj, 0 );
  q8tk_adjustment_set_value( Q8TK_FILE_SELECTION(fsel)->scrolled_window
						    ->stat.scrolled.vadj, 0 );


		/* ディレクトリを調べ、ファイル名を LIST ITEM として登録 */

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
      filename[0]=='\0' ){		/* filenameが無効の場合は、	*/
    filename = osd_get_current_dir();	/* カレントディレクトリとする	*/
    if( filename==NULL ) filename = "";
  }

  q8_strncpy( fname, filename, Q8TK_MAX_FILENAME );
  osd_path_regularize( fname, Q8TK_MAX_FILENAME );	/* 正規化 */


	/**** ディレクトリかどうかを調べる ****/

  if( osd_file_stat( fname )==FILE_STAT_DIR ){

    fsel_set_file_list( fselect, fname );
    q8_strncpy( file_selection_pathname, fname, Q8TK_MAX_FILENAME );
    q8tk_entry_set_text( Q8TK_FILE_SELECTION(fselect)->selection_entry, "" );

    return 0;			/* filename はディレクトリでした */


	/**** ディレクトリじゃなかった場合 ****/
  }else{

		/* ファイル名手前の '/' を検索し、そのディレクトリをひらく */

    child = osd_path_separate( parent, fname, Q8TK_MAX_FILENAME );

    if( parent[0] != '\0' ){			/* パス分離成功		*/

      if( fsel_set_file_list( fselect, parent ) ){  /*ディレクトリ開けたら */

						    /*パスを保持しておいて */
	q8_strncpy( file_selection_pathname, parent, Q8TK_MAX_FILENAME );

	if( child ) filename = child;		    /*パス分離後の         */
	else        filename = "";		    /*ファイル名を指す     */

      }else{					    /* ディレクトリ開けない */

	q8tk_label_set( Q8TK_FILE_SELECTION(fselect)->dir_name, 
							"DIR = non existant" );
	q8tk_label_set( Q8TK_FILE_SELECTION(fselect)->nr_files, 
							"0 file(s)" );
      }
    }

    q8tk_entry_set_text( Q8TK_FILE_SELECTION(fselect)->selection_entry,
			 filename );

    return 1;			/* filename はファイルでした */
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

/* 返り値は、filename が ファイルなら真、ディレクトリなら偽 */
int		q8tk_file_selection_set_filename( Q8tkWidget *fselect,
						  const char *filename )
{
  return fsel_open_file( fselect, filename );
}






/************************************************************************/
/* 雑多な処理								*/
/************************************************************************/

/*--------------------------------------------------------------
 * 表示位置の変更 (一部のウィジットのみ可能)
 *--------------------------------------------------------------*/
void	q8tk_misc_set_placement( Q8tkWidget *widget,
				 int placement_x, int placement_y )
{
  widget->placement_x = placement_x;
  widget->placement_y = placement_y;
}

/*--------------------------------------------------------------
 * 表示サイズの変更 (一部のウィジットのみ可能)
 *	・COMBO		  … 文字列部分の表示幅
 *	・LIST		  … 文字列の表示幅
 *	・SCROLLED WINDOW … ウインドウの幅、高さ
 *	・ENTRY		  … 文字列の表示幅
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
 * 強制再描画
 *--------------------------------------------------------------*/
void	q8tk_misc_redraw( void )
{
  set_construct_flag( TRUE );
}

/************************************************************************/
/* コンテナ関係								*/
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
  case Q8TK_TYPE_LIST:				/* LIST BOX 例外処理 */
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

  case Q8TK_TYPE_MENU_LIST:			/* MENU LIST 例外処理 */
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

  default:					/* 通常の処理 */
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

  if( widget->prev == NULL ){			/* 自分が親の直下の時 */
    Q8tkWidget *n = widget->next;
    if( n ) n->prev = NULL;
    container->child = n;

  }else{					/* そうじゃない時 */
    Q8tkWidget *p = widget->prev;
    Q8tkWidget *n = widget->next;
    if( n ) n->prev = p;
    p->next = n;
  }

  switch( container->type ){
  case Q8TK_TYPE_LIST:				/* LIST BOX 例外処理 */
    if( container->stat.list.selected == widget ){
      container->stat.list.selected = container->child;
      container->stat.list.active   = container->child;
    }
    break;
  case Q8TK_TYPE_MENU_LIST:			/* MENU LIST 例外処理 */
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
/* 表示関係								*/
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
/* 消去関係								*/
/*	自身を消去							*/
/*	自身と子を消去							*/
/*	自身と子と兄弟を消去						*/
/*									*/
/*	TODO : 問題）COMBO の連れはどうする？				*/
/************************************************************************/
void	q8tk_widget_destroy( Q8tkWidget *widget )
{
  Q8tkWidget *work;

  if( widget->type == Q8TK_TYPE_WINDOW &&	/* DIALOG と FILE SELECTION */
      (work = widget->stat.window.work) ){	/* の例外処理		    */
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

  if( widget->type == Q8TK_TYPE_ACCEL_GROUP &&	/* ACCEL の子は全て消去 */
      widget->child ){
    widget_destroy_all( widget->child );
  }


  if( widget->with_label &&			/* XXX_new_with_label()で */
      widget->child      &&			/* 生成された LABELの処理 */
      widget->child->type == Q8TK_TYPE_LABEL ){
    q8tk_widget_destroy( widget->child );
  }
  if( widget->with_label &&			/* 引数 NULL で生成された */
      widget->type==Q8TK_TYPE_SCROLLED_WINDOW ){/* SCROLLED WINDOW の処理 */
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
/* 特定のウィジットにフォーカスを当てる。				*/
/*	通常 フォーカスは、直前にボタンやキー入力がなされたウィジット	*/
/*	になり、TAB により切替え可能だが、この関数で特定のウィジット	*/
/*	にフォーカスを設定することができる。				*/
/*	ただし、そのウィジットの一番先祖の WINDOW が、q8tk_grab_add()の	*/
/*	処理をなされたあとでなければ無効である。			*/
/************************************************************************/
void	q8tk_widget_grab_default( Q8tkWidget *widget )
{
  set_event_widget( widget );
  set_construct_flag( TRUE );
}


/************************************************************************/
/* シグナル関係								*/
/************************************************************************/
/*
 *	任意のウィジットに、任意のシグナルを送る
 */
static	void	widget_signal_do( Q8tkWidget *widget, const char *name )
{
  switch( widget->type ){
  case Q8TK_TYPE_BUTTON:			/* ボタン		*/
    if( strcmp( name, "clicked" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;
  case Q8TK_TYPE_TOGGLE_BUTTON:			/* トグルボタン		*/
  case Q8TK_TYPE_CHECK_BUTTON:			/* チェックボタン	*/
  case Q8TK_TYPE_RADIO_BUTTON:			/* ラジオボタン		*/
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

  case Q8TK_TYPE_NOTEBOOK:			/* ノートブック		*/
    if( strcmp( name, "switch_page" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_COMBO:				/* コンボボックスもどき */
    if( strcmp( name, "changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_LIST:				/* リストボックス	*/
    if( strcmp( name, "selection_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
    if( strcmp( name, "select" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_ADJUSTMENT:			/* アジャストメント */
    if( strcmp( name, "value_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_ENTRY:				/* エントリー		*/
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

  case Q8TK_TYPE_MENU:				/* メニュー */
    if( strcmp( name, "changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_MENU_LIST:			/* メニューリスト	*/
    if( strcmp( name, "selection_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_RADIO_MENU_ITEM:		/* ラジオメニューアイテム */
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
/* 返り値は、無効 (必ず 0 ) */
int	q8tk_signal_connect( Q8tkWidget *widget, const char *name,
			     Q8tkSignalFunc func, void *func_data )
{
  switch( widget->type ){
  case Q8TK_TYPE_BUTTON:			/* ボタン		*/
    if( strcmp( name, "clicked" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;
  case Q8TK_TYPE_TOGGLE_BUTTON:			/* トグルボタン		*/
  case Q8TK_TYPE_CHECK_BUTTON:			/* チェックボタン	*/
  case Q8TK_TYPE_RADIO_BUTTON:			/* ラジオボタン		*/
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

  case Q8TK_TYPE_NOTEBOOK:			/* ノートブック		*/
    if( strcmp( name, "switch_page" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_COMBO:				/* コンボボックスもどき */
    if( strcmp( name, "changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_LIST:				/* リストボックス	*/
    if( strcmp( name, "selection_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
    if( strcmp( name, "select" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_ADJUSTMENT:			/* アジャストメント */
    if( strcmp( name, "value_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_ENTRY:				/* エントリ		*/
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

  case Q8TK_TYPE_MENU:				/* メニュー		*/
    if( strcmp( name, "changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_MENU_LIST:			/* メニューリスト	*/
    if( strcmp( name, "selection_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_RADIO_MENU_ITEM:		/* ラジオメニューアイテム */
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
/* イベントのチェック							*/
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
/* メイン								*/
/************************************************************************/
/*
 *	強制的に、即座に再描画
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
    menu_check_event( &type, &button, &x, &y );	/* イベント待ち	*/

    switch( type ){

    case MENU_EVENT_KEY_ON:
      if( !get_drag_widget() ){			/* ドラッグ中じゃない	*/
	if( button==Q8TK_KEY_TAB ){			/*	[TAB]キー */
	  if( get_event_widget()==NULL )
	    set_event_widget( q8tk_tab_top_widget );
	  else
	    set_event_widget( get_event_widget()->tab_next );
	  {
	    Q8tkWidget *w = get_event_widget();
	    if( w && w->type != Q8TK_TYPE_ADJUSTMENT		/* スクロール*/
	          && w->type != Q8TK_TYPE_LIST			/* 補正が    */
	          && w->type != Q8TK_TYPE_DIALOG		/* なされない*/
	          && w->type != Q8TK_TYPE_FILE_SELECTION ){	/* ウィジット*/
	      set_scroll_adj_widget( w );
	    }
	  }
	  set_construct_flag( TRUE );
	}else{						/*	他のキー */

	  int grab_flag = FALSE;
	  if( window_level_now >= 0 ){				/* アクセラ */
	    Q8tkWidget *w = window_level[ window_level_now ];	/* レーター */
	    Q8tkAssert(w->type==Q8TK_TYPE_WINDOW,NULL);		/* キーが、 */
	    if( w->stat.window.accel ){				/* 設定され */
	      w = (w->stat.window.accel)->child;		/* ていれば */
	      while( w ){					/* 処理する */
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
      if( button & Q8TK_BUTTON_L ){		/* 左ボタン ON		*/
	if( !get_drag_widget() ){		/* ドラッグ中じゃない	*/
	  Q8tkWidget	*focus;
	  focus = (Q8tkWidget *)q8gr_get_focus_screen( mouse.x/8, mouse.y/16 );
	  set_event_widget( focus );
	  widget_button_on( get_event_widget() );
	}
      }else if( button & Q8TK_BUTTON_U ){	/* ホイール UP		*/
	widget_key_on( get_event_widget(), Q8TK_KEY_PAGE_UP );
      }else if( button & Q8TK_BUTTON_D ){	/* ホイール DOWN	*/
	widget_key_on( get_event_widget(), Q8TK_KEY_PAGE_DOWN );
      }
      break;

    case MENU_EVENT_MOUSE_OFF:
      if( button & Q8TK_BUTTON_L ){		/* 左ボタン OFF		*/
	if( get_drag_widget() ){		/* 只今 ドラッグ中	*/
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
      if( get_drag_widget() ){			/* 只今 ドラッグ中	*/
	if( mouse.x/8  != mouse.x_old/8  ||	/* マウス 8dot以上 移動	*/
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
/* ウィジットの表示設定
	親ウィジットをつぎつぎチェックし、
	・全ての親ウィジットが可視状態
	・先祖ウィジットが WINDOW
	の場合、その WINDOW 以下を再計算して表示する
	ように、フラグをたてる。
	実際の再計算、表示は、q8tk_main() で行なう。	*/
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
/* 子ウィジットのサイズを元に、自分自身のサイズを計算する。
   再帰的に、全ての子ウィジェットも同時に計算する。

   *widget … 一番親のウィジット
   *max_sx … 全ての子ウィジットのなかで最大サイズ x
   *max_sy … 全ての子ウィジットのなかで最大サイズ y
   *sum_sx … 子の仲間ウィジットのサイズ総和 x
   *sum_sy … 子の仲間ウィジットのサイズ総和 y          */
/*------------------------------------------------------*/
static	void	widget_resize( Q8tkWidget *widget, int max_sx, int max_sy );
static	void	widget_size( Q8tkWidget *widget, int *max_sx, int *max_sy,
						 int *sum_sx, int *sum_sy )
{
  int	n_msx, n_msy, n_ssx, n_ssy;

/*printf("%d \n",widget->type);fflush(stdout);*/


		/* 自分自身の仲間 (next) が存在すれば、再帰的に計算 */
  
  if( widget->next ){
    widget_size( widget->next, &n_msx, &n_msy, &n_ssx, &n_ssy );
  }else{
    n_msx = n_msy = n_ssx = n_ssy = 0;
  }


  if( widget->visible ){

    int	c_msx, c_msy, c_ssx, c_ssy;

		/* 子ウィジットのサイズ計算(再帰) */

    if( widget->child ){
      widget_size( widget->child, &c_msx, &c_msy, &c_ssx, &c_ssy );
    }else{
      c_msx = c_msy = c_ssx = c_ssy = 0;
    }

		/* 子ウィジットを元に、自身のサイズ計算 */

    switch( widget->type ){
    case Q8TK_TYPE_WINDOW:			/* ウインドウ		*/
      if( widget->stat.window.no_frame ){
	widget->sx = c_msx;
	widget->sy = c_msy;
      }else{
	widget->sx = c_msx +2;
	widget->sy = c_msy +2;
      }
      break;
    case Q8TK_TYPE_BUTTON:			/* ボタン		*/
    case Q8TK_TYPE_TOGGLE_BUTTON:		/* トグルボタン		*/
      widget->sx = c_msx +2;	/* サイズ変更 4 → 2 */
      widget->sy = c_msy +2;
      break;
    case Q8TK_TYPE_CHECK_BUTTON:		/* チェックボタン	*/
    case Q8TK_TYPE_RADIO_BUTTON:		/* ラジオボタン		*/
      widget->sx = c_msx +3;
      widget->sy = (c_msy==0) ? 1 : c_msy;
      break;
    case Q8TK_TYPE_FRAME:			/* フレーム		*/
      widget->sx = ( ( widget->name )
			? Q8TKMAX( c_msx, euclen( widget->name ) )
			: c_msx )   +2;
      widget->sy = c_msy +2;
      break;
    case Q8TK_TYPE_LABEL:			/* ラベル		*/
      widget->sx = ( widget->name ) ? euclen( widget->name ) : 0;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_NOTEBOOK:			/* ノートブック		*/
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
    case Q8TK_TYPE_NOTEPAGE:			/* ノートブックのページ	*/
      widget->sx = c_msx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_VBOX:			/* 縦ボックス		*/
      widget->sx = c_msx;
      widget->sy = c_ssy;
      break;
    case Q8TK_TYPE_HBOX:			/* 横ボックス		*/
      widget->sx = c_ssx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_VSEPARATOR:			/* 縦区切り線		*/
      widget->sx = 1;
      widget->sy = 1;
      break;
    case Q8TK_TYPE_HSEPARATOR:			/* 横区切り線		*/
      widget->sx = 1;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_COMBO:			/* コンボボックスもどき */
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

    case Q8TK_TYPE_LIST:			/* リストボックス	*/
      widget->sx = Q8TKMAX( c_msx, widget->stat.list.width );
      widget->sy = c_ssy;
      break;

    case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
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

	/* 縦横スクロールバーを表示するかどうかは、
	   child のサイズ、scrolled のサイズ、scrolled の policy、
	   の組み合わせにより、以下の 9 パターンに分かれる。

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

    case Q8TK_TYPE_OPTION_MENU:			/* オプションメニュー	*/
      widget->sx = c_msx + 4 + ((widget->stat.option.button) ?2 :0);
      widget->sy = c_msy +     ((widget->stat.option.button) ?2 :0);
      break;
    case Q8TK_TYPE_MENU:			/* メニュー */
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
    case Q8TK_TYPE_RADIO_MENU_ITEM:		/* ラジオメニューアイテム*/
      widget->sx = c_msx;
      widget->sy = c_msy;
      break;
    case Q8TK_TYPE_MENU_LIST:			/* メニューリスト	*/
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


		/* サイズ情報更新 */

  *max_sx = Q8TKMAX( widget->sx, n_msx );
  *max_sy = Q8TKMAX( widget->sy, n_msy );
  *sum_sx = widget->sx + n_ssx;
  *sum_sy = widget->sy + n_ssy;


		/* 子ウィジットにセパレータが含まれる場合は、サイズ調整 */

  widget_resize( widget, widget->sx, widget->sy );


		/* リストボックスなどの場合、子ウィジットのサイズを調整 */

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
 * セパレータなど、親の大きさに依存するウィジットのサイズを再計算する
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
/* スクロールドウインドウに含まれるウィジットの表示調整

   スクロールドウインドウに含まれるウィジットで、ウインドウ外に
   あるものに関して、特定のフラグが設定されていれば、強制的に
   スクロール位置を調整し、可能な限りウィジット全体を表示する
   処理を行なう。そのためにいろいろとワークの準備を行なう。

   スクロールドウインドウにあるウィジットが表示されたかどうかは、
   実際に表示するまでわからないので、実際に表示しながらワークを
   設定し、ワークの内容次第で、再表示ということになる。

   つまり、

	for( i=0; i<2; i++ ){
	  widget_size();
	  widget_scroll_adjust_init();
	  widget_draw();
	  if( ! widget_scroll_adjust() ) break;
	}

   となる。

  なお、処理を行なうのは、最初に見つかった 親の SCROLLED WINDOW のみ
  に対してだけなので、SCROLLED WINDOW の入れ子の場合はどうなるか
  わからない。						*/
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
	   SCROLLED WINDOW 内に乗せるウィジット全体を、左上を原点とした、
	   相対位置 real_y

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
/* 自分自身の表示位置をもとに、描画し、
   再帰的に、全ての子ウィジェットも描画する。		*/
/*------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*
   ウィジットのTAB関連づけ
	TABキーによる、フォーカスの移動を実現するために、
	描画前、描画中、描画後に各種設定を行なう。

		widget_tab_pre()  … TABづけの初期化
		widget_draw()     … 描画。内部でTABづけを行なう
		widget_tab_post() … TABづけの後処理

	widget_draw() の内部では、TABづけを行ないたいウィジットの描画の
	際に、『widget_tab_set()』 関数を呼ぶ。

	また、TAB付けされたウィジットが、アクティブ状態かどうかを知るには、
	描画の前後を、『check_active()』と『check_active_finish()』関数で囲む。

	【一般的な例】

	    widget_tab_set( widget );
	    check_active( widget );
	    q8gr_draw_xxx( x, y, is_active_widget(), widget );
	    if( widget->child ){
	      widget_draw();
	    }
	    check_active_finish();

	is_active_widget() は、自身がアクティブ状態かどうかを調べる関数で
	あり、check_active()～check_active_finish() の間で、有効である。

	上の場合、アクティブ状態は子ウィジットに伝播する。
	例えば、ボタンウィジットの場合、ボタンの子ウィジットであるラベルに、
	（アクティブな場合は）アンダーラインを引く、という風につかえる。
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


		/* HBOX、VBOX の子の場合に限り、*/
		/* 配置を若干変更できる。	*/

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


		/* 自分自身の typeをもとに枠などを書く。*/
		/* 子がいれば、x,y を求る。		*/
		/* 子の仲間(next)の、x,y も求める。	*/
		/* 直下の子に対してのみ再帰的に処理。	*/

/*printf("%s (%d,%d) %d %d\n",debug_type(widget->type),widget->sx,widget->sy,widget->x,widget->y);fflush(stdout);*/

  if( widget->visible ){

    switch( widget->type ){

    case Q8TK_TYPE_WINDOW:		/* ウインドウ		*/
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

    case Q8TK_TYPE_BUTTON:		/* ボタン		*/
    case Q8TK_TYPE_TOGGLE_BUTTON:	/* トグルボタン		*/
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_button( x, y, widget->sx, widget->sy,
		        widget->stat.button.active, widget );
      if( child ){
	child->x = x + 1;	/* サイズ変更 2 → 1 */
	child->y = y + 1;
	widget_draw( child );
      }
      check_active_finish();
      break;
    case Q8TK_TYPE_CHECK_BUTTON:	/* チェックボタン		*/
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
    case Q8TK_TYPE_RADIO_BUTTON:	/* ラジオボタン		*/
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

    case Q8TK_TYPE_FRAME:		/* フレーム		*/
      q8gr_draw_frame( x, y, widget->sx, widget->sy,
		       widget->name, widget->stat.frame.shadow_type );
      if( child ){
	child->x = x + 1;
	child->y = y + 1;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_LABEL:			/* ラベル		*/
      q8gr_euc_puts( x, y, widget->stat.label.foreground,
		           widget->stat.label.background,
		     FALSE, is_active_widget(),
		     (widget->name) ?(widget->name) :"" );
      break;

    case Q8TK_TYPE_NOTEBOOK:		/* ノートブック		*/
      q8gr_draw_notebook( x, y, widget->sx, widget->sy );
      if( child ){
	child->x = x;
	child->y = y;
	widget_draw( child );
      }
      break;
    case Q8TK_TYPE_NOTEPAGE:		/* ノートブックのページ	*/
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

    case Q8TK_TYPE_VBOX:			/* 縦ボックス		*/
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
    case Q8TK_TYPE_HBOX:			/* 横ボックス		*/
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

    case Q8TK_TYPE_VSEPARATOR:			/* 縦区切り線		*/
      q8gr_draw_vseparator( x, y, widget->sy );
      break;
    case Q8TK_TYPE_HSEPARATOR:			/* 横区切り線		*/
      q8gr_draw_hseparator( x, y, widget->sx );
      break;

    case Q8TK_TYPE_COMBO:			/* コンボボックスもどき */
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_combo( x, y, widget->sx -3, is_active_widget(),
		       q8tk_combo_get_text( widget ), widget );
      check_active_finish();
      break;

    case Q8TK_TYPE_LIST:			/* リストボックス	*/
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

    case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
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


	/* 自分自身の仲間 (next) が存在すれば、再帰的に処理 */
  
  if( widget->next ){
    widget = widget->next;
    widget_draw( widget );
  }
}














/*------------------------------------------------------*/
/* スクリーン画面を作成。
   q8tk_grab_add() で設定された WINDOW をトップとして、
   全ての子ウィジェットの大きさ、位置を計算し、
   menu_screen[][]に、表示内容を設定する。
   同時に、TAB キーを押された時の、フォーカスの変更の
   手順も決めておく。					*/
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

      do{				/* TAB TOP を NOTEPAGE 以外に設定 */
	if( w->type != Q8TK_TYPE_NOTEPAGE ) break;
	w = w->tab_next;
      } while( w != q8tk_tab_top_widget );
      q8tk_tab_top_widget = w;

      exist = FALSE;			/* event_widget が実在するかチェック */
      do{			
	if( w==get_event_widget() ){
	  exist = TRUE;
	  break;
	}
	w = w->tab_next;
      } while( w != q8tk_tab_top_widget );
      if( !exist ){			/*    実在しなければ NULL にしておく */
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
/* Q8TK を使う上で便利な、雑多な関数					*/
/************************************************************************/

/****************************************************************/
/* SJIS を EUC に変換 (かなり適当)				*/
/*	*sjis_p の文字列を EUC に変換して、*euc_p に格納する。	*/
/*	注意！）この関数は、バッファあふれをチェックしていない。*/
/*		*euc_p は、*sjis_p の倍以上の長さがないと危険	*/
/****************************************************************/

void	sjis2euc( char *euc_p, const char *sjis_p )
{
  int	h,l, h2, l2;

  while( ( h = (unsigned char)*sjis_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      *euc_p ++ = h;

    }else if( 0xa1 <= h && h <= 0xdf ){		/* 半角カナ */

      *euc_p ++ = (char)0x8e;
      *euc_p ++ = h;

    }else{					/* 全角文字 */

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
/* EUC を SJIS に変換 (かなり適当)				*/
/*	*euc_p の文字列を SJIS に変換して、*sjis_p に格納する。	*/
/*	注意！）この関数は、バッファあふれをチェックしていない。*/
/*		*sjis_p は、*euc_p と同等以上の長さがないと危険	*/
/****************************************************************/

void	euc2sjis( char *sjis_p, const char *euc_p )
{
  int	h,l;

  while( ( h = (unsigned char)*euc_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      *sjis_p ++ = h;

    }else if( h==0x8e ){			/* 半角カナ */

      if( ( h = (unsigned char)*euc_p++ ) ){

	if( 0xa1 <= h && h <= 0xdf )
	  *sjis_p ++ = h;

      }else{
	break;
      }

    }else if( h & 0x80 ){			/* 全角文字 */

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
/* EUC文字列の長さを計算 (けっこう適当)				*/
/*	ASCII・半角カナは1文字、全角漢字は2文字とする。		*/
/*	文字列末の、\0 は長さに含めない。			*/
/****************************************************************/

int	euclen( const char *euc_p )
{
  int	i = 0, h;

  while( ( h = (unsigned char)*euc_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      i++;

    }else if( h == 0x8e ){			/* 半角カナ */

      euc_p ++;
      i++;

    }else{					/* 漢字 */

      euc_p ++;
      i += 2;

    }
  }

  return i;
}
