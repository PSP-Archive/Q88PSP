#ifndef DRIVE_H_INCLUDED
#define DRIVE_H_INCLUDED

#include <stdio.h>

#include "initval.h"
/*	ドライブの数 NR_DRIVE は、initval.h で定義済 */
/*	イメージの数 NR_IMAGE は、initval.h で定義済 */




	/**** ドライブ制御ワーク ****/

typedef	struct{

  FILE	*fp;			/* FILE ポインタ                    : G	*/

  long	file_size;		/* ファイルのサイズ                 : G	*/
  char	read_only;		/* ファイル属性(ro,rw)              : G	*/

  char	over_image;		/* イメージ数が多過ぎる時に真       : G	*/
  char	detect_broken_image;	/* 起動時に壊れたイメージがあると真 : G	*/
  Uchar	image_nr;		/* ファイル内のイメージ数           : G	*/

  struct{			/* ファイル内のイメージ情報         : G	*/
    char	name[17];	/*	イメージ名                  : G	*/
    char	protect;	/*	プロテクト                  : G	*/
    char	type;		/*	タイプ                      : G	*/
    long	size;		/*	サイズ                      : G	*/
  }image[ MAX_NR_IMAGE ];

				/* --- 以上、ファイル共通の情報	*/

  int	track;			/* 現在のトラック番号               : L	*/
  int	sec_nr;			/* トラック内のセクタ数             : L	*/
  int	sec;			/* 現在のセクタ番号                 : L	*/

  long	sec_pos;		/* セクタ  の現在位置               : L	*/
  long	track_top;		/* トラックの先頭位置               : L	*/
  long	disk_top;		/* ディスクの先頭位置               : L	*/
  long	disk_end;		/* イメージの終端位置               : L	*/

  char	protect;		/* ライトプロテクト                 : L	*/
  char	type;			/* ディスクの種類                   : L	*/
  char	selected_image;		/* イメージ番号                     : L	*/
  char	empty;			/* 空指定の時、真                   : L	*/

  char	*filename;		/* ファイル名へのポインタ	    : L	*/

} PC88_DRIVE_T;



extern	PC88_DRIVE_T	drive[ NR_DRIVE ];




	/**** ディスク情報 オフセット ****/
#define	DISK_FILENAME	(0)		/* char x [17]	*/
#define	DISK_PROTECT	(26)		/* char		*/
#define	DISK_TYPE	(27)		/* char		*/
#define	DISK_SIZE	(28)		/* long		*/
#define	DISK_TRACK	(32)		/* long x [164]	*/

	/**** ID情報 オフセット ****/
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

#define	SZ_DISK_ID	(16)		/* ID情報 16Byte*/


	/**** ディスク/ID情報 定数 ****/
#define DISK_PROTECT_TRUE	(0x10)
#define	DISK_PROTECT_FALSE	(0x00)

#define	DISK_TYPE_2D		(0x00)
#define	DISK_TYPE_2DD		(0x10)
#define	DISK_TYPE_2HD		(0x20)

#define	DISK_DENSITY_DOUBLE	(0x00)
#define	DISK_DENSITY_SINGLE	(0x40)

#define	DISK_DELETED_TRUE	(0x10)
#define	DISK_DELETED_FALSE	(0x00)



	/**** 関数 ****/


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
