/*
 *  ���z�I�� CPU �� RAM ��p�ӂ��ĂԂ�񂵂܂��B
 *
 *  ��̓I��
 *    �E���荞�ݏ����͖���
 *    �ERAM �̓��C�� RAM �̂�
 *    �E�|�[�g�����͕K�v�ȂƂ��낾������
 *  �Ȃǂō��������Ă��܂��B
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "z80.h"
#include "memory.h"
#include "screen.h"
#include "monitor.h"
#include "basic.h"

#define BASIC_MAX_ERR_NUM	4	/* �G���[�o�^�\��		*/
#define BASIC_MAX_ERR_STR	20	/* �G���[�\��������		*/

#define BASIC_MAX_LINE		256	/* ��s�̕����ő吔		*/
#define BASIC_MAX_LOOP		200000	/* ���z�v���Z�X�̍ő僋�[�v��	*/


/* �������ǂݍ��ݗp�}�N�� */
#define READ_BYTE(mem, addr)						\
	((mem)[addr] & 0xff)

#define READ_WORD(mem, addr)						\
	((READ_BYTE(mem, addr + 1) << 8) + READ_BYTE(mem, addr))

/* �������������ݗp�}�N�� */
#define WRITE_BYTE(mem, addr, data)					\
	(mem)[addr] = ((data) & 0xff)

#define WRITE_WORD(mem, addr, data)					\
	do {								\
	    WRITE_BYTE(mem, (addr) + 1, (data) >> 8);			\
	    WRITE_BYTE(mem, addr, data);				\
	} while (0)


/* ���W�X�^�������ݗp�}�N�� */
#define SET_REG(reg, data)						\
	((reg).W   = data,						\
	 (reg).B.h = ((data) >> 8) & 0xff,				\
	 (reg).B.l = (data) & 0xff)

/* �G���[�ݒ�p�}�N�� */
#define SET_ERR(err, epc, estr)						\
	do {								\
	    (err).pc = epc;						\
	    strncpy((err).str, estr, BASIC_MAX_ERR_STR);		\
	    (err).str[BASIC_MAX_ERR_STR - 1] = '\0';			\
	} while (0)

/* ���z�v���Z�X���G���[�\���� */
typedef struct {
    word pc;
    char str[BASIC_MAX_ERR_STR];
} basic_err;

int basic_mode = FALSE;

static z80arch pseudo_z80_cpu;			/* ���z CPU               */

static byte *pseudo_ram;			/* ���z������ (MAIN RAM)  */
static word pseudo_window_offset;		/* ���z�������p�E�C���h�E */

static byte *read_pseudo_mem_0000_7fff;		/* ���z���������[�h�|�C���^  */
static byte *read_pseudo_mem_8000_83ff;

static byte *write_pseudo_mem_8000_83ff;	/* ���z���������C�g�|�C���^  */

static word basic_top_addr_addr;		/* ���ԃR�[�h�n�_�A�h���X�i�[*/
static word basic_end_addr_addr;		/* ���ԃR�[�h�I�_�A�h���X�i�[*/
static word basic_top_addr;			/* ���ԃR�[�h�n�_�A�h���X    */
static word basic_end_addr;			/* ���ԃR�[�h�I�_�A�h���X    */
static word basic_buffer_addr;			/* ���ԃR�[�h�o�b�t�@�A�h���X*/
static word basic_buffer_size;			/* ���ԃR�[�h�o�b�t�@�T�C�Y  */
static word basic_sp;				/* �J�n���̃X�^�b�N�|�C���^  */

static word basic_conv_buffer_addr;		/* �ϊ��p�o�b�t�@�A�h���X */

static word encode_start_pc;			/* �G���R�[�h�J�n�A�h���X */
static word encode_end_pc;			/* �G���R�[�h�I���A�h���X */
static basic_err encode_err[BASIC_MAX_ERR_NUM];	/* �G���R�[�h�G���[       */
static int encode_err_num;			/* �G���R�[�h�G���[�o�^�� */

static word decode_start_pc;			/* �f�R�[�h�J�n�A�h���X   */
static word decode_end_pc;			/* �f�R�[�h�I���A�h���X   */
static basic_err decode_err[BASIC_MAX_ERR_NUM];	/* �f�R�[�h�G���[         */
static int decode_err_num;			/* �f�R�[�h�G���[�o�^��   */


/*------------------------------------------------------*/
/* ���z���������蓖��					*/
/*------------------------------------------------------*/
static void pseudo_memory_mapping(void)
{
    if (grph_ctrl & GRPH_CTRL_N){
	read_pseudo_mem_0000_7fff  = &main_rom_n[0x0000];
	read_pseudo_mem_8000_83ff  = &pseudo_ram[0x8000];
	write_pseudo_mem_8000_83ff = &pseudo_ram[0x8000];
    } else {
	read_pseudo_mem_0000_7fff  = &main_rom[0x0000];
	read_pseudo_mem_8000_83ff  = &pseudo_ram[pseudo_window_offset];
	write_pseudo_mem_8000_83ff = &pseudo_ram[pseudo_window_offset];
    }
}

/*------------------------------------------------------*/
/* ���z�������E���C�g					*/
/*------------------------------------------------------*/
static void pseudo_mem_write(word addr, byte data)
{
    if     (addr < 0x8000) pseudo_ram[addr] = data;
    else if(addr < 0x8400) write_pseudo_mem_8000_83ff[addr & 0x03ff] = data;
    else                   pseudo_ram[addr] = data;
}

/*------------------------------------------------------*/
/* ���z�������E���[�h					*/
/*------------------------------------------------------*/
static byte pseudo_mem_read(word addr)
{
    if      (addr < 0x8000) return read_pseudo_mem_0000_7fff[addr];
    else if (addr < 0x8400) return read_pseudo_mem_8000_83ff[addr & 0x03ff];
    else                    return pseudo_ram[addr];
}


/*------------------------------------------------------*/
/* ���z�|�[�g�E���C�g					*/
/*------------------------------------------------------*/
static void pseudo_io_out(byte port, byte data)
{
    if (port == 0x70) {
        pseudo_window_offset = (word)data << 8;
        pseudo_memory_mapping();
    }
}

/*------------------------------------------------------*/
/* ���z�|�[�g�E���[�h					*/
/*------------------------------------------------------*/
static byte pseudo_io_in(byte port)
{
    static byte port40_toggle = 0;

    switch (port) {
	/* N-mode decode */
    case 0x09:			/* fake */
	return 0xff;
    case 0x40:			/* fake */
	port40_toggle ^= 0x20;
	return port40_toggle;

	/* V1,V2-mode encode/decode */
    case 0x70:
	return pseudo_window_offset >> 8;
    default:
	return 0;
    }
}


/*------------------------------------------------------*/
/* ���z CPU ������					*/
/*------------------------------------------------------*/
static void pseudo_z80_init(void)
{
    z80_reset(&pseudo_z80_cpu);

    pseudo_z80_cpu.mem_read  = pseudo_mem_read;
    pseudo_z80_cpu.mem_write = pseudo_mem_write;
    pseudo_z80_cpu.io_read   = pseudo_io_in;
    pseudo_z80_cpu.io_write  = pseudo_io_out;
}

/*------------------------------------------------------*/
/* ���z������������					*/
/*------------------------------------------------------*/
static int pseudo_mem_init(void)
{
    if( verbose_proc ) printf("Allocating 64kB for pseudo ram...");
    if( !(pseudo_ram = allocate_romram( sizeof(byte)*0x10000 ) ) ){
	return(0);
    }
    memset(pseudo_ram, 0x00, 0x10000);
   
    pseudo_memory_mapping();


    if (grph_ctrl & GRPH_CTRL_N) {
	memset(&pseudo_ram[0xf160], 0xc9, 0xc0);
    } else {
	WRITE_BYTE(pseudo_ram, 0xe69d, 0x10);
	memset(&pseudo_ram[0xed00], 0xc9, 0x100);
    }
    return(1);
}

/*------------------------------------------------------*/
/* ���ԃR�[�h�̎n�_�E�I�_�A�h���X�̏�������		*/
/*------------------------------------------------------*/
static void write_basic_addr(void)
{
    WRITE_WORD(main_ram, basic_top_addr_addr, basic_top_addr);
    WRITE_WORD(main_ram, basic_end_addr_addr, basic_end_addr);
    if (grph_ctrl & GRPH_CTRL_N) {
	WRITE_WORD(main_ram, basic_end_addr_addr + 2, basic_end_addr);
	WRITE_WORD(main_ram, basic_end_addr_addr + 4, basic_end_addr);
    }
}

/*------------------------------------------------------*/
/* �G���R�[�h�E�f�R�[�h�p�A�h���X�ݒ�			*/
/*------------------------------------------------------*/
static void pseudo_set_addr(void)
{
    if (grph_ctrl & GRPH_CTRL_N) {
	/* N-mode */

	/* �A�h���X�ݒ� */
	basic_top_addr_addr    = 0xeb54;
	basic_end_addr_addr    = 0xefa0;
	basic_buffer_addr      = 0x8021;
	basic_buffer_size      = 0x4fff;
	/*basic_buffer_size      = 0x4014;*/
	basic_sp               = 0xe8d1;
	basic_conv_buffer_addr = 0xec96;

	/* �G���R�[�h�J�n�E�I���A�h���X�ݒ� */
	encode_start_pc        = 0x3cf2;
	encode_end_pc          = 0x3d6a;
	
	/* �G���R�[�h�G���[�o�^ */
	SET_ERR(encode_err[0], 0x3c9f, "no string");
	SET_ERR(encode_err[1], 0x423b, "no line number");
	SET_ERR(encode_err[2], 0x459e, "only line number");
	encode_err_num = 3;

	/* �f�R�[�h�J�n�E�I���A�h���X�ݒ� */
	decode_start_pc        = 0x5718;
	decode_end_pc          = 0x574e;

	/* �f�R�[�h�G���[�o�^ */
	SET_ERR(decode_err[0], 0x3c82, "no line number");
	SET_ERR(decode_err[1], 0x3c81, "buffer over flow");
	decode_err_num = 1;
    } else {
	/* V1,V2-mode */

	/* �A�h���X�ݒ� */
	basic_top_addr_addr    = 0xe658;
	basic_end_addr_addr    = 0xeb18;
	basic_buffer_addr      = 0x0001;
	basic_buffer_size      = 0x7fff;
	basic_sp               = 0xe5ff;
	basic_conv_buffer_addr = 0xe9b9;

	/* �G���R�[�h�J�n�E�I���A�h���X�ݒ� */
	encode_start_pc        = 0x04e2;
	encode_end_pc          = 0x05a8;

	/* �G���R�[�h�G���[�o�^ */
	SET_ERR(encode_err[0], 0x04a7, "no string");
	SET_ERR(encode_err[1], 0x4c70, "no line number");
	SET_ERR(encode_err[2], 0x0393, "line number is 0");
	SET_ERR(encode_err[3], 0x0c3c, "only line number");
	encode_err_num = 4;
	
	/* �f�R�[�h�J�n�E�I���A�h���X�ݒ� */
	decode_start_pc        = 0x18f1;
	decode_end_pc          = 0x1928;

	/* �f�R�[�h�G���[�o�^ */
	SET_ERR(decode_err[0], 0x047b, "no string");
	SET_ERR(decode_err[1], 0x047a, "buffer over flow");
	decode_err_num = 2;
    }
}


/*------------------------------------------------------*/
/* �G���R�[�h�p���z CPU ���W�X�^�ݒ�			*/
/*------------------------------------------------------*/
static void encode_z80_set_register(void)
{
    SET_REG(pseudo_z80_cpu.PC, encode_start_pc); /* start addr */
    SET_REG(pseudo_z80_cpu.HL, basic_conv_buffer_addr - 1);
    SET_REG(pseudo_z80_cpu.SP, basic_sp);
}


/*------------------------------------------------------*/
/* �G���R�[�h�p���z�������ݒ�				*/
/*------------------------------------------------------*/
static void encode_set_mem(void)
{
    /* �J�n�A�h���X�i�[ */
    WRITE_WORD(pseudo_ram, basic_top_addr_addr, basic_buffer_addr);
    /* �I���A�h���X�i�[ */
    WRITE_WORD(pseudo_ram, basic_end_addr_addr, basic_buffer_addr + 2);
}

/*------------------------------------------------------*/
/* �e�L�X�g���X�g���璆�ԃR�[�h�ɕϊ�			*/
/*------------------------------------------------------*/
int basic_encode_list(FILE *fp)
{
    char *ptr;
    char buf[BASIC_MAX_LINE];
    long loop;
    int i;
    int size;
    int text_line_num;

    pseudo_window_offset = 0;
    pseudo_set_addr();
    pseudo_z80_init();
    if (!pseudo_mem_init()) return(0);

    basic_mode = TRUE;
    size = 0;

    encode_set_mem();
    text_line_num = 0;
    while (fgets(buf, BASIC_MAX_LINE, fp) != NULL) {
	text_line_num++;
	/* ���s�R�[�h�폜 */
	if ((ptr = strchr(buf, '\r')) != NULL) *ptr = '\0';
	if ((ptr = strchr(buf, '\n')) != NULL) *ptr = '\0';

	/* �擪�������łȂ��Ȃ�Ƃ΂� */
	ptr = buf;
	while (*ptr == ' ') ptr++;
	if (!isdigit(*ptr)) continue;
	encode_z80_set_register();

	/* �ϊ��p�o�b�t�@�Ƀ��X�g���R�s�[ */
	strncpy(&pseudo_ram[basic_conv_buffer_addr], ptr, strlen(ptr));
	pseudo_ram[basic_conv_buffer_addr + strlen(ptr)] = 0x00; /* �I�[ */

	/* ���z�v���Z�X�J�n */
	for (loop = 0; loop < BASIC_MAX_LOOP; loop++) {
            z80_emu(&pseudo_z80_cpu, 1);
	    if (pseudo_z80_cpu.PC.W == encode_end_pc) break;
	    for (i = 0; i < encode_err_num; i++) {
		if (pseudo_z80_cpu.PC.W == encode_err[i].pc) {
		    printf("Error in line %d : %s.\n",
			   text_line_num, encode_err[i].str);
		    size = 0;
		    goto end_basic_encode_list;
		}
	    }
        }
        if (loop >= BASIC_MAX_LOOP) {
            printf("Error in line %d : failed to encode.\n", text_line_num);
            break;
        }
    }

    /* �G���R�[�h���ʂ����C�� RAM �ɃR�s�[ */
    basic_top_addr = READ_WORD(pseudo_ram, basic_top_addr_addr);
    basic_end_addr = READ_WORD(pseudo_ram, basic_end_addr_addr);
    if (basic_end_addr < basic_top_addr) {
	printf("Error : failed to encode.\n");
	return(0);
    }
    size = basic_end_addr - basic_top_addr + 1;
    memcpy(&main_ram[basic_top_addr], &pseudo_ram[basic_top_addr], size);
    write_basic_addr();

 end_basic_encode_list:
    basic_mode = FALSE;
    ex_free(pseudo_ram);

    return(size);
}



/*------------------------------------------------------*/
/* �f�R�[�h�p���z�������ݒ� 1				*/
/*------------------------------------------------------*/
static int decode_mem_set1(void)
{
    /* ���ԃR�[�h���R�s�[ */
    basic_top_addr = READ_WORD(main_ram, basic_top_addr_addr);
    basic_end_addr = READ_WORD(main_ram, basic_end_addr_addr);
    WRITE_WORD(pseudo_ram, basic_top_addr_addr, basic_top_addr);
    WRITE_WORD(pseudo_ram, basic_end_addr_addr, basic_end_addr);
    if (basic_end_addr < basic_top_addr) {
	printf("Error : no basic code.\n");
	return(FALSE);
    }
    memcpy(&pseudo_ram[basic_top_addr], &main_ram[basic_top_addr],
	   basic_end_addr - basic_top_addr + 1);

    if (grph_ctrl & GRPH_CTRL_N) {
	WRITE_BYTE(pseudo_ram, 0xea59, 0xff);
	WRITE_BYTE(pseudo_ram, 0xea65, 0x28); /* ���̍ő啶���� */
	WRITE_BYTE(pseudo_ram, 0xeb4a, 0x28); /* ���̍ő啶���� */
	WRITE_WORD(pseudo_ram, 0xef56, 0xef58);
	WRITE_WORD(pseudo_ram, 0xef79, 0xe9ff);
    } else {
	WRITE_BYTE(pseudo_ram, 0xe6a0, 0xff); /* �G���[�֌W�H */
	WRITE_BYTE(pseudo_ram, 0xe6a2, 0xff);
	WRITE_WORD(pseudo_ram, 0xe6c4, 0xf3c8);
	WRITE_WORD(pseudo_ram, 0xeace, 0xead0);
	WRITE_WORD(pseudo_ram, 0xeaf1, 0xe3fd);
	WRITE_BYTE(pseudo_ram, 0xef89, 0x50); /* ���̍ő啶���� */
    }
    return(TRUE);
}


/*------------------------------------------------------*/
/* �f�R�[�h�p���z�������ݒ� 2				*/
/*------------------------------------------------------*/
static void decode_mem_set2(void)
{
/*    pseudo_window_offset = 0;*/
/*    pseudo_memory_mapping();*/

    WRITE_WORD(pseudo_ram, basic_sp, 0xfffa); /* POP DE */

    if (grph_ctrl & GRPH_CTRL_N) {
	WRITE_WORD(pseudo_ram, 0xea63, 0x0101); /* �J�[�\���ʒu */
    } else {
	WRITE_WORD(pseudo_ram, 0xef86, 0x0101); /* �J�[�\���ʒu */
    }

}

/*------------------------------------------------------*/
/* �f�R�[�h�p���z CPU ���W�X�^�ݒ�			*/
/*------------------------------------------------------*/
static void decode_z80_set_register(word top_addr)
{
    SET_REG(pseudo_z80_cpu.PC, decode_start_pc); /* start addr */
    SET_REG(pseudo_z80_cpu.HL, top_addr);
    SET_REG(pseudo_z80_cpu.SP, basic_sp);
}

/*------------------------------------------------------*/
/* ���ԃR�[�h����e�L�X�g���X�g�֕ϊ�			*/
/*------------------------------------------------------*/
/* list �R�}���h�𗬗p */
int basic_decode_list(FILE *fp)
{
    char buf[BASIC_MAX_LINE];
    int i;
    int size;
    int text_line_num;
    long loop;
    word line_top_addr, line_end_addr;
    word line_num;


    pseudo_window_offset = 0;
    pseudo_set_addr();
    pseudo_z80_init();
    if (!pseudo_mem_init()) return(0);
 
    if (!decode_mem_set1()) goto end_basic_decode_list;

    basic_mode = TRUE;
    size = 0;

    line_top_addr = basic_top_addr;
    text_line_num = 0;
    while (line_top_addr < basic_end_addr) {
	text_line_num++;
	line_end_addr = READ_WORD(pseudo_ram, line_top_addr);
	if (line_end_addr == 0x0000) break;
	line_num = READ_WORD(pseudo_ram, line_top_addr + 2);

	decode_mem_set2();
	decode_z80_set_register(line_top_addr);

	/* ���z�v���Z�X�J�n */
	for (loop = 0; loop < BASIC_MAX_LOOP; loop++) {
            z80_emu(&pseudo_z80_cpu, 1);
	    if (pseudo_z80_cpu.PC.W == decode_end_pc) break;
	    for (i = 0; i < decode_err_num; i++) {
		if (pseudo_z80_cpu.PC.W == decode_err[i].pc) {
		    printf("Error in line %d : %s.\n",
			   text_line_num, decode_err[i].str);
		    size = 0;
		    goto end_basic_decode_list;
		}
	    }
        }
        if (loop >= BASIC_MAX_LOOP) {
            printf("Error in line %d : failed to decode.\n", text_line_num);
            break;
        }

	/* ���ʕ\�� */
	size += fprintf(fp, "%d ", line_num);
	for (i = 0; i < BASIC_MAX_LINE - 1; i++) {
	    if (READ_BYTE(pseudo_ram, basic_conv_buffer_addr + i) == 0x00) break;
	    buf[i] = READ_BYTE(pseudo_ram, basic_conv_buffer_addr + i);
	    size++;
	}
	buf[i] = '\0';
	print_hankaku(fp, buf, alt_char);
	fprintf(fp, "\r\n");
	size += 2;
	line_top_addr = line_end_addr;

    }

 end_basic_decode_list:

    basic_mode = FALSE;
    ex_free(pseudo_ram);

    return(size);
}


/*------------------------------------------------------*/
/* �t�@�C�����璆�ԃR�[�h�����C�� RAM �ɓǂݍ���	*/
/*------------------------------------------------------*/
int basic_load_intermediate_code(FILE *fp)
{
    int size;

    pseudo_set_addr();

    size =ex_fread(&main_ram[basic_buffer_addr - 1], 1, basic_buffer_size + 1, fp);
    basic_top_addr = basic_buffer_addr;
    basic_end_addr = basic_buffer_addr + size - 2;
    write_basic_addr();
    
    return(size);
}


/*------------------------------------------------------*/
/* ���C�� RAM ����t�@�C���ɒ��ԃR�[�h����������	*/
/*------------------------------------------------------*/
int basic_save_intermediate_code(FILE *fp)
{
    int size, wsize;

    pseudo_set_addr();

    basic_top_addr = READ_WORD(main_ram, basic_top_addr_addr);
    basic_end_addr = READ_WORD(main_ram, basic_end_addr_addr);
    if (basic_end_addr < basic_top_addr) {
	printf("Error : no basic code.\n");
	return(0);
    }
    /* p88make, p80make �ł��̂܂܎g���܂� */
    size = basic_end_addr - basic_top_addr + 2;
    wsize =ex_fwrite(&main_ram[basic_top_addr - 1], 1, size, fp);
    if (wsize < size) {
	printf("Error : basic size is %d.\n", size);
    }

    return(wsize);
}
