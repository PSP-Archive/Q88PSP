
void init_ftable(void);
FILE *ex_fopen(const char *filename,char *mode);
int ex_fread(void *buf,int size,int block,FILE *fp);
int ex_fwrite(void *buf,int size,int block,FILE *fp);
long ex_fseek(FILE *fp,long ofs,int whence);
long ex_ftell(FILE *fp);
void ex_fclose(FILE *fp);
void ex_setdir(const char *dir);
