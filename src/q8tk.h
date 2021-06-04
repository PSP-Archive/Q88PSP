#ifndef Q8TK_H_INCLUDED
#define Q8TK_H_INCLUDED

#include "quasi88.h"
#include "q8tk-common.h"


typedef	struct	_Q8tkWidget	Q8tkWidget;
typedef	struct	_Q8List		Q8List;

typedef void (*Q8tkSignalFunc)	();

struct	_Q8tkWidget{

  int		type;		/* �E�B�W�b�g�̎��	*/
  int		attr;		/* ����(�R���e�i���ǂ���)*/
  int		visible;

  int		placement_x;	/* �\���ʒu(�V�n���E��) */
  int		placement_y;

  int		x, y, sx, sy;	/* �\�����W�A�\���T�C�Y */

  Q8tkWidget	*parent;	/* �E�B�W�b�g�A���\��	*/
  Q8tkWidget	*child;
  Q8tkWidget	*prev;
  Q8tkWidget	*next;

  Q8tkWidget	*tab_next;	/* TAB�L�[�Ŏ���	*/

  char		*name;		/* ���x�� (malloc����)	*/

  int		with_label;	/* XXX_new_with_label()	*/

  union{			/* �E�B�W�b�g�ʃ��[�N	*/
    struct{				/* ---- �E�C���h�E ---- */
      int	no_frame;
      int	shadow_type;
      int	set_position;
      int	x, y;
      Q8tkWidget *work;
      Q8tkWidget *accel;
    } window;
    struct{				/* ---- �t���[�� ---- */
      int	shadow_type;
    } frame;
    struct{				/* ---- ���x�� ---- */
      int	foreground;
      int	background;
    } label;
    struct{				/* ---- �e��{�^�� ---- */
      int	active;
      Q8List	*list;
    } button;
    struct{				/* ---- �m�[�g�u�b�N ---- */
      Q8tkWidget *page;
    } notebook;
    struct{				/* ---- �R���{���ǂ� ---- */
      Q8List	*list;
      Q8tkWidget *selected;
      int	width;
      int	length;
    } combo;	/* WRITE �s�� */
    struct{				/* ---- ���X�g�{�b�N�X ---- */
      Q8tkWidget *selected;
      Q8tkWidget *active;
      int	width;
    } list;	/* SELECTION TYPE �� BROWSE �̂� */
    Q8Adjust	adj;			/* ---- �A�W���X�g�����g ---- */
    struct{				/* ---- �X�P�[�� ---- */
      Q8tkWidget *adj;
      int	draw_value;
      int	value_pos;
    } scale;
    struct{				/* ---- �X�N���[���h�E�C���h�E ---- */
      Q8tkWidget *hadj;
      Q8tkWidget *vadj;
      int	hpolicy;
      int	vpolicy;
      int	width;
      int	height;
      int	hscrollbar;
      int	vscrollbar;
      int	child_x0, child_y0;
      int	child_sx, child_sy;
      int	vadj_value;
    } scrolled;
    struct{				/* ---- �G���g���[ ---- */
      int	max_length;
      int	malloc_length;
      int	cursor_pos;
      int	disp_pos;
      int	disp_length;
    } entry;
    struct{				/* ---- �I�v�V�������j���[ ---- */
      int	button;
    } option;
    struct{				/* ---- ���j���[ ---- */
      Q8List	*list;
      Q8tkWidget *selected;
    } menu;
    struct{				/* ---- ���j���[���X�g ---- */
      Q8tkWidget *selected;
      Q8tkWidget *active;
    } menulist;

    struct{				/* ---- �A�N�Z�����[�^�[�L�[ ---- */
      Q8tkWidget *widget;
      int	key;
    } accel;

    struct{				/* ---- �_�C�A���O ---- */
      Q8tkWidget	*vbox;
      Q8tkWidget	*action_area;
    } dialog;
    struct{				/* ---- �t�@�C���Z���N�V���� ---- */
      Q8tkWidget	*file_list;
      Q8tkWidget	*selection_entry;
      Q8tkWidget	*ok_button;
      Q8tkWidget	*cancel_button;
      Q8tkWidget	*dir_name;
      Q8tkWidget	*nr_files;
      Q8tkWidget	*scrolled_window;
      int		selection_changed;
    } fselect;

    struct{				/* ---- ���̑� ---- */
      int	attr;
    } misc;

  }stat;

				/* �C�x���g�����֐� */

  void	(*event_button_on)(Q8tkWidget *);
  void	(*event_key_on)(Q8tkWidget *, int);
  void	(*event_dragging)(Q8tkWidget *);
  void	(*event_drag_off)(Q8tkWidget *);

				/* �C�x���g�������[�U�֐� */

  void	(*user_event_0)(Q8tkWidget *, void *);
  void	*user_event_0_parm;
  void	(*user_event_1)(Q8tkWidget *, void *);
  void	*user_event_1_parm;

};

enum{				/* (Q8tkWidget*)->type	*/
  Q8TK_TYPE_WINDOW,		/* �E�C���h�E		*/
  Q8TK_TYPE_BUTTON,		/* �{�^��		*/
  Q8TK_TYPE_TOGGLE_BUTTON,	/* �g�O���{�^��		*/
  Q8TK_TYPE_CHECK_BUTTON,	/* �`�F�b�N�{�^��	*/
  Q8TK_TYPE_RADIO_BUTTON,	/* ���W�I�{�^��		*/
  Q8TK_TYPE_FRAME,		/* �t���[��		*/
  Q8TK_TYPE_LABEL,		/* ���x��		*/
  Q8TK_TYPE_NOTEBOOK,		/* �m�[�g�u�b�N		*/
  Q8TK_TYPE_NOTEPAGE,		/* �m�[�g�u�b�N�̃y�[�W	*/
  Q8TK_TYPE_VBOX,		/* �c�{�b�N�X		*/
  Q8TK_TYPE_HBOX,		/* ���{�b�N�X		*/
  Q8TK_TYPE_VSEPARATOR,		/* �c��؂��		*/
  Q8TK_TYPE_HSEPARATOR,		/* ����؂��		*/
  Q8TK_TYPE_COMBO,		/* �R���{�{�b�N�X	*/
  Q8TK_TYPE_LIST,		/* ���X�g		*/
  Q8TK_TYPE_LIST_ITEM,		/* ���X�g�A�C�e��	*/
  Q8TK_TYPE_ADJUSTMENT,		/*   �A�W���X�g�����g	*/
  Q8TK_TYPE_HSCALE,		/* ���X�P�[��		*/
  Q8TK_TYPE_VSCALE,		/* �c�X�P�[��		*/
  Q8TK_TYPE_SCROLLED_WINDOW,	/* �X�N���[���E�C���h�E	*/
  Q8TK_TYPE_ENTRY,		/* �G���g���[		*/
  Q8TK_TYPE_OPTION_MENU,	/* �I�v�V�������j���[	*/
  Q8TK_TYPE_MENU,		/* ���j���[		*/
  Q8TK_TYPE_RADIO_MENU_ITEM,	/* ���W�I���j���[�A�C�e�� */
  Q8TK_TYPE_MENU_LIST,		/* ���j���[���X�g	*/

  Q8TK_TYPE_ACCEL_GROUP,	/* �A�N�Z�����[�^�L�[	*/
  Q8TK_TYPE_ACCEL_KEY,		/* �V			*/

  Q8TK_TYPE_DIALOG,		/* �_�C�A���O		*/
  Q8TK_TYPE_FILE_SELECTION,	/* �t�@�C���Z���N�V����	*/

  Q8TK_TYPE_END
};
enum{				/* (Q8tkWidget*)->attr	*/
  Q8TK_ATTR_CONTAINER       = (1<<0),	/* �R���e�i		*/
  Q8TK_ATTR_LABEL_CONTAINER = (1<<1),	/* �R���e�i(LABEL��p)	*/
  Q8TK_ATTR_MENU_CONTAINER  = (1<<2),	/* �R���e�i(MENU��p)	*/
  Q8TK_ATTR_END
};
enum{				/* (Q8tkWidget*)->placement_x	*/
  Q8TK_PLACEMENT_X_LEFT,
  Q8TK_PLACEMENT_X_CENTER,
  Q8TK_PLACEMENT_X_RIGHT,
  Q8TK_PLACEMENT_X_END
};
enum{				/* (Q8tkWidget*)->placement_x	*/
  Q8TK_PLACEMENT_Y_TOP,
  Q8TK_PLACEMENT_Y_CENTER,
  Q8TK_PLACEMENT_Y_BOTTOM,
  Q8TK_PLACEMENT_Y_END
};


enum{				/* window_new() �̈���	*/
  Q8TK_WINDOW_TOPLEVEL,		/* �g�b�v�̃E�C���h�E	*/
  Q8TK_WINDOW_DIALOG,
  Q8TK_WINDOW_POPUP,
  Q8TK_WINDOW_END
};
enum{				/* �t���[���̃^�C�v */
  Q8TK_SHADOW_NONE,
  Q8TK_SHADOW_IN,
  Q8TK_SHADOW_OUT,
  Q8TK_SHADOW_ETCHED_IN,
  Q8TK_SHADOW_ETCHED_OUT,
  Q8TK_SHADOW_END
};
enum{				/* �X�N���[���E�C���h�E�̑��� */
  Q8TK_POLICY_ALWAYS,
  Q8TK_POLICY_AUTOMATIC,
  Q8TK_POLICY_NEVER,
  Q8TK_POLICY_END
};


enum{				/* �ėp�ʒu�w�� */
  Q8TK_POS_LEFT,
  Q8TK_POS_RIGHT,
  Q8TK_POS_TOP,
  Q8TK_POS_BOTTOM,
  Q8TK_POS_END
};



struct _Q8List{
  void		*ptr;
  Q8List	*prev;
  Q8List	*next;
};




#define	Q8TKMAX( a, b )		((a)>(b)?(a):(b))



/*--------------------------------------------------------------*/
void	q8tk_init( void );
void	q8tk_term( void );

void	q8tk_grab_add( Q8tkWidget *widget );
void	q8tk_grab_remove( Q8tkWidget *widget );

void	q8tk_box_pack_start( Q8tkWidget *box, Q8tkWidget *widget );
void	q8tk_box_pack_end( Q8tkWidget *box, Q8tkWidget *widget );


Q8tkWidget	*q8tk_window_new( int window_type );

Q8tkWidget	*q8tk_button_new( void );
Q8tkWidget	*q8tk_button_new_with_label( const char *label );



/* TOGGLE/CHECK/RADIO BUTTON �� active ������ɂ́A�K�����̃}�N����ʂ�	*/
/* ��j									*/
/*     Q8tkWidget *toggle = q8tk_tobble_button_new();			*/
/*     if( Q8TK_TOBBLE_BUTTON(toggle)->active ){			*/
/*        :								*/
/*        :								*/
/*     }								*/

#define	Q8TK_TOGGLE_BUTTON(w)	(&((w)->stat.button))



Q8tkWidget	*q8tk_toggle_button_new( void );
Q8tkWidget	*q8tk_toggle_button_new_with_label( const char *label );
void		q8tk_toggle_button_set_state( Q8tkWidget *widget, int status );

Q8tkWidget	*q8tk_check_button_new( void );
Q8tkWidget	*q8tk_check_button_new_with_label( const char *label );

Q8tkWidget	*q8tk_radio_button_new( Q8List *list );
Q8tkWidget	*q8tk_radio_button_new_with_label( Q8List *list,
						   const char *label );
Q8List		*q8tk_radio_button_group( Q8tkWidget *radio_button );


Q8tkWidget	*q8tk_label_new( const char *label );
void		q8tk_label_set( Q8tkWidget *w, const char *label );

Q8tkWidget	*q8tk_frame_new( const char *label );
void		q8tk_frame_set_shadow_type( Q8tkWidget *frame, int shadow_type );

Q8tkWidget	*q8tk_hbox_new( void );
Q8tkWidget	*q8tk_vbox_new( void );

Q8tkWidget	*q8tk_notebook_new( void );
void		q8tk_notebook_append( Q8tkWidget *notebook,
				      Q8tkWidget *widget, const char *label );
int		q8tk_notebook_current_page( Q8tkWidget *notebook );
void		q8tk_notebook_set_page( Q8tkWidget *notebook, int page_num );
void		q8tk_notebook_next_page( Q8tkWidget *notebook );
void		q8tk_notebook_prev_page( Q8tkWidget *notebook );


Q8tkWidget	*q8tk_vseparator_new( void );
Q8tkWidget	*q8tk_hseparator_new( void );

Q8tkWidget	*q8tk_combo_new( void );
void		q8tk_combo_popdown_strings( Q8tkWidget *combo, Q8List *list );
const	char	*q8tk_combo_get_text( Q8tkWidget *combo );
void		q8tk_combo_set_text( Q8tkWidget *combo, const char *text );

Q8tkWidget	*q8tk_list_new( void );
void		q8tk_list_append_items( Q8tkWidget *wlist, Q8List *list );
void		q8tk_list_clear_items( Q8tkWidget *wlist, int start, int end );
void		q8tk_list_select_item( Q8tkWidget *wlist, int item );
void		q8tk_list_select_child( Q8tkWidget *wlist, Q8tkWidget *child );

Q8tkWidget	*q8tk_list_item_new( void );
Q8tkWidget	*q8tk_list_item_new_with_label( const char *label );


/* ADJUSTMENT �� value �Ȃǂ�����ɂ́A�K�����̃}�N����ʂ�		*/
/* ��j									*/
/*     Q8tkWidget *adj = q8tk_adjustment_new();				*/
/*     val = Q8TK_ADJUSTMENT( adj )->value;				*/

#define	Q8TK_ADJUSTMENT(w)	(&((w)->stat.adj))
Q8tkWidget	*q8tk_adjustment_new( int value, int lower, int upper,
				      int step_increment, int page_increment );
void		q8tk_adjustment_set_value( Q8tkWidget *adj, int value );
void		q8tk_adjustment_set_arrow( Q8tkWidget *adj, int arrow );
void		q8tk_adjustment_set_length( Q8tkWidget *adj, int length );
void		q8tk_adjustment_clamp_page( Q8tkWidget *adj,
					    int lower, int upper );

Q8tkWidget	*q8tk_hscale_new( Q8tkWidget *adjustment );
Q8tkWidget	*q8tk_vscale_new( Q8tkWidget *adjustment );
void		q8tk_scale_set_value_pos( Q8tkWidget *scale, int pos );
void		q8tk_scale_set_draw_value( Q8tkWidget *scale, int draw_value );

Q8tkWidget	*q8tk_scrolled_window_new( Q8tkWidget *hadjustment,
					   Q8tkWidget *vadjustment );
void		q8tk_scrolled_window_set_policy( Q8tkWidget *scrolledw,
						 int hscrollbar_policy,
						 int vscrollber_policy );

Q8tkWidget	*q8tk_entry_new( void );
Q8tkWidget	*q8tk_entry_new_with_max_length( int max );
const	char	*q8tk_entry_get_text( Q8tkWidget *entry );
void		q8tk_entry_set_text( Q8tkWidget *entry, const char *text );
void		q8tk_entry_set_position( Q8tkWidget *entry, int position );
void		q8tk_entry_set_max_length( Q8tkWidget *entry, int max );


Q8tkWidget	*q8tk_option_menu_new( void );
void		q8tk_option_menu_set_menu( Q8tkWidget *option_menu,
					   Q8tkWidget *menu );
void		q8tk_option_menu_remove_menu( Q8tkWidget *option_menu );
void		q8tk_option_menu_set_button( Q8tkWidget *option_menu,
					     int button );
Q8tkWidget	*q8tk_menu_new( void );
void		q8tk_menu_append( Q8tkWidget *menu, Q8tkWidget *widget );
Q8tkWidget	*q8tk_menu_get_active( Q8tkWidget *menu );
void		q8tk_menu_set_active( Q8tkWidget *menu, int index );

Q8tkWidget	*q8tk_radio_menu_item_new( Q8List *list );
Q8tkWidget	*q8tk_radio_menu_item_new_with_label( Q8List *list,
						      const char *label );
Q8List*		q8tk_radio_menu_item_group( Q8tkWidget *radio_menu_item );


/* DIALOG �� vhox, action_area ������ɂ́A�K�����̃}�N����ʂ�		*/
/* ��j									*/
/*     Q8tkWidget *dialog = q8tk_dialog_new();				*/
/*     q8tk_box_pack_start( Q8TK_DIALOGE(dialog)->vbox, button );	*/

#define	Q8TK_DIALOG(w)		(&((w)->stat.window.work->stat.dialog))
Q8tkWidget	*q8tk_dialog_new( void );


/* FILE SELECTION �� ok_button �Ȃǂ�����ɂ́A�K�����̃}�N����ʂ�	*/
/* ��j									*/
/*     Q8tkWidget *fselect = q8tk_file_selection_new( "LOAD" );		*/
/*     q8tk_signal_connect( Q8TK_FILE_SELECTION( fselect )->ok_button,	*/
/*			    func, fselect );				*/

#define	Q8TK_FILE_SELECTION(w)	(&((w)->stat.window.work->stat.fselect))


/* FILE SELECTION �ň�����t�@�C���̒����́A�p�X���܂߂�		*/
/* �ȉ��̃T�C�Y�ȓ��Ɍ�����B����ȏ�͌����s�\			*/

#ifndef	OSD_MAX_FILENAME
#define	Q8TK_MAX_FILENAME	(1024)
#else
#define	Q8TK_MAX_FILENAME	(OSD_MAX_FILENAME)
#endif


Q8tkWidget	*q8tk_file_selection_new( const char *title );
const	char	*q8tk_file_selection_get_filename( Q8tkWidget *fselect );
/* �� �Ԃ�l�́Afilename �� �t�@�C���Ȃ�^�A�f�B���N�g���Ȃ�U */
int		q8tk_file_selection_set_filename( Q8tkWidget *fselect,
						  const char *filename );



Q8tkWidget	*q8k_accel_group_new( void );
void	q8tk_accel_group_attach( Q8tkWidget *accel_group, Q8tkWidget *window );
void	q8tk_accel_group_detach( Q8tkWidget *accel_group, Q8tkWidget *window );
void	q8tk_accel_group_add( Q8tkWidget *accel_group, int accel_key,
			      Q8tkWidget *widget,      const char *signal );


Q8List		*q8_list_append( Q8List *list, void *ptr );
void		q8_list_free( Q8List *list );
Q8List		*q8_list_first( Q8List *list );
Q8List		*q8_list_last( Q8List *list );



void	q8tk_misc_set_placement( Q8tkWidget *widget,
				 int placement_x, int placement_y );
void	q8tk_misc_set_size( Q8tkWidget *widget, int width, int height );

void	q8tk_misc_redraw( void );


void	q8tk_container_add( Q8tkWidget *container, Q8tkWidget *widget );
void	q8tk_container_remove( Q8tkWidget *container, Q8tkWidget *widget );

void	q8tk_widget_show( Q8tkWidget *widget );
void	q8tk_widget_hide( Q8tkWidget *widget );

void	q8tk_widget_destroy( Q8tkWidget *widget );

void	q8tk_widget_grab_default( Q8tkWidget *widget );


/* �� �Ԃ�l�́A���� (�K�� 0 ) */
int	q8tk_signal_connect( Q8tkWidget *widget, const char *name,
			     Q8tkSignalFunc func, void *func_data );
void	q8tk_signal_handlers_destroy( Q8tkWidget *widget );

void	q8tk_main( void );
void	q8tk_main_quit( void );





/* Q8TK ���g����ŕ֗��ȁA�G���Ȋ֐� */

void	sjis2euc( char *euc_p, const char *sjis_p );
void	euc2sjis( char *sjis_p, const char *euc_p );
int	euclen( const char *euc_p );

#endif	/* Q8TK_H_INCLUDED */
