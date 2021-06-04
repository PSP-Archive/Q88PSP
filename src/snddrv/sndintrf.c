#include "driver.h"


/***************************************************************************

  Many games use a master-slave CPU setup. Typically, the main CPU writes
  a command to some register, and then writes to another register to trigger
  an interrupt on the slave CPU (the interrupt might also be triggered by
  the first write). The slave CPU, notified by the interrupt, goes and reads
  the command.

***************************************************************************/

static int cleared_value = 0x00;

static int latch,read_debug;


static void soundlatch_callback(int param)
{
	if (read_debug == 0 && latch != param)
		logerror("Warning: sound latch written before being read. Previous: %02x, new: %02x\n",latch,param);
	latch = param;
	read_debug = 0;
}

WRITE_HANDLER( soundlatch_w )
{
	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,data,soundlatch_callback);
}

WRITE16_HANDLER( soundlatch_word_w )
{
	static data16_t word;
	COMBINE_DATA(&word);

	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,word,soundlatch_callback);
}

READ_HANDLER( soundlatch_r )
{
	read_debug = 1;
	return latch;
}

READ16_HANDLER( soundlatch_word_r )
{
	read_debug = 1;
	return latch;
}

WRITE_HANDLER( soundlatch_clear_w )
{
	latch = cleared_value;
}


static int latch2,read_debug2;

static void soundlatch2_callback(int param)
{
	if (read_debug2 == 0 && latch2 != param)
		logerror("Warning: sound latch 2 written before being read. Previous: %02x, new: %02x\n",latch2,param);
	latch2 = param;
	read_debug2 = 0;
}

WRITE_HANDLER( soundlatch2_w )
{
	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,data,soundlatch2_callback);
}

WRITE16_HANDLER( soundlatch2_word_w )
{
	static data16_t word;
	COMBINE_DATA(&word);

	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,word,soundlatch2_callback);
}

READ_HANDLER( soundlatch2_r )
{
	read_debug2 = 1;
	return latch2;
}

READ16_HANDLER( soundlatch2_word_r )
{
	read_debug2 = 1;
	return latch2;
}

WRITE_HANDLER( soundlatch2_clear_w )
{
	latch2 = cleared_value;
}


static int latch3,read_debug3;

static void soundlatch3_callback(int param)
{
	if (read_debug3 == 0 && latch3 != param)
		logerror("Warning: sound latch 3 written before being read. Previous: %02x, new: %02x\n",latch3,param);
	latch3 = param;
	read_debug3 = 0;
}

WRITE_HANDLER( soundlatch3_w )
{
	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,data,soundlatch3_callback);
}

WRITE16_HANDLER( soundlatch3_word_w )
{
	static data16_t word;
	COMBINE_DATA(&word);

	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,word,soundlatch3_callback);
}

READ_HANDLER( soundlatch3_r )
{
	read_debug3 = 1;
	return latch3;
}

READ16_HANDLER( soundlatch3_word_r )
{
	read_debug3 = 1;
	return latch3;
}

WRITE_HANDLER( soundlatch3_clear_w )
{
	latch3 = cleared_value;
}


static int latch4,read_debug4;

static void soundlatch4_callback(int param)
{
	if (read_debug4 == 0 && latch4 != param)
		logerror("Warning: sound latch 4 written before being read. Previous: %02x, new: %02x\n",latch2,param);
	latch4 = param;
	read_debug4 = 0;
}

WRITE_HANDLER( soundlatch4_w )
{
	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,data,soundlatch4_callback);
}

WRITE16_HANDLER( soundlatch4_word_w )
{
	static data16_t word;
	COMBINE_DATA(&word);

	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	timer_set(TIME_NOW,word,soundlatch4_callback);
}

READ_HANDLER( soundlatch4_r )
{
	read_debug4 = 1;
	return latch4;
}

READ16_HANDLER( soundlatch4_word_r )
{
	read_debug4 = 1;
	return latch4;
}

WRITE_HANDLER( soundlatch4_clear_w )
{
	latch4 = cleared_value;
}


void soundlatch_setclearedvalue(int value)
{
	cleared_value = value;
}





/***************************************************************************



***************************************************************************/

static void *sound_update_timer;
static double refresh_period;
static double refresh_period_inv;


struct snd_interface
{
	unsigned sound_num;										/* ID */
	const char *name;										/* description */
	int (*chips_num)(const struct MachineSound *msound);	/* returns number of chips if applicable */
	int (*chips_clock)(const struct MachineSound *msound);	/* returns chips clock if applicable */
	int (*start)(const struct MachineSound *msound);		/* starts sound emulation */
	void (*stop)(void);										/* stops sound emulation */
	void (*update)(void);									/* updates emulation once per frame if necessary */
	void (*reset)(void);									/* resets sound emulation */
};


#if (HAS_CUSTOM)
static const struct CustomSound_interface *cust_intf;

int custom_sh_start(const struct MachineSound *msound)
{
	cust_intf = msound->sound_interface;

	if (cust_intf->sh_start)
		return (*cust_intf->sh_start)(msound);
	else return 0;
}
void custom_sh_stop(void)
{
	if (cust_intf->sh_stop) (*cust_intf->sh_stop)();
}
void custom_sh_update(void)
{
	if (cust_intf->sh_update) (*cust_intf->sh_update)();
}
#endif
#if (HAS_DAC)
int DAC_num(const struct MachineSound *msound) { return ((struct DACinterface*)msound->sound_interface)->num; }
#endif
#if (HAS_ADPCM)
int ADPCM_num(const struct MachineSound *msound) { return ((struct ADPCMinterface*)msound->sound_interface)->num; }
#endif
#if (HAS_OKIM6295)
int OKIM6295_num(const struct MachineSound *msound) { return ((struct OKIM6295interface*)msound->sound_interface)->num; }
int OKIM6295_clock(const struct MachineSound *msound) { return ((struct OKIM6295interface*)msound->sound_interface)->frequency[0]; }
#endif
#if (HAS_MSM5205)
int MSM5205_num(const struct MachineSound *msound) { return ((struct MSM5205interface*)msound->sound_interface)->num; }
#endif
#if (HAS_MSM5232)
int MSM5232_num(const struct MachineSound *msound) { return ((struct MSM5232interface*)msound->sound_interface)->num; }
#endif
#if (HAS_HC55516)
int HC55516_num(const struct MachineSound *msound) { return ((struct hc55516_interface*)msound->sound_interface)->num; }
#endif
#if (HAS_K007232)
int K007232_clock(const struct MachineSound *msound) { return ((struct K007232_interface*)msound->sound_interface)->baseclock; }
int K007232_num(const struct MachineSound *msound) { return ((struct K007232_interface*)msound->sound_interface)->num_chips; }
#endif
#if (HAS_AY8910)
int AY8910_clock(const struct MachineSound *msound) { return ((struct AY8910interface*)msound->sound_interface)->baseclock; }
int AY8910_num(const struct MachineSound *msound) { return ((struct AY8910interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YM2203)
int YM2203_clock(const struct MachineSound *msound) { return ((struct YM2203interface*)msound->sound_interface)->baseclock; }
int YM2203_num(const struct MachineSound *msound) { return ((struct YM2203interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YM2413)
int YM2413_clock(const struct MachineSound *msound) { return ((struct YM2413interface*)msound->sound_interface)->baseclock; }
int YM2413_num(const struct MachineSound *msound) { return ((struct YM2413interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YM2608)
int YM2608_clock(const struct MachineSound *msound) { return ((struct YM2608interface*)msound->sound_interface)->baseclock; }
int YM2608_num(const struct MachineSound *msound) { return ((struct YM2608interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YM2610)
int YM2610_clock(const struct MachineSound *msound) { return ((struct YM2610interface*)msound->sound_interface)->baseclock; }
int YM2610_num(const struct MachineSound *msound) { return ((struct YM2610interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YM2612 || HAS_YM3438)
int YM2612_clock(const struct MachineSound *msound) { return ((struct YM2612interface*)msound->sound_interface)->baseclock; }
int YM2612_num(const struct MachineSound *msound) { return ((struct YM2612interface*)msound->sound_interface)->num; }
#endif
#if (HAS_POKEY)
int POKEY_clock(const struct MachineSound *msound) { return ((struct POKEYinterface*)msound->sound_interface)->baseclock; }
int POKEY_num(const struct MachineSound *msound) { return ((struct POKEYinterface*)msound->sound_interface)->num; }
#endif
#if (HAS_YM3812)
int YM3812_clock(const struct MachineSound *msound) { return ((struct YM3812interface*)msound->sound_interface)->baseclock; }
int YM3812_num(const struct MachineSound *msound) { return ((struct YM3812interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YM3526)
int YM3526_clock(const struct MachineSound *msound) { return ((struct YM3526interface*)msound->sound_interface)->baseclock; }
int YM3526_num(const struct MachineSound *msound) { return ((struct YM3526interface*)msound->sound_interface)->num; }
#endif
#if (HAS_Y8950)
int Y8950_clock(const struct MachineSound *msound) { return ((struct Y8950interface*)msound->sound_interface)->baseclock; }
int Y8950_num(const struct MachineSound *msound) { return ((struct Y8950interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YMZ280B)
int YMZ280B_clock(const struct MachineSound *msound) { return ((struct YMZ280Binterface*)msound->sound_interface)->baseclock[0]; }
int YMZ280B_num(const struct MachineSound *msound) { return ((struct YMZ280Binterface*)msound->sound_interface)->num; }
#endif
#if (HAS_VLM5030)
int VLM5030_clock(const struct MachineSound *msound) { return ((struct VLM5030interface*)msound->sound_interface)->baseclock; }
#endif
#if (HAS_TMS36XX)
int TMS36XX_num(const struct MachineSound *msound) { return ((struct TMS36XXinterface*)msound->sound_interface)->num; }
#endif
#if (HAS_TMS5110)
int TMS5110_clock(const struct MachineSound *msound) { return ((struct TMS5110interface*)msound->sound_interface)->baseclock; }
#endif
#if (HAS_TMS5220)
int TMS5220_clock(const struct MachineSound *msound) { return ((struct TMS5220interface*)msound->sound_interface)->baseclock; }
#endif
#if (HAS_YM2151 || HAS_YM2151_ALT)
int YM2151_clock(const struct MachineSound *msound) { return ((struct YM2151interface*)msound->sound_interface)->baseclock; }
int YM2151_num(const struct MachineSound *msound) { return ((struct YM2151interface*)msound->sound_interface)->num; }
#endif
#if (HAS_NES)
int NES_num(const struct MachineSound *msound) { return ((struct NESinterface*)msound->sound_interface)->num; }
#endif
#if (HAS_SN76477)
int SN76477_num(const struct MachineSound *msound) { return ((struct SN76477interface*)msound->sound_interface)->num; }
#endif
#if (HAS_SN76496)
int SN76496_clock(const struct MachineSound *msound) { return ((struct SN76496interface*)msound->sound_interface)->baseclock[0]; }
int SN76496_num(const struct MachineSound *msound) { return ((struct SN76496interface*)msound->sound_interface)->num; }
#endif
#if (HAS_MSM5205)
int MSM5205_clock(const struct MachineSound *msound) { return ((struct MSM5205interface*)msound->sound_interface)->baseclock; }
#endif
#if (HAS_MSM5232)
int MSM5232_clock(const struct MachineSound *msound) { return ((struct MSM5232interface*)msound->sound_interface)->baseclock; }
#endif
#if (HAS_ASTROCADE)
int ASTROCADE_clock(const struct MachineSound *msound) { return ((struct astrocade_interface*)msound->sound_interface)->baseclock; }
int ASTROCADE_num(const struct MachineSound *msound) { return ((struct astrocade_interface*)msound->sound_interface)->num; }
#endif
#if (HAS_K051649)
int K051649_clock(const struct MachineSound *msound) { return ((struct k051649_interface*)msound->sound_interface)->master_clock; }
#endif
#if (HAS_K053260)
int K053260_clock(const struct MachineSound *msound) { return ((struct K053260_interface*)msound->sound_interface)->clock[0]; }
int K053260_num(const struct MachineSound *msound) { return ((struct K053260_interface*)msound->sound_interface)->num; }
#endif
#if (HAS_K054539)
int K054539_clock(const struct MachineSound *msound) { return ((struct K054539interface*)msound->sound_interface)->clock; }
int K054539_num(const struct MachineSound *msound) { return ((struct K054539interface*)msound->sound_interface)->num; }
#endif
#if (HAS_CEM3394)
int cem3394_num(const struct MachineSound *msound) { return ((struct cem3394_interface*)msound->sound_interface)->numchips; }
#endif
#if (HAS_QSOUND)
int qsound_clock(const struct MachineSound *msound) { return ((struct QSound_interface*)msound->sound_interface)->clock; }
#endif
#if (HAS_SAA1099)
int saa1099_num(const struct MachineSound *msound) { return ((struct SAA1099_interface*)msound->sound_interface)->numchips; }
#endif
#if (HAS_IREMGA20)
int iremga20_clock(const struct MachineSound *msound) { return ((struct IremGA20_interface*)msound->sound_interface)->clock; }
#endif
#if (HAS_ES5505)
int ES5505_clock(const struct MachineSound *msound) { return ((struct ES5505interface*)msound->sound_interface)->baseclock[0]; }
int ES5505_num(const struct MachineSound *msound) { return ((struct ES5505interface*)msound->sound_interface)->num; }
#endif
#if (HAS_ES5506)
int ES5506_clock(const struct MachineSound *msound) { return ((struct ES5506interface*)msound->sound_interface)->baseclock[0]; }
int ES5506_num(const struct MachineSound *msound) { return ((struct ES5506interface*)msound->sound_interface)->num; }
#endif
#if (HAS_BSMT2000)
int BSMT2000_clock(const struct MachineSound *msound) { return ((struct BSMT2000interface*)msound->sound_interface)->baseclock[0]; }
int BSMT2000_num(const struct MachineSound *msound) { return ((struct BSMT2000interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YMF262)
int YMF262_clock(const struct MachineSound *msound) { return ((struct YMF262interface*)msound->sound_interface)->baseclock; }
int YMF262_num(const struct MachineSound *msound) { return ((struct YMF262interface*)msound->sound_interface)->num; }
#endif
#if (HAS_YMF278B)
int YMF278B_clock(const struct MachineSound *msound) { return ((struct YMF278B_interface*)msound->sound_interface)->clock[0]; }
int YMF278B_num(const struct MachineSound *msound) { return ((struct YMF278B_interface*)msound->sound_interface)->num; }
#endif
#if (HAS_X1_010)
int seta_clock(const struct MachineSound *msound) { return ((struct x1_010_interface*)msound->sound_interface)->clock; }
#endif
#if (HAS_MULTIPCM)
int MultiPCM_num(const struct MachineSound *msound) { return ((struct MultiPCM_interface*)msound->sound_interface)->chips; }
#endif
#if (HAS_C6280)
int c6280_clock(const struct MachineSound *msound) { return ((struct C6280_interface*)msound->sound_interface)->clock[0]; }
int c6280_num(const struct MachineSound *msound) { return ((struct C6280_interface*)msound->sound_interface)->num; }
#endif
#if (HAS_TIA)
int TIA_clock(const struct MachineSound *msound) { return ((struct TIAinterface*)msound->sound_interface)->baseclock; }
#endif

#ifdef MESS
#if (HAS_BEEP)
int beep_num(const struct MachineSound *msound) { return ((struct beep_interface*)msound->sound_interface)->num; }
#endif
#if (HAS_SPEAKER)
int speaker_num(const struct MachineSound *msound) { return ((struct Speaker_interface*)msound->sound_interface)->num; }
#endif
#if (HAS_WAVE)
int wave_num(const struct MachineSound *msound) { return ((struct Wave_interface*)msound->sound_interface)->num; }
#endif
#endif
#if 1	/* forQUASI88 */
#if (HAS_BEEP88)
int BEEP88_clock(const struct MachineSound *msound) { return ((struct BEEP88interface*)msound->sound_interface)->baseclock; }
int BEEP88_num(const struct MachineSound *msound) { return ((struct BEEP88interface*)msound->sound_interface)->num; }
#endif

#ifdef	USE_FMGEN
#if (HAS_FMGEN2203)
int FMGEN2203_clock(const struct MachineSound *msound) { return ((struct YM2203interface*)msound->sound_interface)->baseclock; }
int FMGEN2203_num(const struct MachineSound *msound) { return ((struct YM2203interface*)msound->sound_interface)->num; }
#endif
#if (HAS_FMGEN2608)
int FMGEN2608_clock(const struct MachineSound *msound) { return ((struct YM2608interface*)msound->sound_interface)->baseclock; }
int FMGEN2608_num(const struct MachineSound *msound) { return ((struct YM2608interface*)msound->sound_interface)->num; }
#endif
#endif

#endif	/* forQUASI88 */

struct snd_interface sndintf[] =
{
    {
		SOUND_DUMMY,
		"",
		0,
		0,
		0,
		0,
		0,
		0
	},
#if (HAS_CUSTOM)
    {
		SOUND_CUSTOM,
		"Custom",
		0,
		0,
		custom_sh_start,
		custom_sh_stop,
		custom_sh_update,
		0
	},
#endif
#if (HAS_SAMPLES)
    {
		SOUND_SAMPLES,
		"Samples",
		0,
		0,
		samples_sh_start,
		0,
		0,
		0
	},
#endif
#if (HAS_DAC)
    {
		SOUND_DAC,
		"DAC",
		DAC_num,
		0,
		DAC_sh_start,
		0,
		0,
		0
	},
#endif
#if (HAS_DISCRETE)
    {
		SOUND_DISCRETE,
		"Discrete Components",
		0,
		0,
		discrete_sh_start,
		discrete_sh_stop,
		0,
		discrete_sh_reset
	},
#endif
#if (HAS_AY8910)
    {
		SOUND_AY8910,
		"AY-3-8910",
		AY8910_num,
		AY8910_clock,
		AY8910_sh_start,
		AY8910_sh_stop,
		0,
		AY8910_sh_reset
	},
#endif
#if (HAS_YM2203)
    {
		SOUND_YM2203,
		"YM2203",
		YM2203_num,
		YM2203_clock,
		YM2203_sh_start,
		YM2203_sh_stop,
		0,
		YM2203_sh_reset
	},
#endif
#if (HAS_YM2151 || HAS_YM2151_ALT)
    {
		SOUND_YM2151,
		"YM2151",
		YM2151_num,
		YM2151_clock,
		YM2151_sh_start,
		YM2151_sh_stop,
		0,
		YM2151_sh_reset
	},
#endif
#if (HAS_YM2608)
    {
		SOUND_YM2608,
		"YM2608",
		YM2608_num,
		YM2608_clock,
		YM2608_sh_start,
		YM2608_sh_stop,
		0,
		YM2608_sh_reset
	},
#endif
#if (HAS_YM2610)
    {
		SOUND_YM2610,
		"YM2610",
		YM2610_num,
		YM2610_clock,
		YM2610_sh_start,
		YM2610_sh_stop,
		0,
		YM2610_sh_reset
	},
#endif
#if (HAS_YM2610B)
    {
		SOUND_YM2610B,
		"YM2610B",
		YM2610_num,
		YM2610_clock,
		YM2610B_sh_start,
		YM2610_sh_stop,
		0,
		YM2610_sh_reset
	},
#endif
#if (HAS_YM2612)
    {
		SOUND_YM2612,
		"YM2612",
		YM2612_num,
		YM2612_clock,
		YM2612_sh_start,
		YM2612_sh_stop,
		0,
		YM2612_sh_reset
	},
#endif
#if (HAS_YM3438)
    {
		SOUND_YM3438,
		"YM3438",
		YM2612_num,
		YM2612_clock,
		YM2612_sh_start,
		YM2612_sh_stop,
		0,
		YM2612_sh_reset
	},
#endif
#if (HAS_YM2413)
    {
		SOUND_YM2413,
		"YM2413",
		YM2413_num,
		YM2413_clock,
		YM2413_sh_start,
		YM2413_sh_stop,
		0,
		0
	},
#endif
#if (HAS_YM3812)
    {
		SOUND_YM3812,
		"YM3812",
		YM3812_num,
		YM3812_clock,
		YM3812_sh_start,
		YM3812_sh_stop,
		0,
		0
	},
#endif
#if (HAS_YM3526)
    {
		SOUND_YM3526,
		"YM3526",
		YM3526_num,
		YM3526_clock,
		YM3526_sh_start,
		YM3526_sh_stop,
		0,
		0
	},
#endif
#if (HAS_YMZ280B)
    {
		SOUND_YMZ280B,
		"YMZ280B",
		YMZ280B_num,
		YMZ280B_clock,
		YMZ280B_sh_start,
		YMZ280B_sh_stop,
		0,
		0
	},
#endif
#if (HAS_Y8950)
	{
		SOUND_Y8950,
		"Y8950",	/* (MSX-AUDIO) */
		Y8950_num,
		Y8950_clock,
		Y8950_sh_start,
		Y8950_sh_stop,
		0,
		0
	},
#endif
#if (HAS_SN76477)
    {
		SOUND_SN76477,
		"SN76477",
		SN76477_num,
		0,
		SN76477_sh_start,
		SN76477_sh_stop,
		0,
		0
	},
#endif
#if (HAS_SN76496)
    {
		SOUND_SN76496,
		"SN76496",
		SN76496_num,
		SN76496_clock,
		SN76496_sh_start,
        0,
		0
	},
#endif
#if (HAS_POKEY)
    {
		SOUND_POKEY,
		"Pokey",
		POKEY_num,
		POKEY_clock,
		pokey_sh_start,
		pokey_sh_stop,
		0,
		0
	},
#endif
#if (HAS_NES)
    {
		SOUND_NES,
		"Nintendo",
		NES_num,
		0,
		NESPSG_sh_start,
		NESPSG_sh_stop,
		NESPSG_sh_update,
		0
	},
#endif
#if (HAS_ASTROCADE)
    {
		SOUND_ASTROCADE,
		"Astrocade",
		ASTROCADE_num,
		ASTROCADE_clock,
		astrocade_sh_start,
		astrocade_sh_stop,
		astrocade_sh_update,
		0
	},
#endif
#if (HAS_NAMCO)
    {
		SOUND_NAMCO,
		"Namco",
		0,
		0,
		namco_sh_start,
		namco_sh_stop,
		0,
		0
	},
#endif
#if (HAS_NAMCONA)
    {
		SOUND_NAMCONA,
		"Namco NA",
		0,
		0,
		NAMCONA_sh_start,
		NAMCONA_sh_stop,
		0,
		0
	},
#endif
#if (HAS_TMS36XX)
    {
		SOUND_TMS36XX,
		"TMS36XX",
		TMS36XX_num,
        0,
		tms36xx_sh_start,
		tms36xx_sh_stop,
		tms36xx_sh_update,
		0
	},
#endif
#if (HAS_TMS5110)
    {
		SOUND_TMS5110,
		"TMS5110",
		0,
		TMS5110_clock,
		tms5110_sh_start,
		tms5110_sh_stop,
		tms5110_sh_update,
		0
	},
#endif
#if (HAS_TMS5220)
    {
		SOUND_TMS5220,
		"TMS5220",
		0,
		TMS5220_clock,
		tms5220_sh_start,
		tms5220_sh_stop,
		tms5220_sh_update,
		0
	},
#endif
#if (HAS_VLM5030)
    {
		SOUND_VLM5030,
		"VLM5030",
		0,
		VLM5030_clock,
		VLM5030_sh_start,
		VLM5030_sh_stop,
		VLM5030_sh_update,
		0
	},
#endif
#if (HAS_ADPCM)
    {
		SOUND_ADPCM,
		"ADPCM",
		ADPCM_num,
		0,
		ADPCM_sh_start,
		ADPCM_sh_stop,
		ADPCM_sh_update,
		0
	},
#endif
#if (HAS_OKIM6295)
    {
		SOUND_OKIM6295,
		"MSM6295",
		OKIM6295_num,
		OKIM6295_clock,
		OKIM6295_sh_start,
		OKIM6295_sh_stop,
		OKIM6295_sh_update,
		0
	},
#endif
#if (HAS_MSM5205)
    {
		SOUND_MSM5205,
		"MSM5205",
		MSM5205_num,
		MSM5205_clock,
		MSM5205_sh_start,
		0,
		0,
		MSM5205_sh_reset,
	},
#endif
#if (HAS_MSM5232)
    {
		SOUND_MSM5232,
		"MSM5232",
		MSM5232_num,
		MSM5232_clock,
		MSM5232_sh_start,
		MSM5232_sh_stop,
		0,
		MSM5232_sh_reset,
	},
#endif
#if (HAS_UPD7759)
    {
		SOUND_UPD7759,
		"uPD7759",
		0,
		0,
		UPD7759_sh_start,
		0,
		0,
		0
	},
#endif
#if (HAS_HC55516)
    {
		SOUND_HC55516,
		"HC55516",
		HC55516_num,
		0,
		hc55516_sh_start,
		0,
		0,
		0
	},
#endif
#if (HAS_K005289)
    {
		SOUND_K005289,
		"005289",
		0,
		0,
		K005289_sh_start,
		K005289_sh_stop,
		0,
		0
	},
#endif
#if (HAS_K007232)
    {
		SOUND_K007232,
		"007232",
		K007232_num,
		K007232_clock,
		K007232_sh_start,
		0,
		0,
		0
	},
#endif
#if (HAS_K051649)
    {
		SOUND_K051649,
		"051649",
		0,
		K051649_clock,
		K051649_sh_start,
		K051649_sh_stop,
		0,
		0
	},
#endif
#if (HAS_K053260)
    {
		SOUND_K053260,
		"053260",
		K053260_num,
		K053260_clock,
		K053260_sh_start,
		K053260_sh_stop,
		0,
		0
	},
#endif
#if (HAS_K054539)
    {
		SOUND_K054539,
		"054539",
		K054539_num,
		K054539_clock,
		K054539_sh_start,
		K054539_sh_stop,
		0,
		0
	},
#endif
#if (HAS_SEGAPCM)
	{
		SOUND_SEGAPCM,
		"Sega PCM",
		0,
		0,
		SEGAPCM_sh_start,
		SEGAPCM_sh_stop,
		SEGAPCM_sh_update,
		0
	},
#endif
#if (HAS_RF5C68)
	{
		SOUND_RF5C68,
		"RF5C68",
		0,
		0,
		RF5C68_sh_start,
		RF5C68_sh_stop,
		0,
		0
	},
#endif
#if (HAS_CEM3394)
	{
		SOUND_CEM3394,
		"CEM3394",
		cem3394_num,
		0,
		cem3394_sh_start,
		cem3394_sh_stop,
		0,
		0
	},
#endif
#if (HAS_C140)
	{
		SOUND_C140,
		"C140",
		0,
		0,
		C140_sh_start,
		C140_sh_stop,
		0,
		0
	},
#endif
#if (HAS_QSOUND)
	{
		SOUND_QSOUND,
		"QSound",
		0,
		qsound_clock,
		qsound_sh_start,
		qsound_sh_stop,
		0,
		0
	},
#endif
#if (HAS_SAA1099)
	{
		SOUND_SAA1099,
		"SAA1099",
		saa1099_num,
		0,
		saa1099_sh_start,
		saa1099_sh_stop,
		0,
		0
	},
#endif
#if (HAS_IREMGA20)
	{
		SOUND_IREMGA20,
		"GA20",
		0,
		iremga20_clock,
		IremGA20_sh_start,
		IremGA20_sh_stop,
		0,
		0
	},
#endif
#if (HAS_ES5505)
	{
		SOUND_ES5505,
		"ES5505",
		ES5505_num,
		ES5505_clock,
		ES5505_sh_start,
		ES5505_sh_stop,
		0,
		0
	},
#endif
#if (HAS_ES5506)
	{
		SOUND_ES5506,
		"ES5506",
		ES5506_num,
		ES5506_clock,
		ES5506_sh_start,
		ES5506_sh_stop,
		0,
		0
	},
#endif
#if (HAS_BSMT2000)
	{
		SOUND_BSMT2000,
		"BSMT2000",
		BSMT2000_num,
		BSMT2000_clock,
		BSMT2000_sh_start,
		BSMT2000_sh_stop,
		0,
		0
	},
#endif
#if (HAS_YMF262)
	{
		 SOUND_YMF262,
		 "YMF262",
		 YMF262_num,
		 YMF262_clock,
		 YMF262_sh_start,
		 YMF262_sh_stop,
		 0,
		 0
	},
#endif
#if (HAS_YMF278B)
	{
		 SOUND_YMF278B,
		 "YMF278B",
		 YMF278B_num,
		 YMF278B_clock,
		 YMF278B_sh_start,
		 YMF278B_sh_stop,
		 0,
		 0
	},
#endif
#if (HAS_GAELCO_CG1V)
	{
		SOUND_GAELCO_CG1V,
		"GAELCO CG-1V",
		0,
		0,
		gaelco_cg1v_sh_start,
		gaelcosnd_sh_stop,
		0,
		0
	},
#endif
#if (HAS_GAELCO_GAE1)
	{
		SOUND_GAELCO_GAE1,
		"GAELCO GAE1",
		0,
		0,
		gaelco_gae1_sh_start,
		gaelcosnd_sh_stop,
		0,
		0
	},
#endif
#if (HAS_X1_010)
	{
		SOUND_X1_010,
		"X1-010",
		0,
		seta_clock,
		seta_sh_start,
		seta_sh_stop,
		0,
		0
	},
#endif
#if (HAS_MULTIPCM)
	{
		SOUND_MULTIPCM,
		"Sega 315-5560",
		MultiPCM_num,
		0,
		MultiPCM_sh_start,
		MultiPCM_sh_stop,
		0,
		0
	},
#endif
#if (HAS_C6280)
	{
		SOUND_C6280,
		"HuC6280",
		0,
		c6280_clock,
		c6280_sh_start,
		c6280_sh_stop,
		0,
		0
	},
#endif
#if (HAS_TIA)
    {
		SOUND_TIA,
		"TIA",
		0,
		TIA_clock,
		tia_sh_start,
		tia_sh_stop,
		tia_sh_update,
		0
	},
#endif


#ifdef MESS
#if (HAS_BEEP)
	{
		SOUND_BEEP,
		"Beep",
		beep_num,
		0,
		beep_sh_start,
		beep_sh_stop,
		beep_sh_update,
		0
	},
#endif
#if (HAS_SPEAKER)
	{
		SOUND_SPEAKER,
		"Speaker",
		speaker_num,
		0,
		speaker_sh_start,
		speaker_sh_stop,
		speaker_sh_update,
		0
	},
#endif
#if (HAS_WAVE)
	{
		SOUND_WAVE,
		"Cassette",
		wave_num,
		0,
		wave_sh_start,
		wave_sh_stop,
		wave_sh_update,
		0
	},
#endif
#endif

#if 1	/* forQUASI88 */
#if (HAS_BEEP88)
    {
		SOUND_BEEP88,
		"BEEP88",
		BEEP88_num,
		BEEP88_clock,
		BEEP88_sh_start,
		BEEP88_sh_stop,
		0,
		BEEP88_sh_reset,
	},
#endif
#ifdef	USE_FMGEN
#if (HAS_FMGEN2203)
    {
		SOUND_FMGEN2203,
		"FMGEN2203",
		FMGEN2203_num,
		FMGEN2203_clock,
		FMGEN2203_sh_start,
		FMGEN2203_sh_stop,
		0,
		FMGEN2203_sh_reset
	},
#endif
#if (HAS_FMGEN2608)
    {
		SOUND_FMGEN2608,
		"FMGEN2608",
		FMGEN2608_num,
		FMGEN2608_clock,
		FMGEN2608_sh_start,
		FMGEN2608_sh_stop,
		0,
		FMGEN2608_sh_reset
	},
#endif
#endif
#endif	/* forQUASI88 */

};

#if 1	/* forQUASI88 */

static struct YM2203interface ym2203_interface =
{
	1,						/* num				1 chips			*/
	4000000,				/* baseclock		4.0 MHz			*/
	{ YM2203_VOL(10,15) },	/* mixing_level[]	FMVol, SSGVol	*//*OverWrite*/
	{ 0 },					/* portA read handler				*/
	{ 0 },					/* portB read handler				*/
	{ 0 },					/* portA write handler				*/
	{ 0 },					/* portB write handler				*/
	{ 0 }					/* IRQ handler						*/
};
static struct MachineSound pc88_sound_interface =
{
	SOUND_YM2203,
	&ym2203_interface,
	NULL
};
#ifdef	USE_FMGEN
static struct MachineSound pc88_fmgen_interface =
{
	SOUND_FMGEN2203,
	&ym2203_interface,
	NULL
};
#endif

static struct YM2608interface ym2608_interface =
{
	1,						/* num				1 chips			*/
	8000000,				/* baseclock		8.0 MHz			*/
	{ 10 },					/* volumeSSG[]		SSGVol			*//*OverWrite*/
	{ 0 },					/* portA read handler				*/
	{ 0 },					/* portB read handler				*/
	{ 0 },					/* portA write handler				*/
	{ 0 },					/* portB write handler				*/
	{ 0 },					/* IRQ handler						*/
	{ -1 /*dummy*/ },		/* Delta-T memory region ram/rom	*/
	{ YM3012_VOL(15,MIXER_PAN_LEFT,15,MIXER_PAN_RIGHT) }
							/* use YM3012_VOL macro				*//*OverWrite*/
};
static struct MachineSound pc88_sound2_interface =
{
	SOUND_YM2608,
	&ym2608_interface,
	NULL
};
#ifdef	USE_FMGEN
static struct MachineSound pc88_fmgen2_interface =
{
	SOUND_FMGEN2608,
	&ym2608_interface,
	NULL
};
#endif

static struct BEEP88interface beep88_interface =
{
	1,						/* num				1 chips			*/
	2400,					/* baseclock		2400 Hz			*/
	{ 10 },					/* mixing_level[]	BEEPVol			*/
	{ 0 },					/* port40 read handler				*/
	{ 0 }					/* port40 write handler				*/
};
static struct MachineSound pc88_beep_interface =
{
	SOUND_BEEP88,
	&beep88_interface,
	NULL,
};



static	T_XMAME_FUNC pc88_sound_func =
{
  YM2203TimerOver,
  YM2203_read_port_0_r,
  YM2203_status_port_0_r,
  YM2203_control_port_0_w,
  YM2203_write_port_0_w,
  NULL,
  NULL,
  NULL,
  NULL,
  BEEP88_write_port_0_w,
};
static	T_XMAME_FUNC pc88_sound2_func =
{
  YM2608TimerOver,
  YM2608_read_port_0_r,
  YM2608_status_port_0_A_r,
  YM2608_control_port_0_A_w,
  YM2608_data_port_0_A_w,
  NULL,
  YM2608_status_port_0_B_r,
  YM2608_control_port_0_B_w,
  YM2608_data_port_0_B_w,
  BEEP88_write_port_0_w,
};
#ifdef	USE_FMGEN
static	T_XMAME_FUNC pc88_fmgen_func =
{
  NULL,
  FMGEN2203_read_port_0_r,
  FMGEN2203_status_port_0_r,
  FMGEN2203_control_port_0_w,
  FMGEN2203_write_port_0_w,
  NULL,
  NULL,
  NULL,
  NULL,
  BEEP88_write_port_0_w,
};
static	T_XMAME_FUNC pc88_fmgen2_func =
{
  NULL,
  FMGEN2608_read_port_0_r,
  FMGEN2608_status_port_0_A_r,
  FMGEN2608_control_port_0_A_w,
  FMGEN2608_data_port_0_A_w,
  NULL,
  FMGEN2608_status_port_0_B_r,
  FMGEN2608_control_port_0_B_w,
  FMGEN2608_data_port_0_B_w,
  BEEP88_write_port_0_w,
};
#endif



static	struct MachineSound	fm_interface;
static	struct MachineSound	beep_interface;



T_XMAME_FUNC xmame_func_nosound =
{
  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
};


#endif	/* forQUASI88 */



int sound_start(void)
{
#if 0	/* forQUASI88 */
	int totalsound = 0;
	int i;

	/* Verify the order of entries in the sndintf[] array */
	for (i = 0;i < SOUND_COUNT;i++)
	{
		if (sndintf[i].sound_num != i)
		{
            int j;
logerror("Sound #%d wrong ID %d: check enum SOUND_... in src/sndintrf.h!\n",i,sndintf[i].sound_num);
			for (j = 0; j < i; j++)
				logerror("ID %2d: %s\n", j, sndintf[j].name);
            return 1;
		}
	}


	/* samples will be read later if needed */
	Machine->samples = 0;

	refresh_period = TIME_IN_HZ(Machine->drv->frames_per_second);
	refresh_period_inv = 1.0 / refresh_period;
	sound_update_timer = timer_alloc(NULL);
#endif	/* forQUASI88 */




#if 1	/* forQUASI88 */


#ifndef	USE_FMGEN
	if( use_fmgen ){
	  if(verbose_proc) printf("(cisc's Fmgen liblary not available)...");
	  use_fmgen = 0;
	}
#endif


	/* set interface parameter ( volume, mono/stereo, and so on ) */

	/* set interface parameter and set xmame interface function */

#ifdef	USE_FMGEN
	if( use_fmgen ){
	  if( sound_board==SOUND_I ){					/* fmgen ym2203 */

	    fm_interface     = pc88_fmgen_interface;
	    beep_interface   = pc88_beep_interface;
	    xmame_func_sound = pc88_fmgen_func;

		Machine__drv__sound_attributes = SOUND_SUPPORTS_STEREO;

	  }else{										/* fmgen ym2608 */

	    fm_interface     = pc88_fmgen2_interface;
	    beep_interface   = pc88_beep_interface;
	    xmame_func_sound = pc88_fmgen2_func;

		Machine__drv__sound_attributes = SOUND_SUPPORTS_STEREO;
	  }
	}else
#endif
	{
	  if( sound_board==SOUND_I ){					/* xmame ym2203 */

	    fm_interface     = pc88_sound_interface;
	    beep_interface   = pc88_beep_interface;
	    xmame_func_sound = pc88_sound_func;

		Machine__drv__sound_attributes = 0;

	  }else{										/* xmame ym2608 */
	    fm_interface     = pc88_sound2_interface;
	    beep_interface   = pc88_beep_interface;
	    xmame_func_sound = pc88_sound2_func;

		Machine__drv__sound_attributes = SOUND_SUPPORTS_STEREO;
	  }
	}


	/* set volume ( fm, psg, beep ) */

	if( sound_board==SOUND_I ){
	  if( Machine__drv__sound_attributes == 0)
		ym2203_interface.mixing_level[0] = YM2203_VOL(fmvol,psgvol);
	  else
		ym2203_interface.mixing_level[0] = 
						YM3012_VOL(fmvol,MIXER_PAN_LEFT,fmvol,MIXER_PAN_RIGHT);
	}else{
	  ym2608_interface.volumeSSG[0] = psgvol;
	  ym2608_interface.volumeFM[0]  = 
						YM3012_VOL(fmvol,MIXER_PAN_LEFT,fmvol,MIXER_PAN_RIGHT);
	}
	beep88_interface.mixing_level[0] = beepvol;



#endif	/* forQUASI88 */

	if (mixer_sh_start() != 0)
		return 1;

	if (streams_sh_start() != 0)
		return 1;

#if 0	/* forQUASI88 */
	while (Machine->drv->sound[totalsound].sound_type != 0 && totalsound < MAX_SOUND)
	{
		if ((*sndintf[Machine->drv->sound[totalsound].sound_type].start)(&Machine->drv->sound[totalsound]) != 0)
			goto getout;

		totalsound++;
	}
#else	/* forQUASI88 */
	{
	  
		if ((*sndintf[ fm_interface.sound_type ].start)(&fm_interface) != 0)
			goto getout;

		if ((*sndintf[ beep_interface.sound_type ].start)(&beep_interface) != 0)
			goto getout;

	}
#endif	/* forQUASI88 */

	return 0;


getout:
	/* TODO: should also free the resources allocated before */
	return 1;
}



void sound_stop(void)
{
#if 0	/* forQUASI88 */
	int totalsound = 0;


	while (Machine->drv->sound[totalsound].sound_type != 0 && totalsound < MAX_SOUND)
	{
		if (sndintf[Machine->drv->sound[totalsound].sound_type].stop)
			(*sndintf[Machine->drv->sound[totalsound].sound_type].stop)();

		totalsound++;
	}
#else	/* forQUASI88 */
	{
		if (sndintf[ fm_interface.sound_type ].stop)
			(*sndintf[ fm_interface.sound_type ].stop)();

		if (sndintf[ beep_interface.sound_type ].stop)
			(*sndintf[ beep_interface.sound_type ].stop)();
	}
#endif	/* forQUASI88 */

	streams_sh_stop();
	mixer_sh_stop();

#if 0	/* forQUASI88 */
	/* free audio samples */
	Machine->samples = 0;
#endif	/* forQUASI88 */
}



void sound_update(void)
{
#if 0	/* forQUASI88 */
	int totalsound = 0;


	profiler_mark(PROFILER_SOUND);

	while (Machine->drv->sound[totalsound].sound_type != 0 && totalsound < MAX_SOUND)
	{
		if (sndintf[Machine->drv->sound[totalsound].sound_type].update)
			(*sndintf[Machine->drv->sound[totalsound].sound_type].update)();

		totalsound++;
	}
#else	/* forQUASI88 */
	{
		if (sndintf[ fm_interface.sound_type ].update)
			(*sndintf[ fm_interface.sound_type ].update)();
		
		if (sndintf[ beep_interface.sound_type ].update)
			(*sndintf[ beep_interface.sound_type ].update)();

	}
#endif	/* forQUASI88 */

	streams_sh_update();
	mixer_sh_update();

	timer_adjust(sound_update_timer, TIME_NEVER, 0, 0);

	profiler_mark(PROFILER_END);
}


void sound_reset(void)
{
#if 0	/* forQUASI88 */
	int totalsound = 0;


	while (Machine->drv->sound[totalsound].sound_type != 0 && totalsound < MAX_SOUND)
	{
		if (sndintf[Machine->drv->sound[totalsound].sound_type].reset)
			(*sndintf[Machine->drv->sound[totalsound].sound_type].reset)();

		totalsound++;
	}
#else	/* forQUASI88 */
	{
		if (sndintf[ fm_interface.sound_type ].reset)
			(*sndintf[ fm_interface.sound_type ].reset)();

		if (sndintf[ beep_interface.sound_type ].reset)
			(*sndintf[ beep_interface.sound_type ].reset)();
	}
#endif	/* forQUASI88 */
}



const char *soundtype_name(int soundtype)
{
	if (soundtype < SOUND_COUNT)
		return sndintf[soundtype].name;
	else
		return "";
}

const char *sound_name(const struct MachineSound *msound)
{
	return soundtype_name(msound->sound_type);
}

int sound_num(const struct MachineSound *msound)
{
	if (msound->sound_type < SOUND_COUNT && sndintf[msound->sound_type].chips_num)
		return (*sndintf[msound->sound_type].chips_num)(msound);
	else
		return 0;
}

int sound_clock(const struct MachineSound *msound)
{
	if (msound->sound_type < SOUND_COUNT && sndintf[msound->sound_type].chips_clock)
		return (*sndintf[msound->sound_type].chips_clock)(msound);
	else
		return 0;
}


#if 0	/* forQUASI88 */
int sound_scalebufferpos(int value)
{
	int result = (int)((double)value * timer_timeelapsed(sound_update_timer) * refresh_period_inv);
	if (value >= 0) return (result < value) ? result : value;
	else return (result > value) ? result : value;
}
#else	/* forQUASI88 */
int sound_scalebufferpos(int value)
{
	int result = (int)((double)(state_of_cpu + z80main_cpu.state0)/state_of_vsync * value);
	return (result < value) ? result : value;
}
#endif	/* forQUASI88 */
