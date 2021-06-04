#ifndef EMU_H_INCLUDED
#define EMU_H_INCLUDED


extern	int	cpu_timing;			/* CPU 切替えタイミング	*/
extern	int	select_main_cpu;		/* 次にMAINCPUを処理する */
extern	int	dual_cpu_count;			/* -cpu 1時 CPU 切替回数 */
extern	int	CPU_1_COUNT;			/* その、初期値		*/

extern	int	emu_mode;			/* エミュレータ処理状態	*/
extern	int	trace_counter;			/* TRACE 時のカウンタ	*/


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







typedef struct{					/* ブレークポイント制御 */
  short	type;
  word	addr;
} break_t;

typedef struct{					/* FDC ブレークポイント制御 */
  short type;
  short drive;
  short track;
  short sector;
} break_drive_t;

enum BPcpu { BP_MAIN, BP_SUB,                                    EndofBPcpu  };
enum BPtype{ BP_NONE, BP_PC,  BP_READ, BP_WRITE, BP_IN, BP_OUT,  BP_DIAG, 
								 EndofBPtype };

#define	NR_BP			(10)		/* ブレークポイントの数   */
#define	BP_NUM_FOR_SYSTEM	(9)		/* システムが使うBPの番号 */
extern	break_t	break_point[2][NR_BP];
extern  break_drive_t break_point_fdc[NR_BP];

int	check_break_point_PC( void );		/* BP(PC)が設定していれば真 */








	/**** 関数 ****/

void	emu( void );


#endif	/* EMU_H_INCLUDED */
