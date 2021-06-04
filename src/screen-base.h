



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
  SET_WORK( update_check_height,	/* screen_update走査ライン数 */	\
	    update_next_line    )	/* screen_updare次の行加算分 */	\
									\
	/* update_check_height は、1キャラの高さで、200line では 8 or 10 */\
	/* update_next_line    は、updateポインタを次の行に進める増分で、*/\
	/*                         200line では (crtc_font_height-1)*80  */\
	/* 400lineの場合は、16 or 20、(crtc_font_height*2-1)*80 になる   */\
									\
									\
									\
									\
  if( sys_ctrl & SYS_CTRL_80 ){	/* ======== 80桁 ===========================*/\
									\
    for( i=0; i<CRTC_SZ_LINES; i++ ){					\
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){				\
									\
	text = *text_attr ++;		/* テキストの文字・属性を取得 */\
	old  = *text_attr_old ++;					\
	renewal_flag = FALSE;						\
									\
									\
	if( text != old ){		/* テキストが、新旧不一致なら、更新 */\
	  renewal_flag = TRUE;						\
	  for( k=0; k<update_check_height; k++ )			\
	    update[ k*80 ] = 1;						\
									\
	}else{				/* 新旧一致なら screen_update[]検査 */\
									\
	  for( k=0; k<update_check_height; k++ )			\
	    if( update[ k*80 ] ){					\
	      renewal_flag = TRUE;  break;				\
	    }								\
	}								\
									\
									\
	if( renewal_flag ){		/* 結果、1文字分を更新するなら */\
									\
	  do_update = TRUE;						\
									\
	  set_font_style( text );					\
	  tcol = color_pixel[ get_font_color() ];			\
									\
	  for( k=0; k<update_check_height; k++ ){			\
									\
	    IF_400_ALTERNATE__FOLLOWING()				\
	      style = get_font_style();		/* フォント8dot取得 */	\
									\
	    if( update[ k*80 ] ){					\
	      if      ( style==0xff ){		/* TEXT部のみ表示 */	\
		MASK_8DOT()						\
	      }else if( style==0x00 ){		/* VRAM部のみ表示 */	\
		TRANS_8DOT()						\
	      }else{				/* TEXT/VRAM 合成 */	\
		STORE_8DOT()						\
	      }								\
	      COPY_8DOT()			/* ラインの隙間埋め */	\
	    }								\
	    DST_FORWARD()						\
									\
	  }								\
	  DST_BACKWARD()						\
									\
	}								\
						/* ワークを次の文字の位置へ */\
	SRC_NEXT_CHARA()						\
	DST_NEXT_CHARA()						\
	update ++;							\
      }									\
						/* ワークを次TEXT行の位置へ */\
      SRC_NEXT_LINE()							\
      DST_NEXT_LINE()							\
      update += (update_check_height-1)*80;				\
    }									\
									\
									\
									\
									\
  }else{			/* ======== 40桁 ===========================*/\
									\
    for( i=0; i<CRTC_SZ_LINES; i++ ){					\
      for( j=0; j<CRTC_SZ_COLUMNS; j+=2 ){				\
									\
	text = *text_attr,     text_attr += 2;				\
	old  = *text_attr_old, text_attr_old += 2;			\
	renewal_flag = FALSE;						\
									\
									\
	if( text != old ){		/* テキストが、新旧不一致なら、更新 */\
	  renewal_flag = TRUE;						\
	  for( k=0; k<update_check_height; k++ )			\
	    update[ k*80 ] = update[ k*80+1 ] = 1;			\
									\
	}else{				/* 新旧一致なら screen_update[]検査 */\
									\
	  for( k=0; k<update_check_height; k++ )			\
	    if( update[ k*80 ] || update[ k*80 +1 ] ){			\
	      renewal_flag = TRUE;  break;				\
	    }								\
	}								\
									\
									\
	if( renewal_flag ){		/* 結果、1文字分を更新するなら */\
									\
	  do_update = TRUE;						\
									\
	  set_font_style( text );					\
	  tcol = color_pixel[ get_font_color() ];			\
									\
	  for( k=0; k<update_check_height; k++ ){			\
									\
	    IF_400_ALTERNATE__FOLLOWING()				\
	      style = get_font_style();		/* フォント8dot取得 */	\
									\
	    if( update[ k*80 ] || update[ k*80+1 ] ){			\
	      if      ( style==0xff ){		/* TEXT部のみ表示 */	\
		MASK_16DOT()						\
	      }else if( style==0x00 ){		/* VRAM部のみ表示 */	\
		TRANS_16DOT()						\
	      }else{				/* TEXT/VRAM 合成 */	\
		STORE_16DOT()						\
	      }								\
	      COPY_16DOT()			/* ラインの隙間埋め */	\
	    }								\
	    DST_FORWARD()						\
									\
	  }								\
	  DST_BACKWARD()						\
									\
	}								\
						/* ワークを次の文字の位置へ */\
	SRC_NEXT_CHARA()						\
	DST_NEXT_CHARA()						\
	update ++;							\
	SRC_NEXT_CHARA()						\
	DST_NEXT_CHARA()						\
	update ++;							\
      }									\
						/* ワークを次TEXT行の位置へ */\
      SRC_NEXT_LINE()							\
      DST_NEXT_LINE()							\
      update += (update_check_height-1)*80;				\
    }									\
  }									\
}
