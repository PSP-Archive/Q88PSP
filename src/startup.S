# startup code for PSP

		.set noreorder

		.text

		.extern xmain


##############################################################################


		.ent _start
		.weak _start
_start: 
      addiu   $sp, 0x10 
      sw      $31, 0($sp)    
      sw      $16, 4($sp) 
      sw      $17, 8($sp) 

      la     $2,_gp
      move   $gp,$2

      move   $16, $4            # Save args 
      move   $17, $5 

      la     $4, _main_thread_name   # Main thread setup 
      la     $5, xmain 
      li     $6, 0x20            # Priority 
      li     $7, 0x40000         # Stack size 
      lui    $8, 0x8000            # Attributes 
      jal    sceKernelCreateThread 
      move   $9, $0 

      move   $4, $2            # Start thread 
      move   $5, $16 
      jal    sceKernelStartThread 
      move   $6, $17

      lw     $ra, 0($sp) 
      lw     $16, 4($sp) 
      lw     $17, 8($sp) 
      move   $2, $0 
      jr      $ra 
      addiu   $sp, 0x10 

_main_thread_name: 
      .asciiz   "user_main" 

##############################################################################


		.section	.lib.ent,"wa",@progbits
__lib_ent_top:
		.word 0
		.word 0x80000000
		.word 0x00010104
		.word __entrytable


		.section	.lib.ent.btm,"wa",@progbits
__lib_ent_bottom:
		.word	0


		.section	.lib.stub,"wa",@progbits
__lib_stub_top:


		.section	.lib.stub.btm,"wa",@progbits
__lib_stub_bottom:
		.word	0



##############################################################################

		.section	".xodata.sceModuleInfo","wa",@progbits

__moduleinfo:
		.byte	0,0,1,1

		.ascii	"HelloWorld"		#up to 28 char
		.align	5

		.word	_gp
		.word	__lib_ent_top
		.word	__lib_ent_bottom
		.word	__lib_stub_top
		.word	__lib_stub_bottom

##############################################################################

		.section	.rodata.entrytable,"wa",@progbits
__entrytable:
		.word 0xD632ACDB
		.word 0xF01D73A7
		.word _start
		.word __moduleinfo
		.word 0


###############################################################################


	.macro	STUB_START	module,d1,d2

		.section	.rodata.stubmodulename
		.word	0
__stub_modulestr_\@:
		.asciz	"\module"
		.align	2

		.section	.lib.stub
		.word __stub_modulestr_\@
		.word \d1
		.word \d2
		.word __stub_idtable_\@
		.word __stub_text_\@

		.section	.rodata.stubidtable
__stub_idtable_\@:

		.section	.text.stub
__stub_text_\@:

	.endm


	.macro	STUB_END
	.endm


	.macro	STUB_FUNC	funcid,funcname

		.set push
		.set noreorder

		.section	.text.stub
		.weak	\funcname
\funcname:
		jr	$ra
		nop

		.section	.rodata.stubidtable
		.word	\funcid

		.set pop

	.endm


	STUB_START "sceDisplay",0x40010000,0x00110005
	STUB_FUNC 0x0e20f177,sceDisplaySetMode
	STUB_FUNC 0xdea197d4,sceDisplayGetMode
	STUB_FUNC 0xdba6c4c4,sceDisplayGetFramePerSec
	STUB_FUNC 0x7ed59bc4,sceDisplay_Unkonow00
	STUB_FUNC 0xa544c486,sceDisplay_Unkonow01
	STUB_FUNC 0x289d82fe,sceDisplaySetFrameBuf
	STUB_FUNC 0xeeda2e54,sceDisplayGetFrameBuf
	STUB_FUNC 0xb4f378fa,sceDisplay_Unkonow02
	STUB_FUNC 0x31c4baa8,sceDisplay_Unkonow03
	STUB_FUNC 0x9c6eaad7,sceDisplayGetVcount
	STUB_FUNC 0x4d4e10ec,sceDisplayIsVblank
	STUB_FUNC 0x36cdfade,sceDisplayWaitVblank
	STUB_FUNC 0x8eb9ec49,sceDisplayWaitVblankCB
	STUB_FUNC 0x984c27e7,sceDisplayWaitVblankStart
	STUB_FUNC 0x46f186c3,sceDisplayWaitVblankStartCB
	STUB_FUNC 0x773dd3a3,sceDisplayGetCurrentHcount
	STUB_FUNC 0x210eab3a,sceDisplayGetAccumulatedHcount
	STUB_END 

	STUB_START	"sceGe_user",0x40010000,0x00090005
	STUB_FUNC	0xe47e40e4,sceGeEdramGetAddr
	STUB_FUNC	0xb77905ea,sceGeEdramSetAddrTranslation
	STUB_FUNC	0xab49e76a,sceGeListEnQueue
	STUB_FUNC	0x1c0d95a6,sceGeListEnQueueHead
	STUB_FUNC	0x03444eb4,sceGeListSync
	STUB_FUNC	0xb287bd61,sceGeDrawSync
	STUB_FUNC	0xe0d68148,sceGeListUpdateStallAddr
	STUB_FUNC	0xa4fc06a4,sceGeSetCallback
	STUB_FUNC	0x05db22ce,sceGeUnsetCallback

	STUB_START "sceCtrl",0x40010000,0x000a0005 
	STUB_FUNC 0x6a2774f3,sceCtrlSetSamplingCycle
	STUB_FUNC 0x02baad91,sceCtrlGetSamplingCycle
	STUB_FUNC 0x1f4011e6,sceCtrlSetSamplingMode
	STUB_FUNC 0xda6b76a1,sceCtrlGetSamplingMode
	STUB_FUNC 0x3a622550,sceCtrlPeekBufferPositive
	STUB_FUNC 0xc152080a,sceCtrlPeekBufferNegative
	STUB_FUNC 0x1f803938,sceCtrlReadBufferPositive
	STUB_FUNC 0x60b81f86,sceCtrlReadBufferNegative
	STUB_FUNC 0xb1d0e5cd,sceCtrlPeekLatch
	STUB_FUNC 0x0b588501,sceCtrlReadLatch
	STUB_END

	STUB_START	"IoFileMgrForUser",0x40010000,0x00240005
	STUB_FUNC	0xe95a012b, sceIoIoctlAsync
	STUB_FUNC	0x63632449, sceIoIoctl
	STUB_FUNC	0x54f5fb11, sceIoDevctl
	STUB_FUNC	0x89aa9906, sceIoOpenAsync
	STUB_FUNC	0x109f50bc, sceIoOpen
	STUB_FUNC	0xff5940b6, sceIoCloseAsync
	STUB_FUNC	0x810c4bc3, sceIoClose
	STUB_FUNC	0x71b19e77, sceIoLseekAsync
	STUB_FUNC	0x27eb27b8, sceIoLseek
	STUB_FUNC	0x68963324, sceIoLseek32
	STUB_FUNC	0x1B385D8F, sceIoLseek32Async
	STUB_FUNC	0xa0b5a7c2, sceIoReadAsync
	STUB_FUNC	0x6a638d83, sceIoRead
	STUB_FUNC	0x0facab19, sceIoWriteAsync
	STUB_FUNC	0x42ec03ac, sceIoWrite
	STUB_FUNC	0xb29ddf9c, sceIoDopen
	STUB_FUNC	0xe3eb004c, sceIoDread
	STUB_FUNC	0xeb092469, sceIoDclose
	STUB_FUNC	0xf27a9c51, sceIoRemove
	STUB_FUNC	0x06a70004, sceIoMkdir
	STUB_FUNC	0x1117c65f, sceIoRmdir
	STUB_FUNC	0x55f4717d, sceIoChdir
	STUB_FUNC	0xace946e8, sceIoGetStat
	STUB_FUNC	0x779103a0, sceIoRename
	STUB_FUNC	0xb2a628c1, sceIoAssign
	STUB_FUNC	0x6d08a871, sceIoUnassign
	STUB_FUNC	0xe8bc6571, sceIoCancel
	STUB_FUNC	0xb293727f, sceIoChangeAsyncPriority
	STUB_FUNC	0xcb05f8d6, sceIoGetAsyncStat
	STUB_FUNC	0x08bd7374, sceIoGetDevType
	STUB_FUNC	0x3251ea56, sceIoPollAsync
	STUB_FUNC	0xa12a0514, sceIoSetAsyncCallback
	STUB_FUNC	0xab96437f, sceIoSync
	STUB_FUNC	0x35dbd746, sceIoWaitAsyncCB
	STUB_FUNC	0xe23eec33, sceIoWaitAsync
	STUB_FUNC	0xb8a740f4, sceIoChStat
	STUB_END

	STUB_START   "sceSuspendForUser",0x40000000,0x00020005 
	STUB_FUNC   0xEADB1BD7,"DisableSuspend" 
	STUB_FUNC   0x3AEE7261,"EnableSuspend" 
	STUB_END 

	STUB_START   "LoadExecForUser",0x40010000,0x20005 
	STUB_FUNC   0x5572A5F,sceKernelExitGame 
	STUB_FUNC   0x4AC57943,sceKernelRegisterExitCallback 
	STUB_END 

	STUB_START "scePower",0x40010000,0x002e0005 
	STUB_FUNC 0x2b51fe2f,scePower_Unkonow00 
	STUB_FUNC 0x442bfbac,scePower_Unkonow01 
	STUB_FUNC 0xefd3c963,scePowerTick 
	STUB_FUNC 0xedc13fe5,scePowerGetIdleTimer 
	STUB_FUNC 0x7f30b3b1,scePowerIdleTimerEnable 
	STUB_FUNC 0x972ce941,scePowerIdleTimerDisable 
	STUB_FUNC 0x27f3292c,scePowerBatteryUpdateInfo 
	STUB_FUNC 0xe8e4e204,scePower_Unkonow02 
	STUB_FUNC 0xb999184c,scePower_Unkonow03 
	STUB_FUNC 0x87440f5e,scePowerIsPowerOnline 
	STUB_FUNC 0x0afd0d8b,scePowerIsBatteryExist 
	STUB_FUNC 0x1e490401,scePowerIsBatteryCharging 
	STUB_FUNC 0xb4432bc8,scePowerGetBatteryChargingStatus 
	STUB_FUNC 0xd3075926,scePowerIsLowBattery 
	STUB_FUNC 0x78a1a796,scePower_Unkonow04 
	STUB_FUNC 0x94f5a53f,scePower_Unkonow05 
	STUB_FUNC 0xfd18a0ff,scePower_Unkonow06 
	STUB_FUNC 0x2085d15d,scePowerGetBatteryLifePercent 
	STUB_FUNC 0x8efb3fa2,scePowerGetBatteryLifeTime 
	STUB_FUNC 0x28e12023,scePowerGetBatteryTemp 
	STUB_FUNC 0x862ae1a6,scePowerGetBatteryElec 
	STUB_FUNC 0x483ce86b,scePowerGetBatteryVolt 
	STUB_FUNC 0x23436a4a,scePower_Unkonow07 
	STUB_FUNC 0x0cd21b1f,scePower_Unkonow08 
	STUB_FUNC 0x165ce085,scePower_Unkonow09 
	STUB_FUNC 0xd6d016ef,scePowerLock 
	STUB_FUNC 0xca3d34c1,scePowerUnlock 
	STUB_FUNC 0xdb62c9cf,scePowerCancelRequest 
	STUB_FUNC 0x7fa406dd,scePowerIsRequest 
	STUB_FUNC 0x2b7c7cf4,scePowerRequestStandby 
	STUB_FUNC 0xac32c9cc,scePowerRequestSuspend 
	STUB_FUNC 0x2875994b,scePower_Unkonow10 
	STUB_FUNC 0x3951af53,scePower_Unkonow11 
	STUB_FUNC 0x0074ef9b,scePowerGetResumeCount 
	STUB_FUNC 0x04b7766e,scePowerRegisterCallback 
	STUB_FUNC 0xdfa8baf8,scePowerUnregisterCallback 
	STUB_FUNC 0xdb9d28dd,scePower_Unkonow12 
	STUB_FUNC 0x843fbf43,scePowerSetCpuClockFrequency 
	STUB_FUNC 0xb8d7b3fb,scePowerSetBusClockFrequency 
	STUB_FUNC 0xfee03a2f,scePowerGetCpuClockFrequency 
	STUB_FUNC 0x478fe6f5,scePowerGetBusClockFrequency 
	STUB_FUNC 0xfdb5bfe9,scePowerGetCpuClockFrequencyInt 
	STUB_FUNC 0xbd681969,scePowerGetBusClockFrequencyInt 
	STUB_FUNC 0xb1a52c83,scePowerGetCpuClockFrequencyFloat 
	STUB_FUNC 0x9badb3eb,scePowerGetBusClockFrequencyFloat 
	STUB_FUNC 0x737486f2,scePowerSetClockFrequency 
	STUB_END 

	STUB_START	"sceAudio",0x40010000,0x00090005
	STUB_FUNC	0x136CAF51,sceAudioOutputBlocking
	STUB_FUNC	0xE2D56B2D,sceAudioOutputPanned
	STUB_FUNC	0x13F592BC,sceAudioOutputPannedBlocking
	STUB_FUNC	0x5EC81C55,sceAudioChReserve
	STUB_FUNC	0x6FC46853,sceAudioChRelease
	STUB_FUNC	0xE9D97901,sceAudioGetChannelRestLen
	STUB_FUNC	0xCB2E439E,sceAudioSetChannelDataLen 
	STUB_FUNC	0x95FD0C2D,sceAudioChangeChannelConfig
	STUB_FUNC	0xB7E1D8E7,sceAudioChangeChannelVolume
	STUB_END

	STUB_START	"UtilsForUser",0x40010000,0x00040005
	STUB_FUNC	0x91E4F6A7,sceKernelLibcClock
	STUB_FUNC	0x27CC57F0,sceKernelLibcTime
	STUB_FUNC	0x71EC4271,sceKernelLibcGettimeofday
	STUB_FUNC	0x79D1C3FA,sceKernelDcacheWritebackAll
	STUB_END

	STUB_START   "ThreadManForUser",0x40010000,0x000D0005
	STUB_FUNC   0x446D8DE6,sceKernelCreateThread
	STUB_FUNC   0xF475845D,sceKernelStartThread
	STUB_FUNC   0xAA73C935,sceKernelExitThread
	STUB_FUNC   0x9ACE131E,sceKernelSleepThread
	STUB_FUNC   0x55C20A00,sceKernelCreateEventFlag
	STUB_FUNC   0xEF9E4C70,sceKernelDeleteEventFlag
	STUB_FUNC   0x1FB15A32,sceKernelSetEventFlag
	STUB_FUNC   0x812346E4,sceKernelClearEventFlag
	STUB_FUNC   0x402FCF22,sceKernelWaitEventFlag
	STUB_FUNC   0x82826F70,sceKernelPollCallbacks
	STUB_FUNC   0xE81CAF8F,sceKernelCreateCallback
	STUB_FUNC   0x278C0DF5,sceKernelWaitThreadEnd
	STUB_FUNC   0x9FA03CD3,sceKernelDeleteThread
	STUB_END

	STUB_START "sceDmac",0x40010000,0x00020005
	STUB_FUNC   0x617f3fe6,sceDmacMemcpy
	STUB_FUNC   0xd97f94d8,sceDmacTryMemcpy
	STUB_END 

	STUB_START "SysMemUserForUser",0x40000000,0x00050005
	STUB_FUNC 0x237DBD4F,sceKernelAllocPartitionMemory
	STUB_FUNC 0xB6D61D02,sceKernelFreePartitionMemory
	STUB_FUNC 0x9D9A5BA1,sceKernelGetBlockHeadAddr
	STUB_FUNC 0xa291f107,sceKernelMaxFreeMemSize
	STUB_FUNC 0xf919f628,sceKernelTotalFreeMemSize
	STUB_END

	STUB_START "StdioForUser",0x40010000,0x00030005 
	STUB_FUNC 0x172D316E,sceKernelStdin 
	STUB_FUNC 0xA6BAB2E9,sceKernelStdout 
	STUB_FUNC 0xF78BA90A,sceKernelStderr 
	STUB_END

#ifdef PSPE
	STUB_START	"pspeDebug",0x40010000,0x00010005 
	STUB_FUNC	0xDEADBEAF pspeDebugWrite
	STUB_FUNC	0xDEADBEAE pspeDebugPrintf
#endif

###############################################################################

	.text

	.end _start

