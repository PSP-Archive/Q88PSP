#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED


extern	int	use_extram;			/* 128KB�g��RAM���g��	*/
extern	int	use_jisho_rom;			/* ����ROM���g��	*/

extern	char	*file_compatrom;		/* P88SR emu ��ROM���g��*/



extern	byte	*main_rom;			/* ���C�� ROM (32KB)	*/
extern	byte	(*main_rom_ext)[0x2000];	/* �g�� ROM   (8KB *4)	*/
extern	byte	*main_rom_n;			/* N-BASIC    (32KB)	*/
extern	byte	*main_ram;			/* ���C�� RAM (64KB)	*/
extern	byte	*main_high_ram;			/* ���� RAM(�̗�) (4KB)	*/
extern	byte	*sub_romram;			/* �T�u ROM/RAM (32KB)	*/

extern	byte	(*kanji_rom)[65536][2];		/* �����q�n�l (128KB*2)	*/

extern	byte	(*ext_ram)[0x8000];		/* �g�� RAM   (32KB*4�`)*/
extern	byte	(*jisho_rom)[0x4000];		/* ���� ROM   (16KB*32)	*/

extern	byte	(*main_vram)[4];		/* VRAM[0x4000][4]	*/
extern	byte	*font_rom;			/* �t�H���g�C���[�WROM	*/


				/* �C���[�K���ȕ��@�Ń������A�N�Z�X����	*/
#define	main_vram4 (bit32 *)main_vram		/* VRAM long word accrss*/

#define	ROM_VERSION	main_rom[0x79d7]



extern	byte	*dummy_rom;			/* �_�~�[ROM (32KB)	*/
extern	byte	*dummy_ram;			/* �_�~�[RAM (32KB)	*/

extern	byte	*read_mem_0000_5fff;		/* ���������[�h�|�C���^	*/
extern	byte	*read_mem_6000_7fff;
extern	byte	*read_mem_8000_83ff;
extern	byte	*read_mem_c000_efff;
extern	byte	*read_mem_f000_ffff;

extern	byte	*write_mem_0000_7fff;		/* ���������C�g�|�C���^	*/
extern	byte	*write_mem_8000_83ff;
extern	byte	*write_mem_c000_efff;
extern	byte	*write_mem_f000_ffff;





byte	*allocate_romram( size_t romram_size );
int	memory_allocate( void );
void	memory_free( void );

int	memory_load_font( char *filename );

#endif	/* MEMORY_H_INCLUDED */
