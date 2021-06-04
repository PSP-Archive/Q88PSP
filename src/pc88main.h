#ifndef PC88MAIN_H_INCLUDED
#define PC88MAIN_H_INCLUDED


extern	int	boot_basic;			/* �N������ BASIC���[�h	*/
extern	int	boot_dipsw;			/* �N�����̃f�B�b�v�ݒ�	*/
extern	int	boot_from_rom;			/* �N���f�o�C�X�̐ݒ�	*/
extern	int	boot_clock_4mhz;		/* �N������ CPU�N���b�N	*/

extern	int	monitor_15k;			/* 15k ���j�^�[ 1:Yes 0:No  */

extern	char	*file_printer;			/* �p�������o�͂̃t�@�C���� */
extern	char	*file_serialout;		/* �V���A���o�͂̃t�@�C���� */
extern	char	*file_serialin;			/* �V���A�����͂̃t�@�C���� */
extern	char	*file_tapeload;			/* �e�[�v���͂̃t�@�C����   */
extern	char	*file_tapesave;			/* �e�[�v�o�͂̃t�@�C����   */




extern	int	high_mode;			/* �������[�h 1:Yes 0:No     */

extern	int	calendar_stop;			/* ���v��~�t���O	*/

	/**** �f�B�b�v�X�C�b�` ****/

#define SW_1_MASK	(0x3e)
#define	SW_2_MASK	(0x3f)
#define	SW_N88		(0x01)			/* 1: N88  / 0: N	*/
#define	SW_V1		(0x80)			/* 1: V1   / 0: V2	*/
#define	SW_H		(0x40)			/* 1: H    / 0: S	*/
#define	SW_ROMBOOT	(0x08)			/* 1: ROM  / 0: DISK	*/
#define	SW_4MHZ		(0x80)			/* 1: 4MHz / 0: 8MHz	*/


	/**** �h�^�n�|�[�g ****/

extern	byte	dipsw_1;			/* IN[30] �f�B�b�v�X�C�b�` 1 */
extern	byte	dipsw_2;			/* IN[31] �f�B�b�v�X�C�b�` 2 */
extern	byte	ctrl_boot;			/* IN[40] �f�B�X�N�u�[�g��� */
extern	int	memory_bank;			/* OUT[5C-5F] IN[5C] �o���N  */
extern	byte	cpu_clock;			/* IN[6E] CPU �N���b�N       */

extern	byte	misc_ctrl;			/* I/O[32] �e��Ctrl       */
extern	byte	ALU1_ctrl;			/* OUT[34] ALU Ctrl 1     */
extern	byte	ALU2_ctrl;			/* OUT[35] ALU Ctrl 2     */
extern	byte	baudrate_sw;			/* I/O[6F] �{�[���[�g     */
extern	word	window_offset;			/* I/O[70] WINDOW �I�t�Z�b�g*/
extern	byte	ext_rom_bank;			/* I/O[71] �g��ROM BANK   */

extern	byte	ext_ram_ctrl;			/* I/O[E2] �g��RAM����	  */
extern	byte	ext_ram_bank;			/* I/O[E3] �g��RAM�Z���N�g*/

extern	byte	jisho_rom_bank;			/* OUT[F0] ����ROM�Z���N�g*/
extern	byte	jisho_rom_ctrl;			/* OUT[F1] ����ROM�o���N  */


#define	MISC_CTRL_EBANK		(0x03)		/* EROM BANK 00..11        */
#define	MISC_CTRL_AVC		(0x0c)		/* AVC                     */
#define	MISC_CTRL_TEXT_MAIN	(0x10)		/* TEXT   MAIN-RAM/HIGH-RAM*/
#define	MISC_CTRL_ANALOG	(0x20)		/* PALETTE  ANALOG/DEGITAL */
#define	MISC_CTRL_EVRAM		(0x40)		/* VRAM     EXTEND/STANDARD*/
#define INTERRUPT_MASK_SOUND	(0x80)		/* SND INT  Disable/Enable */

#define	ALU1_CTRL_BLUE		(0x11)		/* ALU�����Ώۃv���[�� B   */
#define	ALU1_CTRL_RED		(0x22)		/*                     R   */
#define	ALU1_CTRL_GREEN		(0x44)		/*                     G   */

#define	ALU2_CTRL_DATA		(0x07)		/* ALU�F��r�f�[�^    0..7 */
#define	ALU2_CTRL_MODE		(0x30)		/* ALU�������[�h  00B..11B */
#define	ALU2_CTRL_VACCESS	(0x80)		/* �������A�N�Z�X VRAM/MAIN*/

#define	MEMORY_BANK_MAIN	(3)		/* �o���N�w��        MAIN  */
#define	MEMORY_BANK_GRAM0	(0)		/*		       B   */
#define	MEMORY_BANK_GRAM1	(1)		/*		       R   */
#define	MEMORY_BANK_GRAM2	(2)		/*		       G   */

#define	CPU_CLOCK_4HMZ		(0x80)		/* CPU CLOCK 4MHz / 8MHz   */

#define	EXT_ROM_NOT		(0x01)		/* �g�� ROM ��Z���N�g	   */

#define INTERRUPT_MASK_RTC	(0x01)		/* 1/600 ���荞�� ����     */
#define INTERRUPT_MASK_VSYNC	(0x02)		/* VSYNC ���荞�� ����     */
#define INTERRUPT_MASK_SIO	(0x04)		/* COM   ���荞�� ����     */

#define	JISHO_NOT_SELECT	(0x01)		/* ����ROM�Z���N�g	   */
#define	JISHO_BANK		(0x1f)		/* ����ROM�o���N	   */




	/**** �֐� ****/



void	pc88main_init( int reset_keyboard );
void	pc88main_term( void );
void	pc88main_break_point( void );
void	power_on_ram_init( void );

byte	main_mem_read( word addr );
void	main_mem_write( word addr, byte data );
byte	main_io_in( byte port );
void	main_io_out( byte port, byte data );



	/**** ���� BASIC ���[�h ****/

#define EndofBasicAddr 0xffff
#define HS_BASIC_COUNT 50000000	/* ���荞�݂Ȃ��ŉ񂷃X�e�[�g�� */

extern word highspeed_routine[];

extern int highspeed_mode;	/* ���� BASIC ���� ����Ȃ� �^      */
extern int highspeed_n88rom;	/* MAIN-ROM �o���N�I�����A�^	    */



	/**** PCG ****/

extern	int	use_pcg;	/* �^�ŁAPCG-8100�T�|�[�g	*/

	/**** �V���A���A�p������ ****/

extern	int	cmt_speed;	/* �e�[�v���x 0�Ŏ���   */
extern	int	cmt_intr;	/* �����Ńe�[�v�������� */
extern	int	cmt_wait;	/* �^�ŁA�e�[�v�Ǎ��E�F�C�g����(T88�̂�) */

int	sio_open_tapeload( void );
void	sio_close_tapeload( void );
int	sio_open_tapesave( void );
void	sio_close_tapesave( void );
int	sio_open_serialin( void );
void	sio_close_serialin( void );
int	sio_open_serialout( void );
void	sio_close_serialout( void );

int	sio_tape_rewind( void );
int	sio_tape_pos( long *cur, long *end );
int	sio_intr( void );

int	printer_open( void );
void	printer_close( void );


#endif	/* PC88MAIN_H_INCLUDED */
