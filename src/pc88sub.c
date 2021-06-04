/************************************************************************/
/*									*/
/* PC8801 �T�u�V�X�e��(FDD��)						*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "pc88sub.h"

#include "pc88cpu.h"
#include "fdc.h"
#include "screen.h"	/* state_of_vsync */
#include "intr.h"	/* state_of_vsync */
#include "keyboard.h"	/* state_of_vsync */
#include "memory.h"
#include "pio.h"

#include "emu.h"
#include "suspend.h"



z80arch	z80sub_cpu;			/* Z80 CPU ( sub system )	*/

int	sub_load_rate = 6;		/*				*/

/************************************************************************/
/* �������A�N�Z�X							*/
/*			�������A�N�Z�X�����̕��@�́A�}�����ꂳ���	*/
/*			�����ɂ��A���ǁB				*/
/*				Copyright (c) kenichi kasamatsu		*/
/************************************************************************/
/*----------------------*/
/*    �������E���[�h	*/
/*----------------------*/
byte	sub_mem_read( word addr )
{
#if 0
  if( verbose_io ){
    if( ( 0x2000 <= addr && addr < 0x4000 ) || ( addr & 0x8000 ) ){
      printf("SUB Memory Read BAD %04x\n",addr);
    }
  }
#endif
  return sub_romram[ addr & 0x7fff ];
}


/*----------------------*/
/*     ���������C�g	*/
/*----------------------*/
void	sub_mem_write( word addr, byte data )
{
  if( (addr & 0xc000) == 0x4000 ){

    sub_romram[ addr & 0x7fff ] = data;

  }else{

    if( verbose_io ) printf("SUB Memory Write BAD %04x\n",addr);
    if( (addr & 0x4000 ) == 0x4000 ){
      sub_romram[ addr & 0x7fff ] = data;
    }

  }
}




/************************************************************************/
/* �h�^�n�|�[�g�A�N�Z�X							*/
/************************************************************************/

/*----------------------*/
/*    �|�[�g�E���C�g	*/
/*----------------------*/

void	sub_io_out( byte port, byte data )
{
  switch( port ){

  case 0xf4:				/* �h���C�u���[�h�H 2D/2DD/2HD ? */
    return;
  case 0xf7:				/* �v�����^�|�[�g�o��		*/
    return;
  case 0xf8:				/* �h���C�u���[�^����o��	*/
    return;

  case 0xfb:				/* FDC �f�[�^ WRITE */
    fdc_write( data );
    set_cpu_dormant( z80sub_cpu );
    return;

	/* �o�h�n */

  case 0xfc:
    logpio("   <==%02x\n",data);
    pio_write_AB( PIO_SIDE_S, PIO_PORT_A, data );
    return;
  case 0xfd:
    logpio("   <--%02x\n",data);
    pio_write_AB( PIO_SIDE_S, PIO_PORT_B, data );
    return;
  case 0xfe:
    pio_write_C_direct( PIO_SIDE_S, data );
    return;
  case 0xff:
    if( data & 0x80 ) pio_set_mode( PIO_SIDE_S, data );
    else              pio_write_C( PIO_SIDE_S, data );
    return;

  }



  if( verbose_io ) printf("SUB OUT data %02X to undecoeded port %02XH\n",data,port);

}

/*----------------------*/
/*    �|�[�g�E���[�h	*/
/*----------------------*/

byte	sub_io_in( byte port )
{
  switch( port ){

  case 0xf8:				/* FDC �� TC ���o��	*/
    set_cpu_dormant( z80sub_cpu );
    fdc_TC();
    return 0xff;

  case 0xfa:				/* FDC �X�e�[�^�X ���� */
    set_cpu_dormant( z80sub_cpu );
    return	fdc_status();
  case 0xfb:				/* FDC �f�[�^ READ */
    set_cpu_dormant( z80sub_cpu );
    return	fdc_read();

	/* �o�h�n */

  case 0xfc:
    {
      byte data = pio_read_AB( PIO_SIDE_S, PIO_PORT_A );
      logpio("   ==>%02x\n",data);
      return data;
    }
  case 0xfd:
    {
      byte data = pio_read_AB( PIO_SIDE_S, PIO_PORT_B );
      logpio("   -->%02x\n",data);
      return data;
    }
  case 0xfe:
    return pio_read_C( PIO_SIDE_S );

  }

  if( verbose_io ) printf("SUB IN        from undecoeded port %02XH\n",port);

  return 0xff;
}






/************************************************************************/
/* Peripheral �G�~�����[�V����						*/
/************************************************************************/

/************************************************************************/
/* �}�X�J�u�����荞�݃G�~�����[�g					*/
/************************************************************************/

/*------------------------------*/
/* ������(Z80���Z�b�g���ɌĂ�)	*/
/*------------------------------*/
void	sub_INT_init( void )
{
  FDC_flag = FALSE;
}

/*----------------------------------------------------------------------*/
/* ���荞�݂𐶐�����B�Ɠ����ɁA���̊��荞�݂܂ł́A�ŏ� state ���v�Z	*/
/*	�A��l�́AZ80���������I���̃t���O(TRUE/FALSE)			*/
/*----------------------------------------------------------------------*/
int	sub_INT_update( void )
{
  static int sub_total_state = 0;	/* �T�uCPU�������������ߐ�      */
  int icount;

  icount = fdc_ctrl( z80sub_cpu.state );


	/* �L�[�X�L�������ʕ\���̏����́A���C��CPU������	*/
	/* �s�Ȃ��Ă��邽�߁A-cpu 0 �w�莞�ɁA�T�uCPU�ɐ��䂪	*/
	/* �ڂ����܂ܖ߂��Ă��Ȃ��Ȃ�ƁA���j���[��ʂւ̈ڍs	*/
	/* �Ȃǂ��ł��Ȃ��Ȃ�B					*/
	/* �����ŁA�T�uCPU�ł���莞�ԏ������s�Ȃ����тɁA	*/
	/* �L�[�X�L�����Ɖ�ʕ\�����s�Ȃ����Ƃɂ���B		*/
	/* �������A���x�̓f�B�X�N�A�N�Z�X�Ȃǂ̃T�uCPU�̏�����	*/
	/* ���������ɁA�T�E���h���~�܂�Ȃǂ̕��Q���o�Ă����B	*/
	/* �Ȃ̂ŁA���́u�T�uCPU�ł��L�[�X�L�����v�������s�Ȃ�	*/
	/* �p�x��ύX�ł���悤�ɂ��Ă������B			*/

  if( sub_load_rate && cpu_timing < 2 ){
    sub_total_state += z80sub_cpu.state;
    if( sub_total_state/sub_load_rate >= state_of_vsync ){
      scan_keyboard(1);
      draw_screen();
      sub_total_state = 0;
    }
  }

  z80sub_cpu.state  = 0;
  /*z80sub_cpu.icount = 100000;*/	/* �傫���l�قǂ悢�c�̂��Ȃ� */
  z80sub_cpu.icount = icount;

  if( emu_mode!=EXECUTE ) return 1;
  else                    return 0;
}


/*----------------------------------------------*/
/* �`�F�b�N (�������� 1�X�e�b�v���ɌĂ΂��)	*/
/*----------------------------------------------*/
int	sub_INT_chk( void )
{
  if( FDC_flag ) return 0;

  return -1;
}










/************************************************************************/
/* PC88 �T�u�V�X�e�� ������						*/
/************************************************************************/
void	pc88sub_init( void )
{

	/* Z80 �G�~�����[�^���[�N������ */

  z80_reset( &z80sub_cpu );

  z80sub_cpu.mem_read  = sub_mem_read;
  z80sub_cpu.mem_write = sub_mem_write;
  z80sub_cpu.io_read   = sub_io_in;
  z80sub_cpu.io_write  = sub_io_out;

  z80sub_cpu.intr_update = sub_INT_update;
  z80sub_cpu.intr_chk    = sub_INT_chk;

  z80sub_cpu.break_if_halt = TRUE;

#if	defined( SUB_DISP ) || defined( SUB_FILE )
  z80sub_cpu.log	= TRUE;
#else
  z80sub_cpu.log	= FALSE;
#endif



	/* ���荞�݃G�~�����[�g������ */

  sub_INT_init();


	/* �y���t�F���������� */

  /* fdc_init(); �� drive_init()    �ŏ����ς� */
  /* pio_init(); �� pc88main_init() �ŏ����ς� */
}


/************************************************************************/
/* PC88 �T�u�V�X�e�� �I��						*/
/************************************************************************/
void	pc88sub_term( void )
{
}















/************************************************************************/
/* �u���[�N�|�C���g�֘A							*/
/************************************************************************/
INLINE	void	check_break_point( int type, word addr, char *str )
{
  int	i;

  if ( emu_mode == MONITOR_MAIN ) return; /* ���j�^�[���[�h���̓X���[ */
  for( i=0; i<NR_BP; i++ ){
    if( break_point[BP_SUB][i].type == type &&
        break_point[BP_SUB][i].addr == addr ){
      printf( "*** Break at %04x *** ( SUB[#%d] : %s %04x )\n",
	      z80sub_cpu.PC.W, i+1, str, addr );
      emu_mode = MONITOR;
      break;
    }
  }
}

byte	sub_mem_read_with_BP( word addr )
{
  check_break_point( BP_READ, addr, "READ from" );
  return sub_mem_read( addr );
}

void	sub_mem_write_with_BP( word addr, byte data )
{
  check_break_point( BP_WRITE, addr, "WRITE to" );
  sub_mem_write( addr, data );
}

byte	sub_io_in_with_BP( byte port )
{
  check_break_point( BP_IN, port, "IN from" );
  return sub_io_in( port );
}

void	sub_io_out_with_BP( byte port, byte data )
{
  check_break_point( BP_OUT, port, "OUT to" );
  sub_io_out( port, data );
}




void	pc88sub_break_point( void )
{
  int	i, buf[4];
  for( i=0; i<4; i++ ) buf[i]=0;
  for( i=0; i<NR_BP; i++ ){
    switch( break_point[BP_SUB][i].type ){
    case BP_READ:	buf[0]++;	break;
    case BP_WRITE:	buf[1]++;	break;
    case BP_IN:		buf[2]++;	break;
    case BP_OUT:	buf[3]++;	break;
    }
  }
   
  if( buf[0] ) z80sub_cpu.mem_read  = sub_mem_read_with_BP;
  else         z80sub_cpu.mem_read  = sub_mem_read;

  if( buf[1] ) z80sub_cpu.mem_write = sub_mem_write_with_BP;
  else         z80sub_cpu.mem_write = sub_mem_write;

  if( buf[2] ) z80sub_cpu.io_read   = sub_io_in_with_BP;
  else         z80sub_cpu.io_read   = sub_io_in;

  if( buf[3] ) z80sub_cpu.io_write  = sub_io_out_with_BP;
  else         z80sub_cpu.io_write  = sub_io_out;
}







/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_pc88sub_work[]=
{
  { TYPE_PAIR,	&z80sub_cpu.AF,	},
  { TYPE_PAIR,	&z80sub_cpu.BC,	},
  { TYPE_PAIR,	&z80sub_cpu.DE,	},
  { TYPE_PAIR,	&z80sub_cpu.HL,	},
  { TYPE_PAIR,	&z80sub_cpu.IX,	},
  { TYPE_PAIR,	&z80sub_cpu.IY,	},
  { TYPE_PAIR,	&z80sub_cpu.PC,	},
  { TYPE_PAIR,	&z80sub_cpu.SP,	},
  { TYPE_PAIR,	&z80sub_cpu.AF1,	},
  { TYPE_PAIR,	&z80sub_cpu.BC1,	},
  { TYPE_PAIR,	&z80sub_cpu.DE1,	},
  { TYPE_PAIR,	&z80sub_cpu.HL1,	},
  { TYPE_BYTE,	&z80sub_cpu.I,		},
  { TYPE_BYTE,	&z80sub_cpu.R,		},
  { TYPE_BYTE,	&z80sub_cpu.R_saved,	},
  { TYPE_CHAR,	&z80sub_cpu.IFF,	},
  { TYPE_CHAR,	&z80sub_cpu.IFF2,	},
  { TYPE_CHAR,	&z80sub_cpu.IM,	},
  { TYPE_CHAR,	&z80sub_cpu.HALT,	},
  { TYPE_CHAR,	&z80sub_cpu.discontinue,	},
  { TYPE_INT,	&z80sub_cpu.icount,	},
  { TYPE_INT,	&z80sub_cpu.state,	},
  { TYPE_INT,	&z80sub_cpu.state0,	},
  { TYPE_CHAR,	&z80sub_cpu.log,	},
  { TYPE_CHAR,	&z80sub_cpu.break_if_halt,	},
};


int	suspend_pc88sub( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_pc88sub_work, 
			    countof(suspend_pc88sub_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->pc88sub ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_pc88sub(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_pc88sub_work, 
			   countof(suspend_pc88sub_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}



void	pc88sub_init_at_resume( void )
{

  z80sub_cpu.mem_read  = sub_mem_read;
  z80sub_cpu.mem_write = sub_mem_write;
  z80sub_cpu.io_read   = sub_io_in;
  z80sub_cpu.io_write  = sub_io_out;

  z80sub_cpu.intr_update = sub_INT_update;
  z80sub_cpu.intr_chk    = sub_INT_chk;

  z80sub_cpu.PC_prev   = z80sub_cpu.PC;		/* dummy for monitor */
}

