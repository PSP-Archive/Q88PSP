#undef	vram2pixel

#undef	get_pixel_400_B
#undef	get_pixel_400_R


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
#define	get_pixel_400_B( data, col )				\
		((data)&0x00000080) ? (col) : color_pixel[0]
#define	get_pixel_400_R( data, col )				\
		((data)&0x00008000) ? (col) : color_pixel[0]
#else
#define	get_pixel_400_B( data, col )				\
		((data)&0x80000000) ? (col) : color_pixel[0]
#define	get_pixel_400_R( data, col )				\
		((data)&0x00800000) ? (col) : color_pixel[0]
#endif



/*----------------------------------------------------------------------*/
/* 88VRAMメモリのポインタを定義する					*/
/*		描画時は、main_vram					*/
/*----------------------------------------------------------------------*/
#if	defined( HIRESO )	/*===========================*/
#define	SRC_DEFINE()	bit32	*src = main_vram4;
#endif


/*----------------------------------------------------------------------*/
/* スクリーンバッファのポインタを定義する				*/
/*		表示サイズによって、異なる。				*/
/*----------------------------------------------------------------------*/
#if	defined( FULL ) || defined( HALF )
#define	DST_DEFINE()	TYPE	*dst   = (TYPE *)screen_start;
#elif	defined( DOUBLE )
#define	DST_DEFINE()	TYPE	*dst   = (TYPE *)screen_start;		\
			TYPE	*dst2  = dst  + WIDTH;
#endif

/*----------------------------------------------------------------------*/
/* その他のワークを定義する						*/
/*----------------------------------------------------------------------*/
#if	defined( HIRESO )	/*===========================*/

#define	WORK_DEFINE()				\
  int	m, l;					\
  int	line = 0;				\
  bit32	vram;

#endif				/*===========================*/





/*----------------------------------------------------------------------*/
/* 400ライン描画共通定義						*/
/*----------------------------------------------------------------------*/
#define	SET_WORK( x, y )			\
  x = crtc_font_height*2;			\
  y = (crtc_font_height*2-1)*80;

#define	IF_400_ALTERNATE__FOLLOWING()		if( (k&1)==0 )



/************************************************************************/
/* 400ライン描画							*/
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

#undef	IF_400_ALTERNATE__FOLLOWING
#define	IF_400_ALTERNATE__FOLLOWING()		if( (k&1)==1 ) continue;

#define	MASK_8DOT()	for( m=0; m<4; m++ ) dst[m] = tcol;
#define	MASK_16DOT()	for( m=0; m<8; m++ ) dst[m] = tcol;

#define	COPY_8DOT()	/* nothing */
#define	COPY_16DOT()	/* nothing */

#if	defined( HIRESO )	/*===========================*/

#define	TRANS_8DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram  = *(src + k*80);					      \
		for( l=0; l<4; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram  = *(src + k*80 -80*200);				      \
		for( l=0; l<4; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
	      }

#define	STORE_8DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram  = *(src + k*80);					      \
		for( m=0xc0, l=0; l<4; l++, m>>=2, vram<<=2 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_B( vram, tcol );     \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram  = *(src + k*80 -80*200);				      \
		for( m=0xc0, l=0; l<4; l++, m>>=2, vram<<=2 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_R( vram, tcol );     \
		}							      \
	      }

#define	TRANS_16DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram = *(src + k*80);					      \
		for( l=0; l<4; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
		vram = *(src + k*80+1);					      \
		for(    ; l<8; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram = *(src + k*80 -80*200);				      \
		for( l=0; l<4; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
		vram = *(src + k*80+1 -80*200);				      \
		for(    ; l<8; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
	      }

#define	STORE_16DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram = *(src + k*80);					      \
		for( m=0x80, l=0; l<4; l++, m>>=1, vram<<=2 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_B( vram, tcol );     \
		}							      \
		vram = *(src + k*80+1);					      \
		for(            ; l<8; l++, m>>=1, vram<<=2 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_B( vram, tcol );     \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram = *(src + k*80 -80*200);				      \
		for( m=0x80, l=0; l<4; l++, m>>=1, vram<<=2 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_R( vram, tcol );     \
		}							      \
		vram = *(src + k*80+1 -80*200);				      \
		for(            ; l<8; l++, m>>=1, vram<<=2 ){		      \
		  if( style & m ) dst[l] =  tcol;			      \
		  else            dst[l] = get_pixel_400_R( vram, tcol );     \
		}							      \
	      }


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
      line += crtc_font_height*2;		\
      src  += (crtc_font_height*2-1)*80;

#define	DST_NEXT_LINE()							\
      dst  += SCREEN_OFFSET + (crtc_font_height-1)*WIDTH;




#elif	defined( FULL )		/*-------------------------------------------*/

#define	MASK_8DOT()	for( m=0; m< 8; m++ ) dst[m] = tcol;
#define	MASK_16DOT()	for( m=0; m<16; m++ ) dst[m] = tcol;

#define	COPY_8DOT()	/* nothing */
#define	COPY_16DOT()	/* nothing */

#if	defined( HIRESO )	/*===========================*/

#define	TRANS_8DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram  = *(src + k*80);					      \
		for( l=0; l<8; l++, vram<<=1 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram  = *(src + k*80 -80*200);				      \
		for( l=0; l<8; l++, vram<<=1 ){				      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
	      }

#define	STORE_8DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram  = *(src + k*80);					      \
		for( m=0x80, l=0; l<8; l++, m>>=1, vram<<=1 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_B( vram, tcol );     \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram  = *(src + k*80 -80*200);				      \
		for( m=0x80, l=0; l<8; l++, m>>=1, vram<<=1 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_R( vram, tcol );     \
		}							      \
	      }

#define	TRANS_16DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram = *(src + k*80);					      \
		for( l=0; l<8; l++, vram<<=1 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
		vram = *(src + k*80+1);					      \
		for(    ; l<16; l++, vram<<=1 ){			      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram = *(src + k*80 -80*200);				      \
		for( l=0; l<8; l++, vram<<=1 ){				      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
		vram = *(src + k*80+1 -80*200);				      \
		for(    ; l<16; l++, vram<<=1 ){			      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
	      }

#define	STORE_16DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram = *(src + k*80);					      \
		for( m=0x80, l=0; l<8; l+=2, m>>=1, vram<<=2 ){		      \
		  if( style & m ){dst[l]  =dst[l+1] = tcol; }		      \
		  else           {dst[l]  =get_pixel_400_B( vram,    tcol );  \
				  dst[l+1]=get_pixel_400_B( vram<<1, tcol );} \
		}							      \
		vram = *(src + k*80+1);					      \
		for(            ; l<16; l+=2, m>>=1, vram<<=2 ){	      \
		  if( style & m ){dst[l]  =dst[l+1] = tcol; }		      \
		  else           {dst[l]  =get_pixel_400_B( vram,    tcol );  \
				  dst[l+1]=get_pixel_400_B( vram<<1, tcol );} \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram = *(src + k*80 -80*200);				      \
		for( m=0x80, l=0; l<8; l+=2, m>>=1, vram<<=2 ){		      \
		  if( style & m ){dst[l]  =dst[l+1] = tcol; }		      \
		  else           {dst[l]  =get_pixel_400_R( vram,    tcol );  \
				  dst[l+1]=get_pixel_400_R( vram<<1, tcol );} \
		}							      \
		vram = *(src + k*80+1 -80*200);				      \
		for(            ; l<16; l+=2, m>>=1, vram<<=2 ){	      \
		  if( style & m ){dst[l]  =dst[l+1] = tcol; }		      \
		  else           {dst[l]  =get_pixel_400_R( vram,    tcol );  \
				  dst[l+1]=get_pixel_400_R( vram<<1, tcol );} \
		}							      \
	      }


#endif				/*===========================*/

#define	DST_FORWARD()				\
	    dst += WIDTH;

#define	DST_BACKWARD()				\
	  dst  += -crtc_font_height*2*WIDTH;


#define	SRC_NEXT_CHARA()			\
	src ++;

#define	DST_NEXT_CHARA()			\
	dst += 8;


#define	SRC_NEXT_LINE()				\
      line += crtc_font_height*2;		\
      src  += (crtc_font_height*2-1)*80;

#define	DST_NEXT_LINE()							\
      dst  += SCREEN_OFFSET + (crtc_font_height*2-1)*WIDTH;


#elif	defined( DOUBLE )	/*-------------------------------------------*/


#define	MASK_8DOT()	for( m=0; m<16; m++ ) dst[m] = tcol;
#define	MASK_16DOT()	for( m=0; m<32; m++ ) dst[m] = tcol;

#define	COPY_8DOT()	memcpy( dst2, dst, sizeof(TYPE)*16 );
#define	COPY_16DOT()	memcpy( dst2, dst, sizeof(TYPE)*32 );

#if	defined( HIRESO )	/*===========================*/

#define	TRANS_8DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram  = *(src + k*80);					      \
		for( l=0; l<16; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_B( vram, tcol );	      \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram  = *(src + k*80 -80*200);				      \
		for( l=0; l<16; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_R( vram, tcol );	      \
		}							      \
	      }

#define	STORE_8DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram  = *(src + k*80);					      \
		for( m=0x80, l=0; l<16; l+=2, m>>=1, vram<<=1 ){	      \
		  if( style & m ) dst[l]=dst[l+1]=tcol;			      \
		  else            dst[l]=dst[l+1]=get_pixel_400_B(vram,tcol); \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram  = *(src + k*80 -80*200);				      \
		for( m=0x80, l=0; l<16; l+=2, m>>=1, vram<<=1 ){	      \
		  if( style & m ) dst[l]=dst[l+1]=tcol;			      \
		  else            dst[l]=dst[l+1]=get_pixel_400_R(vram,tcol); \
		}							      \
	      }

#define	TRANS_16DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram = *(src + k*80);					      \
		for( l=0; l<16; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_B( vram, tcol );	      \
		}							      \
		vram = *(src + k*80+1);					      \
		for(    ; l<32; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_B( vram, tcol );	      \
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram = *(src + k*80 -80*200);				      \
		for( l=0; l<16; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_R( vram, tcol );	      \
		}							      \
		vram = *(src + k*80+1 -80*200);				      \
		for(    ; l<32; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_R( vram, tcol );	      \
		}							      \
	      }

#define	STORE_16DOT()							      \
	      if( line + k < 200 ){		/* 画面上部200ラインの場合 */ \
		vram = *(src + k*80);					      \
		for( m=0x80, l=0; l<16; l+=4, m>>=1, vram<<=2 ){	      \
		 if(style&m){dst[l]  =dst[l+1]=dst[l+2]=dst[l+3]= tcol;      }\
		 else       {dst[l]  =dst[l+1]=get_pixel_400_B(vram,   tcol); \
		             dst[l+2]=dst[l+3]=get_pixel_400_B(vram<<1,tcol);}\
		}							      \
		vram = *(src + k*80+1);					      \
		for(            ; l<32; l+=4, m>>=1, vram<<=2 ){	      \
		 if(style&m){dst[l]  =dst[l+1]=dst[l+2]=dst[l+3]= tcol;      }\
		 else       {dst[l]  =dst[l+1]=get_pixel_400_B(vram,   tcol); \
		             dst[l+2]=dst[l+3]=get_pixel_400_B(vram<<1,tcol);}\
		}							      \
	      }else{				/* 画面下部200ラインの場合 */ \
		vram = *(src + k*80 -80*200);				      \
		for( m=0x80, l=0; l<16; l+=4, m>>=1, vram<<=2 ){	      \
		 if(style&m){dst[l]  =dst[l+1]=dst[l+2]=dst[l+3]= tcol;      }\
		 else       {dst[l]  =dst[l+1]=get_pixel_400_R(vram,   tcol); \
		             dst[l+2]=dst[l+3]=get_pixel_400_R(vram<<1,tcol);}\
		}							      \
		vram = *(src + k*80+1 -80*200);				      \
		for(            ; l<32; l+=4, m>>=1, vram<<=2 ){	      \
		 if(style&m){dst[l]  =dst[l+1]=dst[l+2]=dst[l+3]= tcol;      }\
		 else       {dst[l]  =dst[l+1]=get_pixel_400_R(vram,   tcol); \
		             dst[l+2]=dst[l+3]=get_pixel_400_R(vram<<1,tcol);}\
		}							      \
	      }


#endif				/*===========================*/

#define	DST_FORWARD()				\
	    dst += 2*WIDTH;	dst2 += 2*WIDTH;

#define	DST_BACKWARD()				\
	  dst  += -crtc_font_height*4*WIDTH;	\
	  dst2 += -crtc_font_height*4*WIDTH;


#define	SRC_NEXT_CHARA()			\
	src ++;

#define	DST_NEXT_CHARA()			\
	dst += 16;	dst2 += 16;


#define	SRC_NEXT_LINE()				\
      line += crtc_font_height*2;		\
      src  += (crtc_font_height*2-1)*80;

#define	DST_NEXT_LINE()							\
      dst  += SCREEN_OFFSET + (crtc_font_height*4-1)*WIDTH;		\
      dst2 += SCREEN_OFFSET + (crtc_font_height*4-1)*WIDTH;


#endif				/*-------------------------------------------*/


/*****************************************************************************/
#include "screen-base.h"
