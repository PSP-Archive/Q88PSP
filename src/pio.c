/************************************************************************/
/*									*/
/* PIO �� ����								*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "pio.h"

#include "pc88cpu.h"

#include "emu.h"
#include "suspend.h"


/* PIO �A�N�Z�X�ɂ��ACPU �̐ؑւ�					*/
/*	  PC88 �ł́A���C��CPU (�{�̑�) �ƃT�uCPU (�f�B�X�N��)�ƂɁA	*/
/*	���ꂼ��CPU �������Ă���B�ʏ�͂ǂ��炩�Е��݂̂��G�~�����[�g	*/
/*	���APIO �̃A�N�Z�X�����������ɁA�������鑤��CPU��ؑւ���B����	*/
/*	�ɂ��A�G�~�����[�g�̏������x���グ�Ă���B			*/
/*									*/
/*	  ����CPU�̐ؑւ��^�C�~���O�����A�ʏ�́APIO �� C�|�[�g�����[�h	*/
/*	�������݂̂ɍs�Ȃ��BPIO �� C�|�[�g�́AA �܂��� B �|�[�g�̑���M	*/
/*	�̍ۂɁA�f�[�^�̏�����ǂݍ��݂̊����������t���O�����Ƃ肷��	*/
/*	�|�[�g�ł���B							*/
/*		(-cpu 0 �I�v�V�������w�肵�����͏�L�̓��������)	*/
/*									*/
/*	  �������A���ɂ� C�|�[�g������ɁA�f�[�^�̃��[�h/���C�g���s��	*/
/*	���\�t�g�����݂���B(2��CPU�̏������x�̓����Ɉˑ����Ă���)	*/
/*	�����������\�t�g�̑΍�Ƃ��āAA/B/C �|�[�g�̂����ꂩ�Ƀ��C��CPU	*/
/*	���A�N�Z�X�������_��������� (�W���ł� 256�X�e�b�v)�A�T�uCPU	*/
/*	������ɏ�������悤�ɂ����B					*/
/*		(-cpu 1 �I�v�V�������w�肵�����͏�L�̓��������)	*/
/*									*/
/*	  ����ł��܂������Ȃ��\�t�g������BCPU�̏������x�Ɉˑ����Ă���	*/
/*	�̂��낤���H �����ŏ펞�A���C��CPU�ƃT�uCPU�����ɏ�������ݒ�	*/
/*	���݂��Ă݂��B							*/
/*		(-cpu 2 �I�v�V�������w�肵�����͏�L�̓��������)	*/
/*									*/
/*	  �Ȃ��������Ȃ��\�t�g���L��B�����ŁA���C��CPU�ƃT�uCPU�����	*/
/*	�ɏ������邪�AA/B �|�[�g�A�N�Z�X���A���肪�|�[�g�A�N�Z�X����	*/
/*	���Ȃ����́A�����ԃE�G�C�g���Ƃ�悤�ɂ��Ă݂��B		*/
/*		(-cpu 3 �I�v�V�������w�肵�����͏�L�̓��������)	*/

/*---------------------------------------------------------------------------*/
/* ����									     */
/*					A  ######## --\/-- ######## A	     */
/*	�E���C�g�͎����̃��[�N��	B  ######## --/\-- ######## B	     */
/*	  �΂��čs�Ȃ��B		CH ####     --\/-- ####	    CH	     */
/*	�E���[�h�͑���̃��[�N		CL     #### --/\--     #### CL	     */
/*	  ����s�Ȃ��B							     */
/*	  �����̃��[�N��READ�ݒ�Ȃ�A					     */
/*	  �����̃��[�N��ǂށB						     */
/*	���A�����[�h�A�A�����C�g�͗�O�������s�Ȃ��B			     */
/*	���|�[�gC�̃��[�h���C�g���͗�O�������s�Ȃ��B			     */
/*---------------------------------------------------------------------------*/


pio_work	pio_AB[2][2], pio_C[2][2];


static	z80arch	*z80[2] = { &z80main_cpu, &z80sub_cpu };


#define	PIO_CHG_CNT		(16)


/*----------------------------------------------------------------------*/
/* PIO ������								*/
/*	PA / PCL ��M �^ PB / PCH ���M					*/
/*----------------------------------------------------------------------*/
void	pio_init( void )
{
  int	side;

  for( side=0; side<2; side++ ){
    pio_AB[ side ][ PIO_PORT_A ].type    = PIO_READ;
    pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;
    pio_AB[ side ][ PIO_PORT_A ].chg_cnt = PIO_CHG_CNT;
    pio_AB[ side ][ PIO_PORT_A ].data    = 0x00;

    pio_AB[ side ][ PIO_PORT_B ].type    = PIO_WRITE;
    pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;
    pio_AB[ side ][ PIO_PORT_B ].chg_cnt = PIO_CHG_CNT;
    pio_AB[ side ][ PIO_PORT_B ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CH ].type    = PIO_WRITE;
    pio_C[ side ][ PIO_PORT_CH ].chg_cnt = 1;
    pio_C[ side ][ PIO_PORT_CH ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CL ].type    = PIO_READ;
    pio_C[ side ][ PIO_PORT_CL ].chg_cnt = 1;
    pio_C[ side ][ PIO_PORT_CL ].data    = 0x00;
  }

}


/* verbose �w�莞�̃��b�Z�[�W�\���}�N��					*/

#define	pio_mesAB( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s : port = %s\n",			\
		 (side==PIO_SIDE_M)?"M":"S", (port==PIO_PORT_A)?"A":"B" )

#define	pio_mesC( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s\n",					\
		 (side==PIO_SIDE_M)?"M":"S" )

/*----------------------------------------------------------------------*/
/* PIO A or B ���烊�[�h						*/
/*	���[�h�̍ۂ̃f�[�^�́A����̑��^�t�̃|�[�g����ǂݏo���B	*/
/*		����̃|�[�g�̐ݒ肪  READ �Ȃ�G���[�\��		*/
/*		�����̃|�[�g�̐ݒ肪 WRITE �Ȃ�G���[�\��		*/
/*		�A�����[�h�̍ۂ́A�J�E���^���J�E���g�_�E������B	*/
/*			�J�E���^�� 0 �Ȃ���߂āA�A�����[�h����B	*/
/*			�J�E���^�� 1 �ȏ�Ȃ�ACPU ��ؑւ���B		*/
/*----------------------------------------------------------------------*/
byte	pio_read_AB( int side, int port )
{
		/* �|�[�g�����s��v */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_READ  ){	/* ����|�[�g�� READ */
    pio_mesAB( "PIO AB READ PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_WRITE ){	/* �����|�[�g�� WRITE*/
    pio_mesAB( "PIO Read from WRITE-PORT" );
    pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
    return (pio_AB[ side ][ port ].data);
  }
		/* �ǂ݂��� */

  if( pio_AB[ side^1 ][ port^1 ].exist == PIO_EXIST ){	/* -- �ŏ��̓ǂ݂��� */

    pio_AB[ side^1 ][ port^1 ].exist   = PIO_EMPTY;
    pio_AB[ side   ][ port   ].chg_cnt = PIO_CHG_CNT;

  }else{						/* -- �A���̓ǂ݂��� */

    switch( cpu_timing ){
    case 1:						/*     1:�T�uCPU�N�� */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	set_cpu_dormant( z80main_cpu );
      } /*No Break*/
    case 0:						/*     0:���̂܂ܓǂ�*/
    case 2:						/*     2:���̂܂ܓǂ�*/
      pio_mesAB( "PIO Read continuously" );
      pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      break;
    case 3:						/*     3:�������΂�*/
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	z80[side]->discontinue = TRUE;
      }else{
	pio_mesAB( "PIO Read continuously" );
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    default:						/*    -1:CPU��ؑւ�*/
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	select_main_cpu ^= 1;
	z80[side]->discontinue = TRUE;
	/*z80sub_cpu.state = 0;*/
      }else{
	pio_mesAB( "PIO Read continuously" );
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    }

  }
  return (pio_AB[ side^1 ][ port^1 ].data);
}


/*----------------------------------------------------------------------*/
/* PIO A or B �Ƀ��C�g							*/
/*	���C�g�́A�����̑��^�����̃|�[�g�ɑ΂��čs�Ȃ��B		*/
/*		����̃|�[�g�̐ݒ肪 WRITE �Ȃ�G���[�\��		*/
/*		�����̃|�[�g�̐ݒ肪  READ �Ȃ�G���[�\��		*/
/*		�A�����C�g�̍ۂ́A�J�E���^���J�E���g�_�E������B	*/
/*			�J�E���^�� 0 �Ȃ���߂āA�A�����C�g����B	*/
/*			�J�E���^�� 1 �ȏ�Ȃ�ACPU ��ؑւ���B		*/
/*----------------------------------------------------------------------*/
void	pio_write_AB( int side, int port, byte data )
{
		/* �|�[�g�����s��v */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* ����̃|�[�g WRITE*/
    pio_mesAB( "PIO AB Write PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_READ ){	/* �����̃|�[�g READ */
    pio_mesAB( "PIO Write to READ-PORT" );
  }
		/* �������� */

  if( pio_AB[ side ][ port ].exist == PIO_EMPTY ){	/* -- �ŏ��̏������� */

    pio_AB[ side ][ port ].exist   = PIO_EXIST;
    pio_AB[ side ][ port ].data    = data;
    pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;

  }else{						/* -- �A���̏������� */

    switch( cpu_timing ){
    case 1:						/*     1:�T�uCPU�N�� */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	set_cpu_dormant( z80main_cpu );
      } /*No Break*/
    case 0:						/*     0:���̂܂܏���*/
    case 2:						/*     2:���̂܂܏���*/
      pio_mesAB( "PIO Write continuously" );
      pio_AB[ side ][ port ].data    = data;
      pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      break;
    case 3:						/*     3:�������΂�*/
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	z80[side]->discontinue = TRUE;
	break;
      }else{
	pio_mesAB( "PIO Write continuously" );
	pio_AB[ side ][ port ].data    = data;
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    default:						/*    -1:CPU��ؑւ� */
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	select_main_cpu ^= 1;
	z80[side]->discontinue = TRUE;
	/*z80sub_cpu.state = 0;*/
      }else{
	pio_mesAB( "PIO Write continuously" );
	pio_AB[ side ][ port ].data    = data;
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    }

  }
  return;
}








/*----------------------------------------------------------------------*/
/* PIO C ���烊�[�h							*/
/*	���[�h�̍ۂ̃f�[�^�́A����̑��^�t�̃|�[�g����ǂݏo���B	*/
/*		����̃|�[�g�̐ݒ肪  READ �Ȃ�G���[�\��		*/
/*		�����̃|�[�g�̐ݒ肪 WRITE �Ȃ�G���[�\��		*/
/*		���[�h�̍ۂɁACPU��ؑւ����������			*/
/*----------------------------------------------------------------------*/
byte	pio_read_C( int side )
{
  byte	data;

		/* �|�[�g�����s��v */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C READ PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C Read from WRITE-PORT" );
  }
		/* ���[�h */

  if( pio_C[ side ][ PIO_PORT_CH ].type == PIO_READ ){
    data  = pio_C[ side^1 ][ PIO_PORT_CL ].data << 4;
  }else{
    data  = pio_C[ side   ][ PIO_PORT_CH ].data << 4;
  }

  if( pio_C[ side ][ PIO_PORT_CL ].type == PIO_READ ){
    data |= pio_C[ side^1 ][ PIO_PORT_CH ].data;
  }else{
    data |= pio_C[ side   ][ PIO_PORT_CL ].data;
  }

  pio_C[ side ][ PIO_PORT_CL ].chg_cnt ^= 1;
  if( pio_C[ side ][ PIO_PORT_CL ].chg_cnt == 0 ){	/* -- �A���̓ǂ݂��� */

    switch( cpu_timing ){
    case 0:						/*     0:CPU��ؑւ�*/
      select_main_cpu ^= 1;
      z80[side]->discontinue = TRUE;
      /*z80sub_cpu.state = 0;*/
      break;
    case 1:						/*     1:�T�uCPU�N�� */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	set_cpu_dormant( z80main_cpu );
      }
      break;
    case 2:						/*     2:�Ȃɂ����Ȃ�*/
    case 3:						/*     3:�Ȃɂ����Ȃ�*/
      break;
    default:						/*    -1:CPU��ؑւ�*/
      select_main_cpu ^= 1;
      z80[side]->discontinue = TRUE;
      break;
    }

  }

  return data;
}


/*----------------------------------------------------------------------*/
/* PIO C �Ƀ��C�g							*/
/*	���C�g�́A�����̑��^�����̃|�[�g�ɑ΂��čs�Ȃ��B		*/
/*		����̃|�[�g�̐ݒ肪 WRITE �Ȃ�G���[�\��		*/
/*		�����̃|�[�g�̐ݒ肪  READ �Ȃ�G���[�\��		*/
/*		���C�g�̍ۂɁACPU��ؑւ����������			*/
/*----------------------------------------------------------------------*/
void	pio_write_C( int side, byte data )
{
  int port;

  if( data & 0x08 ) port = PIO_PORT_CH;
  else              port = PIO_PORT_CL;
  data &= 0x07;

		/* �|�[�g�����s��v */

  if( pio_C[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* ����̃|�[�g WRITE*/
    pio_mesC( "PIO C Write PORT Mismatch" );
  }
  if( pio_C[ side   ][ port   ].type == PIO_READ ){	/* �����̃|�[�g READ */
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* ���C�g */

  if( data & 0x01 ) pio_C[ side ][ port ].data |=  ( 1 << (data>>1) );
  else              pio_C[ side ][ port ].data &= ~( 1 << (data>>1) );

  switch( cpu_timing ){
  case 0:						/*     0:���̂܂܏���*/
  case 2:						/*     2:���̂܂܏���*/
  case 3:						/*     3:���̂܂܏���*/
    break;
  case 1:						/*     1:�T�uCPU�N�� */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      set_cpu_dormant( z80main_cpu );
    }
    break;
  default:						/*    -1:CPU��ؑւ� */
    select_main_cpu ^= 1;
    /*z80sub_cpu.state = 0;*/
    if( side==PIO_SIDE_M ){
      dual_cpu_count = 1;
      set_cpu_dormant( z80main_cpu );
    }else{
      set_cpu_dormant( z80sub_cpu );
    }
    break;
  }
  return;
}


/*--------------------------------------------------------------*/
/* ���� Port C �ɏ�������					*/
/*--------------------------------------------------------------*/
void	pio_write_C_direct( int side, byte data )
{
		/* �|�[�g�����s��v */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C WRITE PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* ���C�g */

  pio_C[ side ][ PIO_PORT_CH ].data = data >> 4;
  pio_C[ side ][ PIO_PORT_CL ].data = data & 0x0f;

  switch( cpu_timing ){
  case 0:						/*     0:���̂܂܏���*/
  case 2:						/*     2:���̂܂܏���*/
  case 3:						/*     3:���̂܂܏���*/
    break;
  case 1:						/*     1:�T�uCPU�N�� */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      set_cpu_dormant( z80main_cpu );
    }
    break;
  default:						/*    -1:CPU��ؑւ� */
    select_main_cpu ^= 1;
    /*z80sub_cpu.state = 0;*/
    if( side==PIO_SIDE_M ){
      dual_cpu_count = 1;
      set_cpu_dormant( z80main_cpu );
    }else{
      set_cpu_dormant( z80sub_cpu );
    }
    break;
  }
  return;
}







/*----------------------------------------------------------------------*/
/* PIO �ݒ�								*/
/*	PA / PB / PCH / PCL �̑���M���w��B				*/
/*	���[�h��ݒ� (���[�h�� 0 �Ɍ���B�ڍוs��)			*/
/*----------------------------------------------------------------------*/
void	pio_set_mode( int side, byte data )
{
  if( data & 0x60 ){
    if( verbose_pio )
      printf("PIO mode A & CH not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>5)&0x3 );
  }
	/* PIO A */

  if( data & 0x10 ){
    pio_AB[ side ][ PIO_PORT_A ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_A ].type   = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_A ].data    = 0;
  pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;
  pio_AB[ side ][ PIO_PORT_A ].chg_cnt = PIO_CHG_CNT;

	/* PIO C-H */

  if( data & 0x08 ){
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CH ].data    = 0;
  pio_C[ side ][ PIO_PORT_CH ].chg_cnt = 1;

  if( data & 0x04 ){
    if( verbose_pio )
      printf("PIO mode B & CL not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>2)&0x1 );
  }
	/* PIO B */

  if( data & 0x02 ){
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_B ].data    = 0;
  pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;
  pio_AB[ side ][ PIO_PORT_B ].chg_cnt = PIO_CHG_CNT;

	/* PIO C-L */

  if( data & 0x01 ){
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CL ].data    = 0;
  pio_C[ side ][ PIO_PORT_CL ].chg_cnt = 1;

}






/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_pio_work[]=
{
  { TYPE_INT,	&pio_AB[0][0].type,	},
  { TYPE_INT,	&pio_AB[0][0].exist,	},
  { TYPE_INT,	&pio_AB[0][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[0][0].data,	},

  { TYPE_INT,	&pio_AB[0][1].type,	},
  { TYPE_INT,	&pio_AB[0][1].exist,	},
  { TYPE_INT,	&pio_AB[0][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[0][1].data,	},

  { TYPE_INT,	&pio_AB[1][0].type,	},
  { TYPE_INT,	&pio_AB[1][0].exist,	},
  { TYPE_INT,	&pio_AB[1][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[1][0].data,	},

  { TYPE_INT,	&pio_AB[1][1].type,	},
  { TYPE_INT,	&pio_AB[1][1].exist,	},
  { TYPE_INT,	&pio_AB[1][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[1][1].data,	},

  { TYPE_INT,	&pio_C[0][0].type,	},
  { TYPE_INT,	&pio_C[0][0].exist,	},
  { TYPE_INT,	&pio_C[0][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[0][0].data,	},

  { TYPE_INT,	&pio_C[0][1].type,	},
  { TYPE_INT,	&pio_C[0][1].exist,	},
  { TYPE_INT,	&pio_C[0][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[0][1].data,	},

  { TYPE_INT,	&pio_C[1][0].type,	},
  { TYPE_INT,	&pio_C[1][0].exist,	},
  { TYPE_INT,	&pio_C[1][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[1][0].data,	},

  { TYPE_INT,	&pio_C[1][1].type,	},
  { TYPE_INT,	&pio_C[1][1].exist,	},
  { TYPE_INT,	&pio_C[1][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[1][1].data,	},
};


int	suspend_pio( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_pio_work, 
			    countof(suspend_pio_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->pio ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_pio(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_pio_work, 
			   countof(suspend_pio_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}


void	pio_init_at_resume( void )
{
}
