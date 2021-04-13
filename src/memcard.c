#define MEMCARD_FILENAME "memcard.bin"



#include <stdio.h>
#include <stdlib.h>
#include "neocd.h"
//#include "console.h"

#define NEO4ALL_MEMCARD_SIZE 8192

unsigned char           neogeo_memorycard[NEO4ALL_MEMCARD_SIZE];
static char* memcard_filestate=MEMCARD_FILENAME;



static int save_savestate(void)
{
	int	ret=0;
#ifdef USE_MEMCARD
	FILE	*f;
        sound_disable();
	//menu_raise();
	//unlink(memcard_filestate);
	remove(memcard_filestate);

	f=fopen(convert_path(memcard_filestate),"w");
	if (f!=NULL)
	{
		int i;
		ret=1;
		for(i=0;i<8 && ret;i++)
		{
			fwrite(&neogeo_memorycard[1024*i],1,1024,f);
		}
		fclose(f);
	}
#endif
	//menu_unraise();
        sound_enable();
        init_autoframeskip();
	return ret;
}

extern void alert(char *, char *);

static int load_savestate(void)
{
	int	ret=0;
#ifdef USE_MEMCARD
	FILE	*f;

	f=fopen(convert_path(memcard_filestate),"r");
	if (f!=NULL){
		fread((void *)&neogeo_memorycard,1,NEO4ALL_MEMCARD_SIZE,f);
		fclose(f);
	}
#endif
	return ret;
}


void memcard_init(void)
{
		load_savestate();
}

void memcard_shutdown(void)
{
	save_savestate();
}

void memcard_update(void)
{
	/* check for memcard writes */
	if (memcard_write > 0) {
	   memcard_write--;
	   if(!memcard_write)
	   {
		   save_savestate();
		   init_autoframeskip();
	   }
	}
}
