#ifndef FDC_H_INCLUDED
#define FDC_H_INCLUDED


extern int fdc_debug_mode;	/* FDC �f�o�b�O���[�h�̃t���O		*/
extern int disk_exchange;	/* �f�B�X�N�^������ւ��t���O		*/
extern int disk_ex_drv;		/* �f�B�X�N�^������ւ��h���C�u		*/

extern	int	FDC_flag;			/* FDC ���荞�ݐM��	*/
extern	int	fdc_wait;			/* FDC �� �E�G�C�g	*/



int	fdc_ctrl( int interval );

void	fdc_write( byte data );
byte	fdc_read( void );
byte	fdc_status( void );
void	fdc_TC( void );


void pc88fdc_break_point(void);

#endif	/* FDC_H_INCLUDED */
