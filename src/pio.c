/************************************************************************/
/*									*/
/* PIO の 処理								*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "pio.h"

#include "pc88cpu.h"

#include "emu.h"
#include "suspend.h"


/* PIO アクセスによる、CPU の切替え					*/
/*	  PC88 では、メインCPU (本体側) とサブCPU (ディスク側)とに、	*/
/*	それぞれCPU を持っている。通常はどちらか片方のみをエミュレート	*/
/*	し、PIO のアクセスがあった時に、処理する側のCPUを切替える。これ	*/
/*	により、エミュレートの処理速度を上げている。			*/
/*									*/
/*	  このCPUの切替えタイミングだが、通常は、PIO の Cポートをリード	*/
/*	した時のみに行なう。PIO の Cポートは、A または B ポートの送受信	*/
/*	の際に、データの準備や読み込みの完了を示すフラグをやりとりする	*/
/*	ポートである。							*/
/*		(-cpu 0 オプションを指定した時は上記の動作をする)	*/
/*									*/
/*	  しかし、中には Cポートを介さずに、データのリード/ライトを行な	*/
/*	うソフトも存在する。(2個のCPUの処理速度の同期に依存している)	*/
/*	こういったソフトの対策として、A/B/C ポートのいずれかにメインCPU	*/
/*	がアクセスした時点から一定期間 (標準では 256ステップ)、サブCPU	*/
/*	も並列に処理するようにした。					*/
/*		(-cpu 1 オプションを指定した時は上記の動作をする)	*/
/*									*/
/*	  それでもまだ動かないソフトがある。CPUの処理速度に依存している	*/
/*	のだろうか？ そこで常時、メインCPUとサブCPUを並列に処理する設定	*/
/*	も設けてみた。							*/
/*		(-cpu 2 オプションを指定した時は上記の動作をする)	*/
/*									*/
/*	  なおも動かないソフトが有る。そこで、メインCPUとサブCPUを並列	*/
/*	に処理するが、A/B ポートアクセス時、相手がポートアクセスして	*/
/*	いない時は、一定期間ウエイトをとるようにしてみた。		*/
/*		(-cpu 3 オプションを指定した時は上記の動作をする)	*/

/*---------------------------------------------------------------------------*/
/* 処理									     */
/*					A  ######## --\/-- ######## A	     */
/*	・ライトは自分のワークに	B  ######## --/\-- ######## B	     */
/*	  対して行なう。		CH ####     --\/-- ####	    CH	     */
/*	・リードは相手のワーク		CL     #### --/\--     #### CL	     */
/*	  から行なう。							     */
/*	  自分のワークがREAD設定なら、					     */
/*	  自分のワークを読む。						     */
/*	○連続リード、連続ライトは例外処理を行なう。			     */
/*	○ポートCのリードライト時は例外処理を行なう。			     */
/*---------------------------------------------------------------------------*/


pio_work	pio_AB[2][2], pio_C[2][2];


static	z80arch	*z80[2] = { &z80main_cpu, &z80sub_cpu };


#define	PIO_CHG_CNT		(16)


/*----------------------------------------------------------------------*/
/* PIO 初期化								*/
/*	PA / PCL 受信 ／ PB / PCH 送信					*/
/*----------------------------------------------------------------------*/
void	pio_init( void )
{
  int	side;

  for( side=0; side<2; side++ ){
    pio_AB[ side ][ PIO_PORT_A ].type    = PIO_READ;
    pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;
    pio_AB[ side ][ PIO_PORT_A ].chg_cnt = PIO_CHG_CNT;
    pio_AB[ side ][ PIO_PORT_A ].data    = 0x00;

    pio_AB[ side ][ PIO_PORT_B ].type    = PIO_WRITE;
    pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;
    pio_AB[ side ][ PIO_PORT_B ].chg_cnt = PIO_CHG_CNT;
    pio_AB[ side ][ PIO_PORT_B ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CH ].type    = PIO_WRITE;
    pio_C[ side ][ PIO_PORT_CH ].chg_cnt = 1;
    pio_C[ side ][ PIO_PORT_CH ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CL ].type    = PIO_READ;
    pio_C[ side ][ PIO_PORT_CL ].chg_cnt = 1;
    pio_C[ side ][ PIO_PORT_CL ].data    = 0x00;
  }

}


/* verbose 指定時のメッセージ表示マクロ					*/

#define	pio_mesAB( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s : port = %s\n",			\
		 (side==PIO_SIDE_M)?"M":"S", (port==PIO_PORT_A)?"A":"B" )

#define	pio_mesC( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s\n",					\
		 (side==PIO_SIDE_M)?"M":"S" )

/*----------------------------------------------------------------------*/
/* PIO A or B からリード						*/
/*	リードの際のデータは、相手の側／逆のポートから読み出す。	*/
/*		相手のポートの設定が  READ ならエラー表示		*/
/*		自分のポートの設定が WRITE ならエラー表示		*/
/*		連続リードの際は、カウンタをカウントダウンする。	*/
/*			カウンタが 0 なら諦めて、連続リードする。	*/
/*			カウンタが 1 以上なら、CPU を切替える。		*/
/*----------------------------------------------------------------------*/
byte	pio_read_AB( int side, int port )
{
		/* ポート属性不一致 */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_READ  ){	/* 相手ポートが READ */
    pio_mesAB( "PIO AB READ PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_WRITE ){	/* 自分ポートが WRITE*/
    pio_mesAB( "PIO Read from WRITE-PORT" );
    pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
    return (pio_AB[ side ][ port ].data);
  }
		/* 読みだし */

  if( pio_AB[ side^1 ][ port^1 ].exist == PIO_EXIST ){	/* -- 最初の読みだし */

    pio_AB[ side^1 ][ port^1 ].exist   = PIO_EMPTY;
    pio_AB[ side   ][ port   ].chg_cnt = PIO_CHG_CNT;

  }else{						/* -- 連続の読みだし */

    switch( cpu_timing ){
    case 1:						/*     1:サブCPU起動 */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	set_cpu_dormant( z80main_cpu );
      } /*No Break*/
    case 0:						/*     0:そのまま読む*/
    case 2:						/*     2:そのまま読む*/
      pio_mesAB( "PIO Read continuously" );
      pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      break;
    case 3:						/*     3:処理を飛ばす*/
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	z80[side]->discontinue = TRUE;
      }else{
	pio_mesAB( "PIO Read continuously" );
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    default:						/*    -1:CPUを切替え*/
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	select_main_cpu ^= 1;
	z80[side]->discontinue = TRUE;
	/*z80sub_cpu.state = 0;*/
      }else{
	pio_mesAB( "PIO Read continuously" );
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    }

  }
  return (pio_AB[ side^1 ][ port^1 ].data);
}


/*----------------------------------------------------------------------*/
/* PIO A or B にライト							*/
/*	ライトは、自分の側／自分のポートに対して行なう。		*/
/*		相手のポートの設定が WRITE ならエラー表示		*/
/*		自分のポートの設定が  READ ならエラー表示		*/
/*		連続ライトの際は、カウンタをカウントダウンする。	*/
/*			カウンタが 0 なら諦めて、連続ライトする。	*/
/*			カウンタが 1 以上なら、CPU を切替える。		*/
/*----------------------------------------------------------------------*/
void	pio_write_AB( int side, int port, byte data )
{
		/* ポート属性不一致 */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* 相手のポート WRITE*/
    pio_mesAB( "PIO AB Write PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_READ ){	/* 自分のポート READ */
    pio_mesAB( "PIO Write to READ-PORT" );
  }
		/* 書き込み */

  if( pio_AB[ side ][ port ].exist == PIO_EMPTY ){	/* -- 最初の書き込み */

    pio_AB[ side ][ port ].exist   = PIO_EXIST;
    pio_AB[ side ][ port ].data    = data;
    pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;

  }else{						/* -- 連続の書き込み */

    switch( cpu_timing ){
    case 1:						/*     1:サブCPU起動 */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	set_cpu_dormant( z80main_cpu );
      } /*No Break*/
    case 0:						/*     0:そのまま書く*/
    case 2:						/*     2:そのまま書く*/
      pio_mesAB( "PIO Write continuously" );
      pio_AB[ side ][ port ].data    = data;
      pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      break;
    case 3:						/*     3:処理を飛ばす*/
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	z80[side]->discontinue = TRUE;
	break;
      }else{
	pio_mesAB( "PIO Write continuously" );
	pio_AB[ side ][ port ].data    = data;
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    default:						/*    -1:CPUを切替え */
      if( -- pio_AB[ side ][ port ].chg_cnt ){
	select_main_cpu ^= 1;
	z80[side]->discontinue = TRUE;
	/*z80sub_cpu.state = 0;*/
      }else{
	pio_mesAB( "PIO Write continuously" );
	pio_AB[ side ][ port ].data    = data;
	pio_AB[ side ][ port ].chg_cnt = PIO_CHG_CNT;
      }
      break;
    }

  }
  return;
}








/*----------------------------------------------------------------------*/
/* PIO C からリード							*/
/*	リードの際のデータは、相手の側／逆のポートから読み出す。	*/
/*		相手のポートの設定が  READ ならエラー表示		*/
/*		自分のポートの設定が WRITE ならエラー表示		*/
/*		リードの際に、CPUを切替え判定を入れる			*/
/*----------------------------------------------------------------------*/
byte	pio_read_C( int side )
{
  byte	data;

		/* ポート属性不一致 */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C READ PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C Read from WRITE-PORT" );
  }
		/* リード */

  if( pio_C[ side ][ PIO_PORT_CH ].type == PIO_READ ){
    data  = pio_C[ side^1 ][ PIO_PORT_CL ].data << 4;
  }else{
    data  = pio_C[ side   ][ PIO_PORT_CH ].data << 4;
  }

  if( pio_C[ side ][ PIO_PORT_CL ].type == PIO_READ ){
    data |= pio_C[ side^1 ][ PIO_PORT_CH ].data;
  }else{
    data |= pio_C[ side   ][ PIO_PORT_CL ].data;
  }

  pio_C[ side ][ PIO_PORT_CL ].chg_cnt ^= 1;
  if( pio_C[ side ][ PIO_PORT_CL ].chg_cnt == 0 ){	/* -- 連続の読みだし */

    switch( cpu_timing ){
    case 0:						/*     0:CPUを切替え*/
      select_main_cpu ^= 1;
      z80[side]->discontinue = TRUE;
      /*z80sub_cpu.state = 0;*/
      break;
    case 1:						/*     1:サブCPU起動 */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	set_cpu_dormant( z80main_cpu );
      }
      break;
    case 2:						/*     2:なにもしない*/
    case 3:						/*     3:なにもしない*/
      break;
    default:						/*    -1:CPUを切替え*/
      select_main_cpu ^= 1;
      z80[side]->discontinue = TRUE;
      break;
    }

  }

  return data;
}


/*----------------------------------------------------------------------*/
/* PIO C にライト							*/
/*	ライトは、自分の側／自分のポートに対して行なう。		*/
/*		相手のポートの設定が WRITE ならエラー表示		*/
/*		自分のポートの設定が  READ ならエラー表示		*/
/*		ライトの際に、CPUを切替え判定を入れる			*/
/*----------------------------------------------------------------------*/
void	pio_write_C( int side, byte data )
{
  int port;

  if( data & 0x08 ) port = PIO_PORT_CH;
  else              port = PIO_PORT_CL;
  data &= 0x07;

		/* ポート属性不一致 */

  if( pio_C[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* 相手のポート WRITE*/
    pio_mesC( "PIO C Write PORT Mismatch" );
  }
  if( pio_C[ side   ][ port   ].type == PIO_READ ){	/* 自分のポート READ */
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* ライト */

  if( data & 0x01 ) pio_C[ side ][ port ].data |=  ( 1 << (data>>1) );
  else              pio_C[ side ][ port ].data &= ~( 1 << (data>>1) );

  switch( cpu_timing ){
  case 0:						/*     0:そのまま書く*/
  case 2:						/*     2:そのまま書く*/
  case 3:						/*     3:そのまま書く*/
    break;
  case 1:						/*     1:サブCPU起動 */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      set_cpu_dormant( z80main_cpu );
    }
    break;
  default:						/*    -1:CPUを切替え */
    select_main_cpu ^= 1;
    /*z80sub_cpu.state = 0;*/
    if( side==PIO_SIDE_M ){
      dual_cpu_count = 1;
      set_cpu_dormant( z80main_cpu );
    }else{
      set_cpu_dormant( z80sub_cpu );
    }
    break;
  }
  return;
}


/*--------------------------------------------------------------*/
/* 直接 Port C に書き込む					*/
/*--------------------------------------------------------------*/
void	pio_write_C_direct( int side, byte data )
{
		/* ポート属性不一致 */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C WRITE PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* ライト */

  pio_C[ side ][ PIO_PORT_CH ].data = data >> 4;
  pio_C[ side ][ PIO_PORT_CL ].data = data & 0x0f;

  switch( cpu_timing ){
  case 0:						/*     0:そのまま書く*/
  case 2:						/*     2:そのまま書く*/
  case 3:						/*     3:そのまま書く*/
    break;
  case 1:						/*     1:サブCPU起動 */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      set_cpu_dormant( z80main_cpu );
    }
    break;
  default:						/*    -1:CPUを切替え */
    select_main_cpu ^= 1;
    /*z80sub_cpu.state = 0;*/
    if( side==PIO_SIDE_M ){
      dual_cpu_count = 1;
      set_cpu_dormant( z80main_cpu );
    }else{
      set_cpu_dormant( z80sub_cpu );
    }
    break;
  }
  return;
}







/*----------------------------------------------------------------------*/
/* PIO 設定								*/
/*	PA / PB / PCH / PCL の送受信を指定。				*/
/*	モードを設定 (モードは 0 に限定。詳細不明)			*/
/*----------------------------------------------------------------------*/
void	pio_set_mode( int side, byte data )
{
  if( data & 0x60 ){
    if( verbose_pio )
      printf("PIO mode A & CH not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>5)&0x3 );
  }
	/* PIO A */

  if( data & 0x10 ){
    pio_AB[ side ][ PIO_PORT_A ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_A ].type   = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_A ].data    = 0;
  pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;
  pio_AB[ side ][ PIO_PORT_A ].chg_cnt = PIO_CHG_CNT;

	/* PIO C-H */

  if( data & 0x08 ){
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CH ].data    = 0;
  pio_C[ side ][ PIO_PORT_CH ].chg_cnt = 1;

  if( data & 0x04 ){
    if( verbose_pio )
      printf("PIO mode B & CL not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>2)&0x1 );
  }
	/* PIO B */

  if( data & 0x02 ){
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_B ].data    = 0;
  pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;
  pio_AB[ side ][ PIO_PORT_B ].chg_cnt = PIO_CHG_CNT;

	/* PIO C-L */

  if( data & 0x01 ){
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CL ].data    = 0;
  pio_C[ side ][ PIO_PORT_CL ].chg_cnt = 1;

}






/****************************************************************/	
/* サスペンド／レジューム					*/
/****************************************************************/	
static	T_SUSPEND_W	suspend_pio_work[]=
{
  { TYPE_INT,	&pio_AB[0][0].type,	},
  { TYPE_INT,	&pio_AB[0][0].exist,	},
  { TYPE_INT,	&pio_AB[0][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[0][0].data,	},

  { TYPE_INT,	&pio_AB[0][1].type,	},
  { TYPE_INT,	&pio_AB[0][1].exist,	},
  { TYPE_INT,	&pio_AB[0][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[0][1].data,	},

  { TYPE_INT,	&pio_AB[1][0].type,	},
  { TYPE_INT,	&pio_AB[1][0].exist,	},
  { TYPE_INT,	&pio_AB[1][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[1][0].data,	},

  { TYPE_INT,	&pio_AB[1][1].type,	},
  { TYPE_INT,	&pio_AB[1][1].exist,	},
  { TYPE_INT,	&pio_AB[1][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_AB[1][1].data,	},

  { TYPE_INT,	&pio_C[0][0].type,	},
  { TYPE_INT,	&pio_C[0][0].exist,	},
  { TYPE_INT,	&pio_C[0][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[0][0].data,	},

  { TYPE_INT,	&pio_C[0][1].type,	},
  { TYPE_INT,	&pio_C[0][1].exist,	},
  { TYPE_INT,	&pio_C[0][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[0][1].data,	},

  { TYPE_INT,	&pio_C[1][0].type,	},
  { TYPE_INT,	&pio_C[1][0].exist,	},
  { TYPE_INT,	&pio_C[1][0].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[1][0].data,	},

  { TYPE_INT,	&pio_C[1][1].type,	},
  { TYPE_INT,	&pio_C[1][1].exist,	},
  { TYPE_INT,	&pio_C[1][1].chg_cnt,	},
  { TYPE_BYTE,	&pio_C[1][1].data,	},
};


int	suspend_pio( FILE *fp, long offset )
{
  int	ret = suspend_work( fp, offset, 
			    suspend_pio_work, 
			    countof(suspend_pio_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->pio ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  return TRUE;
}


int	resume_pio(  FILE *fp, long offset )
{
  int	ret = resume_work( fp, offset, 
			   suspend_pio_work, 
			   countof(suspend_pio_work) );

  if( ret<0 ) return FALSE;
  return TRUE;
}


void	pio_init_at_resume( void )
{
}
