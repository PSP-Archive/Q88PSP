#ifndef QUASI88_H_INCLUDED
#define QUASI88_H_INCLUDED


/*----------------------------------------------------------------------*/
/* SDL�Ɋւ����`							*/
/*----------------------------------------------------------------------*/

#ifdef HAVE_SDL
#include <SDL_main.h>
#include <SDL_byteorder.h>


/* �G���f�B�A���l�X���`�F�b�N */
#if	( SDL_BYTEORDER == SDL_LIL_ENDIAN )
#define	LSB_FIRST
#endif

/* select()���g���킯�ł͂Ȃ����Ams�P�ʂ� sleep ���\�Ȃ̂Œ�`���Ƃ��� */
#define	HAVE_SELECT

/* X11-DGA ���g���킯�ł͂Ȃ����A�t���X�N���[���ؑւ��\�Ȃ̂Œ�`���Ƃ��� */
#define	USE_DGA

/* SDL�� QUASI88 �̂��߂̎��ʗp */

#define	USE_SDL

#endif


/* �^�C�g���ƃo�[�W���� */
#ifndef	Q_TITLE
#define	Q_TITLE		"QUASI88(SDL)"
#endif
#ifndef	Q_VERSION
#define	Q_VERSION	"0.5.5s1"
#endif

/* PI(��) �� M_PI(��) ���` */
#ifndef	UNIX88
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif
#endif

/* �W���C�X�e�B�b�N���� */
#define	USE_JOY
#define	USE_JOY_SDL

/*
  MAME/XMAME �̃T�E���h��g�ݍ��ޏꍇ�A
	USE_SOUND
  ���`���Ă����B

  FMGEN ��g�ݍ��ޏꍇ�́A
	USE_FMGEN
  ����`���Ă����B

  ��L�́A�R���p�C�����Ɉȉ��̂悤�ɂ��Ē�`����B
  gcc  �̏ꍇ�A�R���p�C���ɃI�v�V���� -DUSE_SOUND   �ȂǂƎw�肷��B
  VC++ �̏ꍇ�A�R���p�C���ɃI�v�V���� /D"USE_SOUND" �ȂǂƎw�肷��B
  MWP  �̏ꍇ�A�R���p�C���ɃI�v�V���� -d USE_SOUND  �ȂǂƎw�肷��B
*/




/*
 *	VC++ depend
 */

#ifdef _MSC_VER

/* VC �̃C�����C���L�[���[�h */
#define	INLINE		__inline

#endif

#if	(_MSC_VER) || (__MINGW32__)

/* Windows�� QUASI88 �̂��߂̎��ʗp */
#define	WIN88

/* �p�X�E�t�@�C�����̍ő咷�� ? */
#define	OSD_MAX_FILENAME	(240)

#endif

#define EXPRINTF(format,...) \
{ printf(format,## __VA_ARGS__); } 

/*
 *
 */

#ifndef	BIT32
#define	BIT32		int
#endif


/*----------------------------------------------------------------------*/
/* ���ʒ�`								*/
/*----------------------------------------------------------------------*/

typedef	unsigned char	Uchar;
typedef	unsigned short	Ushort;
typedef	unsigned int	Uint;
typedef	unsigned long	Ulong;

typedef unsigned char  byte;
typedef unsigned short word;
typedef signed   char  offset;


typedef unsigned char  bit8;
typedef unsigned short bit16;
typedef unsigned BIT32 bit32;


#define	countof(arr)	(int)(sizeof(arr)/sizeof((arr)[0]))


#ifdef LSB_FIRST			/* ���g���G���f�B�A�� */

typedef union
{
  struct { byte l,h; }	B;
  word			W;
} pair;

#else					/* �r�b�O�G���f�C�A�� */

typedef union
{
  struct { byte h,l; }	B;
  word			W;
} pair;

#endif



#ifndef TRUE
#define	TRUE	(1)
#endif
#ifndef FALSE
#define	FALSE	(0)
#endif



#ifndef	INLINE
#define	INLINE	static
#endif



/*----------------------------------------------------------------------*/
/* �ϐ� (verbose_*)�A�֐�						*/
/*----------------------------------------------------------------------*/
extern	int	verbose_level;		/* �璷���x��			*/
extern	int	verbose_proc;		/* �����̐i�s�󋵂̕\��		*/
extern	int	verbose_z80;		/* Z80�����G���[��\��		*/
extern	int	verbose_io;		/* ������ I/O�A�N�Z�X���	*/
extern	int	verbose_pio;		/* PIO �̕s���g�p��\��		*/
extern	int	verbose_fdc;		/* FD�C���[�W�ُ���		*/
extern	int	verbose_wait;		/* �E�G�C�g�҂����ُ̈���	*/
extern	int	verbose_suspend;	/* �T�X�y���h���ُ̈���	*/
extern	int	verbose_snd;		/* �T�E���h�̃��b�Z�[�W		*/


void	main_exit( int val );
void	main_reset( void );



/*----------------------------------------------------------------------*/
/*	�f�o�b�O�p�ɉ�ʂ�t�@�C���Ƀ��O���o�� (�������x���ቺ���܂�)	*/
/*----------------------------------------------------------------------*/

/*#define PIO_DISP*/		/* PIO �֌W�̃��O����ʂɕ\��     */
/*#define PIO_FILE*/		/*		   �t�@�C���ɏo�� */

/*#define FDC_DISP*/		/* FDC �֌W�̃��O����ʂɕ\��     */
/*#define FDC_FILE*/		/*		   �t�@�C���ɏo�� */

/*#define MAIN_DISP*/		/* ���C�� Z80 �֌W�̃��O����ʂɕ\�� */
/*#define MAIN_FILE*/		/*		   �t�@�C���ɏo��    */

/*#define SUB_DISP*/		/* �T�u Z80 �֌W�̃��O����ʂɕ\�� */
/*#define SUB_FILE*/		/*		   �t�@�C���ɏo��  */


#ifndef	USE_MONITOR
#undef PIO_DISP
#undef PIO_FILE
#undef FDC_DISP
#undef FDC_FILE
#undef MAIN_DISP
#undef MAIN_FILE
#undef SUB_DISP
#undef SUB_FILE
#endif


/* ���O���t�@�C���Ɏ��ꍇ�́A�t�@�C�����J�� */
#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
#include <stdio.h>
extern	FILE	*LOG;
#endif


/* ���O�o�͂̃}�N��							*/
/*	�E�ϒ������̊֐����Ăяo���}�N���ł���B			*/
/*	�E�}�N�����̂��̂𖳌��ɂ���ɂ́A��ʂɂ�			*/
/*		#define  logxxx   (void)				*/
/*		#define  logxxx   if(1){}else printf			*/
/*	�Ȃǂ��g���B�O�҂ł̓��[�j���O���o�鎞������̂Ō�҂��̗p�����B*/


#if	defined( PIO_DISP ) || defined( PIO_FILE )
void	logpio( const char *format, ... );
#else
#define	logpio	if(1){}else printf
#endif

#if	defined( FDC_DISP ) || defined( FDC_FILE )
void	logfdc( const char *format, ... );
#else
#define	logfdc	if(1){}else printf
#endif

#if	defined( MAIN_DISP ) || defined( MAIN_FILE ) || defined( SUB_DISP ) || defined( SUB_FILE )
void	logz80( const char *format, ... );
#else
#define	logz80	if(1){}else printf
#endif



#endif		/* QUASI88_H_INCLUDED */
