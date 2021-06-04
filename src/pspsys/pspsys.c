// pspsys.c by BouKiCHi
//

#include "pspsim.h"
#include "pspsys.h"
// #include <memory.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

DIR *pspsys_dir;
char dopen_name[256];

void sceDisplayWaitVblankStart(void)
{
	pspsim_waitvblank();
}

void sceDisplayWaitVblank(void)
{
	pspsim_waitvblank();
}

void sceDisplaySetMode(long unk1,long unk2,long unk3)
{
	fprintf(stderr,
		"sceDisplaySetMode : %d : %d : %d\n",
		unk1,unk2,unk3);
}

void sceDisplaySetFrameBuf(char *top,long lsize,long psize,long unk)
{
//	fprintf(stderr,
//		"sceDisplaySetFrameBuf : %08x : ls=%d : ps=%d\n",
//		top,lsize,psize);
	pspsim_setbufaddr(top);
	pspsim_flip();
	pspsim_checkevt();
}

void sceCtrlInit(int flag)
{
	fprintf(stderr,
		"sceCtrlInit : %d\n",
		flag);
}

void sceCtrlSetAnalogMode(int flag)
{
	fprintf(stderr,
		"sceCtrlSetAnalogMode : %d\n",
		flag);
}
void sceCtrlRead(ctrl_t *data,int flag)
{
	int i;

	memset(data,0,sizeof(ctrl_t));
	for(i=0; i < 4; i++)
		data->analog[i] = 0x80;

	data->buttons = pspsim_keystate();
}

void sceCtrlPeek(ctrl_t *data,int flag)
{
	int i;

	memset(data,0,sizeof(ctrl_t));
	for(i=0; i < 4; i++)
		data->analog[i] = 0x80;
	data->buttons = pspsim_keystate();
}

/* file */
int sceIoOpen(const char *filename,int mode,int flag)
{
	int mflag;

	mflag = 0;
	if ((mode & PSP_RDONLY) == PSP_RDONLY) mflag = O_RDONLY;
	if ((mode & PSP_WRONLY) == PSP_WRONLY) mflag = O_WRONLY;
	if ((mode & PSP_RDWR) == PSP_RDWR)     mflag = O_RDWR;

	if ((mode & PSP_APPEND) == PSP_APPEND) mflag |= O_APPEND;
	if ((mode & PSP_CREAT) == PSP_CREAT)   mflag |= O_CREAT;
	if ((mode & PSP_TRUNC) == PSP_TRUNC)   mflag |= O_TRUNC;

#ifdef __WIN32__
	mflag |= O_BINARY;
#endif
	fprintf(stderr,
		"sceIoOpen : %s : %d : %d\n",filename,mode,flag);
	return open(filename,mflag,flag);
}

void sceIoClose(int fd)
{
	close(fd);
}

int sceIoRead(int fd,void *data,int size)
{
	int temp;
	struct stat st;

//	fstat(fd,&st);
//	if (size > st.st_size-1) size = st.st_size-1;

	temp = read(fd,data,size);
	return temp;
}

int sceIoWrite(int fd,void *data,int size)
{
	return write(fd,data,size);
}

long long sceIoLseek(int fd,i64 offset,int direct)
{
	return lseek(fd,offset,direct);
}


int sceIoRemove(const char *filename)
{
	return 0;

}

int sceIoMkdir(const char *dir,int mode)
{
	fprintf(stderr,
		"sceIoMkdir : %s : %d \n",dir,mode);

#ifdef WIN32
	return mkdir(dir);
#else
	return mkdir(dir,mode);
#endif
}

int sceIoRmdir(const char *dir)
{
	return 0;

}
int sceIoRename(const char *oldname,const char *newname)
{
	return 0;

}

int sceIoDopen(const char *fn)
{
	fprintf(stderr,
		"sceIoDopen : %s \n",fn);

	strcpy(dopen_name,fn);
	pspsys_dir = opendir(fn);

	return 0;

}

void pspsys_convtime(psp_de_tm *dst,struct tm *src)
{
	dst->sec = src->tm_sec;
	dst->min = src->tm_min;
	dst->hour = src->tm_hour;
	dst->mday = src->tm_mday;
	dst->mon = src->tm_mon;
	dst->year = src->tm_year+1900;
}

int sceIoDread(int desc,psp_dirent *pde)
{
	struct tm *tim;
	struct stat st;
	struct dirent *de;
	char fn[512];

	de = readdir(pspsys_dir);

	if (!de) return 0;
	memset(pde,0,sizeof(pde));
	strcpy(fn,dopen_name);
	strcat(fn,de->d_name);

	strcpy(pde->name,de->d_name);

	stat(fn,&st);

	if (S_ISDIR(st.st_mode))
		pde->type = PSPTYPE_DIR;
	else
		pde->type = PSPTYPE_FILE;

	pspsys_convtime(&pde->ctime,localtime(&st.st_ctime));
	pspsys_convtime(&pde->mtime,localtime(&st.st_mtime));
	pspsys_convtime(&pde->atime,localtime(&st.st_atime));


	return 1;
}
void sceIoDclose(int fd)
{
	closedir(pspsys_dir);
}

void sceAudio_0()
{
}
void sceAudio_1()
{
}
void sceAudio_2(long unk1,long unk2,long unk3,void *unk4)
{
}
void sceAudio_3(long unk1,long samplecount,long unk2)
{
}
void sceAudio_4(long handle)
{
}
void sceAudio_5()
{
}
void sceAudio_6(long unk1,long unk2)
{
}
void sceAudio_7()
{
}
void sceAudio_8()
{
}

/* kernel */

int sceKernelCreateThread(const char *name,
		psp_thread_func ptr_func,u32 prio,u32 stack,u32 attr,void *unk)
{
	fprintf(stderr,
		"sceKernelCreateThread : %s \n",name);

	return 0;

}

int sceKernelCreateCallback(const char *name,psp_thread_func ptr_func)
{
	fprintf(stderr,
		"sceKernelCreateCallback : %s \n",name);

	return 0;
}

int scePowerRegisterCallback(int unk,int handle)
{
	return 0;
}

int  sceKernelStartThread(int handle,int arg0,void *data)
{
	return 0;

}
void sceKernelExitThread(int ret)
{
}
int  sceKernelWaitThreadEnd(int handle,void *p)
{
	return 0;
}

int  sceKernelDeleteThread(int handle)
{
	return 0;
}

void sceKernelExitGame(void)
{
}

void SetExitCallback(int id)
{
}


void KernelPollCallbacks()
{
}


/* system */

u32 sceKernelLibcClock(void)
{
	return pspsim_tick() * 1000; // ordering micro sec
}

u32 sceKernelLibcTime(u32 *ptr)
{
	return time(ptr);
}
int scePowerGetBatteryLifeTime(void)
{
	return 100;
}

int scePowerGetBatteryLifePercent(void)
{
	return 100;
}

int scePowerIsPowerOnline(void)
{
	return 1;
}

int scePowerIsBatteryExist(void)
{
	return 1;
}

long scePowerSetClockFrequency(long p1,long p2,long p3)
{
	return 0;
}

int sceDmacMemcpy(void *dest,void *src,long size)
{
	return 0;
}
