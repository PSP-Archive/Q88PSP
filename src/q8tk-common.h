#ifndef Q8TK_COMMON_H_INCLUDED
#define Q8TK_COMMON_H_INCLUDED


/*--------------------------------------------------------------
 * メニュー表示用の画面
 *--------------------------------------------------------------*/

#define	Q8GR_SCREEN_X	(80)
#define	Q8GR_SCREEN_Y	(25)

typedef	struct{
  Uint	background:	4;	/* 背景パレットコード (0～15)		*/
  Uint	foreground:	4;	/* 表示パレットコード (0～15)		*/
  Uint	reverse:	1;	/* 反転表示		通常=0 反転=1	*/
  Uint	underline:	1;	/* アンダーライン	なし=0 あり=1	*/
  Uint	font_type:	2;	/* フォントタイプ (下参照)		*/
  Uint	addr:		16;	/* 漢字ROM アドレス			*/
} T_Q8GR_SCREEN;

extern	T_Q8GR_SCREEN	menu_screen[ Q8GR_SCREEN_Y ][ Q8GR_SCREEN_X ];

enum FontType{		/* menu_screen[][].font の値 */
  FONT_ANK,			/* ASCII (Alphabet, Number, Kana etc)	*/
  FONT_LOW,			/* 漢字  (左半分)			*/
  FONT_HIGH,			/* 漢字  (右半分)			*/
  EndofFontType
};


/*--------------------------------------------------------------
 * パレットコード
 *--------------------------------------------------------------*/

#define	Q8GR_PALETTE_FOREGROUND	(0)
#define	Q8GR_PALETTE_BACKGROUND	(1)
#define	Q8GR_PALETTE_LIGHT	(2)
#define	Q8GR_PALETTE_SHADOW	(3)
#define	Q8GR_PALETTE_FONT_FG	(4)
#define	Q8GR_PALETTE_FONT_BG	(5)
#define	Q8GR_PALETTE_SCALE_SLD	(6)
#define	Q8GR_PALETTE_SCALE_BAR	(7)
#define	Q8GR_PALETTE_SCALE_ACT	(8)
#define	Q8GR_PALETTE_DUMMY_09	(9)
#define	Q8GR_PALETTE_DUMMY_10	(10)
#define	Q8GR_PALETTE_DUMMY_11	(11)
#define	Q8GR_PALETTE_DUMMY_12	(12)
#define	Q8GR_PALETTE_DUMMY_13	(13)
#define	Q8GR_PALETTE_DUMMY_14	(14)
#define	Q8GR_PALETTE_DUMMY_15	(15)



/*--------------------------------------------------------------
 * キーコード ( Q8TK 専用の特殊キー )
 *--------------------------------------------------------------*/

#define	Q8TK_KEY_BS		(0x08)
#define	Q8TK_KEY_TAB		(0x09)
#define	Q8TK_KEY_RET		(0x0d)
#define	Q8TK_KEY_ESC		(0x1b)
#define	Q8TK_KEY_RIGHT		(0x1c)
#define	Q8TK_KEY_LEFT		(0x1d)
#define	Q8TK_KEY_UP		(0x1e)
#define	Q8TK_KEY_DOWN		(0x1f)
#define	Q8TK_KEY_PAGE_UP	(0x0e)
#define	Q8TK_KEY_PAGE_DOWN	(0x0f)
#define	Q8TK_KEY_SPACE		((int)' ')

#define	Q8TK_KEY_F1		(0xff01)
#define	Q8TK_KEY_F2		(0xff02)
#define	Q8TK_KEY_F3		(0xff03)
#define	Q8TK_KEY_F4		(0xff04)
#define	Q8TK_KEY_F5		(0xff05)
#define	Q8TK_KEY_F6		(0xff06)
#define	Q8TK_KEY_F7		(0xff07)
#define	Q8TK_KEY_F8		(0xff08)
#define	Q8TK_KEY_F9		(0xff09)
#define	Q8TK_KEY_F10		(0xff10)
#define	Q8TK_KEY_F11		(0xff11)
#define	Q8TK_KEY_F12		(0xff12)


#define	Q8TK_BUTTON_L		(0x01)
#define	Q8TK_BUTTON_R		(0x02)
#define	Q8TK_BUTTON_U		(0x04)
#define	Q8TK_BUTTON_D		(0x08)

#define	Q8TK_BUTTON_OFF		(0)
#define	Q8TK_BUTTON_ON		(1)






/*--------------------------------------------------------------
 * アジャストメントの表示情報
 *--------------------------------------------------------------*/
typedef	struct	_Q8Adjust	Q8Adjust;
struct	_Q8Adjust{
  int	value;			/* 現在値 */
  int	lower;			/* 最小値 */
  int	upper;			/* 最大値 */
  int	step_increment;		/* 増分(小) */
  int	page_increment;		/*     (大) */
  int	max_length;		/* バーサイズ(矢印除)、0で自動 */
  int	x, y;			/* 表示時 : 座標           */
  int	length;			/*	  : スケールサイズ */
  int	pos;			/*        : スライダー位置 */
  int	horizontal;		/*        : TRUEで水平配置 */
  int	arrow;			/*        : TRUEで矢印あり */
  float	scale;			/*        : 表示倍率	   */
};


#endif	/* Q8TK_COMMON_H_INCLUDED */
