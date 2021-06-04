#ifndef Q8TK_COMMON_H_INCLUDED
#define Q8TK_COMMON_H_INCLUDED


/*--------------------------------------------------------------
 * ���j���[�\���p�̉��
 *--------------------------------------------------------------*/

#define	Q8GR_SCREEN_X	(80)
#define	Q8GR_SCREEN_Y	(25)

typedef	struct{
  Uint	background:	4;	/* �w�i�p���b�g�R�[�h (0�`15)		*/
  Uint	foreground:	4;	/* �\���p���b�g�R�[�h (0�`15)		*/
  Uint	reverse:	1;	/* ���]�\��		�ʏ�=0 ���]=1	*/
  Uint	underline:	1;	/* �A���_�[���C��	�Ȃ�=0 ����=1	*/
  Uint	font_type:	2;	/* �t�H���g�^�C�v (���Q��)		*/
  Uint	addr:		16;	/* ����ROM �A�h���X			*/
} T_Q8GR_SCREEN;

extern	T_Q8GR_SCREEN	menu_screen[ Q8GR_SCREEN_Y ][ Q8GR_SCREEN_X ];

enum FontType{		/* menu_screen[][].font �̒l */
  FONT_ANK,			/* ASCII (Alphabet, Number, Kana etc)	*/
  FONT_LOW,			/* ����  (������)			*/
  FONT_HIGH,			/* ����  (�E����)			*/
  EndofFontType
};


/*--------------------------------------------------------------
 * �p���b�g�R�[�h
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
 * �L�[�R�[�h ( Q8TK ��p�̓���L�[ )
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
 * �A�W���X�g�����g�̕\�����
 *--------------------------------------------------------------*/
typedef	struct	_Q8Adjust	Q8Adjust;
struct	_Q8Adjust{
  int	value;			/* ���ݒl */
  int	lower;			/* �ŏ��l */
  int	upper;			/* �ő�l */
  int	step_increment;		/* ����(��) */
  int	page_increment;		/*     (��) */
  int	max_length;		/* �o�[�T�C�Y(���)�A0�Ŏ��� */
  int	x, y;			/* �\���� : ���W           */
  int	length;			/*	  : �X�P�[���T�C�Y */
  int	pos;			/*        : �X���C�_�[�ʒu */
  int	horizontal;		/*        : TRUE�Ő����z�u */
  int	arrow;			/*        : TRUE�Ŗ�󂠂� */
  float	scale;			/*        : �\���{��	   */
};


#endif	/* Q8TK_COMMON_H_INCLUDED */
