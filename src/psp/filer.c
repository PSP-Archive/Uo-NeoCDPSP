//#include "main.h"

#include <string.h>
#include "psp.h"
#include "filer.h"

#define TITLE_COL ((31)|(26<<5)|(31<<10))
#define PATH_COL ((31)|(24<<5)|(28<<10))

#define FILE_COL ((20)|(20<<5)|(31<<10))
#define DIR_COL ((8)|(31<<5)|(8<<10))

#define SEL_COL ((30)|(30<<5)|(31<<10))
#define SELDIR_COL ((28)|(31<<5)|(28<<10))


#define INFOBAR_COL ((31)|(24<<5)|(20<<10))

#define MAX_PATH 512		//temp, not confirmed

#define MAXPATH 512
#define MAXNAME 256
#define MAX_ENTRY 1024

u32 new_pad,old_pad;

enum {
    TYPE_DIR=0x10,
    TYPE_FILE=0x20
};

struct dirent {
    u32 unk0;
    u32 type;
    u32 size;
    u32 unk[19];
    char name[0x108];
};


static struct dirent files[MAX_ENTRY];
static int nfiles;
struct dirent file;

extern int psp_cpuclock;
int neocddir;

////////////////////////////////////////////////////////////////////////
// クイックソート
int cmpFile(struct dirent *a, struct dirent *b)
{
	char ca, cb;
	int i, n, ret;

	if(a->type==b->type){
		n=strlen(a->name);
		for(i=0; i<=n; i++){
			ca=a->name[i]; cb=b->name[i];
			if(ca>='a' && ca<='z') ca-=0x20;
			if(cb>='a' && cb<='z') cb-=0x20;

			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}

	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}

void sort(struct dirent *a, int left, int right) {
	struct dirent tmp, pivot;
	int i, p;

	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(&a[i],&pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

// 拡張子管理用
const struct {
	char *szExt;
	int nExtId;
} stExtentions[] = {
    "zip",EXT_ZIP,
 		NULL, EXT_UNKNOWN
};

int getExtId(const char *szFilePath) {
	char *pszExt;
	int i;
	if((pszExt = strrchr(szFilePath, '.'))) {
		pszExt++;
		for (i = 0; stExtentions[i].nExtId != EXT_UNKNOWN; i++) {
			if (!strcasecmp(stExtentions[i].szExt,pszExt)) {
				return stExtentions[i].nExtId;
			}
		}
	}
	return EXT_UNKNOWN;
}


char *find_file(char *pattern,char *path){
	int fd,found;

	fd = sceIoDopen(path);
	if (fd<0) return NULL;
	found=0;
	while(1){
		if(sceIoDread(fd, &file)<=0) break;
		if (strcasestr(file.name,pattern)) {found=1;break;}
	}
	sceIoDclose(fd);
	if (found) return file.name;
	return NULL;
}

void getDir(const char *path) {
	int fd, b=0;
//	char *p;

	nfiles = 0;

	if(strcmp(path,"ms0:/")){
		strcpy(files[nfiles].name,"..");
		files[nfiles].type = TYPE_DIR;
		nfiles++;
		b=1;
	}

	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].name[0] == '.') continue;

		/*neogeo stuff*/
		if (!strcasecmp(files[nfiles].name,"ipl.txt")) neocddir=1;
		/**/

		if(files[nfiles].type == TYPE_DIR){
			strcat(files[nfiles].name, "/");
			nfiles++;
			continue;
		}
		if(getExtId(files[nfiles].name) != EXT_UNKNOWN) nfiles++;
	}
	sceIoDclose(fd);
	if(b)
		sort(files+1, 0, nfiles-2);
	else
		sort(files, 0, nfiles-1);
}

char LastPath[MAX_PATH];
char FilerMsg[256];
char FileName[MAX_PATH];

//int psp_ExitCheck(void);


int getFilePath(char *out)
{
//    int counter=0;
	unsigned long color=RGB_WHITE;
	int sel=0, rows=21, top=0, x, y, h, i, /*len,*/ bMsg=0, up=0,tempo;
	int pad,pad_cnt;
	int frame_cpt;
	int retval=0;

	char path[MAXPATH], oldDir[MAXNAME], *p;

	strcpy(path, LastPath);
//    strcpy(path,"ms0:/");
	if(FilerMsg[0])
		bMsg=1;

	getDir(path);

	pgDrawFrame(16,16,480-16,272-16,28|(28<<5)|(28<<10));
/*	char str[255];
	for (i=0;i<256;i++) str[i]=(i+1)&255;
	pgPrint(0,0,0x1F,str,0);
	pgScreenFlipV();
	for (;;){
	}
    */
  pad_cnt=5;
  old_pad=0;
	for(;;){
       // pgFillvram(0);
       frame_cpt++;
       show_background(192);
       pgDrawFrame(12,12,480-12,272-12,28|(28<<5)|(28<<10));
       pgDrawFrame(11,11,480-11,272-11,30|(30<<5)|(30<<10));

       new_pad=0;
       if (!pad_cnt) new_pad=get_pad();
       else pad_cnt--;
       pgWaitV();

			 if (new_pad) {
			 		bMsg=0;
			 		if (old_pad==new_pad) pad_cnt=2;
			 		else pad_cnt=5;
			 		old_pad=new_pad;
			 	}

		if(new_pad & PSP_CTRL_CIRCLE){
			if(files[sel].type == TYPE_DIR){
				if(!strcmp(files[sel].name,"..")){  up=1; }
                else{
					strcat(path,files[sel].name);
					getDir(path);
					sel=0;
					while (get_pad()) pgWaitV();
				}
			}else{
				pgFillBox(240-170+1,136-30+1,240+170-1,136+35-1,(1<<10)|(1<<5)|4);
				pgDrawFrame(240-170,136-30,240+170,136+35,(0<<10)|(0<<5)|2);
				// pgPrintCenter(15,(1<<10)|(2<<5)|24,"Warning : zipped game can be very slow",0);
				pgPrintCenter(17,(15<<10)|(31<<5)|31,"Try to uncompress the zip file",0);
				pgPrintCenter(19,(31<<10)|(31<<5)|31,"\1 to launch, \2 to browse",0);
				pgScreenFlipV();
				while (get_pad()) pgWaitV();
				while (1){
					pad=get_pad();
					if (pad) while (get_pad()) pgWaitV();
					if (pad&PSP_CTRL_CIRCLE) {
						strcpy(out, path);
						strcat(out, files[sel].name);
						strcpy(LastPath,path);
						retval=1;break;
					}
					if (pad&PSP_CTRL_CROSS) break;
				}
				if (retval) break;
			}
		}
        else /*if(new_pad & PSP_CTRL_CROSS)   { retval=0;break; }
        else*/ if(new_pad & PSP_CTRL_TRIANGLE){ up=1;     }
        else if(new_pad & PSP_CTRL_UP)      { sel--;    }
        else if(new_pad & PSP_CTRL_DOWN)    { sel++;    }
        else if(new_pad & PSP_CTRL_LEFT)    { sel-=10;if (sel<0) sel=0;  }
        else if(new_pad & PSP_CTRL_RIGHT)   { sel+=10;if(sel >= nfiles) sel=nfiles-1;  }
        else if(new_pad & PSP_CTRL_RTRIGGER) {
        		if (psp_cpuclock==222) psp_cpuclock=266;
        		else if (psp_cpuclock==266) psp_cpuclock=333;
        		else psp_cpuclock=222;
        		pad_cnt=20;
        }

		if(up){
			if(strcmp(path,"ms0:/")){
				p=strrchr(path,'/');
				*p=0;
				p=strrchr(path,'/');
				p++;
				strcpy(oldDir,p);
				strcat(oldDir,"/");
				*p=0;
				getDir(path);

				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!strcmp(oldDir, files[i].name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			up=0;
		}

		if (neocddir){
			neocddir=0;
			pgFillBox(240-150+1,136-15+1,240+150-1,136+22-1,(16<<10)|(10<<5)|7);
			pgDrawFrame(240-150,136-15,240+150,136+22,(12<<10)|(8<<5)|5);
			pgPrintCenter(16,(15<<10)|(31<<5)|31,"Found NEOCD Directory",0);
			pgPrintCenter(18,(31<<10)|(31<<5)|31,"\1 to launch, \2 to browse",0);
			pgScreenFlipV();
			while (get_pad()) pgWaitV();
			while (1){
				pad=get_pad();
				if (pad) while (get_pad()) pgWaitV();
				if (pad&PSP_CTRL_CIRCLE) {
					strcpy(out, path);
					p=strrchr(out,'/');
					*p=0;
					strcpy(LastPath,path);
					retval=2;break;
				}
				if (pad&PSP_CTRL_CROSS) break;
			}
			if (retval) break;

		}

		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=0;
		if(sel < 0)				sel=nfiles-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;

        if(bMsg) {
          pgPrint(1,0,TITLE_COL,FilerMsg,0);

        }
        else {
          pgPrint(1,0,PATH_COL,path,0);

        }
        pgPrintDecTrim(54,0,TITLE_COL,psp_cpuclock,0);
        pgPrint(57,0,TITLE_COL,"Mhz",0);
        pgPrint(1,33,INFOBAR_COL,"\1 OK \4 Parent dir.         \5,\7,\6,\b to browse list",0);

		// スクロールバー
		if(nfiles > rows){
			h = 219;
//			pgDrawFrame(445,25,446,248,setting.color[1]);
			pgDrawFrame(445,25,446,248,0x1f<<10);
			pgFillBox(448, h*top/nfiles + 27,
				460, h*(top+rows)/nfiles + 27,0x1f<<10);
//				460, h*(top+rows)/nfiles + 27,setting.color[1]);
		}

		x=4; y=4;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) {color = SEL_COL;pgPrintSel(x, y, color, files[top+i].name);}
			else {color = FILE_COL;pgPrint(x, y, color, files[top+i].name,0);}
			if (files[i].type==TYPE_DIR){
				if (color==SEL_COL) {color=SELDIR_COL;pgPrintSel(x, y,color, files[top+i].name);}
				else {color = DIR_COL;pgPrint(x, y, color, files[top+i].name,0);}
			}

			y+=1;
		}

//        counter++;
//        pgPrintHex(0,10,0x7fff,counter,1);
//        pgPrintHex(0,11,0x7fff,up,1);
		pgScreenFlipV();

        /*if(psp_ExitCheck()) {
            retval=-1;break;
        }*/
	}

	while (get_pad()) pgWaitV();
	return retval;
}


int InitFiler(char*msg,char*path)
{
    strcpy(FilerMsg,msg);
    strcpy(LastPath,path);
    memset(files,0,sizeof(files));
    return 1;
}
