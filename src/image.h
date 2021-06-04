#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

int	fread_header( FILE *fp, long offset, Uchar header[32] );



FILE	*fopen_image_edit( char *filename, char *mode, int *result );

int	append_blank( FILE *fp );
int	fwrite_protect( FILE *fp, int img, char *protect );
int	fwrite_name( FILE *fp, int img, char *name );
int	fwrite_format( FILE *fp, int img );
int	fwrite_unformat( FILE *fp, int img );

void	update_after_append_blank( int drv );
void	update_after_fwrite_protect( int drv, int img, char *protect );
void	update_after_fwrite_name( int drv, int img, char *name );
void	update_after_fwrite_format( int drv, int img );



#define	read_size_in_header( header )					     \
	 (long)header[DISK_SIZE  ]      + ((long)header[DISK_SIZE+1]<<8)  +  \
	((long)header[DISK_SIZE+2]<<16) + ((long)header[DISK_SIZE+3]<<24)


#endif	/* IMAGE_H_INCLUDED */
