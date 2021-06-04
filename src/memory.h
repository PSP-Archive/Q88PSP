#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED


extern	int	use_extram;			/* 128KB拡張RAMを使う	*/
extern	int	use_jisho_rom;			/* 辞書ROMを使う	*/

extern	char	*file_compatrom;		/* P88SR emu のROMを使う*/



extern	byte	*main_rom;			/* メイン ROM (32KB)	*/
extern	byte	(*main_rom_ext)[0x2000];	/* 拡張 ROM   (8KB *4)	*/
extern	byte	*main_rom_n;			/* N-BASIC    (32KB)	*/
extern	byte	*main_ram;			/* メイン RAM (64KB)	*/
extern	byte	*main_high_ram;			/* 高速 RAM(の裏) (4KB)	*/
extern	byte	*sub_romram;			/* サブ ROM/RAM (32KB)	*/

extern	byte	(*kanji_rom)[65536][2];		/* 漢字ＲＯＭ (128KB*2)	*/

extern	byte	(*ext_ram)[0x8000];		/* 拡張 RAM   (32KB*4～)*/
extern	byte	(*jisho_rom)[0x4000];		/* 辞書 ROM   (16KB*32)	*/

extern	byte	(*main_vram)[4];		/* VRAM[0x4000][4]	*/
extern	byte	*font_rom;			/* フォントイメージROM	*/


				/* イリーガルな方法でメモリアクセスする	*/
#define	main_vram4 (bit32 *)main_vram		/* VRAM long word accrss*/

#define	ROM_VERSION	main_rom[0x79d7]



extern	byte	*dummy_rom;			/* ダミーROM (32KB)	*/
extern	byte	*dummy_ram;			/* ダミーRAM (32KB)	*/

extern	byte	*read_mem_0000_5fff;		/* メモリリードポインタ	*/
extern	byte	*read_mem_6000_7fff;
extern	byte	*read_mem_8000_83ff;
extern	byte	*read_mem_c000_efff;
extern	byte	*read_mem_f000_ffff;

extern	byte	*write_mem_0000_7fff;		/* メモリライトポインタ	*/
extern	byte	*write_mem_8000_83ff;
extern	byte	*write_mem_c000_efff;
extern	byte	*write_mem_f000_ffff;





byte	*allocate_romram( size_t romram_size );
int	memory_allocate( void );
void	memory_free( void );

int	memory_load_font( char *filename );

#endif	/* MEMORY_H_INCLUDED */
