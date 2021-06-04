#ifndef PC88SUB_H_INCLUDED
#define PC88SUB_H_INCLUDED


	/**** ïœêî ****/

extern	int	sub_load_rate;


	/**** ä÷êî ****/


void	pc88sub_init( void );
void	pc88sub_term( void );
void	pc88sub_break_point( void );
byte	sub_mem_read( word addr );
void	sub_mem_write( word addr, byte data );
byte	sub_io_in( byte port );
void	sub_io_out( byte port, byte data );


void	subcpu_keyscan_draw( void );


#endif	/* PC88SUB_H_INCLUDED */
