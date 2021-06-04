#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED


extern	int	tenkey_emu;		/* 10 KEY �̓��͂𐶐� */
extern	int	cursor_emu;		/* Cursor KEY -> 10 KEY (by funa) */
extern	int	numlock_emu;		/* software NUM lock	*/

extern	int	mouse_mode;		/* �}�E�X 0:No 1:Yes 2:Joy  */

extern	int	cursor_assign;		/* Cursor Key -> �C�ӂ̃L�[ */

enum {
  CURSOR_ASSIGN_LEFT,
  CURSOR_ASSIGN_UP,
  CURSOR_ASSIGN_RIGHT,
  CURSOR_ASSIGN_DOWN,
  CURSOR_ASSIGN_End,
};

enum {					/* F6-F10 KEY �̋@�\	*/
  FN_FUNC,
  FN_FRATE_UP,
  FN_FRATE_DOWN,
  FN_VOLUME_UP,
  FN_VOLUME_DOWN,
  FN_PAUSE,
  FN_RESIZE,
  FN_NOWAIT,
  FN_SPEED_UP,
  FN_SPEED_DOWN,
  FN_MOUSE_HIDE,
  FN_DGA,
  FN_IMAGE_NEXT1,
  FN_IMAGE_PREV1,
  FN_IMAGE_NEXT2,
  FN_IMAGE_PREV2,
  FN_NUMLOCK,
  FN_RESET,
  FN_KANA,
  FN_ROMAJI,
  FN_CAPS,
  FN_KETTEI,
  FN_HENKAN,
  FN_ZENKAKU,
  FN_PC,
  FN_SNAPSHOT,
  FN_SUSPEND,
  FN_end		/* ��`�𑝂₷���́AFN_end �̒��O�ɒǉ����Ă������� */
};

typedef	struct {
  int		num;
  const	char	*str;
  const	char	*str2;
} FKEY_TABLE;
extern	FKEY_TABLE	fkey_table[ FN_end ];

extern	int	function_f6;
extern	int	function_f7;
extern	int	function_f8;
extern	int	function_f9;
extern	int	function_f10;

extern	int	romaji_type;			/* ���[�}���ϊ��̃^�C�v	     */


extern	byte	key_scan[0x10];			/* IN[00-0F] �L�[�X�L����    */

/*extern int	mouse_x;*/			/* �}�E�X���W		     */
/*extern int	mouse_y;*/
extern	int	mouse_dx;			/* �}�E�X���W����(-127�`+127)*/
extern	int	mouse_dy;

extern	int	enable_b2menu;			/* ���N���b�N�Ń��j���[��   */

extern	int	need_focus;			/* �t�H�[�J�X�A�E�g��~���� */

extern	char	*file_rec;			/* �L�[���͋L�^�̃t�@�C���� */
extern	char	*file_pb;			/* �L�[���͍Đ��̃t�@�C���� */


void	keyboard_init( void );
void	scan_keyboard( int );
void	scan_mouse( void );
void	check_mouse( void );

void	scan_focus( void );
void	scan_expose( void );

void	romaji_make_list( void );
void	romaji_clear( void );

void	set_tenkey_emu_effective( void );
void	set_tenkey_emu_invalid( void );
void	set_cursor_emu_effective( void );
void	set_cursor_emu_invalid( void );
void	set_numlock_emu_effective( void );
void	set_numlock_emu_invalid( void );

void	set_cursor_assign_effective( void );
void	set_cursor_assign_invalid( void );
void	set_cursor_assign_key( int index, byte code );
byte	get_cursor_assign_key( int index );

void	SetKeyConfig(int Button ,int Port ,int Bit);

void	KANAPUT();
void	CAPSPUT();

#endif	/* KEYBOARD_H_INCLUDED */
