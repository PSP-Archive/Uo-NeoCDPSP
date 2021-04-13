int zip_open(const char *name);
int zip_close(void);
int zopen(const char *filename);
int zread(int fd,void * buf,unsigned size);
int zclose(int fd);
char* zgets(int fd,char*buf,unsigned buflen);
int zsize(int fd);
