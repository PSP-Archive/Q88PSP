/************************************************************************/
/*									*/
/* CRTC �� DMAC �̏���							*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "crtcdmac.h"

#include "screen.h"
#include "suspend.h"


int		text_display = TEXT_ENABLE;	/* �e�L�X�g�\���t���O	*/

int		blink_cycle;		/* �_�ł̎���	8/16/24/32	*/
int		blink_counter = 0;	/* �_�Ő���J�E���^		*/



static	int	crtc_command;
static	int	crtc_param_num;

static	byte	crtc_status;
static	byte	crtc_light_pen[2];
static	byte	crtc_load_cursor_position;


	int	crtc_active;		/* CRTC�̏�� 0:CRTC�쓮 1:CRTC��~ */
	int	crtc_intr_mask;		/* CRTC�̊����}�X�N ==3 �ŕ\��	    */
	int	crtc_cursor[2];		/* �J�[�\���ʒu ��\���̎���(-1,-1) */
	byte	crtc_format[5];		/* CRTC �������̃t�H�[�}�b�g	    */


	int	crtc_reverse_display;	/* �^�c���]�\�� / �U�c�ʏ�\��	*/

	int	crtc_skip_line;		/* �^�c1�s��΂��\�� / �U�c�ʏ� */
	int	crtc_cursor_style;	/* �u���b�N / �A���_���C��	*/
	int	crtc_cursor_blink;	/* �^�c�_�ł��� �U�c�_�ł��Ȃ�	*/
	int	crtc_attr_non_separate;	/* �^�cVRAM�AATTR �����݂ɕ���	*/
	int	crtc_attr_color;	/* �^�c�J���[ �U�c����		*/
	int	crtc_attr_non_special;	/* �U�c�s�̏I��� ATTR ������	*/

	int	CRTC_SZ_LINES	   =20;	/* �\�����錅�� (20/25)		*/
#define		CRTC_SZ_COLUMNS	   (80)	/* �\������s�� (80�Œ�)	*/

	int	crtc_sz_lines      =20;	/* ���� (20�`25)		*/
	int	crtc_sz_columns    =80;	/* �s�� (2�`80)			*/
	int	crtc_sz_attrs      =20;	/* ������ (1�`20)		*/
	int	crtc_byte_per_line=120;	/* 1�s������̃����� �o�C�g��	*/
	int	crtc_font_height   =10;	/* �t�H���g�̍��� �h�b�g��(8/10)*/



/******************************************************************************

			���������������� crtc_byte_per_line  ����������������
			������   crtc_sz_columns  ������ ��  crtc_sz_attrs ��
			+-------------------------------+-------------------+
		      ��|				|��		    |
		      ��|	+--+ ��			|��		    |
		      ��|	|  | crtc_font_height	|��		    |
			|	+--+ ��			|		    |
	   CRTC_SZ_LINES|				|crtc_sz_lines	    |
			|				|		    |
		      ��|				|��		    |
		      ��|				|��		    |
		      ��|				|��		    |
			+-------------------------------+-------------------+
			������   CRTC_SZ_COLUMNS  ������ 

	crtc_sz_columns		����	2�`80
	crtc_sz_attrs		������	1�`20
	crtc_byte_per_line	1�s������̃�������	columns + attrs*2
	crtc_sz_lines		�s��	20�`25
	crtc_font_height	�t�H���g�̍����h�b�g��	8/10
	CRTC_SZ_COLUMNS		�\�����錅��	80
	CRTC_SZ_LINES		�\������s��	20/25

******************************************************************************/









/* �Q�l�܂łɁc�c�c 						*/
/*	SORCERIAN          �c 1�s��΂��w��			*/
/*	Marchen Veil       �c �A�g���r���[�g�Ȃ����[�h		*/
/*	Xanadu II (E disk) �c             �V			*/
/*	Wizardry V         �c �m���g�����X�y�A�����g�������[�h	*/


enum{
  CRTC_RESET		= 0,
  CRTC_STOP_DISPLAY	= 0,
  CRTC_START_DISPLAY,
  CRTC_SET_INTERRUPT_MASK,
  CRTC_READ_LIGHT_PEN,
  CRTC_LOAD_CURSOR_POSITION,
  CRTC_RESET_INTERRUPT,
  CRTC_RESET_COUNTERS,
  CRTC_READ_STATUS,
  EndofCRTC
};
#define CRTC_STATUS_VE	(0x10)		/* ��ʕ\���L��		*/
#define CRTC_STATUS_U	(0x08)		/* DMA�A���_�[����	*/
#define CRTC_STATUS_N	(0x04)		/* ���ꐧ�䕶���������� */
#define CRTC_STATUS_E	(0x02)		/* �\���I����������	*/
#define CRTC_STATUS_LP	(0x01)		/* ���C�g�y������ 	*/


/****************************************************************/
/* CRTC�֓����M���𑗂� (OUT 40H,A ... bit3)			*/
/*	���ɃG�~�����[�g�̕K�v�Ȃ��B�B�B�B�B�Ǝv���B		*/
/****************************************************************/
#ifdef	SUPPORT_CRTC_SEND_SYNC_SIGNAL
void	crtc_send_sync_signal( int flag )
{
}
#endif




/****************************************************************/
/*    CRTC �G�~�����[�V����					*/
/****************************************************************/

/*-------- ������ --------*/

void	crtc_init( void )
{
  crtc_out_command( CRTC_RESET << 5 );
  crtc_out_parameter( 0xce );
  crtc_out_parameter( 0x98 );
  crtc_out_parameter( 0x6f );
  crtc_out_parameter( 0x58 );
  crtc_out_parameter( 0x53 );

  crtc_out_command( CRTC_LOAD_CURSOR_POSITION << 5 );
  crtc_out_parameter( 0 );
  crtc_out_parameter( 0 );
}

/*-------- �R�}���h���͎� --------*/

void	crtc_out_command( byte data )
{
  crtc_command = data >> 5;
  crtc_param_num = 0;

  switch( crtc_command ){

  case CRTC_RESET:					/* ���Z�b�g */
    crtc_status &= ~( CRTC_STATUS_VE | CRTC_STATUS_N | CRTC_STATUS_E );
    crtc_active = FALSE;
    set_text_display();
    set_screen_update_force();
    break;

  case CRTC_START_DISPLAY:				/* �\���J�n */
    crtc_reverse_display = data & 0x01;
    crtc_status |= CRTC_STATUS_VE;
    crtc_status &= ~( CRTC_STATUS_U );
    crtc_active = TRUE;
    set_text_display();
    set_screen_update_chg_pal();
    break;

  case CRTC_SET_INTERRUPT_MASK:
    crtc_intr_mask = data & 0x03;
    set_text_display();
    set_screen_update_force();
    break;

  case CRTC_READ_LIGHT_PEN:
    crtc_status &= ~( CRTC_STATUS_LP );
    break;

  case CRTC_LOAD_CURSOR_POSITION:			/* �J�[�\���ݒ� */
    crtc_load_cursor_position = data & 0x01;
    crtc_cursor[ 0 ] = -1;
    crtc_cursor[ 1 ] = -1;
    break;

  case CRTC_RESET_INTERRUPT:
  case CRTC_RESET_COUNTERS:
    crtc_status &= ~( CRTC_STATUS_N | CRTC_STATUS_E );
    break;

  }
}

/*-------- �p�����[�^���͎� --------*/

void	crtc_out_parameter( byte data )
{
  switch( crtc_command ){
  case CRTC_RESET:
    if( crtc_param_num < 5 ){
      crtc_format[ crtc_param_num++ ] = data;
    }

    crtc_skip_line         = crtc_format[2] & 0x80;		/* bool  */

    crtc_attr_non_separate = crtc_format[4] & 0x80;		/* bool */
    crtc_attr_color        = crtc_format[4] & 0x40;		/* bool */
    crtc_attr_non_special  = crtc_format[4] & 0x20;		/* bool */

    crtc_cursor_style      =(crtc_format[2] & 0x40) ?ATTR_REVERSE :ATTR_LOWER;
    crtc_cursor_blink      = crtc_format[2] & 0x20;		/* bool */
    blink_cycle            =(crtc_format[1]>>6) * 8 +8;		/* 8,16,24,48*/

    crtc_sz_lines          =(crtc_format[1] & 0x3f) +1;		/* 1�`25 */
    if     ( crtc_sz_lines <= 20 ) crtc_sz_lines = 20;
    else if( crtc_sz_lines >= 25 ) crtc_sz_lines = 25;
    else                           crtc_sz_lines = 24;

    crtc_sz_columns        =(crtc_format[0] & 0x7f) +2;		/* 2�`80 */
    if( crtc_sz_columns > 80 ) crtc_sz_columns = 80;

    crtc_sz_attrs          =(crtc_format[4] & 0x1f) +1;		/* 1�`20 */
    if     ( crtc_attr_non_special ) crtc_sz_attrs = 0;
    else if( crtc_sz_attrs > 20 )    crtc_sz_attrs = 20;

    crtc_byte_per_line  = crtc_sz_columns + crtc_sz_attrs * 2;	/*column+attr*/

    crtc_font_height    = (crtc_sz_lines>20) ?  8 : 10;
    CRTC_SZ_LINES	= (crtc_sz_lines>20) ? 25 : 20;

    blink_ctrl_update();
    break;

  case CRTC_LOAD_CURSOR_POSITION:
    if( crtc_param_num < 2 ){
      if( crtc_load_cursor_position ){
	crtc_cursor[ crtc_param_num++ ] = data;
      }else{
	crtc_cursor[ crtc_param_num++ ] = -1;
      }
    }
    break;

  }
}

/*-------- �X�e�[�^�X�o�͎� --------*/

byte	crtc_in_status( void )
{
  return crtc_status;
}

/*-------- �p�����[�^�o�͎� --------*/

byte	crtc_in_parameter( void )
{
  byte data = 0xff;

  switch( crtc_command ){
  case CRTC_READ_LIGHT_PEN:
    if( crtc_param_num < 2 ){
      data = crtc_light_pen[ crtc_param_num++ ];
    }
    return data;
  }

  return 0xff;
}





/****************************************************************/
/*    DMAC �G�~�����[�V����					*/
/****************************************************************/

static	int	dmac_flipflop;
	pair	dmac_address[4];
	pair	dmac_counter[4];
	int	dmac_mode;


void	dmac_init( void )
{
  dmac_flipflop = 0;
  dmac_address[0].W = 0;
  dmac_address[1].W = 0;
  dmac_address[2].W = 0xf3c8;
  dmac_address[3].W = 0;
  dmac_counter[0].W = 0;
  dmac_counter[1].W = 0;
  dmac_counter[2].W = 0;
  dmac_counter[3].W = 0;
}


void	dmac_out_mode( byte data )
{
  dmac_flipflop = 0;
  dmac_mode = data;

  set_text_display();
  set_screen_update_force();
}
byte	dmac_in_status( void )
{
  return 0x1f;
}


void	dmac_out_address( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_address[ addr ].B.l=data;
  else                   dmac_address[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
  set_screen_update_force();	/* �{���́Aaddr==2�̎��̂݁c�c�c */
}
void	dmac_out_counter( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_counter[ addr ].B.l=data;
  else                   dmac_counter[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
}


byte	dmac_in_address( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_address[ addr ].B.l;
  else                   data = dmac_address[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}
byte	dmac_in_counter( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_counter[ addr ].B.l;
  else                   data = dmac_counter[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}


/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_crtcdmac_work[]=
{
  { TYPE_INT,	&text_display,		},
  { TYPE_INT,	&blink_cycle,		},
  { TYPE_INT,	&blink_counter,		},
  { TYPE_INT,	&crtc_command,		},
  { TYPE_INT,	&crtc_param_num,	},
  { TYPE_BYTE,	&crtc_status,		},
  { TYPE_BYTE,	&crtc_light_pen[0],	},
  { TYPE_BYTE,	&crtc_light_pen[1],	},
  { TYPE_BYTE,	&crtc_load_cursor_position,	},
  { TYPE_INT,	&crtc_active,		},
  { TYPE_INT,	&crtc_intr_mask,	},
  { TYPE_INT,	&crtc_cursor[0],	},
  { TYPE_INT,	&crtc_cursor[1],	},
  { TYPE_BYTE,	&crtc_format[0],	},
  { TYPE_BYTE,	&crtc_format[1],	},
  { TYPE_BYTE,	&crtc_format[2],	},
  { TYPE_BYTE,	&crtc_format[3],	},
  { TYPE_BYTE,	&crtc_format[4],	},
  { TYPE_INT,	&crtc_reverse_display,	},
  { TYPE_INT,	&crtc_skip_line,	},
  { TYPE_INT,	&crtc_cursor_style,	},
  { TYPE_INT,	&crtc_cursor_blink,	},
  { TYPE_INT,	&crtc_attr_non_separate,},
  { TYPE_INT,	&crtc_attr_color,	},
  { TYPE_INT,	&crtc_attr_non_special,	},
  { TYPE_INT,	&CRTC_SZ_LINES,		},
  { TYPE_INT,	&crtc_sz_lines,		},
  { TYPE_INT,	&crtc_sz_columns,	},
  { TYPE_INT,	&crtc_sz_attrs,		},
  { TYPE_INT,	&crtc_byte_per_line,	},
  { TYPE_INT,	&crtc_font_height,	},
  { TYPE_INT,	&dmac_flipflop,		},
  { TYPE_PAIR,	&dmac_address[0],	},
  { TYPE_PAIR,	&dmac_address[1],	},
  { TYPE_PAIR,	&dmac_address[2],	},
  { TYPE_PAIR,	&dmac_address[3],	},
  { TYPE_PAIR,	&dmac_counter[0],	},
  { TYPE_PAIR,	&dmac_counter[1],	},
  { TYPE_PAIR,	&dmac_counter[2],	},
  { TYPE_PAIR,	&dmac_counter[3],	},
  { TYPE_INT,	&dmac_mode,		},
};





int	suspend_crtcdmac( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_crtcdmac_work, 
			    countof(suspend_crtcdmac_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->crtcdmac ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_crtcdmac(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_crtcdmac_work, 
			   countof(suspend_crtcdmac_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}


void	crtc_dmac_init_at_resume( void )
{
  set_text_display();
  blink_ctrl_update();
}
