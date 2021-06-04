#ifndef INDICATOR_H_INCLUDED
#define INDICATOR_H_INCLUDED


/*
   ����Ȋ����ɂ����������񂾂��ǁA�r�b�g�t�B�[���h�͂�߂܂����E�E�E
   struct{
     Uint  message_allways:	 1;	���b�Z�[�W�펞�\��
     Uint  message_timer:	10;	���b�Z�[�W�\������
     Uint  message_id:		 4;	���b�Z�[�W�ԍ�
     Uint  message:		 1;	���b�Z�[�W�\���� ON/OFF
     Uint  fdd_stat:		 8;	�\�����e
     Uint  :			 5;
     Uint  fdd_pos:		 2;	FDD��Ԃ̕\���ʒu 0�`3 = ����`�E��
     Uint  fdd_disp:		 1;	FDD��ԕ\���� ON/OFF
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


extern	Uint	indicator_flag;		/* �e���Ԃ̕\��	*/
extern	int	bootup_logo;		/* �^�C�g�����S�\���E��\�� */



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
  int	x;		/* �������\������ x���W (8 �h�b�g�P��) */
  int	y;		/* �������\������ y���W (1 �h�b�g�P��) */
  const char *ver;	/* �\�����镶����			 */
} T_INDI_VER;

extern	const	T_INDI_VER indi_ver[];
extern	const	char	   *indi_guide[ NR_LANG ];


#endif	/* INDICATOR_H_INCLUDED */
