#ifndef Z80_H_INCLUDED
#define Z80_H_INCLUDED



/* --- Z80 CPU �̃G�~�����[�g�\���� --- */

typedef struct{

  pair	AF, BC, DE, HL;			/* �ėp���W�X�^ */
  pair	IX, IY, PC, SP;			/* ��p���W�X�^ */
  pair	AF1,BC1,DE1,HL1;		/*  �� ���W�X�^	*/
  byte	I, R;				/* ���ꃌ�W�X�^ */
  byte	R_saved;			/* R reg �ۑ��p */
  Uchar	IFF,IFF2;			/* IFF1 �AIFF2	*/
  Uchar	IM;				/* �������[�h	*/
  Uchar	HALT;				/* HALT �t���O	*/
  Uchar	discontinue;			/* ���ꒆ�f���� */

  int	icount;				/* ���荞�ݔ����܂ł̃X�e�[�g�� */
  int	state;				/* �����������߂̑��X�e�[�g��	*/
  int	state0;				/*		�V		*/


  Uchar	log;				/* �^�Ȃ�f�o�b�O�p�̃��O���L�^	*/
  Uchar	break_if_halt;			/* HALT���ɏ������[�v���狭���E�o*/

  byte	(*mem_read)(word);		/* ���������[�h�֐�	*/
  void	(*mem_write)(word,byte);	/* ���������C�g�֐�	*/
  byte	(*io_read)(byte);		/* I/O ���͊֐�		*/
  void	(*io_write)(byte,byte);		/* I/O �o�͊֐�		*/

  int	(*intr_update)(void);		/* �������X�V�֐�	*/
  int	(*intr_chk)(void);		/* �����`�F�b�N�֐�	*/

  pair  PC_prev;			/* ���O�� PC (���j�^�p)	*/

} z80arch;


/* z80arch �̊e�R�[���o�b�N�֐����ɂāA
   state + state0 �����݂܂łɏ������� ���X�e�[�g���ƂȂ�B */




/* IFF �̒��g */
#define INT_DISABLE	(0)
#define INT_ENABLE	(1)




extern	void	z80_reset( z80arch *z80 );
extern	int	z80_emu( z80arch *z80, int state_of_exec );
extern	void	z80_debug( z80arch *z80, char *mes );
extern	int	z80_line_disasm( z80arch *z80, word addr );

extern	void	z80_logging( z80arch *z80 );



extern	int	highspeed_flag;

#define	refresh_intr_timing( z80 )	if(highspeed_flag==FALSE) z80.icount=0
#define	set_cpu_dormant( z80 )		z80.icount = 0
					    

#endif		/* Z80_H_INCLUDED */
