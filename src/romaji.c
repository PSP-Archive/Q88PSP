/************************************************************************/
/*									*/
/* ���[�}�����J�i�ϊ�����						*/
/*									*/
/*		keyboard.c �t�@�C��������Ainclude ����Ă���B		*/
/************************************************************************/


static	int	romaji_input_mode = FALSE;	/* �^�̎��A���[�}�����̓��[�h*/

static	char	input_buf[4];			/* ���͍ς݂̕����̃o�b�t�@  */
static	int	input_size;			/* ���͍ς݂̕����̐�	     */


#define	ROMAJI_QUE_SIZE	(64)
static	int	romaji_set;			/* ���[�}�����͂��ꂽ�J�i�� */
static	int	romaji_ptr;			/* �L���[�ɒ~�����A����   */
static	Uchar	romaji_que[ ROMAJI_QUE_SIZE ];	/* I/O �|�[�g�ɑ����Ă��� */


/* �L���[�ɒ~����l�B�|�[�g��񂪃p�b�N���Ă��� */
#define	RJ( port, bit, shift )	((Uchar)( (port<<4) | (shift<<3) | bit ))


static	int	press_timer;			/* �L�[�I���E�I�t�̃^�C�}�[ */
#define	KEY_ON_OFF_INTERVAL	(4)		/* �L�[�I���E�I�t�̎���	    */


/*
 *	�L���[�ɒ~����֐�
 */
static	void	set_romaji_que( const Uchar *p )
{
  Uchar c;
  while( (c = *p++ ) ){
    romaji_que[ romaji_set++ ] = c;
    romaji_set &= (ROMAJI_QUE_SIZE-1);
  }
}


/*
 *	�L���[����|�[�g�ɏo�͂��Ă����֐�
 */
static	void	romaji_out_put( void )
{
  Uint c;

  switch( press_timer ){
  case 0:
    if( romaji_ptr != romaji_set ){
      c = romaji_que[ romaji_ptr ];
      if( c & 0x08 ) key_scan[P8] &= ~Bt6;
      key_scan[ c>>4 ] &= ~(1<<(c&7));
      press_timer ++;
    }
    break;

  case KEY_ON_OFF_INTERVAL:
    c = romaji_que[ romaji_ptr++ ];
    romaji_ptr &= (ROMAJI_QUE_SIZE-1);
    if( c & 0x08 ) key_scan[P8] |=  Bt6;
    key_scan[ c>>4 ] |=  (1<<(c&7));
    press_timer ++;
    break;

  case KEY_ON_OFF_INTERVAL*2:
    press_timer = 0;
    break;

  default:
    press_timer ++;
  }
}


/*
   ���[�}�����̓��[�h�ɓ��������^�o�����̏���
*/
#define	romaji_input_init()		\
	do{				\
	  romaji_input_mode = TRUE;	\
	  romaji_clear();		\
	}while(0)

#define	romaji_input_term()		\
	do{				\
	  romaji_input_mode = FALSE;	\
	}while(0)


/*
 *	���[�}�� �� �J�i �ϊ��e�[�u��
 */
typedef struct {
  const char *s;
  Uchar      list[4];
} romaji_list;

#if 0	/* romaji-table.c ���Œ�` */
static const romaji_list list_NN;
static const romaji_list list_tu;
static const romaji_list list_mark[];
static const romaji_list list_msime[];
static const romaji_list list_atok[];
static const romaji_list list_egg[];
#endif

#include "romaji-table.c"


/*
 *	���[�}���J�i�ϊ��e�[�u���̍쐬
 */
#include <stdlib.h>

static romaji_list list[280];
static int         nr_list;

static int romajicmp( const void *p1, const void *p2 )
{
  return strcmp( ((const romaji_list *)p1)->s, ((const romaji_list *)p2)->s );
}

void	romaji_make_list( void )
{
  int i,            nr_p;
  const romaji_list *p;

  romaji_clear();
  nr_list = 0;

  if     ( romaji_type==1 ){ p = list_msime; nr_p = countof(list_msime); }
  else if( romaji_type==2 ){ p = list_atok;  nr_p = countof(list_atok);  }
  else                     { p = list_egg;   nr_p = countof(list_egg);   }
  for( i=0; i<nr_p; i++ ){
    list[ nr_list ++ ] = *p ++;
    if( nr_list >= countof(list) ) break;
  }
  

  qsort( &list, nr_list, sizeof(romaji_list), romajicmp );

  p    = list_mark;
  nr_p = countof(list_mark);
  for( i=0; i<nr_p; i++ ){
    list[ nr_list ++ ] = *p ++;
    if( nr_list >= countof(list) ) break;
  }
  
/*
  printf("%d\n",nr_list);
  for( i=0; i<nr_list; i++ ){ printf("%s ", list[i].s ); }
  printf("\n%d %d %d\n",countof(list_msime),countof(list_atok),countof(list_egg));
*/
}




void	romaji_clear( void )
{
  romaji_set  = 0;
  romaji_ptr  = 0;
  press_timer = 0;
  input_size  = 0;
}

/*
 *	���͂����[�}���ɕϊ����A�L���[�ɒ~���鏈���֐�
 */
static	void	romaji_check( int key )
{
  int i, j;

  if( key > 0x7f ) return;
  if( key==' ' ||
      key=='@' ||
      key=='[' ||
      key=='/' ||
      key=='-' ||
      key=='{' ||
      key=='}' ||
      key=='.' ||
      key==',' ||
      key=='\'' ) ;				/* �����̋L���͗L�� */
  else if( islower(key) ) key = toupper( key );	/* a-z �� A-Z �ɕϊ�  */
  else if( isupper(key) )  ;			/* A-Z �͂��̂܂ܗL�� */
  else return;					/* ����ȊO�͖���     */

  input_buf[ input_size ] = key;
  input_size ++;

  while( input_size ){
    
    int          list_size = nr_list;
    romaji_list *list_p   = list;

    int same   = FALSE;
    int nearly = FALSE;

    for( i=0; i<list_size; i++, list_p++ ){

      const char *s1 = input_buf;
      const char *s2 = list_p->s;

      for( j=0; j<input_size; j++, s1++, s2++ ){
	if( *s1 != *s2 ){ j=0; break; }
      }

      if( j==0 ){			/* �s��v */
	if( nearly ) break;			/* �r���܂ň�v���Ă��̂� */
	/* list->s �̃\�[�g�ς݂��O�� */	/* ��v���Ȃ��Ȃ����璆�f */
      }else{
	if( *s2 == '\0' ){		/* ���S�Ɉ�v */
	  same   = TRUE;	break;
	}else{				/* �r���܂ň�v */
	  nearly = TRUE;
	}
      }
    }

    if( same ){					/*** ���S��v�̏ꍇ */
      set_romaji_que( list_p->list );			/* �L���[�ɃZ�b�g */
      input_size = 0;					/* ���͂��̂Ă�   */
      break;

    }else if( nearly ){				/*** �r���܂ň�v�̏ꍇ */
      break;						/* ���͂͂��̂܂� */

    }else{					/*** �s��v�̏ꍇ */

      if( input_buf[0] == 'N' ){
	set_romaji_que( list_NN.list );			/* ����L���[�ɃZ�b�g*/

      }else if( input_size >= 2 &&
		input_buf[0]==input_buf[1] ){
	set_romaji_que( list_tu.list );			/* �����L���[�ɃZ�b�g*/
      }

      input_size --;					/* ���͂����炷 */
      memmove( &input_buf[0], &input_buf[1],input_size );

      /* ��v����܂Ń`�F�b�N���Ȃ��� */
    }
  }


  if( input_size >= (int)sizeof(input_buf) )	/* �o�b�t�@�I�[�o�[�h�~ */
    input_size = 0;

}
