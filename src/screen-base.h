



#define	vram2pixel( mode, size, type )					\
void	vram2pixel_##mode##_##size##_##type( void )			\
{									\
									\
  int	i, j, k, renewal_flag;						\
  Ushort *text_attr     = &text_attr_buf[ text_attr_flipflop   ][0];	\
  Ushort *text_attr_old = &text_attr_buf[ text_attr_flipflop^1 ][0];	\
  Ushort text, old;							\
  char	*update = &screen_update[0];					\
  int	update_check_height, update_next_line;				\
									\
  byte	style;								\
  Ulong	tcol;								\
									\
  SRC_DEFINE()								\
  DST_DEFINE()								\
  WORK_DEFINE()								\
									\
									\
  SET_WORK( update_check_height,	/* screen_update�������C���� */	\
	    update_next_line    )	/* screen_updare���̍s���Z�� */	\
									\
	/* update_check_height �́A1�L�����̍����ŁA200line �ł� 8 or 10 */\
	/* update_next_line    �́Aupdate�|�C���^�����̍s�ɐi�߂鑝���ŁA*/\
	/*                         200line �ł� (crtc_font_height-1)*80  */\
	/* 400line�̏ꍇ�́A16 or 20�A(crtc_font_height*2-1)*80 �ɂȂ�   */\
									\
									\
									\
									\
  if( sys_ctrl & SYS_CTRL_80 ){	/* ======== 80�� ===========================*/\
									\
    for( i=0; i<CRTC_SZ_LINES; i++ ){					\
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){				\
									\
	text = *text_attr ++;		/* �e�L�X�g�̕����E�������擾 */\
	old  = *text_attr_old ++;					\
	renewal_flag = FALSE;						\
									\
									\
	if( text != old ){		/* �e�L�X�g���A�V���s��v�Ȃ�A�X�V */\
	  renewal_flag = TRUE;						\
	  for( k=0; k<update_check_height; k++ )			\
	    update[ k*80 ] = 1;						\
									\
	}else{				/* �V����v�Ȃ� screen_update[]���� */\
									\
	  for( k=0; k<update_check_height; k++ )			\
	    if( update[ k*80 ] ){					\
	      renewal_flag = TRUE;  break;				\
	    }								\
	}								\
									\
									\
	if( renewal_flag ){		/* ���ʁA1���������X�V����Ȃ� */\
									\
	  do_update = TRUE;						\
									\
	  set_font_style( text );					\
	  tcol = color_pixel[ get_font_color() ];			\
									\
	  for( k=0; k<update_check_height; k++ ){			\
									\
	    IF_400_ALTERNATE__FOLLOWING()				\
	      style = get_font_style();		/* �t�H���g8dot�擾 */	\
									\
	    if( update[ k*80 ] ){					\
	      if      ( style==0xff ){		/* TEXT���̂ݕ\�� */	\
		MASK_8DOT()						\
	      }else if( style==0x00 ){		/* VRAM���̂ݕ\�� */	\
		TRANS_8DOT()						\
	      }else{				/* TEXT/VRAM ���� */	\
		STORE_8DOT()						\
	      }								\
	      COPY_8DOT()			/* ���C���̌��Ԗ��� */	\
	    }								\
	    DST_FORWARD()						\
									\
	  }								\
	  DST_BACKWARD()						\
									\
	}								\
						/* ���[�N�����̕����̈ʒu�� */\
	SRC_NEXT_CHARA()						\
	DST_NEXT_CHARA()						\
	update ++;							\
      }									\
						/* ���[�N����TEXT�s�̈ʒu�� */\
      SRC_NEXT_LINE()							\
      DST_NEXT_LINE()							\
      update += (update_check_height-1)*80;				\
    }									\
									\
									\
									\
									\
  }else{			/* ======== 40�� ===========================*/\
									\
    for( i=0; i<CRTC_SZ_LINES; i++ ){					\
      for( j=0; j<CRTC_SZ_COLUMNS; j+=2 ){				\
									\
	text = *text_attr,     text_attr += 2;				\
	old  = *text_attr_old, text_attr_old += 2;			\
	renewal_flag = FALSE;						\
									\
									\
	if( text != old ){		/* �e�L�X�g���A�V���s��v�Ȃ�A�X�V */\
	  renewal_flag = TRUE;						\
	  for( k=0; k<update_check_height; k++ )			\
	    update[ k*80 ] = update[ k*80+1 ] = 1;			\
									\
	}else{				/* �V����v�Ȃ� screen_update[]���� */\
									\
	  for( k=0; k<update_check_height; k++ )			\
	    if( update[ k*80 ] || update[ k*80 +1 ] ){			\
	      renewal_flag = TRUE;  break;				\
	    }								\
	}								\
									\
									\
	if( renewal_flag ){		/* ���ʁA1���������X�V����Ȃ� */\
									\
	  do_update = TRUE;						\
									\
	  set_font_style( text );					\
	  tcol = color_pixel[ get_font_color() ];			\
									\
	  for( k=0; k<update_check_height; k++ ){			\
									\
	    IF_400_ALTERNATE__FOLLOWING()				\
	      style = get_font_style();		/* �t�H���g8dot�擾 */	\
									\
	    if( update[ k*80 ] || update[ k*80+1 ] ){			\
	      if      ( style==0xff ){		/* TEXT���̂ݕ\�� */	\
		MASK_16DOT()						\
	      }else if( style==0x00 ){		/* VRAM���̂ݕ\�� */	\
		TRANS_16DOT()						\
	      }else{				/* TEXT/VRAM ���� */	\
		STORE_16DOT()						\
	      }								\
	      COPY_16DOT()			/* ���C���̌��Ԗ��� */	\
	    }								\
	    DST_FORWARD()						\
									\
	  }								\
	  DST_BACKWARD()						\
									\
	}								\
						/* ���[�N�����̕����̈ʒu�� */\
	SRC_NEXT_CHARA()						\
	DST_NEXT_CHARA()						\
	update ++;							\
	SRC_NEXT_CHARA()						\
	DST_NEXT_CHARA()						\
	update ++;							\
      }									\
						/* ���[�N����TEXT�s�̈ʒu�� */\
      SRC_NEXT_LINE()							\
      DST_NEXT_LINE()							\
      update += (update_check_height-1)*80;				\
    }									\
  }									\
}
