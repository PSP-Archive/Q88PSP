/************************************************************************/
/*									*/
/* ���[�h�ɉ����āA�����֐����Ăяo���B					*/
/*	���[�h�́A���s(EXCUTE)�A���j���[(MENU)�A���j�^�[(MONITOR)�A	*/
/*	��~(PAUSE) �ɑ傫���킩���B					*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "initval.h"
#include "emu.h"

#include "pc88cpu.h"

#include "menu.h"
#include "monitor.h"
#include "pause.h"
#include "wait.h"
#include "suspend.h"




int	cpu_timing	= DEFAULT_CPU;		/* CPU �ؑւ��^�C�~���O	*/
int	select_main_cpu = TRUE;			/* ����MAINCPU���������� */
int	dual_cpu_count	= 0;			/* -cpu 1�� CPU �ؑ։� */
int	CPU_1_COUNT	= 256;			/* ���́A�����l		*/

int	emu_mode	= EXECUTE;		/* �G�~�����[�^�������	*/
int	trace_counter	= 1;			/* TRACE ���̃J�E���^	*/

static	int	balance = 0;			/* -cpu 2/3 �� MAIN/SUB	*/
						/* �ؑւ��^�C�~���O�v�� */



	/*-------- MAIN CPU / SUB CPU �� 1���ߏ������� --------*/

#define	main_exec(x)	z80_emu( &z80main_cpu, (x) )
#define	sub_exec(x)	z80_emu( &z80sub_cpu, (x) )

	/*-------- ���ߏ�����ɁAPC �� BP ���`�F�b�N���� --------*/

#define	main_exec_bp()							\
		do{							\
		  balance -= main_exec(1);				\
		  for( i=0; i<NR_BP; i++ ){				\
		    if( break_point[BP_MAIN][i].type==BP_PC &&		\
		        break_point[BP_MAIN][i].addr==z80main_cpu.PC.W ){\
		      printf( "*** Break at %04x *** ( MAIN[#%d] : PC )\n",\
			      z80main_cpu.PC.W, i+1 );			\
		      emu_mode = MONITOR;				\
		      z80_debug( &z80main_cpu, "[MAIN CPU]\n" );	\
		      if(i==BP_NUM_FOR_SYSTEM){				\
			break_point[BP_MAIN][i].type=BP_NONE;		\
		      }							\
		    }							\
		  }							\
		}while(0)

#define	sub_exec_bp()							\
		do{							\
		  balance += sub_exec(1);				\
		  for( i=0; i<NR_BP; i++ ){				\
		    if( break_point[BP_SUB][i].type == BP_PC &&		\
		        break_point[BP_SUB][i].addr == z80sub_cpu.PC.W ){\
		      printf( "*** Break at %04x *** ( SUB[#%d] : PC )\n",\
			      z80sub_cpu.PC.W, i+1 );			\
		      emu_mode = MONITOR;				\
		      z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );		\
		      if(i==BP_NUM_FOR_SYSTEM){				\
			break_point[BP_SUB][i].type=BP_NONE;		\
		      }							\
		    }							\
		  }							\
		}while(0)


break_t	break_point[2][NR_BP];		/* �u���[�N�|�C���g		*/
break_drive_t break_point_fdc[NR_BP];	/* FDC �u���[�N�|�C���g		*/


/****************************************************************/
/* �G�~�����[�g�����̐���					*/
/*	MAIN/SUB CPU �ؑւ� �^ �f�o�b�O���[�h�̈ڍs �Ȃ�	*/
/*								*/
/*	-cpu 2 �̎��́AMAIN/SUB �����݂ɏ�������̂����A	*/
/*	��萳�m�ɏ������邽�߂ɁA�����������߂̃X�e�[�g����	*/
/*	�����ɂȂ�悤�Ɍ��݂ɏ�������悤�ɂ��Ă���B		*/
/****************************************************************/
void	emu( void )
{
  int	i, j, exit_flag, counter, wk;


	/* �u���[�N�|�C���g�̃��[�N������ (���j�^�[���[�h�p) */

  for( j=0; j<2; j++ )
    for( i=0; i<NR_BP; i++ )
      break_point[j][i].type = BP_NONE;

  for( i=0; i<NR_BP; i++ )
    break_point_fdc[i].type = BP_NONE;


  exit_flag = FALSE;


  while( exit_flag==FALSE ){

    switch( emu_mode ){

    /* ---------------------------------------------------------------- */
    case EXECUTE:				/* �����ɏ������J��Ԃ� */

      switch( cpu_timing ){

      case 0:
	while( 1 ){
	  if( select_main_cpu ) main_exec(0);
	  else                  sub_exec(0);
	  if( emu_mode!=EXECUTE ) break;
	}
	break;

      case 1:
	while( 1 ){
	  if( dual_cpu_count==0 ) main_exec(0);
	  else                    main_exec(1);
	  if( dual_cpu_count )    sub_exec(1);
	  if( emu_mode!=EXECUTE ) break;
	}
	break;

      case 2: case 3:
	while( 1 ){
	  if( balance >= 0 ){ balance -= main_exec(1); }
	  if( balance < 0  ){ balance += sub_exec(1);  }
	  if( emu_mode!=EXECUTE ) break;
	}
	break;

      default:	/* ver 0.3.0 �ȑO�́A-cpu 1 ���� */
	while( 1 ){
	  if( select_main_cpu ) main_exec(0);
	  else                  sub_exec(0);
	  if( emu_mode!=EXECUTE ) break;
	}
	break;
      }
      break;



    /* -------------------------------------------------------------*/
    case TRACE:					/* �����߁A�������� */
    case STEP:					/* 1 ���߁A�������� */

      if( emu_mode==TRACE ) counter = trace_counter;
      else      /*==STEP */ counter = 1;
      emu_mode = EXECUTE;

      switch( cpu_timing ){

      case 0:
	for( ; counter; counter-- ){
	  if( select_main_cpu ) main_exec(1);
	  else                  sub_exec(1);
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	  if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      case 1:
	for( ; counter; counter-- ){
	                       main_exec(1);
	  if( dual_cpu_count ) sub_exec(1);
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	                       z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  if( dual_cpu_count ) z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      case 2: case 3:
	for( ; counter; counter-- ){
	  if( balance >= 0 ){ balance -= main_exec(1); }
	  if( balance < 0  ){ balance += sub_exec(1);  }
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	  z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      default:	/* ver 0.3.0 �ȑO�́A-cpu 1 ���� */
	for( ; counter; counter-- ){
	  if( select_main_cpu ) main_exec(1);
	  else                  sub_exec(1);
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	  if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      }
      emu_mode = MONITOR;
      break;



    /* ------------------------------------------------------------------*/
    case TRACE_CHANGE:			/* CPU���؂�ւ��܂ŏ��������� */
      if( cpu_timing >= 1 ){
	printf( "command 'trace change' can use when -cpu 0\n");
	emu_mode = MONITOR;
	break;
      }

      emu_mode = EXECUTE;

      wk = select_main_cpu;
      while( wk==select_main_cpu ){
	if( select_main_cpu ) main_exec(0);
	else                  sub_exec(0);
	if( emu_mode!=EXECUTE ) break;
      }
      if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
      emu_mode = MONITOR;
      break;



    /* ==================================================================== */

    /* ---------------------------------------------------------------------*/
    case EXECUTE_BP:		/* �����ɏ������J��Ԃ� (Break Point�Œ�~) */

      emu_mode = EXECUTE;

      switch( cpu_timing ){

      case 0:
	while( 1 ){
	  if( select_main_cpu ) main_exec_bp();
	  else                  sub_exec_bp();
	  if( emu_mode!=EXECUTE ) break;
	}
	break;

      case 1:
	while( 1 ){
	                       main_exec_bp();
	  if( dual_cpu_count ) sub_exec_bp();
	  if( emu_mode!=EXECUTE ) break;
	}
	break;

      case 2: case 3:
	while( 1 ){
	  if( balance >= 0 ){ main_exec_bp(); }
	  if( balance < 0  ){ sub_exec_bp();  }
	  if( emu_mode!=EXECUTE ) break;
	}
	break;

      default:	/* ver 0.3.0 �ȑO�́A-cpu 1 ���� */
	while( 1 ){
	  if( select_main_cpu ) main_exec_bp();
	  else                  sub_exec_bp();
	  if( emu_mode!=EXECUTE ) break;
	}
	break;

      }
      break;



    /* ------------------------------------------------------------------*/
    case TRACE_BP:		/* �����߁A��������  (Break Point�Œ�~) */
    case STEP_BP:		/* 1 ���߁A��������  (Break Point�Œ�~) */

      if( emu_mode==TRACE_BP ) counter = trace_counter;
      else      /*==STEP_BP */ counter = 1;
      emu_mode = EXECUTE;

      switch( cpu_timing ){

      case 0:
	for( ; counter; counter-- ){
	  if( select_main_cpu ) main_exec_bp();
	  else                  sub_exec_bp();
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	  if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      case 1:
	for( ; counter; counter-- ){
	                       main_exec_bp();
	  if( dual_cpu_count ) sub_exec_bp();
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	                       z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  if( dual_cpu_count ) z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      case 2: case 3:
	for( ; counter; counter-- ){
	  if( balance >= 0 ){ main_exec_bp(); }
	  if( balance < 0  ){ sub_exec_bp();  }
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	  z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      default:	/* ver 0.3.0 �ȑO�́A-cpu 1 ���� */
	for( ; counter; counter-- ){
	  if( select_main_cpu ) main_exec_bp();
	  else                  sub_exec_bp();
	  if( emu_mode!=EXECUTE ) counter = 1;
	}
	if( counter==0 ){
	  if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	}
	break;

      }
      emu_mode = MONITOR;
      break;



    /* ----------------------------------------------------------------------*/
    case TRACE_CHANGE_BP:	/* CPU���؂�ւ��܂ŏ���(Break Point�Œ�~)*/
      if( cpu_timing >= 1 ){
	printf( "command 'trace change' can use when -cpu 0\n");
	emu_mode = MONITOR;
	break;
      }

      emu_mode = EXECUTE;

      wk = select_main_cpu;
      while( wk==select_main_cpu ){
	if( select_main_cpu ) main_exec_bp();
	else                  sub_exec_bp();
	if( emu_mode!=EXECUTE ) break;
      }
      if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
      emu_mode = MONITOR;
      break;



    /* ==================================================================== */

    /* ---------------------------------------------------------------------*/
    case MONITOR:				/* ���j�^�[���[�h�Ɉڍs���� */
      monitor_init();
      emu_mode = MONITOR_MAIN;
    case MONITOR_MAIN:
      monitor_main();
      wait_vsync_reset();
      break;


    /* ---------------------------------------------------------------------*/
    case MENU:					/* ���j���[���[�h�Ɉڍs���� */
      menu_init();
      emu_mode = MENU_MAIN;
    case MENU_MAIN:
      menu_main();
      wait_vsync_reset();
      break;


    /* ---------------------------------------------------------------------*/
    case PAUSE:					/* �|�[�Y���[�h�Ɉڍs���� */
      pause_init();
      emu_mode = PAUSE_MAIN;
    case PAUSE_MAIN:
      pause_main();
      wait_vsync_reset();
      break;


    /* ----------------------------------------------------------------*/
    case QUIT:					/* quasi88 �� �I������ */
      exit_flag = TRUE;
      break;

    }
  }
}



/****************************************************************/
/* �u���[�N�|�C���g (�^�C�v PC) �̗L�����`�F�b�N����		*/
/****************************************************************/
int	check_break_point_PC( void )
{
  int	i, j;

  for( i=0; i<NR_BP; i++ ) if( break_point[BP_MAIN][i].type == BP_PC ) break;
  for( j=0; j<NR_BP; j++ ) if( break_point[BP_SUB][j].type  == BP_PC ) break;

  if( i==NR_BP && j==NR_BP ) return FALSE;
  else                       return TRUE;
}






/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_emu_work[] =
{
  { TYPE_INT,	&cpu_timing,		},
  { TYPE_INT,	&select_main_cpu,	},
  { TYPE_INT,	&dual_cpu_count,	},
  { TYPE_INT,	&CPU_1_COUNT,		},
  { TYPE_INT,	&emu_mode,		},
  { TYPE_INT,	&trace_counter,		},
  { TYPE_INT,	&balance,		},
};


int	suspend_emu( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_emu_work, 
			    countof(suspend_emu_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->emu ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_emu(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_emu_work, 
			   countof(suspend_emu_work) );

  if( ret<0 ) return FALSE;

  emu_mode = EXECUTE;

  return TRUE;
}
