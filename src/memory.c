/************************************************************************/
/*									*/
/* メモリの確保								*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "memory.h"

#include "sound.h"		/* sound_board, sound2_adpcm	*/

#include "menu.h"		/* menu_lang	*/
#include "file-op.h"
#include "suspend.h"

#include "exmem.h"



int	use_extram	= DEFAULT_EXTRAM;	/* 拡張RAMのカード数	*/
int	use_jisho_rom	= DEFAULT_JISHO;	/* 辞書ROMを使う	*/

char	*file_compatrom = NULL;			/* P88SR emu のROMを使う*/


/*----------------------------------------------------------------------*/
/* フォントROM、漢字ROM ともにない時用の、フォントデータ		*/
/*----------------------------------------------------------------------*/
#include "font.c"




byte	*main_rom;			/* メイン ROM [0x8000] (32KB)	*/
byte	(*main_rom_ext)[0x2000];	/* 拡張 ROM [4][0x2000](8KB *4)	*/
byte	*main_rom_n;			/* N-BASIC [0x8000]    (32KB)	*/
byte	*main_ram;			/* メイン RAM [0x10000](64KB)	*/
byte	*main_high_ram;			/* 高速 RAM(の裏)[0x1000] (4KB)	*/
byte	(*kanji_rom)[65536][2];		/* 漢字ＲＯＭ[2][65536][2]	*/
byte	*sub_romram;			/* サブ ROM/RAM [0x8000] (32KB)	*/

byte	(*ext_ram)[0x8000];		/* 拡張 RAM[4][0x8000](32KB*4～)*/
byte	(*jisho_rom)[0x4000];		/* 辞書 ROM[32][0x4000](16KB*32)*/

bit8	(*main_vram)[4];		/* VRAM[0x4000][4](=G/R/G/pad)	*/
bit8	*font_rom;			/* フォントイメージROM[8*256*2]	*/


byte	*dummy_rom;			/* ダミーROM (32KB)		*/
byte	*dummy_ram;			/* ダミーRAM (32KB)		*/

byte	*read_mem_0000_5fff;		/* メインメモリ リードポインタ	*/
byte	*read_mem_6000_7fff;
byte	*read_mem_8000_83ff;
byte	*read_mem_c000_efff;
byte	*read_mem_f000_ffff;

byte	*write_mem_0000_7fff;		/* メインメモリ ライトポインタ	*/
byte	*write_mem_8000_83ff;
byte	*write_mem_c000_efff;
byte	*write_mem_f000_ffff;




/*----------------------------------------------------------------------*/
/* PC8801 エミュレートで使用する ROM/RAM 用のメモリ確保 ＆ ROMロード	*/
/*----------------------------------------------------------------------*/
byte	*allocate_romram( size_t romram_size )
{
  byte	*ptr;

  ptr  = (byte *)ex_malloc( sizeof(byte)*romram_size );
  if( !ptr ){ if( verbose_proc ) printf("FAILED\n"); }
  else      { if( verbose_proc ) printf("OK\n");     }

  return ptr;
}

static	size_t	load_rom( char *filename, byte *rom_ptr, size_t rom_size )
{
  FILE *fp;
  int	status;
  size_t load_size = 0;

  if( (fp = osd_fopen_rom( filename, &status )) ){
    load_size =ex_fread( rom_ptr, sizeof(byte), rom_size, fp );
    ex_fclose(fp);
  }

  if( verbose_proc ){
    if( load_size==rom_size ) printf("OK\n");
    else{
      if( fp ) printf("FAILED (data missing)\n");
      else{
	if( status==FOPEN_NO_MEM ) printf("FAILED (malloc error)\n");
	else                       printf("FAILED (not open)\n");
      }
    }
  }

  return load_size;
}



/************************************************************************/
/* PC8801 VRAM 用のメモリを確保 (0x4000 * 4 Byte = B/R/G/pad)		*/
/************************************************************************/
static	int	vram_allocate( void )
{
  if( verbose_proc ) printf("Allocating 48kB for video ram...");
  if( !(main_vram
	      = (byte (*)[0x4])allocate_romram( sizeof(byte)*0x4000*4 ) ) ){
    return(0);
  }
  memset( &main_vram[0][0], 0x00, 0x4000*4 );
  return(1);
}
static	void	vram_free( void )
{
  ex_free( main_vram );
}


/************************************************************************/
/* PC8801 テキストフォント用のメモリを確保 ＆ フォントロード		*/
/************************************************************************/
static	int	exist_kanji_rom	= FALSE;
static	int	use_system_font = FALSE;

static	int	font_allocate( char *filename )
{
  if( verbose_proc ) printf("Allocating 4kB for text font...");
  if( !( font_rom = allocate_romram( sizeof(byte)*8*256*2 ) ) ){
    use_system_font = TRUE;
  }else{
    if( verbose_proc ) printf("  Loading %s...",FONT_FILE);
    if( load_rom( filename, font_rom, 8*256*1 )==8*256*1 ){
      /* セミグラフィック用文字は内蔵のものを使用 */
      memcpy( &font_rom[0x100*8], &system_font[0x100*8], 0x100*8 );
    } else {
      /* ロード失敗したら、代わりのフォントを使う */
      if( exist_kanji_rom ){
	if( verbose_proc ) printf("  Using KANJI-font-rom.\n");
	memcpy( &font_rom[0], &kanji_rom[0][(1<<11)][0], 0x100*8 );
	memcpy( &font_rom[0x100*8], &system_font[0x100*8], 0x100*8 );
      }else{
	ex_free( font_rom );
	use_system_font = TRUE;
      }
    }
  }
  if( use_system_font ){
    if( verbose_proc ) printf("  Using QUASI88-system-fonts.\n");
    font_rom = system_font;
  }
  return(1);
}
static	void	font_free( void )
{
  if( !use_system_font ) ex_free( font_rom );
}















/************************************************************************/
/* QUASI88 で使用するメモリの確保					*/
/*	peachさんにより、M88 の ROMファイルも使えるように拡張されました	*/
/************************************************************************/

int	memory_allocate( void )
{
  size_t	load_size;


		/* main_rom … メイン ROM */

  if( verbose_proc ) printf("Allocating 32kB for N88BASIC rom...");
  if( !( main_rom = allocate_romram( sizeof(byte)*0x8000 ) ) ){
    return(0);
  }
  if( !file_compatrom ){
    if( verbose_proc ) printf("  Loading %s...", N88_ROM_FILE);
    load_size = load_rom( N88_ROM_FILE, main_rom, 0x8000 );
    if( load_size != 0x8000 ){
      memset( &main_rom[load_size], 0x76, 0x8000-load_size );
    }
  }


		/* main_rom_ext[0～3] … 拡張 ROM */

  if( verbose_proc ) printf("Allocating 8*4kB for N88BASIC EXT rom...");
  if( !( main_rom_ext
             =(byte (*)[0x2000])allocate_romram( sizeof(byte)*0x2000*4 ) ) ){
    return(0);
  }
  if( !file_compatrom ){

    if( verbose_proc ) printf("  Loading %s...",N88EXT0_ROM_FILE);
    load_size = load_rom( N88EXT0_ROM_FILE, main_rom_ext[0], 0x2000 );
    if( load_size == 0 ){
      if( verbose_proc ) printf("  Loading %s...",N88EXT0_ROM_FILE2);
      load_size = load_rom( N88EXT0_ROM_FILE2, main_rom_ext[0], 0x2000 );
    }
    if( load_size != 0x2000 ){
      memset( &main_rom_ext[0][load_size], 0x76, 0x2000-load_size );
    }

    if( verbose_proc ) printf("  Loading %s...",N88EXT1_ROM_FILE);
    load_size = load_rom( N88EXT1_ROM_FILE, main_rom_ext[1], 0x2000 );
    if( load_size == 0 ){
      if( verbose_proc ) printf("  Loading %s...",N88EXT1_ROM_FILE2);
      load_size = load_rom( N88EXT1_ROM_FILE2, main_rom_ext[1], 0x2000 );
    }
    if( load_size != 0x2000 ){
      memset( &main_rom_ext[1][load_size], 0x76, 0x2000-load_size );
    }

    if( verbose_proc ) printf("  Loading %s...",N88EXT2_ROM_FILE);
    load_size = load_rom( N88EXT2_ROM_FILE, main_rom_ext[2], 0x2000 );
    if( load_size == 0 ){
      if( verbose_proc ) printf("  Loading %s...",N88EXT2_ROM_FILE2);
      load_size = load_rom( N88EXT2_ROM_FILE2, main_rom_ext[2], 0x2000 );
    }
    if( load_size != 0x2000 ){
      memset( &main_rom_ext[2][load_size], 0x76, 0x2000-load_size );
    }

    if( verbose_proc ) printf("  Loading %s...",N88EXT3_ROM_FILE);
    load_size = load_rom( N88EXT3_ROM_FILE, main_rom_ext[3], 0x2000 );
    if( load_size == 0 ){
      if( verbose_proc ) printf("  Loading %s...",N88EXT3_ROM_FILE2);
      load_size = load_rom( N88EXT3_ROM_FILE2, main_rom_ext[3], 0x2000 );
    }
    if( load_size != 0x2000 ){
      memset( &main_rom_ext[3][load_size], 0x76, 0x2000-load_size );
    }
  }


		/* main_rom_n … N - BASIC */

  if( verbose_proc ) printf("Allocating 32kB for NBASIC rom...");
  if( !( main_rom_n = allocate_romram( sizeof(byte)*0x8000 ) ) ){
    return(0);
  }
  if( !file_compatrom ){
    if( verbose_proc ) printf("  Loading %s...",N88N_ROM_FILE);
    load_size = load_rom( N88N_ROM_FILE, main_rom_n, 0x8000 );
    if( load_size == 0 ){
      if( verbose_proc ) printf("  Loading %s...",N88N_ROM_FILE2);
      load_size = load_rom( N88N_ROM_FILE2, main_rom_n, 0x8000 );
    }
    if( load_size != 0x8000 ){
      memset( &main_rom_n[load_size], 0x76, 0x8000-load_size );
    }
  }


		/* main_ram … メイン RAM */

  if( verbose_proc ) printf("Allocating 64kB for main ram...");
  if( !(main_ram = allocate_romram( sizeof(byte)*0x10000 ) ) ){
    return(0);
  }



		/* main_high_ram … 高速 RAM(の裏) */

  if( verbose_proc ) printf("Allocating 4kB for High ram...");
  if( !( main_high_ram = allocate_romram( sizeof(byte)*0x1000 ) ) ){
    return(0);
  }


		/* kanji_rom … 漢字 ROM */

  if( verbose_proc ) printf("Allocating 256kB for kanji font rom...");
  if( !( kanji_rom
	   = (byte (*)[65536][2])allocate_romram( sizeof(byte)*2*65536*2 ) ) ){
    return(0);
  }

  if( verbose_proc ) printf("  Loading %s...",N88KNJ1_ROM_FILE);
  load_size = load_rom( N88KNJ1_ROM_FILE, kanji_rom[0][0], 0x20000 );
  if( load_size == 0 ){
    if( verbose_proc ) printf("  Loading %s...",N88KNJ1_ROM_FILE2);
    load_size = load_rom( N88KNJ1_ROM_FILE2, kanji_rom[0][0], 0x20000 );
  }
  if( load_size != 0x20000 ){
    memset( &kanji_rom[0][0][load_size], 0xff, 0x20000-load_size );
    menu_lang = LANG_ENGLISH;
  }
  else{
    exist_kanji_rom = TRUE;
  }

  if( verbose_proc ) printf("  Loading %s...",N88KNJ2_ROM_FILE);
  load_size = load_rom( N88KNJ2_ROM_FILE, kanji_rom[1][0], 0x20000 );
  if( load_size == 0 ){
    if( verbose_proc ) printf("  Loading %s...",N88KNJ2_ROM_FILE2);
    load_size = load_rom( N88KNJ2_ROM_FILE2, kanji_rom[1][0], 0x20000 );
  }
  if( load_size != 0x20000 ){
    memset( &kanji_rom[1][0][load_size], 0xff, 0x20000-load_size );
  }



		/* VRAM 用のメモリ確保 */

  if( !vram_allocate() ) return(0);


		/* テキストフォント用のメモリ確保 ＆ フォントロード */

  if( !font_allocate( FONT_FILE ) ) return (0);


		/* sub_romram … サブ ROM/RAM */

  if( verbose_proc ) printf("Allocating 32kB for SUB SYSTEM rom/ram...");
  if( !( sub_romram = allocate_romram( sizeof(byte)*0x8000 ) ) ){
    return(0);
  }
  if( !file_compatrom ){
    if( verbose_proc ) printf("  Loading %s...",N88SUB_ROM_FILE);
    load_size = load_rom( N88SUB_ROM_FILE, sub_romram, 0x2000 );
    if( load_size == 0 ){
      if( verbose_proc ) printf("  Loading %s...",N88SUB_ROM_FILE2);
      load_size = load_rom( N88SUB_ROM_FILE2, sub_romram, 0x2000 );
    }
    if( load_size <= 0x800 ){
      if( load_size < 0x800 ){
	memset( &sub_romram[load_size], 0x76, 0x800-load_size );
      }
      memcpy( &sub_romram[0x0800], &sub_romram[0x0000], 0x0800 );
      memcpy( &sub_romram[0x1000], &sub_romram[0x0000], 0x1000 );
    }else if( load_size < 0x2000 ){
      memset( &sub_romram[load_size], 0x76, 0x2000-load_size );
    }
    memcpy( &sub_romram[0x2000], &sub_romram[0x0000], 0x2000 );
  }
  memset( &sub_romram[0x4000], 0xff, 0x4000 );


		/* ext_ram   … 拡張 RAM  */
		/* dummy_rom … ダミーROM */
		/* dummy_ram … ダミーRAM */

  if( use_extram ){
    if( verbose_proc ) printf("Allocating %dkB for Ext ram...",128*use_extram);
    if( !( ext_ram = (byte (*)[0x8000])
	  	       allocate_romram( sizeof(byte)*0x8000*4*use_extram ) ) ){
      return(0);
    }
    memset( &ext_ram[0][0], 0xff, 0x8000*4*use_extram );
    if( verbose_proc ) printf("Allocating 32kB for DUMMY ROM...");
    if( !( dummy_rom = allocate_romram( sizeof(byte)*0x8000 ) ) ){
      return(0);
    }
    memset( &dummy_rom[0], 0xff, 0x8000 );
    if( verbose_proc ) printf("Allocating 32kB for DUMMY RAM...");
    if( !( dummy_ram = allocate_romram( sizeof(byte)*0x8000 ) ) ){
      return(0);
    }
  }


		/* jisho_rom … 辞書 ROM  */

  if( use_jisho_rom ){
    if( verbose_proc ) printf("Allocating 512kB for Jisho rom...");
    if( !( jisho_rom
	     =(byte (*)[0x4000])allocate_romram( sizeof(byte)*0x4000*32 ) ) ){
      return(0);
    }
    if( verbose_proc ) printf("  Loading %s...",N88JISHO_ROM_FILE);
    load_size = load_rom( N88JISHO_ROM_FILE, jisho_rom[0], 0x80000 );
    if( load_size == 0 ){
      if( verbose_proc ) printf("  Loading %s...",N88JISHO_ROM_FILE2);
      load_size = load_rom( N88JISHO_ROM_FILE2, jisho_rom[0], 0x80000 );
    }
    if( load_size != 0x80000 ){
      memset( &jisho_rom[0][load_size], 0x00, 0x80000-load_size );
    }
  }


		/* sound2_adpcm … ADPCM用 RAM  */

  if( sound_board==SOUND_II ){
    if( sound2_adpcm==NULL ){
      if( verbose_proc ) printf("Allocating 256kB for ADPCM ram...");
      if( !( sound2_adpcm = allocate_romram( sizeof(byte)*0x40000 ) ) ){
	return(0);
      }
    }
  }


		/* P88SR エミュのROMイメージを使う場合 */

  if( file_compatrom ){
    FILE *fp;
    int  success = FALSE, n_rom = FALSE;

    if( verbose_proc ) printf("Loading Compatible ROM IMAGE...");
    if( (fp=ex_fopen( file_compatrom, "rb" )) ){
      if( ex_fseek( fp,       0, SEEK_SET ) ) goto ERR;
      if(ex_fread( main_rom,        1, 0x8000, fp )!=0x8000 ) goto ERR;
      if( ex_fseek( fp, 0x0c000, SEEK_SET ) ) goto ERR;
      if(ex_fread( main_rom_ext[0], 1, 0x2000, fp )!=0x2000 ) goto ERR;
      if( ex_fseek( fp, 0x0e000, SEEK_SET ) ) goto ERR;
      if(ex_fread( main_rom_ext[1], 1, 0x2000, fp )!=0x2000 ) goto ERR;
      if( ex_fseek( fp, 0x10000, SEEK_SET ) ) goto ERR;
      if(ex_fread( main_rom_ext[2], 1, 0x2000, fp )!=0x2000 ) goto ERR;
      if( ex_fseek( fp, 0x12000, SEEK_SET ) ) goto ERR;
      if(ex_fread( main_rom_ext[3], 1, 0x2000, fp )!=0x2000 ) goto ERR;
      if( ex_fseek( fp, 0x08000, SEEK_SET ) ) goto ERR;
      if(ex_fread( &main_rom_n[0x6000], 1, 0x2000, fp )!=0x2000 ) goto ERR;
      if( ex_fseek( fp, 0x14000, SEEK_SET ) ) goto ERR;
      if(ex_fread( sub_romram,      1, 0x2000, fp )!=0x2000 ) goto ERR;
      memcpy( &sub_romram[0x2000], &sub_romram[0x0000], 0x2000 );
      success = TRUE;
      if( ex_fseek( fp, 0x16000, SEEK_SET ) ) goto ERR;
      if(ex_fread( main_rom_n,      1, 0x6000, fp )!=0x6000 ) goto ERR;
      n_rom = TRUE;
    ERR:
      ex_fclose(fp);
    }
    if( !success ){ if( verbose_proc ) printf("FAILED\n"); }
    else{
      if( n_rom ) { if( verbose_proc ) printf("OK\n"); }
      else        { if( verbose_proc ) printf("OK(Not exist N-BASIC ROM)\n");
		    memset( main_rom_n, 0x76, 0x6000 ); }
    }
  }

  return 1;
}


/************************************************************************/
/* 確保したメモリの解放							*/
/************************************************************************/
void	memory_free( void )
{
  ex_free( main_rom );
  ex_free( main_rom_ext );
  ex_free( main_rom_n );
  ex_free( main_ram );
  ex_free( main_high_ram );
  ex_free( kanji_rom );

  vram_free();
  font_free();

  ex_free( sub_romram );

  if( use_extram )    ex_free( ext_ram );
  if( use_jisho_rom ) ex_free( jisho_rom );
  if( sound_board==SOUND_II ) ex_free( sound2_adpcm );

}



/************************************************************************/
/* フォントをロードする関数						*/
/************************************************************************/
int	memory_load_font( char *filename )
{
  FILE *fp;
  size_t load_size = 0;

  if( use_system_font ){
    if( !(font_rom = (bit8*)ex_malloc( sizeof(byte)*8*256*2 ) ) ){
      font_rom = system_font;
      return 0;
    }
    use_system_font = FALSE;
  }

  if( ( fp = ex_fopen( filename, "rb" ) ) ){
    load_size =ex_fread( font_rom, sizeof(byte), 8*256*2, fp );
    ex_fclose(fp);
    if( load_size == 8*256*2 ){
      return 1;
    }
  }

  memcpy( font_rom, system_font, 8*256*2 );
  return 0;
}




/****************************************************************/	
/* サスペンド／レジューム					*/
/****************************************************************/	
static	byte	suspend_romversion;
static	int	suspend_adpcm;
static	T_SUSPEND_W	suspend_memory_work[]=
{
  { TYPE_INT,	&use_extram,	},
  { TYPE_INT,	&use_jisho_rom,	},
  { TYPE_BYTE,	&suspend_romversion, },
  { TYPE_INT,	&suspend_adpcm, },
};



int	suspend_memory( FILE *fp, long offset, long offset2 )
{
  int	ret;

  suspend_romversion = ROM_VERSION;
  if( sound_board==SOUND_II ) suspend_adpcm = TRUE;
  else                        suspend_adpcm = FALSE;

  ret = suspend_work( fp, offset, 
		      suspend_memory_work, 
		      countof(suspend_memory_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->memory ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }


  if( ex_fseek( fp, offset2, SEEK_SET ) == 0 ){
    if(ex_fwrite( main_ram,            sizeof(byte),  0x10000,  fp )
						!=  0x10000 ) return FALSE;
    if(ex_fwrite( main_high_ram,       sizeof(byte),   0x1000,  fp )
						!=   0x1000 ) return FALSE;
    if(ex_fwrite( &sub_romram[0x4000], sizeof(byte),   0x4000,  fp )
						!=   0x4000 ) return FALSE;
    if(ex_fwrite( main_vram,           sizeof(bit8), 4*0x4000,  fp )
						!= 4*0x4000 ) return FALSE;

    if( suspend_adpcm ){
      if(ex_fwrite( sound2_adpcm,      sizeof(bit8),  0x40000,  fp )
						!=  0x40000 ) return FALSE;
    }
    if( use_extram ){
      if(ex_fwrite( ext_ram,   sizeof(bit8), 0x8000*4*use_extram,  fp )
			       != (size_t)(0x8000*4*use_extram) ) return FALSE;
    }

  }else{
    return FALSE;
  }

  return TRUE;
}


int	resume_memory(  FILE *fp, long offset, long offset2 )
{
  int	ret;
  int	check_use_extram, check_suspend_adpcm, check_use_jisho_rom;

  check_use_extram    = use_extram;
  check_use_jisho_rom = use_jisho_rom;
  check_suspend_adpcm = ( sound_board==SOUND_II ) ? TRUE : FALSE;

  ret = resume_work( fp, offset, 
		     suspend_memory_work, 
		     countof(suspend_memory_work) );

  if( ret<0 ) return FALSE;

  if( check_use_extram != use_extram ){
    printf( "resume : need option -extram %d\n", use_extram );
    return FALSE;
  }
  if( check_use_jisho_rom != use_jisho_rom ){
    printf( "resume : need option %s\n", 
			((use_jisho_rom==NOT_JISHO)? "-nojisho" : "-jisho") );
    return FALSE;
  }
  if( check_suspend_adpcm != suspend_adpcm ){
    printf( "resume : need option %s\n", ((suspend_adpcm) ? "-sd2" : "-sd") );
    return FALSE;
  }      

  if( ex_fseek( fp, offset2, SEEK_SET ) == 0 ){
    if(ex_fread(  main_ram,            sizeof(byte),  0x10000,  fp )
						!=  0x10000 ) return FALSE;
    if(ex_fread(  main_high_ram,       sizeof(byte),   0x1000,  fp )
						!=   0x1000 ) return FALSE;
    if(ex_fread(  &sub_romram[0x4000], sizeof(byte),   0x4000,  fp )
						!=   0x4000 ) return FALSE;
    if(ex_fread(  main_vram,           sizeof(bit8), 4*0x4000,  fp )
						!= 4*0x4000 ) return FALSE;

    if( suspend_adpcm ){
      if(ex_fread( sound2_adpcm,       sizeof(bit8),  0x40000,  fp )
						!=  0x40000 ) return FALSE;
    }
    if( use_extram ){
      if(ex_fread( ext_ram,    sizeof(bit8), 0x8000*4*use_extram,  fp )
			       != (size_t)(0x8000*4*use_extram) ) return FALSE;
    }

  }else{
    return FALSE;
  }

  ROM_VERSION = suspend_romversion;
  return TRUE;
}



/*
 * レジュームは、サウンドの初期化およびメモリの確保の後に行われるのだが、
 * 余分なメモリ ( 拡張RAM・辞書ROM・サウンドボードII ) を使ってることが
 * わかると再度、サウンド初期化やメモリ確保をやらないといけない。
 *
 * なので、初期化前にサスペンド情報ファイルを覗き見て、余分なメモリを
 * 使っている場合は、予め設定ワークを書き換えてしまおう。
 *
 */

int	resume_memory_preparation(  FILE *fp, long offset )
{
  int	ret;
  int	save_use_extram    = use_extram;
  int	save_use_jisho_rom = use_jisho_rom;

	  /* use_extram と use_jisho_rom をサスペンド時の状態に復帰させる */

  ret = resume_work( fp, offset, 
		     suspend_memory_work, 
		     countof(suspend_memory_work) );

  if( ret<0 ){
    use_extram    = save_use_extram;
    use_jisho_rom = save_use_jisho_rom;
    return FALSE;
  }

	  /* sound_board をサスペンド時の状態に復帰させる */

  if( suspend_adpcm == FALSE ) sound_board = SOUND_I;
  else                         sound_board = SOUND_II;

  return TRUE;
}
