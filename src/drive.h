#ifndef DRIVE_H_INCLUDED
#define DRIVE_H_INCLUDED

#include <stdio.h>

#include "initval.h"
/*	�h���C�u�̐� NR_DRIVE �́Ainitval.h �Œ�`�� */
/*	�C���[�W�̐� NR_IMAGE �́Ainitval.h �Œ�`�� */




	/**** �h���C�u���䃏�[�N ****/

typedef	struct{

  FILE	*fp;			/* FILE �|�C���^                    : G	*/

  long	file_size;		/* �t�@�C���̃T�C�Y                 : G	*/
  char	read_only;		/* �t�@�C������(ro,rw)              : G	*/

  char	over_image;		/* �C���[�W�������߂��鎞�ɐ^       : G	*/
  char	detect_broken_image;	/* �N�����ɉ�ꂽ�C���[�W������Ɛ^ : G	*/
  Uchar	image_nr;		/* �t�@�C�����̃C���[�W��           : G	*/

  struct{			/* �t�@�C�����̃C���[�W���         : G	*/
    char	name[17];	/*	�C���[�W��                  : G	*/
    char	protect;	/*	�v���e�N�g                  : G	*/
    char	type;		/*	�^�C�v                      : G	*/
    long	size;		/*	�T�C�Y                      : G	*/
  }image[ MAX_NR_IMAGE ];

				/* --- �ȏ�A�t�@�C�����ʂ̏��	*/

  int	track;			/* ���݂̃g���b�N�ԍ�               : L	*/
  int	sec_nr;			/* �g���b�N���̃Z�N�^��             : L	*/
  int	sec;			/* ���݂̃Z�N�^�ԍ�                 : L	*/

  long	sec_pos;		/* �Z�N�^  �̌��݈ʒu               : L	*/
  long	track_top;		/* �g���b�N�̐擪�ʒu               : L	*/
  long	disk_top;		/* �f�B�X�N�̐擪�ʒu               : L	*/
  long	disk_end;		/* �C���[�W�̏I�[�ʒu               : L	*/

  char	protect;		/* ���C�g�v���e�N�g                 : L	*/
  char	type;			/* �f�B�X�N�̎��                   : L	*/
  char	selected_image;		/* �C���[�W�ԍ�                     : L	*/
  char	empty;			/* ��w��̎��A�^                   : L	*/

  char	*filename;		/* �t�@�C�����ւ̃|�C���^	    : L	*/

} PC88_DRIVE_T;



extern	PC88_DRIVE_T	drive[ NR_DRIVE ];




	/**** �f�B�X�N��� �I�t�Z�b�g ****/
#define	DISK_FILENAME	(0)		/* char x [17]	*/
#define	DISK_PROTECT	(26)		/* char		*/
#define	DISK_TYPE	(27)		/* char		*/
#define	DISK_SIZE	(28)		/* long		*/
#define	DISK_TRACK	(32)		/* long x [164]	*/

	/**** ID��� �I�t�Z�b�g ****/
#define	DISK_C		(0)		/* char		*/
#define	DISK_H		(1)		/* char		*/
#define	DISK_R		(2)		/* char		*/
#define	DISK_N		(3)		/* char		*/
#define	DISK_SEC_NR	(4)		/* short	*/
#define	DISK_DENSITY	(6)		/* char		*/
#define	DISK_DELETED	(7)		/* char		*/
#define	DISK_STATUS	(8)		/* char		*/
#define	DISK_RESERVED	(9)		/* char	x [5]	*/
#define	DISK_SEC_SZ	(14)		/* char	x [???]	*/

#define	SZ_DISK_ID	(16)		/* ID��� 16Byte*/


	/**** �f�B�X�N/ID��� �萔 ****/
#define DISK_PROTECT_TRUE	(0x10)
#define	DISK_PROTECT_FALSE	(0x00)

#define	DISK_TYPE_2D		(0x00)
#define	DISK_TYPE_2DD		(0x10)
#define	DISK_TYPE_2HD		(0x20)

#define	DISK_DENSITY_DOUBLE	(0x00)
#define	DISK_DENSITY_SINGLE	(0x40)

#define	DISK_DELETED_TRUE	(0x10)
#define	DISK_DELETED_FALSE	(0x00)



	/**** �֐� ****/


void	drive_init( void );
void	drive_reset( void );
int	disk_insert( int drv, char *filename, int img );
int	disk_change_image( int drv, int img );
void	disk_eject( int drv );

void	drive_set_empty( int drv );
void	drive_unset_empty( int drv );
void	drive_change_empty( int drv );
int	drive_check_empty( int drv );

int	get_drive_ready( int drv );




#define	disk_image_exist( drv )		(drive[ drv ].fp)
#define	disk_image_num_of_drive( drv )	(drive[ drv ].image_nr)
#define	disk_image_now_selected( drv )	(drive[ drv ].selected_image)


#endif	/* DRIVE_H_INCLUDED */
