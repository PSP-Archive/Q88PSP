#undef	FONT_H
#undef	FONT_W
#undef	FONT_8x8
#undef	FONT_8x16
#undef	FONT_16x16
#undef	GET_FONT
#undef	PUT_FONT


/*----------------------------------------------------------------------*/

#if	defined( FULL )

#define	FONT_H	16
#define	FONT_W	8

#define	FONT_8x8()				\
	font_inc  = 1;				\
	font_dup  = TRUE;			\
	font_skip = 2;

#define	FONT_8x16()				\
	font_inc  = 1;				\
	font_dup  = FALSE;			\
	font_skip = 1;

#define	FONT_16x16()				\
	font_inc  = 2;				\
	font_dup  = FALSE;			\
	font_skip = 1;

#define	GET_FONT()	*font_ptr

#define	PUT_FONT()							\
	if( style & 0x80 ) dst[0] = fg;   else   dst[0] = bg; 		\
	if( style & 0x40 ) dst[1] = fg;   else   dst[1] = bg; 		\
	if( style & 0x20 ) dst[2] = fg;   else   dst[2] = bg; 		\
	if( style & 0x10 ) dst[3] = fg;   else   dst[3] = bg; 		\
	if( style & 0x08 ) dst[4] = fg;   else   dst[4] = bg; 		\
	if( style & 0x04 ) dst[5] = fg;   else   dst[5] = bg; 		\
	if( style & 0x02 ) dst[6] = fg;   else   dst[6] = bg; 		\
	if( style & 0x01 ) dst[7] = fg;   else   dst[7] = bg; 		\
	if( font_dup ){			/* 8x8フォント(縦に伸長)*/	\
	  memcpy( dst+WIDTH, dst, sizeof(TYPE)*8 );			\
	  dst += 2*WIDTH;						\
	}else{				/* 8x16フォント		*/	\
	  dst += WIDTH;							\
	}								

#elif	defined( HALF )

#define	FONT_H	8
#define	FONT_W	4

#define	FONT_8x8()				\
	font_inc  = 1; 				\
	font_dup  = FALSE;			\
	font_skip = 2;

#define	FONT_8x16()				\
	font_inc  = 2;				\
	font_dup  = FALSE;			\
	font_skip = 2;

#define	FONT_16x16()				\
	font_inc  = 4;				\
	font_dup  = FALSE;			\
	font_skip = 2;


#define	GET_FONT()	( *font_ptr | *(font_ptr+font_inc/2) )

#define	PUT_FONT()							\
	if( reverse ){							\
	  if((style & 0xc0)==0xc0 ) dst[0] = fg;   else   dst[0] = bg; 	\
	  if((style & 0x30)==0x30 ) dst[1] = fg;   else   dst[1] = bg; 	\
	  if((style & 0x0c)==0x0c ) dst[2] = fg;   else   dst[2] = bg; 	\
	  if((style & 0x03)==0x03 ) dst[3] = fg;   else   dst[3] = bg; 	\
	}else{								\
	  if( style & 0xc0 ) dst[0] = fg;   else   dst[0] = bg; 	\
	  if( style & 0x30 ) dst[1] = fg;   else   dst[1] = bg; 	\
	  if( style & 0x0c ) dst[2] = fg;   else   dst[2] = bg; 	\
	  if( style & 0x03 ) dst[3] = fg;   else   dst[3] = bg; 	\
	}								\
	dst += WIDTH;



#elif	defined( DOUBLE )


#define	FONT_H	32
#define	FONT_W	16

#define	FONT_8x8()				\
	font_inc  = 1; 				\
	font_dup  = TRUE;			\
	font_skip = 2;

#define	FONT_8x16()				\
	font_inc  = 1;				\
	font_dup  = FALSE;			\
	font_skip = 1;

#define	FONT_16x16()				\
	font_inc  = 2;				\
	font_dup  = FALSE;			\
	font_skip = 1;

#define	GET_FONT()	*font_ptr

#define	PUT_FONT()							\
	if( style & 0x80 ) dst[ 0]=dst[ 1]=fg;   else   dst[ 0]=dst[ 1]=bg;\
	if( style & 0x40 ) dst[ 2]=dst[ 3]=fg;   else   dst[ 2]=dst[ 3]=bg;\
	if( style & 0x20 ) dst[ 4]=dst[ 5]=fg;   else   dst[ 4]=dst[ 5]=bg;\
	if( style & 0x10 ) dst[ 6]=dst[ 7]=fg;   else   dst[ 6]=dst[ 7]=bg;\
	if( style & 0x08 ) dst[ 8]=dst[ 9]=fg;   else   dst[ 8]=dst[ 9]=bg;\
	if( style & 0x04 ) dst[10]=dst[11]=fg;   else   dst[10]=dst[11]=bg;\
	if( style & 0x02 ) dst[12]=dst[13]=fg;   else   dst[12]=dst[13]=bg;\
	if( style & 0x01 ) dst[14]=dst[15]=fg;   else   dst[14]=dst[15]=bg;\
	if( font_dup ){			/* 8x8フォント(縦に伸長)*/	\
	  memcpy( dst+  WIDTH, dst, sizeof(TYPE)*16 );			\
	  memcpy( dst+2*WIDTH, dst, sizeof(TYPE)*16 );			\
	  memcpy( dst+3*WIDTH, dst, sizeof(TYPE)*16 );			\
	  dst += 4*WIDTH;						\
	}else{				/* 8x16フォント		*/	\
	  memcpy( dst+WIDTH, dst, sizeof(TYPE)*16 );			\
	  dst += 2*WIDTH;						\
	}								

#endif




/*----------------------------------------------------------------------*/
/* メイン処理								*/
/*----------------------------------------------------------------------*/
#define	menu2pixel( size, type )					\
void	menu2pixel_##size##_##type( void )				\
{									\
  int	j, k, l, font_inc, font_dup, font_skip;				\
  byte	style, *font_ptr;						\
  Ulong	fg, bg;								\
  int	reverse, underline;						\
									\
  TYPE	*dst = (TYPE *)screen_start;					\
									\
  for( l=0; l<25; l++ ){						\
    for( k=0; k<80; k++ ){						\
									\
      reverse   = (menu_screen[l][k].reverse) ? 0xff : 0x00;		\
      underline = menu_screen[l][k].underline;				\
      fg        = color_pixel[ menu_screen[l][k].foreground ];		\
      bg        = color_pixel[ menu_screen[l][k].background ];		\
									\
      if( menu_screen[l][k].font_type==FONT_ANK ){    /*フォントROM*/	\
	  font_ptr = &font_rom[ menu_screen[l][k].addr ];		\
	  FONT_8x8()							\
      }else{								\
	font_ptr = &kanji_rom[0][ menu_screen[l][k].addr ][0];		\
	if      ( menu_screen[l][k].addr < 0x0800 ){  /*漢字ROM 半角*/	\
	  FONT_8x16()							\
	}else if( menu_screen[l][k].addr < 0x0c00 ){  /*漢字ROM 1/4角*/	\
	  FONT_8x8()							\
	}else{					      /*漢字ROM 全角*/	\
	  FONT_16x16()							\
	  if( menu_screen[l][k].font_type==FONT_HIGH ) font_ptr ++;	\
	}								\
      }									\
									\
      for( j=16; j; j-=font_skip ){					\
	style = GET_FONT() ^ reverse;					\
	if( j<=2 && underline ) style = 0xff;				\
									\
	PUT_FONT()							\
									\
	font_ptr += font_inc;						\
      }									\
      dst += -FONT_H * WIDTH + FONT_W;					\
    }									\
    dst += SCREEN_OFFSET + (FONT_H-1)*WIDTH;				\
  }									\
}
