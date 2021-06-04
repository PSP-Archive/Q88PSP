#undef	vram2pixel

#undef	get_pixel_index
#undef	make_mask_mono
#undef	get_pixel_mono

#undef	SRC_DEFINE
#undef	DST_DEFINE
#undef	WORK_DEFINE

#undef	SET_WORK
#undef	IF_400_ALTERNATE__FOLLOWING

#undef	MASK_8DOT
#undef	MASK_16DOT
#undef	TRANS_8DOT
#undef	TRANS_16DOT
#undef	STORE_8DOT
#undef	STORE_16DOT
#undef	COPY_8DOT
#undef	COPY_16DOT
#undef	DST_FORWARD
#undef	DST_BACKWARD
#undef	SRC_NEXT_CHARA
#undef	DST_NEXT_CHARA
#undef	SRC_NEXT_LINE
#undef	DST_NEXT_LINE


/*----------------------------------------------------------------------*/
/* 88VRAMメモリパレット情報を、スクリーンバッファの色情報に変換		*/
/*----------------------------------------------------------------------*/

#ifdef LSB_FIRST
#define	get_pixel_index(data,x)					\
	(((data) & ((bit32)0x00000088>>(x)) ) >> (  3-(x) ) |	\
	 ((data) & ((bit32)0x00008800>>(x)) ) >> ( 10-(x) ) |	\
	 ((data) & ((bit32)0x00880000>>(x)) ) >> ( 17-(x) ) )
#else
#define	get_pixel_index(data,x)					\
	(((data) & ((bit32)0x88000000>>(x)) ) >> ( 27-(x) ) |	\
	 ((data) & ((bit32)0x00880000>>(x)) ) >> ( 18-(x) ) |	\
	 ((data) & ((bit32)0x00008800>>(x)) ) >> (  9-(x) ) )
#endif

/*----------------------------------------------------------------------*/

#ifdef LSB_FIRST
#define	make_mask_mono( mask )						\
	do{								\
	  mask = 0xffffffff;						\
	  if( grph_pile & GRPH_PILE_BLUE  ) mask &= 0x00ffff00;		\
	  if( grph_pile & GRPH_PILE_RED   ) mask &= 0x00ff00ff;		\
	  if( grph_pile & GRPH_PILE_GREEN ) mask &= 0x0000ffff;		\
	}while(0)
#define	get_pixel_mono( data, col )					\
		((data)&0x00808080) ? (col) : color_pixel[0]
#else
#define	make_mask_mono( mask )						\
	do{								\
	  mask = 0xffffffff;						\
	  if( grph_pile & GRPH_PILE_BLUE  ) mask &= 0x00ffff00;		\
	  if( grph_pile & GRPH_PILE_RED   ) mask &= 0xff00ff00;		\
	  if( grph_pile & GRPH_PILE_GREEN ) mask &= 0xffff0000;		\
	}while(0)
#define	get_pixel_mono( data, col )					\
		((data)&0x80808000) ? (col) : color_pixel[0]
#endif




/*----------------------------------------------------------------------*/
/* 88VRAMメモリのポインタを定義する					*/
/*		描画時は、main_vram、消去時は未定義			*/
/*----------------------------------------------------------------------*/
#if	defined( COLOR ) || defined( MONO )
#define	SRC_DEFINE()	bit32	*src = main_vram4;
#elif	defined( CLEAR )
#define	SRC_DEFINE()	bit32	*src = main_vram4; /*dummy*/
#endif

/*----------------------------------------------------------------------*/
/* スクリーンバッファのポインタを定義する				*/
/*		表示サイズによって、異なる。				*/
/*----------------------------------------------------------------------*/
#if	defined( FULL )
#define	DST_DEFINE()	TYPE	*dst  = (TYPE *)screen_start;		\
			TYPE	*dst2 = dst;
#elif	defined( HALF )
#define	DST_DEFINE()	TYPE	*dst  = (TYPE *)screen_start;

#elif	defined( DOUBLE )
#define	DST_DEFINE()	TYPE	*dst  = (TYPE *)screen_start;		\
			TYPE	*dst2 = dst  + WIDTH;			\
			TYPE	*dst3 = dst2 + WIDTH;			\
			TYPE	*dst4 = dst3 + WIDTH;
#endif


/*----------------------------------------------------------------------*/
/* その他のワークを定義する						*/
/*----------------------------------------------------------------------*/
#if	defined( COLOR )	/*===========================*/

#define	WORK_DEFINE()				\
  int	m;					\
  bit32	vram;  					\
  bit32	vcol[4];


#elif	defined( MONO )		/*===========================*/

#define	WORK_DEFINE()				\
  int	m, l;					\
  bit32	vram;					\
  bit32	mask;					\
  make_mask_mono( mask );

#elif	defined( CLEAR )	/*===========================*/

#define	WORK_DEFINE()				\
  int	m;					\
  Ulong	vcol;

#endif				/*===========================*/



/*----------------------------------------------------------------------*/
/* 200ライン描画共通定義						*/
/*----------------------------------------------------------------------*/
#define	SET_WORK( x, y )			\
  x = crtc_font_height;				\
  y = (crtc_font_height-1)*80;

#define	IF_400_ALTERNATE__FOLLOWING()	/* nothing */



/************************************************************************/
/* 200ライン描画							*/
/*		MASK_8DOT()	 ………  8ドット描画 (TEXT のみ）	*/
/*		MASK_16DOT()	 ……… 16ドット描画 (TEXT のみ）	*/
/*		TRANS_8DOT()	 ………  8ドット描画 (VRAM のみ）	*/
/*		TRANS_16DOT()	 ……… 16ドット描画 (VRAM のみ）	*/
/*		STORE_8DOT()	 ………  8ドット描画 (TEXT+VRAM）	*/
/*		STORE_16DOT()	 ……… 16ドット描画 (TEXT+VRAM）	*/
/*		COPY_8DOT()	 ………  8ドット複写 (隙間のライン埋め)	*/
/*		COPY_16DOT()	 ……… 16ドット複写 (隙間のライン埋め)	*/
/*		DST_FORWARD()    ……… 8(16)ドット描画後、次ラインへ	*/
/*		DST_BACKWARD()   ……… 1キャラ分描画後、元のラインへ	*/
/*		SRC_NEXT_CHARA() ……… 1キャラ分描画後、次の文字へ	*/
/*		DST_NEXT_CHARA() ……… 1キャラ分描画後、次の文字へ	*/
/*		SRC_NEXT_LINE()  ……… 桁最終キャラ描画後 次の文字行へ	*/
/*		DST_NEXT_LINE()  ……… 桁最終キャラ描画後 次の文字行へ	*/
/************************************************************************/
#if	defined( HALF )		/*-------------------------------------------*/

#define	MASK_8DOT()	for( m=0; m<4; m++ ) dst[m] = tcol;
#define	MASK_16DOT()	for( m=0; m<8; m++ ) dst[m] = tcol;

#define	COPY_8DOT()	/* nothing */
#define	COPY_16DOT()	/* nothing */

#if	defined( COLOR )	/*===========================*/

#if	defined( INTERP )
#define	TRANS_8DOT()							\
	      vram = *(src + k*80);					\
	      vcol[0] = get_pixel_index( vram, 0 );			\
	      vcol[1] = get_pixel_index( vram, 1 );			\
	      vcol[2] = get_pixel_index( vram, 2 );			\
	      vcol[3] = get_pixel_index( vram, 3 );			\
	      dst[0] = color_half_pixel[ vcol[0] >>4 ][ vcol[1] >>4 ];	\
	      dst[1] = color_half_pixel[ vcol[2] >>4 ][ vcol[3] >>4 ];	\
	      dst[2] = color_half_pixel[ vcol[0] & 7 ][ vcol[1] & 7 ];	\
	      dst[3] = color_half_pixel[ vcol[2] & 7 ][ vcol[3] & 7 ];
#else
#define	TRANS_8DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 2 );	\
	      dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      dst[1] = color_pixel[ vcol[1] >>4 ];	\
	      dst[2] = color_pixel[ vcol[0] & 7 ];	\
	      dst[3] = color_pixel[ vcol[1] & 7 ];
#endif

#if 	defined( INTERP )
#if 1		/* テキストにも色補間をかけてみる */
#define	STORE_8DOT()							     \
	      vram = *(src + k*80);					     \
	      vcol[0] = get_pixel_index( vram, 0 );			     \
	      vcol[1] = get_pixel_index( vram, 1 );			     \
	      vcol[2] = get_pixel_index( vram, 2 );			     \
	      vcol[3] = get_pixel_index( vram, 3 );			     \
	      {								     \
		int h,l;						     \
		if( style&0x80 ) h = get_font_color(); else h = vcol[0] >>4; \
		if( style&0x40 ) l = get_font_color(); else l = vcol[1] >>4; \
		dst[0] = color_half_pixel[ h ][ l ];			     \
		if( style&0x20 ) h = get_font_color(); else h = vcol[2] >>4; \
		if( style&0x10 ) l = get_font_color(); else l = vcol[3] >>4; \
		dst[1] = color_half_pixel[ h ][ l ];			     \
		if( style&0x08 ) h = get_font_color(); else h = vcol[0] & 7; \
		if( style&0x04 ) l = get_font_color(); else l = vcol[1] & 7; \
		dst[2] = color_half_pixel[ h ][ l ];			     \
		if( style&0x02 ) h = get_font_color(); else h = vcol[2] & 7; \
		if( style&0x01 ) l = get_font_color(); else l = vcol[3] & 7; \
		dst[3] = color_half_pixel[ h ][ l ];			     \
	      }
#else		/* floiさんのオリジナル。テキストは補間しない */
#define	STORE_8DOT()											\
	      vram = *(src + k*80);									\
	      vcol[0] = get_pixel_index( vram, 0 );							\
	      vcol[1] = get_pixel_index( vram, 1 );							\
	      vcol[2] = get_pixel_index( vram, 2 );							\
	      vcol[3] = get_pixel_index( vram, 3 );							\
              if( !(text & ATTR_REVERSE) ){								\
	        if( style & 0xc0 ) dst[0] = tcol;							\
	        else               dst[0] = color_half_pixel[ vcol[0] >>4 ][ vcol[1] >>4 ];		\
	        if( style & 0x30 ) dst[1] = tcol;							\
	        else               dst[1] = color_half_pixel[ vcol[2] >>4 ][ vcol[3] >>4 ];		\
	        if( style & 0x0c ) dst[2] = tcol;							\
	        else               dst[2] = color_half_pixel[ vcol[0] & 7 ][ vcol[1] & 7 ];		\
	        if( style & 0x03 ) dst[3] = tcol;							\
	        else               dst[3] = color_half_pixel[ vcol[2] & 7 ][ vcol[3] & 7 ];		\
	      }else{											\
	        if( (style & 0xc0) == 0xc0 ) dst[0] = tcol;						\
	        else                         dst[0] = color_half_pixel[ vcol[0] >>4 ][ vcol[1] >>4 ];	\
	        if( (style & 0x30) == 0x30 ) dst[1] = tcol;						\
	        else                         dst[1] = color_half_pixel[ vcol[2] >>4 ][ vcol[3] >>4 ];	\
	        if( (style & 0x0c) == 0x0c ) dst[2] = tcol;						\
	        else                         dst[2] = color_half_pixel[ vcol[0] & 7 ][ vcol[1] & 7 ];	\
	        if( (style & 0x03) == 0x03 ) dst[3] = tcol;						\
	        else                         dst[3] = color_half_pixel[ vcol[2] & 7 ][ vcol[3] & 7 ];	\
	      }
#endif
#else
#define	STORE_8DOT()							\
	      vram = *(src + k*80);					\
	      vcol[0] = get_pixel_index( vram, 0 );			\
	      vcol[1] = get_pixel_index( vram, 2 );			\
	      if( style & 0xc0 ) dst[0] = tcol;				\
	      else               dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      if( style & 0x30 ) dst[1] = tcol;				\
	      else               dst[1] = color_pixel[ vcol[1] >>4 ];	\
	      if( style & 0x0c ) dst[2] = tcol;				\
	      else               dst[2] = color_pixel[ vcol[0] & 7 ];	\
	      if( style & 0x03 ) dst[3] = tcol;				\
	      else               dst[3] = color_pixel[ vcol[1] & 7 ];
#endif


#if	defined( INTERP )
#define	TRANS_16DOT()							\
	      vram = *(src + k*80);					\
	      vcol[0] = get_pixel_index( vram, 0 );			\
	      vcol[1] = get_pixel_index( vram, 1 );			\
	      vcol[2] = get_pixel_index( vram, 2 );			\
	      vcol[3] = get_pixel_index( vram, 3 );			\
	      dst[0] = color_half_pixel[ vcol[0] >>4 ][ vcol[1] >>4 ];	\
	      dst[1] = color_half_pixel[ vcol[2] >>4 ][ vcol[3] >>4 ];	\
	      dst[2] = color_half_pixel[ vcol[0] & 7 ][ vcol[1] & 7 ];	\
	      dst[3] = color_half_pixel[ vcol[2] & 7 ][ vcol[3] & 7 ];	\
	      vram = *(src + k*80+1);					\
	      vcol[0] = get_pixel_index( vram, 0 );			\
	      vcol[1] = get_pixel_index( vram, 1 );			\
	      vcol[2] = get_pixel_index( vram, 2 );			\
	      vcol[3] = get_pixel_index( vram, 3 );			\
	      dst[4] = color_half_pixel[ vcol[0] >>4 ][ vcol[1] >>4 ];	\
	      dst[5] = color_half_pixel[ vcol[2] >>4 ][ vcol[3] >>4 ];	\
	      dst[6] = color_half_pixel[ vcol[0] & 7 ][ vcol[1] & 7 ];	\
	      dst[7] = color_half_pixel[ vcol[2] & 7 ][ vcol[3] & 7 ];
#else
#define	TRANS_16DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 2 );	\
	      vram = *(src + k*80+1);			\
	      vcol[2] = get_pixel_index( vram, 0 );	\
	      vcol[3] = get_pixel_index( vram, 2 );	\
	      dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      dst[1] = color_pixel[ vcol[1] >>4 ];	\
	      dst[2] = color_pixel[ vcol[0] & 7 ];	\
	      dst[3] = color_pixel[ vcol[1] & 7 ];	\
	      dst[4] = color_pixel[ vcol[2] >>4 ];	\
	      dst[5] = color_pixel[ vcol[3] >>4 ];	\
	      dst[6] = color_pixel[ vcol[2] & 7 ];	\
	      dst[7] = color_pixel[ vcol[3] & 7 ];
#endif

#if	defined( INTERP )
#define	STORE_16DOT()										\
	      vram = *(src + k*80);								\
	      vcol[0] = get_pixel_index( vram, 0 );						\
	      vcol[1] = get_pixel_index( vram, 1 );						\
	      vcol[2] = get_pixel_index( vram, 2 );						\
	      vcol[3] = get_pixel_index( vram, 3 );						\
	      if( style & 0x80 ) dst[0] = tcol;							\
	      else               dst[0] = color_half_pixel[ vcol[0] >>4 ][ vcol[1] >>4 ];	\
	      if( style & 0x40 ) dst[1] = tcol;							\
	      else               dst[1] = color_half_pixel[ vcol[2] >>4 ][ vcol[3] >>4 ];	\
	      if( style & 0x20 ) dst[2] = tcol;							\
	      else               dst[2] = color_half_pixel[ vcol[0] & 7 ][ vcol[1] & 7 ];	\
	      if( style & 0x10 ) dst[3] = tcol;							\
	      else               dst[3] = color_half_pixel[ vcol[2] & 7 ][ vcol[3] & 7 ];	\
	      vram = *(src + k*80+1);								\
	      vcol[0] = get_pixel_index( vram, 0 );						\
	      vcol[1] = get_pixel_index( vram, 1 );						\
	      vcol[2] = get_pixel_index( vram, 2 );						\
	      vcol[3] = get_pixel_index( vram, 3 );						\
	      if( style & 0x08 ) dst[4] = tcol;							\
	      else               dst[4] = color_half_pixel[ vcol[0] >>4 ][ vcol[1] >>4 ];	\
	      if( style & 0x04 ) dst[5] = tcol;							\
	      else               dst[5] = color_half_pixel[ vcol[2] >>4 ][ vcol[3] >>4 ];	\
	      if( style & 0x02 ) dst[6] = tcol;							\
	      else               dst[6] = color_half_pixel[ vcol[0] & 7 ][ vcol[1] & 7 ];	\
	      if( style & 0x01 ) dst[7] = tcol;							\
	      else               dst[7] = color_half_pixel[ vcol[2] & 7 ][ vcol[3] & 7 ];
#else
#define	STORE_16DOT()							\
	      vram = *(src + k*80);					\
	      vcol[0] = get_pixel_index( vram, 0 );			\
	      vcol[1] = get_pixel_index( vram, 2 );			\
	      vram = *(src + k*80+1);					\
	      vcol[2] = get_pixel_index( vram, 0 );			\
	      vcol[3] = get_pixel_index( vram, 2 );			\
	      if( style & 0x80 ) dst[0] = tcol;				\
	      else               dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      if( style & 0x40 ) dst[1] = tcol;				\
	      else               dst[1] = color_pixel[ vcol[1] >>4 ];	\
	      if( style & 0x20 ) dst[2] = tcol;				\
	      else               dst[2] = color_pixel[ vcol[0] & 7 ];	\
	      if( style & 0x10 ) dst[3] = tcol;				\
	      else               dst[3] = color_pixel[ vcol[1] & 7 ];	\
	      if( style & 0x08 ) dst[4] = tcol;				\
	      else               dst[4] = color_pixel[ vcol[2] >>4 ];	\
	      if( style & 0x04 ) dst[5] = tcol;				\
	      else               dst[5] = color_pixel[ vcol[3] >>4 ];	\
	      if( style & 0x02 ) dst[6] = tcol;				\
	      else               dst[6] = color_pixel[ vcol[2] & 7 ];	\
	      if( style & 0x01 ) dst[7] = tcol;				\
	      else               dst[7] = color_pixel[ vcol[3] & 7 ];
#endif

#elif	defined( MONO )		/*===========================*/

#define	TRANS_8DOT()					\
	      vram  = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<4; l++, vram<<=2 ){		\
		dst[l] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_8DOT()							\
	      vram  = *(src + k*80);					\
	      vram &= mask;						\
	      for( m=0xc0, l=0; l<4; l++, m>>=2, vram<<=2 ){		\
		if( style & m ) dst[l] = tcol;				\
		else            dst[l] = get_pixel_mono( vram, tcol );	\
	      }

#define	TRANS_16DOT()					\
	      vram = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<4; l++, vram<<=2 ){		\
		dst[l] = get_pixel_mono( vram, tcol );	\
	      }						\
	      vram = *(src + k*80+1);			\
	      vram &= mask;				\
	      for(    ; l<8; l++, vram<<=2 ){		\
		dst[l] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_16DOT()							\
	      vram = *(src + k*80);					\
	      vram &= mask;						\
	      for( m=0x80, l=0; l<4; l++, m>>=1, vram<<=2 ){		\
		if( style & m ) dst[l] = tcol;				\
		else            dst[l] = get_pixel_mono( vram, tcol );	\
	      }								\
	      vram = *(src + k*80+1);					\
	      vram &= mask;						\
	      for(            ; l<8; l++, m>>=1, vram<<=2 ){		\
		if( style & m ) dst[l] = tcol;				\
		else            dst[l] = get_pixel_mono( vram, tcol );	\
	      }

#elif	defined( CLEAR )	/*===========================*/

#define	TRANS_8DOT()							\
	      vcol = black;						\
	      for( m=0; m<4; m++ ) dst[m] = vcol;

#if 	defined( INTERP )
#define	STORE_8DOT()							 \
	      {								 \
	        int black_x = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;	 \
		int h,l;						 \
		if( style&0x80 ) h = get_font_color(); else h = black_x; \
		if( style&0x40 ) l = get_font_color(); else l = black_x; \
		dst[0] = color_half_pixel[ h ][ l ];			 \
		if( style&0x20 ) h = get_font_color(); else h = black_x; \
		if( style&0x10 ) l = get_font_color(); else l = black_x; \
		dst[1] = color_half_pixel[ h ][ l ];			 \
		if( style&0x08 ) h = get_font_color(); else h = black_x; \
		if( style&0x04 ) l = get_font_color(); else l = black_x; \
		dst[2] = color_half_pixel[ h ][ l ];			 \
		if( style&0x02 ) h = get_font_color(); else h = black_x; \
		if( style&0x01 ) l = get_font_color(); else l = black_x; \
		dst[3] = color_half_pixel[ h ][ l ];			 \
	      }
#else
#define	STORE_8DOT()							\
	      vcol = black;						\
	      if( style & 0xc0 ) dst[0] = tcol;  else dst[0] = vcol;	\
	      if( style & 0x30 ) dst[1] = tcol;  else dst[1] = vcol;	\
	      if( style & 0x0c ) dst[2] = tcol;  else dst[2] = vcol;	\
	      if( style & 0x03 ) dst[3] = tcol;  else dst[3] = vcol;
#endif

#define	TRANS_16DOT()							\
	      vcol = black;						\
	      for( m=0; m<8; m++ ) dst[m] = vcol;

#define	STORE_16DOT()							\
	      vcol = black;						\
	      if( style & 0x80 ) dst[0] = tcol;  else dst[0] = vcol;	\
	      if( style & 0x40 ) dst[1] = tcol;  else dst[1] = vcol;	\
	      if( style & 0x20 ) dst[2] = tcol;  else dst[2] = vcol;	\
	      if( style & 0x10 ) dst[3] = tcol;  else dst[3] = vcol;	\
	      if( style & 0x08 ) dst[4] = tcol;  else dst[4] = vcol;	\
	      if( style & 0x04 ) dst[5] = tcol;  else dst[5] = vcol;	\
	      if( style & 0x02 ) dst[6] = tcol;  else dst[6] = vcol;	\
	      if( style & 0x01 ) dst[7] = tcol;  else dst[7] = vcol;

#endif				/*===========================*/

#define	DST_FORWARD()				\
	    dst += WIDTH;

#define	DST_BACKWARD()				\
	  dst  += -crtc_font_height*WIDTH;


#define	SRC_NEXT_CHARA()			\
	src ++;

#define	DST_NEXT_CHARA()			\
	dst += 4;


#define	SRC_NEXT_LINE()				\
      src  += (crtc_font_height-1)*80;

#define	DST_NEXT_LINE()							\
      dst  += SCREEN_OFFSET + (crtc_font_height-1)*WIDTH;


#elif	defined( FULL )		/*-------------------------------------------*/

#if	!defined( INTERLACE )
#define	MASK_8DOT()	for( m=0; m< 8; m++ ) if(narabi8dot[m] <= 5) dst[narabi8dot[m]] = tcol;
#define	MASK_16DOT()	for( m=0; m<16; m++ ) if(narabi8dot[m] <= 11) dst[narabi8dot[m]] = tcol;
#else
#define	MASK_8DOT()	for( m=0; m< 8; m++ ) if(narabi8dot[m] <= 5) dst[narabi8dot[m]] = dst2[narabi8dot[m]] = tcol;
#define	MASK_16DOT()	for( m=0; m<16; m++ ) if(narabi8dot[m] <= 11) dst[narabi8dot[m]] = dst2[narabi8dot[m]] = tcol;
#endif

#if	defined( INTERLACE )
#if	0
#define	COPY_8DOT()	memset( dst2, black, sizeof(TYPE)*8 );
#define	COPY_16DOT()	memset( dst2, black, sizeof(TYPE)*16 );
#else
#define	COPY_8DOT()
#define	COPY_16DOT()
#endif
#else
#define	COPY_8DOT()	memcpy( dst2, dst, sizeof(TYPE)*6 );
#define	COPY_16DOT()	memcpy( dst2, dst, sizeof(TYPE)*12 );
#endif

#if	!defined( INTERLACE )
#if	defined( COLOR )	/*===========================*/

#define	TRANS_8DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      dst[1] = color_pixel[ vcol[2] >>4 ];	\
	      dst[2] = color_pixel[ vcol[3] >>4 ];	\
	      dst[3] = color_pixel[ vcol[0] & 7 ];	\
	      dst[4] = color_pixel[ vcol[2] & 7 ];	\
	      dst[5] = color_pixel[ vcol[3] & 7 ];

#define	STORE_8DOT()							\
	      vram = *(src + k*80);					\
	      vcol[0] = get_pixel_index( vram, 0 );			\
	      vcol[1] = get_pixel_index( vram, 1 );			\
	      vcol[2] = get_pixel_index( vram, 2 );			\
	      vcol[3] = get_pixel_index( vram, 3 );			\
	      if( style & 0x80 ) dst[0] = tcol;				\
	      else               dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      if( style & 0x20 ) dst[1] = tcol;				\
	      else               dst[1] = color_pixel[ vcol[2] >>4 ];	\
	      if( style & 0x10 ) dst[2] = tcol;				\
	      else               dst[2] = color_pixel[ vcol[3] >>4 ];	\
	      if( style & 0x08 ) dst[3] = tcol;				\
	      else               dst[3] = color_pixel[ vcol[0] & 7 ];	\
	      if( style & 0x02 ) dst[4] = tcol;				\
	      else               dst[4] = color_pixel[ vcol[2] & 7 ];	\
	      if( style & 0x01 ) dst[5] = tcol;				\
	      else               dst[5] = color_pixel[ vcol[3] & 7 ];

#define	TRANS_16DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      dst[1] = color_pixel[ vcol[2] >>4 ];	\
	      dst[2] = color_pixel[ vcol[3] >>4 ];	\
	      dst[3] = color_pixel[ vcol[0] & 7 ];	\
	      dst[4] = color_pixel[ vcol[2] & 7 ];	\
	      dst[5] = color_pixel[ vcol[3] & 7 ];	\
	      vram = *(src + k*80+1);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[ 6] = color_pixel[ vcol[0] >>4 ];	\
	      dst[ 7] = color_pixel[ vcol[2] >>4 ];	\
	      dst[ 8] = color_pixel[ vcol[3] >>4 ];	\
	      dst[ 9] = color_pixel[ vcol[0] & 7 ];	\
	      dst[10] = color_pixel[ vcol[2] & 7 ];	\
	      dst[11] = color_pixel[ vcol[3] & 7 ];

#define	STORE_16DOT()							  \
	      vram = *(src + k*80);					  \
	      vcol[0] = get_pixel_index( vram, 0 );			  \
	      vcol[1] = get_pixel_index( vram, 1 );			  \
	      vcol[2] = get_pixel_index( vram, 2 );			  \
	      vcol[3] = get_pixel_index( vram, 3 );			  \
	      if( style & 0x80 ){ dst[0] = dst[1] = tcol; }		  \
	      else              { dst[0] = color_pixel[ vcol[0] >>4 ];	  \
	      }\
	      if( style & 0x40 ){ dst[1] = dst[2] = tcol; }		  \
	      else              { dst[1] = color_pixel[ vcol[2] >>4 ];	  \
				  dst[2] = color_pixel[ vcol[3] >>4 ]; }  \
	      if( style & 0x20 ){ dst[3] = dst[ 4] = tcol; }		  \
	      else              { dst[3] = color_pixel[ vcol[0] & 7 ];	  \
	      }\
	      if( style & 0x10 ){ dst[4] = dst[5] = tcol; }		  \
	      else              { dst[4] = color_pixel[ vcol[2] & 7 ];	  \
	      			  dst[5] = color_pixel[ vcol[3] & 7 ]; }  \
	      vram = *(src + k*80+1);					  \
	      vcol[0] = get_pixel_index( vram, 0 );			  \
	      vcol[1] = get_pixel_index( vram, 1 );			  \
	      vcol[2] = get_pixel_index( vram, 2 );			  \
	      vcol[3] = get_pixel_index( vram, 3 );			  \
	      if( style & 0x08 ){ dst[ 6] = dst[ 7] = tcol; }		  \
	      else              { dst[ 6] = color_pixel[ vcol[0] >>4 ];	  \
	      }\
	      if( style & 0x04 ){ dst[ 7] = dst[ 8] = tcol; }		  \
	      else              { dst[ 7] = color_pixel[ vcol[2] >>4 ];	  \
				  dst[ 8] = color_pixel[ vcol[3] >>4 ]; } \
	      if( style & 0x02 ){ dst[ 9] = dst[10] = tcol; }		  \
	      else              { dst[ 9] = color_pixel[ vcol[0] & 7 ];	  \
	      }\
	      if( style & 0x01 ){ dst[10] = dst[11] = tcol; }		  \
	      else              { dst[10] = color_pixel[ vcol[2] & 7 ];	  \
	      			  dst[11] = color_pixel[ vcol[3] & 7 ]; }


#elif	defined( MONO )		/*===========================*/

#define	TRANS_8DOT()					\
	      vram  = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<8; l++, vram<<=1 ){		\
			if(narabi8dot[l] <= 5)  dst[narabi8dot[l]] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_8DOT()							\
	      vram  = *(src + k*80);					\
	      vram &= mask;						\
	      for( m=0x80, l=0; l<8; l++, m>>=1, vram<<=1 ){		\
			if(narabi8dot[l] <= 5) {							\
				if( style & m ) dst[narabi8dot[l]] = tcol;				\
				else            dst[narabi8dot[l]] = get_pixel_mono( vram, tcol );	\
				}																\
	      }

#define	TRANS_16DOT()					\
	      vram = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<8; l++, vram<<=1 ){		\
			if(narabi16dot[l] <= 11) dst[narabi16dot[l]] = get_pixel_mono( vram, tcol );	\
	      }						\
	      vram = *(src + k*80+1);			\
	      vram &= mask;				\
	      for(    ; l<16; l++, vram<<=1 ){		\
			if(narabi16dot[l] <= 11) dst[narabi16dot[l]] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_16DOT()							      \
	      vram = *(src + k*80);					      \
	      vram &= mask;						      \
	      for( m=0x80, l=0; l<8; l+=2, m>>=1, vram<<=2 ){		      \
	      	if(narabi16dot[l] <= 11) {\
				if( style & m ){ dst[narabi16dot[l]]   = dst[narabi16dot[l+1]] = tcol; }		      \
				else           { dst[narabi16dot[l]]   = get_pixel_mono( vram,    tcol );  \
				 dst[narabi16dot[l+1]] = get_pixel_mono( vram<<1, tcol ); }\
		      }								      \
		    }\
	      vram = *(src + k*80+1);					      \
	      vram &= mask;						      \
	      for(            ; l<16; l+=2, m>>=1, vram<<=2 ){		      \
	      if(narabi16dot[l] <= 11) {\
			if( style & m ){ dst[narabi16dot[l]]   = dst[narabi16dot[l+1]] = tcol; }		      \
			else           { dst[narabi16dot[l]]   = get_pixel_mono( vram,    tcol );  \
					 dst[narabi16dot[l+1]] = get_pixel_mono( vram<<1, tcol ); }\
		      }\
		  }

#elif	defined( CLEAR )	/*===========================*/

#define	TRANS_8DOT()							\
	      vcol = black;						\
	      for( m=0; m<8; m++ ){			\
	       if(narabi8dot[m] <= 5) dst[narabi8dot[m]] = vcol;\
	      }

#define	STORE_8DOT()							\
	      vcol = black;						\
	      if( style & 0x80 ) dst[0] = tcol;  else dst[0] = vcol;	\
	      if( style & 0x20 ) dst[1] = tcol;  else dst[1] = vcol;	\
	      if( style & 0x10 ) dst[2] = tcol;  else dst[2] = vcol;	\
	      if( style & 0x08 ) dst[3] = tcol;  else dst[3] = vcol;	\
	      if( style & 0x02 ) dst[4] = tcol;  else dst[4] = vcol;	\
	      if( style & 0x01 ) dst[5] = tcol;  else dst[5] = vcol;

#define	TRANS_16DOT()							\
	      vcol = black;						\
	      for( m=0; m<16; m++ ){			\
	       if(narabi16dot[m] <= 11) dst[narabi16dot[m]] = vcol;\
	      }

#define	STORE_16DOT()							\
	      vcol = black;						\
	      if( style & 0x80 ) dst[ 0] = dst[1] = tcol;		\
	      else               dst[ 0] = dst[1] = vcol;		\
	      if( style & 0x40 ) dst[ 1] = dst[ 2] = tcol;		\
	      else               dst[ 1] = dst[ 2] = vcol;		\
	      if( style & 0x20 ) dst[ 3] = dst[ 4] = tcol;		\
	      else               dst[ 3] = dst[ 4] = vcol;		\
	      if( style & 0x10 ) dst[ 4] = dst[ 5] = tcol;		\
	      else               dst[ 4] = dst[ 5] = vcol;		\
	      if( style & 0x08 ) dst[ 6] = dst[ 7] = tcol;		\
	      else               dst[ 6] = dst[ 7] = vcol;		\
	      if( style & 0x04 ) dst[ 7] = dst[ 8] = tcol;		\
	      else               dst[ 7] = dst[ 8] = vcol;		\
	      if( style & 0x02 ) dst[ 9] = dst[10] = tcol;		\
	      else               dst[ 9] = dst[10] = vcol;		\
	      if( style & 0x01 ) dst[10] = dst[11] = tcol;		\
	      else               dst[10] = dst[11] = vcol;


#endif				/*===========================*/
#else
#if	defined( COLOR )	/*===========================*/

#define	TRANS_8DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      dst[1] = color_pixel[ vcol[1] >>4 ];	\
	      dst[2] = color_pixel[ vcol[2] >>4 ];	\
	      dst[3] = color_pixel[ vcol[3] >>4 ];	\
	      dst[4] = color_pixel[ vcol[0] & 7 ];	\
	      dst[5] = color_pixel[ vcol[1] & 7 ];	\
	      dst[6] = color_pixel[ vcol[2] & 7 ];	\
	      dst[7] = color_pixel[ vcol[3] & 7 ];	\
	      memset( dst2, black, sizeof(TYPE)*8 );


#define	STORE_8DOT()							\
	      vram = *(src + k*80);					\
	      vcol[0] = get_pixel_index( vram, 0 );			\
	      vcol[1] = get_pixel_index( vram, 1 );			\
	      vcol[2] = get_pixel_index( vram, 2 );			\
	      vcol[3] = get_pixel_index( vram, 3 );			\
	      if( style & 0x80 ){ dst[0]  = dst2[0] = tcol; }		\
	      else              { dst[0]  = color_pixel[ vcol[0] >>4 ];	\
	      			  dst2[0] = black; }			\
	      if( style & 0x40 ){ dst[1]  = dst2[1] = tcol; }		\
	      else              { dst[1]  = color_pixel[ vcol[1] >>4 ];	\
	      			  dst2[1] = black; }			\
	      if( style & 0x20 ){ dst[2]  = dst2[2] = tcol; }		\
	      else              { dst[2]  = color_pixel[ vcol[2] >>4 ];	\
	      			  dst2[2] = black; }			\
	      if( style & 0x10 ){ dst[3]  = dst2[3] = tcol; }		\
	      else              { dst[3]  = color_pixel[ vcol[3] >>4 ];	\
	      			  dst2[3] = black; }			\
	      if( style & 0x08 ){ dst[4]  = dst2[4] = tcol; }		\
	      else              { dst[4]  = color_pixel[ vcol[0] & 7 ];	\
	      			  dst2[4] = black; }			\
	      if( style & 0x04 ){ dst[5]  = dst2[5] = tcol; }		\
	      else              { dst[5]  = color_pixel[ vcol[1] & 7 ];	\
	      			  dst2[5] = black; }			\
	      if( style & 0x02 ){ dst[6]  = dst2[6] = tcol; }		\
	      else              { dst[6]  = color_pixel[ vcol[2] & 7 ];	\
	      			  dst2[6] = black; }			\
	      if( style & 0x01 ){ dst[7]  = dst2[7] = tcol; }		\
	      else              { dst[7]  = color_pixel[ vcol[3] & 7 ];	\
	      			  dst2[7] = black; }

#define	TRANS_16DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[0] = color_pixel[ vcol[0] >>4 ];	\
	      dst[1] = color_pixel[ vcol[1] >>4 ];	\
	      dst[2] = color_pixel[ vcol[2] >>4 ];	\
	      dst[3] = color_pixel[ vcol[3] >>4 ];	\
	      dst[4] = color_pixel[ vcol[0] & 7 ];	\
	      dst[5] = color_pixel[ vcol[1] & 7 ];	\
	      dst[6] = color_pixel[ vcol[2] & 7 ];	\
	      dst[7] = color_pixel[ vcol[3] & 7 ];	\
	      vram = *(src + k*80+1);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[ 8] = color_pixel[ vcol[0] >>4 ];	\
	      dst[ 9] = color_pixel[ vcol[1] >>4 ];	\
	      dst[10] = color_pixel[ vcol[2] >>4 ];	\
	      dst[11] = color_pixel[ vcol[3] >>4 ];	\
	      dst[12] = color_pixel[ vcol[0] & 7 ];	\
	      dst[13] = color_pixel[ vcol[1] & 7 ];	\
	      dst[14] = color_pixel[ vcol[2] & 7 ];	\
	      dst[15] = color_pixel[ vcol[3] & 7 ];	\
	      memset( dst2, black, sizeof(TYPE)*16 );

#define	STORE_16DOT()							  \
	      vram = *(src + k*80);					  \
	      vcol[0] = get_pixel_index( vram, 0 );			  \
	      vcol[1] = get_pixel_index( vram, 1 );			  \
	      vcol[2] = get_pixel_index( vram, 2 );			  \
	      vcol[3] = get_pixel_index( vram, 3 );			  \
	      if( style & 0x80 ){ dst[0]  = dst[1] = dst2[0] = dst2[1] = tcol; }	\
	      else              { dst[0]  = color_pixel[ vcol[0] >>4 ];			\
	      			  dst[1]  = color_pixel[ vcol[1] >>4 ];			\
	      			  dst2[0] = dst2[1] = black; }				\
	      if( style & 0x40 ){ dst[2]  = dst[3] = dst2[2] = dst2[3] = tcol; }	\
	      else              { dst[2]  = color_pixel[ vcol[2] >>4 ];			\
				  dst[3]  = color_pixel[ vcol[3] >>4 ];			\
	      			  dst2[2] = dst2[3] = black; }				\
	      if( style & 0x20 ){ dst[4]  = dst[5] = dst2[4] = dst2[5] = tcol; }	\
	      else              { dst[4]  = color_pixel[ vcol[0] & 7 ];			\
	      			  dst[5]  = color_pixel[ vcol[1] & 7 ];			\
	      			  dst2[4] = dst2[5] = black; }				\
	      if( style & 0x10 ){ dst[6]  = dst[7] = dst2[6] = dst2[7] = tcol; }	\
	      else              { dst[6]  = color_pixel[ vcol[2] & 7 ];			\
	      			  dst[7]  = color_pixel[ vcol[3] & 7 ];			\
	      			  dst2[6] = dst2[7] = black; }				\
	      vram = *(src + k*80+1);							\
	      vcol[0] = get_pixel_index( vram, 0 );					\
	      vcol[1] = get_pixel_index( vram, 1 );					\
	      vcol[2] = get_pixel_index( vram, 2 );					\
	      vcol[3] = get_pixel_index( vram, 3 );					\
	      if( style & 0x08 ){ dst[ 8]  = dst[ 9] = dst2[ 8] = dst2[ 9] = tcol; }	\
	      else              { dst[ 8]  = color_pixel[ vcol[0] >>4 ];		\
	      			  dst[ 9]  = color_pixel[ vcol[1] >>4 ];		\
	      			  dst2[ 8] = dst2[ 9] = black; }			\
	      if( style & 0x04 ){ dst[10]  = dst[11] = dst2[10] = dst2[11] = tcol; }	\
	      else              { dst[10]  = color_pixel[ vcol[2] >>4 ];		\
				  dst[11]  = color_pixel[ vcol[3] >>4 ];		\
	      			  dst2[10] = dst2[11] = black; }			\
	      if( style & 0x02 ){ dst[12]  = dst[13] = dst2[12] = dst2[13] = tcol; }	\
	      else              { dst[12]  = color_pixel[ vcol[0] & 7 ];		\
	      			  dst[13]  = color_pixel[ vcol[1] & 7 ];		\
	      			  dst2[12] = dst2[13] = black; }			\
	      if( style & 0x01 ){ dst[14]  = dst[15] = dst2[14] = dst2[15] = tcol; }	\
	      else              { dst[14]  = color_pixel[ vcol[2] & 7 ];		\
	      			  dst[15]  = color_pixel[ vcol[3] & 7 ];		\
	      			  dst2[14] = dst2[15] = black; }


#elif	defined( MONO )		/*===========================*/

#define	TRANS_8DOT()					\
	      vram  = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<8; l++, vram<<=1 ){		\
		dst[l]  = get_pixel_mono( vram, tcol );	\
		dst2[l] = black;			\
	      }

#define	STORE_8DOT()								\
	      vram  = *(src + k*80);						\
	      vram &= mask;							\
	      for( m=0x80, l=0; l<8; l++, m>>=1, vram<<=1 ){			\
		if( style & m ){ dst[l]  = dst2[l] = tcol; }			\
		else           { dst[l]  = get_pixel_mono( vram, tcol );	\
				 dst2[l] = black; }				\
	      }

#define	TRANS_16DOT()					\
	      vram = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<8; l++, vram<<=1 ){		\
		dst[l]  = get_pixel_mono( vram, tcol );	\
		dst2[l] = black;			\
	      }						\
	      vram = *(src + k*80+1);			\
	      vram &= mask;				\
	      for(    ; l<16; l++, vram<<=1 ){		\
		dst[l]  = get_pixel_mono( vram, tcol );	\
		dst2[l] = black;			\
	      }

#define	STORE_16DOT()									\
	      vram = *(src + k*80);							\
	      vram &= mask;								\
	      for( m=0x80, l=0; l<8; l+=2, m>>=1, vram<<=2 ){				\
		if( style & m ){ dst[l]   = dst[l+1] = dst2[l] = dst2[l+1] = tcol; }	\
		else           { dst[l]   = get_pixel_mono( vram,    tcol );		\
				 dst[l+1] = get_pixel_mono( vram<<1, tcol );		\
				 dst2[l]  = dst2[l+1] = black; }			\
	      }										\
	      vram = *(src + k*80+1);							\
	      vram &= mask;								\
	      for(            ; l<16; l+=2, m>>=1, vram<<=2 ){				\
		if( style & m ){ dst[l]   = dst[l+1] = dst2[l] = dst2[l+1] = tcol; }	\
		else           { dst[l]   = get_pixel_mono( vram,    tcol );		\
				 dst[l+1] = get_pixel_mono( vram<<1, tcol );		\
				 dst2[l]  = dst2[l+1] = black; }			\
	      }

#elif	defined( CLEAR )	/*===========================*/

#define	TRANS_8DOT()							\
	      vcol = black;						\
	      for( m=0; m<8; m++ ) dst[m] = dst2[m] = vcol;

#define	STORE_8DOT()										\
	      vcol = black;									\
	      if( style & 0x80 ) dst[0] = dst2[0] = tcol;  else dst[0] = dst2[0] = vcol;	\
	      if( style & 0x40 ) dst[1] = dst2[1] = tcol;  else dst[1] = dst2[1] = vcol;	\
	      if( style & 0x20 ) dst[2] = dst2[2] = tcol;  else dst[2] = dst2[2] = vcol;	\
	      if( style & 0x10 ) dst[3] = dst2[3] = tcol;  else dst[3] = dst2[3] = vcol;	\
	      if( style & 0x08 ) dst[4] = dst2[4] = tcol;  else dst[4] = dst2[4] = vcol;	\
	      if( style & 0x04 ) dst[5] = dst2[5] = tcol;  else dst[5] = dst2[5] = vcol;	\
	      if( style & 0x02 ) dst[6] = dst2[6] = tcol;  else dst[6] = dst2[6] = vcol;	\
	      if( style & 0x01 ) dst[7] = dst2[7] = tcol;  else dst[7] = dst2[7] = vcol;

#define	TRANS_16DOT()							\
	      vcol = black;						\
	      for( m=0; m<16; m++ ) dst[m] = dst2[m] = vcol;

#define	STORE_16DOT()									\
	      vcol = black;								\
	      if( style & 0x80 ){ dst[ 0] = dst[ 1] = dst2[ 0] = dst2[ 1] = tcol; }	\
	      else              { dst[ 0] = dst[ 1] = dst2[ 0] = dst2[ 1] = vcol; }	\
	      if( style & 0x40 ){ dst[ 2] = dst[ 3] = dst2[ 2] = dst2[ 3] = tcol; }	\
	      else              { dst[ 2] = dst[ 3] = dst2[ 2] = dst2[ 3] = vcol; }	\
	      if( style & 0x20 ){ dst[ 4] = dst[ 5] = dst2[ 4] = dst2[ 5] = tcol; }	\
	      else              { dst[ 4] = dst[ 5] = dst2[ 4] = dst2[ 5] = vcol; }	\
	      if( style & 0x10 ){ dst[ 6] = dst[ 7] = dst2[ 6] = dst2[ 7] = tcol; }	\
	      else              { dst[ 6] = dst[ 7] = dst2[ 6] = dst2[ 7] = vcol; }	\
	      if( style & 0x08 ){ dst[ 8] = dst[ 9] = dst2[ 8] = dst2[ 9] = tcol; }	\
	      else              { dst[ 8] = dst[ 9] = dst2[ 8] = dst2[ 9] = vcol; }	\
	      if( style & 0x04 ){ dst[10] = dst[11] = dst2[10] = dst2[11] = tcol; }	\
	      else              { dst[10] = dst[11] = dst2[10] = dst2[11] = vcol; }	\
	      if( style & 0x02 ){ dst[12] = dst[13] = dst2[12] = dst2[13] = tcol; }	\
	      else              { dst[12] = dst[13] = dst2[12] = dst2[13] = vcol; }	\
	      if( style & 0x01 ){ dst[14] = dst[15] = dst2[14] = dst2[15] = tcol; }	\
	      else              { dst[14] = dst[15] = dst2[14] = dst2[15] = vcol; }

#endif				/*===========================*/
#endif

#define	DST_FORWARD()					\
		int zoubunn = 1;				\
		switch(kaigyou){				\
		case 1:							\
			zoubunn = 2;				\
			break;						\
		case 3:							\
			kaigyou = 0;				\
		}								\
		kaigyou += 1;					\
	    dst += zoubunn*WIDTH;			\
	    if(kaigyou == 1){				\
		    if((crtc_font_height == 8) && ((i % 6) == 0) && (k == (crtc_font_height - 2))){				\
				dst2 = dst;					\
			}else{							\
				dst2 = dst + WIDTH;			\
			}								\
	    }else{							\
		    if((crtc_font_height == 10) && !((i % 2) == 0) && (k == (crtc_font_height - 2))){				\
				dst2 = dst + WIDTH;			\
			}else{							\
				dst2 = dst;					\
			}								\
	    }								

#define	DST_BACKWARD()				\
	kaigyou = 0;						\
	dst  += -((crtc_font_height*1 + (crtc_font_height+1)/3)*WIDTH);	\
	dst2 = dst;


#define	SRC_NEXT_CHARA()			\
	src ++;

#define	DST_NEXT_CHARA()			\
	dst += 8-2;	dst2 += 8-2;


#define	SRC_NEXT_LINE()				\
      src  += (crtc_font_height-1)*80;

#define	DST_NEXT_LINE()							\
		int gen = 0;							\
		switch(crtc_font_height){				\
		case 8:									\
			switch(i % 6){					\
			case 0:								\
				gen = 1;						\
			}									\
			break;								\
		case 10:								\
			switch(i % 2){					\
			case 0:								\
				gen = 1;						\
			}									\
		}										\
		dst  += SCREEN_OFFSET + (crtc_font_height*1 - gen + crtc_font_height/3)*WIDTH;		\
		dst2 = dst;


#elif	defined( DOUBLE )	/*-------------------------------------------*/

#if	!defined( INTERLACE )
#define	MASK_8DOT()	for( m=0; m<16; m++ ) dst[m] = tcol;
#define	MASK_16DOT()	for( m=0; m<32; m++ ) dst[m] = tcol;
#else
#define	MASK_8DOT()	for( m=0; m<16; m++ ) dst[m] = dst3[m] = tcol;
#define	MASK_16DOT()	for( m=0; m<32; m++ ) dst[m] = dst3[m] = tcol;
#endif

#if	defined( INTERLACE )
#if	0
#define	COPY_8DOT()	memcpy( dst2, dst,   sizeof(TYPE)*16 );	\
			memset( dst3, black, sizeof(TYPE)*16 );	\
			memset( dst4, black, sizeof(TYPE)*16 );
#define	COPY_16DOT()	memcpy( dst2, dst,   sizeof(TYPE)*32 );	\
			memset( dst3, black, sizeof(TYPE)*32 );	\
			memset( dst4, black, sizeof(TYPE)*32 );
#else
#define	COPY_8DOT()	memcpy( dst2, dst,   sizeof(TYPE)*16 );	\
			memcpy( dst4, dst3,  sizeof(TYPE)*16 );
#define	COPY_16DOT()	memcpy( dst2, dst,   sizeof(TYPE)*32 );	\
			memcpy( dst4, dst3,  sizeof(TYPE)*32 );
#endif
#else
#define	COPY_8DOT()	memcpy( dst2, dst, sizeof(TYPE)*16 );	\
			memcpy( dst3, dst, sizeof(TYPE)*16 );	\
			memcpy( dst4, dst, sizeof(TYPE)*16 );
#define	COPY_16DOT()	memcpy( dst2, dst, sizeof(TYPE)*32 );	\
			memcpy( dst3, dst, sizeof(TYPE)*32 );	\
			memcpy( dst4, dst, sizeof(TYPE)*32 );
#endif

#if	!defined( INTERLACE )
#if	defined( COLOR )	/*===========================*/

#define	TRANS_8DOT()						\
	      vram = *(src + k*80);				\
	      vcol[0] = get_pixel_index( vram, 0 );		\
	      vcol[1] = get_pixel_index( vram, 1 );		\
	      vcol[2] = get_pixel_index( vram, 2 );		\
	      vcol[3] = get_pixel_index( vram, 3 );		\
	      dst[ 0] = dst[ 1] = color_pixel[ vcol[0] >>4 ];	\
	      dst[ 2] = dst[ 3] = color_pixel[ vcol[1] >>4 ];	\
	      dst[ 4] = dst[ 5] = color_pixel[ vcol[2] >>4 ];	\
	      dst[ 6] = dst[ 7] = color_pixel[ vcol[3] >>4 ];	\
	      dst[ 8] = dst[ 9] = color_pixel[ vcol[0] & 7 ];	\
	      dst[10] = dst[11] = color_pixel[ vcol[1] & 7 ];	\
	      dst[12] = dst[13] = color_pixel[ vcol[2] & 7 ];	\
	      dst[14] = dst[15] = color_pixel[ vcol[3] & 7 ];

#define	STORE_8DOT()							      \
	      vram = *(src + k*80);					      \
	      vcol[0] = get_pixel_index( vram, 0 );			      \
	      vcol[1] = get_pixel_index( vram, 1 );			      \
	      vcol[2] = get_pixel_index( vram, 2 );			      \
	      vcol[3] = get_pixel_index( vram, 3 );			      \
	      if( style & 0x80 ) dst[ 0]=dst[ 1]= tcol;			      \
	      else               dst[ 0]=dst[ 1]= color_pixel[ vcol[0] >>4 ]; \
	      if( style & 0x40 ) dst[ 2]=dst[ 3]= tcol;			      \
	      else               dst[ 2]=dst[ 3]= color_pixel[ vcol[1] >>4 ]; \
	      if( style & 0x20 ) dst[ 4]=dst[ 5]= tcol;			      \
	      else               dst[ 4]=dst[ 5]= color_pixel[ vcol[2] >>4 ]; \
	      if( style & 0x10 ) dst[ 6]=dst[ 7]= tcol;			      \
	      else               dst[ 6]=dst[ 7]= color_pixel[ vcol[3] >>4 ]; \
	      if( style & 0x08 ) dst[ 8]=dst[ 9]= tcol;			      \
	      else               dst[ 8]=dst[ 9]= color_pixel[ vcol[0] & 7 ]; \
	      if( style & 0x04 ) dst[10]=dst[11]= tcol;			      \
	      else               dst[10]=dst[11]= color_pixel[ vcol[1] & 7 ]; \
	      if( style & 0x02 ) dst[12]=dst[13]= tcol;			      \
	      else               dst[12]=dst[13]= color_pixel[ vcol[2] & 7 ]; \
	      if( style & 0x01 ) dst[14]=dst[15]= tcol;			      \
	      else               dst[14]=dst[15]= color_pixel[ vcol[3] & 7 ];

#define	TRANS_16DOT()						\
	      vram = *(src + k*80);				\
	      vcol[0] = get_pixel_index( vram, 0 );		\
	      vcol[1] = get_pixel_index( vram, 1 );		\
	      vcol[2] = get_pixel_index( vram, 2 );		\
	      vcol[3] = get_pixel_index( vram, 3 );		\
	      dst[ 0] = dst[ 1] = color_pixel[ vcol[0] >>4 ];	\
	      dst[ 2] = dst[ 3] = color_pixel[ vcol[1] >>4 ];	\
	      dst[ 4] = dst[ 5] = color_pixel[ vcol[2] >>4 ];	\
	      dst[ 6] = dst[ 7] = color_pixel[ vcol[3] >>4 ];	\
	      dst[ 8] = dst[ 9] = color_pixel[ vcol[0] & 7 ];	\
	      dst[10] = dst[11] = color_pixel[ vcol[1] & 7 ];	\
	      dst[12] = dst[13] = color_pixel[ vcol[2] & 7 ];	\
	      dst[14] = dst[15] = color_pixel[ vcol[3] & 7 ];	\
	      vram = *(src + k*80+1);				\
	      vcol[0] = get_pixel_index( vram, 0 );		\
	      vcol[1] = get_pixel_index( vram, 1 );		\
	      vcol[2] = get_pixel_index( vram, 2 );		\
	      vcol[3] = get_pixel_index( vram, 3 );		\
	      dst[16] = dst[17] = color_pixel[ vcol[0] >>4 ];	\
	      dst[18] = dst[19] = color_pixel[ vcol[1] >>4 ];	\
	      dst[20] = dst[21] = color_pixel[ vcol[2] >>4 ];	\
	      dst[22] = dst[23] = color_pixel[ vcol[3] >>4 ];	\
	      dst[24] = dst[25] = color_pixel[ vcol[0] & 7 ];	\
	      dst[26] = dst[27] = color_pixel[ vcol[1] & 7 ];	\
	      dst[28] = dst[29] = color_pixel[ vcol[2] & 7 ];	\
	      dst[30] = dst[31] = color_pixel[ vcol[3] & 7 ];


#define	STORE_16DOT()							      \
	      vram = *(src + k*80);					      \
	      vcol[0] = get_pixel_index( vram, 0 );			      \
	      vcol[1] = get_pixel_index( vram, 1 );			      \
	      vcol[2] = get_pixel_index( vram, 2 );			      \
	      vcol[3] = get_pixel_index( vram, 3 );			      \
	      if( style & 0x80 ){ dst[ 0]=dst[ 1]=dst[ 2]=dst[ 3] = tcol;   } \
	      else              { dst[ 0]=dst[ 1]=color_pixel[ vcol[0]>>4 ];  \
	      			  dst[ 2]=dst[ 3]=color_pixel[ vcol[1]>>4 ];} \
	      if( style & 0x40 ){ dst[ 4]=dst[ 5]=dst[ 6]=dst[ 7] = tcol;   } \
	      else              { dst[ 4]=dst[ 5]=color_pixel[ vcol[2]>>4 ];  \
	      			  dst[ 6]=dst[ 7]=color_pixel[ vcol[3]>>4 ];} \
	      if( style & 0x20 ){ dst[ 8]=dst[ 9]=dst[10]=dst[11] = tcol;   } \
	      else              { dst[ 8]=dst[ 9]=color_pixel[ vcol[0] &7 ];  \
	      			  dst[10]=dst[11]=color_pixel[ vcol[1] &7 ];} \
	      if( style & 0x10 ){ dst[12]=dst[13]=dst[14]=dst[15] = tcol;   } \
	      else              { dst[12]=dst[13]=color_pixel[ vcol[2] &7 ];  \
	      			  dst[14]=dst[15]=color_pixel[ vcol[3] &7 ];} \
	      vram = *(src + k*80 +1);					      \
	      vcol[0] = get_pixel_index( vram, 0 );			      \
	      vcol[1] = get_pixel_index( vram, 1 );			      \
	      vcol[2] = get_pixel_index( vram, 2 );			      \
	      vcol[3] = get_pixel_index( vram, 3 );			      \
	      if( style & 0x08 ){ dst[16]=dst[17]=dst[18]=dst[19] = tcol;   } \
	      else              { dst[16]=dst[17]=color_pixel[ vcol[0]>>4 ];  \
	      			  dst[18]=dst[19]=color_pixel[ vcol[1]>>4 ];} \
	      if( style & 0x04 ){ dst[20]=dst[21]=dst[22]=dst[23] = tcol;   } \
	      else              { dst[20]=dst[21]=color_pixel[ vcol[2]>>4 ];  \
	      			  dst[22]=dst[23]=color_pixel[ vcol[3]>>4 ];} \
	      if( style & 0x02 ){ dst[24]=dst[25]=dst[26]=dst[27] = tcol;   } \
	      else              { dst[24]=dst[25]=color_pixel[ vcol[0] &7 ];  \
	      			  dst[26]=dst[27]=color_pixel[ vcol[1] &7 ];} \
	      if( style & 0x01 ){ dst[28]=dst[29]=dst[30]=dst[31] = tcol;   } \
	      else              { dst[28]=dst[29]=color_pixel[ vcol[2] &7 ];  \
	      			  dst[30]=dst[31]=color_pixel[ vcol[3] &7 ];}

#elif	defined( MONO )		/*===========================*/

#define	TRANS_8DOT()							\
	      vram  = *(src + k*80);					\
	      vram &= mask;						\
	      for( l=0; l<16; l+=2, vram<<=1 ){				\
		dst[l] = dst[l+1] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_8DOT()							      \
	      vram  = *(src + k*80);					      \
	      vram &= mask;						      \
	      for( m=0x80, l=0; l<16; l+=2, m>>=1, vram<<=1 ){		      \
		if( style & m ) dst[l]=dst[l+1]=tcol;			      \
		else            dst[l]=dst[l+1]=get_pixel_mono( vram, tcol ); \
	      }

#define	TRANS_16DOT()							\
	      vram = *(src + k*80);					\
	      vram &= mask;						\
	      for( l=0; l<16; l+=2, vram<<=1 ){				\
		dst[l] = dst[l+1] = get_pixel_mono( vram, tcol );	\
	      }								\
	      vram = *(src + k*80+1);					\
	      vram &= mask;						\
	      for(    ; l<32; l+=2, vram<<=1 ){				\
		dst[l] = dst[l+1] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_16DOT()							      \
	      vram = *(src + k*80);					      \
	      vram &= mask;						      \
	      for( m=0x80, l=0; l<16; l+=4, m>>=1, vram<<=2 ){		      \
		if( style&m ){dst[l]  =dst[l+1]=dst[l+2]=dst[l+3] = tcol;    }\
		else         {dst[l]  =dst[l+1]=get_pixel_mono(vram,   tcol); \
			      dst[l+2]=dst[l+3]=get_pixel_mono(vram<<1,tcol);}\
	      }								      \
	      vram = *(src + k*80+1);					      \
	      vram &= mask;						      \
	      for(            ; l<32; l+=4, m>>=1, vram<<=2 ){		      \
		if( style&m ){dst[l]  =dst[l+1]=dst[l+2]=dst[l+3] = tcol;    }\
		else         {dst[l]  =dst[l+1]=get_pixel_mono(vram,   tcol); \
			      dst[l+2]=dst[l+3]=get_pixel_mono(vram<<1,tcol);}\
	      }

#elif	defined( CLEAR )	/*===========================*/

#define	TRANS_8DOT()							\
	      vcol = black;						\
	      for( m=0; m<16; m++ ) dst[m] = vcol;

#define	STORE_8DOT()							      \
	      vcol = black;						\
	      if(style&0x80) dst[ 0]=dst[ 1]=tcol;  else dst[ 0]=dst[ 1]=vcol;\
	      if(style&0x40) dst[ 2]=dst[ 3]=tcol;  else dst[ 2]=dst[ 3]=vcol;\
	      if(style&0x20) dst[ 4]=dst[ 5]=tcol;  else dst[ 4]=dst[ 5]=vcol;\
	      if(style&0x10) dst[ 6]=dst[ 7]=tcol;  else dst[ 6]=dst[ 7]=vcol;\
	      if(style&0x08) dst[ 8]=dst[ 9]=tcol;  else dst[ 8]=dst[ 9]=vcol;\
	      if(style&0x04) dst[10]=dst[11]=tcol;  else dst[10]=dst[11]=vcol;\
	      if(style&0x02) dst[12]=dst[13]=tcol;  else dst[12]=dst[13]=vcol;\
	      if(style&0x01) dst[14]=dst[15]=tcol;  else dst[14]=dst[15]=vcol;

#define	TRANS_16DOT()							\
	      vcol = black;						\
	      for( m=0; m<32; m++ ) dst[m] = vcol;

#define	STORE_16DOT()							\
	      vcol = black;						\
	      if( style & 0x80 ) dst[ 0]=dst[ 1]=dst[ 2]=dst[ 3]= tcol;	\
	      else               dst[ 0]=dst[ 1]=dst[ 2]=dst[ 3]= vcol;	\
	      if( style & 0x40 ) dst[ 4]=dst[ 5]=dst[ 6]=dst[ 7]= tcol;	\
	      else               dst[ 4]=dst[ 5]=dst[ 6]=dst[ 7]= vcol;	\
	      if( style & 0x20 ) dst[ 8]=dst[ 9]=dst[10]=dst[11]= tcol;	\
	      else               dst[ 8]=dst[ 9]=dst[10]=dst[11]= vcol;	\
	      if( style & 0x10 ) dst[12]=dst[13]=dst[14]=dst[15]= tcol;	\
	      else               dst[12]=dst[13]=dst[14]=dst[15]= vcol;	\
	      if( style & 0x08 ) dst[16]=dst[17]=dst[18]=dst[19]= tcol;	\
	      else               dst[16]=dst[17]=dst[18]=dst[19]= vcol;	\
	      if( style & 0x04 ) dst[20]=dst[21]=dst[22]=dst[23]= tcol;	\
	      else               dst[20]=dst[21]=dst[22]=dst[23]= vcol;	\
	      if( style & 0x02 ) dst[24]=dst[25]=dst[26]=dst[27]= tcol;	\
	      else               dst[24]=dst[25]=dst[26]=dst[27]= vcol;	\
	      if( style & 0x01 ) dst[28]=dst[29]=dst[30]=dst[31]= tcol;	\
	      else               dst[28]=dst[29]=dst[30]=dst[31]= vcol;

#endif				/*===========================*/
#else
#if	defined( COLOR )	/*===========================*/

#define	TRANS_8DOT()						\
	      vram = *(src + k*80);				\
	      vcol[0] = get_pixel_index( vram, 0 );		\
	      vcol[1] = get_pixel_index( vram, 1 );		\
	      vcol[2] = get_pixel_index( vram, 2 );		\
	      vcol[3] = get_pixel_index( vram, 3 );		\
	      dst[ 0] = dst[ 1] = color_pixel[ vcol[0] >>4 ];	\
	      dst[ 2] = dst[ 3] = color_pixel[ vcol[1] >>4 ];	\
	      dst[ 4] = dst[ 5] = color_pixel[ vcol[2] >>4 ];	\
	      dst[ 6] = dst[ 7] = color_pixel[ vcol[3] >>4 ];	\
	      dst[ 8] = dst[ 9] = color_pixel[ vcol[0] & 7 ];	\
	      dst[10] = dst[11] = color_pixel[ vcol[1] & 7 ];	\
	      dst[12] = dst[13] = color_pixel[ vcol[2] & 7 ];	\
	      dst[14] = dst[15] = color_pixel[ vcol[3] & 7 ];	\
	      memset( dst3, black, sizeof(TYPE)*16 );

#define	STORE_8DOT()									\
	      vram = *(src + k*80);							\
	      vcol[0] = get_pixel_index( vram, 0 );					\
	      vcol[1] = get_pixel_index( vram, 1 );					\
	      vcol[2] = get_pixel_index( vram, 2 );					\
	      vcol[3] = get_pixel_index( vram, 3 );					\
	      if( style & 0x80 ){ dst[ 0] =dst[ 1] = dst3[ 0]=dst3[ 1]= tcol; }		\
	      else              { dst[ 0] =dst[ 1] = color_pixel[ vcol[0] >>4 ];	\
	      			  dst3[ 0]=dst3[ 1]= black; }				\
	      if( style & 0x40 ){ dst[ 2] =dst[ 3] = dst3[ 2]=dst3[ 3]= tcol; }		\
	      else              { dst[ 2] =dst[ 3] = color_pixel[ vcol[1] >>4 ];	\
	      			  dst3[ 2]=dst3[ 3]= black; }				\
	      if( style & 0x20 ){ dst[ 4] =dst[ 5] = dst3[ 4]=dst3[ 5]= tcol; }		\
	      else              { dst[ 4] =dst[ 5] = color_pixel[ vcol[2] >>4 ];	\
	      			  dst3[ 4]=dst3[ 5]= black; }				\
	      if( style & 0x10 ){ dst[ 6] =dst[ 7] = dst3[ 6]=dst3[ 7]= tcol; }		\
	      else              { dst[ 6] =dst[ 7] = color_pixel[ vcol[3] >>4 ];	\
	      			  dst3[ 6]=dst3[ 7]= black; }				\
	      if( style & 0x08 ){ dst[ 8] =dst[ 9] = dst3[ 8]=dst3[ 9]= tcol; }		\
	      else              { dst[ 8] =dst[ 9] = color_pixel[ vcol[0] & 7 ];	\
	      			  dst3[ 8]=dst3[ 9]= black; }				\
	      if( style & 0x04 ){ dst[10] =dst[11] = dst3[10]=dst3[11]= tcol; }		\
	      else              { dst[10] =dst[11] = color_pixel[ vcol[1] & 7 ];	\
	      			  dst3[10]=dst3[11]= black; }				\
	      if( style & 0x02 ){ dst[12] =dst[13] = dst3[12]=dst3[13]= tcol; }		\
	      else              { dst[12] =dst[13] = color_pixel[ vcol[2] & 7 ];	\
	      			  dst3[12]=dst3[13]= black; }				\
	      if( style & 0x01 ){ dst[14] =dst[15] = dst3[14]=dst3[15]= tcol; }		\
	      else              { dst[14] =dst[15] = color_pixel[ vcol[3] & 7 ];	\
	      			  dst3[14]=dst3[15]= black; }

#define	TRANS_16DOT()						\
	      vram = *(src + k*80);				\
	      vcol[0] = get_pixel_index( vram, 0 );		\
	      vcol[1] = get_pixel_index( vram, 1 );		\
	      vcol[2] = get_pixel_index( vram, 2 );		\
	      vcol[3] = get_pixel_index( vram, 3 );		\
	      dst[ 0] = dst[ 1] = color_pixel[ vcol[0] >>4 ];	\
	      dst[ 2] = dst[ 3] = color_pixel[ vcol[1] >>4 ];	\
	      dst[ 4] = dst[ 5] = color_pixel[ vcol[2] >>4 ];	\
	      dst[ 6] = dst[ 7] = color_pixel[ vcol[3] >>4 ];	\
	      dst[ 8] = dst[ 9] = color_pixel[ vcol[0] & 7 ];	\
	      dst[10] = dst[11] = color_pixel[ vcol[1] & 7 ];	\
	      dst[12] = dst[13] = color_pixel[ vcol[2] & 7 ];	\
	      dst[14] = dst[15] = color_pixel[ vcol[3] & 7 ];	\
	      vram = *(src + k*80+1);				\
	      vcol[0] = get_pixel_index( vram, 0 );		\
	      vcol[1] = get_pixel_index( vram, 1 );		\
	      vcol[2] = get_pixel_index( vram, 2 );		\
	      vcol[3] = get_pixel_index( vram, 3 );		\
	      dst[16] = dst[17] = color_pixel[ vcol[0] >>4 ];	\
	      dst[18] = dst[19] = color_pixel[ vcol[1] >>4 ];	\
	      dst[20] = dst[21] = color_pixel[ vcol[2] >>4 ];	\
	      dst[22] = dst[23] = color_pixel[ vcol[3] >>4 ];	\
	      dst[24] = dst[25] = color_pixel[ vcol[0] & 7 ];	\
	      dst[26] = dst[27] = color_pixel[ vcol[1] & 7 ];	\
	      dst[28] = dst[29] = color_pixel[ vcol[2] & 7 ];	\
	      dst[30] = dst[31] = color_pixel[ vcol[3] & 7 ];	\
	      memset( dst3, black, sizeof(TYPE)*32 );


#define	STORE_16DOT()									\
	      vram = *(src + k*80);							\
	      vcol[0] = get_pixel_index( vram, 0 );					\
	      vcol[1] = get_pixel_index( vram, 1 );					\
	      vcol[2] = get_pixel_index( vram, 2 );					\
	      vcol[3] = get_pixel_index( vram, 3 );					\
	      if( style & 0x80 ){ dst[ 0] =dst[ 1] =dst[ 2] =dst[ 3]  = tcol;		\
	      			  dst3[ 0]=dst3[ 1]=dst3[ 2]=dst3[ 3] = tcol; }		\
	      else              { dst[ 0] =dst[ 1] =color_pixel[ vcol[0]>>4 ];		\
	      			  dst[ 2] =dst[ 3] =color_pixel[ vcol[1]>>4 ];		\
	      			  dst3[ 0]=dst3[ 1]=dst3[ 2]=dst3[ 3] = black; }	\
	      if( style & 0x40 ){ dst[ 4] =dst[ 5] =dst[ 6] =dst[ 7]  = tcol;		\
	      			  dst3[ 4]=dst3[ 5]=dst3[ 6]=dst3[ 7] = tcol; }		\
	      else              { dst[ 4] =dst[ 5] =color_pixel[ vcol[2]>>4 ];		\
	      			  dst[ 6] =dst[ 7] =color_pixel[ vcol[3]>>4 ];		\
	      			  dst3[ 4]=dst3[ 5]=dst3[ 6]=dst3[ 7] = black; }	\
	      if( style & 0x20 ){ dst[ 8] =dst[ 9] =dst[10] =dst[11]  = tcol;		\
	      			  dst3[ 8]=dst3[ 9]=dst3[10]=dst3[11] = tcol; }		\
	      else              { dst[ 8] =dst[ 9] =color_pixel[ vcol[0] &7 ];		\
	      			  dst[10] =dst[11] =color_pixel[ vcol[1] &7 ];		\
	      			  dst3[ 8]=dst3[ 9]=dst3[10]=dst3[11] = black; }	\
	      if( style & 0x10 ){ dst[12] =dst[13] =dst[14] =dst[15]  = tcol;		\
	      			  dst3[12]=dst3[13]=dst3[14]=dst3[15] = tcol; }		\
	      else              { dst[12] =dst[13] =color_pixel[ vcol[2] &7 ];		\
	      			  dst[14] =dst[15] =color_pixel[ vcol[3] &7 ];		\
	      			  dst3[12]=dst3[13]=dst3[14]=dst3[15] = black; }	\
	      vram = *(src + k*80 +1);							\
	      vcol[0] = get_pixel_index( vram, 0 );					\
	      vcol[1] = get_pixel_index( vram, 1 );					\
	      vcol[2] = get_pixel_index( vram, 2 );					\
	      vcol[3] = get_pixel_index( vram, 3 );					\
	      if( style & 0x08 ){ dst[16] =dst[17] =dst[18] =dst[19]  = tcol;		\
	      			  dst3[16]=dst3[17]=dst3[18]=dst3[19] = tcol; }		\
	      else              { dst[16] =dst[17] =color_pixel[ vcol[0]>>4 ];		\
	      			  dst[18] =dst[19] =color_pixel[ vcol[1]>>4 ];		\
	      			  dst3[16]=dst3[17]=dst3[18]=dst3[19] = black; }	\
	      if( style & 0x04 ){ dst[20] =dst[21] =dst[22] =dst[23]  = tcol;		\
	      			  dst3[20]=dst3[21]=dst3[22]=dst3[23] = tcol; }		\
	      else              { dst[20] =dst[21] =color_pixel[ vcol[2]>>4 ];		\
	      			  dst[22] =dst[23] =color_pixel[ vcol[3]>>4 ];		\
	      			  dst3[20]=dst3[21]=dst3[22]=dst3[23] = black; }	\
	      if( style & 0x02 ){ dst[24] =dst[25] =dst[26]=dst[27] = tcol;		\
	      			  dst3[24]=dst3[25]=dst3[26]=dst3[27] = tcol; }		\
	      else              { dst[24] =dst[25] =color_pixel[ vcol[0] &7 ];		\
	      			  dst[26] =dst[27] =color_pixel[ vcol[1] &7 ];		\
	      			  dst3[24]=dst3[25]=dst3[26]=dst3[27] = black; }	\
	      if( style & 0x01 ){ dst[28] =dst[29] =dst[30]=dst[31] = tcol;		\
	      			  dst3[28]=dst3[29]=dst3[30]=dst3[31] = tcol; }		\
	      else              { dst[28] =dst[29] =color_pixel[ vcol[2] &7 ];		\
	      			  dst[30] =dst[31] =color_pixel[ vcol[3] &7 ];		\
	      			  dst3[28]=dst3[29]=dst3[30]=dst3[31] = black; }

#elif	defined( MONO )		/*===========================*/

#define	TRANS_8DOT()							\
	      vram  = *(src + k*80);					\
	      vram &= mask;						\
	      for( l=0; l<16; l+=2, vram<<=1 ){				\
		dst[l]  = dst[l+1]  = get_pixel_mono( vram, tcol );	\
		dst3[l] = dst3[l+1] = black;				\
	      }

#define	STORE_8DOT()									\
	      vram  = *(src + k*80);							\
	      vram &= mask;								\
	      for( m=0x80, l=0; l<16; l+=2, m>>=1, vram<<=1 ){				\
		if( style & m ){ dst[l] =dst[l+1] =dst3[l]=dst3[l+1]=tcol; }		\
		else           { dst[l] =dst[l+1] =get_pixel_mono( vram, tcol );	\
				 dst3[l]=dst3[l+1]=black; }				\
	      }

#define	TRANS_16DOT()							\
	      vram = *(src + k*80);					\
	      vram &= mask;						\
	      for( l=0; l<16; l+=2, vram<<=1 ){				\
		dst[l]  = dst[l+1]  = get_pixel_mono( vram, tcol );	\
		dst3[l] = dst3[l+1] = black;				\
	      }								\
	      vram = *(src + k*80+1);					\
	      vram &= mask;						\
	      for(    ; l<32; l+=2, vram<<=1 ){				\
		dst[l]  = dst[l+1]  = get_pixel_mono( vram, tcol );	\
		dst3[l] = dst3[l+1] = black;				\
	      }

#define	STORE_16DOT()									\
	      vram = *(src + k*80);							\
	      vram &= mask;								\
	      for( m=0x80, l=0; l<16; l+=4, m>>=1, vram<<=2 ){				\
		if( style&m ){dst[l]   =dst[l+1] =dst[l+2] =dst[l+3]  = tcol;		\
			      dst3[l]  =dst3[l+1]=dst3[l+2]=dst3[l+3] = tcol; }		\
		else         {dst[l]   =dst[l+1] =get_pixel_mono(vram,   tcol);		\
			      dst[l+2] =dst[l+3] =get_pixel_mono(vram<<1,tcol);		\
			      dst3[l]  =dst3[l+1]=dst3[l+2]=dst3[l+3] = black; }	\
	      }										\
	      vram = *(src + k*80+1);							\
	      vram &= mask;								\
	      for(            ; l<32; l+=4, m>>=1, vram<<=2 ){				\
		if( style&m ){dst[l]   =dst[l+1] =dst[l+2] =dst[l+3]  = tcol;		\
			      dst3[l]  =dst3[l+1]=dst3[l+2]=dst3[l+3] = tcol; }		\
		else         {dst[l]   =dst[l+1] =get_pixel_mono(vram,   tcol);		\
			      dst[l+2] =dst[l+3] =get_pixel_mono(vram<<1,tcol);		\
			      dst3[l]  =dst3[l+1]=dst3[l+2]=dst3[l+3] = black; }	\
	      }

#elif	defined( CLEAR )	/*===========================*/

#define	TRANS_8DOT()							\
	      vcol = black;						\
	      for( m=0; m<16; m++ ) dst[m] = dst3[m] = vcol;

#define	STORE_8DOT()													\
	      vcol = black;												\
	      if(style&0x80) dst[ 0]=dst[ 1]=dst3[ 0]=dst3[ 1]=tcol;  else dst[ 0]=dst[ 1]=dst3[ 0]=dst3[ 1]=vcol;	\
	      if(style&0x40) dst[ 2]=dst[ 3]=dst3[ 2]=dst3[ 3]=tcol;  else dst[ 2]=dst[ 3]=dst3[ 2]=dst3[ 3]=vcol;	\
	      if(style&0x20) dst[ 4]=dst[ 5]=dst3[ 4]=dst3[ 5]=tcol;  else dst[ 4]=dst[ 5]=dst3[ 4]=dst3[ 5]=vcol;	\
	      if(style&0x10) dst[ 6]=dst[ 7]=dst3[ 6]=dst3[ 7]=tcol;  else dst[ 6]=dst[ 7]=dst3[ 6]=dst3[ 7]=vcol;	\
	      if(style&0x08) dst[ 8]=dst[ 9]=dst3[ 8]=dst3[ 9]=tcol;  else dst[ 8]=dst[ 9]=dst3[ 8]=dst3[ 9]=vcol;	\
	      if(style&0x04) dst[10]=dst[11]=dst3[10]=dst3[11]=tcol;  else dst[10]=dst[11]=dst3[10]=dst3[11]=vcol;	\
	      if(style&0x02) dst[12]=dst[13]=dst3[12]=dst3[13]=tcol;  else dst[12]=dst[13]=dst3[12]=dst3[13]=vcol;	\
	      if(style&0x01) dst[14]=dst[15]=dst3[14]=dst3[15]=tcol;  else dst[14]=dst[15]=dst3[14]=dst3[15]=vcol;

#define	TRANS_16DOT()							\
	      vcol = black;						\
	      for( m=0; m<32; m++ ) dst[m] = dst3[m] = vcol;

#define	STORE_16DOT()												\
	      vcol = black;											\
	      if( style & 0x80 ) dst[ 0]=dst[ 1]=dst[ 2]=dst[ 3]=dst3[ 0]=dst3[ 1]=dst3[ 2]=dst3[ 3]= tcol;	\
	      else               dst[ 0]=dst[ 1]=dst[ 2]=dst[ 3]=dst3[ 0]=dst3[ 1]=dst3[ 2]=dst3[ 3]= vcol;	\
	      if( style & 0x40 ) dst[ 4]=dst[ 5]=dst[ 6]=dst[ 7]=dst3[ 4]=dst3[ 5]=dst3[ 6]=dst3[ 7]= tcol;	\
	      else               dst[ 4]=dst[ 5]=dst[ 6]=dst[ 7]=dst3[ 4]=dst3[ 5]=dst3[ 6]=dst3[ 7]= vcol;	\
	      if( style & 0x20 ) dst[ 8]=dst[ 9]=dst[10]=dst[11]=dst3[ 8]=dst3[ 9]=dst3[10]=dst3[11]= tcol;	\
	      else               dst[ 8]=dst[ 9]=dst[10]=dst[11]=dst3[ 8]=dst3[ 9]=dst3[10]=dst3[11]= vcol;	\
	      if( style & 0x10 ) dst[12]=dst[13]=dst[14]=dst[15]=dst3[12]=dst3[13]=dst3[14]=dst3[15]= tcol;	\
	      else               dst[12]=dst[13]=dst[14]=dst[15]=dst3[12]=dst3[13]=dst3[14]=dst3[15]= vcol;	\
	      if( style & 0x08 ) dst[16]=dst[17]=dst[18]=dst[19]=dst3[16]=dst3[17]=dst3[18]=dst3[19]= tcol;	\
	      else               dst[16]=dst[17]=dst[18]=dst[19]=dst3[16]=dst3[17]=dst3[18]=dst3[19]= vcol;	\
	      if( style & 0x04 ) dst[20]=dst[21]=dst[22]=dst[23]=dst3[20]=dst3[21]=dst3[22]=dst3[23]= tcol;	\
	      else               dst[20]=dst[21]=dst[22]=dst[23]=dst3[20]=dst3[21]=dst3[22]=dst3[23]= vcol;	\
	      if( style & 0x02 ) dst[24]=dst[25]=dst[26]=dst[27]=dst3[24]=dst3[25]=dst3[26]=dst3[27]= tcol;	\
	      else               dst[24]=dst[25]=dst[26]=dst[27]=dst3[24]=dst3[25]=dst3[26]=dst3[27]= vcol;	\
	      if( style & 0x01 ) dst[28]=dst[29]=dst[30]=dst[31]=dst3[28]=dst3[29]=dst3[30]=dst3[31]= tcol;	\
	      else               dst[28]=dst[29]=dst[30]=dst[31]=dst3[28]=dst3[29]=dst3[30]=dst3[31]= vcol;

#endif				/*===========================*/
#endif

#define	DST_FORWARD()					\
	    dst  += 4*WIDTH;	dst2 += 4*WIDTH;	\
	    dst3 += 4*WIDTH;	dst4 += 4*WIDTH;

#define	DST_BACKWARD()				\
	  dst  += -crtc_font_height*4*WIDTH;	\
	  dst2 += -crtc_font_height*4*WIDTH;	\
	  dst3 += -crtc_font_height*4*WIDTH;	\
	  dst4 += -crtc_font_height*4*WIDTH;


#define	SRC_NEXT_CHARA()			\
	src ++;

#define	DST_NEXT_CHARA()			\
	dst  += 16;	dst2 += 16;		\
	dst3 += 16;	dst4 += 16;


#define	SRC_NEXT_LINE()				\
      src  += (crtc_font_height-1)*80;

#define	DST_NEXT_LINE()							\
      dst  += SCREEN_OFFSET + (crtc_font_height*4-1)*WIDTH;		\
      dst2 += SCREEN_OFFSET + (crtc_font_height*4-1)*WIDTH;		\
      dst3 += SCREEN_OFFSET + (crtc_font_height*4-1)*WIDTH;		\
      dst4 += SCREEN_OFFSET + (crtc_font_height*4-1)*WIDTH;


#endif				/*-------------------------------------------*/


/*****************************************************************************/
#include "screen-base.h"
