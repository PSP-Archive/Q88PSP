#include "pspsim.h"
#include <stdio.h>
#include <SDL.h>

unsigned char *pspsim_vram = NULL;
unsigned char *pspsim_vram_cur = NULL;
SDL_Surface *pspsim_surface;

unsigned char pspsim_keydata[256];

int now_tick,next_tick;

#define BYTES_LINE 1024
#define SUR_X 480 // 512
#define SUR_Y 272

#define SDL_X 0x2d
#define SDL_Z 0x2c
#define SDL_C 0x2e

#define SDL_DW 0xd0
#define SDL_UP 0xc8
#define SDL_LF 0xcb
#define SDL_RT 0xcd

#define BIT_SELECT		0x0001
#define BIT_START		0x0008
#define BIT_TRIANGLE	0x1000
#define BIT_CIRCLE		0x2000
#define BIT_CROSS		0x4000
#define BIT_SQUARE		0x8000
#define BIT_UP			0x0010
#define BIT_RIGHT		0x0020
#define BIT_DOWN		0x0040
#define BIT_LEFT		0x0080
#define BIT_LT			0x0100
#define BIT_RT			0x0200

enum {
	KEY_SELECT = 0x00,
	KEY_START,
	KEY_TRIANGLE,
	KEY_CIRCLE,
	KEY_CROSS,
	KEY_SQUARE,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_LEFT,
	KEY_LT,
	KEY_RT
} pspsim_keys;

unsigned short tr_red[32];
unsigned short tr_green[32];
unsigned short tr_blue[32];


void pspsim_init(void)
{
	int i;
	char rmax,gmax,bmax;

	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
	{
		fprintf(stderr,"Unable to init SDL",SDL_GetError());
		exit(1);
	}
	SDL_WM_SetCaption("PSPSIM",NULL);

	pspsim_surface = SDL_SetVideoMode(SUR_X,SUR_Y,16,SDL_SWSURFACE); 

	if (!pspsim_surface)
	{
		fprintf(stderr,"Unable to make surface\n");
		SDL_Quit();
		exit(2);
	}

	for(i=0; i < 0x100; i++)
		pspsim_keydata[i] = 0;

	rmax = pspsim_surface->format->Rmask >> pspsim_surface->format->Rshift;
	gmax = pspsim_surface->format->Gmask >> pspsim_surface->format->Gshift;
	bmax = pspsim_surface->format->Bmask >> pspsim_surface->format->Bshift;

	fprintf(stderr,"r:%d g:%d b:%d\n",rmax,gmax,bmax);

	for(i=0; i < 0x20; i++)
	{
		tr_red[i]   = ((rmax*i)/0x20) << pspsim_surface->format->Rshift;
		tr_green[i] = ((gmax*i)/0x20) << pspsim_surface->format->Gshift;
		tr_blue[i]  = ((bmax*i)/0x20) << pspsim_surface->format->Bshift;
	}

	now_tick = next_tick = SDL_GetTicks();
	pspsim_vram = malloc(0x200000); // 2MB
	pspsim_vram_cur = pspsim_vram;

}

unsigned long pspsim_tick(void)
{
	return SDL_GetTicks();
}

unsigned short pspsim_keystate(void)
{
	unsigned long temp = 0;

// SDLK_DOWN if sym
	if (pspsim_keydata[KEY_DOWN])     temp |= BIT_DOWN;
	if (pspsim_keydata[KEY_UP])       temp |= BIT_UP;
	if (pspsim_keydata[KEY_LEFT])     temp |= BIT_LEFT;
	if (pspsim_keydata[KEY_RIGHT])    temp |= BIT_RIGHT;
	if (pspsim_keydata[KEY_CROSS])    temp |= BIT_CROSS;
	if (pspsim_keydata[KEY_CIRCLE])   temp |= BIT_CIRCLE;
	if (pspsim_keydata[KEY_SQUARE])   temp |= BIT_SQUARE;
	if (pspsim_keydata[KEY_TRIANGLE]) temp |= BIT_TRIANGLE;
	if (pspsim_keydata[KEY_SELECT])   temp |= BIT_SELECT;
	if (pspsim_keydata[KEY_START])    temp |= BIT_START;
	if (pspsim_keydata[KEY_LT])       temp |= BIT_LT;
	if (pspsim_keydata[KEY_RT])       temp |= BIT_RT;

	return temp;
}

void pspsim_shiftclr(unsigned short *dest,unsigned short *src,int len)
{
	int i;
	SDL_PixelFormat *fmt;

	fmt = pspsim_surface->format;
	unsigned short temp;
	for(i=0; i < len; i++)
	{
	// RGB
		temp = src[i];
		temp = tr_blue[(temp>>10)&0x1f]  | tr_green[(temp>>5)&0x1f] | tr_red[temp&0x1f];
		dest[i]=(temp&0xffff);
	}
}


void pspsim_blit(void)
{
	unsigned char *dest,*src;
	int y;
	int x;

	if (SDL_MUSTLOCK(pspsim_surface))
	{
		if (SDL_LockSurface(pspsim_surface) < 0) return;
	}

	for(y=0; y < SUR_Y; y++)
	{
		src = pspsim_vram_cur + (y * BYTES_LINE);
		dest = (unsigned char *)pspsim_surface->pixels +  (y * pspsim_surface->pitch );
		pspsim_shiftclr((unsigned short *)dest,(unsigned short *)src,SUR_X);
	}

	if (SDL_MUSTLOCK(pspsim_surface))
	{
		SDL_UnlockSurface(pspsim_surface);
	}
}

void pspsim_setbufaddr(void *addr)
{
	if (!addr) return;
	pspsim_vram_cur = addr;

}


void pspsim_update(void)
{
	SDL_UpdateRect(pspsim_surface,0,0,0,0);
}



unsigned char *pspsim_getvram(void)
{
	return pspsim_vram;
}

void pspsim_free(void)
{
	SDL_Quit();

	if (pspsim_vram) { free(pspsim_vram); pspsim_vram = NULL; }
}

void pspsim_key(SDLKey sym,unsigned char trig)
{
	switch(sym)
	{
		case SDLK_DOWN:
			pspsim_keydata[KEY_DOWN] = trig;
		break;
		case SDLK_UP:
			pspsim_keydata[KEY_UP] = trig;
		break;
		case SDLK_LEFT:
			pspsim_keydata[KEY_LEFT] = trig;
		break;
		case SDLK_RIGHT:
			pspsim_keydata[KEY_RIGHT] = trig;
		break;
		case SDLK_z:
			pspsim_keydata[KEY_CROSS] = trig;
		break;
		case SDLK_x:
			pspsim_keydata[KEY_CIRCLE] = trig;
		break;
		case SDLK_a:
			pspsim_keydata[KEY_SQUARE] = trig;
		break;
		case SDLK_s:
			pspsim_keydata[KEY_TRIANGLE] = trig;
		break;
		case SDLK_c:
			pspsim_keydata[KEY_SELECT] = trig;
		break;
		case SDLK_v:
			pspsim_keydata[KEY_START] = trig;
		break;
		case SDLK_q:
			pspsim_keydata[KEY_LT] = trig;
		break;
		case SDLK_w:
			pspsim_keydata[KEY_RT] = trig;
		break;
	}
}

void pspsim_checkevt(void)
{
	SDL_Event e;
	int cnt;

	for(cnt=0; cnt < 5 && SDL_PollEvent(&e); cnt++)
	{
		switch(e.type) {
			case SDL_KEYDOWN:
				pspsim_key(e.key.keysym.sym,1);
			break;
			case SDL_KEYUP:
				pspsim_key(e.key.keysym.sym,0);
			break;

			case SDL_QUIT:
				pspsim_exit(0);
			break;
		}

	}
}

void pspsim_flip(void)
{
	pspsim_blit();
	pspsim_update();
}

void pspsim_waitvblank(void)
{
	pspsim_flip();

	pspsim_checkevt();

	now_tick = SDL_GetTicks();
	if (now_tick < next_tick) SDL_Delay(next_tick-now_tick);
		next_tick = next_tick + 16;
}

void pspsim_exit(int code)
{
	pspsim_free();
	exit(code);
}

