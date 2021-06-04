/************************************************************************/
/*									*/
/* �X�N���[�� �X�i�b�v�V���b�g �̉摜�C���[�W����			*/
/*									*/
/************************************************************************/

#undef	CHRS_SKIP
#undef	SET_DST_UPDATE
#undef	CHK_DST_UPDATE
#undef	MASK_DOT

#undef	LINE200_OR_ODD_LINE
#undef	FONT_HEIGHT
#undef	LORESO

#undef	WORK_DEFINE
#undef	TRANS_DOT
#undef	STORE_DOT

#undef	vram2snapshot
#undef	get_pixel_index
#undef	make_mask_mono
#undef	get_pixel_mono
#undef	get_pixel_400_B
#undef	get_pixel_400_R



/*----------------------------------------------------------------------*/
/* 88VRAM�������p���b�g�����A�X�N���[���o�b�t�@�̐F���ɕϊ�		*/
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
		((data)&0x00808080) ? (col) : 0
#else
#define	make_mask_mono( mask )						\
	do{								\
	  mask = 0xffffffff;						\
	  if( grph_pile & GRPH_PILE_BLUE  ) mask &= 0x00ffff00;		\
	  if( grph_pile & GRPH_PILE_RED   ) mask &= 0xff00ff00;		\
	  if( grph_pile & GRPH_PILE_GREEN ) mask &= 0xffff0000;		\
	}while(0)
#define	get_pixel_mono( data, col )					\
		((data)&0x80808000) ? (col) : 0
#endif

/*----------------------------------------------------------------------*/

#ifdef LSB_FIRST
#define	get_pixel_400_B( data, col )				\
		((data)&0x00000080) ? (col) : 0
#define	get_pixel_400_R( data, col )				\
		((data)&0x00008000) ? (col) : 0
#else
#define	get_pixel_400_B( data, col )				\
		((data)&0x80000000) ? (col) : 0
#define	get_pixel_400_R( data, col )				\
		((data)&0x00800000) ? (col) : 0
#endif


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/
#if	defined( WIDTH80 )

#define	CHRS_SKIP		1
#define	SET_DST_UPDATE( x )	dst[(x)] = -1
#define	CHK_DST_UPDATE( x )	(dst[(x)] < 0)
#define	MASK_DOT()		for( m=0; m< 8; m++ ) dst[m] = tcol

#elif	defined( WIDTH40 )

#define	CHRS_SKIP		2
#define	SET_DST_UPDATE( x )	dst[(x)] = dst[(x)+8] = -1
#define	CHK_DST_UPDATE( x )	(dst[(x)] < 0 || dst[(x)+8] < 0)
#define	MASK_DOT()		for( m=0; m<16; m++ ) dst[m] = tcol

#else
#error
#endif


#if	defined( COLOR ) || defined( MONO ) || defined( CLEAR )

#define	LINE200_OR_ODD_LINE(line)	(TRUE)
#define	FONT_HEIGHT			crtc_font_height
#define	LORESO				1

#elif	defined( HIRESO )

#define	LINE200_OR_ODD_LINE(line)	((line&1)==0)
#define	FONT_HEIGHT			crtc_font_height*2
#define	LORESO				0

#else
#error
#endif




#if	defined( COLOR )	/* ====== �J���[�̏ꍇ =================*/

#define	WORK_DEFINE()				\
  int	l;					\
  bit32	vram;  					\
  bit32	vcol[4];

#if	defined( WIDTH80 )		/*------------------------------*/

#define	TRANS_DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[0] = vcol[0] >>4;			\
	      dst[1] = vcol[1] >>4;			\
	      dst[2] = vcol[2] >>4;			\
	      dst[3] = vcol[3] >>4;			\
	      dst[4] = vcol[0] & 7;			\
	      dst[5] = vcol[1] & 7;			\
	      dst[6] = vcol[2] & 7;			\
	      dst[7] = vcol[3] & 7;

#define	STORE_DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      if( style & 0x80 ) dst[0] = tcol;		\
	      else               dst[0] = vcol[0] >>4;	\
	      if( style & 0x40 ) dst[1] = tcol;		\
	      else               dst[1] = vcol[1] >>4;	\
	      if( style & 0x20 ) dst[2] = tcol;		\
	      else               dst[2] = vcol[2] >>4;	\
	      if( style & 0x10 ) dst[3] = tcol;		\
	      else               dst[3] = vcol[3] >>4;	\
	      if( style & 0x08 ) dst[4] = tcol;		\
	      else               dst[4] = vcol[0] & 7;	\
	      if( style & 0x04 ) dst[5] = tcol;		\
	      else               dst[5] = vcol[1] & 7;	\
	      if( style & 0x02 ) dst[6] = tcol;		\
	      else               dst[6] = vcol[2] & 7;	\
	      if( style & 0x01 ) dst[7] = tcol;		\
	      else               dst[7] = vcol[3] & 7;


#elif	defined( WIDTH40 )		/*------------------------------*/

#define	TRANS_DOT()					\
	      vram = *(src + k*80);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[0] = vcol[0] >>4;			\
	      dst[1] = vcol[1] >>4;			\
	      dst[2] = vcol[2] >>4;			\
	      dst[3] = vcol[3] >>4;			\
	      dst[4] = vcol[0] & 7;			\
	      dst[5] = vcol[1] & 7;			\
	      dst[6] = vcol[2] & 7;			\
	      dst[7] = vcol[3] & 7;			\
	      vram = *(src + k*80+1);			\
	      vcol[0] = get_pixel_index( vram, 0 );	\
	      vcol[1] = get_pixel_index( vram, 1 );	\
	      vcol[2] = get_pixel_index( vram, 2 );	\
	      vcol[3] = get_pixel_index( vram, 3 );	\
	      dst[ 8] = vcol[0] >>4;			\
	      dst[ 9] = vcol[1] >>4;			\
	      dst[10] = vcol[2] >>4;			\
	      dst[11] = vcol[3] >>4;			\
	      dst[12] = vcol[0] & 7;			\
	      dst[13] = vcol[1] & 7;			\
	      dst[14] = vcol[2] & 7;			\
	      dst[15] = vcol[3] & 7;

#define	STORE_DOT()							  \
	      vram = *(src + k*80);					  \
	      vcol[0] = get_pixel_index( vram, 0 );			  \
	      vcol[1] = get_pixel_index( vram, 1 );			  \
	      vcol[2] = get_pixel_index( vram, 2 );			  \
	      vcol[3] = get_pixel_index( vram, 3 );			  \
	      if( style & 0x80 ){ dst[0] = dst[1] = tcol; }		  \
	      else              { dst[0] = vcol[0] >>4;			  \
	      			  dst[1] = vcol[1] >>4; }		  \
	      if( style & 0x40 ){ dst[2] = dst[3] = tcol; }		  \
	      else              { dst[2] = vcol[2] >>4;			  \
				  dst[3] = vcol[3] >>4; }		  \
	      if( style & 0x20 ){ dst[4] = dst[5] = tcol; }		  \
	      else              { dst[4] = vcol[0] & 7; 		  \
	      			  dst[5] = vcol[1] & 7; }		  \
	      if( style & 0x10 ){ dst[6] = dst[7] = tcol; }		  \
	      else              { dst[6] = vcol[2] & 7;			  \
	      			  dst[7] = vcol[3] & 7; }		  \
	      vram = *(src + k*80+1);					  \
	      vcol[0] = get_pixel_index( vram, 0 );			  \
	      vcol[1] = get_pixel_index( vram, 1 );			  \
	      vcol[2] = get_pixel_index( vram, 2 );			  \
	      vcol[3] = get_pixel_index( vram, 3 );			  \
	      if( style & 0x08 ){ dst[ 8] = dst[ 9] = tcol; }		  \
	      else              { dst[ 8] = vcol[0] >>4;		  \
	      			  dst[ 9] = vcol[1] >>4; }		  \
	      if( style & 0x04 ){ dst[10] = dst[11] = tcol; }		  \
	      else              { dst[10] = vcol[2] >>4;		  \
				  dst[11] = vcol[3] >>4; }		  \
	      if( style & 0x02 ){ dst[12] = dst[13] = tcol; }		  \
	      else              { dst[12] = vcol[0] & 7;		  \
	      			  dst[13] = vcol[1] & 7; }		  \
	      if( style & 0x01 ){ dst[14] = dst[15] = tcol; }		  \
	      else              { dst[14] = vcol[2] & 7;		  \
	      			  dst[15] = vcol[3] & 7; }

#endif

#elif	defined( MONO )		/* ====== �����̏ꍇ ===================*/

#define	WORK_DEFINE()				\
  int	l;					\
  bit32	vram;					\
  bit32	mask;					\
  make_mask_mono( mask );

#if	defined( WIDTH80 )		/*------------------------------*/

#define	TRANS_DOT()					\
	      vram  = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<8; l++, vram<<=1 ){		\
		dst[l] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_DOT()							\
	      vram  = *(src + k*80);					\
	      vram &= mask;						\
	      for( m=0x80, l=0; l<8; l++, m>>=1, vram<<=1 ){		\
		if( style & m ) dst[l] = tcol;				\
		else            dst[l] = get_pixel_mono( vram, tcol );	\
	      }

#elif	defined( WIDTH40 )		/*------------------------------*/

#define	TRANS_DOT()					\
	      vram = *(src + k*80);			\
	      vram &= mask;				\
	      for( l=0; l<8; l++, vram<<=1 ){		\
		dst[l] = get_pixel_mono( vram, tcol );	\
	      }						\
	      vram = *(src + k*80+1);			\
	      vram &= mask;				\
	      for(    ; l<16; l++, vram<<=1 ){		\
		dst[l] = get_pixel_mono( vram, tcol );	\
	      }

#define	STORE_DOT()							      \
	      vram = *(src + k*80);					      \
	      vram &= mask;						      \
	      for( m=0x80, l=0; l<8; l+=2, m>>=1, vram<<=2 ){		      \
		if( style & m ){ dst[l]   = dst[l+1] = tcol; }		      \
		else           { dst[l]   = get_pixel_mono( vram,    tcol );  \
				 dst[l+1] = get_pixel_mono( vram<<1, tcol ); }\
	      }								      \
	      vram = *(src + k*80+1);					      \
	      vram &= mask;						      \
	      for(            ; l<16; l+=2, m>>=1, vram<<=2 ){		      \
		if( style & m ){ dst[l]   = dst[l+1] = tcol; }		      \
		else           { dst[l]   = get_pixel_mono( vram,    tcol );  \
				 dst[l+1] = get_pixel_mono( vram<<1, tcol ); }\
	      }

#endif

#elif	defined( HIRESO )	/* ====== ���𑜓x�̏ꍇ ===============*/

#define	WORK_DEFINE()				\
  int	l;					\
  bit32	vram;

#if	defined( WIDTH80 )		/*------------------------------*/

#define	TRANS_DOT()							      \
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

#define	STORE_DOT()							      \
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

#elif	defined( WIDTH40 )		/*------------------------------*/

#define	TRANS_DOT()							      \
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

#define	STORE_DOT()							      \
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

#endif

#elif	defined( CLEAR )	/* ====== ��\���̏ꍇ =================*/

#define	WORK_DEFINE()				\
  int	l;					\
  bit32	vcol;

#if	defined( WIDTH80 )		/*------------------------------*/

#define	TRANS_DOT()							\
	      vcol = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;			\
	      for( m=0; m<8; m++ ) dst[m] = vcol;

#define	STORE_DOT()							\
	      vcol = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;			\
	      if( style & 0x80 ) dst[0] = tcol;  else dst[0] = vcol;	\
	      if( style & 0x40 ) dst[1] = tcol;  else dst[1] = vcol;	\
	      if( style & 0x20 ) dst[2] = tcol;  else dst[2] = vcol;	\
	      if( style & 0x10 ) dst[3] = tcol;  else dst[3] = vcol;	\
	      if( style & 0x08 ) dst[4] = tcol;  else dst[4] = vcol;	\
	      if( style & 0x04 ) dst[5] = tcol;  else dst[5] = vcol;	\
	      if( style & 0x02 ) dst[6] = tcol;  else dst[6] = vcol;	\
	      if( style & 0x01 ) dst[7] = tcol;  else dst[7] = vcol;

#elif	defined( WIDTH40 )		/*------------------------------*/

#define	TRANS_DOT()							\
	      vcol = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;			\
	      for( m=0; m<16; m++ ) dst[m] = vcol;

#define	STORE_DOT()							\
	      vcol = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;			\
	      if( style & 0x80 ) dst[ 0] = dst[ 1] = tcol;		\
	      else               dst[ 0] = dst[ 1] = vcol;		\
	      if( style & 0x40 ) dst[ 2] = dst[ 3] = tcol;		\
	      else               dst[ 2] = dst[ 3] = vcol;		\
	      if( style & 0x20 ) dst[ 4] = dst[ 5] = tcol;		\
	      else               dst[ 4] = dst[ 5] = vcol;		\
	      if( style & 0x10 ) dst[ 6] = dst[ 7] = tcol;		\
	      else               dst[ 6] = dst[ 7] = vcol;		\
	      if( style & 0x08 ) dst[ 8] = dst[ 9] = tcol;		\
	      else               dst[ 8] = dst[ 9] = vcol;		\
	      if( style & 0x04 ) dst[10] = dst[11] = tcol;		\
	      else               dst[10] = dst[11] = vcol;		\
	      if( style & 0x02 ) dst[12] = dst[13] = tcol;		\
	      else               dst[12] = dst[13] = vcol;		\
	      if( style & 0x01 ) dst[14] = dst[15] = tcol;		\
	      else               dst[14] = dst[15] = vcol;

#endif

#else
#error
#endif





/*======================================================================*/
/* TVRAM �� GVRAM ���d�˂��킹�āAsnapshot �𐶐�����			*/
/*======================================================================*/

#if	0
#define	vram2snapshot( mode, size )					      \
void	vram2snapshot_##mode##_width##size( void )			      \
{									      \
  int	i, j, k, m;							      \
  int	line = 0;							      \
  Ushort text, *text_attr = &text_attr_buf[ text_attr_flipflop ][0];	      \
  bit32	*src  = main_vram4;						      \
  char	*dst  = &screen_snapshot[0];					      \
  byte	style = 0;							      \
  int	tcol;								      \
									      \
  WORK_DEFINE();		/* ���[�h�ɂ��K�v�ȃ��[�N���قȂ� */	      \
									      \
									      \
  for( i=0; i<CRTC_SZ_LINES; i++ ){	/* 1�����P�ʂŕ`��f�[�^���쐬���� */ \
					/* �Ȃ̂ŁA�s�~�� ���A���[�v������ */ \
									      \
    for( j=0; j<CRTC_SZ_COLUMNS; j+=CHRS_SKIP ){    /* 40���Ȃ�2�����u���� */ \
									      \
      text = *text_attr;		    /* �e�L�X�g�̕���,�������擾 */   \
	      text_attr += CHRS_SKIP;					      \
									      \
      /* FONT_HEIGHT ��  8 or 10 (200line��25�sor20�s)			      \
	 �Ȃ�����       16 or 20 (400line��25�sor20�s) */		      \
									      \
      set_font_style( text );		/* �t�H���g�p�^�[�������A�F�擾 */    \
      tcol = get_font_color();						      \
									      \
      for( k=0; k<FONT_HEIGHT; k++ ){	/* �t�H���g���������[�v���� */	      \
									      \
	if( LINE200_OR_ODD_LINE(k) )	    /*400line�ŋ������C���� �܂��� */ \
	  style = get_font_style();	    /*200line�� �t�H���g8�h�b�g�擾*/ \
									      \
	if      ( style==0xff ){		    /* TEXT���̂ݍX�V�̎� */  \
	  MASK_DOT();							      \
	}else if( style==0x00 ){		    /* VRAM���̂ݍX�V�̎� */  \
	  TRANS_DOT();							      \
	}else{					    /* TEXT/VRAM�����̎�  */  \
	  STORE_DOT();							      \
	}								      \
									      \
	dst += 640;			    /* �����C���ɐi�� */	      \
      }									      \
      dst += -FONT_HEIGHT * 640;	/* ���C���擪�ɖ߂� */		      \
									      \
      /* ���[�N�����̕����̈ʒu�� */					      \
      src +=   CHRS_SKIP;						      \
      dst += 8*CHRS_SKIP;						      \
    }									      \
									      \
    /* ���[�N�����̍s�̈ʒu�� */					      \
    src  += (FONT_HEIGHT-1)* 80;					      \
    dst  += (FONT_HEIGHT-1)*640;					      \
    line += FONT_HEIGHT;		/* ���̃��[�N�� 400line���̂ݎg�p */  \
  }									      \
}
#else
#define	vram2snapshot( mode, size )					      \
void	vram2snapshot_##mode##_width##size( void )			      \
{									      \
  int	i, j, k, m;							      \
  int	line = 0;							      \
  Ushort text, *text_attr = &text_attr_buf[ text_attr_flipflop ][0];	      \
  bit32	*src  = main_vram4;						      \
  char	*dst  = &screen_snapshot[0];					      \
  byte	style = 0;							      \
  int	tcol;								      \
									      \
  WORK_DEFINE();		/* ���[�h�ɂ��K�v�ȃ��[�N���قȂ� */	      \
									      \
									      \
  for( i=0; i<CRTC_SZ_LINES; i++ ){	/* 1�����P�ʂŕ`��f�[�^���쐬���� */ \
					/* �Ȃ̂ŁA�s�~�� ���A���[�v������ */ \
									      \
    for( j=0; j<CRTC_SZ_COLUMNS; j+=CHRS_SKIP ){    /* 40���Ȃ�2�����u���� */ \
									      \
      text = *text_attr;		    /* �e�L�X�g�̕���,�������擾 */   \
	      text_attr += CHRS_SKIP;					      \
									      \
      /* FONT_HEIGHT ��  8 or 10 (200line��25�sor20�s)			      \
	 �Ȃ�����       16 or 20 (400line��25�sor20�s) */		      \
									      \
      set_font_style( text );		/* �t�H���g�p�^�[�������A�F�擾 */    \
      tcol = get_font_color();						      \
									      \
      for( k=0; k<FONT_HEIGHT; k++ ){	/* �t�H���g���������[�v���� */	      \
									      \
	if( LINE200_OR_ODD_LINE(k) )	    /*400line�ŋ������C���� �܂��� */ \
	  style = get_font_style();	    /*200line�� �t�H���g8�h�b�g�擾*/ \
									      \
	if      ( style==0xff ){		    /* TEXT���̂ݍX�V�̎� */  \
	  MASK_DOT();							      \
	}else if( style==0x00 ){		    /* VRAM���̂ݍX�V�̎� */  \
	  TRANS_DOT();							      \
	}else{					    /* TEXT/VRAM�����̎�  */  \
	  STORE_DOT();							      \
	}								      \
									      \
	dst += 640;			    /* �����C���ɐi�� */	      \
	if( LORESO ){							      \
	  if ( use_interlace ){						      \
	    bit32 b = (grph_ctrl&GRPH_CTRL_COLOR) ?8 :0;		      \
	    if      ( style==0xff ){		    /* TEXT���̂ݍX�V�̎� */  \
	      MASK_DOT();						      \
	    }else if( style==0x00 ){		    /* VRAM���̂ݍX�V�̎� */  \
	      for( l=0; l< 8*CHRS_SKIP; l++ ) dst[l] = b;		      \
	    }else{				    /* TEXT/VRAM�����̎�  */  \
	      if( CHRS_SKIP>1 ){					      \
		for( m=0x80, l=0; l<16; l+=2, m>>=1 ){			      \
		  if( style & m ){ dst[l] = dst[l+1] = tcol; }		      \
		  else           { dst[l] = dst[l+1] = b; }		      \
		}							      \
	      }else{							      \
		for( m=0x80, l=0; l<8; l++, m>>=1 ){			      \
		  if( style & m ){ dst[l] = tcol; }			      \
		  else           { dst[l] = b; }			      \
		}							      \
	      }								      \
	    }								      \
	  }else{							      \
	    memcpy( dst, dst - 640, sizeof(char)*8*CHRS_SKIP );		      \
	  }	  							      \
	  dst += 640;			    /* �����C���ɐi�� */	      \
	}								      \
      }									      \
      dst += -FONT_HEIGHT * ( LORESO + 1 ) * 640;			      \
									      \
      /* ���[�N�����̕����̈ʒu�� */					      \
      src +=   CHRS_SKIP;						      \
      dst += 8*CHRS_SKIP;						      \
    }									      \
									      \
    /* ���[�N�����̍s�̈ʒu�� */					      \
    src  += (FONT_HEIGHT-1)* 80;					      \
    dst  += (FONT_HEIGHT * ( LORESO + 1 )-1)*640;			      \
    line += FONT_HEIGHT;		/* ���̃��[�N�� 400line���̂ݎg�p */  \
  }									      \
}
#endif

