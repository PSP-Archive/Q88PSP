#include <stdio.h>

#include "quasi88.h"
#include "pg.h"

#define MAX_EXMEM 4 * 1024 * 1024

static long exmem_ptr = 0;


unsigned char exmem[MAX_EXMEM];
char exdir[256];

typedef struct 
{
	int fd;
	char *filename;
} EXFILE;

EXFILE ftable[16];
static int ftable_ptr = 0;

void init_ftable(void)
{
	int i;
	for(i = 0;i < 16;i++) ftable[i].fd = -1;
}

void *ex_malloc(long size)
{
	void *ptr;
	ptr = (void *)&exmem[exmem_ptr];
	exmem_ptr += size;

//	printf("memsize = %d,left = %d\n",size,MAX_EXMEM - exmem_ptr);

	return ptr;
}

void ex_free(void *ptr)
{
#if 0
	//メモリ開放してねぇ･･･
	exmem_ptr -= sizeof(ptr);
	void *new_ptr;
	new_ptr = (void *)&exmem[exmem_ptr];
#endif
}

void ex_setdir(const char *dir)
{
	strcpy(exdir,dir);
}

FILE *ex_fopen(const char *filename,char *mode)
{
	int flag = 0;
	int fd = -1;
	EXFILE *ep;
	char tmp_file[512];
	
	strcpy(tmp_file,exdir);
	strcat(tmp_file,filename);

	if (ftable_ptr > 15) 
		return NULL;

	while(*mode)
	{
		switch(*mode)
		{
			case 'r':
				flag |= SCE_O_RDONLY;
			break;
			case 'w':
//				flag |= SCE_O_WRONLY;
				flag |= SCE_O_RDWR;
			break;
			case '+':
				flag |= SCE_O_APPEND;
			break;
			case 'c':
				flag |= SCE_O_CREAT;
			break;
			case 't':
				flag |= SCE_O_TRUNC;
			break;
		}
		mode++;
	}
	fd = sceIoOpen(tmp_file,flag,0777);
	if (fd < 0)
		return NULL;

//	ep = &ftable[ftable_ptr++];

	//ここであいている(fd == -1)ftableを探す。
	ftable_ptr  = 0;
	while((ftable[ftable_ptr].fd != -1) && ftable_ptr < 15) ftable_ptr++;
	if (ftable_ptr > 15) return NULL;
	ep = &ftable[ftable_ptr];
	
	ep->fd = fd;
	ep->filename = tmp_file;
	
//	printf("ex_fopen : %s\n",filename);
	return (FILE *)ep;
}

int ex_fread(void *buf,int size,int block,FILE *fp)
{
	int result;
	EXFILE *ep;
	
	ep = (EXFILE *)fp;
	result = sceIoRead(ep->fd,buf,size*block);
	return (result <= 0 ? result : result/size);
}

int ex_fwrite(void *buf,int size,int block,FILE *fp)
{
	int result;
	EXFILE *ep;
	
	
	ep = (EXFILE *)fp;
	//いったん閉じてWrite属性で開きなおし
//	sceIoClose(ep->fd);
//	ep->fd = sceIoOpen(ep->filename,SCE_O_WRONLY|SCE_O_TRUNC,0777);
	result = sceIoWrite(ep->fd,buf,size*block);
	//さらに閉じてReadonly	属性で開きなおし
//	sceIoClose(ep->fd);
//	ep->fd = sceIoOpen(ep->filename,SCE_O_RDONLY,0777);
	return (result <= 0 ? result : result/size);
}

long ex_fseek(FILE *fp,long ofs,int whence)
{
	EXFILE *ep;
	ep = (EXFILE *)fp;
	sceIoLseek(ep->fd,ofs,whence);
	return 0;
}

long ex_ftell(FILE *fp)
{
	EXFILE *ep;
	ep = (EXFILE *)fp;
	return (long)sceIoLseek(ep->fd,0,SEEK_CUR);
}


void ex_fclose(FILE *fp)
{
	EXFILE *ep;
	
	ep = (EXFILE *)fp;
	sceIoClose(ep->fd);
	ep->fd = -1;
//	ftable_ptr--;
}

