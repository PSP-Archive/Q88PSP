#ifndef INITVAL_H_INCLUDED
#define INITVAL_H_INCLUDED


/*----------------------------------------------------------------------*/
/* �萔									*/
/*----------------------------------------------------------------------*/
#define	CONST_4MHZ_CLOCK	(3.9936)
#define	CONST_8MHZ_CLOCK	(CONST_4MHZ_CLOCK*2.0)
#define	CONST_VSYNC_FREQ	(55.4)


/*----------------------------------------------------------------------*/
/* �N�����̏����ݒ�l							*/
/*----------------------------------------------------------------------*/

enum BasicMode { BASIC_AUTO=-1, BASIC_N,    BASIC_V1S,  BASIC_V1H,  BASIC_V2 };
enum CpuClock  {                CLOCK_8MHZ, CLOCK_4MHZ  };
enum BootDevice{ BOOT_AUTO =-1, BOOT_DISK,  BOOT_ROM    };
enum SoundBoard{                SOUND_I,    SOUND_II    };
enum JishoRom  {                NOT_JISHO,  EXIST_JISHO };
enum MenuLang  {                LANG_ENGLISH, LANG_JAPAN, NR_LANG };
enum BaudRate  { BAUDRATE_75,   BAUDRATE_150,   BAUDRATE_300,  BAUDRATE_600,
		 BAUDRATE_1200, BAUDRATE_2400,  BAUDRATE_4800, BAUDRATE_9600,
		 BAUDRATE_19200 };


/* DEFAULT_BASIC    BASIC ���[�h       -1:���� / 0:N / 1:V1S / 2:V1H / 3:V2  */
/* DEFAULT_CLOCK    CPU�N���b�N        -1:���� / 0:8MHz / 1:4MHz             */
/* DEFAULT_BOOT     �N���f�o�C�X       -1:���� / 0:DISK / 1:ROM              */
/* DEFAULT_EXTRAM   �g��RAM             0:�Ȃ� / 1�`4:����(�J�[�h��)         */
/* DEFAULT_JISHO    ����ROM             0:�Ȃ� / 1:����                      */
/* DEFAULT_SOUND    �T�E���h�{�[�h      0:���� / 1:�T�E���h�{�[�hII����      */
/* DEFAULT_DIPSW    �f�B�b�v�X�C�b�`   16bit�l���w��                         */
/* DEFAULT_BAUDRATE �{�[���[�g         BADU_RATE_75 �` BADU_RATE_19200���w�� */

/* DEFAULT_VERBOSE         -verbose   �̏����l */
/* DEFAULT_FRAMESKIP       -frameskip �̏����l */
/* DEFAULT_CPU             -cpu       �̏����l */


#define	DEFAULT_BASIC		( BASIC_AUTO )
#define	DEFAULT_CLOCK		( CLOCK_4MHZ )
#define	DEFAULT_BOOT		( BOOT_AUTO  )
#define	DEFAULT_SOUND		( SOUND_I    )
#define	DEFAULT_JISHO		( NOT_JISHO  )
#define	DEFAULT_EXTRAM		( 0 )
#define	DEFAULT_DIPSW		( 0x391a )
#define	DEFAULT_BAUDRATE	( BAUDRATE_300 )

#define	DEFAULT_VERBOSE		(0x00)
#define	DEFAULT_FRAMESKIP	(2)
#define	DEFAULT_CPU		(0)


/*----------------------------------------------------------------------*/
/* DEFAULT_CPU_CLOCK_MHZ	���C�� CPU�̃N���b�N     double�l [MHz] */
/* DEFAULT_SOUND_CLOCK_MHZ	�T�E���h�`�b�v�̃N���b�N double�l [MHz] */
/* DEFAULT_VSYNC_FREQ_HZ	VSYNC ���荞�݂̎���     int�l    [Hz]  */
/* DEFAULT_WAIT_FREQ		�E�G�C�g�����p���g��     int�l    [Hz]  */

#define	DEFAULT_CPU_CLOCK_MHZ		CONST_4MHZ_CLOCK
#define	DEFAULT_SOUND_CLOCK_MHZ		CONST_4MHZ_CLOCK
#define	DEFAULT_VSYNC_FREQ_HZ		CONST_VSYNC_FREQ
#define	DEFAULT_WAIT_FREQ_HZ		CONST_VSYNC_FREQ


/*----------------------------------------------------------------------*/
/* ���t�@�C���Ȃǂ̃t�@�C����						*/
/*----------------------------------------------------------------------*/

#ifdef	UNIX88

#define	QUASI88RC_FILE		".quasi88rc"
#define	QUASI88STATE_FILE	".quasi88-state"
#define	QUASI88RC_ESC		'\\'

#else	/* WIN88, other */

#define	QUASI88RC_FILE		"QUASI88.INI"
#define	QUASI88STATE_FILE	"QUASI88.STA"
#define	QUASI88RC_ESC		'\\'

#endif

/*----------------------------------------------------------------------*/
/* ROM �C���[�W�t�@�C���̃t�@�C����					*/
/*----------------------------------------------------------------------*/

#define	N88_ROM_FILE		"N88.ROM"
#define	N88EXT0_ROM_FILE	"N88EXT0.ROM"
#define	N88EXT1_ROM_FILE	"N88EXT1.ROM"
#define	N88EXT2_ROM_FILE	"N88EXT2.ROM"
#define	N88EXT3_ROM_FILE	"N88EXT3.ROM"
#define	N88N_ROM_FILE		"N88N.ROM"
#define	N88SUB_ROM_FILE		"N88SUB.ROM"
#define	N88KNJ1_ROM_FILE	"N88KNJ1.ROM"
#define	N88KNJ2_ROM_FILE	"N88KNJ2.ROM"
#define	N88JISHO_ROM_FILE	"N88JISHO.ROM"
#define	FONT_FILE		"FONT.ROM"

/* M88 �Ŏg���t�@�C���� */
#define	N88EXT0_ROM_FILE2	"N88_0.ROM"
#define	N88EXT1_ROM_FILE2	"N88_1.ROM"
#define	N88EXT2_ROM_FILE2	"N88_2.ROM"
#define	N88EXT3_ROM_FILE2	"N88_3.ROM"
#define	N88N_ROM_FILE2		"N80.ROM"
#define	N88SUB_ROM_FILE2	"DISK.ROM"
#define	N88KNJ1_ROM_FILE2	"KANJI1.ROM"
#define	N88KNJ2_ROM_FILE2	"KANJI2.ROM"
#define	N88JISHO_ROM_FILE2	"JISYO.ROM"


/*----------------------------------------------------------------------*/
/* �h���C�u�̐� ����� �t�@�C�����̃C���[�W�̍ő吔			*/
/*	�h���C�u�̐� �� 2�ȊO�̒l�͖��Ή��B�ݒ��ς��Ȃ��悤�ɁI�I	*/
/*----------------------------------------------------------------------*/
enum{
  DRIVE_1,
  DRIVE_2,
  NR_DRIVE
};
#define	MAX_NR_IMAGE	(32)

#endif		/* INITVAL_H_INCLUDED */
