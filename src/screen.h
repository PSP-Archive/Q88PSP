#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED


extern	int	frameskip_rate;			/* ��ʕ\���̍X�V�Ԋu	*/
extern	int	monitor_analog;			/* �A�i���O���j�^�[	*/


typedef struct{
  unsigned	char	blue;			/* �a�ʋP�x (0�`7) << 5	*/
  unsigned	char	red;			/* �q�ʋP�x (0�`7) << 5	*/
  unsigned 	char	green;			/* �f�ʋP�x (0�`7) << 5	*/
  unsigned 	char	padding;
} PC88_PALETTE_T;

extern	PC88_PALETTE_T	vram_bg_palette;	/* �w�i�p���b�g	*/
extern	PC88_PALETTE_T	vram_palette[8];	/* �e��p���b�g	*/

extern	byte	sys_ctrl;			/* OUT[30] SystemCtrl     */
extern	byte	grph_ctrl;			/* OUT[31] GraphCtrl      */
extern	byte	grph_pile;			/* OUT[53] �d�ˍ��킹     */

extern	int	do_update;			/* �`��̕K�v����	*/

extern	int	use_half_interp;		/* ��ʃT�C�Y����(320x200)���ɏk����Ԃ��� */
extern	int	now_half_interp;		/* ����HALF���̕�ԉ\ */

extern	int	use_interlace;			/* �C���^�[���[�X�\��	*/

extern	int	use_auto_skip;			/* �I�[�g�t���[���X�L�b�v�g�p���� */
extern	int	do_skip_draw;			/* �X�N���[���ւ̕`����X�L�b�v���� */
extern	int	already_skip_draw; 		/* �X�L�b�v������ */

#define	SYS_CTRL_80		(0x01)		/* TEXT COLUMN80 / COLUMN40*/
#define	SYS_CTRL_MONO		(0x02)		/* TEXT MONO     / COLOR   */

#define	GRPH_CTRL_200		(0x01)		/* VRAM-MONO 200 / 400 line*/
#define	GRPH_CTRL_64RAM		(0x02)		/* RAM   64K-RAM / ROM-RAM */
#define	GRPH_CTRL_N		(0x04)		/* BASIC       N / N88     */
#define GRPH_CTRL_VDISP		(0x08)		/* VRAM  DISPLAY / HIDE    */
#define GRPH_CTRL_COLOR		(0x10)		/* VRAM  COLOR   / MONO    */
#define	GRPH_CTRL_25		(0x20)		/* TEXT  LINE25  / LINE20  */

#define	GRPH_PILE_TEXT		(0x01)		/* �d�ˍ��킹 ��\�� TEXT  */
#define	GRPH_PILE_BLUE		(0x02)		/*		       B   */
#define	GRPH_PILE_RED		(0x04)		/*		       R   */
#define	GRPH_PILE_GREEN		(0x08)		/*		       G   */



extern	char	screen_update[ 0x4000*2 ];	/* ��ʕ\�������X�V�t���O */
extern	int	screen_update_force;		/* ��ʋ����X�V�t���O	  */
extern	int	screen_update_palette;		/* �p���b�g�X�V�t���O	  */
extern	int	screen_update_hide;		/* VRAM��\�������σt���O */

#define	set_screen_update( x )		screen_update[x] = 1
#define	set_screen_update_force()	screen_update_force = TRUE
#define	set_screen_update_chg_pal()	screen_update_force = TRUE, \
					screen_update_palette = TRUE
#define	set_screen_update_hide()	screen_update_hide = FALSE



extern	bit8	*font_ptr;
extern	bit8	font_style[16];
extern	int	font_color;
extern	void	set_font_style( int attr );
#define	get_font_style()	*font_ptr++
#define	get_font_color()	font_color

extern	int	text_attr_flipflop;
extern	Ushort	text_attr_buf[2][2048];




extern	int	frame_counter;
#define	reset_frame_counter()		frame_counter = 0

void	draw_screen( void );
void	redraw_screen( int do_disp_indicator );

void	blink_ctrl_update( void );	/* �u�����N�̃��[�N���X�V����	*/

void	screen_buf_init( void );


#endif	/* SCREEN_H_INCLUDED */
