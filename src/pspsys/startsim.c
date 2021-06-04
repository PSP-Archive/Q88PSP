// startsim.c by BouKiCHi
// 20060228

#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include "pspsim.h"

extern int xmain(int argc, char *argv);

int main(int argc,char *argv[])
{
	int ret;
	char path[512];

	pspsim_init();
	getcwd(path,512);
	strcat(path,"/file.exe");
	ret = xmain(strlen(path),path);
	pspsim_free();

	return ret;
}



