/* pspsys.h by BouKiCHi */

#ifndef __PSPSYS_H__
#define __PSPSYS_H__

typedef long long i64;
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

/* display */
void sceDisplayWaitVblankStart(void);
void sceDisplayWaitVblank(void);
void sceDisplaySetMode(long,long,long);
void sceDisplaySetFrameBuf(char *top,long lsize,long psize,long);

typedef struct 
{
	u32 frame;
	u32 buttons;
	u8 analog[4];
	u32 unused;
} ctrl_t;

/* control */
void sceCtrlInit(int flag);
void sceCtrlSetAnalogMode(int flag);
void sceCtrlRead(ctrl_t *data,int flag);
void sceCtrlPeek(ctrl_t *data,int flag);

/* file */

#define PSP_RDONLY 0x01
#define PSP_WRONLY 0x02
#define PSP_RDWR   0x03
#define PSP_NBLOCK 0x10
#define PSP_APPEND 0x100
#define PSP_CREAT  0x0200
#define PSP_TRUNC  0x0400
#define PSP_NOWAIT 0x8000

#define PSP_SEEK_SET 0x00
#define PSP_SEEK_CUR 0x01
#define PSP_SEEK_END 0x02


int sceIoOpen(const char *filename,int mode,int flag);
void sceIoClose(int fd);
int sceIoRead(int fd,void *data,int size);
int sceIoWrite(int fd,void *data,int size);
long long  sceIoLseek(int fd,i64 offset,int direct);
int sceIoRemove(const char *filename);
int sceIoMkdir(const char *dir,int flag);
int sceIoRmdir(const char *dir);
int sceIoRename(const char *oldname,const char *newname);

typedef struct 
{
	u16 unk[2];
	u16 year;
	u16 mon;
	u16 mday;
	u16 hour;
	u16 min;
	u16 sec;
} psp_de_tm;

#define PSPTYPE_DIR 0x10
#define PSPTYPE_FILE 0x20

typedef struct
{
	u32 unk1;
	u32 type;
	u32 size;
	psp_de_tm ctime;
	psp_de_tm atime;
	psp_de_tm mtime;
	u32 unk[7];
	char name[0x108];
} psp_dirent;

/* directory */
int sceIoDopen(const char *fn);
int sceIoDread(int desc,psp_dirent *de);
void sceIoDclose(int fd);


/* audio */
void sceAudio_0();
void sceAudio_1();
void sceAudio_2(long,long,long,void *);
void sceAudio_3(long unk,long samplecount,long);
void sceAudio_4(long handle);
void sceAudio_5();
void sceAudio_6(long,long);
void sceAudio_7();
void sceAudio_8();

/* thread */

typedef int (*psp_thread_func)(int len,void *ptr);
int sceKernelCreateThread(const char *name,
		psp_thread_func ptr_func,u32 prio,u32 stack,u32 attr,void *unk);

int  sceKernelStartThread(int handle,int arg0,void *data);
void sceKernelExitThread(int ret);
int  sceKernelWaitThreadEnd(int handle,void *p);
int  sceKernelDeleteThread(int handle);

/* system */

u32 sceKernelLibcClock(void);
u32 sceKernelLibcTime(u32 *ptr);
int scePowerGetBatteryLifeTime(void);
int scePowerGetBatteryLifePercent(void);
int scePowerIsPowerOnline(void);
int scePowerIsBatteryExist(void);

long scePowerSetClockFrequency(long,long,long);
int sceDmacMemcpy(void *dest,void *src,long size);

#endif
