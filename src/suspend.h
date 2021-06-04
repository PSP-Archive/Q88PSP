#ifndef SUSPEND_H_INCLUDED
#define SUSPEND_H_INCLUDED

#include <stdio.h>


extern	int	resume_flag;			/* 起動時のレジューム	*/
extern	int	resume_force;			/* 強制レジューム	*/
extern	char	*file_resume;			/* ステートファイル名	*/

typedef	struct{
  enum {
    TYPE_INT,
    TYPE_SHORT,
    TYPE_CHAR,
    TYPE_BYTE,
    TYPE_WORD,
    TYPE_PAIR,
    TYPE_DOUBLE,
    TYPE_256,
    TYPE_STR
  }	type;
  void	*work;
} T_SUSPEND_W;



#define	OFFSETOF(s, m)	((size_t)(&((s *)0)->m))

typedef	struct{
  char	header[128];
  char	emu[128];
  char	graph[128];
  char	memory[128];
  char	wait[128];
  char	pc88main[256];
  char	crtcdmac[128];
  char	sound[128*6];
  char	pio[128];
  char	screen[128];
  char	intr[128*2];
  char	keyboard[128];
  char	pc88sub[128];
  char	fdc[128*17];
  char	snddrv[128];

  char	fdc_data[0x4000];
  char	memory_data[  0x10000		/* メインRAM 64KB */
		   +   0x1000		/* 高速RAM    4KB */
		   +   0x4000		/* サブRAM   16KB */
		   + 4*0x4000 ];	/* VRAM+α 4*16KB */
} suspend_0;



int	suspend_emu	( FILE *fp, long offset );
int	 resume_emu	( FILE *fp, long offset );
int	suspend_graph	( FILE *fp, long offset );
int	 resume_graph	( FILE *fp, long offset );
int	suspend_memory	( FILE *fp, long offset, long offset2 );
int	 resume_memory	( FILE *fp, long offset, long offset2 );
int	suspend_wait	( FILE *fp, long offset );
int	 resume_wait	( FILE *fp, long offset );
int	suspend_pc88main( FILE *fp, long offset );
int	 resume_pc88main( FILE *fp, long offset );
int	suspend_crtcdmac( FILE *fp, long offset );
int	 resume_crtcdmac( FILE *fp, long offset );
int	suspend_sound	( FILE *fp, long offset );
int	 resume_sound	( FILE *fp, long offset );
int	suspend_pio	( FILE *fp, long offset );
int	 resume_pio	( FILE *fp, long offset );
int	suspend_screen	( FILE *fp, long offset );
int	 resume_screen	( FILE *fp, long offset );
int	suspend_intr	( FILE *fp, long offset );
int	 resume_intr	( FILE *fp, long offset );
int	suspend_keyboard( FILE *fp, long offset );
int	 resume_keyboard( FILE *fp, long offset );
int	suspend_pc88sub	( FILE *fp, long offset );
int	 resume_pc88sub	( FILE *fp, long offset );
int	suspend_fdc	( FILE *fp, long offset, long offset2 );
int	 resume_fdc	( FILE *fp, long offset, long offset2 );
int	suspend_snddrv	( FILE *fp, long offset );
int	 resume_snddrv	( FILE *fp, long offset );

int	resume_memory_preparation(  FILE *fp, long offset );

int	suspend_work( FILE *fp, long offset, T_SUSPEND_W *tbl, int count );
int	resume_work( FILE *fp, long offset, T_SUSPEND_W *tbl, int count );


void	pc88main_init_at_resume( void );
void	crtc_dmac_init_at_resume( void );
void	sound_init_at_resume( void );
void	pio_init_at_resume( void );
void	main_INT_init_at_resume( void );
void	keyboard_init_at_resume( void );
void	pc88sub_init_at_resume( void );
void	fdc_init_at_resume( void );


int	suspend( const char *suspend_filename );
int	resume( const char *suspend_filename );

void	resume_init( void );

#endif	/* SUSPEND_H_INCLUDED */
