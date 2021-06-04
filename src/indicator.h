#ifndef INDICATOR_H_INCLUDED
#define INDICATOR_H_INCLUDED


/*
   こんな感じにしたかったんだけど、ビットフィールドはやめました・・・
   struct{
     Uint  message_allways:	 1;	メッセージ常時表示
     Uint  message_timer:	10;	メッセージ表示時間
     Uint  message_id:		 4;	メッセージ番号
     Uint  message:		 1;	メッセージ表示の ON/OFF
     Uint  fdd_stat:		 8;	表示内容
     Uint  :			 5;
     Uint  fdd_pos:		 2;	FDD状態の表示位置 0～3 = 左上～右下
     Uint  fdd_disp:		 1;	FDD状態表示の ON/OFF
   } indicator_flag;
*/

#define	INDICATE_MES		(0x00010000)
#define	INDICATE_FDD		(0x00000001)

#define	INDICATE_FDD_POS	(0x00000006)
#define	INDICATE_FDD_MASK	(INDICATE_FDD_POS|INDICATE_FDD)

#define	INDICATE_FDD_STAT	(0x0000ff00)



#define	INDICATE_MES_MASK	(0xffff0000)
#define	INDICATE_MES_ALLWAYS	(0x80000000)
#define	INDICATE_MES_TIME	(0x7fe00000)
#define	INDICATE_MES_TIME_SHIFT	(21)

#define	INDICATE_MES_ID		(0x001e0000)
#define	INDICATE_MED_ID_SHIFT	(17)

#define	INDICATE_MES_ID_FRATE		(0)
#define	INDICATE_MES_ID_VOL		(1)
#define	INDICATE_MES_ID_PAUSE		(2)
#define	INDICATE_MES_ID_WAIT_ON		(3)
#define	INDICATE_MES_ID_WAIT_OFF	(4)
#define	INDICATE_MES_ID_WAIT		(5)
#define	INDICATE_MES_ID_DR1_EMP		(6)
#define	INDICATE_MES_ID_DR1_CHG		(7)
#define	INDICATE_MES_ID_DR2_EMP		(8)
#define	INDICATE_MES_ID_DR2_CHG		(9)
#define	INDICATE_MES_ID_LOGO		(10)
#define	INDICATE_MES_ID_SUSPEND		(11)


extern	Uint	indicator_flag;		/* 各種状態の表示	*/
extern	int	bootup_logo;		/* タイトルロゴ表示・非表示 */



void	clear_indicator( void );
void	disp_indicator( void );

void	indicate_change_fdd_stat( void );

void	indicate_change_frame( int sign );
void	indicate_change_volume( int sign );
void	indicate_change_wait( int sign );
void	indicate_change_image_empty( int drv );
void	indicate_change_image_change( int drv, int direction );
void	indicate_change_pause( void );
void	indicate_bootup_logo( void );
void	indicate_change_suspend( void );



typedef	struct{
  int	x;		/* 文字列を表示する x座標 (8 ドット単位) */
  int	y;		/* 文字列を表示する y座標 (1 ドット単位) */
  const char *ver;	/* 表示する文字列			 */
} T_INDI_VER;

extern	const	T_INDI_VER indi_ver[];
extern	const	char	   *indi_guide[ NR_LANG ];


#endif	/* INDICATOR_H_INCLUDED */
