// pspsim.h

#ifndef __PSPSIM_H__
#define __PSPSIM_H__

void pspsim_init(void);
void pspsim_blit(void);
void pspsim_update(void);
void pspsim_free(void);

void pspsim_flip(void);
void pspsim_checkevt(void);

void pspsim_waitvblank(void);
void pspsim_setbufaddr(void *addr);
void pspsim_exit(int code);

unsigned char *pspsim_getvram(void);
unsigned short pspsim_keystate(void);
unsigned long  pspsim_tick(void);

#endif
