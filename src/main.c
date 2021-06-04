#include <stdio.h>
#include <string.h>
#include "syscall.h"
#include "pg.h"

// ホームボタン終了時にコールバック
int exit_callback(void)
{
	sceKernelExitGame();
	return 0;
}

// スリープ時や不定期にコールバック
void power_callback(int unknown, int pwrflags)
{
	// コールバック関数の再登録
	// （一度呼ばれたら再登録しとかないと次にコールバックされない）
	int cbid = sceKernelCreateCallback("Power Callback", power_callback);
	scePowerRegisterCallback(0, cbid);
}

// ポーリング用スレッド
int CallbackThread(int args, void *argp)
{
	int cbid;
	
	// コールバック関数の登録
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback);
	sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", power_callback);
	scePowerRegisterCallback(0, cbid);
	
	// ポーリング
	sceKernelPollCallbacks();
}

int SetupCallbacks(void)
{
	int thid = 0;
	
	// ポーリング用スレッドの生成
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	
	return thid;
}


//44100,chan:2固定
static void wavout_snd0_callback(short *buf, unsigned long reqn)
{
	int i;
	memset(buf,0,reqn*4);

}

//return 0 if success
int wavoutInit()
{
	pgaSetChannelCallback(0,wavout_snd0_callback);
	return 0;
}

int xmain(int argc, char *argv)
{


	pgInit();

	SetupCallbacks();
	pgScreenFrame(1,0);

	wavoutInit();

	pgFillvram(0);
	pgScreenFlipV();
	pgFillvram(0);
	pgScreenFlipV();

	pgcLocate(0,0);
	pgPrint_drawbg(0,0,0xFFFF,0,"Q88PSP testprog");
	pgPrint_drawbg(0,1,0xFFFF,0,"This is a test for PSP");
	pgPrint_drawbg(0,2,0xFFFF,0,"You may do it");
	pgPrint_drawbg(0,3,0xFFFF,0,"I hope this will be worked fine");

    while(1)
	{
		readpad();
	}

}


