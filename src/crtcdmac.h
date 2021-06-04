#ifndef CRTCDMAC_H_INCLUDED
#define CRTCDMAC_H_INCLUDED



		/* CRTC�����猩���A�g���r���[�g	*/

#define MONO_SECRET	0x01
#define MONO_BLINK	0x02
#define MONO_REVERSE	0x04
#define MONO_UPPER	0x10
#define MONO_UNDER	0x20
#define MONO_GRAPH	0x80
#define COLOR_SWITCH	0x08
#define COLOR_GRAPH	0x10
#define COLOR_B		0x20
#define COLOR_R		0x40
#define COLOR_G		0x80

		/* �����\���Ŏg�p����A�g���r���[�g */

#define ATTR_REVERSE	0x01			/* ���]			*/
#define ATTR_SECRET	0x02			/* �\��/��\��		*/
#define ATTR_UPPER	0x04			/* �A�b�p�[���C��	*/
#define ATTR_LOWER	0x08			/* �A���_�[���C��	*/
#define ATTR_GRAPH	0x10			/* �O���t�B�b�N���[�h	*/
#define ATTR_B		0x20			/* �F Blue		*/
#define ATTR_R		0x40			/* �F Reg		*/
#define ATTR_G		0x80			/* �F Green		*/

#define MONO_MASK	0x0f
#define COLOR_MASK	0xe0



extern	int	crtc_active;		/* CRTC�̏�� 0:CRTC�쓮 1:CRTC��~ */
extern	int	crtc_intr_mask;		/* CRTC�̊����}�X�N ==3 �ŕ\��	    */
extern	int	crtc_cursor[2];		/* �J�[�\���ʒu ��\���̎���(-1,-1) */
extern	byte	crtc_format[5];		/* CRTC �������̃t�H�[�}�b�g	    */

extern	int	crtc_reverse_display;	/* �^�c���]�\�� / �U�c�ʏ�\��	*/
extern	int	crtc_skip_line;		/* �^�c1�s��΂��\�� / �U�c�ʏ� */
extern	int	crtc_cursor_style;	/* �u���b�N / �A���_���C��	*/
extern	int	crtc_cursor_blink;	/* �^�c�_�ł��� �U�c�_�ł��Ȃ�	*/
extern	int	crtc_attr_non_separate;	/* �^�cVRAM�AATTR �����݂ɕ���	*/
extern	int	crtc_attr_color;	/* �^�c�J���[ �U�c����		*/
extern	int	crtc_attr_non_special;	/* �U�c�s�̏I��� ATTR ������	*/

extern	int	CRTC_SZ_LINES;		/* �\�����錅�� (20/25)		*/
#define		CRTC_SZ_COLUMNS	(80)	/* �\������s�� (80�Œ�)	*/
extern	int	crtc_sz_lines;		/* ���� (20�`25)		*/
extern	int	crtc_sz_columns;	/* �s�� (2�`80)			*/
extern	int	crtc_sz_attrs;		/* ������ (1�`20)		*/
extern	int	crtc_byte_per_line;	/* 1�s������̃����� �o�C�g��	*/
extern	int	crtc_font_height;	/* �t�H���g�̍��� �h�b�g��(8/10)*/


extern	pair	dmac_address[4];
extern	pair	dmac_counter[4];
#define	text_dma_addr	dmac_address[2]

extern	int	dmac_mode;



	/**** �e�L�X�g�\�� ****/

enum {
  TEXT_DISABLE,		/* �e�L�X�g�\���Ȃ�				*/
  TEXT_ATTR_ONLY,	/*  �V    �A�������̂ݗL�� (�����O���t�B�b�N��)	*/
  TEXT_ENABLE,		/* �e�L�X�g�\������				*/
  End_of_TEXT
};
extern	int	text_display;		/* �e�L�X�g�\����ԃt���O	*/
extern	int	blink_cycle;		/* �_�ł̎���	8/16/24/32	*/
extern	int	blink_counter;		/* �_�Ő���J�E���^		*/


/* ���̃}�N���́ACRTC,DMAC�ݒ莞����сAI/O 31H / 53H �o�͎��ɌĂ�	*/

#define	set_text_display()						\
	do{								\
	  if( (dmac_mode & 0x4) && (crtc_active) && crtc_intr_mask==3){	\
	    if( !(grph_pile & GRPH_PILE_TEXT) ){			\
	      text_display = TEXT_ENABLE;				\
	    }else{							\
	      if( grph_ctrl & GRPH_CTRL_COLOR )				\
		text_display = TEXT_DISABLE;				\
	      else							\
		text_display = TEXT_ATTR_ONLY;				\
	    }								\
	  }else{							\
	    text_display = TEXT_DISABLE;				\
	  }								\
	}while(0)






void	crtc_init( void );

void	crtc_out_command( byte data );
void	crtc_out_parameter( byte data );
byte	crtc_in_status( void );
byte	crtc_in_parameter( void );


void	dmac_init( void );

void	dmac_out_mode( byte data );
byte	dmac_in_status( void );
void	dmac_out_address( byte addr, byte data );
void	dmac_out_counter( byte addr, byte data );
byte	dmac_in_address( byte addr );
byte	dmac_in_counter( byte addr );





#undef	SUPPORT_CRTC_SEND_SYNC_SIGNAL

#ifdef	SUPPORT_CRTC_SEND_SYNC_SIGNAL

void	crtc_send_sync_signal( int flag );
#define	set_crtc_sync_bit()	crtc_send_sync_signal( 1 )
#define	clr_crtc_sync_bit()	crtc_send_sync_signal( 0 )

#else

#define	set_crtc_sync_bit()	((void)0)
#define	clr_crtc_sync_bit()	((void)0)

#endif


#endif	/* CRTCDMAC_H_INCLUDED */
