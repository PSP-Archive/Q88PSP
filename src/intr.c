/************************************************************************/
/*									*/
/* ���荞�݂̃G�~�����[�g						*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "initval.h"
#include "intr.h"

#include "pc88cpu.h"
#include "pc88main.h"
#include "screen.h"
#include "keyboard.h"
#include "sound.h"
#include "joystick.h"

#include "emu.h"
#include "snddrv.h"
#include "wait.h"
#include "suspend.h"



int	intr_level;			/* OUT[E4] ���荞�݃��x��	*/
int	intr_priority;			/* OUT[E4] ���荞�ݗD��x	*/
int	intr_sio_enable;		/* OUT[E6] ���荞�݃}�X�N SIO	*/ 
int	intr_vsync_enable;		/* OUT[E6] ���荞�݃}�X�N VSYNC	*/ 
int	intr_rtc_enable;		/* OUT[E6] ���荞�݃}�X�N RTC	*/ 



double	cpu_clock_mhz   = DEFAULT_CPU_CLOCK_MHZ;   /* MAIN CPU Clock   [MHz] */
double	sound_clock_mhz = DEFAULT_SOUND_CLOCK_MHZ; /* SOUND chip Clock [MHz] */
double	vsync_freq_hz   = DEFAULT_VSYNC_FREQ_HZ;   /* VSYNC ��������   [Hz]  */



#define	CPU_CLOCK_MHZ		cpu_clock_mhz
#define	SOUND_CLOCK_MHZ		sound_clock_mhz
#define	VSYNC_FREQ_HZ		vsync_freq_hz

#define	VRTC_RATE		(48.0/448.0)
#define	RTC_FREQ_HZ		(600)

#define	TIMER_A_CONST		(12)
#define	TIMER_B_CONST		(192)

#define	CPU_CLOCK		(CPU_CLOCK_MHZ * 1000000)

/*****************************************************************************/

int	state_of_cpu = 0;		/* ���C��CPU�������������ߐ�	*/
int	state_of_vsync;			/* VSYNC�����̃X�e�[�g��	*/

int	cpu_load      = 0;		/* �E�G�C�g �������قǒx��	*/
int	no_wait	      = FALSE;		/* �E�G�C�g�Ȃ�			*/


	int	RS232C_flag    = FALSE;	/* RS232C */
static	int	rs232c_intr_base;
static	int	rs232c_intr_timer;

	int	VSYNC_flag     = FALSE;	/* VSYNC */
static	int	vsync_intr_base;
static	int	vsync_intr_timer;

	int	ctrl_vrtc      = 1;	/* VRTC (�����A����:1 / �\����:0) */
static	int	vrtc_base;
static	int	vrtc_timer;

	int	RTC_flag       = FALSE;	/* RTC */
static	int	rtc_intr_base;
static	int	rtc_intr_timer;

	int	SOUND_flag     = FALSE;	/* SOUND Timer-A/Timer-B */
static	int	sd_A_intr_base;
static	int	sd_A_intr_timer;
static	int	sd_B_intr_base;
static	int	sd_B_intr_timer;
static	int	sd2_BRDY_intr_base;
static	int	sd2_BRDY_intr_timer;
static	int	sd2_EOS_intr_base;
static	int	sd2_EOS_intr_timer;




/*------------------------------------------------------
 * �^�C�}�[���荞�݃G�~�����[�g�̃��[�N��������
 *	VSYNC / VRTC / RTC         ���[�N�͋N�����ɏ�����
 *	RS232C / Timer-A / TImer-B ���[�N�͐ݒ莞�ɏ�����
 *------------------------------------------------------*/

/*
 * ���荞�݃~�����[�g������ �c Z80 �̋N�����ɌĂ�
 */
static	void	interval_work_init_generic( void )
{
  vsync_intr_timer = vsync_intr_base = CPU_CLOCK / VSYNC_FREQ_HZ;
  vrtc_timer       = vrtc_base       = (vsync_intr_base * VRTC_RATE);

  rtc_intr_timer   = rtc_intr_base   = CPU_CLOCK / RTC_FREQ_HZ;

  select_main_cpu = TRUE;
  dual_cpu_count  = 0;

  state_of_vsync = vsync_intr_base;
  state_of_cpu   = 0;
}


/*
 * RS232C���荞�݃G�~�����[�g������ �c Z80�N�����ɌĂ�
 */
static	void	interval_work_init_RS232C( void )
{
  interval_work_set_RS232C( 0, 0 );
}

/*
 * �T�E���h���荞�݃G�~�����[�g������ �c Z80�N�����ɌĂ�
 */
static	void	interval_work_init_TIMER_A( void )
{
  interval_work_set_TIMER_A();
  sd_A_intr_timer = sd_A_intr_base;
}
static	void	interval_work_init_TIMER_B( void )
{
  interval_work_set_TIMER_B();
  sd_B_intr_timer = sd_B_intr_base;
}




/*
 * �S���荞�݃G�~�����[�g������ �c Z80�N������A�N���b�N�ύX���ȂǂɌĂ�
 */
void	interval_work_init_all( void )
{
  interval_work_init_generic();
  interval_work_init_RS232C();
  interval_work_init_TIMER_A();
  interval_work_init_TIMER_B();
}



/*
 * RS232C���荞�݃G�~�����[�g�ď����� �c RS232C�����ݒ莞�ɌĂ�
 */
void	interval_work_set_RS232C( int bps, int framesize )
{
  if( bps == 0 || framesize == 0 ){
    rs232c_intr_base = 0x7fffffff;
  }else{
    rs232c_intr_base = CPU_CLOCK / ( (double)bps / (double)framesize );
    if( rs232c_intr_base < 100 ) rs232c_intr_base = 100;
  }
  rs232c_intr_timer = rs232c_intr_base;
}




/************************************************************************/
/* ���荞�݂Ɋւ�� �T�E���h���W�X�^�X�V���̏���			*/
/************************************************************************/
/*
 * �T�E���h�̃^�C�}�v���Z�b�g�l �ύX���ɌĂ�
 */
void	interval_work_set_TIMER_A( void )
{
  sd_A_intr_base  = TIMER_A_CONST * sound_prescaler * (1024 - sound_TIMER_A)
					* ( CPU_CLOCK_MHZ / SOUND_CLOCK_MHZ );
}
void	interval_work_set_TIMER_B( void )
{
  sd_B_intr_base  = TIMER_B_CONST * sound_prescaler * (256 - sound_TIMER_B)
					* ( CPU_CLOCK_MHZ / SOUND_CLOCK_MHZ );
}

/*
 * �T�E���h�� �e��t���O ����� �v���X�P�[���[�l�ύX���ɌĂ�
 */
static	int	sound_flags_update     = 0;
static	int	sound_prescaler_update = 0;

void	change_sound_flags( int port )
{
  sound_flags_update = (int)port;
  refresh_intr_timing( z80main_cpu );
}
void	change_sound_prescaler( int new_prescaler )
{
  sound_prescaler_update = new_prescaler;
  refresh_intr_timing( z80main_cpu );
}




/*
 * �T�E���h�̃v���X�P�[���[�l�E�e��t���O�ύX���̍ۂɁA�t���O������������B
 *	���荞�ݍX�V��ɌĂ΂��B
 */
static	void	check_sound_parm_update( void )
{
  byte data;

  if( sound_prescaler_update ){		/* ���� �ύX����������		    */
					/* �^�C�}�l�� (�ύX��/�ύX�O)�{���� */
					/* �^�C�}�l�̂��܂����킹��B   */
    sd_A_intr_base  = sd_A_intr_base  * sound_prescaler_update/sound_prescaler;
    sd_A_intr_timer = sd_A_intr_timer * sound_prescaler_update/sound_prescaler;
    sd_B_intr_base  = sd_B_intr_base  * sound_prescaler_update/sound_prescaler;
    sd_B_intr_timer = sd_B_intr_timer * sound_prescaler_update/sound_prescaler;
    sound_prescaler = sound_prescaler_update;
    sound_prescaler_update = 0;
  }


  switch( sound_flags_update ){		/* �t���O�ύX����������A�X�V */

  case 0x27:		/*---------------- RESET_B/A | ENABLE_B/A | LOAD_B/A */
    data = sound_reg[0x27];

					/* LOAD�̗����オ��ɁA�^�C�}�l�X�V */
    if( (sound_LOAD_A==0) && (data&0x01) ) sd_A_intr_timer = sd_A_intr_base;
    if( (sound_LOAD_B==0) && (data&0x02) ) sd_B_intr_timer = sd_B_intr_base;
    sound_LOAD_A = data & 0x01;
    sound_LOAD_B = data & 0x02;

					/* ENABLE ��ۑ� */
    sound_ENABLE_A = ( data & 0x04 ) ? 1 : 0;
    sound_ENABLE_B = ( data & 0x08 ) ? 1 : 0;

					/* RESET �� 1 �Ȃ�A�t���O�N���A */
    if( data & 0x10 ) sound_FLAG_A = 0;
    if( data & 0x20 ) sound_FLAG_B = 0;
    break;

  case 0x29:		/*---------------- EN_ZERO/BRDY/EOS/TB/TA  */
    if( sound_board==SOUND_II ){
      data = sound_reg[0x29];
      sound2_EN_TA   = data & 0x01;
      sound2_EN_TB   = data & 0x02;
      sound2_EN_EOS  = data & 0x04;
      sound2_EN_BRDY = data & 0x08;
      sound2_EN_ZERO = data & 0x10;
    }
    break;

  case 0x10:		/*---------------- IRQ_RET | MSK_ZERO/BRDY/EOS/TB/TA */
    data = sound2_reg[0x10];
    if( data & 0x80 ){
      sound_FLAG_A     = 0;
      sound_FLAG_B     = 0;
      sound2_FLAG_EOS  = 0;
      sound2_FLAG_BRDY = 0;
      sound2_FLAG_ZERO = 0;
    }else{
      sound2_MSK_TA   = data & 0x01;
      sound2_MSK_TB   = data & 0x02;
      sound2_MSK_EOS  = data & 0x04;
      sound2_MSK_BRDY = data & 0x08;
      sound2_MSK_ZERO = data & 0x10;
    }
    break;

  }

  sound_flags_update = 0;

}


/*
 * �T�E���h�{�[�hII�֘A
 */
void	interval_work_set_BDRY( void )
{
  sd2_BRDY_intr_base  = sound2_intr_base * 2 * ( CPU_CLOCK_MHZ / 4.0 );
  sd2_BRDY_intr_timer = sd2_BRDY_intr_base;

/*printf("%d\n",sd2_BRDY_intr_base);*/
}
void	interval_work_set_EOS( int length )
{
  sd2_EOS_intr_base  = sd2_BRDY_intr_base * length;
  sd2_EOS_intr_timer = sd2_EOS_intr_base;

/*printf("%d\n",sd2_EOS_intr_base);*/
}



/************************************************************************/
/* 1/60 sec �̊��荞�ݖ��ɍs�Ȃ�����					*/
/************************************************************************/

/* #define ���ƁAVSYNC�J�n���ɕ\���A#undef ����VBLANK �I�����ɕ\�� */
#undef	DRAW_SCREEN_AT_VSYNC_START

int snd = 0;

static	void	vsync( void )
{
//	if(snd == 0)
  xmame_sound_update();			/* �T�E���h�o�� */
//   snd = !snd;

  if( !no_wait ) wait_vsync();		/* �����ԂŃE�G�C�g����� */


  xmame_update_video_and_audio();	/* �T�E���h�o�� ����2 */

  scan_joystick();                      /* Joystick ���� */
  scan_keyboard(0);			/* �L�[����	*/
  scan_mouse();				/* �}�E�X����	*/
  scan_expose();
#ifdef	DRAW_SCREEN_AT_VSYNC_START
  draw_screen();			/* ��ʏo��	*/
#endif

  scan_focus();				/* �E�C���h�E�t�H�[�J�X�`�F�b�N	*/


  state_of_cpu -= state_of_vsync;	/* (== vsync_intr_base) */
}






/*----------------------------------------------------------------------*/
/* ���荞�݂𐶐�����B�Ɠ����ɁA���̊��荞�݂܂ł́A�ŏ� state ���v�Z	*/
/*	�A��l�́AZ80���������I���̃t���O(TRUE/FALSE)			*/
/*----------------------------------------------------------------------*/
#define	MIN(x,y)	(((x)<(y))?(x):(y))
int	main_INT_update( void )
{

  int	icount = rtc_intr_base;		/* ���̊��荞�ݔ����܂ł̍ŏ�state�� */
					/* �Ƃ肠�����ARTC�����̎����ŏ����� */


		/* -------- RS232C ���荞�� -------- */

  rs232c_intr_timer -= z80main_cpu.state;
  if( rs232c_intr_timer < 0 ){
    rs232c_intr_timer += rs232c_intr_base;
    if( sio_intr() ){
      RS232C_flag = TRUE;
    }
  }
  icount = MIN( icount, rs232c_intr_timer );


		/* -------- VSYNC ���荞�� -------- */

  state_of_cpu += z80main_cpu.state;

  vsync_intr_timer -= z80main_cpu.state;
  if( vsync_intr_timer < 0 ){
    vsync();					/* �E�G�C�g�A�\���A���� */
    ctrl_vrtc  = 1;				/* VBLANK�J�n */
    VSYNC_flag = TRUE;
    vsync_intr_timer += vsync_intr_base;
  }
  icount = MIN( icount, vsync_intr_timer );


		/* -------- VRTC ���� -------- */

  if( ctrl_vrtc ){
    if( vrtc_timer < 0 ){
      vrtc_timer = vrtc_base;
    }else{
      vrtc_timer -= z80main_cpu.state;
      if( vrtc_timer < 0 ){
	ctrl_vrtc  = 0;				/* VBLANK �I�� */
	/*VSYNC_flag = FALSE;*/			/* VSYNC�M����OFF���Ȃ� */
		/* OFF����ƌ����E�l�����Ƃ��������Ȃ�   thanks peach ! */
#ifndef	DRAW_SCREEN_AT_VSYNC_START
	draw_screen();				/* ��ʏo��	*/
#endif
      }
    }
  }
  if( ctrl_vrtc ) icount = MIN( icount, vrtc_timer );


		/* -------- RTC ���荞�� -------- */

  rtc_intr_timer -= z80main_cpu.state;
  if( rtc_intr_timer < 0 ){
    RTC_flag = TRUE;
    rtc_intr_timer += rtc_intr_base;
  }
  icount = MIN( icount, rtc_intr_timer );


		/* -------- SOUND TIMER A ���荞�� -------- */

  if( sound_LOAD_A ){
    sd_A_intr_timer -= z80main_cpu.state;
    if( sd_A_intr_timer < 0 ){
      xmame_sound_timer_over(0);
      sd_A_intr_timer += sd_A_intr_base;
      if( sound_ENABLE_A ){
	if( sound2_MSK_TA ) sound_FLAG_A = 0;
	else                sound_FLAG_A = 1;
	if( sound_FLAG_A && sound2_EN_TA ) SOUND_flag = TRUE;
      }
    }
    icount = MIN( icount, sd_A_intr_timer );
  }


		/* -------- SOUND TIMER B ���荞�� -------- */

  if( sound_LOAD_B ){
    sd_B_intr_timer -= z80main_cpu.state;
    if( sd_B_intr_timer < 0 ){
      xmame_sound_timer_over(1);
      sd_B_intr_timer += sd_B_intr_base;
      if( sound_ENABLE_B ){
	if( sound2_MSK_TB ) sound_FLAG_B = 0;
	else                sound_FLAG_B = 1;
	if( sound_FLAG_B && sound2_EN_TB ) SOUND_flag = TRUE;
      }
    }
    icount = MIN( icount, sd_B_intr_timer );
  }

  if( sound2_FLAG_PCMBSY ){

    sd2_BRDY_intr_timer -= z80main_cpu.state;
    if( sd2_BRDY_intr_timer < 0 ){
      sd2_BRDY_intr_timer += sd2_BRDY_intr_base;
      {
	if( sound2_MSK_BRDY ) sound2_FLAG_BRDY = 0;
	else                  sound2_FLAG_BRDY = 1;
	if( sound2_FLAG_BRDY && sound2_EN_BRDY ) SOUND_flag = TRUE;
      }
    }

    if( sound2_notice_EOS ){
      sd2_EOS_intr_timer -= z80main_cpu.state;
      if( sd2_EOS_intr_timer < 0 ){
	sd2_EOS_intr_timer += sd2_EOS_intr_base;
	if( sound2_MSK_EOS ) sound2_FLAG_EOS = 0;
	else                 sound2_FLAG_EOS = 1;
	if( sound2_FLAG_EOS && sound2_EN_EOS ) SOUND_flag = TRUE;
	if( !sound2_repeat )  sound2_FLAG_PCMBSY = 0;
	sound2_notice_EOS = FALSE;
      }
    }

  }


	/* �T�E���h�́A���荞�݂Ɋւ�郌�W�X�^���ύX����ĂȂ����m�F */

  check_sound_parm_update();


	/* ���̊��荞�ݔ����܂ł́A�X�e�[�g�����Z�b�g */

  if( cpu_load ) icount = MIN( icount, cpu_load );

  z80main_cpu.icount = icount;
  z80main_cpu.state  = 0;


	/* Ctrl-C ��AF12 �L�[���͂̍ۂ́A�����Œʒm */

  if( dual_cpu_count ){ dual_cpu_count--; return 1; }

  if( emu_mode!=EXECUTE ) return 1;
  else                    return 0;
}






/************************************************************************/
/* �}�X�J�u�����荞�݃G�~�����[�g					*/
/************************************************************************/
/*--------------------------------------*/
/* ������ (Z80���Z�b�g���ɌĂ�)		*/
/*--------------------------------------*/
void	main_INT_init( void )
{
  RS232C_flag  = FALSE;
  VSYNC_flag   = FALSE;
  RTC_flag     = FALSE;
  SOUND_flag   = FALSE;

  interval_work_init_all();
  ctrl_vrtc = 1;

/*
printf("CPU    %f\n",cpu_clock_mhz);
printf("SOUND  %f\n",sound_clock_mhz);
printf("SYNC   %f\n",vsync_freq_hz);
printf("RS232C %d\n",rs232c_intr_base);
printf("VSYNC  %d\n",vsync_intr_base);
printf("VRTC   %d\n",vrtc_base);
printf("RTC    %d\n",rtc_intr_base);
printf("A      %d\n",sd_A_intr_base);
printf("B      %d\n",sd_B_intr_base);
*/
}

/*----------------------------------------------*/
/* �`�F�b�N (�������� 1�X�e�b�v���ɌĂ΂��)	*/
/*						*/
/*	��P�jIM!=2 �̎��͊��荞�݂͂ǂ��Ȃ�H	*/
/*	��Q�jintr_priority ���^�̎��́A	*/
/*	      ���荞�ݏ�Ԃ͂ǂ��Ȃ�H		*/
/*		  (����A����)			*/
/*						*/
/*----------------------------------------------*/
int	main_INT_chk( void )
{
  int	intr_no = -1;

  /*  if( z80main_cpu.IM!=2 ) return -1;*/


  if( intr_level==0 ){				/* ���x���ݒ� 0 */
    {						/*    ���荞�݂͎󂯕t���Ȃ� */
      intr_no = -1;
    }
  }
  else if( intr_level>=1 &&			/* ���x���ݒ� 1 */
	   intr_sio_enable && RS232C_flag ){	/*    RS232S ��M ���荞�� */
    RS232C_flag = FALSE;
    intr_no = 0;
  }
  else if( intr_level>=2 &&			/* ���x���ݒ� 2 */
	   intr_vsync_enable && VSYNC_flag ){	/*    VSYNC ���荞�� */
    VSYNC_flag = FALSE;
    intr_no = 1;
  }
  else if( intr_level>=3 &&			/* ���x���ݒ� 3 */
	   intr_rtc_enable && RTC_flag ){	/*    1/600�b RTC ���荞�� */
    RTC_flag = FALSE;
    intr_no = 2;
  }
  else if( intr_level>=5 &&			/* ���x���ݒ� 5 */
	   intr_sound_enable && SOUND_flag ){	/*    SOUND TIMER ���荞�� */
    SOUND_flag = FALSE;
    intr_no = 4;
  }


  if( intr_no >= 0 ){
    if( intr_priority ) intr_level = 7;
    else                intr_level = intr_no;
    return intr_no;
  }else{
    return -1;
  }
}








/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_intr_work[]=
{
  { TYPE_INT,	&intr_level,		},
  { TYPE_INT,	&intr_priority,		},
  { TYPE_INT,	&intr_sio_enable,	},
  { TYPE_INT,	&intr_vsync_enable,	},
  { TYPE_INT,	&intr_rtc_enable,	},

  { TYPE_DOUBLE,&cpu_clock_mhz,		},
  { TYPE_DOUBLE,&sound_clock_mhz,	},
  { TYPE_DOUBLE,&vsync_freq_hz,		},

  { TYPE_INT,	&state_of_cpu,		},
  { TYPE_INT,	&state_of_vsync,	},

  { TYPE_INT,	&cpu_load,		},
  { TYPE_INT,	&no_wait,		},

  { TYPE_INT,	&RS232C_flag,		},
  { TYPE_INT,	&rs232c_intr_base,	},
  { TYPE_INT,	&rs232c_intr_timer,	},

  { TYPE_INT,	&VSYNC_flag,		},
  { TYPE_INT,	&vsync_intr_base,	},
  { TYPE_INT,	&vsync_intr_timer,	},

  { TYPE_INT,	&ctrl_vrtc,		},
  { TYPE_INT,	&vrtc_base,		},
  { TYPE_INT,	&vrtc_timer,		},

  { TYPE_INT,	&RTC_flag,		},
  { TYPE_INT,	&rtc_intr_base,		},
  { TYPE_INT,	&rtc_intr_timer,	},

  { TYPE_INT,	&SOUND_flag,		},
  { TYPE_INT,	&sd_A_intr_base,	},
  { TYPE_INT,	&sd_A_intr_timer,	},
  { TYPE_INT,	&sd_B_intr_base,	},
  { TYPE_INT,	&sd_B_intr_timer,	},

  { TYPE_INT,	&sound_flags_update,	},
  { TYPE_INT,	&sound_prescaler_update,},

  { TYPE_INT,	&sd2_BRDY_intr_base,	},
  { TYPE_INT,	&sd2_BRDY_intr_timer,	},
  { TYPE_INT,	&sd2_EOS_intr_base,	},
  { TYPE_INT,	&sd2_EOS_intr_timer,	},
};


int	suspend_intr( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_intr_work, 
			    countof(suspend_intr_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->intr ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_intr(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_intr_work, 
			   countof(suspend_intr_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}



void	main_INT_init_at_resume( void )
{
  /* ���W���[����̍ď����� */
  /* �s�v �c�c�c �Ȃ̂��H */
}
