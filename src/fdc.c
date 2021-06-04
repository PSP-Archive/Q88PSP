/************************************************************************/
/*									*/
/* FDC �̐��� �� �f�B�X�N�C���[�W�̓ǂݏ���				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "drive.h"
#include "image.h"
#include "fdc.h"

#include "emu.h"		/* emu_mode			*/

#include "file-op.h"
#include "suspend.h"



static int fdc_break_flag = FALSE;

int	fdc_debug_mode = FALSE;	/* FDC �f�o�b�O���[�h�̃t���O		*/
int	disk_exchange = FALSE;	/* �f�B�X�N�^������ւ��t���O		*/
int	disk_ex_drv = 0;	/* �f�B�X�N�^������ւ��h���C�u		*/
				/*    drive 1 ... bit 0			*/
				/*    drive 2 ... bit 1			*/
/* ��L�����́Apeach���̒񋟂ɂ�� */

int	FDC_flag = 0;			/* FDC ���荞�ݐM��		*/
int	fdc_wait = 0;			/* FDC �� �E�G�C�g 0�� 1�L	*/


#define	MAX_DRIVE	(4)		/* FDC �Ő���\�ȍő�h���C�u�� */
					/* ���ۂɂ́ANR_DRIVE(==2)�����ɑΉ� */


/*
  �f�B�X�N���A�N�Z�X���ɔ����o�����ۂ̋���

  ID ����������O �� INDEX �p���X�����o���Ȃ��̂ŁA���܂ł����Ă��I��Ȃ�
  ID �ǂݍ��ݒ�   �� ID CRC �G���[����
  ID �� DATA �̊� �� DDAM ����莞�ԓ��Ɍ�����Ȃ��G���[
  DATA �ǂݍ��ݒ� �� DATA CRC �G���[
  �������ݓ���    �� �Ȃɂ��Ȃ��I��B(�ł��A�ǂގ���CRC�G���[�̂͂�)


  �G�~�����[�g���ɃC���[�W���j�󂳂�Ă�����
  �g���b�N������ �� �I�t�Z�b�g�� 0 �ɂȂ��Ă���^EOF�ɒB����
                                 �c�c �A���t�H�[�}�b�g�g���b�N�Ƃ��Ĉ���
  �Z�N�^������   �� EOT�ɒB����  �c�c ID ���������́AID CRC �G���[
                                      DATA ���������́ADATA CRC �G���[
  �������݃G���[ �� EOT�ɒB����  �c�c �Ȃɂ����������ɏI��


  ����
  �f�B�X�N�Z�b�g     �� PCNx2 �̃g���b�N�ɃV�[�N
  �f�B�X�N�C�W�F�N�g �� �f�B�X�N�����ݒ�
  READ ID            �� E-Phase �ɁAfseek/fread
  READ DATA          �� C-Phase �I����� fseek/fread ���A�o�b�t�@�ɓǂݍ���
                        E-Phase �ŁA���e��f���o��
  WRITE ID           �� C-Phase �� fseek�AE-Phase �ŁAfwrite
  WRITE DATA         �� E-Phase �ŁA���e���o�b�t�@�ɂ��߁A�I������ fwrite
  READ DAIGNOSTIC    �� C-Phase �ŁA�o�b�t�@�ɓ��e���쐬�AE-Phase �œf���o��
  SEEK/RECALIBRATE   �� PCN �X�V�B�f�B�X�N������� PCNx2 �̃g���b�N�ɃV�[�N
   C-Phase �œ����g���b�N�ɑ΂��鏈���́A�Z�N�^�̑������A�N�Z�X
             �Ⴄ�g���b�N�ɑ΂��鏈���́A�擪�Z�N�^���A�N�Z�X



  ���b�Z�[�W
  WRITE ID ���ɁA�t�H�[�}�b�g�T�C�Y��6250�o�C�g�ȏ�̎��́A���Ӄ��b�Z�[�W��
                 �\������B(�������݂͐���ɍs�Ȃ���)
		 �������݃T�C�Y���A0x1600�o�C�g(2D)�ȏ�̎��A(N=6,7�ł�
		 �t�H�[�}�b�g���܂�)�́A�x����\�����A�A���t�H�[�}�b�g���쐬
  WRITE ID ���ɁACHRN �� �Z�N�^�� N �����ƂȂ鎞�́A���Ӄ��b�Z�[�W��\��
  WRITE DATA ���ɁACHRN �� �Z�N�^�� N �����ƂȂ鎞�́A�x�����b�Z�[�W��\�����A
                   �������ށB(�����A���̃f�B�X�N�͂����g���Ȃ�)

  �����C���[�W
  �f�B�X�N��ݒ肷�鎞�A���ׂẴC���[�W���������A���O�A�����A�T�C�Y��
  ���[�N�ɋL�^���Ă����B(���ۂɂ͑S�Ăł͂Ȃ��AMAX_NR_IMAGE�܂�)
  ���̎��A��ꂽ�C���[�W(�V�[�N�G���[�A�w�b�_�̃��[�h�G���[�A�f�[�^�T�C�Y���A
  �w�b�_���e�ƈقȂ�)������ꍇ�� ���[�N detect_broken_image ��^�ɐݒ肵��
  �����A���̃C���[�W�ȍ~�̃C���[�W�̓A�N�Z�X�ł��Ȃ��悤�ɂ���B
  �Ȃ��A�G���[���o��ȑO�̃C���[�W�ɂ̓A�N�Z�X�\�B

  ���łɃI�[�v�����Ă���C���[�W�t�@�C���ɑ΂�����ύX����сA�u�����N
  �C���[�W�̒ǉ��́Adetect_broken_image ���^�̏ꍇ�͌x���������B

  ���ύX�A�C���[�W�ǉ��̍ۂɃG���[���o�����́A�ň��C���[�W�j���̉\����
  ����̂ŁA�x���������B

  �A�N�Z�X���̃`�F�b�N�Bfseek & fwrite �̑O�ɁA�C���[�W�̏I�[�̈ʒu���`�F�b�N
  ���āA�Ԉ���Ď��̃C���[�W�ɏ������܂Ȃ��悤�ɂ���B����́A��ꂽ�C���[�W
  �w�b�_�ɑ΂����ł���Bfseek & fread �̑O�ɂ��`�F�b�N���悤���Ǝv�������A
  �ǂ����ŏI�g���b�N�ɑ΂��Ă����L���łȂ����@�Ȃ̂ŁA��߂ɂ����B

*/


/*
 * �f�B�X�N�C���[�W�Ɋւ��郏�[�N (�h���C�u�P��)
 */
PC88_DRIVE_T	drive[ NR_DRIVE ];



/*
 * �f�B�X�N�C���[�W����Z�N�^��ǂ񂾎��ۂ́A�����i�[
 */
static	struct{
  Uchar   c;			/* �Z�N�^ ID �� C	*/
  Uchar   h;			/* �Z�N�^ ID �� H	*/
  Uchar   r;			/* �Z�N�^ ID �� R	*/
  Uchar   n;			/* �Z�N�^ ID �� N	*/
  Uchar   density;		/* �Z�N�^�̋L�^���x	*/
  Uchar   deleted;		/* DELETED DATA �t���O	*/
  Uchar   status;		/* PC98 BIOS�̃X�e�[�^�X*/
  Uchar   padding;
  int     sec_nr;		/* �g���b�N���̃Z�N�^��	*/
  int	  size;			/* DATA �T�C�Y		*/
} sec_buf;


/*
 * READ / WRITE ���̃f�[�^�͂����ɃZ�b�g
 *	WRITE ID �ł́A4�o�C�g�f�[�^�~�Z�N�^���������ɃZ�b�g����B
 *	READ DIAGNOSTIC �ł́A�x�^�f�[�^�̃C���[�W�������ɍ쐬�����B
 */

#define DATA_BUF_SIZE 0x4000	/* �ő� 2D/2DD=6250byte�A2HD=10416byte ��? */

static	Uchar	data_buf[ DATA_BUF_SIZE ];


/*
 * FDC �̊e���񃏁[�N
 *	�z�X�g����󂯎�����R�}���h�A������̃X�e�[�^�X�́A�����ɁB
 */
static	struct{

  int	command;		/* �R�}���h (enum�l)	*/
  int	phase;			/* PHASE (C/E/R)	*/
  int	step;			/* PAHSE���̏����菇	*/
  int	counter;		/* �e��J�E���^		*/
  int	data_ptr;		/* �f�[�^�]���̃|�C���^	*/

  int	limit;			/* �f�[�^�]���^�C���A�E�g�_�E���J�E���^ */
  int	wait;			/* ���������s�J�n�܂ł̃E�G�C�g		*/
  int	seek_wait[MAX_DRIVE];	/* �h���C�u�� �V�[�N�ɂ����鎞�� (4�䕪)*/

  byte	status;			/* STATUS		*/
  byte	read;			/* DATA  for  FDC->MAIN */
  byte	write;			/* DATA  for  FDC<-MAIN */
  byte	TC;			/* TC (1 or 0 )		*/

  Uchar	cmd;			/* �R�}���h		*/
  Uchar	sk;			/* SK �r�b�g		*/
  Uchar	mf;			/* MF �r�b�g		*/
  Uchar	mt;			/* MT �r�b�g		*/
  Uchar	us;			/* US �ԍ�		*/
  Uchar	hd;			/* HD ��		*/
  Uchar	c;			/* ID - C		*/
  Uchar	h;			/* ID - H		*/
  Uchar	r;			/* ID - R		*/
  Uchar	n;			/* ID - N		*/
  Uchar	eot;			/* EOT �ԍ�		*/
  Uchar	gpl;			/* GPL ����		*/
  Uchar	dtl;			/* DTL ����		*/
  Uchar	d;			/* D   �f�[�^		*/
  Uchar	sc;			/* SC  �Z�N�^��		*/
  Uchar	stp;			/* STP �Ԋu		*/
  Uchar	srt_ms;			/* SRT(ms���Z)		*/
  Uchar	hut;			/* HUT			*/
  Uchar	hlt_nd;			/* HLT & ND		*/
  Uchar	ncn[MAX_DRIVE];		/* NCN �ʒu (4�䕪)	*/
  Uchar	pcn[MAX_DRIVE];		/* PCN �ʒu (4�䕪)	*/
  Uchar	st0;			/* ST0			*/
  Uchar	st1;			/* ST1			*/
  Uchar	st2;			/* ST2			*/
  Uchar	st3;			/* ST3			*/
  Uchar	st0_seek[MAX_DRIVE];	/* SEEK����ST0 (4�䕪)	*/
  Uchar	sense_us;		/* SENSE_I_S ������ US	*/
  Uchar	skip_ddam_job;		/* �X�L�b�v�w�莞�̏���	*/

} fdc;


/* �e��}�N�� */


#define	disk_not_exist(drv)	(drive[drv].fp==NULL  || drive[drv].empty)
#define	disk_unformat(drv)	(drive[drv].sec_nr<=0 || drive[drv].empty)
#define disk_unformatable(drv)	(drive[drv].sec_nr<0  || drive[drv].empty)

#define	sector_density_mismatch()					\
		(((sec_buf.density==DISK_DENSITY_SINGLE)&&( fdc.mf))||	\
		 ((sec_buf.density==DISK_DENSITY_DOUBLE)&&(!fdc.mf)) )

#define	idr_match()							\
		( fdc.c==sec_buf.c && fdc.h==sec_buf.h &&		\
		  fdc.r==sec_buf.r && fdc.n==sec_buf.n )

#define	printf_system_error( code )					      \
    do{									      \
      if     ( code==1 ) pgPrint_drawbg(3,32,0xFFFF,0,"FDC Read/Write Error");\
      else if( code==2 ) pgPrint_drawbg(3,32,0xFFFF,0,"FDC Seek Error");      \
      else if( code==3 ) pgPrint_drawbg(3,32,0xFFFF,0,"FDC Over-size Error"); \
      else               pgPrint_drawbg(3,32,0xFFFF,0,"Internal error !");			      \
    }while(0)
//      if     ( code==1 ) printf("FDC Read/Write Error in DRIVE %d:\n", drv+1);\
//      else if( code==2 ) printf("FDC Seek Error in DRIVE %d:\n", drv+1);      \
//      else if( code==3 ) printf("FDC Over-size Error in DRIVE %d:\n", drv+1); \
//      else               printf("Internal error !\n");			      \

#define effective_size( size )						\
	((size)>=0x1000) ? ((size)&0xf000 ) : 				\
		(((size)>=0x100) ? ((size)&0xf00) : ((size)&0x80))


/*	------ �f�B�X�N�C���[�W�̃X�e�[�^�X ------			*/
/*		���ۂɈӖ��̂���X�e�[�^�X�͈ȉ��̂Ƃ���		*/
/*			STATUS_MA	���̃Z�N�^�� ID �͖���		*/
/*			STATUS_DE	ID CRC Error			*/
/*			STATUS_MA_MD	���̃Z�N�^�� DATA �͖���	*/
/*			STATUS_DE_DD	DATA CRC Error			*/
/*			STATUS_CM	���� (DELETED DATA ����)	*/
/*			���̑�		����				*/

#define	STATUS_NORMAL	(0x00)		/* Normal End			*/
#define	STATUS_CM	(0x10)		/* Control Mark			*/
#define	STATUS_ALIGN	(0x20)		/* Alignment Error		*/
#define	STATUS_EN	(0x30)		/* End of Cylinder		*/
#define STATUS_EC	(0x40)		/* Equipment Check		*/
#define	STATUS_OR	(0x50)		/* Over Run			*/
#define	STATUS_NR	(0x60)		/* Not Ready			*/
#define	STATUS_NW	(0x70)		/* Not Writable			*/
#define	STATUS_UNDEF	(0x80)		/* Another Error		*/
#define	STATUS_TIMEOUT	(0x90)		/* Time out			*/
#define	STATUS_DE	(0xa0)		/* Data Error (ID)		*/
#define	STATUS_DE_DD	(0xb0)		/* Data Error (DATA)		*/
#define	STATUS_ND	(0xc0)		/* No Data			*/
#define	STATUS_BC	(0xd0)		/* Bad Cylinder			*/
#define	STATUS_MA	(0xe0)		/* Missing Address Mark (ID)	*/
#define	STATUS_MA_MD	(0xf0)		/* Missing Address Mark (DATA)	*/




/* FDC �� �X�e�[�^�X */

#define FD0_BUSY	(0x01)
#define	FD1_BUSY	(0x02)
#define	FD2_BUSY	(0x04)
#define	FD3_BUSY	(0x08)
#define FDC_BUSY	(0x10)
#define NON_DMA		(0x20)
#define DATA_IO		(0x40)
#define	REQ_MASTER	(0x80)

/* FDC �� ���U���g�X�e�[�^�X */

#define	ST0_US		(0x03)		/* Unit Select			*/
#define	ST0_HD		(0x04)		/* Head Address			*/
#define	ST0_NR		(0x08)		/* Not Ready			*/
#define	ST0_EC		(0x10)		/* Equipment Check		*/
#define	ST0_SE		(0x20)		/* Seek End			*/
#define	ST0_IC		(0xc0)		/* Interrupt Code		*/
#define	ST0_IC_NT	(0x00)		/* 	Normal Terminate	*/
#define	ST0_IC_AT	(0x40)		/* 	Abnormal Terminate	*/
#define	ST0_IC_IC	(0x80)		/* 	Invalid Command		*/
#define	ST0_IC_AI	(0xc0)		/* 	Attention Interrupt	*/

#define	ST1_MA		(0x01)		/* Missing Address Mark		*/
#define	ST1_NW		(0x02)		/* Not Writable			*/
#define	ST1_ND		(0x04)		/* No Data			*/
#define	ST1_OR		(0x10)		/* Over Run			*/
#define	ST1_DE		(0x20)		/* Data Error			*/
#define	ST1_EN		(0x80)		/* End of Cylinder		*/

#define	ST2_MD		(0x01)		/* Missing Address Mark in Data	*/
#define	ST2_BC		(0x02)		/* Bad Cylinder			*/
#define	ST2_SN		(0x04)		/* Scan Not Satisfied		*/
#define	ST2_SH		(0x08)		/* Scan Equal Hit		*/
#define	ST2_NC		(0x10)		/* No Cylinder			*/
#define	ST2_DD		(0x20)		/* Data Error in Data Files	*/
#define	ST2_CM		(0x40)		/* Control Mark			*/

#define	ST3_US		(0x03)		/* Unit Select			*/
#define	ST3_HD		(0x04)		/* Head Address			*/
#define	ST3_TS		(0x08)		/* Two Side			*/
#define	ST3_T0		(0x10)		/* Track 0			*/
#define	ST3_RY		(0x20)		/* Ready			*/
#define	ST3_WP		(0x40)		/* Write Protect		*/
#define	ST3_FT		(0x80)		/* Fault			*/



/* FDC �� �R�}���h */

enum FdcCommand
{
  WAIT			= 0,		/* �����҂��̏�� */
  READ_DATA,
  READ_DELETED_DATA,
  READ_DIAGNOSTIC,
  READ_ID,
  WRITE_DATA,		/* = 5 */
  WRITE_DELETED_DATA,
  WRITE_ID,
  SCAN_EQUAL,
  SCAN_LOW_OR_EQUAL,
  SCAN_HIGH_OR_EQUAL,	/* = 10 */
  SEEK,
  RECALIBRATE,
  SENSE_INT_STATUS,
  SENSE_DEVICE_STATUS,
  SPECIFY,		/* = 15 */
  INVALID,
  EndofFdcCmd
};
enum FdcPhase
{
  C_PHASE,
  E_PHASE,
  R_PHASE,
  EndofFdcPhase
};








/************************************************************************/
/* fdc �f�o�b�O								*/
/************************************************************************/
/*
 * FDC �f�o�b�O�����́Apeach���ɂ��񋟂���܂����B
 */
void pc88fdc_break_point(void)
{
    int i;
    for(i = 0; i < NR_BP; i++){
	if (break_point_fdc[i].type != BP_NONE) {
	    fdc_break_flag = TRUE;
	    return;
	}
    }
    fdc_break_flag = FALSE;
}


#ifndef	USE_MONITOR

#define	print_fdc_status(nStatus,nDrive,nTrack,nSector)

#else	/* USE_MONITOR */

void print_fdc_status(int nStatus, int nDrive, int nTrack, int nSector)
{
    static int oDrive = -1;
    static int oTrack[2];
    static int oSector[2];
    static int oStatus;
    char c = ' ';
    int i;
    
    if (fdc_debug_mode == TRUE) {
	if (oDrive < 0 || nStatus != oStatus || nDrive != oDrive ||
	    nTrack != oTrack[nDrive])
	{
	    oStatus = nStatus;
	    oDrive = nDrive;
	    oTrack[oDrive] = nTrack;
	    oSector[oDrive] = nSector;
	    switch (nStatus) {
	    case BP_READ:  c = 'R'; break;
	    case BP_WRITE: c = 'W'; break;
	    case BP_DIAG:  c = 'D'; break;
	    }
	    printf("\n%c D:%d T:%d S:%d", c, nDrive+1, nTrack, nSector+1);
	    fflush(stdout);
	} else if (nSector != oSector[nDrive]){
	    oSector[nDrive] = nSector;
	    printf(",%d", nSector+1);
	    fflush(stdout);
	}
    }
    
    if (fdc_break_flag == TRUE) {
	for (i = 0; i < NR_BP; i++) {
	    if (break_point_fdc[i].type == nStatus &&
	        break_point_fdc[i].drive == nDrive + 1 &&
		break_point_fdc[i].track == nTrack) {
		if (break_point_fdc[i].sector == nSector + 1 ||
		    break_point_fdc[i].sector < 0) {
		    printf( "*** Break at D:%d T:%d S:%d *** ",
			    nDrive + 1, nTrack, nSector + 1);
		    switch (nStatus) {
		    case BP_READ:  printf("( Read )\n"); break;
		    case BP_WRITE: printf("( Write )\n"); break;
		    case BP_DIAG:  printf("( Diag )\n"); break;
	 	    }
		    emu_mode = MONITOR;
		    break;
		}
	    }
	}
    }
 
}
#endif	/* USE_MONITOR */

/************************************************************************/
/* �Z�N�^�Ԃ𖄂߂�                                                     */
/************************************************************************/
/*
 * READ DIAG �̃Z�N�^�Ԃ̃f�[�^�𖄂߂鏈���́Apeach���ɂ��񋟂���܂����B
 */
static int fill_sector_gap(int ptr, int drv, Uchar fdc_mf);








/************************************************************************/
/* �h���C�u�̏�����							*/
/************************************************************************/
static	void	fdc_init( void );
void	drive_init( void )
{
  int	i;

  fdc_init();

  for( i=0; i<NR_DRIVE; i++ ){
    drive[ i ].fp     = NULL;
    drive[ i ].sec_nr = -1;
    drive[ i ].empty  = TRUE;
    drive[ i ].filename = NULL;
  }
  disk_ex_drv = 0;
}


/************************************************************************/
/* �h���C�u�̃��Z�b�g�B(���ݐݒ肳��Ă���C���[�W�Ƀ��[�N���Đݒ�)	*/
/************************************************************************/
void	drive_reset( void )
{
  int	i;

  fdc_init();

  for( i=0; i<NR_DRIVE; i++ ){
    if( drive[ i ].fp ){
      disk_change_image( i, drive[ i ].selected_image );
    }
  }
  disk_ex_drv = 0;
}



/************************************************************************/
/* �h���C�u �Ƀf�B�X�N��}������					*/
/*	drv�c�h���C�u(0/1)  filename�c�t�@�C����  dsk�c�f�B�X�N�ԍ�(0�`)*/
/*									*/
/*	1. �w��̃f�B�X�N�C���[�W�t�@�C�����㏑�����[�h�ŃI�[�v������B	*/
/*	2. �������݋֎~�̎��́A���[�h�I�����[�Ńt�@�C�����I�[�v������B	*/
/*	3. ���łɊJ���Ă���t�@�C���̏ꍇ�́A�����R�s�[		*/
/*	4. fdc.pcn[drv]*2 �g���b�N�ɃV�[�N���A�e�탏�[�N�������B	*/
/*	�G���[���́A�f�B�X�N���Z�b�g������ 1 ��Ԃ��B			*/
/*									*/
/*		�EDRIVE �� �������͊������Ă��邱�ƁB			*/
/*		�Einsert �̑O�� eject ���s�Ȃ��Ă��邱�ƁB		*/
/************************************************************************/
static	void	disk_insert_error( char *s, int drv )
{
  if( emu_mode != MENU_MAIN  ||
     ( emu_mode == MENU_MAIN && verbose_proc )){
    printf( "\n" "[[[ %-26s ]]]\n", s );
    printf(      "[[[   Eject Disk from drive %d: ]]]\n" "\n", drv+1 );
	pgPrint_drawbg(3,32,0xFFFF,0,s);
  }
  disk_eject( drv );
}


int	disk_insert( int drv, char *filename, int img )
{
  int	i, exit_flag;
  Uchar c[32];
  long	offset;
  int	num, same_image;
  char	*fname;
  int	status;


	/* READ/WRITE �Ńt�@�C�����J���B			*/
	/*	�G���[�Ȃ烁�b�Z�[�W��\���� 1��Ԃ��B		*/
	/*	����������A�t�@�C�������Z�b�g����B		*/

  drive[ drv ].filename = NULL;
  drive[ drv ].file_size = osd_file_size ( filename );
  drive[ drv ].fp = osd_fopen_image( filename, FALSE, &fname, &status );

  if( drive[ drv ].fp == NULL ){
    if( status == FOPEN_NO_MEM ){
      disk_insert_error( "Out of Memory", drv );
    }else{
      disk_insert_error( "Open Error", drv );
    }
    return 1;
  }

  if( status == FOPEN_OK_RO ){
    if( verbose_proc ) printf( " (( Read Only in drive %d: ))\n", drv+1 );
    drive[ drv ].read_only = TRUE;
  }else{
    drive[ drv ].read_only = FALSE;
  }

  drive[ drv ].filename = fname;




	/* �����t�@�C�������łɊJ���Ă��Ȃ����`�F�b�N����B	*/
	/*	2 �h���C�u�ł���ƌ��肵�A���΂̃h���C�u������	*/

  if( drive[ drv^1 ].fp ){				/*���΃h���C�u�g�p�� */

    switch( osd_file_cmp( drive[ drv ].filename, drive[ drv^1 ].filename ) ){
    case FILE_CMP_OK:						/* ��v */
      same_image = TRUE;
      break;
    case FILE_CMP_NOT_OK:					/* �s��v */
      same_image = FALSE;
      break;
    case FILE_CMP_ERR:						/* �G���[ */
      disk_insert_error( "Same Check Failed", drv );	return 1;
    default:
      disk_insert_error( "Internal Error", drv );	return 1;
    }

  }else{						/* ���΃h���C�u���g�p*/
    same_image = FALSE;
  }


	/* ���΃h���C�u�Ɠ����t�@�C���̎��́A���΃h���C�u�̃��[�N���R�s�[ */
	/* �����łȂ����́A�t�@�C����ǂ�ŁA�e�탏�[�N��ݒ�		  */

  if( same_image ){			/* ���΃h���C�u�Ɠ����t�@�C���̏ꍇ */

    ex_fclose( drive[ drv ].fp );
    drive[ drv ].fp                  = drive[ drv^1 ].fp;
    drive[ drv ].read_only           = drive[ drv^1 ].read_only;
    drive[ drv ].file_size           = drive[ drv^1 ].file_size;
    drive[ drv ].over_image          = drive[ drv^1 ].over_image;
    drive[ drv ].detect_broken_image = drive[ drv^1 ].detect_broken_image;
    drive[ drv ].image_nr            = drive[ drv^1 ].image_nr;
    memcpy( &drive[ drv   ].image,
	    &drive[ drv^1 ].image, sizeof(drive[ drv ].image) );

  }else{				/* ���΃h���C�u�ƈႤ�t�@�C���̏ꍇ */

    // drive[ drv ].file_size = osd_file_size( drive[ drv ].filename );
    if( drive[ drv ].file_size == -1 ){
      disk_insert_error( "Access Error(size)", drv );	return 1;
    }

    drive[ drv ].over_image          = FALSE;
    drive[ drv ].detect_broken_image = FALSE;

    num = 0;	offset = 0;
    exit_flag = FALSE;

    while( !exit_flag ){
      switch( fread_header( drive[ drv ].fp, offset, c ) ){

      case 0:					/* �C���[�W���擾���� */
	for(i=0;i<16;i++) drive[ drv ].image[ num ].name[i] = c[i];
	drive[ drv ].image[ num ].name[16] = '\0';
	drive[ drv ].image[ num ].protect  = c[DISK_PROTECT];
	drive[ drv ].image[ num ].type     = c[DISK_TYPE];
	drive[ drv ].image[ num ].size     = read_size_in_header( c );
	offset += read_size_in_header( c );
	num ++;
	if( num >= MAX_NR_IMAGE           &&	/* �C���[�W�����������͒��f */
	    offset < drive[ drv ].file_size ){
	  if( verbose_proc ) 
	    printf(" (( %s : Image number over %d ))\n",filename,MAX_NR_IMAGE);
	  drive[ drv ].over_image = TRUE;
	  exit_flag = TRUE;
	}
	break;

      case 1:					/* ����ȏ�C���[�W���Ȃ� */
	exit_flag = TRUE;
	break;

      case 2:					/* ���̃C���[�W�͉��Ă��� */
	if( verbose_proc )
	  printf(" (( %s : Image No. %d Broken ))\n",filename,num+1);
	drive[ drv ].detect_broken_image = TRUE;
	exit_flag = TRUE;
	break;

      case 3:					/* �V�[�N�G���[���� */
	printf(" (( %s : Image No. %d Seek error ))\n",filename,num+1);
	drive[ drv ].detect_broken_image = TRUE;
	exit_flag = TRUE;
	break;

      }
    }

    if( num==0 ){ disk_insert_error( "Image broken", drv );  return 1; }
    drive[ drv ].image_nr = num;

  }



	/* disk_top ��img ���ڂ̃f�B�X�N�C���[�W�̐擪�ɐݒ�	*/

  if( img >= drive[ drv ].image_nr ){
    disk_insert_error( "Image Number Bad", drv );	return 1;
  }

  disk_change_image( drv, img );



  return 0;
}



/************************************************************************/
/* �C���[�W��ύX����B							*/
/*	disk_top ��img ���ڂ̃f�B�X�N�C���[�W�̐擪�ɐݒ肵�A		*/
/*	pcn*2 �g���b�N�̐擪�Ɉړ�����B				*/
/*	drv�c�h���C�u(0/1)						*/
/************************************************************************/
static void disk_now_track( int drv, int trk );

int	disk_change_image( int drv, int img )
{
  int	i;



  if( drive[ drv ].fp==NULL ){				/* �t�@�C���������� */
    return 1;
  }
  
  if( img >= drive[ drv ].image_nr ){			/* �w��C���[�W���� */
    return 2;
  }

		/* disk_top ���v�Z */

  drive[ drv ].selected_image = img;
  drive[ drv ].empty          = FALSE;

  drive[ drv ].disk_top = 0;
  for( i=0; i<img; i++ ){
    drive[ drv ].disk_top += drive[ drv ].image[ i ].size;
  }
  drive[ drv ].disk_end = drive[ drv ].disk_top + drive[ drv ].image[img].size;
  drive[ drv ].protect  = drive[ drv ].image[ img ].protect;
  drive[ drv ].type     = drive[ drv ].image[ img ].type;



  if( drive[ drv ].read_only ) drive[ drv ].protect = DISK_PROTECT_TRUE;


		/* pcn*2 �g���b�N�̐擪�Ɉړ����� */

  disk_now_track( drv, fdc.pcn[drv]*2 );

  if (disk_exchange) disk_ex_drv |= 1 << drv;	/* �f�B�X�N����ւ������ */

  return 0;
}



/************************************************************************/
/* �f�B�X�N���C�W�F�N�g����						*/
/*	2 �h���C�u�ł���ƌ��肵�A���΂̃h���C�u�Ɣ�r����B		*/
/*	�����ł���΁A���[�N���������B�Ⴆ�΁A�t�@�C�������		*/
/*	drv�c�h���C�u(0/1)						*/
/************************************************************************/
void	disk_eject( int drv )
{
  if( drive[ drv ].fp ){
    if( drive[ drv ].fp != drive[ drv^1 ].fp ){
      ex_fclose( drive[ drv ].fp );
    }
  }
  drive[ drv ].fp = NULL;
  drive[ drv ].sec_nr = -1;
  drive[ drv ].empty  = TRUE;

  if( drive[ drv ].filename ){
    ex_free( drive[ drv ].filename );
    drive[ drv ].filename = NULL;
  }
}

/************************************************************************/
/* �h���C�u���ꎞ�I�ɋ�ɂ���^���Ƃɖ߂��^�ؑւ���^�ǂ����̏�Ԃ��m��	*/
/*	drv�c�h���C�u(0/1)						*/
/************************************************************************/
void	drive_set_empty( int drv )
{
  drive[ drv ].empty  = TRUE;
}
void	drive_unset_empty( int drv )
{
  drive[ drv ].empty  = FALSE;
}
void	drive_change_empty( int drv )
{
  drive[ drv ].empty  ^= 1;
}
int	drive_check_empty( int drv )
{
  return drive[ drv ].empty;
}










/*======================================================================*/
/* �w�b�h���g���b�N�̐擪�Ɉړ�����					*/
/*	drv�c�h���C�u(0/1)  trk�c�g���b�N�ԍ�(0�`)			*/
/*									*/
/*	�G���[���o���ꍇ�́A���̃g���b�N�̓A���t�H�[�}�b�g�ɂȂ�B	*/
/*======================================================================*/
static int disk_now_sec( int drv );

static	void	disk_now_track( int drv, int trk )
{
  int	error = 0;
  Uchar c[4];
  long	track_top;



	/* �V�[�N�\�V�����_�̃`�F�b�N */

  if     ( drive[ drv ].type==DISK_TYPE_2D  && trk>=84  ) trk =  83;
  else if( drive[ drv ].type==DISK_TYPE_2DD && trk>=164 ) trk = 163;
  else if( drive[ drv ].type==DISK_TYPE_2HD && trk>=158 ) trk = 157;
  else if( trk>=164 ) trk = 163; /* �����܂ł̂΂��� 2DD/2HD �ɑΉ��ł��� */
						        /* thanks peach ! */

	/* ���[�N�ݒ� & ������ */

  drive[ drv ].track     = trk;
  drive[ drv ].sec       = 0;


	/* �g���b�N�̃C���f�b�N�X�Ŏw�肳�ꂽ�t�@�C���ʒu���擾 */

  if( ex_fseek( drive[ drv ].fp,
	     drive[ drv ].disk_top + DISK_TRACK + trk*4,  SEEK_SET )==0 ){
    if(ex_fread( c, sizeof(Uchar), 4, drive[ drv ].fp )==4 ){

	/* �g���b�N����сA�擪�Z�N�^�̈ʒu��ݒ�   */
	/* ���̃Z�N�^�̃Z�N�^��񂨂�сA�Z�N�^���𓾂� */

      track_top = (long)c[0]+((long)c[1]<<8)+((long)c[2]<<16)+((long)c[3]<<24);
      if( track_top!=0 ){
	drive[ drv ].track_top =
	drive[ drv ].sec_pos   = drive[ drv ].disk_top + track_top;
	drive[ drv ].sec_nr    = disk_now_sec( drv );
      }else{
	drive[ drv ].track_top =
	drive[ drv ].sec_pos   = drive[ drv ].disk_top;
	drive[ drv ].sec_nr    = -1;
      }
    }
    else error = 1;
  } else error = 2;

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }

	/* �G���[���́A�A���t�H�[�}�b�g(�ăt�H�[�}�b�g�s�\)�ɂ��āA�߂� */

  if( error ){
    drive[ drv ].track_top =
    drive[ drv ].sec_pos   = drive[ drv ].disk_top;
    drive[ drv ].sec_nr    = -1;
  }
  return;
}






/*======================================================================*/
/* �w�肳�ꂽ�f�B�X�N�̌��݂̃Z�N�^�̏���ǂ݂Ƃ�			*/
/*	drv�c�h���C�u(0/1)						*/
/*									*/
/*	�G���[���́A���̃Z�N�^�̂� ID CRC Error �G���[�ɐݒ肷��B	*/
/*	�Ԃ�l�́A���̃Z�N�^�́A�u�Z�N�^��(DISK_SEC_NR)�v�̒l		*/
/*======================================================================*/
static	int	disk_now_sec( int drv )
{
  int	error = 0;
  Uchar	c[16];

	/* �t�@�C���ʒu sec_pos �� ID��� ��ǂ݁A�Z�N�^����Ԃ� */

  if( ex_fseek( drive[ drv ].fp,  drive[ drv ].sec_pos,  SEEK_SET )==0 ){
    if(ex_fread( c, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
      sec_buf.c       = c[DISK_C];
      sec_buf.h       = c[DISK_H];
      sec_buf.r       = c[DISK_R];
      sec_buf.n       = c[DISK_N];
      sec_buf.density = c[DISK_DENSITY];
      sec_buf.deleted = c[DISK_DELETED];
      sec_buf.status  = c[DISK_STATUS];
      sec_buf.sec_nr  = c[DISK_SEC_NR] + (int)c[DISK_SEC_NR+1]*256;
      sec_buf.size    = c[DISK_SEC_SZ] + (int)c[DISK_SEC_SZ+1]*256;
      if( sec_buf.status==STATUS_CM ){
	sec_buf.deleted = DISK_DELETED_TRUE;
	sec_buf.status  = STATUS_NORMAL;
      }
    }
    else error = 1;
  } else error = 2;

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }



	/* ���s������AID CRC Error �ɂ��A0 (==unformat) ��Ԃ� */

  if( error ){
    sec_buf.sec_nr  = 0;
    sec_buf.status  = STATUS_DE;
  }

  return ( sec_buf.sec_nr );
}


/*======================================================================*/
/* �w�肳�ꂽ�f�B�X�N�̎��̃Z�N�^�̏���ǂ݂Ƃ�			*/
/*	drv�c�h���C�u(0/1)						*/
/*======================================================================*/
static	void	disk_next_sec( int drv )
{
  int	overwrite_id;

	/* �A���t�H�[�}�b�g���́A�Ȃɂ����Ȃ� */

  if( disk_unformat( drv ) ) return;


	/* sec_top �����̃Z�N�^�ɁB�ŏI�Z�N�^�̎��̓g���b�N�擪�� */

			/* �~�b�N�X�Z�N�^�쐬���ɏ㏑�����ꂽ ID �̐� */
			/* ���̕ӂ����m�Ƀ`�F�b�N�ł��Ȃ��B�ǂ����悤 */

  if( sec_buf.size == 0x80 ||		/* sec_buf.size�� 0x80,0x100,0x200 */
     (sec_buf.size & 0xff) == 0 ){	/* 0x400,0x800,0x1000 �̏ꍇ(����) */
    overwrite_id = 0;
  }else{				/* ����ȊO�́A�~�b�N�X�Z�N�^����  */
    overwrite_id = ( sec_buf.size - ( 128 << (sec_buf.n & 7)) ) / SZ_DISK_ID;
    if( overwrite_id < 0 ) overwrite_id = 0;
  }


  drive[ drv ].sec += ( 1 + overwrite_id );
  if( drive[ drv ].sec < drive[ drv ].sec_nr ){

    drive[ drv ].sec_pos += (sec_buf.size + SZ_DISK_ID);

  }else{

    drive[ drv ].sec = 0;
    drive[ drv ].sec_pos = drive[ drv ].track_top;

  }

	/* sec_pos �� �Z�N�^ID��� ��ǂ� */

  disk_now_sec( drv );
}




















/*----------------------------------------------------------------------*/
/* READ ID								*/
/*	���݂̃Z�N�^ ID ������Ȃ�΁A���̓��e��Ԃ��B			*/
/*	�ُ�Ȃ�΁A���̃Z�N�^ ID ��ǂ�ŁA���Ȃ����B		*/
/*	2�����`�F�b�N���Ă�����ȃZ�N�^���������́A�G���[�ŏI���B	*/
/*	������̏ꍇ���A�߂�O�ɁA���̃Z�N�^��ǂ�ł����B		*/
/*	���[�N�j���펞	fdc.c, h, r, n, st0, st1, st2			*/
/*		�ُ펞	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
static	int	disk_read_id( void )
{
  int	index_cnt, exist_am;
  int	drv = fdc.us;


	/* �h���C�u 2/3 �͖��T�|�[�g */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    fdc.c=fdc.h=fdc.r=fdc.n=0xff;
    return 1;
  }

	/* �f�B�X�N���������́A���܂ł����Ă��I��Ȃ� */

  if( disk_not_exist( drv ) ) return 0;


	/* ���݂̃w�b�h�ʒu���w��ƈႤ���́A�w�b�h�ړ� */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }


	/* �A���t�H�[�}�b�g�̎��́A�I�� */

  if( disk_unformat( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    fdc.c=fdc.h=fdc.r=fdc.n=0xff;
    return 1;
  }


	/* ID CRC �G���[�̂Ȃ��ŏ��̃Z�N�^�𓾂�B */

  index_cnt = 0;				/* �C���f�b�N�X�z�[�����o��*/
  exist_am  = FALSE;				/* AM�� 1�x�ł�����������^*/

  while( 1 ){	/*===== �����邩�A�C���f�b�N�X�z�[��2�񌟏o�܂Ń��[�v =====*/

    if( index_cnt >= 2 ){		/* �C���f�b�N�X�z�[��2�񌟏o���� */
      if( exist_am ){
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_ND;
	fdc.st2 = 0;
	if( fdc.c!=fdc.pcn[drv] ) fdc.st2 |= ST2_NC;
	if( fdc.c==0xff )         fdc.st2 |= ST2_BC;
      }else{
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_MA;
	fdc.st2 = 0;
      }
      fdc.c=fdc.h=fdc.r=fdc.n=0xff;
      return 1;				/*	�� ���s 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* ���̃Z�N�^�ɂ� AM ���Ȃ� */
        sec_buf.status == STATUS_MA ){

      /* �c�O�B���̃Z�N�^�̌����ɐi�� */

    }else{					/* ���̃Z�N�^�ɂ� AM ������ */
      exist_am = TRUE;

      if( sec_buf.status == STATUS_DE ){		/* ID CRC Err�̎� */

	/* �c�O�B���̃Z�N�^�̌����ɐi�� */

      }else{						/* ID CRC ����̎� */
	/* �悵�A�Z�N�^���� ST0�`2 �ɐݒ� */
	break;
      }
    }

    disk_next_sec( drv );		/* ���̃Z�N�^�̌���	*/
  }		/*===========================================================*/



		/* ID ��ݒ肷�� */

  fdc.c   = sec_buf.c;
  fdc.h   = sec_buf.h;
  fdc.r   = sec_buf.r;
  fdc.n   = sec_buf.n;
  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  disk_next_sec( drv );
  return 1;
}


/*----------------------------------------------------------------------*/
/* READ DATA								*/
/* READ DELELTED DATA							*/
/*	���݂̃Z�N�^���珇�ɁAID �������B�w�肳�ꂽID��������΁A	*/
/*	�f�[�^���o�b�t�@�Ɋi�[�Bsec_top �͎��̃Z�N�^�̐擪�������B	*/
/*	�g���b�N2���� ID ���������Ă��w�肵��ID���������̓G���[�B���̎�	*/
/*	sec_top �̓g���b�N�̐擪���w���B				*/
/*	ID �� N �� size ���قȂ�ꍇ�Asize >= N �Ȃ�A�]��̃f�[�^��	*/
/*	���������Bsize < N �Ȃ�A����Ȃ������͓K���ɖ��߂���B	*/
/*	���̎��AID �X�e�[�^�X�͕K���ADATA CRC �G���[�ɂ���B		*/
/*	���[�N�j���펞	fdc.st0, st1, st2 / data_buf			*/
/*		�ُ펞	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : �w��� ID ������   */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_data__id( void )
{
  int	index_cnt, exist_am;
  int	drv = fdc.us;


	/* �h���C�u 2/3 �͖��T�|�[�g */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* �f�B�X�N���������́A���܂ł����Ă��I��Ȃ� */

  if( disk_not_exist( drv ) ) return 0;


	/* ���݂̃w�b�h�ʒu���w��ƈႤ���́A�w�b�h�ړ� */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }


	/* �A���t�H�[�}�b�g�̎��́A�I�� */

  if( disk_unformat( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* IDR �ƈ�v����Z�N�^��T���B		*/

  index_cnt = 0;				/* �C���f�b�N�X�z�[�����o��*/
  exist_am  = FALSE;				/* AM�� 1�x�ł�����������^*/

  while( 1 ){	/*===== �����邩�A�C���f�b�N�X�z�[��2�񌟏o�܂Ń��[�v =====*/

    if( index_cnt >= 2 ){		/* �C���f�b�N�X�z�[��2�񌟏o���� */
      if( exist_am ){
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_ND;
	fdc.st2 = 0;
	if( fdc.c!=fdc.pcn[drv] ) fdc.st2 |= ST2_NC;
	if( fdc.c==0xff )         fdc.st2 |= ST2_BC;
      }else{
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_MA;
	fdc.st2 = 0;
      }
      return 1;				/*	�� ���s 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* ���̃Z�N�^�ɂ� AM ���Ȃ� */
        sec_buf.status == STATUS_MA ){

      /* �c�O�B���̃Z�N�^�̌����ɐi�� */

    }else{					/* ���̃Z�N�^�ɂ� AM ������ */
      exist_am = TRUE;

      if( idr_match() ){			/* IDR ����v���� 	*/

	if( sec_buf.status == STATUS_DE ){		/* ID CRC Err�̎� */
	  fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	  fdc.st1 = ST1_DE;
	  fdc.st2 = 0;
	  disk_next_sec( drv );
	  return 1;					/*	�� ���s */
	}else{						/* ID CRC ����̎� */
	  /* �悵�A�f�[�^�����[�h�� */
	  break;					/*	�� ���� */
	}

      }else{					/*  IDR ��v���Ȃ�	*/

	/* �c�O�B���̃Z�N�^�̌����ɐi�� */

      }
    }

    disk_next_sec( drv );		/* ���̃Z�N�^�̌���	*/
  }		/*===========================================================*/



		/* DAM/DDAM ������ */

  if( sec_buf.status == STATUS_MA_MD ){		/* ---- DAM/DDAM ���Ȃ� */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = ST2_MD;
    disk_next_sec( drv );
    return 1;
  }


		/* DELETED ���ǂ����� ST2 �ɃZ�b�g����U�I���BST0 �� NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : ���̃Z�N�^�����[�h */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_data__data( void )
{
  int	read_size, size, ptr, error;
  int	drv = fdc.us;

  print_fdc_status(BP_READ, drv, drive[drv].track, drive[drv].sec);

		/* DATA CRC �G���[�̏ꍇ�ASTATUS ��ݒ� */

  if( sec_buf.status==STATUS_DE_DD ){
    fdc.st0 |= ST0_IC_AT;
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;
  }


		/* DATA ������ǂ� */

  read_size = 128 << (fdc.n & 7);		/* �ǂݍ��݃T�C�Y       */
  ptr       = 0;				/* �������݈ʒu		*/

  while( read_size > 0 ){	/*========== �w��T�C�Y���ǂݑ����� =========*/

    size = effective_size( sec_buf.size );
    if( ex_fseek( drive[ drv ].fp,
	       drive[ drv ].sec_pos + SZ_DISK_ID,  SEEK_SET )==0 ){
      if(ex_fread( &data_buf[ptr], sizeof(Uchar),
		 size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    if( error ){
      printf_system_error( error );
      fdc.st0 |= ST0_IC_AT;
      fdc.st1 |= ST1_DE;
      fdc.st2 |= ST2_DD;
    }

    ptr       += size;
    read_size -= size;
    if( read_size <= 0 ) break;


    fdc.st0 |= ST0_IC_AT;			/* ���̃Z�N�^�Ɍׂ��� */
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;
    if( verbose_fdc )
      printf("FDC Read Data : Sector OverRead in track %d (DRIVE %d:)\n",
	      drive[drv].track, drv+1);


	/* �Z�N�^�Ԃ𖄂߂�B�����ɂ́A   				*/
	/* DATA CRC, GAP3, ID SYNC, AM, ID, GAP2 �Ȃǂ�����(���e���쐬)	*/
		    /* CRC  GAP3  SYNC   AM    ID  CRC GAP2 */
    if( fdc.mf ) size = 2 + 0x36 + 12 + (3+1) + 4 + 2 + 22;
    else         size = 2 + 0x2a +  6 + (1+1) + 4 + 2 + 11;

    ptr       += size;
    read_size -= size;

    disk_next_sec( drv );
  }			/*===================================================*/

  disk_next_sec( drv );
  return 1;
}




/*----------------------------------------------------------------------*/
/* READ DIAGNOSTIC							*/
/*	track_top���珇�ɁAID �������BAM �̂���ID ���݂���΁A	*/
/*	IDR �̈�v�s��v�Ƃ͖��֌W�ɁA�f�[�^���o�b�t�@�Ɋi�[�B		*/
/*	�w��� N ���AID �� N �����傫����΁A���������̃Z�N�^�̃f�[�^	*/
/*	���o�b�t�@�Ɋi�[�B						*/
/*	����ȊO�͊�{�I�ɁAREAD DATA �ɏ�����				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : �w��� ID ������   */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_diagnostic__id( void )
{
  int	index_cnt;
  int	drv = fdc.us;


	/* �h���C�u 2/3 �͖��T�|�[�g */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* �f�B�X�N���������́A���܂ł����Ă��I��Ȃ� */

  if( disk_not_exist( drv ) ) return 0;


	/* �w�b�h�ړ�(�擪�Z�N�^�Ɉړ�����) */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }else{
    disk_now_track( drv,   drive[drv].track );
  }


	/* �A���t�H�[�}�b�g�̎��́A�I�� */

  if( disk_unformat( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* ID �̗L���ȁA�ŏ��̃Z�N�^�𓾂�B */

  index_cnt = 0;				/*�C���f�b�N�X�z�[�����o�� */

  while( 1 ){	/*===== �����邩�A�C���f�b�N�X�z�[��2�񌟏o�܂Ń��[�v =====*/

    if( index_cnt >= 2 ){		/* �C���f�b�N�X�z�[��2�񌟏o���� */
      fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
      fdc.st1 = ST1_MA;
      fdc.st2 = 0;
      return 1;				/*	�� ���s 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* ���̃Z�N�^�ɂ� AM ���Ȃ� */
        sec_buf.status == STATUS_MA ){

      /* �c�O�B���̃Z�N�^�̌����ɐi�� */

    }else{					/* ���̃Z�N�^�ɂ� AM ������ */

      if( sec_buf.status == STATUS_MA_MD ){		/* DATA mark �Ȃ� */

	return 0;					/* �� �n���O���� */

      }else{						/* DATA mark ���� */
	/* �悵�A�f�[�^�����[�h�_�C�A�O�m�X�e�B�b�N�� */
	break;
      }

    }

    disk_next_sec( drv );		/* ���̃Z�N�^�̌���	*/
  }		/*===========================================================*/



		/* ��U�߂�BST0 �� NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : �f�[�^���x�^�ǂ�   */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_diagnostic__data( void )
{
  int	diag_size, size, ptr, error;
  int	drv = fdc.us;

  print_fdc_status(BP_DIAG, drv, drive[drv].track, drive[drv].sec);

		/* �Ƃ肠�����ASTATUS ��ݒ� */

  if( sec_buf.status==STATUS_DE ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_DE;
    fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  else
  if( sec_buf.status==STATUS_DE_DD ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_DE;
    fdc.st2 = ST2_DD | ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  else{
    fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  if( !idr_match() ){
    fdc.st0 |= ST0_IC_AT;
    fdc.st1 |= ST1_ND;
  }


		/* DATA ������ǂ� */

  diag_size = 128 << (fdc.n & 7);		/* �ǂݍ��݃T�C�Y       */
  ptr       = 0;				/* �������݈ʒu		*/

  if((sec_buf.size==0x80 && diag_size!=0x80) ||	 /* �Z�N�^��N �ƁA�ǂ݂���*/
     (sec_buf.size & 0xff00) != diag_size    ){	 /* ��N ���Ⴄ���́A      */
    fdc.st0 |= ST0_IC_AT;			 /* DATA CRC err          */
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;
  }

  while( diag_size > 0 ){	/*========== �w��T�C�Y���ǂݑ����� =========*/

    size = effective_size( sec_buf.size );
    if( ex_fseek( drive[ drv ].fp,
	       drive[ drv ].sec_pos + SZ_DISK_ID,  SEEK_SET )==0 ){
      if(ex_fread( &data_buf[ptr], sizeof(Uchar),
		 size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    if( error ){
      printf_system_error( error );
      fdc.st0 |= ST0_IC_AT;
      fdc.st1 |= ST1_DE;
      fdc.st2 |= ST2_DD;
    }

    ptr       += size;
    diag_size -= size;
    if( diag_size <= 0 ) break;

    fdc.st0 |= ST0_IC_AT;			/* ���̃Z�N�^�Ɍׂ��� */
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;

	/* �Z�N�^�Ԃ𖄂߂�B�����ɂ́A   				*/
	/* DATA CRC, GAP3, ID SYNC, AM, ID, GAP2 �Ȃǂ�����		*/

#if 0	/* ���o�[�W�����ł́A�Z�N�^�Ԃ̃f�[�^�쐬�Ȃ� */
		    /* CRC  GAP3  SYNC   AM    ID  CRC GAP2 */
    if( fdc.mf ) size = 2 + 0x36 + 12 + (3+1) + 4 + 2 + 22;
    else         size = 2 + 0x2a +  6 + (1+1) + 4 + 2 + 11;

    ptr       += size;
    diag_size -= size;

    disk_next_sec( drv );

#else	/* peach�����A�Z�N�^�ԃf�[�^�����������񋟂���܂��� */

    size = fill_sector_gap(ptr, drv, fdc.mf);
    if (size < -1) return 1;

    ptr       += size;
    diag_size -= size;
#endif
  }			/*===================================================*/

  disk_next_sec( drv );
  return 1;
}






/*----------------------------------------------------------------------*/
/* WRITE ID								*/
/*	�g���b�N�̐擪����AID ����������ł����B			*/
/*	���[�N�j���펞	fdc.st0, st1, st2 / data_buf			*/
/*		�ُ펞	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : �f�B�X�N���`�F�b�N */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_id__check( void )
{
  int	drv = fdc.us;


	/* �h���C�u 2/3 �͖��T�|�[�g */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* �f�B�X�N���������́A���܂ł����Ă��I��Ȃ� */

  if( disk_not_exist( drv ) ) return 0;


	/* ���C�g�v���e�N�g���́A���U�ŏI�� */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_NW;
    fdc.st2 = 0;
    return 1;
  }

  
	/* ��U�I���BST0 �� NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : �t�H�[�}�b�g����   */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_id__write( void )
{
  int	size, error, i, j, id_ptr;
  long	format_pos;
  char	id[SZ_DISK_ID],	data[128];
  int	drv = fdc.us;
  char *fname;
  int status;

	/* �����̍Œ��ɁA�f�B�X�N���������ꂽ�肵�����̏��� */

  if( disk_not_exist( drv ) ) return 0;			/* �f�B�X�N���� */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){	/* �������݋֎~ */
    fdc.st0 = ST0_IC_AT | ST0_EC | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }


	/* �w�b�h�ړ�(�擪�Z�N�^�Ɉړ�����) */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }else{
    disk_now_track( drv,   drive[drv].track );
  }


	/* �t�H�[�}�b�g�s�\�̎��́A�Ȃɂ������������̂悤�ɏI�� */
	/* ^^^^^^^^^^^^^^^^ ---> DISK�C���[�W���̂����Ă��鎞 */

  if( disk_unformatable( drv ) ){
    if( verbose_fdc )
      printf("FDC Write ID : Track %d missing (DRIVE %d:)\n",
	     drive[drv].track, drv+1);
    fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }


	/* �t�H�[�}�b�g�f�[�^�̏��� */

  for( size=0; size<128; size++ ) data[size] = fdc.d;


	/*        �t�H�[�}�b�g�p�����[�^�̉��       */
	/* �g���b�N1�����z���Ȃ�������B�z�������́A */
	/* �Ō��1���Ɏ��܂镪�݂̂� WRITE ID ����B */

  id_ptr = 0;
  {
    int	SZ_BYTE = (drive[ drv ].type==DISK_TYPE_2HD) ? 10400 : 6250;
    int	SZ_GAP0 = (fdc.mf) ? (80+12+3+1    +50) : (40+6+1+1+    50);
    int	SZ_AM   = (fdc.mf) ? (   12+3+1+4+2+22) : (   6+1+1+4+2+11);
    int	SZ_DAM  = (fdc.mf) ? (   12+3+1+0+2+ 0) : (   6+1+1+0+2+ 0);
    int	max_sec;
    max_sec = (SZ_BYTE-SZ_GAP0)/( SZ_AM+SZ_DAM + 128*(1<<(fdc.n&7))+ fdc.gpl );
    if( fdc.sc > max_sec ){

      if( verbose_fdc )
        printf("FDC Write ID : Over Sector %d -> fixed %d (DRIVE %d:)\n",
	       fdc.sc, max_sec, drv+1 );

      id_ptr = (fdc.sc - max_sec) * 4;
      fdc.sc = max_sec;

    }
  }

	/* ID / DATA ���t�@�C���ɏ������� */

  format_pos = drive[ drv ].track_top;
  error = 0;

  if( fdc.sc==0 ){				/* �A���t�H�[�}�b�g���쐬 */

    for( i=0; i<SZ_DISK_ID; i++ ) id[ i ] = 0;

    if( format_pos + 16 <= drive[drv].disk_end ){
    if( ex_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
      if(ex_fwrite( id, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    } else error = 3;

  }else for( i=0; i<fdc.sc; i++ ){		/* �t�H�[�}�b�g���쐬 */

    id[ DISK_C ] = data_buf[ id_ptr++ ];
    id[ DISK_H ] = data_buf[ id_ptr++ ];
    id[ DISK_R ] = data_buf[ id_ptr++ ];
    id[ DISK_N ] = data_buf[ id_ptr++ ];
    id[ DISK_SEC_NR   ] = (fdc.sc    ) & 0xff;
    id[ DISK_SEC_NR+1 ] = (fdc.sc>>8 ) & 0xff;
    id[ DISK_DENSITY ] = (fdc.mf) ? DISK_DENSITY_DOUBLE : DISK_DENSITY_SINGLE;
    id[ DISK_DELETED ] = DISK_DELETED_FALSE;
    id[ DISK_STATUS ] = STATUS_NORMAL;
    id[ DISK_RESERVED+0 ] = 0;
    id[ DISK_RESERVED+1 ] = 0;
    id[ DISK_RESERVED+2 ] = 0;
    id[ DISK_RESERVED+3 ] = 0;
    id[ DISK_RESERVED+4 ] = 0;
    id[ DISK_SEC_SZ   ] = ( 128*(1<<(fdc.n&7))      ) & 0xff;
    id[ DISK_SEC_SZ+1 ] = ( 128*(1<<(fdc.n&7)) >> 8 ) & 0xff;

    if( verbose_fdc )
      if( id[ DISK_N ] != fdc.n )
        printf("FDC Write ID : Mix Sector in track %d (DRIVE %d:)\n",
	       drive[drv].track, drv+1);

    if( format_pos + 16 <= drive[drv].disk_end ){
    if( ex_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
      if(ex_fwrite( id, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
	format_pos += 16;

	for( j=0; j<(1<<(fdc.n&7)); j++ ){

	  if( format_pos + 128 <= drive[drv].disk_end ){
	  if( ex_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
	    if(ex_fwrite( data, sizeof(Uchar), 128, drive[ drv ].fp )==128 ){
	      format_pos += 128;
	    }
	    else{ error = 1; break; }
	  } else{ error = 2; break; }
	  } else{ error = 3; break; }

	}

      }
      else error = 1;
    } else error = 2;
    } else error = 3;

  }

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }

  fflush( drive[ drv ].fp );


	/* �r���A�V�X�e���̃G���[���N�����Ă�����I������ */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;


  disk_now_track( drv,   drive[drv].track );
  return 1;

}
/*----------------------------------------------------------------------*/
/* WRITE DATA								*/
/* WRITE DELELTED DATA							*/
/*	���݂̃Z�N�^���珇�ɁAID �������B�w�肳�ꂽID��������΁A	*/
/*	�o�b�t�@�̃f�[�^���������ށBsec_top �͎��̃Z�N�^�̐擪�������B	*/
/*	�g���b�N2���� ID ���������Ă��w�肵��ID���������̓G���[�B���̎�	*/
/*	sec_top �̓g���b�N�̐擪���w���B				*/
/*	ID �� N �� size ���قȂ�ꍇ�Asize >= N �Ȃ�A�]��̕����͍X�V	*/
/*	����Ȃ��Bsize < N �Ȃ�A���̃Z�N�^�ɂ܂������ăf�[�^���������� */
/*	���̎����̃Z�N�^�� ID �X�e�[�^�X�ُ͈�ɐݒ肳���B		*/
/*	���[�N�j���펞	fdc.st0, st1, st2 / data_buf			*/
/*		�ُ펞	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : �f�B�X�N���`�F�b�N */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_data__check( void )
{
  int	drv = fdc.us;

	/* �h���C�u 2/3 �͖��T�|�[�g */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* �f�B�X�N���������́A���܂ł����Ă��I��Ȃ� */

  if( disk_not_exist( drv ) ) return 0;


	/* ���C�g�v���e�N�g���́A���U�ŏI�� */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_NW;
    fdc.st2 = 0;
    return 1;
  }

  
	/* ��U�I���BST0 �� NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : �w��� ID ������   */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_data__search( void )
{
  int	index_cnt, exist_am;
  int	drv = fdc.us;


	/* �����̍Œ��ɁA�f�B�X�N���������ꂽ�肵�����̏��� */

  if( disk_not_exist( drv ) ) return 0;			/* �f�B�X�N���� */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){	/* �������݋֎~ */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* ���݂̃w�b�h�ʒu���w��ƈႤ���́A�w�b�h�ړ� */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }


	/* �A���t�H�[�}�b�g �y�сA�t�H�[�}�b�g�s�\�̎��́A�I�� */

  if( disk_unformat( drv ) || disk_unformatable( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* IDR �ƈ�v����Z�N�^��T���B		*/

  index_cnt = 0;				/* �C���f�b�N�X�z�[�����o��*/
  exist_am  = FALSE;				/* AM�� 1�x�ł�����������^*/

  while( 1 ){	/*===== �����邩�A�C���f�b�N�X�z�[��2�񌟏o�܂Ń��[�v =====*/

    if( index_cnt >= 2 ){		/* �C���f�b�N�X�z�[��2�񌟏o���� */
      if( exist_am ){
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_ND;
	fdc.st2 = 0;
	if( fdc.c!=fdc.pcn[drv] ) fdc.st2 |= ST2_NC;
	if( fdc.c==0xff )         fdc.st2 |= ST2_BC;
      }else{
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_MA;
	fdc.st2 = 0;
      }
      return 1;				/*	�� ���s 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* ���̃Z�N�^�ɂ� AM ���Ȃ� */
        sec_buf.status == STATUS_MA ){

      /* �c�O�B���̃Z�N�^�̌����ɐi�� */

    }else{					/* ���̃Z�N�^�ɂ� AM ������ */
      exist_am = TRUE;

      if( idr_match() ){			/* IDR ����v���� 	*/

	if( sec_buf.status == STATUS_DE ){		/* ID CRC Err�̎� */
	  fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	  fdc.st1 = ST1_DE;
	  fdc.st2 = 0;
	  disk_next_sec( drv );
	  return 1;					/*	�� ���s */
	}else{						/* ID CRC ����̎� */
	  /* �悵�A�f�[�^�����C�g�� */
	  break;					/*	�� ���� */
	}

      }else{					/*  IDR ��v���Ȃ�	*/

	/* �c�O�B���̃Z�N�^�̌����ɐi�� */

      }
    }

    disk_next_sec( drv );		/* ���̃Z�N�^�̌���	*/
  }		/*===========================================================*/



		/* ID ���������B��U�I���B ST0 �� NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 3 : ���̃Z�N�^�Ƀ��C�g */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_data__write( void )
{
  long	id_pos, write_pos;
  int	write_size, total_size, size, ptr, error;
  int	drv = fdc.us;
  unsigned char	c[2];
  char *fname;
  int status;

  print_fdc_status(BP_WRITE, drv, drive[drv].track, drive[drv].sec);

	/* �����̍Œ��ɁA�f�B�X�N���������ꂽ�肵�����̏��� */

  if( disk_not_exist( drv ) ) return 0;			/* �f�B�X�N���� */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){	/* �������݋֎~ */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }

  if( disk_unformat( drv ) || disk_unformatable( drv ) ){/* �A���t�H�[�}�b�g */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;		/*  �t�H�[�}�b�g�s�\ */
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


		/* DATA ���������� */

  id_pos     = drive[ drv ].sec_pos;		/* ID�������߂��ۂ̏��   */

  write_size = 128 << (fdc.n & 7);		/* �������݃T�C�Y         */
  ptr        = 0;				/* �ǂݏo���̃|�C���^     */
  write_pos  = drive[drv].sec_pos + SZ_DISK_ID;	/* �������݂̃t�@�C���ʒu */
  total_size = sec_buf.size;			/* �Z�N�^�̃f�[�^���T�C�Y */

  while( write_size > 0 ){	/*========== �w��T�C�Y������������ =========*/

    size = effective_size( sec_buf.size );
    if( write_pos + size <= drive[drv].disk_end ){
    if( ex_fseek( drive[ drv ].fp,
	       write_pos,  SEEK_SET )==0 ){
      if(ex_fwrite( &data_buf[ptr], sizeof(Uchar),
		 size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    } else error = 3;
    if( error ){
      printf_system_error( error );
    }

    ptr        += size;
    write_size -= size;

    if( write_size<=0 ) break;

    disk_next_sec( drv );			/* ���̃Z�N�^�Ɍׂ��� */

    write_pos  += size;
    total_size += (sec_buf.size + SZ_DISK_ID);

    if( verbose_fdc )
      printf("FDC Write Data : Sector OverWrite in track %d (DRIVE %d:)\n",
	     drive[drv].track, drv+1);

  }			/*===================================================*/


		/* ID �����X�V����B*/

  if( fdc.mf && fdc.n==0 ){
    c[0] = DISK_DELETED_FALSE;
    c[1] = STATUS_MA_MD;
  }else{
    if( fdc.command==WRITE_DATA ){
      c[0] = DISK_DELETED_FALSE;
      c[1] = STATUS_NORMAL;
    }else{
      c[0] = DISK_DELETED_TRUE;
      c[1] = STATUS_CM;
    }
  }
  if( ex_fseek( drive[ drv ].fp,			/* ID �́ADAM/DDAM���X�V */
	     id_pos + DISK_DELETED,  SEEK_SET )==0 ){
    if(ex_fwrite( &c[0], sizeof(Uchar), 2, drive[ drv ].fp )==2 ){
      error = 0;
    }
    else error = 1;
  } else error = 2;
  if( error ){
    printf_system_error( error );
  }


  c[0] = ( total_size >>  0  ) & 0xff;
  c[1] = ( total_size >>  8  ) & 0xff;
  if( ex_fseek( drive[ drv ].fp,			/* ID �́A�Z�N�^�T�C�Y���X�V */
	     id_pos + DISK_SEC_SZ,  SEEK_SET )==0 ){
    if(ex_fwrite( &c[0], sizeof(Uchar), 2, drive[ drv ].fp )==2 ){
      error = 0;
    }
    else error = 1;
  } else error = 2;
  if( error ){
    printf_system_error( error );
  }


//  fflush( drive[ drv ].fp );


	/* �r���A�V�X�e���̃G���[���N�����Ă�����I������ */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = ( (fdc.command==WRITE_DELETED_DATA)? ST2_CM: 0 );


  disk_next_sec( drv );
  return 1;
}










/************************************************************************/
/* FDC �̏�����								*/
/************************************************************************/
static	void	fdc_init( void )
{
  int	i;

  fdc.status  = 0 | REQ_MASTER;
  fdc.read    = 0xff;
  fdc.write   = 0xff;
  fdc.TC      = FALSE;

  fdc.command = WAIT;

  for( i=0; i<MAX_DRIVE; i++ ){
    fdc.seek_wait[ i ] = -1;
    fdc.ncn[ i ]  = 0;
    fdc.pcn[ i ]  = 0;
  }
}



/************************************************************************/
/* CPU�� FDC �ɃA�N�Z�X�������ɌĂԊ֐��S				*/
/*	void	fdc_write( byte data )	�c�c�c OUT A,(0FBH)		*/
/*	byte	fdc_read( void )	�c�c�c IN  A,(0FBH)		*/
/*	byte	fdc_status( void )	�c�c�c IN  A,(0FAH)		*/
/*	void	fdc_TC( void )		�c�c�c IN  A,(0F8H)		*/
/************************************************************************/
void	fdc_write( byte data )
{
  if( (fdc.status & DATA_IO)==0 ){
    fdc.status &= ~REQ_MASTER;
    fdc.write   = data;
  }
}

byte	fdc_read( void )
{
  if( (fdc.status & DATA_IO) ){
    fdc.status &= ~REQ_MASTER;
    return fdc.read;
  }else{
    return 0xff;
  }
}

byte	fdc_status( void )
{
  return fdc.status;
}

void	fdc_TC( void )
{
  fdc.TC = TRUE;
}

/************************************************************************/
/* FDC ����ACPU�ւ̊��荞�ݒʒm�֐� (�}�N��)				*/
/************************************************************************/
#define	fdc_occur_interrupt()	FDC_flag = TRUE
#define	fdc_cancel_interrupt()	FDC_flag = FALSE



/************************************************************************/
/* E-PHASE ����I�����ɁA���� CHRN ���w��				*/
/*	2D/2DD �̎������Ȃ��c�c�c ;_;					*/
/************************************************************************/
static	int	fdc_update_next_chrn( void )
{
  if( fdc.mt==0 ){		/* �}���`�g���b�N���������Ȃ��� */

    if( fdc.r == fdc.eot ){		/* �ŏI�Z�N�^�Ȃ�A	*/
      fdc.c ++;				/* C+=1, R=1 �ɂ���B	*/
      fdc.r = 1;			/* �Ԃ�l�� �G���[	*/
      return 1;				/*   (TC ���ɔ�����)	*/
    }else{
      fdc.r ++;				/* �ŏI�łȂ���΁AR+=1	*/
      return 0;				/* �Ԃ�l�� ����	*/
    }

  }else{			/* �}���`�g���b�N�����̎� */

    if( fdc.hd==0 ){		/*   �\�ʏ����� */

      if( fdc.r == fdc.eot ){		/* �ŏI�Z�N�^�Ȃ�A	*/
	fdc.hd = 1;			/* ���ʂɐؑւ��āA	*/
	fdc.h ^= 1;			/* H ���]�AR = 1	*/
	fdc.r  = 1;			/* �Ԃ�l�� ����	*/
	return 0;
      }else{
	fdc.r ++;			/* �ŏI�łȂ���΁AR+=1	*/
	return 0;			/* �Ԃ�l�� ����	*/
      }

    }else{			/*   ���ʏ����� */

      if( fdc.r == fdc.eot ){		/* �ŏI�Z�N�^�Ȃ�A	*/
	fdc.h ^= 1;			/* H ���]�A		*/
	fdc.c ++;			/* C+=1, R=1 �ɂ���B	*/
	fdc.r = 1;			/* �Ԃ�l�� �G���[	*/
	return 1;			/*   (TC ���ɔ�����)	*/
      }else{
	fdc.r ++;			/* �ŏI�łȂ���΁AR+=1	*/
	return 0;			/* �Ԃ�l�� ����	*/
      }
    }
  }
}



/************************************************************************/
/* FDC �� �������C��							*/
/*	������ �O��ɂ��̊֐����Ă񂾎�����̌o�ߎ���( 4MHz�X�e�[�g�� )	*/
/************************************************************************/

#define	RD	(4)		/*  1us (4MHz) */
#define	WR	(4)		/*  1us (4MHz) */
#define	BYTE	(128)		/* 32us (4MHz) */
#define	ID	(  22*BYTE)	/*     22 byte */
#define	DATA	( 296*BYTE)	/* 256+40 byte */
#define	TRACK	(6250*BYTE)	/*   6250 byte */
#define	SEEK	(4000)		/* 1ms  (4MHz) */

/*---------------------------------------------------------------------*/

#define	REPEAT()	do{ if( !fdc_wait ) loop_flag = TRUE; }while(0)
#define	ICOUNT(x)	do{ if(  fdc_wait ) fdc.wait = (x);   }while(0)
#define	SCOUNT(d,x)	do{ if(  fdc_wait ) fdc.seek_wait[(d)] = (x);	\
			    else            fdc.seek_wait[(d)] = 1;   }while(0)

#define	ABS(x)		( ((x) >= 0)? (x) : -(x) )


int	fdc_ctrl( int interval )
{
  int	i, w;
  int loop_flag;

  if( ! fdc_wait ){			/* �E�G�C�g�Ȃ��̏ꍇ */

    loop_flag = TRUE;				/* FDC�������s�Ȃ�*/

  }else{				/* �E�G�C�g����̏ꍇ */
    if( fdc.wait == -1 ){			/* wait==-1 �Ȃ�AFDC���� */
      loop_flag = TRUE;
    }else{					/* wait>0 �Ȃ�A���Ԍo�ߔ��� */
      fdc.wait -= interval;
      if (fdc.wait<=0){ fdc.wait=0;   loop_flag = TRUE;  }
      else            {               loop_flag = FALSE; }
    }
  }

	/* seek_wait[] �Ƀh���C�u�ʂ́A�V�[�N���Ԃ������Ă���		*/
	/*     > 1    �c �V�[�N�� (�l�́A�����܂ł�4MHz�X�e�[�g��)	*/
	/*    1 �� 0  �c 1�ŁA�V�[�N�����B���荞�݂𔭐����āA0�ɂȂ�B	*/
	/*    0 �� -1 �c �����������BSENSE INT �Ŋ����������A-1�ɂȂ�B	*/
	/*      -1    �c ����Ȃ�					*/

  for( i=0; i<MAX_DRIVE; i++ ){
    if( fdc.seek_wait[i] > 1 ){
      fdc.seek_wait[i] -= interval;
      if( fdc.seek_wait[i] <= 0 ) fdc.seek_wait[i] = 1;
    }
  }



  /******* FDC �����B�E�F�C�g�Ȃ��Ȃ�A�����ɂ���ẮA�������J��Ԃ� *******/

  while( loop_flag ){
    loop_flag = FALSE;


    switch( fdc.command ){

    case WAIT:		/* --------------------------- �R�}���h�҂� ---	*/
      if( !( fdc.status & REQ_MASTER ) ){	/* �R�}���h��M�����ꍇ	*/
	switch( fdc.write & 0x1f ){
	case 0x06:	fdc.command = READ_DATA;		break;
	case 0x0c:	fdc.command = READ_DELETED_DATA;	break;
	case 0x0a:	fdc.command = READ_ID;			break;
	case 0x0d:	fdc.command = WRITE_ID;			break;
	case 0x05:	fdc.command = WRITE_DATA;		break;
	case 0x09:	fdc.command = WRITE_DELETED_DATA;	break;
	case 0x02:	fdc.command = READ_DIAGNOSTIC;		break;
	case 0x11:	fdc.command = SCAN_EQUAL;		break;
	case 0x19:	fdc.command = SCAN_LOW_OR_EQUAL;	break;
	case 0x1d:	fdc.command = SCAN_HIGH_OR_EQUAL;	break;
	case 0x0f:	fdc.command = SEEK;			break;
	case 0x07:	fdc.command = RECALIBRATE;		break;
	case 0x08:	fdc.command = SENSE_INT_STATUS;		break;
	case 0x04:	fdc.command = SENSE_DEVICE_STATUS;	break;
	case 0x03:	fdc.command = SPECIFY;			break;
	default:	fdc.command = INVALID;			break;
	}
	fdc.phase = C_PHASE;
	fdc.step  = 0;
	fdc.TC    = FALSE;
	loop_flag = TRUE;				/* �����J��Ԃ� */

      }else {					/* �R�}���h�Ȃ��ꍇ	*/
	for( i=0; i<MAX_DRIVE; i++ ){			/* �V�[�N������ */
	  if( fdc.seek_wait[i] == 1 ){			/* �`�F�b�N���� */
	    if( (fdc.st0_seek[fdc.us] & ST0_NR) == 0){
	      fdc.pcn[ fdc.us ] = fdc.ncn[ fdc.us ];
	      if( !disk_not_exist( fdc.us ) ){
		disk_now_track( fdc.us, fdc.ncn[ fdc.us ]*2 + fdc.hd );
	      }
	    }
	    fdc_occur_interrupt();			/* ���荞�ݔ��� */
	    fdc.seek_wait[i] = 0;
	    break;
	  }
	}
	ICOUNT( -1 );
      }
      break;




    case READ_DATA:		/* ---------------------- READ DATA ---	*/
    case READ_DELETED_DATA:	/* -------------- READ DELETED DATA ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.sk  = (fdc.write & 0x20) >> 5;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.c   =  fdc.write;		break;
	  case 3:   fdc.h   =  fdc.write;		break;
	  case 4:   fdc.r   =  fdc.write;		break;
	  case 5:   fdc.n   =  fdc.write;		break;
	  case 6:   fdc.eot =  fdc.write;		break;
	  case 7:   fdc.gpl =  fdc.write;		break;
	  case 8:   fdc.dtl =  fdc.write;		break;
	  }
	  if( fdc.step < 8 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("READ(%d) sk%d mf%d mt%d us%d hd%d eot%d\n",
		   fdc.command,fdc.sk,fdc.mf,fdc.mt,fdc.us,fdc.hd,fdc.eot);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  logfdc("C:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);
	  fdc.skip_ddam_job = 0;
	  if( disk_read_data__id() ){	    /* ID�����J�n (�f�B�X�N�}�����^) */
	    if( fdc.st0 & ST0_IC ){		/* �������s */
	      logfdc("A\n");
	      fdc.step = 4;
	      ICOUNT( TRACK * 3/2 );
	    }
	    else				/* ���������AD/DAM�s��v */
	    if( ( fdc.command==READ_DATA         &&
		  ( fdc.st2 & ST2_CM )           )  ||
		( fdc.command==READ_DELETED_DATA &&
		  !( fdc.st2 & ST2_CM )          )  ){
	      if( fdc.sk ){			    /* �X�L�b�v���鎞 */
		if( fdc_update_next_chrn()==0 ){	/* ���Z�N�^��ǂ� */
		  logfdc("B\n");
		  fdc.step = 0;
		  ICOUNT( ID + DATA );
		}else{					/* ���Z�N�^�� EOT */
		  fdc.st0 |= ST0_IC_AT;
		  fdc.st1 |= ST1_EN;
		  logfdc("C\n");
		  fdc.step = 4;
		  ICOUNT( ID );
		}
	      }else{				    /* �X�L�b�v���Ȃ��� */
		fdc.skip_ddam_job = 1;
		disk_read_data__data();
		fdc.data_ptr = 0;
		if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
		else           fdc.counter = 128 << (fdc.n & 7);
		fdc.step ++;
		ICOUNT( ID );
	      }
	    }
	    else{				/* ���������AD/DAM��v */
	      disk_read_data__data();
	      fdc.data_ptr = 0;
	      if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
	      else           fdc.counter = 128 << (fdc.n & 7);
	      fdc.step ++;
	      ICOUNT( ID );
	    }
	    REPEAT();
	  }else{			    /* ID�����s�� (�f�B�X�N���}��) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Read : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC�M�� �L�ŁA���f */
	    if( fdc.st0 & ST0_IC ){		/* CRC�G���[ */
	      logfdc("D\n");
	    }else{				/* CRC���� */
	      if( fdc.skip_ddam_job==0 ){	    /* D/DAM�s��v�X�L�b�v�� */
		fdc_update_next_chrn();		    /* �ȊO�́A���Z�N�^���w��*/
		logfdc("E\n");
	      }else
		logfdc("F\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* TC�M�� ���ŁA�������� */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|DATA_IO|REQ_MASTER;
	    fdc.read   = data_buf[ fdc.data_ptr ++ ];
	    fdc.limit  = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* ��莞�Ԍo�߂Ń^�C���A�E�g */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Read : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC�M�� �L�ŁA���f */
	    fdc_cancel_interrupt();
	    if( fdc.st0 & ST0_IC ){		/* CRC�G���[ */
	      logfdc("G\n");
	    }else{				/* CRC���� */
	      if( fdc.skip_ddam_job==0 ){	    /* D/DAM�s��v�X�L�b�v�� */
		fdc_update_next_chrn();		    /* �ȊO�́A���Z�N�^���w��*/
		logfdc("H\n");
	      }else
		logfdc("I\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* �f�[�^���ǂ݂����ꂽ�ꍇ */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  fdc.limit += BYTE;}
	      else                {fdc.step --; ICOUNT( fdc.limit ); }
	    }
	    else{			    /* �܂��A�ǂ܂�Ȃ� */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.st0 & ST0_IC ){	    /* CRC�G���[����̏ꍇ */
	    logfdc("J\n");
	    fdc.step = 4;
	    ICOUNT( BYTE );
	    REPEAT();
	  }else{
	    if( fdc.TC ){		    /* TC�M�� �L�ŁA���f */
	      if( fdc.skip_ddam_job==0 ){	    /* D/DAM�s��v�X�L�b�v�� */
		fdc_update_next_chrn();		    /* �ȊO�́A���Z�N�^���w��*/
		logfdc("K\n");
	      }else
		logfdc("L\n");
	      fdc.step = 4;
	      ICOUNT( 0 );
	      REPEAT();
	    }else{
	      fdc.limit -= interval;
	      if( fdc.limit <= 0 ){	    /* TC�M�� ���ŁA��莞�Ԍo�� */
		/*if( verbose_fdc ) printf("FDC Read : Multi\n");*/
		if( fdc.skip_ddam_job==0 ){	    /* D/DAM�s��v�X�L�b�v�� */
		  if( fdc_update_next_chrn()==0 ){  /* �ȊO�́A���Z�N�^����  */
		    logfdc("M\n");
		    fdc.step = 0;
		  }else{			    /* ���Z�N�^�� EOT */
		    fdc.st0 |= ST0_IC_AT;
		    fdc.st1 |= ST1_EN;
		    logfdc("N\n");
		    fdc.step = 4;
		  }
		}else{				    /* D/DAM�s��v�X�L�b�v�� */
		  fdc.st0 |= ST0_IC_AT;
		  fdc.st1 |= ST1_EN;
		  logfdc("O\n");
		  fdc.step = 4;
		}
		ICOUNT( 0 );
		REPEAT();
	      }else{			    /* �܂�TC�M���Ȃ� */
		ICOUNT( fdc.limit );
	      }
	    }
	  }
	  break;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;	
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;






    case READ_DIAGNOSTIC:	/* ---------------- READ DIAGNOSTIC ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.mf  = (fdc.write & 0x40) >> 6;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.c   =  fdc.write;		break;
	  case 3:   fdc.h   =  fdc.write;		break;
	  case 4:   fdc.r   =  fdc.write;		break;
	  case 5:   fdc.n   =  fdc.write;		break;
	  case 6:   fdc.eot =  fdc.write;		break;
	  case 7:   fdc.gpl =  fdc.write;		break;
	  case 8:   fdc.dtl =  fdc.write;		break;
	  }
	  if( fdc.step < 8 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.mt     = 0;
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("DIAG mf%d us%d hd%d eot%d\n",fdc.mf,fdc.us,fdc.hd,fdc.eot);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  logfdc("C:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);
	  if( disk_read_diagnostic__id() ){ /* ID�����J�n (�f�B�X�N�}�����^) */
	    if( fdc.st0 & ST0_IC ){		/* �������s */
	      logfdc("A\n");
	      fdc.step = 4;
	      ICOUNT( TRACK + TRACK );
	    }
	    else{				/* �������� */
	      disk_read_diagnostic__data();
	      fdc.data_ptr = 0;
	      if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
	      else           fdc.counter = 128 << (fdc.n & 7);
	      fdc.step ++;
	      ICOUNT( ID + DATA );
	    }
	    REPEAT();
	  }else{			    /* ID�����s�� (�f�B�X�N���}��) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Diag : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC�M�� �L�ŁA���f */
	    if( fdc.st0 & ST0_IC ){		/* CRC�G���[ */
	      logfdc("B\n");
	    }else{				/* CRC���� */
	      fdc_update_next_chrn();
	      logfdc("C\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* TC�M�� ���ŁA�������� */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|DATA_IO|REQ_MASTER;
	    fdc.read   = data_buf[ fdc.data_ptr ++ ];
	    fdc.limit  = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* ��莞�Ԍo�߂Ń^�C���A�E�g */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Diag : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC�M�� �L�ŁA���f */
	    fdc_cancel_interrupt();
	    if( fdc.st0 & ST0_IC ){		/* CRC�G���[ */
	      logfdc("D\n");
	    }else{				/* CRC���� */
	      fdc_update_next_chrn();
	      logfdc("E\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* �f�[�^���ǂ݂����ꂽ�ꍇ */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  fdc.limit += BYTE;}
	      else                {fdc.step --; ICOUNT( fdc.limit ); }
	    }
	    else{			    /* �܂��A�ǂ܂�Ȃ� */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.TC ){			    /* TC�M�� �L�ŁA���f */
	    fdc_update_next_chrn();
	    logfdc("F\n");
	    fdc.step = 4;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{
	    fdc.limit -= interval;
	    if( fdc.limit <= 0 ){	    /* TC�M�� ���ŁA��莞�Ԍo�� */
	      /*if( verbose_fdc ) printf("FDC Diag : Multi\n");*/
	      if( fdc_update_next_chrn()==0 ){	    /* ���Z�N�^���� */
		logfdc("G\n");
		fdc.step = 0;
	      }else{				    /* ���Z�N�^��EOT */
		fdc.st0 |= ST0_IC_AT;
		fdc.st1 |= ST1_EN;
		logfdc("H\n");
		fdc.step = 4;
	      }
	      ICOUNT( 0 );
	      REPEAT();
	    }else{			    /* �܂�TC�M���Ȃ� */
	      ICOUNT( fdc.limit );
	    }
	  }
	  break;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;	
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;






    case READ_ID:		/* ------------------------ READ ID ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.sk  = (fdc.write & 0x20) >> 5;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  }
	  if( fdc.step < 1 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("READ ID mf%d us%d hd%d    ",fdc.mf,fdc.us,fdc.hd);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_read_id() ){		    /* ID�����J�n (�f�B�X�N�}�����^) */
	    fdc.step ++;
	    REPEAT();
	    ICOUNT( DATA + ID );
	  }else{			    /* ID�����s�� (�f�B�X�N���}��) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;	
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;

      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  logfdc("%02x %02x %02x %02x %02x %02x %02x\n",
		 fdc.st0,fdc.st1,fdc.st2,fdc.c,fdc.h,fdc.r,fdc.n);
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;






    case WRITE_ID:		/* ----------------------- WRITE ID ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.sk  = (fdc.write & 0x20) >> 5;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.n   =  fdc.write;		break;
	  case 3:   fdc.sc  =  fdc.write;		break;
	  case 4:   fdc.gpl =  fdc.write;		break;
	  case 5:   fdc.d   =  fdc.write;		break;
	  }
	  if( fdc.step < 5 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("W ID mf%d us%d n%d sc%d\n",fdc.mf,fdc.us,fdc.n,fdc.sc);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_id__check() ){	    /* ��������   (�f�B�X�N�}�����^) */
	    if( fdc.st0 & ST0_IC ){		/* �������ݕs�\ */
	      logfdc("A\n");
	      fdc.step = 5;
	      ICOUNT( 0 );
	    }
	    else{				/* �������݉\ */
	      fdc.data_ptr = 0;
	      if( fdc.sc==0 ){
		fdc.counter = 256 * 4;
		if( verbose_fdc ) printf("FDC Wr ID : no sector\n");
	      }else
		fdc.counter = fdc.sc * 4;
	      fdc.step ++;
	      ICOUNT( TRACK/2 );
	    }
	    REPEAT();
	  }else{			    /* ��������s�� (�f�B�X�N���}��) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Wr ID : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC�M�� �L�ŁA���f */
	    fdc.TC = FALSE;
	    fdc.step = 3;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* TC�M�� ���ŁA�������� */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|REQ_MASTER;
	    fdc.limit = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* ��莞�Ԍo�߂Ń^�C���A�E�g */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Wr ID : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC�M�� �L�ŁA���f */
	    fdc_cancel_interrupt();
	    fdc.TC = FALSE;
	    fdc.step ++;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* �f�[�^���������܂ꂽ�ꍇ */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      data_buf[ fdc.data_ptr ++ ] = fdc.write;
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  }
	      else                {fdc.step --; ICOUNT( fdc.limit
					+ ((fdc.counter%4)!=0)? 0: ID+DATA ); }
	      logfdc("%02X ",fdc.write);
	      if((fdc.counter%4)==0) { logfdc("\n"); }
	    }
	    else{			    /* �܂��A������Ȃ� */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.counter!=0 ){
	    if( verbose_fdc ) printf("FDC Write Id : missing\n");
	    while( (fdc.counter%4)!=0 ){		/* 4�o�C�g�ɖ����Ȃ� */
	      data_buf[ fdc.data_ptr ++ ] = 0x00;	/* ������00H�Ŗ��߂� */
	      -- fdc.counter;
	    }
	    fdc.sc = (4*fdc.sc - fdc.counter) /4;	/* ���̌� SC ���X�V */
	  }
	  fdc.step ++;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_id__write() ){	    /* �t�H�[�}�b�g(�f�B�X�N�}�����^)*/
	    fdc.step ++;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			   /*�t�H�[�}�b�g�s��(�f�B�X�N���}��)*/
	    ICOUNT( 0 );
	  }
	  break;
	case 5:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  REPEAT();
	  ICOUNT( 0 );
	  break;
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;




    case WRITE_DATA:		/* --------------------- WRITE DATA ---	*/
    case WRITE_DELETED_DATA:	/* ------------- WRITE DELETED DATA ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.c   =  fdc.write;		break;
	  case 3:   fdc.h   =  fdc.write;		break;
	  case 4:   fdc.r   =  fdc.write;		break;
	  case 5:   fdc.n   =  fdc.write;		break;
	  case 6:   fdc.eot =  fdc.write;		break;
	  case 7:   fdc.gpl =  fdc.write;		break;
	  case 8:   fdc.dtl =  fdc.write;		break;
	  }
	  if( fdc.step < 8 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("WRITE(%d) mf%d mt%d us%d eot%d\n",
		   fdc.command,fdc.mf,fdc.mt,fdc.us,fdc.eot);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_data__check() ){   /* ��������   (�f�B�X�N�}�����^) */
	    if( fdc.st0 & ST0_IC ){		/* �������ݕs�\ */
	      logfdc("A\n");
	      fdc.step = 7;
	    }else{				/* �������݉\ */
	      fdc.step ++;
	    }
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* ��������s�� (�f�B�X�N���}��) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_data__search() ){  /* ID�����J�n (�f�B�X�N�}�����^) */
	    if( fdc.st0 & ST0_IC ){		/* �������s */
	      logfdc("B\n");
	      fdc.step = 7;
	      ICOUNT( TRACK * 3/2 );
	    }else{				/* �������� */
	      logfdc("C:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);
	      fdc.data_ptr = 0;
	      if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
	      else           fdc.counter = 128 << (fdc.n & 7);
	      fdc.step ++;
	      ICOUNT( ID );
	    }
	    REPEAT();
	  }else{			    /* ID�����s�� (�f�B�X�N���}��) */
	    ICOUNT( 0 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Write : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC�M�� �L�ŁA���f */
	    logfdc("D\n");
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* TC�M�� ���ŁA�������� */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|REQ_MASTER;
	    fdc.limit = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* ��莞�Ԍo�߂Ń^�C���A�E�g */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Write : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC�M�� �L�ŁA���f */
	    fdc_cancel_interrupt();
	    logfdc("E\n");
	    fdc.step ++;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* �f�[�^���������܂ꂽ�ꍇ */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      data_buf[ fdc.data_ptr ++ ] = fdc.write;
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  fdc.limit += BYTE;}
	      else                {fdc.step --; ICOUNT( fdc.limit ); }
	    }
	    else{			    /* �܂��A������Ȃ� */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  while( fdc.counter ){
	    data_buf[ fdc.data_ptr ++ ] = 0x00;		/*	�Z�N�^�T�C�Y */
	    -- fdc.counter;				/*	�ɖ����Ȃ��� */
	  }						/*	�́A00H��    */
	  if( fdc.n==0 && fdc.dtl<128 ){		/*	���߂�B     */
	    for( i=0; i<128-fdc.dtl; i++ ){
	      data_buf[ fdc.data_ptr ++ ] = 0x00;
	    }
	  }
	  fdc.step ++;
	case 5:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_data__write() ){   /* �������� (�f�B�X�N�}�����^)   */
	    fdc.step ++;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* �������ݎ��s (�f�B�X�N���}��) */
	    ICOUNT( 0 );
	  }
	  break;
	case 6:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.st0 & ST0_IC ){	    /* �ُ킠��ŁA���I�� */
	    logfdc("F\n");
	    fdc.step ++;
	    ICOUNT( BYTE );
	    REPEAT();
	  }else{			    /* �ُ�Ȃ��ŁATC��҂� */
	    if( fdc.TC ){			/* TC�M�� �L�ŁA���f */
	      fdc_update_next_chrn();
	      logfdc("G\n");
	      fdc.step ++;
	      ICOUNT( 0 );
	      REPEAT();
	    }else{
	      fdc.limit -= interval;
	      if( fdc.limit <= 0 ){		/* TC�M�� ���ŁA��莞�Ԍo�� */
		/*if( verbose_fdc ) printf("FDC Write : Multi\n");*/
		if( fdc_update_next_chrn()==0 ){	    /* ���Z�N�^���� */
		  logfdc("H\n");
		  fdc.step = 1;
		}else{					    /* ���Z�N�^��EOT */
		  fdc.st0 |= ST0_IC_AT;
		  fdc.st1 |= ST1_EN;
		  logfdc("I\n");
		  fdc.step = 7;
		}
		ICOUNT( 0 );
		REPEAT();
	      }else{				/* �܂�TC�M���Ȃ� */
		ICOUNT( fdc.limit );
	      }
	    }
	  }
	  break;
	case 7:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;
	

      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;	break;
	  case  8:   fdc.read = fdc.h;	break;
	  case 10:   fdc.read = fdc.r;	break;
	  case 12:   fdc.read = fdc.n;	break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	  case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;




    case SEEK:		/* ----------------------------------- SEEK ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.ncn[ fdc.us ] =  fdc.write;	break;
	  }
	  if( fdc.step < 2 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    if( fdc.us >= NR_DRIVE ||			/* �����h���C�u or */
		fdc.status & (1<<fdc.us) ){		/* ���݃V�[�N��    */

	      fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	      fdc.st0_seek[fdc.us] = ST0_IC_AT | ST0_SE | ST0_NR | fdc.us;
	      SCOUNT( fdc.us, 1 );

	    }else{
	      fdc.status = (fdc.status&0x0f) | REQ_MASTER | (1<<fdc.us);
	      fdc.st0_seek[fdc.us] = ST0_IC_NT | ST0_SE | fdc.us;
	      SCOUNT( fdc.us,
		      1+ABS(fdc.ncn[fdc.us]-fdc.pcn[fdc.us])*fdc.srt_ms*SEEK);
	    }
	    logfdc("SEEK %d-%x (Tr.%d)\n", 
		   fdc.us, fdc.ncn[fdc.us], fdc.ncn[fdc.us]*2 );
	    fdc.command = WAIT;
	    fdc.step = 0;
	    ICOUNT( -1 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;
      }
      break;


    case RECALIBRATE:	/* ---------------------------- RECALIBRATE ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  }
	  if( fdc.step < 1 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.ncn[ fdc.us ] = 0;
	    if( fdc.us >= NR_DRIVE ||			/* �����h���C�u or */
		fdc.status & (1<<fdc.us) ){		/* ���݃V�[�N��    */

	      fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	      fdc.st0_seek[fdc.us] = ST0_IC_AT | ST0_SE | ST0_NR | fdc.us;
	      SCOUNT( fdc.us, 1 );

	    }else{
	      fdc.status = (fdc.status&0x0f) | REQ_MASTER | (1<<fdc.us);
	      fdc.st0_seek[fdc.us] = ST0_IC_NT | ST0_SE | fdc.us;
	      SCOUNT( fdc.us,
		      1+ABS(fdc.ncn[fdc.us]-fdc.pcn[fdc.us])*fdc.srt_ms*SEEK);
	    }
	    logfdc("RECALIBRATE %d\n",fdc.us);
	    fdc.command = WAIT;
	    fdc.step = 0;
	    ICOUNT( -1 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;
      }
      break;




    case SENSE_INT_STATUS:	/* --------------- SENSE_INT_STATUS ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  }
	  {
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = R_PHASE;
	    fdc.step   = 0;
	    logfdc("SENSE INT\n");
	    ICOUNT( RD );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  for( i=0; i<MAX_DRIVE; i++ )	/* �V�[�N����(��������)�h���C�u���� */
	    if( fdc.seek_wait[i]==0 ) break;
	  fdc.sense_us = i;
	  fdc.step ++;
	case 1:	case 3:	/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  if( fdc.sense_us==MAX_DRIVE ){	/* �V�[�N�����h���C�u�Ȃ� */
	    switch( fdc.step ){
	    case  1:   fdc.read = ST0_IC_IC;	break;
	    case  3:   fdc.read = 0;		break;
	    }
	  }else{				/* �V�[�N�����h���C�u���� */
	    fdc_cancel_interrupt();
	    fdc.seek_wait[ fdc.sense_us ] = -1;
	    switch( fdc.step ){
	    case  1:   fdc.read = fdc.st0_seek[ fdc.sense_us ];	break;
	    case  3:   fdc.read = fdc.pcn[ fdc.sense_us ];	break;
	    }
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 2:	case 4:	/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==5 ) REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 5:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.read = 0xff;
	  if( fdc.sense_us < MAX_DRIVE )
	    fdc.status &= ~(1<<fdc.sense_us);
	  fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;



    case SENSE_DEVICE_STATUS:	/* ------------ SENSE_DEVICE_STATUS ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  }
	  if( fdc.step < 1 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = R_PHASE;
	    fdc.step   = 0;
	    logfdc("SENSE DEV %d\n",fdc.us);
	    ICOUNT( RD );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;

      case R_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  if( fdc.us >= NR_DRIVE ){

	    fdc.read = ST3_FT | ((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );

	  }else{
	    if( disk_not_exist( fdc.us ) ||
		(disk_ex_drv & (1 << fdc.us)) ){ /* �f�B�X�N����ւ��`�F�b�N */
							    /* thanks peach! */
	      disk_ex_drv ^= (1 << fdc.us); /* �h���C�u�ԍ��̃r�b�g���] */
	      fdc.read =((fdc.status & (1<<fdc.us)) ? 0 : ST3_RY ) |
			((fdc.pcn[fdc.us]==0) ? ST3_T0 : 0 ) |
			((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );

	    }else{

	      fdc.read =((fdc.status & (1<<fdc.us)) ? 0 : ST3_RY ) |
	      		((drive[fdc.us].protect==DISK_PROTECT_TRUE)?ST3_WP:0) |
			((fdc.pcn[fdc.us]==0) ? ST3_T0 : 0 ) | ST3_TS |
			((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );

	    }
	  }
	  fdc.step ++;
	  ICOUNT( WR );
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.read = 0xff;
	  fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;



    case SPECIFY:	/* -------------------------------- SPECIFY ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd    = fdc.write & 0x1f;		break;
	  case 1:   fdc.srt_ms = 32 - ((fdc.write>>4)&0x0f)*2;
		    fdc.hut    = fdc.write & 0x0f;		break;
	  case 2:   fdc.hlt_nd = fdc.write;			break;
	  }
	  if( fdc.step < 2 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	    fdc.command = WAIT;
	    fdc.step = 0;
	    logfdc("SPECIFY\n");
	    ICOUNT( -1 );
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;
      }
      break;



    case INVALID:	/* -------------------------------- Invalid ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  }
	  {
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = R_PHASE;
	    fdc.step   = 0;
	    ICOUNT( RD );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;

      case R_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  {
	    fdc.read = ST0_IC_IC;
	  }
	  fdc.step ++;
	  ICOUNT( WR );
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( !( fdc.status & REQ_MASTER ) ){		/* �ǂ܂ꂽ */
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    REPEAT();
	  }else{					/* �܂��ǂ܂�Ȃ� */
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.read = 0xff;
	  fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;


    default:		/* --------------------------------------------	*/
      printf("FDC COM=%d\n",fdc.command );
      fflush(0);
      emu_mode = MONITOR;
      break;

    }
  }




  if( ! fdc_wait ){			/* �E�G�C�g�Ȃ��̏ꍇ */
    						/* �|�[�g�A�N�Z�X���N���閘*/
    w = 999999;					/* �i�v�ɑ҂B		   */

  }else{				/* �E�G�C�g����̏ꍇ */
    if( fdc.wait==-1 ) w = 9999999;		/* wait==-1�Ȃ�A�i�v�ɑ҂�  */
    else               w = fdc.wait;		/* wait>=0�Ȃ�A��莞�ԑ҂� */
  }

  if( fdc.command == WAIT ){		/* �ҋ@���̂݁A�V�[�N�������Ԃ��m�F */
    for( i=0; i<MAX_DRIVE; i++ )
      if( fdc.seek_wait[i] && fdc.seek_wait[i] < w ) w = fdc.seek_wait[i];
  }
  return w;					
}




/************************************************************************/
/* �h���C�u�̏�Ԃ�Ԃ��֐�						*/
/************************************************************************/
int	get_drive_ready( int drv )
{
  if( fdc.command==WAIT ) return 1;
  else{
    if( fdc.us==drv ) return 0;
    else              return 1;
  }
}







/************************************************************************/
/* �Z�N�^�Ԃ𖄂߂�                                                     */
/************************************************************************/
/*
 * READ DIAG �̃Z�N�^�Ԃ̃f�[�^�𖄂߂鏈���́Apeach���ɂ��񋟂���܂����B
 */
#define IF_WITHIN_DATA_BUF_SIZE(s)	\
if (s >= DATA_BUF_SIZE) {		\
    printf("Buffer over flow\n");	\
    fflush(stdout);			\
    return(-1);				\
} else

#define RET_GAP_ERR(cond) if (cond != TRUE) return(-1);

/*
 * GAP3 �̌v�Z
 */
INLINE
int calc_gap3_size(int n, Uchar fdc_mf)
{
    int gap3_size;

    /* GAP3 �̃T�C�Y�͌��܂��Ă���킯�ł͂Ȃ��̂ł����܂ŕW���I�Ȓl */
    switch (n) {
    case 1:	gap3_size = 27;	break;
    case 2:	gap3_size = 42;	break;
    case 3:	gap3_size = 58;	break;
    default:	gap3_size = 58; /* ���̑��͕������ */
    }
    if (fdc_mf) gap3_size *= 2;	/* �{���x */
    return(gap3_size);
}

INLINE
int input_safely_data(int ptr, int *ad, int data, int size)
{
    if (size == 0) return(TRUE);

    if (ptr + *ad + size < DATA_BUF_SIZE) {
	memset(&data_buf[ptr + *ad], data, size);
	*ad += size;
	return(TRUE);
    } else {
	printf("Buffer overflow\n");
	fflush(stdout);
	return(FALSE);
    }
}

static int fill_sector_gap(int ptr, int drv, Uchar fdc_mf)
{
    int   sync_size, am_size;
    int   gap0_size, gap1_size, gap2_size, gap3_size, gap4_size;
    int   track_size;
    Uchar gap;
    Uchar undel;
    int   size;
    int   tmp_size;
    
    if (fdc_mf) {
	/* �{���x */
	track_size = 6250;
	gap0_size = 80;
	sync_size = 12;
	am_size   = 3 + 1;
	gap1_size = 50;
	gap2_size = 22;
	gap = 0x4e;
	/*if      (sec_buf.sec_nr <= 8)  gap4_size = 654;
	  else if (sec_buf.sec_nr <= 15) gap4_size = 400;
	  else                           gap4_size = 598;*/
    } else {
	/* �P���x */
	track_size = 3100;
	gap0_size = 40;
	sync_size = 6;
	am_size = 1;
	gap1_size = 26;
	gap2_size = 11;
	gap = 0xff;
	/*if      (sec_buf.sec_nr <= 8)  gap4_size = 311;
	  else if (sec_buf.sec_nr <= 15) gap4_size = 170;
	  else                           gap4_size = 247;*/
    }
    gap3_size = calc_gap3_size(sec_buf.n, fdc_mf);

    size = 0;
    /* DATA �t�B�[���h�̍Ō�� */
    size += 2;			/* DATA CRC */

    /* GAP3 */
    RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap3_size));

    /* �ŏI�Z�N�^�Ȃ�v���A���u�� + �|�X�g�A���u�� */
    if (drive[drv].sec + 1 >= drive[drv].sec_nr) {
	/* �|�X�g�A���u�� (GAP4) �̃T�C�Y�𒲂ׂ� */
	tmp_size = gap0_size + sync_size + am_size + gap1_size;
	/* �S�ẴZ�N�^�� GAP ���J�E���g */
	do {
	    disk_next_sec(drv);
	    tmp_size += sync_size + am_size + 4 + 2; /* ID �t�B�[���h */
	    tmp_size += gap2_size;
	    tmp_size += sync_size + am_size + sec_buf.size + 2;
	    tmp_size += calc_gap3_size(sec_buf.n, fdc_mf);
	} while (drive[drv].sec + 1 < drive[drv].sec_nr);
	gap4_size = track_size - tmp_size;
	if (gap4_size < 0) {
	    printf("Abnormal sector\n");
	    return(-1);
	}

	
	RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap4_size));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0xc2, am_size - 1));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0xfc, 1));
	RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap1_size));
    }
  
    /* ���̃Z�N�^ */
    disk_next_sec(drv);

    /* ID �t�B�[���h */
    RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xa1, am_size - 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xfe, 1));
  
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.c, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.h, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.r, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.n, 1));
    size += 2;			/* ID CRC */

    /* GAP2 */
    RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap2_size));

    /* DATA �t�B�[���h */
    RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xa1, am_size - 1));

    if (sec_buf.deleted == DISK_DELETED_TRUE) undel = 0xf8;
    else undel = 0xfb;
    RET_GAP_ERR(input_safely_data(ptr, &size, undel, 1));
  
    return(size);
}




/****************************************************************/	
/* �T�X�y���h�^���W���[��					*/
/****************************************************************/	
static	char	filename[2][1024];

static	T_SUSPEND_W	suspend_fdc_work[]=
{
  { TYPE_STR,	&filename[0][0],		},
  { TYPE_STR,	&filename[1][0],		},
  { TYPE_CHAR,	&drive[0].selected_image,	},
  { TYPE_CHAR,	&drive[1].selected_image,	},

  { TYPE_INT,	&FDC_flag,			},
  { TYPE_INT,	&fdc_wait,			},

  { TYPE_INT,	&fdc.command,	},
  { TYPE_INT,	&fdc.phase,	},
  { TYPE_INT,	&fdc.step,	},
  { TYPE_INT,	&fdc.counter,	},
  { TYPE_INT,	&fdc.data_ptr,	},

  { TYPE_INT,	&fdc.limit,	},
  { TYPE_INT,	&fdc.wait,	},
  { TYPE_INT,	&fdc.seek_wait[0],	},
  { TYPE_INT,	&fdc.seek_wait[1],	},
  { TYPE_INT,	&fdc.seek_wait[2],	},
  { TYPE_INT,	&fdc.seek_wait[3],	},

  { TYPE_BYTE,	&fdc.status,	},
  { TYPE_BYTE,	&fdc.read,	},
  { TYPE_BYTE,	&fdc.write,	},
  { TYPE_BYTE,	&fdc.TC,	},

  { TYPE_CHAR,	&fdc.cmd,	},
  { TYPE_CHAR,	&fdc.sk,	},
  { TYPE_CHAR,	&fdc.mf,	},
  { TYPE_CHAR,	&fdc.mt,	},
  { TYPE_CHAR,	&fdc.us,	},
  { TYPE_CHAR,	&fdc.hd,	},
  { TYPE_CHAR,	&fdc.c,		},
  { TYPE_CHAR,	&fdc.h,		},
  { TYPE_CHAR,	&fdc.r,		},
  { TYPE_CHAR,	&fdc.n,		},
  { TYPE_CHAR,	&fdc.eot,	},
  { TYPE_CHAR,	&fdc.gpl,	},
  { TYPE_CHAR,	&fdc.dtl,	},
  { TYPE_CHAR,	&fdc.d,		},
  { TYPE_CHAR,	&fdc.sc,	},
  { TYPE_CHAR,	&fdc.stp,	},
  { TYPE_CHAR,	&fdc.srt_ms,	},
  { TYPE_CHAR,	&fdc.hut,	},
  { TYPE_CHAR,	&fdc.hlt_nd,	},
  { TYPE_CHAR,	&fdc.ncn[0],	},
  { TYPE_CHAR,	&fdc.ncn[1],	},
  { TYPE_CHAR,	&fdc.ncn[2],	},
  { TYPE_CHAR,	&fdc.ncn[3],	},
  { TYPE_CHAR,	&fdc.pcn[0],	},
  { TYPE_CHAR,	&fdc.pcn[1],	},
  { TYPE_CHAR,	&fdc.pcn[2],	},
  { TYPE_CHAR,	&fdc.pcn[3],	},
  { TYPE_CHAR,	&fdc.st0,	},
  { TYPE_CHAR,	&fdc.st1,	},
  { TYPE_CHAR,	&fdc.st2,	},
  { TYPE_CHAR,	&fdc.st3,	},
  { TYPE_CHAR,	&fdc.st0_seek[0],	},
  { TYPE_CHAR,	&fdc.st0_seek[1],	},
  { TYPE_CHAR,	&fdc.st0_seek[2],	},
  { TYPE_CHAR,	&fdc.st0_seek[3],	},
  { TYPE_CHAR,	&fdc.sense_us,		},
  { TYPE_CHAR,	&fdc.skip_ddam_job,	},
};


int	suspend_fdc( FILE *fp, long offset, long offset2 )
{
  int	ret;

  if( drive[0].filename ) strncpy( filename[0], drive[0].filename, 1024 );
  else                             filename[0][0] = '\0';
  if( drive[1].filename ) strncpy( filename[1], drive[1].filename, 1024 );
  else                             filename[1][0] = '\0';

  ret = suspend_work( fp, offset, 
		      suspend_fdc_work, 
		      countof(suspend_fdc_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->fdc ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  if( ex_fseek( fp, offset2, SEEK_SET ) == 0 ){
    if(ex_fwrite( data_buf, sizeof(Uchar), 0x4000, fp ) != 0x4000 ) return FALSE;
  }else{
    return FALSE;
  }

  return TRUE;
}


int	resume_fdc(  FILE *fp, long offset, long offset2 )
{
  int	ret = resume_work( fp, offset, 
			   suspend_fdc_work, 
			   countof(suspend_fdc_work) );

  if( ret<0 ) return FALSE;

  if( ex_fseek( fp, offset2, SEEK_SET ) == 0 ){
    if(ex_fread( data_buf, sizeof(Uchar), 0x4000, fp ) != 0x4000 ) return FALSE;
  }else{
    return FALSE;
  }
  return TRUE;
}



void	fdc_init_at_resume( void )
{
  int drv;

  for( drv=0; drv<2; drv ++ ){

    if( filename[ drv ][0] != '\0' ){

      if( verbose_proc )
	printf( "DRIVE %d: [ %s ] image no.( %d ) ... ", 
		drv+1, filename[drv], drive[drv].selected_image+1 );

      if( disk_insert( drv, filename[drv], drive[drv].selected_image ) == 0 ){
	if( verbose_proc )
	  printf( "OK\n" );
      }
    }
  }
}


















/* �f�o�b�O�p�̊֐� */
void monitor_fdc(void)
{
  printf("com = %d phs = %d  step = %d\n",fdc.command,fdc.phase,fdc.step);
  printf("FDC flag = %d\n",FDC_flag);
#if 0
  printf("\n");
  printf("fp       = %d , %d\n", drive[0].fp,       drive[1].fp      );
  printf("track    = %d , %d\n", drive[0].track,    drive[1].track   );
  printf("sec_nr   = %d , %d\n", drive[0].sec_nr,   drive[1].sec_nr  );
  printf("sec      = %d , %d\n", drive[0].sec,      drive[1].sec       );
  printf("sec_pos  = %d , %d\n", drive[0].sec_pos,  drive[1].sec_pos   );
  printf("track_top= %d , %d\n", drive[0].track_top,drive[1].track_top );
  printf("disk_top = %d , %d\n", drive[0].disk_top, drive[1].sec     );
  printf("type     = %d , %d\n", drive[0].type,     drive[1].type );
  printf("protect  = %d , %d\n", drive[0].protect,  drive[1].protect     );

  printf("\n");
  {
    int	i,j,k;
    for(k=0;k<(128<<(fdc.n&7))/256;k++){
      for(i=0;i<16;i++){
	for(j=0;j<16;j++){
	  printf("%02X ",data_buf[k*256+i*16+j]);
	}
	printf("\n");
      }
    }
  }
  printf("\n");
#endif
}
