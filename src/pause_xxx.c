/************************************************************************/
/*									*/
/* 一時停止処理 (OS依存)						*/
/*									*/
/*	変数 pause_by_focus_out により処理が変わる			*/
/*	・pause_by_focus_out == 0 の時					*/
/*		ESCが押されると解除。	画面中央に PAUSEと表示		*/
/*	・pause_by_focus_out != 0 の時					*/
/*		X のマウスが画面内に入ると解除				*/
/*									*/
/* 【関数】								*/
/* void pause_init( void )						*/
/*									*/
/* void pause_main( void )						*/
/*									*/
/************************************************************************/

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
}


void	pause_main( void )
{
}
