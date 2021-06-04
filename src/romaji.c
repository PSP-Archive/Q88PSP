/************************************************************************/
/*									*/
/* ローマ字→カナ変換処理						*/
/*									*/
/*		keyboard.c ファイル内から、include されている。		*/
/************************************************************************/


static	int	romaji_input_mode = FALSE;	/* 真の時、ローマ字入力モード*/

static	char	input_buf[4];			/* 入力済みの文字のバッファ  */
static	int	input_size;			/* 入力済みの文字の数	     */


#define	ROMAJI_QUE_SIZE	(64)
static	int	romaji_set;			/* ローマ字入力されたカナは */
static	int	romaji_ptr;			/* キューに蓄えられ、順次   */
static	Uchar	romaji_que[ ROMAJI_QUE_SIZE ];	/* I/O ポートに送られていく */


/* キューに蓄える値。ポート情報がパックしてある */
#define	RJ( port, bit, shift )	((Uchar)( (port<<4) | (shift<<3) | bit ))


static	int	press_timer;			/* キーオン・オフのタイマー */
#define	KEY_ON_OFF_INTERVAL	(4)		/* キーオン・オフの時間	    */


/*
 *	キューに蓄える関数
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
 *	キューからポートに出力していく関数
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
   ローマ字入力モードに入った時／出た時の処理
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
 *	ローマ字 → カナ 変換テーブル
 */
typedef struct {
  const char *s;
  Uchar      list[4];
} romaji_list;

#if 0	/* romaji-table.c 内で定義 */
static const romaji_list list_NN;
static const romaji_list list_tu;
static const romaji_list list_mark[];
static const romaji_list list_msime[];
static const romaji_list list_atok[];
static const romaji_list list_egg[];
#endif

#include "romaji-table.c"


/*
 *	ローマ字カナ変換テーブルの作成
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
 *	入力をローマ字に変換し、キューに蓄える処理関数
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
      key=='\'' ) ;				/* これらの記号は有効 */
  else if( islower(key) ) key = toupper( key );	/* a-z → A-Z に変換  */
  else if( isupper(key) )  ;			/* A-Z はそのまま有効 */
  else return;					/* それ以外は無効     */

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

      if( j==0 ){			/* 不一致 */
	if( nearly ) break;			/* 途中まで一致してたのに */
	/* list->s のソート済みが前提 */	/* 一致しなくなったら中断 */
      }else{
	if( *s2 == '\0' ){		/* 完全に一致 */
	  same   = TRUE;	break;
	}else{				/* 途中まで一致 */
	  nearly = TRUE;
	}
      }
    }

    if( same ){					/*** 完全一致の場合 */
      set_romaji_que( list_p->list );			/* キューにセット */
      input_size = 0;					/* 入力を捨てる   */
      break;

    }else if( nearly ){				/*** 途中まで一致の場合 */
      break;						/* 入力はそのまま */

    }else{					/*** 不一致の場合 */

      if( input_buf[0] == 'N' ){
	set_romaji_que( list_NN.list );			/* んをキューにセット*/

      }else if( input_size >= 2 &&
		input_buf[0]==input_buf[1] ){
	set_romaji_que( list_tu.list );			/* っをキューにセット*/
      }

      input_size --;					/* 入力をずらす */
      memmove( &input_buf[0], &input_buf[1],input_size );

      /* 一致するまでチェックしなおす */
    }
  }


  if( input_size >= (int)sizeof(input_buf) )	/* バッファオーバー防止 */
    input_size = 0;

}
