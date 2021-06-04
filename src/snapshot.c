/************************************************************************/
/*									*/
/* �X�N���[�� �X�i�b�v�V���b�g						*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "screen.h"
#include "graph.h"

#include "crtcdmac.h"
#include "memory.h"
#include "file-op.h"
#include "snapshot.h"

#include "exmem.h"

/* �f�t�H���g�́A�X�N���[�� �X�i�b�v�V���b�g�̃t�@�C���x�[�X�� */

#define	SNAPSHOT_FILENAME	"save"


char	*file_snapshot   = NULL;	/* �X�i�b�v�V���b�g�t�@�C����	*/
int	snapshot_format  = 0;		/* �X�i�b�v�V���b�g�t�H�[�}�b�g	*/
char	*snapshot_cmd    = NULL;	/* �X�i�b�v�V���b�g��R�}���h	*/
char	snapshot_cmd_do  = FALSE;	/* �R�}���h���s�̗L��		*/



void	screen_snapshot_init( void )
{
  int dummy;

  snapshot_format = 0;					/* ppm(raw) �`�� */

  file_snapshot = getenv( "QUASI88_SSS_BASE" );		/* �t�@�C����   */

  if( file_snapshot == NULL ){

    file_snapshot = osd_get_home_filename( SNAPSHOT_FILENAME, &dummy );

    if( file_snapshot == NULL ){
      file_snapshot = "";
    }

    /* �ǂ����� ex_free(file_snapshot)�����ق����������ǁc�܂������� */
  }


  snapshot_cmd = getenv("QUASI88_SSS_CMD");		/* ���s�R�}���h */

  if( snapshot_cmd == NULL ){
    snapshot_cmd = "";
  }

  snapshot_cmd_do = FALSE;	/* �����l�́A�R�}���h���s�w���Ȃ��x�ɂ��� */
}





static char		screen_snapshot[ 640*400 ];
static int		line400_snapshot;
static SYSTEM_PALETTE_T	palette_snapshot[16];



#define	COLOR
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( color, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( color, 40 )
#undef	WIDTH40
#undef	COLOR

#define	MONO
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( mono, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( mono, 40 )
#undef	WIDTH40
#undef	MONO

#define	HIRESO
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( hireso, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( hireso, 40 )
#undef	WIDTH40
#undef	HIRESO

#define	CLEAR
#define	WIDTH80
#include "snapshot-base.h"
INLINE	vram2snapshot( clear, 80 )
#undef	WIDTH80
#define	WIDTH40
#include "snapshot-base.h"
INLINE	vram2snapshot( clear, 40 )
#undef	WIDTH40
#undef	CLEAR



/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

static	void	make_snapshot( void )
{
  int i;

  line400_snapshot = FALSE;

	/* VRAM �̓��e�� screen_snapshot[] �ɓ]�� */

  if( grph_ctrl & GRPH_CTRL_VDISP ){		/* VRAM �\������ */

    if( grph_ctrl & GRPH_CTRL_COLOR ){		    /* �J���[ */

      if( sys_ctrl & SYS_CTRL_80 ){
	vram2snapshot_color_width80();
      }else{
	vram2snapshot_color_width40();
      }

    }else{

      if( grph_ctrl & GRPH_CTRL_200 ){		    /* ���� */

	if( sys_ctrl & SYS_CTRL_80 ){
	  vram2snapshot_mono_width80();
	}else{
	  vram2snapshot_mono_width40();
	}

      }else{					    /* 400���C�� */
	if( sys_ctrl & SYS_CTRL_80 ){
	  vram2snapshot_hireso_width80();
	}else{
	  vram2snapshot_hireso_width40();
	}
	line400_snapshot = TRUE;
      }
    }

  }else{					/* VRAM �\�����Ȃ� */

    screen_update_hide = TRUE;				/* ��\���σt���OON */

    if( sys_ctrl & SYS_CTRL_80 ){
      vram2snapshot_clear_width80();
    }else{
      vram2snapshot_clear_width40();
    }

  }




	/* VRAM �� �J���[�p���b�g   palette_snapshot[0]�`[7] */

  if( grph_ctrl & GRPH_CTRL_COLOR ){		/* VRAM �J���[ */

    if( monitor_analog ){
#if	0
      for( i=0; i<8; i++ ){
	palette_snapshot[i].red   = vram_palette[i].red;
	palette_snapshot[i].green = vram_palette[i].green;
	palette_snapshot[i].blue  = vram_palette[i].blue;
      }
#else
      for( i=0; i<8; i++ ){
	palette_snapshot[i].red   = vram_palette[i].red   * 73 >> 6;
	palette_snapshot[i].green = vram_palette[i].green * 73 >> 6;
	palette_snapshot[i].blue  = vram_palette[i].blue  * 73 >> 6;
      }
#endif
    }else{
      for( i=0; i<8; i++ ){
	palette_snapshot[i].red   = vram_palette[i].red   ? 0xff : 0;
	palette_snapshot[i].green = vram_palette[i].green ? 0xff : 0;
	palette_snapshot[i].blue  = vram_palette[i].blue  ? 0xff : 0;
      }
    }

  }else{					/* VRAM ���� */

    if( monitor_analog ){
#if	0
      palette_snapshot[0].red   = vram_bg_palette.red;
      palette_snapshot[0].green = vram_bg_palette.green;
      palette_snapshot[0].blue  = vram_bg_palette.blue;
#else
      palette_snapshot[0].red   = vram_bg_palette.red   * 73 >> 6;
      palette_snapshot[0].green = vram_bg_palette.green * 73 >> 6;
      palette_snapshot[0].blue  = vram_bg_palette.blue  * 73 >> 6;
#endif
    }else{
      palette_snapshot[0].red   = vram_bg_palette.red   ? 0xff : 0;
      palette_snapshot[0].green = vram_bg_palette.green ? 0xff : 0;
      palette_snapshot[0].blue  = vram_bg_palette.blue  ? 0xff : 0;
    }
    for( i=1; i<8; i++ ){
      palette_snapshot[i].red   = 0;
      palette_snapshot[i].green = 0;
      palette_snapshot[i].blue  = 0;
    }

  }


	/* TEXT �� �J���[�p���b�g   palette_snapshot[8]�`[15] */

  if( !crtc_attr_color ){			/* TEXT ���� */

    palette_snapshot[8].red   = 0;
    palette_snapshot[8].green = 0;
    palette_snapshot[8].blue  = 0;
    for( i=9; i<16; i++ ){
      palette_snapshot[i].red   = 0xff;
      palette_snapshot[i].green = 0xff;
      palette_snapshot[i].blue  = 0xff;
    }

  }else{					/* TEXT �J���[ */

    if( grph_ctrl & GRPH_CTRL_COLOR ){			/* VRAM �J���[ */

      for( i=8; i<16; i++ ){
	palette_snapshot[i].red   = (i&0x02) ? 0xff : 0;
	palette_snapshot[i].green = (i&0x04) ? 0xff : 0;
	palette_snapshot[i].blue  = (i&0x01) ? 0xff : 0;
      }

    }else{						/* VRAM ����   */

      if( monitor_analog ){
#if	0
	for( i=8; i<16; i++ ){
	  palette_snapshot[i].red   = vram_palette[i&0x7].red;
	  palette_snapshot[i].green = vram_palette[i&0x7].green;
	  palette_snapshot[i].blue  = vram_palette[i&0x7].blue;
	}
#else
	for( i=8; i<16; i++ ){
	  palette_snapshot[i].red   = vram_palette[i&0x7].red   * 73 >> 6;
	  palette_snapshot[i].green = vram_palette[i&0x7].green * 73 >> 6;
	  palette_snapshot[i].blue  = vram_palette[i&0x7].blue  * 73 >> 6;
	}
#endif
      }else{
	for( i=8; i<16; i++ ){
	  palette_snapshot[i].red   = vram_palette[i&0x7].red   ? 0xff : 0;
	  palette_snapshot[i].green = vram_palette[i&0x7].green ? 0xff : 0;
	  palette_snapshot[i].blue  = vram_palette[i&0x7].blue  ? 0xff : 0;
	}
      }

    }
  }


  /* line400==FALSE �̎��A[0]�`[640*200-1] ���摜�f�[�^ 	*/
  /* line400==TRUE  �̎��A[0]�`[640*400-1] ���摜�f�[�^ 	*/
  /* �f�[�^�́A0�`15 �̒l�ŁARGB�l�� palette_snapshot[0]�`[15]	*/



}


/*----------------------------------------------------------------------*/
/* �L���v�`���������e���Axpm �`���Ńt�@�C���ɏo��			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_xpm( FILE *fp )
{
  int i, j;
  char *p = screen_snapshot;
  int black_pal = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;

  if( fp==NULL ) return 0;

  fprintf( fp, "/* XPM */\n" ); 
  fprintf( fp, "static char * quasi88_xpm[] = {\n" );
  fprintf( fp, "\"640 400 16 1\",\n" );
  
  for( i=0; i<16; i++ )
    fprintf( fp, "\"%1X      c #%04X%04X%04X\",\n",
	     i,
	     (unsigned short)palette_snapshot[i].red   << 8,
	     (unsigned short)palette_snapshot[i].green << 8,
	     (unsigned short)palette_snapshot[i].blue  << 8 );

  for( i=0; i<200; i++ ){

    fprintf( fp, "\"");
    for( j=0; j<640; j++ ){
      fprintf( fp, "%1X", *p++ );
    }
    fprintf( fp, "\",\n");


#if	0
    if( line400_snapshot==FALSE && use_interlace==FALSE ) p -= 640;

    if( line400_snapshot || use_interlace==FALSE ){
      fprintf( fp, "\"");
      for( j=0; j<640; j++ ){
	fprintf( fp, "%1X", *p++ );
      }
      fprintf( fp, "\",\n");
    }else{
      fprintf( fp, "\"");
      for( j=0; j<640; j++ ){
	fprintf( fp, "%1X", black_pal );
      }
      fprintf( fp, "\",\n");
    }
#else
    fprintf( fp, "\"");
    for( j=0; j<640; j++ ){
      fprintf( fp, "%1X", *p++ );
    }
    fprintf( fp, "\",\n");
#endif

  }

  fprintf( fp, "};\n" );

  return 1;
}



/*----------------------------------------------------------------------*/
/* �L���v�`���������e���Appm �`��(raw)�Ńt�@�C���ɏo��			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm_raw( FILE *fp )
{
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  fprintf( fp, "P6\n" );
  fprintf( fp, "# QUASI88\n" );
  fprintf( fp, "640 400\n" );
  fprintf( fp, "255\n" );
  
  for( i=0; i<200; i++ ){

    for( j=0; j<640; j++ ){
      fprintf( fp, "%c%c%c",
	       palette_snapshot[ *p ].red,
	       palette_snapshot[ *p ].green,
	       palette_snapshot[ *p ].blue );
      p++;
    }

#if	0
    if( line400_snapshot==FALSE && use_interlace==FALSE ) p -= 640;

    if( line400_snapshot || use_interlace==FALSE ){
      for( j=0; j<640; j++ ){
	fprintf( fp, "%c%c%c",
		 palette_snapshot[ *p ].red,
		 palette_snapshot[ *p ].green,
		 palette_snapshot[ *p ].blue );
	p++;
      }
    }else{
      for( j=0; j<640; j++ ){
	fprintf( fp, "%c%c%c", 0, 0, 0 );
      }
    }
#else
    for( j=0; j<640; j++ ){
      fprintf( fp, "%c%c%c",
       palette_snapshot[ *p ].red,
       palette_snapshot[ *p ].green,
       palette_snapshot[ *p ].blue );
      p++;
    }
#endif

  }

  return 1;
}

/*----------------------------------------------------------------------*/
/* �L���v�`���������e���Appm �`��(ascii)�Ńt�@�C���ɏo��		*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm_ascii( FILE *fp )
{
  int i, j, k;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  fprintf( fp, "P3\n" );
  fprintf( fp, "# QUASI88\n" );
  fprintf( fp, "640 400\n" );
  fprintf( fp, "255\n" );
  
  for( i=0; i<200; i++ ){

    for( j=0; j<640; j+=5 ){
      for( k=0; k<5; k++ ){
	fprintf( fp, "%3d %3d %3d ", 
		 palette_snapshot[ *p ].red,
		 palette_snapshot[ *p ].green,
		 palette_snapshot[ *p ].blue );
	p++;
      }
      fprintf( fp, "\n");
    }

#if	0
    if( line400_snapshot==FALSE && use_interlace==FALSE ) p -= 640;

    if( line400_snapshot || use_interlace==FALSE ){
      for( j=0; j<640; j+=5 ){
	for( k=0; k<5; k++ ){
	  fprintf( fp, "%3d %3d %3d ", 
		   palette_snapshot[ *p ].red,
		   palette_snapshot[ *p ].green,
		   palette_snapshot[ *p ].blue );
	  p++;
	}
	fprintf( fp, "\n");
      }
    }else{
      for( j=0; j<640; j+=5 ){
	for( k=0; k<5; k++ ){
	  fprintf( fp, "%3d %3d %3d ", 0, 0, 0 );
	}
	fprintf( fp, "\n");
      }
    }
#else
    for( j=0; j<640; j+=5 ){
      for( k=0; k<5; k++ ){
	fprintf( fp, "%3d %3d %3d ",
         palette_snapshot[ *p ].red,
         palette_snapshot[ *p ].green,
         palette_snapshot[ *p ].blue );
	p++;
      }
      fprintf( fp, "\n");
    }
#endif

  }

  return 1;
}





/************************************************************************/
/* ��ʂ̃X�i�b�v�V���b�g���Z�[�u����					*/
/*	�����_��VRAM�����ƂɃX�i�b�v�V���b�g���쐬����B		*/
/*	���ݕ\������Ă����ʂ��Z�[�u����킯�ł͂Ȃ��B		*/
/*									*/
/*	���ϐ� ${QUASI88_SSS_CMD} ����`����Ă���ꍇ�A�Z�[�u���	*/
/*	���̓��e���R�}���h�Ƃ��Ď��s����B���̍ہA%a ���t�@�C�����ɁA	*/
/*	%b ���t�@�C��������T�t�B�b�N�X���폜�������̂ɁA�u�������B	*/
/*									*/
/*	��) setenv QUASI88_SSS_CMD 'ppmtopng %a > %b.png'		*/
/*									*/
/************************************************************************/


static	int snapshot_no = 0;

int	save_screen_snapshot( void )
{
  int  result;
  FILE *fp;

  char   *filename, *cmd, *s, *d;
  char   number[8];
  static const char *suffix[] = { ".ppm", ".ppm", ".xpm", };
  static const char *checksuffix[] = { ".ppm", 
				       ".xpm", 
				       ".png", 
				       ".bmp", 
				       ".rgb", 
				       ".gif", 
				       ".xwd", 
				       ".pict", 
				       ".tiff", 
				       ".tif", 
				       ".jpeg", 
				       ".jpg", 
  };
  int    i, len, a_len, b_len;


  if( file_snapshot   ==NULL ) return 0;
  if( file_snapshot[0]=='\0' ) return 0;

  if( snapshot_format >= countof(suffix) ) return 0;


  filename = (char*)ex_malloc( strlen( file_snapshot ) + sizeof( "NNNN.ppm" ) );
  if( filename==NULL ) return 0;


  for( result=10000; result; result-- ){    /* ���݂��Ȃ��t�@�C���������� */

    sprintf( number, "%04d", snapshot_no ++ );
    if( snapshot_no > 9999 ) snapshot_no = 0;
    strcpy( filename, file_snapshot );
    strcat( filename, number );
    len = strlen( filename );

    for( i=countof(checksuffix); i; i-- ){
      filename[len] = '\0';
      strcat( filename, checksuffix[ i-1 ] );
      if( osd_file_stat( filename )!=FILE_STAT_ANOTHER ) break;
    }
    if( i==0 ){
      filename[len] = '\0';
      strcat( filename, suffix[ snapshot_format ] );
      break;
    }
  }

  /* ���݂��Ȃ��t�@�C�������������āA�t�@�C�����J�����Ƃɐ��������� */

  if( result && (fp = ex_fopen( filename, "wb" ) ) ){

    make_snapshot();

    switch( snapshot_format ){
    case 0:	result = save_snapshot_ppm_raw( fp );	break;
    case 1:	result = save_snapshot_ppm_ascii( fp );	break;
    case 2:	result = save_snapshot_xpm( fp );	break;
    }

    ex_fclose( fp );


    /* �����������݂ɐ������A���s�R�}���h���Z�b�g����Ă���� */

#ifdef	USE_SSS_CMD
    if( result &&
	snapshot_cmd && snapshot_cmd_do ){

      a_len = strlen( filename );
      b_len = a_len - 4;		/* �T�t�B�b�N�X ".???" ��4���������Z */

      len = 0;
      s = snapshot_cmd;
      while( *s ){
	if( *s == '%' ){
	  switch( *(s+1) ){
	  case '%': len ++;		s++;	break; 
	  case 'a': len += a_len;	s++;	break; 
	  case 'b': len += b_len;	s++;	break; 
	  default:  len ++;			break; 
	  }
	}else       len ++;

	s++;
      }

      cmd = (char *)ex_malloc( len + 1 );
      if( cmd ){

	s = snapshot_cmd;
	d = cmd;
	while( *s ){
	  if( *s == '%' ){
	    switch( *(s+1) ){
	    case '%': *d++ = *s;				s++;	break; 
	    case 'a': memcpy( d, filename, a_len );  d+=a_len;	s++;	break; 
	    case 'b': memcpy( d, filename, b_len );  d+=b_len;	s++;	break;
	    default:  *d++ = *s;
	    }
	  }else       *d++ = *s;
	  s++;
	}
	*d = '\0';

	fprintf( stderr, "[SNAPSHOT command]%% %s\n",cmd);
	system( cmd );

	ex_free(cmd);
      }
    }
#endif	/* USE_SSS_CMD */

  }else{
    result = 0;
  }

  ex_free( filename );
  return result;
}
