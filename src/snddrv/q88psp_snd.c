/************************************************************************/
/*									*/
/* サウンドの処理 (ここから、XMAMEの関数を呼び出す)			*/
/*									*/
/*	すべて、XMAME の処理を行なう関数				*/
/*	QUASI88 は、ここの関数を通して、XMAMEのサウンド処理を行なう	*/
/*									*/
/*	ファイル後半は、MAME が呼び出す osd_xxx 関数が記述されいてる。	*/
/*	この osd_xxx は XMAME のソースを改造して作成された。		*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __XMAME_QUASI88_C_
#include "xmame-quasi88.h"

#ifdef	USE_SOUND

#include "fm.h"			/* YM2203TimerOver()	*/
#include "q88psp_snd.h"


int	attenuation = 0;	/* ボリューム -32～0 [db] 現在未サポート */
int	sample_rate = DEF_SAMPRATE;	/* sampling rate [Hz]	*/

int	fmvol     = 100;	/* level of FM    (0-100)[%] */
int	psgvol    =  20;	/* level of PSG   (0-100)[%] */
int	beepvol   =  30;	/* level of BEEP  (0-100)[%] */
int	rhythmvol =  90;	/* level of RHYTHM(0-100)[%] depend on fmvol */
int	adpcmvol  =  90;	/* level of ADPCM (0-100)[%] depend on fmvol */
int	close_device = FALSE;	/* close sound device at menu mode */

T_XMAME_FUNC	*xmame_func      = &xmame_func_nosound;


int	use_sound = TRUE;	/* 0:not use / 1 use sound	*/

short snd_buffer[SND_BUFLEN];
int snd_ptr = 0;

/****************************************************************/
/* XMAME のサウンドを使用するための初期化／終了関数		*/
/****************************************************************/
int	xmame_system_init( void )
{
  snd_ptr = 0;
  return OSD_OK;
}
void	xmame_system_term( void )
{
}


/****************************************************************/
enum {
  OPT_SOUND, OPT_NOSOUND,
  OPT_VOLUME, OPT_FMVOL, OPT_PSGVOL, OPT_BEEPVOL,
  OPT_RHYTHMVOL, OPT_ADPCMVOL,
  OPT_SAMPLEFREQ, OPT_SDLBUFSIZE, OPT_SDLBUFNUM,
  OPT_CLOSE, OPT_NOCLOSE, OPT_FMGEN, OPT_NOFMGEN,
  EndofOPT
};

static struct {
  int		num;
  const	char	*str;
} opt_index[] =
{
  { OPT_SOUND,     "sound",     },
  { OPT_NOSOUND,   "nosound",   },
  { OPT_VOLUME,    "volume",    },
  { OPT_FMVOL,     "fmvol",     },
  { OPT_PSGVOL,    "psgvol",    },
  { OPT_BEEPVOL,   "beepvol",   },
  { OPT_RHYTHMVOL, "rhythmvol", },
  { OPT_ADPCMVOL,  "adpcmvol",  },
  { OPT_SAMPLEFREQ,"samplefreq",},
  { OPT_SDLBUFSIZE,"sdlbufsize",},
  { OPT_SDLBUFNUM, "sdlbufnum",	},
  { OPT_CLOSE,     "close",     },
  { OPT_NOCLOSE,   "noclose",   },
  { OPT_FMGEN,     "fmgen",     },
  { OPT_NOFMGEN,   "nofmgen",   },
};

#define	ERR_MES1( fmt )						\
		do{						\
		  fprintf( stderr, "error : %s\n", fmt );	\
		}while(0)

#define	ERR_MES2( fmt, arg )					\
		do{						\
		  fprintf( stderr, "error : " );		\
		  fprintf( stderr, fmt, arg );			\
		  fprintf( stderr, "\n" );			\
		}while(0)

#define	CONV_TO_INT(opt2)	strtol( opt2, &conv_end, 0 )
#define CONV_ERR()		(*conv_end!='\0')
#define CONV_OK()		(*conv_end=='\0')

#define	SET_OPT2_NUM_TO_WORK( work, low, high, mes_overflow, mes_no_opt )\
		do{							\
		  if( opt2 ){						\
		    ret_val ++;						\
		    work = CONV_TO_INT(opt2);				\
		    if( CONV_ERR() || work < low || high < work ){	\
		      ERR_MES1( mes_overflow );				\
		      return -1;					\
		    }							\
		  }else{						\
		    ERR_MES1( mes_no_opt );				\
		    return -1;						\
		  }							\
		}while(0)


/****************************************************************/
/* XMAME のオプションを解析するための関数			*/
/* -1 引数が足りないか、範囲外					*/
/* 0  該当オプションなし					*/
/* 1  引数1個処理 (優先度により処理せずも含む)			*/
/* 2  引数2個処理 (優先度により処理せずも含む)			*/
/****************************************************************/
int	xmame_check_option( char *opt1, char *opt2, int priority )
{
 return 0;
}




/****************************************************************/
/* XMAME のオプションを表示するための関数			*/
/****************************************************************/
void	xmame_show_option( void )
{
}




/****************************************************************/
/* XMAME のサウンドの開始					*/
/* XMAME のサウンドの更新					*/
/* XMAME のサウンドの終了					*/
/* XMAME のサウンドの中断					*/
/* XMAME のサウンドの再開					*/
/* XMAME のサウンドのリセット					*/
/****************************************************************/
static	int	sound_stream = FALSE;

int	xmame_sound_start( void )
{
  xmame_func_set(0);

  if( use_sound ){ sound_enabled = TRUE;  sound_stream = TRUE; }
  else           { sound_enabled = FALSE; sound_stream = FALSE;  return 1; }


  if(verbose_proc) printf("Starting sound server: ");

  if( sound_start()==0 ){

    sound_enabled = TRUE;
    xmame_func_set(sound_enabled);
    if(verbose_proc) printf("OK\n");
    sound_reset();
    osd_sound_enable(1);

  }else{

    sound_enabled = FALSE;
    xmame_func_set(sound_enabled);
    if(verbose_proc) printf("FAILED...Can't use sound\n");

  }
  return 1;
}

void	xmame_sound_update( void )
{
  if( sound_enabled ){
    sound_update();
  }
}

void	xmame_sound_stop( void )
{
  if( sound_enabled ){
    sound_stop();
  }
}

void	xmame_sound_suspend( void )
{
  if( close_device ){
    osd_sound_enable(0);
  }

}
void	xmame_sound_resume( void )
{
  if( close_device ){
    osd_sound_enable(1);
  }
}
void	xmame_sound_reset( void )
{
  if( sound_enabled )
    sound_reset();
}

int	xmame_sound_is_enable( void )
{
  if( sound_stream ) return TRUE;
  else               return FALSE;
}

void	xmame_update_video_and_audio( void )
{
  /* nothing */
}



/****************************************************************/
/* サウンドポート入出力毎に呼ぶ					*/
/****************************************************************/
byte	xmame_sound_in_data( void )
{
  if( xmame_func->sound_in_data ) return (xmame_func->sound_in_data)(0);
  else                            return 0xff;
}
byte	xmame_sound_in_status( void )
{
  if( xmame_func->sound_in_data ) return (xmame_func->sound_in_status)(0);
  else                            return 0;
}
void	xmame_sound_out_reg( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound_out_reg)(0,data);
}
void	xmame_sound_out_data( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound_out_data)(0,data);
}


byte	xmame_sound2_in_data( void )
{
  if( !use_sound ) return 0xff;
  else{
    if( sound_board==SOUND_I ) return 0xff;
    else                       return 0;
  }
}
byte	xmame_sound2_in_status( void )
{
  if( xmame_func->sound_in_data ) return (xmame_func->sound2_in_status)(0);
  else                            return 0xff;
}
void	xmame_sound2_out_reg( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound2_out_reg)(0,data);
}
void	xmame_sound2_out_data( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound2_out_data)(0,data);
}


void	xmame_beep_out_data( byte data )
{
  if( xmame_func->beep_out_data ) (xmame_func->beep_out_data)(0,data);
}




/****************************************************************/
/* サウンドのタイマーオーバーフロー時に呼ぶ			*/
/*	timer = 0 TimerAOver / 1 TimerBOver			*/
/****************************************************************/
void	xmame_sound_timer_over( int timer )
{
  if( xmame_func->sound_timer_over ) (xmame_func->sound_timer_over)(0, timer);
}




/****************************************************************/
/* ボリューム取得						*/
/*	現在の音量を取得する					*/
/****************************************************************/
int	xmame_get_sound_volume( void )
{
  return osd_get_mastervolume();
}

/****************************************************************/
/* ボリューム変更						*/
/*	引数に、音量を与える。音量は、-32～0 まで		*/
/****************************************************************/
void	xmame_set_sound_volume( int vol )
{
    if( vol > VOL_MAX ) vol = VOL_MAX;
    if( vol < VOL_MIN ) vol = VOL_MIN;
    osd_set_mastervolume( vol );
}



/****************************************************************/
/* チャンネル別レベル取得					*/
/*	引数に、チャンネルを与える				*/
/*	チャンネルは、0==PSG、 1==FM、2==BEEP			*/
/*	※ xmame 内部でのチャンネル				*/
/*	    サウンドボードI	CH0～2  PSG			*/
/*				CH3	FM      		*/
/*				CH4	BEEP      		*/
/*	    サウンドボードII	CH0～2  PSG			*/
/*				CH3	FM(L)			*/
/*				CH4	FM(R)			*/
/*				CH5	BEEP			*/
/****************************************************************/
int	xmame_get_mixer_volume( int ch )
{
  if( use_fmgen==0 ){
    if( sound_board==SOUND_I ){
      switch( ch ){
      case XMAME_MIXER_PSG:	ch = 0;		break;
      case XMAME_MIXER_FM:	ch = 3;		break;
      case XMAME_MIXER_BEEP:	ch = 4;		break;
      case XMAME_MIXER_RHYTHM:	return 0;
      case XMAME_MIXER_ADPCM:	return 0;
      default:			return 0;
      }
    }else{
      switch( ch ){
      case XMAME_MIXER_PSG:	ch = 0;		break;
      case XMAME_MIXER_FM:	ch = 3;		break;
      case XMAME_MIXER_BEEP:	ch = 5;		break;
      case XMAME_MIXER_RHYTHM:	return rhythmvol;
      case XMAME_MIXER_ADPCM:	return adpcmvol;
      default:			return 0;
      }
    }
  }else{
    switch( ch ){
    case XMAME_MIXER_FMPSG:	ch = 0;		break;
    case XMAME_MIXER_BEEP:	ch = 2;		break;
    default:			return 0;
    }
  }
  return mixer_get_mixing_level( ch );
}

/****************************************************************/
/* チャンネル別レベル変更					*/
/*	引数に、チャンネルとレベルを与える			*/
/*	レベルは、    0～100 まで				*/
/*	チャンネルは、0==PSG、 1==FM、2==BEEP			*/
/*				それ以外は一覧を表示する	*/
/*	※ xmame 内部でのチャンネル				*/
/*	    サウンドボードI	CH0～2  PSG			*/
/*				CH3	FM      		*/
/*				CH4	BEEP      		*/
/*	    サウンドボードII	CH0～2  PSG			*/
/*				CH3	FM(L)			*/
/*				CH4	FM(R)			*/
/*				CH5	BEEP			*/
/*	    FMGENの場合		CH0	PSG/FM(L)		*/
/*				CH2	PSG/FM(R)		*/
/*				CH2	BEEP      		*/
/****************************************************************/
void	xmame_set_mixer_volume( int ch, int level )
{
  if( use_fmgen ){
  switch( ch ){
  case XMAME_MIXER_FMPSG:
    if( level < FMPSGVOL_MIN ) level = FMPSGVOL_MIN;
    if( level > FMPSGVOL_MAX ) level = FMPSGVOL_MAX;
    mixer_set_mixing_level( 0, level );
    mixer_set_mixing_level( 1, level );
    break;
  case XMAME_MIXER_BEEP:
    if( level < BEEPVOL_MIN ) level = BEEPVOL_MIN;
    if( level > BEEPVOL_MAX ) level = BEEPVOL_MAX;
    mixer_set_mixing_level( 2, level );
    break;
  default:
    for( ch=0; ch<MIXER_MAX_CHANNELS ; ch++ ){
      const char *name = mixer_get_name(ch);
      if(name) printf( "%d[ch] %s\t:%d\n", ch,name,mixer_get_mixing_level(ch));
    }
    break;
  }
  return;
  }

  switch( ch ){
  case XMAME_MIXER_PSG:
    if( level < PSGVOL_MIN ) level = PSGVOL_MIN;
    if( level > PSGVOL_MAX ) level = PSGVOL_MAX;
    mixer_set_mixing_level( 0, level );
    mixer_set_mixing_level( 1, level );
    mixer_set_mixing_level( 2, level );
    break;
    
  case XMAME_MIXER_FM:
    if( level < FMVOL_MIN ) level = FMVOL_MIN;
    if( level > FMVOL_MAX ) level = FMVOL_MAX;
    if( sound_board==SOUND_I ){
      mixer_set_mixing_level( 3, level );
    }else{
      mixer_set_mixing_level( 3, level );
      mixer_set_mixing_level( 4, level );
    }
    break;
    
  case XMAME_MIXER_BEEP:
    if( level < BEEPVOL_MIN ) level = BEEPVOL_MIN;
    if( level > BEEPVOL_MAX ) level = BEEPVOL_MAX;
    if( sound_board==SOUND_I ){
      mixer_set_mixing_level( 4, level );
    }else{
      mixer_set_mixing_level( 5, level );
    }
    break;

  case XMAME_MIXER_RHYTHM:
    if( level < RHYTHMVOL_MIN ) level = RHYTHMVOL_MIN;
    if( level > RHYTHMVOL_MAX ) level = RHYTHMVOL_MAX;
    if( sound_board==SOUND_II ){
      rhythmvol = level;
    }
    break;

  case XMAME_MIXER_ADPCM:
    if( level < ADPCMVOL_MIN ) level = ADPCMVOL_MIN;
    if( level > ADPCMVOL_MAX ) level = ADPCMVOL_MAX;
    if( sound_board==SOUND_II ){
      adpcmvol = level;
    }
    break;

  default:
    for( ch=0; ch<MIXER_MAX_CHANNELS ; ch++ ){
      const char *name = mixer_get_name(ch);
      if(name) printf( "%d[ch] %s\t:%d\n", ch,name,mixer_get_mixing_level(ch));
    }
    break;
  }
}





/*==============================================================*/
/* osd セクション						*/
/*==============================================================*/

static struct sysdep_dsp_struct *sound_dsp = NULL;
static float sound_bufsize = 3.0;
static int sound_samples_per_frame = 0;
static int type;


void osd_set_mastervolume (int attenuation)
{
}


int osd_get_mastervolume (void)
{
  return 0;
}


/*
 * xmame-0.71.1/src/unix/sysdep/sysdep_dsp.c 
 */
static struct sysdep_dsp_struct *sysdep_dsp_create(
	int *samplerate, 	/* sample_rate (==22050) */
	int *type, 		/* */
	float bufsize)		/* 3.0 / 55.4 */
{
   struct sysdep_dsp_struct *dsp = NULL;
   struct sysdep_dsp_create_params params;
   
   return dsp;
}
/*
 * xmame-0.71.1/src/unix/sysdep/sysdep_dsp.c 
 */
static void sysdep_dsp_destroy(struct sysdep_dsp_struct *dsp)
{
}


/*
 * xmame-0.71.1/src/unix/sound.c
 */
int osd_start_audio_stream(int stereo)
{
   type = SYSDEP_DSP_16BIT | (stereo? SYSDEP_DSP_STEREO:SYSDEP_DSP_MONO);
   
   /* create sound_stream */
   if(sound_enabled)
   {
      /* calculate samples_per_frame */
      sound_samples_per_frame = (int)(Machine__sample_rate /
        Machine__drv__frames_per_second);	      /* mean 22050Hz/55.4Hz */
   }

   /* if sound is not enabled, set the samplerate of the core to 0 */

   if(!sound_enabled)
   {
      sound_samples_per_frame = 0;
   }
   
   /* return the samples to play the first frame */
   return sound_samples_per_frame;
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
int osd_update_audio_stream(INT16 *samples)
{
   int i,j,k;
   /* sound enabled ? */
   if (sound_enabled)
   {
   	   if(tmp_cnt != 0){//オーバーラン時は一時バッファに転送バッファのデータを渡しておく
   	   	//tmp_cnt + tmp_ptr = sound_samples_per_frameの予定
			for(k = 0;k < tmp_cnt;k++){//前回あふれたデータをサウンドバッファに送り込む。
			    snd_buffer[snd_ptr] = tmp_buffer[k];
				if (snd_ptr < SND_BUFLEN-1)
					snd_ptr++;
				else
					snd_ptr = 0;
				//ここでオーバーランが出たらあきらめる？
				if(snd_ptr == buf_pbptr) return sound_samples_per_frame;
			}
			for(i = 0; i < tmp_ptr; i++){//要求分だけサウンドバッファに転送
				snd_buffer[snd_ptr] = samples[i];
				if (snd_ptr < SND_BUFLEN-1)
					snd_ptr++;
				else
					snd_ptr = 0;
				//ここでオーバーランが出たら
				if(snd_ptr == buf_pbptr){
					for(j = 0;j < (sound_samples_per_frame - i);j++){//あまりは一時バッファに置いておく
					tmp_buffer[tmp_cnt] = samples[i + j];
					tmp_cnt++;
					}
					tmp_ptr = 1 + tmp_cnt;
					return tmp_ptr;//バッファリングできた分だけ要求
				}
			}
			tmp_cnt = 0;
			tmp_ptr = sound_samples_per_frame;
   	   }else{
		if(tmp_ptr == 0) tmp_ptr = sound_samples_per_frame;
			for(i = 0; i < tmp_ptr; i++)
			{
				snd_buffer[snd_ptr] = samples[i];

				if (snd_ptr < SND_BUFLEN-1)
					snd_ptr++;
				else
					snd_ptr = 0;

			if(snd_ptr == buf_pbptr) break;
/*				if(snd_ptr == buf_pbptr){
					tmp_cnt = 0;
					for(j = 0;j < (sound_samples_per_frame - i);j++){//あまりは一時バッファに置いておく
					tmp_buffer[j] = samples[i + j];
					tmp_cnt++;
					}
					tmp_ptr = 1;
					return tmp_ptr;//バッファリングできた分だけ要求
				}
*/			}
		}
   }
   /* return the samples to play this next frame */
   tmp_ptr = sound_samples_per_frame;
   return sound_samples_per_frame;
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
void osd_stop_audio_stream(void)
{
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
void osd_sound_enable (int enable_it)
{
	if (enable_it)
		sound_enabled = 1;
	else
		sound_enabled = 0;
}


#else		/* !USE_SOUND */

/*
 *  USE_SOUND 未定義にも関わらずソースを組み込んだ場合のリンクエラー回避
 */

int	fmvol;
int	psgvol;
int	beepvol;
int	rhythmvol;
int	adpcmvol;

int osd_start_audio_stream(int stereo){return 0;}
void osd_stop_audio_stream(void){}
int osd_update_audio_stream(short *buffer){return 0;}

#endif
