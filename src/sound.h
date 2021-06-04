#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED


extern	int	sound_board;			/* �T�E���h�{�[�h	*/

#define	SD_PORT_44_45	(0x01)			/* �|�[�g 44H�`45H �g�p	*/
#define	SD_PORT_46_47	(0x02)			/* �|�[�g 46H�`47H �g�p	*/
#define	SD_PORT_A8_AD	(0x04)			/* �|�[�g A8H�`ADH �g�p	*/
extern	int	sound_port;			/* �T�E���h�|�[�g�̎��	*/

extern	int	intr_sound_enable;		/*         �����}�X�N���� */




extern	int	sound_ENABLE_A;			/* �T�E���h�^�C�}�[���� */
extern	int	sound_ENABLE_B;
extern	int	sound_LOAD_A;			/* �T�E���h�^�C�}�[�쓮�� */
extern	int	sound_LOAD_B;
extern	int	sound_FLAG_A;			/* FLAG �̏��		*/
extern	int	sound_FLAG_B;

extern	int	sound_TIMER_A;			/* �T�E���h�^�C�}�[�����Ԋu */
extern	int	sound_TIMER_B;

extern	int	sound_prescaler;		/* 1/�v���X�P�[���[ (2,3,6) */

extern	byte	sound_reg[0x100];
extern	int	sound_reg_select;


extern	int	sound2_MSK_TA;		/* TIMER A ���荞�݃}�X�N	*/
extern	int	sound2_MSK_TB;		/* TIMER B ���荞�݃}�X�N	*/
extern	int	sound2_MSK_EOS;		/* EOS     ���荞�݃}�X�N	*/ 
extern	int	sound2_MSK_BRDY;	/* BRDY    ���荞�݃}�X�N	*/ 
extern	int	sound2_MSK_ZERO;	/* ZERO    ���荞�݃}�X�N	*/ 

extern	int	sound2_EN_TA;		/* TIMER A ���荞�݋���		*/
extern	int	sound2_EN_TB;		/* TIMER B ���荞�݋���		*/
extern	int	sound2_EN_EOS;		/* EOS     ���荞�݋���		*/
extern	int	sound2_EN_BRDY;		/* BDRY    ���荞�݋���		*/
extern	int	sound2_EN_ZERO;		/* ZERO    ���荞�݋���		*/

extern	int	sound2_FLAG_EOS;	/* FLAG EOS  �̏��		*/
extern	int	sound2_FLAG_BRDY;	/* FLAG BRDY �̏��		*/
extern	int	sound2_FLAG_ZERO;	/* FLAG ZERO �̏��		*/
extern	int	sound2_FLAG_PCMBSY;	/* FLAG PCMBSY �̏��		*/

extern	byte	sound2_reg[0x100];
extern	int	sound2_reg_select;
extern	byte	*sound2_adpcm;			/* ADPCM�p DRAM (256KB)	*/

extern	int	sound2_repeat;			/* ADPCM ���s�[�g�v���C	*/
extern	int	sound2_intr_base;		/* ADPCM ���荞�݃��[�g	*/
extern	int	sound2_notice_EOS;		/* EOS�`�F�b�N�̗v�s�v	*/



void	sound_init( void );

void	sound_out_reg( byte data );
void	sound_out_data( byte data );
byte	sound_in_status( void );
byte	sound_in_data( int always_sound_II );
void	sound2_out_reg( byte data );
void	sound2_out_data( byte data );
byte	sound2_in_status( void );
byte	sound2_in_data( void );


#endif	/* SOUND_H_INCLUDED */
