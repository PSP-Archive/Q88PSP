/************************************************************************/
/*									*/
/* �ꎞ��~���� (OS�ˑ�)						*/
/*									*/
/*	�ϐ� pause_by_focus_out �ɂ�菈�����ς��			*/
/*	�Epause_by_focus_out == 0 �̎�					*/
/*		ESC���������Ɖ����B	��ʒ����� PAUSE�ƕ\��		*/
/*	�Epause_by_focus_out != 0 �̎�					*/
/*		X �̃}�E�X����ʓ��ɓ���Ɖ���				*/
/*									*/
/* �y�֐��z								*/
/* void pause_init( void )						*/
/*									*/
/* void pause_main( void )						*/
/*									*/
/************************************************************************/
#include <SDL.h>

#include "quasi88.h"
#include "pause.h"

#include "emu.h"
#include "initval.h"
#include "indicator.h"
#include "graph.h"
#include "screen.h"
#include "device.h"
#include "snddrv.h"



int	pause_by_focus_out = FALSE;


void	pause_quit_signal( void )
{
}


void	pause_init( void )
{
  xmame_sound_suspend();
}


void	pause_main( void )
{
  SDL_Event E;
  int	exit_flag = FALSE;

  SDL_EnableKeyRepeat(500, 30);

  indicate_change_pause();

  redraw_screen( TRUE );


  while( exit_flag == FALSE ){

    SDL_PumpEvents();
    if( SDL_PeepEvents(&E, 1, SDL_GETEVENT,
		     SDL_EVENTMASK(SDL_QUIT)) ){
      if (E.type == SDL_QUIT) {
	emu_mode = QUIT;
      }
    }
    if( SDL_PeepEvents(&E, 1, SDL_GETEVENT,
		     SDL_EVENTMASK(SDL_KEYDOWN)|SDL_EVENTMASK(SDL_KEYUP)) ){

      if( E.type==SDL_KEYDOWN ){

	if( E.key.keysym.sym==SDLK_ESCAPE )

	  exit_flag = TRUE;
	  if( check_break_point_PC() ) emu_mode = EXECUTE_BP;
	  else                         emu_mode = EXECUTE;

      }else if( E.key.keysym.sym==SDLK_F12 ){

	  exit_flag = TRUE;
	  emu_mode = MENU;

	}
    }
  }


	/* �����̃C�x���g�����ׂĔj�� */

  SDL_EnableKeyRepeat(0, 30);


  indicator_flag = ( indicator_flag & ~INDICATE_MES_MASK );
  redraw_screen( FALSE );


  pause_by_focus_out = FALSE;

  xmame_sound_resume();
}
