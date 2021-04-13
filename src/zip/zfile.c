#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <string.h>

#include "unzip.h"
#include "psp.h"



#define	PATH_MAX	256
static unzFile unzfile=NULL;
static char basedir[PATH_MAX];
char* basedirend = basedir;

int zip_open(const char *name)
{
	char path[PATH_MAX];
	unzfile = unzOpen(convert_path(name));
	if (unzfile) return (int)unzfile;

	strcpy(basedir,convert_path(name));
			
	basedirend = basedir + strlen(basedir);
	*basedirend++='/';
	
	return -1;
}

int zip_close(void)
{
	if (unzfile) {
		unzClose(unzfile);
		unzfile = NULL;
	}
}

#define printf	pspDebugScreenPrintf

int zopen(const char *filename)
{
  /* Carefull zopen now returns 0 in case of error to be compatible
     with fopen */
  if (unzfile==NULL) {
    return (int)fopen(convert_path(filename),"rb");
  }

  printf("locating %s\n",filename);
  int ret = unzLocateFile(unzfile,filename,0);
  if (ret!=UNZ_OK) return (int)fopen(convert_path(filename),"rb");
  printf("opening %s\n",filename);
  ret = unzOpenCurrentFile(unzfile);
  if (ret!=UNZ_OK) return 0;
  return (int)unzfile;
}

int zread(int fd,void * buf,unsigned size)
{
 if (unzfile == NULL)
   return fread(buf,1,size,(FILE*)fd);
 printf("reading file.. %d bytes\n",size);
	return unzReadCurrentFile(unzfile,buf,size);
}

int zsize(int fd) {
  /* Return the size of the file */
   unz_file_info info;
  if (unzfile == NULL) {
    FILE *f = (FILE*)fd;
    int len,pos = ftell(f);
    fseek(f,0L,SEEK_END);
    len = ftell(f);
    fseek(f,pos,SEEK_SET);
    return len;
  }
  unzGetCurrentFileInfo(unzfile,&info,NULL,0,NULL,0,NULL,0);
  return info.uncompressed_size;
}

int zclose(int fd)
{
	if (unzfile == NULL) {
		if (fd!=-1) fclose((FILE*)fd);
		return 0;
	}
			       printf("closing file\n");
	return unzCloseCurrentFile(unzfile);
}

char* zgets(int fd,char*buf,unsigned buflen)
{
	int i;
	char ch;
	for(i=0;i<buflen-1;) {
		int r = zread(fd,&ch,1);
		if (r<=0) break;
		buf[i++] = ch;
		if (ch=='\n') break;
	}
	buf[i] = 0;
	return buf;
}
