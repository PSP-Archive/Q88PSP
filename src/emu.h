#ifndef EMU_H_INCLUDED
#define EMU_H_INCLUDED


extern	int	cpu_timing;			/* CPU �ؑւ��^�C�~���O	*/
extern	int	select_main_cpu;		/* ����MAINCPU���������� */
extern	int	dual_cpu_count;			/* -cpu 1�� CPU �ؑ։� */
extern	int	CPU_1_COUNT;			/* ���́A�����l		*/

extern	int	emu_mode;			/* �G�~�����[�^�������	*/
extern	int	trace_counter;			/* TRACE ���̃J�E���^	*/


enum EmuMode
{
  EXECUTE = 0,
  TRACE,
  STEP,
  TRACE_CHANGE,

  EXECUTE_BP,
  TRACE_BP,
  STEP_BP,
  TRACE_CHANGE_BP,

  QUIT,

  MONITOR,
  MONITOR_MAIN,
  MENU,
  MENU_MAIN,
  PAUSE,
  PAUSE_MAIN,

  EndofMODE
};







typedef struct{					/* �u���[�N�|�C���g���� */
  short	type;
  word	addr;
} break_t;

typedef struct{					/* FDC �u���[�N�|�C���g���� */
  short type;
  short drive;
  short track;
  short sector;
} break_drive_t;

enum BPcpu { BP_MAIN, BP_SUB,                                    EndofBPcpu  };
enum BPtype{ BP_NONE, BP_PC,  BP_READ, BP_WRITE, BP_IN, BP_OUT,  BP_DIAG, 
								 EndofBPtype };

#define	NR_BP			(10)		/* �u���[�N�|�C���g�̐�   */
#define	BP_NUM_FOR_SYSTEM	(9)		/* �V�X�e�����g��BP�̔ԍ� */
extern	break_t	break_point[2][NR_BP];
extern  break_drive_t break_point_fdc[NR_BP];

int	check_break_point_PC( void );		/* BP(PC)���ݒ肵�Ă���ΐ^ */








	/**** �֐� ****/

void	emu( void );


#endif	/* EMU_H_INCLUDED */
