#define DEF_SAMPRATE 22050
#define SND_BUFLEN 16384//0x8000
//#define SND_BUFLEN_H (SND_BUFLEN/2)

extern short snd_buffer[SND_BUFLEN];
extern int snd_ptr;

extern int buf_pbptr;

static short tmp_buffer[SND_BUFLEN];
static int tmp_cnt = 0;
static int tmp_ptr = 0;
