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
