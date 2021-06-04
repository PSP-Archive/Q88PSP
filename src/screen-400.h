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
/* 88VRAM�������p���b�g�����A�X�N���[���o�b�t�@�̐F���ɕϊ�		*/
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
/* 88VRAM�������̃|�C���^���`����					*/
/*		�`�掞�́Amain_vram					*/
/*----------------------------------------------------------------------*/
#if	defined( HIRESO )	/*===========================*/
#define	SRC_DEFINE()	bit32	*src = main_vram4;
#endif


/*----------------------------------------------------------------------*/
/* �X�N���[���o�b�t�@�̃|�C���^���`����				*/
/*		�\���T�C�Y�ɂ���āA�قȂ�B				*/
/*----------------------------------------------------------------------*/
#if	defined( FULL ) || defined( HALF )
#define	DST_DEFINE()	TYPE	*dst   = (TYPE *)screen_start;
#elif	defined( DOUBLE )
#define	DST_DEFINE()	TYPE	*dst   = (TYPE *)screen_start;		\
			TYPE	*dst2  = dst  + WIDTH;
#endif

/*----------------------------------------------------------------------*/
/* ���̑��̃��[�N���`����						*/
/*----------------------------------------------------------------------*/
#if	defined( HIRESO )	/*===========================*/

#define	WORK_DEFINE()				\
  int	m, l;					\
  int	line = 0;				\
  bit32	vram;

#endif				/*===========================*/





/*----------------------------------------------------------------------*/
/* 400���C���`�拤�ʒ�`						*/
/*----------------------------------------------------------------------*/
#define	SET_WORK( x, y )			\
  x = crtc_font_height*2;			\
  y = (crtc_font_height*2-1)*80;

#define	IF_400_ALTERNATE__FOLLOWING()		if( (k&1)==0 )



/************************************************************************/
/* 400���C���`��							*/
/*		MASK_8DOT()	 �c�c�c  8�h�b�g�`�� (TEXT �̂݁j	*/
/*		MASK_16DOT()	 �c�c�c 16�h�b�g�`�� (TEXT �̂݁j	*/
/*		TRANS_8DOT()	 �c�c�c  8�h�b�g�`�� (VRAM �̂݁j	*/
/*		TRANS_16DOT()	 �c�c�c 16�h�b�g�`�� (VRAM �̂݁j	*/
/*		STORE_8DOT()	 �c�c�c  8�h�b�g�`�� (TEXT+VRAM�j	*/
/*		STORE_16DOT()	 �c�c�c 16�h�b�g�`�� (TEXT+VRAM�j	*/
/*		COPY_8DOT()	 �c�c�c  8�h�b�g���� (���Ԃ̃��C������)	*/
/*		COPY_16DOT()	 �c�c�c 16�h�b�g���� (���Ԃ̃��C������)	*/
/*		DST_FORWARD()    �c�c�c 8(16)�h�b�g�`���A�����C����	*/
/*		DST_BACKWARD()   �c�c�c 1�L�������`���A���̃��C����	*/
/*		SRC_NEXT_CHARA() �c�c�c 1�L�������`���A���̕�����	*/
/*		DST_NEXT_CHARA() �c�c�c 1�L�������`���A���̕�����	*/
/*		SRC_NEXT_LINE()  �c�c�c ���ŏI�L�����`��� ���̕����s��	*/
/*		DST_NEXT_LINE()  �c�c�c ���ŏI�L�����`��� ���̕����s��	*/
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
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram  = *(src + k*80);					      \
		for( l=0; l<4; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
		vram  = *(src + k*80 -80*200);				      \
		for( l=0; l<4; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
	      }

#define	STORE_8DOT()							      \
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram  = *(src + k*80);					      \
		for( m=0xc0, l=0; l<4; l++, m>>=2, vram<<=2 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_B( vram, tcol );     \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
		vram  = *(src + k*80 -80*200);				      \
		for( m=0xc0, l=0; l<4; l++, m>>=2, vram<<=2 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_R( vram, tcol );     \
		}							      \
	      }

#define	TRANS_16DOT()							      \
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram = *(src + k*80);					      \
		for( l=0; l<4; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
		vram = *(src + k*80+1);					      \
		for(    ; l<8; l++, vram<<=2 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
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
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
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
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
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
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram  = *(src + k*80);					      \
		for( l=0; l<8; l++, vram<<=1 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
		vram  = *(src + k*80 -80*200);				      \
		for( l=0; l<8; l++, vram<<=1 ){				      \
		  dst[l] = get_pixel_400_R( vram, tcol );		      \
		}							      \
	      }

#define	STORE_8DOT()							      \
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram  = *(src + k*80);					      \
		for( m=0x80, l=0; l<8; l++, m>>=1, vram<<=1 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_B( vram, tcol );     \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
		vram  = *(src + k*80 -80*200);				      \
		for( m=0x80, l=0; l<8; l++, m>>=1, vram<<=1 ){		      \
		  if( style & m ) dst[l] = tcol;			      \
		  else            dst[l] = get_pixel_400_R( vram, tcol );     \
		}							      \
	      }

#define	TRANS_16DOT()							      \
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram = *(src + k*80);					      \
		for( l=0; l<8; l++, vram<<=1 ){				      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
		vram = *(src + k*80+1);					      \
		for(    ; l<16; l++, vram<<=1 ){			      \
		  dst[l] = get_pixel_400_B( vram, tcol );		      \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
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
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
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
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
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
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram  = *(src + k*80);					      \
		for( l=0; l<16; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_B( vram, tcol );	      \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
		vram  = *(src + k*80 -80*200);				      \
		for( l=0; l<16; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_R( vram, tcol );	      \
		}							      \
	      }

#define	STORE_8DOT()							      \
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram  = *(src + k*80);					      \
		for( m=0x80, l=0; l<16; l+=2, m>>=1, vram<<=1 ){	      \
		  if( style & m ) dst[l]=dst[l+1]=tcol;			      \
		  else            dst[l]=dst[l+1]=get_pixel_400_B(vram,tcol); \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
		vram  = *(src + k*80 -80*200);				      \
		for( m=0x80, l=0; l<16; l+=2, m>>=1, vram<<=1 ){	      \
		  if( style & m ) dst[l]=dst[l+1]=tcol;			      \
		  else            dst[l]=dst[l+1]=get_pixel_400_R(vram,tcol); \
		}							      \
	      }

#define	TRANS_16DOT()							      \
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
		vram = *(src + k*80);					      \
		for( l=0; l<16; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_B( vram, tcol );	      \
		}							      \
		vram = *(src + k*80+1);					      \
		for(    ; l<32; l+=2, vram<<=1 ){			      \
		  dst[l] = dst[l+1] = get_pixel_400_B( vram, tcol );	      \
		}							      \
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
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
	      if( line + k < 200 ){		/* ��ʏ㕔200���C���̏ꍇ */ \
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
	      }else{				/* ��ʉ���200���C���̏ꍇ */ \
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
