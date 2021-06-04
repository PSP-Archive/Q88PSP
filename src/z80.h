#ifndef Z80_H_INCLUDED
#define Z80_H_INCLUDED



/* --- Z80 CPU のエミュレート構造体 --- */

typedef struct{

  pair	AF, BC, DE, HL;			/* 汎用レジスタ */
  pair	IX, IY, PC, SP;			/* 専用レジスタ */
  pair	AF1,BC1,DE1,HL1;		/*  裏 レジスタ	*/
  byte	I, R;				/* 特殊レジスタ */
  byte	R_saved;			/* R reg 保存用 */
  Uchar	IFF,IFF2;			/* IFF1 、IFF2	*/
  Uchar	IM;				/* 割込モード	*/
  Uchar	HALT;				/* HALT フラグ	*/
  Uchar	discontinue;			/* 特殊中断処理 */

  int	icount;				/* 割り込み発生までのステート数 */
  int	state;				/* 処理した命令の総ステート数	*/
  int	state0;				/*		〃		*/


  Uchar	log;				/* 真ならデバッグ用のログを記録	*/
  Uchar	break_if_halt;			/* HALT時に処理ループから強制脱出*/

  byte	(*mem_read)(word);		/* メモリリード関数	*/
  void	(*mem_write)(word,byte);	/* メモリライト関数	*/
  byte	(*io_read)(byte);		/* I/O 入力関数		*/
  void	(*io_write)(byte,byte);		/* I/O 出力関数		*/

  int	(*intr_update)(void);		/* 割込情報更新関数	*/
  int	(*intr_chk)(void);		/* 割込チェック関数	*/

  pair  PC_prev;			/* 直前の PC (モニタ用)	*/

} z80arch;


/* z80arch の各コールバック関数内にて、
   state + state0 が現在までに処理した 総ステート数となる。 */




/* IFF の中身 */
#define INT_DISABLE	(0)
#define INT_ENABLE	(1)




extern	void	z80_reset( z80arch *z80 );
extern	int	z80_emu( z80arch *z80, int state_of_exec );
extern	void	z80_debug( z80arch *z80, char *mes );
extern	int	z80_line_disasm( z80arch *z80, word addr );

extern	void	z80_logging( z80arch *z80 );



extern	int	highspeed_flag;

#define	refresh_intr_timing( z80 )	if(highspeed_flag==FALSE) z80.icount=0
#define	set_cpu_dormant( z80 )		z80.icount = 0
					    

#endif		/* Z80_H_INCLUDED */
