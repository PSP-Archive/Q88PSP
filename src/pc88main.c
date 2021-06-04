/************************************************************************/
/*									*/
/* PC8801 ���C���V�X�e��(�{�̑�)					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "quasi88.h"
#include "initval.h"
#include "pc88main.h"

#include "pc88cpu.h"
#include "crtcdmac.h"
#include "screen.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "pio.h"
#include "sound.h"
#include "joystick.h"
#include "fdc.h"		/* disk_ex_drv */

#include "emu.h"
#include "snddrv.h"
#include "suspend.h"

#include "exmem.h"


int	boot_basic	= DEFAULT_BASIC;	/* �N������ BASIC���[�h	*/
int	boot_dipsw      = DEFAULT_DIPSW;	/* �N�����̃f�B�b�v�ݒ�	*/
int	boot_from_rom   = DEFAULT_BOOT;		/* �N���f�o�C�X�̐ݒ�	*/
int	boot_clock_4mhz = DEFAULT_CLOCK;	/* �N������ CPU�N���b�N	*/
int	monitor_15k     = 0x02;			/* 15k ���j�^�[ 2:Yes 0:No  */

char	*file_printer   = NULL;			/* �p�������o�͂̃t�@�C���� */
char	*file_serialout	= NULL;			/* �V���A���o�͂̃t�@�C���� */
char	*file_serialin	= NULL;			/* �V���A�����͂̃t�@�C���� */
char	*file_tapeload  = NULL;			/* �e�[�v���͂̃t�@�C����   */
char	*file_tapesave  = NULL;			/* �e�[�v�o�͂̃t�@�C����   */


z80arch	z80main_cpu;			/* Z80 CPU ( main system )	*/

int	high_mode;			/* �������[�h 1:Yes 0:No	*/


byte	dipsw_1;			/* IN[30] �f�B�b�v�X�C�b�` 1	*/
byte	dipsw_2;			/* IN[31] �f�B�b�v�X�C�b�` 2	*/
byte	ctrl_boot;			/* IN[40] �f�B�X�N�u�[�g���	*/
int	memory_bank;			/* OUT[5C-5F] IN[5C] �������o���N*/
byte	cpu_clock;			/* IN[6E] CPU �N���b�N		*/

byte	common_out_data;		/* OUT[10] PRT/���v		*/
byte	misc_ctrl;			/* I/O[32] �e��Ctrl		*/
byte	ALU1_ctrl;			/* OUT[34] ALU Ctrl 1		*/
byte	ALU2_ctrl;			/* OUT[35] ALU Ctrl 2		*/
byte	ctrl_signal;			/* OUT[40] �R���g���[���M���o�͒l�ۑ�*/
byte	baudrate_sw = DEFAULT_BAUDRATE;	/* I/O[6F] �{�[���[�g		*/
word	window_offset;			/* I/O[70] WINDOW �I�t�Z�b�g	*/
byte	ext_rom_bank;			/* I/O[71] �g��ROM BANK		*/
byte	ext_ram_ctrl;			/* I/O[E2] �g��RAM����		*/
byte	ext_ram_bank;			/* I/O[E3] �g��RAM�Z���N�g	*/

pair	kanji1_addr;			/* OUT[E8-E9] ����ROM(��1) ADDR	*/
pair	kanji2_addr;			/* OUT[EC-ED] ����ROM(��2) ADDR	*/

byte	jisho_rom_bank;			/* OUT[F0] ����ROM�Z���N�g	*/
byte	jisho_rom_ctrl;			/* OUT[F1] ����ROM�o���N	*/


int	calendar_stop = FALSE;		/* ���v��~�t���O		*/
char	calendar_data[7] =		/* ���v��~���� (�N�����j�����b)*/
{ 85, 0, 1, 0, 0, 0, 0, };


int	use_pcg = FALSE;		/* �^�ŁAPCG-8100�T�|�[�g	*/

static	FILE	*fp_so = NULL;		/* �V���A���o�͗p		*/
static	FILE	*fp_si = NULL;		/*	   ���͗p		*/
static	FILE	*fp_to = NULL;		/* �e�[�v�o�͗p			*/
static	FILE	*fp_ti = NULL;		/*       ���͗p			*/

static	int	com_EOF = FALSE;	/* �^�ŁA�V���A������ EOF 	*/
static	int	cmt_EOF = FALSE;	/* �^�ŁA�e�[�v���� EOF   	*/

int	cmt_speed = 0;			/* �e�[�v���x(BPS)�A 0�͎���	*/
int	cmt_intr  = TRUE;		/* �^�ŁA�e�[�v�Ǎ��Ɋ����g�p	*/
int	cmt_wait  = TRUE;		/* �^�ŁA�e�[�v�Ǎ��E�F�C�g����	*/

static byte sio_in_data( void );	/* IN[20] RS232C���� (�f�[�^)       */
static byte sio_in_status( void );	/* IN[21] RS232C���� (����)         */
static byte in_ctrl_signal( void );	/* IN[40] �R���g���[���M������      */

static void sio_out_data( byte );	/* OUT[20] RS232C�o�� (�f�[�^)      */
static void sio_out_command( byte );	/* OUT[21] RS232C�o�� (�R�}���h)    */
static void out_ctrl_signal( byte );	/* OUT[40] �R���g���[���M���o��     */

static void sio_tape_highspeed_load( void );
static void sio_set_intr_base( void );
static void sio_check_cmt_error( void );

#define	sio_tape_readable()	(fp_ti && !cmt_EOF)	/* �e�[�v�Ǎ��H   */
#define	sio_tape_writable()	(fp_to)			/* �e�[�v�����H   */
#define	sio_serial_readable()	(fp_si && !com_EOF)	/* �V���A���Ǎ��H */
#define	sio_serial_writable()	(fp_so)			/* �V���A�������H */


/****************************************************************/
/* ���� BASIC ���[�h						*/
/****************************************************************/
/*
 * ���� BASIC �����́Apeach���ɂ��񋟂���܂����B
 */

int highspeed_mode = FALSE;		/* ���� BASIC ���� ����Ȃ� �^      */
int highspeed_n88rom = FALSE;		/* MAIN-ROM �o���N�I�����A�^	    */
					/* ���̎��A����BASIC �������s����   */

/* ���� BASIC ���[�h�ɓ���Ƃ��̃A�h���X (BIOS�ˑ�����?) */
word highspeed_routine[] = {
    0x6e9a,			/* PSET   */
    0x6eae,			/* LINE   */
    0x6eca,			/* ROLL   */
    0x6ece,			/* CIRCLE */
    0x6eda,			/* PAINT  */
    0x7198,			/* GET@   */
    0x71a6,			/* PUT@   */
    EndofBasicAddr
};


/************************************************************************/
/* �������A�N�Z�X							*/
/*			�������A�N�Z�X�����̕��@�́A�}�����ꂳ���	*/
/*			�����ɂ��A���ǁB				*/
/*				Copyright (c) kenichi kasamatsu		*/
/************************************************************************/

/*
   ���C���������̓o���N�؂�ւ��ɂ���āA�ȉ��̂悤�Ƀ}�b�s���O�����B

   0000	+------++------+				+------+ +------+
	|      ||      |				|      | |      |+
	|      ||      |				|      | |      ||
	|      || MAIN |				|N-    | | EXT  ||
	|      || ROM  |				| BASIC| | RAM  ||
	|      ||      |				|  ROM | |      ||
   6000	+      ++      ++------++------++------++------++      + | (x4) ||
	|      ||      ||Ext.0 ||Ext.1 ||Ext.2 ||Ext.3 ||      | |      ||
   8000	+ MAIN ++------++------++------++------++------++------+ +------+|
	| RAM  ||Window|                                          +------+
   8400	+      ++------+
	|      |
	|      |
   C000	+      +	+------++------++------+                 +------+
	|      |	|      ||      ||      |		 |      |+
	|      |	| VRAM || VRAM || VRAM |		 | ���� ||
   F000	+      ++------+|   B  ||   R  ||   G  |		 | ROM  ||
	|      || High ||      ||      ||      |		 | (x32)||
   FFFF	+------++------++------++------++------+		 +------+|
								  +------+
   �܂�A�傫��������ƁA�ȉ���6�̃G���A�ɕ�������B

	0000H�`5FFFH	MAIN RAM / MAIN ROM / N-BASIC ROM / �g��RAM
	6000H�`7FFFH	MAIN RAM / MAIN ROM / �g��ROM / N-BASIC ROM / �g��RAM
	8000H�`83FFH	MAIN RAM / �E�C���h�E
	8400H�`BFFFH	MAIN RAM
	C000H�`EFFFH	MAIN RAM / VRAM / ����ROM
	F000H�`FFFFH	MAIN RAM / ����RAM / VRAM / ����ROM

   �o���N�؂�ւ����s�Ȃ������ɁA�e�X�̃G���A���ǂ̃o���N�Ɋ��蓖�Ă�ꂽ�̂�
   ���`�F�b�N���A���ۂ̃������A�N�Z�X�͂��̊��蓖�ď��ɂ��s�Ȃ��B
*/



/*------------------------------------------------------*/
/* address : 0x0000 �` 0x7fff �� ���������蓖��		*/
/*		ext_ram_ctrl, ext_ram_bank, grph_ctrl,	*/
/*		ext_rom_bank, misc_ctrl �ɂ��ω�	*/
/*------------------------------------------------------*/
INLINE	void	main_memory_mapping_0000_7fff( void )
{
  highspeed_n88rom = FALSE;	/* �f�t�H���g */

  switch( ext_ram_ctrl ){

  case 0x00:					/* �g��RAM RW�s�� */
    if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
      read_mem_0000_5fff  = &main_ram[ 0x0000 ];
      read_mem_6000_7fff  = &main_ram[ 0x6000 ];
      write_mem_0000_7fff = &main_ram[ 0x0000 ];
    }else{						/* ROM/RAM mode */
      if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
	read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
	read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
      }else{							/*N88 BASIC*/
	read_mem_0000_5fff = &main_rom[ 0x0000 ];
	if( ext_rom_bank&EXT_ROM_NOT ){				/* �ʏ�ROM */
	  read_mem_6000_7fff = &main_rom[ 0x6000 ];
	  highspeed_n88rom = TRUE;
	}else{							/* �g��ROM */
	  read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
	}
      }
      write_mem_0000_7fff = &main_ram[ 0x0000 ];
    }
    break;

  case 0x01:					/* �g��RAM R�� W�s�� */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff = &ext_ram[ ext_ram_bank ][ 0x6000 ];
    }else{
      read_mem_0000_5fff = dummy_rom;
      read_mem_6000_7fff = dummy_rom;
    }
    write_mem_0000_7fff = &main_ram[ 0x0000 ];
    break;

  case 0x10:					/* �g��RAM R�s�� W��  */
		/* buf fix by peach (thanks!) */
    if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
      read_mem_0000_5fff  = &main_ram[ 0x0000 ];
      read_mem_6000_7fff  = &main_ram[ 0x6000 ];
    }else{						/* ROM/RAM mode */
      if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
	read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
	read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
      }else{							/*N88 BASIC*/
	read_mem_0000_5fff = &main_rom[ 0x0000 ];
	if( ext_rom_bank&EXT_ROM_NOT ){				/* �ʏ�ROM */
	  read_mem_6000_7fff = &main_rom[ 0x6000 ];
	  highspeed_n88rom = TRUE;
	}else{							/* �g��ROM */
	  read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
	}
      }
    }
    if( ext_ram_bank < use_extram*4 ){
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }else{
      write_mem_0000_7fff = dummy_ram;
    }
    break;

  case 0x11:					/* �g��RAM RW�� */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff  = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff  = &ext_ram[ ext_ram_bank ][ 0x6000 ];
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }else{
      read_mem_0000_5fff  = dummy_rom;
      read_mem_6000_7fff  = dummy_rom;
      write_mem_0000_7fff = dummy_ram;
    }
    break;
  }
}


/*------------------------------------------------------*/
/* address : 0x8000 �` 0x83ff �� ���������蓖��		*/
/*		grph_ctrl, window_offset �ɂ��ω�	*/
/*------------------------------------------------------*/
INLINE	void	main_memory_mapping_8000_83ff( void )
{
  if( grph_ctrl & ( GRPH_CTRL_64RAM | GRPH_CTRL_N ) ){
    read_mem_8000_83ff  = &main_ram[ 0x8000 ];
    write_mem_8000_83ff = &main_ram[ 0x8000 ];
  }else{
    if( high_mode ){
      if( window_offset <= 0xf000 - 0x400 ){
	read_mem_8000_83ff  = &main_ram[ window_offset ];
	write_mem_8000_83ff = &main_ram[ window_offset ];
      }else if( 0xf000 <= window_offset && window_offset <= 0x10000 - 0x400 ){
	read_mem_8000_83ff  = &main_high_ram[ window_offset - 0xf000 ];
	write_mem_8000_83ff = &main_high_ram[ window_offset - 0xf000 ];
      }else{
	read_mem_8000_83ff  = NULL;
	write_mem_8000_83ff = NULL;
      }
    }else{
      read_mem_8000_83ff  = &main_ram[ window_offset ];
      write_mem_8000_83ff = &main_ram[ window_offset ];
    }
  }
}


/*------------------------------------------------------*/
/* address : 0xc000 �` 0xffff �� ���������蓖��		*/
/*		jisho_rom_ctrl, jisho_rom_bank, 	*/
/*		misc_ctrl �ɂ��ω�			*/
/*------------------------------------------------------*/
INLINE	void	main_memory_mapping_c000_ffff( void )
{
  if( jisho_rom_ctrl ){
    read_mem_c000_efff = &main_ram[ 0xc000 ];
    if( high_mode && (misc_ctrl&MISC_CTRL_TEXT_MAIN) ){
      read_mem_f000_ffff = &main_high_ram[ 0x0000 ];
    }else{
      read_mem_f000_ffff = &main_ram[ 0xf000 ];
    }
  }else{
    read_mem_c000_efff = &jisho_rom[ jisho_rom_bank ][ 0x0000 ];
    read_mem_f000_ffff = &jisho_rom[ jisho_rom_bank ][ 0x3000 ];
  }

  write_mem_c000_efff = &main_ram[ 0xc000 ];
  if( high_mode && (misc_ctrl&MISC_CTRL_TEXT_MAIN) ){
    write_mem_f000_ffff = &main_high_ram[ 0x0000 ];
  }else{
    write_mem_f000_ffff = &main_ram[ 0xf000 ];
  }
}


/*------------------------------------------------------*/
/* address : 0xc000 �` 0xffff �� ���C������VARM�؂�ւ�	*/
/*		misc_ctrl, ALU2_ctrl,			*/
/*		memory_bank �ɂ��ω�			*/
/*------------------------------------------------------*/
static	int	vram_access_way;	/* vram �A�N�Z�X�̕��@	*/
enum VramAccessWay{
  VRAM_ACCESS_BANK,
  VRAM_ACCESS_ALU,
  VRAM_NOT_ACCESS,
  EndofVramAcc
};

INLINE	void	main_memory_vram_mapping( void )
{
  if( misc_ctrl & MISC_CTRL_EVRAM ){		/* �g���A�N�Z�X���[�h */

    /* ���[�h���S���Ŏg�p (port 35H �̕��͂���Ȃ������c) by peach */
    memory_bank = MEMORY_BANK_MAIN;

    if( ALU2_ctrl & ALU2_CTRL_VACCESS ){		/* VRAM�g���A�N�Z�X */
      vram_access_way = VRAM_ACCESS_ALU;
    }else{						/* MAIN RAM�A�N�Z�X */
      vram_access_way = VRAM_NOT_ACCESS;
    }
  }else{					/* �Ɨ��A�N�Z�X���[�h */
    if( memory_bank == MEMORY_BANK_MAIN ){		/* MAIN RAM�A�N�Z�X */
      vram_access_way = VRAM_NOT_ACCESS;
    }else{						/* VRAM�A�N�Z�X     */
      vram_access_way = VRAM_ACCESS_BANK;
    }
  }
}




/*------------------------------*/
/* �ʏ�̂u�q�`�l���[�h		*/
/*------------------------------*/
INLINE	byte	vram_read( word addr )
{
  return main_vram[addr][ memory_bank ];
}

/*------------------------------*/
/* �ʏ�̂u�q�`�l���C�g		*/
/*------------------------------*/
INLINE	void	vram_write( word addr, byte data )
{
  set_screen_update( addr );

  main_vram[addr][ memory_bank ] = data;
}

/*------------------------------*/
/* �`�k�t������u�q�`�l���[�h	*/
/*------------------------------*/
typedef	union {
  bit8		c[4];
  bit32		l;
} ALU_memory;

static	ALU_memory	ALU_buf;
static	ALU_memory	ALU_comp;

#ifdef LSB_FIRST
#define	set_ALU_comp()						\
	do{							\
	  ALU_comp.l = 0;					\
	  if( (ALU2_ctrl&0x01)==0 ) ALU_comp.l |= 0x000000ff;	\
	  if( (ALU2_ctrl&0x02)==0 ) ALU_comp.l |= 0x0000ff00;	\
	  if( (ALU2_ctrl&0x04)==0 ) ALU_comp.l |= 0x00ff0000;	\
	}while(0)
#else
#define	set_ALU_comp()						\
	do{							\
	  ALU_comp.l = 0;					\
	  if( (ALU2_ctrl&0x01)==0 ) ALU_comp.l |= 0xff000000;	\
	  if( (ALU2_ctrl&0x02)==0 ) ALU_comp.l |= 0x00ff0000;	\
	  if( (ALU2_ctrl&0x04)==0 ) ALU_comp.l |= 0x0000ff00;	\
	}while(0)
#endif

INLINE	byte	ALU_read( word addr )
{
  ALU_memory	wk;

  ALU_buf.l  = (main_vram4)[addr];
  wk.l       = ALU_comp.l ^ ALU_buf.l;

  return  wk.c[0] & wk.c[1] & wk.c[2];
}

/*------------------------------*/
/* �`�k�t������u�q�`�l���C�g	*/
/*------------------------------*/
INLINE	void	ALU_write( word addr, byte data )
{
  int	i, mode;

  set_screen_update( addr );

  switch( ALU2_ctrl&ALU2_CTRL_MODE ){

  case 0x00:
    mode = ALU1_ctrl;
    for( i=0;  i<3;  i++, mode>>=1 ){
      switch( mode&0x11 ){
      case 0x00:  main_vram[addr][i] &= ~data;	break;
      case 0x01:  main_vram[addr][i] |=  data;	break;
      case 0x10:  main_vram[addr][i] ^=  data;	break;
      default:					break;
      }
    }
    break;

  case 0x10:
    (main_vram4)[addr] = ALU_buf.l;
    break;

  case 0x20:
    main_vram[addr][0] = ALU_buf.c[1];
    break;

  default:
    main_vram[addr][1] = ALU_buf.c[0];
    break;

  }
}


/*----------------------*/
/*    �������E���[�h	*/
/*----------------------*/
byte	main_mem_read( word addr )
{
  if     ( addr < 0x6000 ) return  read_mem_0000_5fff[ addr ];
  else if( addr < 0x8000 ) return  read_mem_6000_7fff[ addr & 0x1fff ];
  else if( addr < 0x8400 ){
    if( read_mem_8000_83ff ) return  read_mem_8000_83ff[ addr & 0x03ff ];
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) return  main_ram[ addr ];
      else                return  main_high_ram[ addr & 0x0fff ];
    }
  }
  else if( addr < 0xc000 ) return  main_ram[ addr ];
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  return  ALU_read(  addr & 0x3fff );
    case VRAM_ACCESS_BANK: return  vram_read( addr & 0x3fff );
    default:
      if( addr < 0xf000 )  return  read_mem_c000_efff[ addr & 0x3fff ];
      else                 return  read_mem_f000_ffff[ addr & 0x0fff ];
    }
  }
}

/*----------------------*/
/*     �������E���C�g	*/
/*----------------------*/
void	main_mem_write( word addr, byte data )
{
  if     ( addr < 0x8000 ) write_mem_0000_7fff[ addr ]          = data;
  else if( addr < 0x8400 ){
    if( write_mem_8000_83ff ) write_mem_8000_83ff[ addr & 0x03ff ] = data;
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) main_ram[ addr ]               = data;
      else                main_high_ram[ addr & 0x0fff ] = data;
    }
  }
  else if( addr < 0xc000 ) main_ram[ addr ]                     = data;
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  ALU_write( addr & 0x3fff, data );	break;
    case VRAM_ACCESS_BANK: vram_write( addr & 0x3fff, data );	break;
    default:
      if( addr < 0xf000 )  write_mem_c000_efff[ addr & 0x3fff ] = data;
      else                 write_mem_f000_ffff[ addr & 0x0fff ] = data;
    }
  }
}





/************************************************************************/
/* �h�^�n�|�[�g�A�N�Z�X							*/
/************************************************************************/

/*----------------------*/
/*    �|�[�g�E���C�g	*/
/*----------------------*/

void	main_io_out( byte port, byte data )
{
  PC88_PALETTE_T new_pal;

  switch( port ){

	/* �����e�[�v���[�h */
  case 0x00:
    if( use_pcg == FALSE ){
      sio_tape_highspeed_load();
    }
    break;


	/* �v�����^�o�́^�J�����_�N���b�N �o�̓f�[�^ */
  case 0x10:
    common_out_data = data;
    return;


	/* RS-232C�^CMT �o�̓f�[�^ */
  case 0x20:
    sio_out_data( data );
    return;

	/* RS-232C�^CMT ����R�}���h */
  case 0x21:
    sio_out_command( data );
    return;


	/* �V�X�e���R���g���[���o�� */
  case 0x30:
    if( (sys_ctrl^data) & (SYS_CTRL_80|SYS_CTRL_MONO) )
						set_screen_update_chg_pal();
    if( sio_tape_readable() ){
      if( (sys_ctrl & 0x08) && !(data & 0x08) ) sio_check_cmt_error();
    }

    sys_ctrl = data;
    sio_set_intr_base();
/*
printf("CMT %02x, %s: Motor %s: CDS %d\n",data,
       ((data&0x3)==0)?"  600":( ((data&0x30)==0x10)?" 1200":"RS232"),
       ((data&8)==0)?"Off":"On ",(data>>2)&1);
*/
    return;

	/* �O���t�B�b�N�R���g���[���o�� */
  case 0x31:
    if( (grph_ctrl^data) &
	    (GRPH_CTRL_200|GRPH_CTRL_VDISP|GRPH_CTRL_COLOR|GRPH_CTRL_25) ){
						set_screen_update_hide();
						set_screen_update_chg_pal();
    }
    /* M88 �ł͂����Ȃ��Ă�H (peach) */
    /*if ((grph_ctrl^data) & (GRPH_CTRL_64RAM|GRPH_CTRL_N))*/
    /*grph_ctrl = data & (GRPH_CTRL_64RAM|GRPH_CTRL_N);*/
    /*else grph_ctrl = data;*/
    grph_ctrl = data;
    set_text_display();
    main_memory_mapping_0000_7fff();
    main_memory_mapping_8000_83ff();
    return;

	/* �e��ݒ���o�� */
  case 0x32:
    if( (misc_ctrl^data) & MISC_CTRL_ANALOG ) set_screen_update_chg_pal();
    misc_ctrl = data;
    if( sound_port & SD_PORT_44_45 ){
      intr_sound_enable = (data & INTERRUPT_MASK_SOUND) ^ INTERRUPT_MASK_SOUND;
      refresh_intr_timing( z80main_cpu );
    }
    /*SOUND_flag = 0;*/
    main_memory_mapping_0000_7fff();
    main_memory_mapping_c000_ffff();
    main_memory_vram_mapping();
    return;


	/* �g��VRAM���� */
  case 0x34:
    ALU1_ctrl = data;
    return;
  case 0x35:
    ALU2_ctrl = data;
    set_ALU_comp();

    /* �N�����]���R�⃏�[�h���S��,STAR TRADER�ȂǂŎg�p */
    if (data & ALU2_CTRL_VACCESS) memory_bank = MEMORY_BANK_MAIN;
					/* bug fix by peach (thanks!) */

    main_memory_vram_mapping();
    return;


	/* �R���g���[���M���o�� */
  case 0x40:
    out_ctrl_signal( data );
    return;


	/* �T�E���h�o�� */
  case 0x44:
    if( sound_port & SD_PORT_44_45 ) sound_out_reg( data );
    return;
  case 0x45:
    if( sound_port & SD_PORT_44_45 ) sound_out_data( data );
    return;
  case 0x46:
    if( sound_port & SD_PORT_46_47 ) sound2_out_reg( data );
    return;
  case 0x47:
    if( sound_port & SD_PORT_46_47 ) sound2_out_data( data );
    return;


    	/* CRTC�o�� */
  case 0x50:
    crtc_out_parameter( data );
/*printf("CRTC PARM %02x\n",data);*/
    return;
  case 0x51:
    crtc_out_command( data );
/*printf("CRTC CMD %02x\n",data);*/
    return;

	/* �w�i�F�i�f�W�^���j*/
  case 0x52:
    if( data&0x1 ) new_pal.blue  = 7 << 5;
    else           new_pal.blue  = 0;
    if( data&0x2 ) new_pal.red   = 7 << 5;
    else           new_pal.red   = 0;
    if( data&0x4 ) new_pal.green = 7 << 5;
    else           new_pal.green = 0;

    if( new_pal.blue  != vram_bg_palette.blue  ||
	new_pal.red   != vram_bg_palette.red   ||
        new_pal.green != vram_bg_palette.green ){
      vram_bg_palette.blue  = new_pal.blue;
      vram_bg_palette.red   = new_pal.red;
      vram_bg_palette.green = new_pal.green;
      set_screen_update_chg_pal();
    }
    return;

	/* ��ʏd�ˍ��킹 */
  case 0x53:
    grph_pile = data;
    set_text_display();
    set_screen_update_force();
    return;

	/* �p���b�g�ݒ� */
  case 0x54:
    if( data&0x80 &&
	misc_ctrl & MISC_CTRL_ANALOG ){		/* �A�i���O���[�h */
      if( (data & 0x40) == 0 ){
	new_pal.blue  = (data & 0x07) << 5;
	new_pal.red   = (data & 0x38) <<(5-3);
	new_pal.green = vram_bg_palette.green;
      }else{
	new_pal.blue  = vram_bg_palette.blue;
	new_pal.red   = vram_bg_palette.red;
	new_pal.green = (data & 0x07) << 5;
      }
      if( new_pal.blue  != vram_bg_palette.blue  ||
	  new_pal.red   != vram_bg_palette.red   ||
          new_pal.green != vram_bg_palette.green ){
	vram_bg_palette.blue  = new_pal.blue;
	vram_bg_palette.red   = new_pal.red;
	vram_bg_palette.green = new_pal.green;
	set_screen_update_chg_pal();
      }
      return;
    }	/* else no return; (.. continued) */
    /* FALLTHROUGH */
  case 0x55:
  case 0x56:
  case 0x57:
  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
/*printf("PAL %02xH %02x\n",port,data );*/
    if( ! (misc_ctrl&MISC_CTRL_ANALOG) ){	/* �f�W�^�����[�h */

      if( data&0x1 ) new_pal.blue  = 7 << 5;
      else           new_pal.blue  = 0;
      if( data&0x2 ) new_pal.red   = 7 << 5;
      else           new_pal.red   = 0;
      if( data&0x4 ) new_pal.green = 7 << 5;
      else           new_pal.green = 0;

    }else{					/* �A�i���O���[�h */
      if( (data & 0x40) == 0 ){
	new_pal.blue  = (data & 0x07) << 5;
	new_pal.red   = (data & 0x38) <<(5-3);
	new_pal.green = vram_palette[ port-0x54 ].green;
      }else{
	new_pal.green = (data & 0x07) << 5;
	new_pal.red   = vram_palette[ port-0x54 ].red;
	new_pal.blue  = vram_palette[ port-0x54 ].blue;
      }
    }

    if( new_pal.blue  != vram_palette[ port-0x54 ].blue  ||
	new_pal.red   != vram_palette[ port-0x54 ].red   ||
	new_pal.green != vram_palette[ port-0x54 ].green ){
      vram_palette[ port-0x54 ].blue  = new_pal.blue;
      vram_palette[ port-0x54 ].red   = new_pal.red;
      vram_palette[ port-0x54 ].green = new_pal.green;
      set_screen_update_chg_pal();
    }
    return;

    
	/* �������o���N�ؑւ� */
  case 0x5c:
    memory_bank = MEMORY_BANK_GRAM0;
    main_memory_vram_mapping();
    return;
  case 0x5d:
    memory_bank = MEMORY_BANK_GRAM1;
    main_memory_vram_mapping();
    return;
  case 0x5e:
    memory_bank = MEMORY_BANK_GRAM2;
    main_memory_vram_mapping();
    return;
  case 0x5f:
    memory_bank = MEMORY_BANK_MAIN;
    main_memory_vram_mapping();
    return;

	/* DMAC�o�� */

  case 0x60:
  case 0x62:
  case 0x64:
  case 0x66:
    dmac_out_address( (port-0x60)/2, data );
/*printf("DMAC %x ADDR %02x\n",(port-0x60)/2,data );*/
    return;
  case 0x61:
  case 0x63:
  case 0x65:
  case 0x67:
    dmac_out_counter( (port-0x61)/2, data );
/*printf("DMAC %x CNTR %02x\n",(port-0x61)/2,data );*/
    return;
  case 0x68:
    dmac_out_mode( data );
/*printf("DMAC MODE %02x\n",data );*/
    return;


	/* �{�[���[�g */
  case 0x6f:
    if( ROM_VERSION >= '8' ) baudrate_sw = data;	/* FH/MH �ȍ~�ɑΉ� */
    return;


	/* Window �I�t�Z�b�g�A�h���X���o�� */
  case 0x70:
    window_offset = (word)data << 8;
    main_memory_mapping_8000_83ff();
    return;

	/* �g�� ROM �o���N */
  case 0x71:
    ext_rom_bank = data;
    main_memory_mapping_0000_7fff();
    return;

	/* Window �I�t�Z�b�g�A�h���X �C���N�������g */

  case 0x78:
    window_offset += 0x100;
    main_memory_mapping_8000_83ff();
    return;



	/* �T�E���h�o��(�I�v�V����) */
  case 0xa8:
    if( sound_port & SD_PORT_A8_AD ){
      sound_out_reg( data );
    }
    return;
  case 0xa9:
    if( sound_port & SD_PORT_A8_AD ){
      sound_out_data( data );
    }
    return;
  case 0xaa:
    if( sound_port & SD_PORT_A8_AD ){
      intr_sound_enable = (data & INTERRUPT_MASK_SOUND) ^ INTERRUPT_MASK_SOUND;
      refresh_intr_timing( z80main_cpu );
    }
    return;
  case 0xac:
    if( sound_port & SD_PORT_A8_AD ){
      sound2_out_reg( data );
    }
    return;
  case 0xad:
    if( sound_port & SD_PORT_A8_AD ){
      sound2_out_data( data );
    }
    return;


	/* �g�� RAM ���� */
  case 0xe2:
    if( use_extram ){
      ext_ram_ctrl = data & 0x11;
      main_memory_mapping_0000_7fff();
    }
    return;
  case 0xe3:
    if( use_extram ){
      ext_ram_bank = data;
      main_memory_mapping_0000_7fff();
    }
    return;


	/* ���荞�݃��x���̐ݒ� */
  case 0xe4:
    intr_priority = data & 0x08;
    if( intr_priority ) intr_level = 7;
    else                intr_level = data & 0x07;
    refresh_intr_timing( z80main_cpu );
    return;

	/* ���荞�݃}�X�N */
  case 0xe6:
    intr_sio_enable   = data & INTERRUPT_MASK_SIO;
    intr_vsync_enable = data & INTERRUPT_MASK_VSYNC;
    intr_rtc_enable   = data & INTERRUPT_MASK_RTC;

    /* ���}���V�A�΍�c�c */
/*
    if((data & INTERRUPT_MASK_VSYNC) && VSYNC_flag) printf("vsync reset!!\n");
*/
/*  if(data & INTERRUPT_MASK_SIO)   RS232C_flag = FALSE;*/
    if(data & INTERRUPT_MASK_VSYNC) VSYNC_flag  = FALSE;
/*  if(data & INTERRUPT_MASK_RTC)   RTC_flag    = FALSE;*/

    refresh_intr_timing( z80main_cpu );
    return;


	/* �����q�n�l �A�h���X�ݒ� */
  case 0xe8:
    kanji1_addr.B.l = data;
    return;
  case 0xe9:
    kanji1_addr.B.h = data;
    return;

  case 0xea:
  case 0xeb:
    return;

  case 0xec:
    kanji2_addr.B.l = data;
    return;
  case 0xed:
    kanji2_addr.B.h = data;
    return;


	/* ����ROM�̐ݒ� */

  case 0xf0:
    if( use_jisho_rom ){
      jisho_rom_bank = data & JISHO_BANK;
      main_memory_mapping_c000_ffff();
    }
    return;
  case 0xf1:
    if( use_jisho_rom ){
      jisho_rom_ctrl = data & JISHO_NOT_SELECT; 
      main_memory_mapping_c000_ffff();
    }
    return;

	/* �o�h�n */

  case 0xfc:
    logpio(" %02x-->\n",data);
    pio_write_AB( PIO_SIDE_M, PIO_PORT_A, data );
    return;
  case 0xfd:
    logpio(" %02x==>\n",data);
    pio_write_AB( PIO_SIDE_M, PIO_PORT_B, data );
    return;
  case 0xfe:
    pio_write_C_direct( PIO_SIDE_M, data );
    return;
  case 0xff:
    if( data & 0x80 ) pio_set_mode( PIO_SIDE_M, data );
    else              pio_write_C( PIO_SIDE_M, data );
    return;




	/* ���̑��̃|�[�g */

  case 0x90:  case 0x91:  case 0x92:  case 0x93:	/* CD-ROM */
  case 0x94:  case 0x95:  case 0x96:  case 0x97:
  case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
  case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:

  case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:	/* MUSIC & NETWORK */

			  case 0xc2:  case 0xc3:	/* MUSIC */
  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
  case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:

  case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:	/* MUSIC & GP-IB*/
  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
  case 0xd8:						/* GP-IB */

  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:	/* MODEM */

  case 0xb4:  case 0xb5:				/* VIDEO ART */


  case 0xc1:				/* ??? Access in N88-BASIC ver 1.8 */
  case 0xf3:  case 0xf4:  case 0xf8:
  
  case 0xe7:				/* ??? Access in N-BASIC ver 1.8 */

    return;
  }


  if( verbose_io )printf("OUT data %02X to undecoeded port %02XH\n",data,port);

}

/*----------------------*/
/*    �|�[�g�E���[�h	*/
/*----------------------*/
byte	main_io_in( byte port )
{
  switch( port ){

	/* �L�[�{�[�h */
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
  case 0x06:
  case 0x07:
  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
  case 0x0c:
  case 0x0d:
  case 0x0e:
  case 0x0f:
    disk_ex_drv = 0;		/* �L�[���͂Ń��Z�b�g */
#ifdef	USE_KEYBOARD_BUG				/* peach���� */
    {
      int i;
      byte mkey, mkey_old;

      mkey = key_scan[port];
      do {
	mkey_old = mkey;
	for (i = 0; i < 0x10; i++) {
	  if (i != port && key_scan[i] != 0xff) {
	    /* [SHIFT],[CTRL],[GRAPH],[�J�i]�ɂ͓K�p���Ȃ� */
	    if ((i == 0x08 && (mkey | key_scan[i] | 0xf0) != 0xff) ||
		(i != 0x08 && (mkey | key_scan[i])        != 0xff))
	      mkey &= key_scan[i];
	  }
	}
      } while (mkey_old != mkey);
      return(mkey);
    }
#else
    return key_scan[ port ];
#endif


	/* RS-232C�^CMT ���̓f�[�^ */
  case 0x20:
    return sio_in_data();


	/* RS-232C/CMT ���� */
  case 0x21:
    return sio_in_status();


	/* �f�B�b�v�X�C�b�`���� */

  case 0x30:
    return dipsw_1 | 0xc0;
  case 0x31:
    return dipsw_2;

	/* �e��ݒ���� */
  case 0x32:
    return misc_ctrl;

	/* �R���g���[���M������ */
  case 0x40:
    return in_ctrl_signal() | 0xc0 | 0x04;
 /* return in_ctrl_signal() | 0xc0;*/


	/* �T�E���h���� */
	
  case 0x44:
    if( sound_port & SD_PORT_44_45 ) return sound_in_status( );
    else                             return 0xff;
  case 0x45:
    if( sound_port & SD_PORT_44_45 ) return sound_in_data( FALSE );
    else                             return 0xff;
  case 0x46:
    if( sound_port & SD_PORT_46_47 ) return sound2_in_status( );
    else                             return 0xff;
  case 0x47:
    if( sound_port & SD_PORT_46_47 ) return sound2_in_data( );
    else                             return 0xff;


    	/* CRTC���� */
  case 0x50:
/*printf("READ CRTC parm\n");*/
    return crtc_in_parameter( );
  case 0x51:
/*printf("READ CRTC stat\n");*/
    return crtc_in_status( );


	/* �������o���N */
  case 0x5c:
    return (1<<memory_bank) | 0xf8;


	/* DMAC���� */

  case 0x60:
  case 0x62:
  case 0x64:
  case 0x66:
/*printf("READ DMAC addr\n");*/
    return dmac_in_address( (port-0x60)/2 );
  case 0x61:
  case 0x63:
  case 0x65:
  case 0x67:
/*printf("READ DMAC cntr\n");*/
    return dmac_in_counter( (port-0x61)/2 );
  case 0x68:
/*printf("READ DMAC stat\n");*/
    return dmac_in_status( );


	/* CPU �N���b�N */
  case 0x6e:
    if( ROM_VERSION >= '8' ) return cpu_clock | 0x10;	/* FH/MH �ȍ~�ɑΉ� */
    else		     return 0xff;


	/* �{�[���[�g */
  case 0x6f:
    if( ROM_VERSION >= '8' ) return baudrate_sw | 0xf0;	/* FH/MH �ȍ~�ɑΉ� */
    else		     return 0xff;


	/* Window �I�t�Z�b�g�A�h���X���o�� */
  case 0x70:
    return window_offset >> 8;


	/* �g�� ROM �o���N */
  case 0x71:
    return ext_rom_bank;


	/* �g�� RAM ���� */
  case 0xe2:
    if( use_extram ) return ~ext_ram_ctrl | 0xee;
    return 0xff;
  case 0xe3:
    if( use_extram ) return ext_ram_bank;
    return 0xff;


	/* �T�E���h����(�I�v�V����) */
  case 0xa8:
    if( sound_port & SD_PORT_A8_AD ) return sound_in_status( );
    else                             return 0xff;
  case 0xa9:
    if( sound_port & SD_PORT_A8_AD ) return sound_in_data( TRUE );
    else                             return 0xff;
  case 0xaa:
    if( sound_port & SD_PORT_A8_AD ) return intr_sound_enable | 0x7f;
    else                             return 0xff;
  case 0xac:
    if( sound_port & SD_PORT_A8_AD ) return sound2_in_status( );
    else                             return 0xff;
  case 0xad:
    if( sound_port & SD_PORT_A8_AD ) return sound2_in_data( );
    else                             return 0xff;



	/* �����q�n�l �t�H���g���� */
  case 0xe8:
    return kanji_rom[0][kanji1_addr.W][1];
  case 0xe9:
    return kanji_rom[0][kanji1_addr.W][0];

  case 0xec:
    return kanji_rom[1][kanji2_addr.W][1];
  case 0xed:
    return kanji_rom[1][kanji2_addr.W][0];



	/* �o�h�n */

  case 0xfc:
    {
      byte data = pio_read_AB( PIO_SIDE_M, PIO_PORT_A );
      logpio(" %02x<--\n",data);
/*      {
	static byte debug_pio_halt[4] = { 0,0,0,0 };
	debug_pio_halt[0] = debug_pio_halt[1];
	debug_pio_halt[1] = debug_pio_halt[2];
	debug_pio_halt[2] = debug_pio_halt[3];
	debug_pio_halt[3] = data;
	if(debug_pio_halt[0]==0x20&&
	   debug_pio_halt[1]==0x3d&&
	   debug_pio_halt[2]==0x02&&
	   debug_pio_halt[3]==0x00) emu_mode=MONITOR;
      }*/
      return data;
    }
  case 0xfd:
    {
      byte data = pio_read_AB( PIO_SIDE_M, PIO_PORT_B );
      logpio(" %02x<==\n",data);
      return data;
    }
  case 0xfe:
    return pio_read_C( PIO_SIDE_M );




	/* ���̑��̃|�[�g */

  case 0x90:  case 0x91:  case 0x92:  case 0x93:	/* CD-ROM */
  case 0x94:  case 0x95:  case 0x96:  case 0x97:
  case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
  case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:

  case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:	/* MUSIC & NETWORK */

			  case 0xc2:  case 0xc3:	/* MUSIC */
  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
  case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:

  case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:	/* MUSIC & GP-IB*/
  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
  case 0xd8:						/* GP-IB */

  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:	/* MODEM */

  case 0xb4:  case 0xb5:				/* VIDEO ART */


  case 0xc1:				/* ??? Access in N88-BASIC ver 1.8 */
  case 0xf3:  case 0xf4:  case 0xf8:
  
    return 0xff;
#if 0
  case 0xf4:				/* ??? */
    return 0xff;
  case 0xf8:				/* ??? */
    return 0xff;
#endif
  }


  if( verbose_io )printf("IN        from undecoeded port %02XH\n",port);

  return 0xff;
}






/************************************************************************/
/* Peripheral �G�~�����[�V����						*/
/************************************************************************/
/*-----------------------*/
/*      Serial Port	 */
/*-----------------------*/
static	int	sio_instruction;		/* USART �̃R�}���h��� */
static	byte	sio_mode;			/* USART �̐ݒ胂�[�h   */
static	byte	sio_command;			/* USART �̃R�}���h	*/
static	int	sio_data_exist;			/* �Ǎ�����SIO�f�[�^����*/
static	byte	sio_data;			/* SIO�f�[�^            */

static	int	com_X_flow = FALSE;		/* �^�ŁAX�t���[���䒆	*/

static	int	cmt_dummy_read_cnt = 0;
static	int	cmt_is_t88;			/* �^�cT88�A�U�cCMT	*/
static	int	cmt_block_size;			/* �f�[�^�^�O���̃T�C�Y(T88)*/
static	long	cmt_size;			/* �C���[�W�̃T�C�Y	*/

static	int	cmt_skip;			/* �����f�[�^���̓ǂݔ�΂� */
static	int	cmt_skip_data;			/* �ǂݔ�΂�����̃f�[�^   */



/*-------- char *file_tapeload �̃t�@�C���� "rb" �ŊJ�� --------*/

int	sio_open_tapeload( void )
{
  sio_close_tapeload();

  if( file_tapeload ){
    if( (fp_ti = ex_fopen( file_tapeload, "rb" )) == NULL ){
      printf(" (( %s : Tape load image can't open ))\n", file_tapeload );
      return FALSE;
    }
    sio_set_intr_base();
    return sio_tape_rewind();
  }
  return TRUE;
}
void	sio_close_tapeload( void )
{
  if( fp_ti ){ ex_fclose( fp_ti ); fp_ti = NULL; }
  sio_set_intr_base();
}

/*-------- char *file_tapesave �̃t�@�C���� "ab" �ŊJ�� --------*/

int	sio_open_tapesave( void )
{
  sio_close_tapesave();

  if( file_tapesave ){
    if( (fp_to = ex_fopen( file_tapesave, "ab" )) == NULL ){
      printf(" (( %s : Tape save image can't open ))\n", file_tapesave );
      return FALSE;
    }
  }
  return TRUE;
}
void	sio_close_tapesave( void )
{
  if( fp_to ){ ex_fclose( fp_to ); fp_to = NULL; }
}

/*-------- char *file_serialin �̃t�@�C���� "rb" �ŊJ�� --------*/

int	sio_open_serialin( void )
{
  sio_close_serialin();

  if( file_serialin ){
    if( (fp_si = ex_fopen( file_serialin, "rb" )) == NULL ){
      printf(" (( %s : Serial input file can't open ))\n", file_serialin );
      return FALSE;
    }
    sio_set_intr_base();
    com_EOF = FALSE;
  }
  return TRUE;
}
void	sio_close_serialin( void )
{
  if( fp_si ){ ex_fclose( fp_si ); fp_si = NULL; }
  sio_set_intr_base();
  /* com_X_flow = FALSE; */
}

/*-------- char *file_serialout �̃t�@�C���� "ab" �ŊJ�� --------*/

int	sio_open_serialout( void )
{
  sio_close_serialout();

  if( file_serialout ){
    if( (fp_so = ex_fopen( file_serialout, "ab" )) == NULL ){
      printf(" (( %s : Serial output file can't open ))\n", file_serialout );
      return FALSE;
    }
  }
  return TRUE;
}
void	sio_close_serialout( void )
{
  if( fp_so ){ ex_fclose( fp_so ); fp_so = NULL; }
}

/*-------- �J���Ă���e�[�v�C���[�W�������߂� --------*/

#define T88_HEADER_STR		"PC-8801 Tape Image(T88)"
int	sio_tape_rewind( void )
{
  int size;
  char buf[ sizeof(T88_HEADER_STR) ];

  if( fp_ti ){
    if( ex_fseek( fp_ti, 0, SEEK_END ) ) goto ERR;
    if( (cmt_size = ex_ftell( fp_ti )) < 0 ) goto ERR;

    if( ex_fseek( fp_ti, 0, SEEK_SET ) ) goto ERR;

    size =ex_fread( buf, sizeof(char), sizeof(buf), fp_ti );
    if( size == sizeof(buf) &&
	memcmp( buf, T88_HEADER_STR, sizeof(buf) ) == 0 ){	/* T88 */
      cmt_is_t88     = TRUE;
      cmt_block_size = 0;
      cmt_EOF        = FALSE;
      cmt_skip       = 0;
    }else{							/* CMT */
      cmt_is_t88     = FALSE;
      cmt_EOF        = FALSE;
      cmt_skip       = 0;
      if( ex_fseek( fp_ti, 0, SEEK_SET ) ) goto ERR;
    }
    return TRUE;
  }

 ERR:
  if( fp_ti ){
    printf(" (( Tape image access error ))\n" );
  }
  sio_close_tapeload();
  return FALSE;
}

/*-------- �J���Ă���e�[�v�̌��݈ʒu��Ԃ� --------*/

int	sio_tape_pos( long *cur, long *end )	
{
  long v;

  if( fp_ti ){
    if( cmt_EOF ){		/* �I�[�Ȃ�A�ʒu=0/�I�[=0 �ɂ��A�^��Ԃ� */
      *cur = 0;
      *end = 0;
      return TRUE;
    }else{			/* �r���Ȃ�A�ʒu�ƏI�[���Z�b�g���^��Ԃ� */
      v = ex_ftell( fp_ti );
      if( v >= 0 ){
	*cur = v;
	*end = cmt_size;
	return TRUE;
      }
    }
  }
  *cur = 0;			/* �s�����́A�ʒu=0/�I�[=0 �ɂ��A�U��Ԃ� */
  *end = 0;
  return FALSE;
}





/*
 * �J���Ă���sio�C���[�W����1�����ǂݍ��� 
 */
static	int	sio_getc( int is_cmt, int *tick )
{
  int i, c, id, size, time;

  if( tick ) *tick = 0;

  if( is_cmt==FALSE ){			/* �V���A������ */
    if( fp_si==NULL ) return EOF;
    if( com_EOF )     return EOF;

    c = fgetc( fp_si );
    if( c==EOF ){
      printf(" (( %s : Serial input file EOF ))\n", file_serialin );
      com_EOF = TRUE;
    }
    return c;

  }else{				/* �e�[�v���� */
    if( fp_ti==NULL ) return EOF;
    if( cmt_EOF )     return EOF;

    if( cmt_is_t88 == FALSE ){			/* CMT�`���̏ꍇ */
      c = fgetc( fp_ti );

    }else{					/* T88�`���̏ꍇ */

      while( cmt_block_size == 0 ){

	if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	id = c;
	if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	id += c << 8;

	if( id==0x0000 ){				/* �I���^�O */
	  cmt_EOF=TRUE; return EOF;
	}
	else{
	  if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	  size = c;
	  if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	  size += c << 8;

	  if( id == 0x0101 ){				/* �f�[�^�^�O */

	    if( size < 12 ){ cmt_EOF=TRUE; return EOF; }

	    for( i=0; i<12; i++ ){	/* ���͑S�Ė��� */
	      if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	    }
	    cmt_block_size = size - 12;

	  }else{					

	    if( id == 0x0100 ||				/* �u�����N�^�O */
		id == 0x0102 ||				/* �X�y�[�X�^�O */
		id == 0x0103 ){				/* �}�[�N�^�O   */

	      if( size != 8 ){ cmt_EOF=TRUE; return EOF; }

	      for( i=0; i<4; i++ ){	/* �J�n���Ԃ͖��� */
		if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	      }
					/* �������Ԃ͎擾 */
	      if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	      time = c;
	      if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	      time += c << 8;
	      if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	      time += c << 16;
	      if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	      time += c << 24;

	      if( tick ) *tick += time;

	    }else{					/* ���̃^�O(����) */

	      for( i=0; i<size; i++ ){
		if( (c=fgetc(fp_ti))==EOF ){ cmt_EOF=TRUE; return EOF; }
	      }

	    }
	  }
	}
      }

      cmt_block_size --;
      c = fgetc( fp_ti );
    }

    if( c==EOF ){
      cmt_EOF = TRUE;
    }
    return c;
  }
}

/* 
 * �e�[�v�̓ǂݍ��ݓr���Ƀ��[�^OFF���ꂽ��A�ǂݍ��݃G���[�����Ƃ���
 * 1�o�C�g�ǂݔ�΂��B�f�[�^�̓r�����ǂ����� T88 �łȂ��ƃ`�F�b�N�ł��Ȃ��B
 * ����ȃ`�F�b�N�A�K�v�Ȃ̂��H�H
 */
static	void	sio_check_cmt_error( void )
{
  int c;
  if( sio_tape_readable() ){
    if( cmt_is_t88     &&	/* T88 ���A�f�[�^�^�O�̓r���̎��̂� */
	cmt_skip == 0  &&
	cmt_block_size ){
      cmt_block_size --;
      c = fgetc( fp_ti );
/*printf("LOST!!");fflush(stdout);*/
      if( c==EOF ){
	cmt_EOF = TRUE;
      }
    }
  }
}






/*
 * �J���Ă���sio�C���[�W��1������������ 
 */
static	int	sio_putc( int is_cmt, int c )
{
  FILE *fp;

  if( is_cmt==FALSE ){ fp = fp_so; }	/* �V���A���o�� */
  else               { fp = fp_to; }	/* �e�[�v�o�� */
  
  if( fp ){
    fprintf( fp, "%c", c );
    fflush( fp );
  }
  return c;
}


/*
 * �����e�[�v���[�h �c�c �ڍוs���B����ȋ@�\�������̂��c 
 */
static	void	sio_tape_highspeed_load( void )
{
  int c, sum, addr, size;

  if( sio_tape_readable()==FALSE ) return;

			  /* �}�V����w�b�_��T�� */

  do{						/* 0x3a ���o�Ă���܂Ń��[�h */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  } while( c != 0x3a );
						/* �]����A�h���X H */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum = c;
  addr = c * 256;
						/* �]����A�h���X L */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum += c;
  addr += c;
						/* �w�b�_���T�� */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum += c;
  if( (sum&0xff) != 0 ){ return; }


		/* ���Ƃ̓f�[�^���̌J��Ԃ� */

  while( TRUE ){

    do{						/* 0x3a ���o�Ă���܂Ń��[�h */
      if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    } while( c != 0x3a );

						/* �f�[�^�� */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    sum  = c;
    size = c;
    if( c==0 ){						/* �f�[�^��==0�ŏI�[ */
      return;
    }

    for( ; size; size -- ){			/* �f�[�^�����A�]�� */

      if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
      sum += c;
      main_mem_write( addr, c );
      addr ++;
    }
						/* �f�[�^���T�� */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    sum += c;
    if( (sum&0xff) != 0 ){ return; }
  }
}



/*
 * RS232C���荞�ݎ����̃Z�b�g 
 */
static int sio_bps;		/* BPS */
static int sio_framesize;	/* StartBit + Bit�� + StopBit ��K���Ɍv�Z */

static void sio_set_intr_base( void )
{
  static const int table[] = {
    75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
  };

  /* �C���[�W�t�@�C���Z�b�g�ς� ���A
     ��M Enable ���A
     RS232C (I/O 30h:bit5=ON) ���ACMT�����[�^ON (I/O 30h:bit5=OFF,bit3=ON)
     �̎��ɁA�����I�Ɋ��荞�݂𔭐�������B���̎������v�Z����B */

  if( (fp_si || fp_ti) &&
      (sio_command & 0x04) &&
      ( (sys_ctrl & 0x20) || (sys_ctrl & 0x08) ) ){

    if( sys_ctrl & 0x20 ){		/* RS232C �w�莞  */

      sio_bps = table[ baudrate_sw ];		/* BPS �� �{�[���[�g�ݒ� */
      sio_framesize = 10;			/* �t���[������10bit�Œ� */

    }else{				/* CMT �w�莞 */

      if( cmt_speed == 0 ){			/* �ʏ�́A*/
	if( sys_ctrl & 0x10 ) sio_bps = 1200;	/* I/O 30h:bit4=1 �� 1200bps */
	else                  sio_bps =  600;	/*             =0 ��  600bps */
      }else{
	sio_bps = cmt_speed;			/* �����w�莞�͂��̒l */
      }
      sio_framesize = 11;			/* �t���[������11bit�Œ� */
    }

  }else{				/* �V���A���s�� */
    sio_bps = 0;
    sio_framesize = 0;
  }

  interval_work_set_RS232C( sio_bps, sio_framesize );  
}


/*
 * T88 �t�H�[�}�b�g �� TICK���Ԃ� �V���A�����荞�݉񐔂Ɋ��Z����
 *
 * �u�����N�A�X�y�[�X�A�}�[�N�^�O�́A1/4800s�P�ʂ̐�Ύ��Ԃ��L�q���Ă���B
 * �Ȃ̂ŁA���Ԃ� n �̏ꍇ�An/4800�b�E�F�C�g�������Ƃ��������ɂȂ�͂��B
 * �܂�A ( n / 4800 ) * CPU_CLOCK / rs232c_intr_base �񕪁A
 * RS232C���荞�݂�]���ɑ҂Ă΂����B���̎��́A�u���������
 *	     ( n / 4800 ) * ( bps / framesize )
 * �ɂȂ�B�܂�������ɂ��悩�Ȃ�K���ȃE�F�C�g�ł͂��邪�B
*/
static	int	tick_2_intr_skip( int tick )
{
  if( sio_framesize == 0 ) return 0;

  return (tick * sio_bps / 4800 / sio_framesize) + 1;
}



/*
 *
 */
static	void	sio_init( void )
{
  sio_data_exist = FALSE;
  sio_data = 0xff;

  cmt_dummy_read_cnt = 0;

  sio_instruction = 0;
  sio_command     = 0;
  sio_mode        = 0;
}
/*
 *
 */
static	void	sio_out_command( byte data )
{
  if( sio_instruction==0 ){			/* �������Z�b�g����́A */
    sio_mode        = data;				/* ���[�h��t�� */
    sio_instruction = 1;

  }else{					/* ����ȊO�̓R�}���h��t�� */

    if( data & 0x40 ){					/* �������Z�b�g */
      sio_mode        = 0;
      sio_instruction = 0;
      sio_command     = 0x40;
    }else{						/* ���̑�       */
      sio_command     = data;
    }

    if( (sio_command & 0x04) == 0 ){			/* ���Z�b�gor��M�֎~*/
      sio_data_exist = FALSE;				/* �Ȃ�A��M���[�N  */
      RS232C_flag   = FALSE;				/* ���N���A����      */
    }

    sio_set_intr_base();
  }
/*printf("SIO %02x -> mode:%02x cmd:%02x\n",data,sio_mode,sio_command);*/
}
/*
 *
 */
static	void	sio_out_data( byte data )
{
  int is_cmt;

  if( (sio_command & 0x01) ){		/* ���M�C�l�[�u�� */
    if( sys_ctrl & 0x20 ){			/* �V���A���o�͂̏ꍇ */
      is_cmt = FALSE;
      if     ( data==0x11 ){				/* ^Q �o�� */
	com_X_flow = FALSE;
      }else if( data==0x13 ){				/* ^S �o�� */
	com_X_flow = TRUE;
      }
    }else{					/* �e�[�v�o�͂̏ꍇ */
      is_cmt = TRUE;
    }
    sio_putc( is_cmt, data );
  }
}
/*
 *
 */
static	byte	sio_in_data( void )
{
/*printf("->%02x ",sio_data);fflush(stdout);*/
  sio_data_exist = FALSE;
  RS232C_flag = FALSE;
  return sio_data;
}
/*
 *
 */
static	byte	sio_in_status( void )
{
  int c;
  byte	status = 0x80 | 0x04;		/* ���M�o�b�t�@�G���v�e�B */
                /* DSR| TxE */

  if( sio_command & 0x04 ){		/* ���݁A��M�C�l�[�u���̏ꍇ */

    if( (sys_ctrl & 0x20)==0 && 		/* �e�[�v�ŁASIO���荞�݂�  */
	sio_tape_readable() &&			/* �g��Ȃ��ꍇ�A�����œǂ� */
	cmt_intr == FALSE ){

      cmt_dummy_read_cnt ++;			/* IN 21 �� 2����s����x�� */
      if( cmt_dummy_read_cnt >= 2 ){
	cmt_dummy_read_cnt = 0;

	c = sio_getc( TRUE, 0 );		/* �e�[�v����1�����ǂ� */
/*printf("[%03x]",c&0xfff);fflush(stdout);*/
	if( c != EOF ){
	  sio_data = (byte)c;
	  sio_data_exist = TRUE;
	}
      }
    }

    if( sio_data_exist ){			/* �f�[�^������� */
      status |= 0x02;					/* ��M���f�B */
             /* RxRDY */
    }
  }

  if( sio_command & 0x01 ){		/* ���݁A���M�C�l�[�u���̏ꍇ */
    if(( (sys_ctrl & 0x20) /*&& sio_serial_writable()*/ ) ||
       (!(sys_ctrl & 0x20)   && sio_tape_writable()     ) ){
      status |= 0x01;				/* ���M���f�B */
             /* TxRDY */
    }
  }

  return	status;
}
/*
 *
 */
static	void	sio_term( void )
{
}



/*
 ***************** RS-232C ��M���荞�ݏ��� ***************
 */
int	sio_intr( void )
{
  int c = EOF;
  int tick;

  if( (sio_command & 0x04) &&		/* ���݁A��M�C�l�[�u����   */
      ! sio_data_exist ){		/* �Ǎ����̃f�[�^���Ȃ��ꍇ */

    if( sys_ctrl & 0x20 ){			/* �V���A������ */

      if( com_X_flow ) return FALSE;
      c = sio_getc( FALSE, 0 );

    }else{					/* �e�[�v����(�����g�p���̂�)*/
      if( cmt_intr ){

	if( cmt_skip==0 ){
	  if( cmt_wait ){
	    c = sio_getc( TRUE, &tick );
	    if( tick ){
	      cmt_skip = tick_2_intr_skip( tick );
	      if( cmt_skip!=0  ){
		cmt_skip_data = c;
		c = EOF;
	      }
	    }
	  }else{
	    c = sio_getc( TRUE, 0 );
	  }
	}else{						/* T88�̏ꍇ�́A    */
	  cmt_skip --;					/* �����f�[�^������ */
	  if( cmt_skip==0 ){				/* �ԁA���Ԓׂ����� */
	    c = cmt_skip_data;
	  }
	}
      }
    }

    if( c!=EOF ){
      sio_data = (byte)c;
      sio_data_exist = TRUE;
/*printf("<%02x> ",sio_data);fflush(stdout);*/
      return TRUE;				/* RxRDY���荞�ݔ��� */
    }
  }
  return FALSE;
}



/*----------------------*/
/*     Printer Port	*/
/*----------------------*/
static	FILE *fp_prn;
int	printer_open( void )
{
  printer_close();

  if( file_printer ){
    if( (fp_prn=ex_fopen( file_printer, "ab"))==NULL ){
      printf(" (( %s : Printer output file can't open ))\n", file_printer );
      return FALSE;
    }
  }
  return TRUE;
}
void	printer_close( void )
{
  if( fp_prn ){ ex_fclose( fp_prn ); fp_prn = NULL; }
}



void	printer_init( void )
{
}
void	printer_stlobe( void )
{
  if( file_printer && fp_prn ){
    fprintf( fp_prn, "%c", common_out_data );
    fflush( fp_prn );
  }
}
void	printer_term( void )
{
}


/*----------------------*/
/*   �J�����_�N���b�N	*/
/*----------------------*/
static	Uchar	shift_reg[7];
static	Uchar	calendar_cdo;
static	double	calendar_diff = 0.0;
static	int	calendar_diff_int;

static	void	get_calendar_work( void )
{
  struct tm t;

  if( calendar_stop==FALSE ){
    time_t now_time;
    struct tm *tp;

    now_time  = time( NULL );
    now_time += (time_t)calendar_diff;
    tp = localtime( &now_time );
    t = *tp;
  }else{
    t.tm_year = calendar_data[0] + 1900;
    t.tm_mon  = calendar_data[1];
    t.tm_mday = calendar_data[2];
    t.tm_wday = calendar_data[3];
    t.tm_hour = calendar_data[4];
    t.tm_min  = calendar_data[5];
    t.tm_sec  = calendar_data[6];
  }

  shift_reg[0] = ( t.tm_sec %10 <<4 );
  shift_reg[1] = ( t.tm_min %10 <<4 ) | ( t.tm_sec /10 );
  shift_reg[2] = ( t.tm_hour%10 <<4 ) | ( t.tm_min /10 );
  shift_reg[3] = ( t.tm_mday%10 <<4 ) | ( t.tm_hour/10 );
  shift_reg[4] = ( t.tm_wday    <<4 ) | ( t.tm_mday/10 );
  shift_reg[5] = ( (t.tm_year%100)%10 <<4 ) | ( t.tm_mon+1 );
  shift_reg[6] = ( (t.tm_year%100)/10 );
}
static	void	set_calendar_work( int x )
{
#define BCD2INT(b)	((((b)>>4)&0x0f)*10 + ((b)&0x0f))
  time_t now_time;
  time_t chg_time;
  struct tm *tp;
  struct tm t;
  int i;

  static const char *week[]=
  { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "???" };

  if(x==0){
    if( verbose_io )
      printf("Set Clock %02d/%02x(%s) %02x:%02x:%02x\n",
	     (shift_reg[4]>>4)&0x0f, shift_reg[3], week[ shift_reg[4]&0x07 ],
	     shift_reg[2], shift_reg[1], shift_reg[0]);

    now_time  = time( NULL );
    now_time += (time_t)calendar_diff;
    tp = localtime( &now_time );
    t.tm_year = tp->tm_year;

  }else{
    if( verbose_io )
      printf("Set Clock %02x/%02d/%02x(%s) %02x:%02x:%02x\n",
	     shift_reg[5],
	     (shift_reg[4]>>4)&0x0f, shift_reg[3], week[ shift_reg[4]&0x07 ],
	     shift_reg[2], shift_reg[1], shift_reg[0]);

    i = BCD2INT( shift_reg[5] );
    if( i >= 38 ) t.tm_year = 1900 + i -1900;
    else          t.tm_year = 2000 + i -1900;
  }

  t.tm_mon  = ((shift_reg[4]>>4)&0x0f) -1;
  t.tm_mday = BCD2INT( shift_reg[3] );
  t.tm_wday = shift_reg[4]&0x07;
  t.tm_hour = BCD2INT( shift_reg[2] );
  t.tm_min  = BCD2INT( shift_reg[1] );
  t.tm_sec  = BCD2INT( shift_reg[0] );
  t.tm_yday = 0;
  t.tm_isdst= 0;

  now_time = time( NULL );
  chg_time = mktime( &t );

  if( now_time != -1 &&
      chg_time != -1  )  calendar_diff = difftime( chg_time, now_time );

#undef BCD2INT
}


void	calendar_init( void )
{
  int	i;
  for(i=0;i<7;i++) shift_reg[i] = 0;
  calendar_cdo = 0;

  /* calendar_diff = 0.0; */
}

void	calendar_init_at_resume( void )
{
  calendar_diff = (double)calendar_diff_int;
}

void	calendar_shift_clock( void )
{
  byte	x = ( common_out_data>>3 ) & 0x01;

  calendar_cdo = shift_reg[0] & 0x01;
  shift_reg[0] = ( shift_reg[0]>>1 ) | ( shift_reg[1]<<7 );
  shift_reg[1] = ( shift_reg[1]>>1 ) | ( shift_reg[2]<<7 );
  shift_reg[2] = ( shift_reg[2]>>1 ) | ( shift_reg[3]<<7 );
  shift_reg[3] = ( shift_reg[3]>>1 ) | ( shift_reg[4]<<7 );
  shift_reg[4] = ( shift_reg[4]>>1 ) | ( shift_reg[5]<<7 );
  shift_reg[5] = ( shift_reg[5]>>1 ) | ( shift_reg[6]<<7 );
  shift_reg[6] = ( shift_reg[6]>>1 ) | ( x<<3 );
}

void	calendar_stlobe( void )
{
  switch( common_out_data & 0x7 ){
  case 0:	/*calendar_init();*/	break;		/* ������ */
  case 1:	calendar_shift_clock();	break;		/* �V�t�g */
  case 2:	calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		set_calendar_work(0);	break;
  case 3:	get_calendar_work();			/* �����擾 */
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();	break;
  case 4:	break;
  case 5:	break;
  case 6:	break;
  case 7:
    switch( shift_reg[6] & 0xf ){

    case 0:	/*calendar_init();*/	break;		/* ������ */
    case 1:	calendar_shift_clock();	break;		/* �V�t�g */
    case 2:	set_calendar_work(1);	break;		/* �����ݒ� */
    case 3:	get_calendar_work();	break;		/* �����擾 */
    case 4:	break;
    case 5:	break;
    case 6:	break;
    case 7:	break;
    case 8:	break;
    case 9:	break;
    case 10:	break;
    case 11:	break;
    case 12:	break;
    case 13:	break;
    case 14:	break;
    case 15:	/*test_mode();*/	break;
    }
    break;
  }
}


/*----------------------*/
/*        �}�E�X	*/
/*----------------------*/
static	int	jop1_on_sign;
static	int	jop1_off_sign;
void	jop1_init( void )
{
  jop1_on_sign  = 0;
  jop1_off_sign = 0;
}
void	jop1_stlobe_on( void )
{
  if( mouse_mode==1 ){
    if( sound_reg[ 0x07 ] & 0x80 ){
      sound_reg[0x0e] = 0xff;
    }else{
      if( jop1_on_sign==0 ){ check_mouse();
			     sound_reg[0x0e] = ((-mouse_dx)>>4) & 0x0f; }
      else                   sound_reg[0x0e] = ((-mouse_dy)>>4) & 0x0f;
      jop1_on_sign ^= 1;
    }
  }
}
void	jop1_stlobe_off( void )
{
  if( mouse_mode==1 ){
    if( sound_reg[ 0x07 ] & 0x80 ){
      sound_reg[0x0e] = 0xff;
    }else{
      if( jop1_off_sign==0 ) sound_reg[0x0e] = (-mouse_dx) & 0x0f;
      else                   sound_reg[0x0e] = (-mouse_dy) & 0x0f;
      jop1_off_sign ^= 1;
    }
  }
}



/*------------------------------*/
/*    �R���g���[���M�����o��	*/
/*------------------------------*/
void	out_ctrl_signal( byte data )
{
  byte	trg_on  = ~ctrl_signal &  data;
  byte	trg_off =  ctrl_signal & ~data;

  if( trg_on  & 0x01 ) printer_stlobe();
  if( trg_off & 0x02 ) calendar_stlobe();
  if( trg_off & 0x04 ) calendar_shift_clock();

  if( data & 0x08 ) set_crtc_sync_bit();
  else		    clr_crtc_sync_bit();

  if( (trg_on & (0x80|0x20)) || (trg_off & (0x80|0x20)) ){
    xmame_beep_out_data( data );
  }

  if( trg_on  & 0x40 ) jop1_stlobe_on();
  if( trg_off & 0x40 ) jop1_stlobe_off();

  ctrl_signal = data;
}

byte	in_ctrl_signal( void )
{
  return ((ctrl_vrtc    << 5 ) |
	  (calendar_cdo << 4 ) |
	   ctrl_boot           |
	   monitor_15k         );
}








/************************************************************************/
/* �������̏����� (�d���������̂�)					*/
/************************************************************************/
void	power_on_ram_init( void )
{
  int   addr, i;
  Uchar data;

		/* ���C�� RAM �����ȃp�^�[���Ŗ��߂� */

  for( addr = 0; addr < 0x10000; addr += 0x100 ){
    if( (addr&0x0d00)==0x0100 || (addr&0x0f00)==0x0500 ||
        (addr&0x0f00)==0x0a00 || (addr&0x0d00)==0x0c00 )  data = 0xff;
    else                                                  data = 0x00;

    if( addr&0x4000 ) data ^= 0xff;
    if( addr&0x8000 ) data ^= 0xff;
    if((addr&0xf000)==0xb000 ) data ^= 0xff;
#if 0
    if((addr&0xf000)==0xe000 ) data ^= 0xff; /* �Ƃ肠�������] */
					     /* changed by peach */
#endif

    for(i=0;i<4;i++){
      memset( &main_ram[ addr + i*64     ], data,      16 );
      memset( &main_ram[ addr + i*64 +16 ], data^0xff, 16 );
      memset( &main_ram[ addr + i*64 +32 ], data,      16 );
      memset( &main_ram[ addr + i*64 +48 ], data^0xff, 16 );
      data ^= 0xff;
    }
  }
  if( high_mode ){
    for( i=0xf000; i<0x10000; i++ ) main_ram[i] ^= 0xff;
  }


		/* ���� RAM(�̗�) �����ȃp�^�[���Ŗ��߂� */

  memcpy( main_high_ram, &main_ram[0xf000], 0x1000 );
  for( addr=0xf000; addr<0x10000; addr++ ) main_ram[addr] ^= 0xff;
}



/************************************************************************/
/* �}�X�J�u�����荞�݃G�~�����[�g					*/
/************************************************************************/



/************************************************************************/
/* PC88 ���C���V�X�e�� ������						*/
/************************************************************************/
void	pc88main_init( int reset_keyboard )
{
  int	i;


	/* Z80 �G�~�����[�^���[�N������ */

  z80_reset( &z80main_cpu );

  z80main_cpu.mem_read  = main_mem_read;
  z80main_cpu.mem_write = main_mem_write;
  z80main_cpu.io_read   = main_io_in;
  z80main_cpu.io_write  = main_io_out;

  z80main_cpu.intr_update = main_INT_update;
  z80main_cpu.intr_chk    = main_INT_chk;

  z80main_cpu.break_if_halt = FALSE;

#if	defined( MAIN_DISP ) || defined( MAIN_FILE )
  z80main_cpu.log	= TRUE;
#else
  z80main_cpu.log	= FALSE;
#endif



	/* ���荞�݃G�~�����[�g������ */

  main_INT_init();


	/* �y���t�F���������� */

  if( reset_keyboard )
    keyboard_init();
  joystick_init();

  sio_init();
  crtc_init();
  dmac_init();
  printer_init();
  calendar_init();
  jop1_init();
  sound_init();
  pio_init();

  dipsw_1 = (boot_dipsw   ) & SW_1_MASK;
  dipsw_2 = (boot_dipsw>>8) & SW_2_MASK;

  switch( boot_basic ){
  case BASIC_N:
    dipsw_1 &= ~SW_N88;
    dipsw_2 |=  SW_V1;
    dipsw_2 &= ~SW_H;
    high_mode = FALSE;
    break;
  case BASIC_V1S:
    dipsw_1 |=  SW_N88;
    dipsw_2 |=  SW_V1;
    dipsw_2 &= ~SW_H;
    high_mode = FALSE;
    break;
  case BASIC_V1H:
    dipsw_1 |=  SW_N88;
    dipsw_2 |=  SW_V1;
    dipsw_2 |=  SW_H;
    high_mode = TRUE;
    break;
  case BASIC_V2:
    dipsw_1 |=  SW_N88;
    dipsw_2 &= ~SW_V1;
    dipsw_2 |=  SW_H;
    high_mode = TRUE;
    break;
  }


  ctrl_boot		= (boot_from_rom) ? SW_ROMBOOT : 0;
  memory_bank		= MEMORY_BANK_MAIN;
  cpu_clock		= (boot_clock_4mhz) ? SW_4MHZ : 0;

  sys_ctrl		= 0x31;
  grph_ctrl		= 0x39;
  misc_ctrl		= 0x90;
  ALU1_ctrl		= 0x77;
  ALU2_ctrl		= 0x00;
  ctrl_signal		= 0x0f;
  grph_pile		= 0x00;
  /*baudrate_sw		= 0;*/
  window_offset		= 0x0000;
  ext_rom_bank		= 0xff;

  ext_ram_ctrl		= 0;
  ext_ram_bank		= 0;

  jisho_rom_ctrl	= JISHO_NOT_SELECT;
  jisho_rom_bank	= 0;

  vram_bg_palette.blue  = 0 << 5;
  vram_bg_palette.red   = 0 << 5;
  vram_bg_palette.green = 0 << 5;
  for( i=0; i<8; i++ ){
    vram_palette[ i ].blue  = (i&1) ? (7<<5) : 0;
    vram_palette[ i ].red   = (i&2) ? (7<<5) : 0;
    vram_palette[ i ].green = (i&4) ? (7<<5) : 0;
  }

  intr_level		= 7;
  intr_priority		= 0;
  intr_sio_enable	= 0x00;
  intr_vsync_enable	= 0x00;
  intr_rtc_enable	= 0x00;

  main_memory_mapping_0000_7fff();
  main_memory_mapping_8000_83ff();
  main_memory_mapping_c000_ffff();
  main_memory_vram_mapping();
}


/************************************************************************/
/* PC88 ���C���V�X�e�� �I��						*/
/************************************************************************/
void	pc88main_term( void )
{
  joystick_term();
  printer_term();
  sio_term();
}










/************************************************************************/
/* �u���[�N�|�C���g�֘A							*/
/************************************************************************/
INLINE	void	check_break_point( int type, word addr, byte data, char *str )
{
  int	i;

  if ( emu_mode == MONITOR_MAIN ) return; /* ���j�^�[���[�h���̓X���[ */
  for( i=0; i<NR_BP; i++ ){
    if( break_point[BP_MAIN][i].type == type &&
        break_point[BP_MAIN][i].addr == addr ){
      printf( "*** Break at %04x *** "
	      "( MAIN - #%d [ %s %04XH , data = %02XH ]\n",
	      z80main_cpu.PC.W, i+1, str, addr, data );
      emu_mode = MONITOR;
      break;
    }
  }
}

byte	main_mem_read_with_BP( word addr )
{
  byte	data = main_mem_read( addr );
  check_break_point( BP_READ, addr, data, "READ from" );
  return data;
}

void	main_mem_write_with_BP( word addr, byte data )
{
  main_mem_write( addr, data );
  check_break_point( BP_WRITE, addr, data, "WRITE to" );
}

byte	main_io_in_with_BP( byte port )
{
  byte	data =  main_io_in( port );
  check_break_point( BP_IN, port, data, "IN from" );
  return data;
}

void	main_io_out_with_BP( byte port, byte data )
{
  main_io_out( port, data );
  check_break_point( BP_OUT, port, data, "OUT to" );
}




void	pc88main_break_point( void )
{
  int	i, buf[4];
  for( i=0; i<4; i++ ) buf[i]=0;
  for( i=0; i<NR_BP; i++ ){
    switch( break_point[BP_MAIN][i].type ){
    case BP_READ:	buf[0]++;	break;
    case BP_WRITE:	buf[1]++;	break;
    case BP_IN:		buf[2]++;	break;
    case BP_OUT:	buf[3]++;	break;
    }
  }
   
  if( buf[0] ) z80main_cpu.mem_read  = main_mem_read_with_BP;
  else         z80main_cpu.mem_read  = main_mem_read;

  if( buf[1] ) z80main_cpu.mem_write = main_mem_write_with_BP;
  else         z80main_cpu.mem_write = main_mem_write;

  if( buf[2] ) z80main_cpu.io_read   = main_io_in_with_BP;
  else         z80main_cpu.io_read   = main_io_in;

  if( buf[3] ) z80main_cpu.io_write  = main_io_out_with_BP;
  else         z80main_cpu.io_write  = main_io_out;
}








/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_pc88main_work[]=
{
  { TYPE_INT,	&boot_basic,	},
  { TYPE_INT,	&boot_dipsw,	},
  { TYPE_INT,	&boot_from_rom,	},
  { TYPE_INT,	&boot_clock_4mhz,},
  { TYPE_INT,	&monitor_15k,	},

  { TYPE_PAIR,	&z80main_cpu.AF,	},
  { TYPE_PAIR,	&z80main_cpu.BC,	},
  { TYPE_PAIR,	&z80main_cpu.DE,	},
  { TYPE_PAIR,	&z80main_cpu.HL,	},
  { TYPE_PAIR,	&z80main_cpu.IX,	},
  { TYPE_PAIR,	&z80main_cpu.IY,	},
  { TYPE_PAIR,	&z80main_cpu.PC,	},
  { TYPE_PAIR,	&z80main_cpu.SP,	},
  { TYPE_PAIR,	&z80main_cpu.AF1,	},
  { TYPE_PAIR,	&z80main_cpu.BC1,	},
  { TYPE_PAIR,	&z80main_cpu.DE1,	},
  { TYPE_PAIR,	&z80main_cpu.HL1,	},
  { TYPE_BYTE,	&z80main_cpu.I,		},
  { TYPE_BYTE,	&z80main_cpu.R,		},
  { TYPE_BYTE,	&z80main_cpu.R_saved,	},
  { TYPE_CHAR,	&z80main_cpu.IFF,	},
  { TYPE_CHAR,	&z80main_cpu.IFF2,	},
  { TYPE_CHAR,	&z80main_cpu.IM,	},
  { TYPE_CHAR,	&z80main_cpu.HALT,	},
  { TYPE_CHAR,	&z80main_cpu.discontinue,	},
  { TYPE_INT,	&z80main_cpu.icount,	},
  { TYPE_INT,	&z80main_cpu.state,	},
  { TYPE_INT,	&z80main_cpu.state0,	},
  { TYPE_CHAR,	&z80main_cpu.log,	},
  { TYPE_CHAR,	&z80main_cpu.break_if_halt,	},

  { TYPE_INT,	&high_mode,	},

  { TYPE_BYTE,	&dipsw_1,	},
  { TYPE_BYTE,	&dipsw_2,	},
  { TYPE_BYTE,	&ctrl_boot,	},
  { TYPE_INT,	&memory_bank,	},
  { TYPE_BYTE,	&cpu_clock,	},

  { TYPE_BYTE,	&common_out_data,},
  { TYPE_BYTE,	&misc_ctrl,	},
  { TYPE_BYTE,	&ALU1_ctrl,	},
  { TYPE_BYTE,	&ALU2_ctrl,	},
  { TYPE_BYTE,	&ctrl_signal,	},
  { TYPE_BYTE,	&baudrate_sw,	},
  { TYPE_WORD,	&window_offset,	},
  { TYPE_BYTE,	&ext_rom_bank,	},
  { TYPE_BYTE,	&ext_ram_ctrl,	},
  { TYPE_BYTE,	&ext_ram_bank,	},

  { TYPE_PAIR,	&kanji1_addr,	},
  { TYPE_PAIR,	&kanji2_addr,	},

  { TYPE_BYTE,	&jisho_rom_bank,	},
  { TYPE_BYTE,	&jisho_rom_ctrl,	},

  { TYPE_INT,	&calendar_stop,	   },
  { TYPE_CHAR,	&calendar_data[0], },
  { TYPE_CHAR,	&calendar_data[1], },
  { TYPE_CHAR,	&calendar_data[2], },
  { TYPE_CHAR,	&calendar_data[3], },
  { TYPE_CHAR,	&calendar_data[4], },
  { TYPE_CHAR,	&calendar_data[5], },
  { TYPE_CHAR,	&calendar_data[6], },

  { TYPE_INT,	&ALU_buf.l,	},	/*  TYPE_CHAR, ALU_buf.c[0]-[3] ?  */
  { TYPE_INT,	&ALU_comp.l,	},	/*  TYPE_CHAR, ALU_comp.c[0]-[3] ? */

  { TYPE_INT,	&sio_instruction,},
  { TYPE_BYTE,	&sio_mode,	},
  { TYPE_BYTE,	&sio_command,	},

  { TYPE_CHAR,	&shift_reg[0],	},
  { TYPE_CHAR,	&shift_reg[1],	},
  { TYPE_CHAR,	&shift_reg[2],	},
  { TYPE_CHAR,	&shift_reg[3],	},
  { TYPE_CHAR,	&shift_reg[4],	},
  { TYPE_CHAR,	&shift_reg[5],	},
  { TYPE_CHAR,	&shift_reg[6],	},
  { TYPE_CHAR,	&calendar_cdo,	},

  { TYPE_INT,	&jop1_on_sign,	},
  { TYPE_INT,	&jop1_off_sign,	},

  { TYPE_INT,	&calendar_diff_int, },
};


int	suspend_pc88main( FILE *fp, long offset )
{
  int	ret;

  calendar_diff_int = (int)calendar_diff;

  ret = suspend_work( fp, offset, 
		      suspend_pc88main_work, 
		      countof(suspend_pc88main_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->pc88main ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_pc88main(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_pc88main_work, 
			   countof(suspend_pc88main_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}



void	pc88main_init_at_resume( void )
{

  z80main_cpu.mem_read  = main_mem_read;
  z80main_cpu.mem_write = main_mem_write;
  z80main_cpu.io_read   = main_io_in;
  z80main_cpu.io_write  = main_io_out;

  z80main_cpu.intr_update = main_INT_update;
  z80main_cpu.intr_chk    = main_INT_chk;

  z80main_cpu.PC_prev   = z80main_cpu.PC;	/* dummy for monitor */


  main_INT_init_at_resume();


  keyboard_init_at_resume();
  joystick_init();

  calendar_init_at_resume();
  sio_init();				/* SIO �̓��W���[�����Ȃ�	*/
  crtc_dmac_init_at_resume();
  printer_init();			/* PRINTER �̓��W���[�����Ȃ�	*/
  sound_init_at_resume();
  pio_init_at_resume();

  main_memory_mapping_0000_7fff();
  main_memory_mapping_8000_83ff();
  main_memory_mapping_c000_ffff();
  main_memory_vram_mapping();

}
