#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED


extern	int	frameskip_rate;			/* 画面表示の更新間隔	*/
extern	int	monitor_analog;			/* アナログモニター	*/


typedef struct{
  unsigned	char	blue;			/* Ｂ面輝度 (0～7) << 5	*/
  unsigned	char	red;			/* Ｒ面輝度 (0～7) << 5	*/
  unsigned 	char	green;			/* Ｇ面輝度 (0～7) << 5	*/
  unsigned 	char	padding;
} PC88_PALETTE_T;

extern	PC88_PALETTE_T	vram_bg_palette;	/* 背景パレット	*/
extern	PC88_PALETTE_T	vram_palette[8];	/* 各種パレット	*/

extern	byte	sys_ctrl;			/* OUT[30] SystemCtrl     */
extern	byte	grph_ctrl;			/* OUT[31] GraphCtrl      */
extern	byte	grph_pile;			/* OUT[53] 重ね合わせ     */

extern	int	do_update;			/* 描画の必要あり	*/

extern	int	use_half_interp;		/* 画面サイズ半分(320x200)時に縮小補間する */
extern	int	now_half_interp;		/* 現在HALF時の補間可能 */

extern	int	use_interlace;			/* インターレース表示	*/

extern	int	use_auto_skip;			/* オートフレームスキップ使用する */
extern	int	do_skip_draw;			/* スクリーンへの描画をスキップする */
extern	int	already_skip_draw; 		/* スキップしたか */

#define	SYS_CTRL_80		(0x01)		/* TEXT COLUMN80 / COLUMN40*/
#define	SYS_CTRL_MONO		(0x02)		/* TEXT MONO     / COLOR   */

#define	GRPH_CTRL_200		(0x01)		/* VRAM-MONO 200 / 400 line*/
#define	GRPH_CTRL_64RAM		(0x02)		/* RAM   64K-RAM / ROM-RAM */
#define	GRPH_CTRL_N		(0x04)		/* BASIC       N / N88     */
#define GRPH_CTRL_VDISP		(0x08)		/* VRAM  DISPLAY / HIDE    */
#define GRPH_CTRL_COLOR		(0x10)		/* VRAM  COLOR   / MONO    */
#define	GRPH_CTRL_25		(0x20)		/* TEXT  LINE25  / LINE20  */

#define	GRPH_PILE_TEXT		(0x01)		/* 重ね合わせ 非表示 TEXT  */
#define	GRPH_PILE_BLUE		(0x02)		/*		       B   */
#define	GRPH_PILE_RED		(0x04)		/*		       R   */
#define	GRPH_PILE_GREEN		(0x08)		/*		       G   */



extern	char	screen_update[ 0x4000*2 ];	/* 画面表示差分更新フラグ */
extern	int	screen_update_force;		/* 画面強制更新フラグ	  */
extern	int	screen_update_palette;		/* パレット更新フラグ	  */
extern	int	screen_update_hide;		/* VRAM非表示処理済フラグ */

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

void	blink_ctrl_update( void );	/* ブリンクのワークを更新する	*/

void	screen_buf_init( void );


#endif	/* SCREEN_H_INCLUDED */
