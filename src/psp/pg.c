
// primitive graphics for Hello World sce

#include "psp.h"
#include "pg.h"

#include "font.c"
#include "imageio.h"
#include <math.h>


static unsigned char nullbuffer[10240];

//variables
//char *pg_vramtop=(char *)0x04000000;
#define pg_vramtop ((char *)0x04000000)
long pg_screenmode;
long pg_showframe;
long pg_drawframe=0;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];

void pgPutChar_buffer(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	char *vptr0;		//pointer to vram
	char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	cfont=font+ch*8;
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}


void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
/*#ifdef PSP_MODE
        sceDisplayWaitVblank();
#else    */
        sceDisplayWaitVblankStart();
//#endif
}


char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+pg_drawframe*FRAMESIZE+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
}

u8 *pgGetVramShowAddr(unsigned long x,unsigned long y)
{
  int frame = pg_drawframe ^ 1;
  return pg_vramtop+frame*FRAMESIZE+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
    // return pg_vramtop+pg_drawframe*FRAMESIZE+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
}


void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str,int bg)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,bg,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgFillBoxHalfer(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			color = ((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE];
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = ((((color>>10)&31)*2/3)<<10)|
			((((color>>5)&31)*2/3)<<5)|((color&31)*2/3);
		}
	}
}

void pgPrintSel(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	int xm,ym;
	int i=0;
	const char *str2;

	xm=x;ym=y;
	str2=str;
	while (*str!=0 && xm<CMAX_X && ym<CMAX_Y) {
		str++;
		xm++;
		if (xm>=CMAX_X) {
			xm=0;
			ym++;
		}
	}
	pgFillBoxHalfer(16,y*8,464,ym*8+8,(7<<10)|(7<<5)|7);
	pgPrint(x,y,color,str2,0);
}

void pgPrintCenter(unsigned long y,unsigned long color,const char *str,int bg){
	unsigned long x=(480-strlen(str)*8)>>4;
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,bg,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX2_X && y<CMAX2_Y) {
		pgPutChar(x*16,y*16,color,0,*str,1,0,2);
		str++;
		x++;
		if (x>=CMAX2_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX4_X && y<CMAX4_Y) {
		pgPutChar(x*32,y*32,color,0,*str,1,0,4);
		str++;
		x++;
		if (x>=CMAX4_X) {
			x=0;
			y++;
		}
	}
}

void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = color;
		((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = color;
		((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = color;
	}
}

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgFillvram(unsigned long color)
{

	char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}

}

void pgFillAllvram(unsigned long color)
{
	u32 *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=(u32*)((u8*)pg_vramtop+0x40000000);
	for (i=0; i<2*FRAMESIZE/2; i++) {
		*vptr0++=color;
	}
}

void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	char *vptr0;		//pointer to vram
	char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;

	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}

}

void pgBitBltSt(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		if(yy%9){
			for (xx=80; xx>0; --xx) {
				dx=*(d++);
				d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
				d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
				v0[LINESIZE/2]=d0;
				v0[LINESIZE/2+1]=d1;
				*(v0++)=d0;
				*(v0++)=d1;
			}
			v0+=(LINESIZE-160);
		}else{
			for (xx=80; xx>0; --xx) {
				dx=*(d++);
				d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
				d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
				*(v0++)=d0;
				*(v0++)=d1;
			}
			v0+=(LINESIZE/2-160);
		}
		d+=8;
	}
}

//ÇøÇÂÇ¢ëÅÇ¢x1 - LCK
void pgBitBltN1(unsigned long x,unsigned long y,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long yy;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<144; yy++) {
#ifdef WIN32
		memcpy(v0,d,80);
#else
		__memcpy4a(v0,d,80);
#endif
		v0+=(LINESIZE/2-80);
		d+=8;
	}
}

//Ç†ÇÒÇ‹ÇËïœÇÌÇÁÇ»Ç¢x1.5 -LCK
void pgBitBltN15(unsigned long x,unsigned long y,unsigned long *d)
{
	unsigned short *vptr0;		//pointer to vram
	unsigned short *vptr;		//pointer to vram
	unsigned long xx,yy;

	vptr0=(unsigned short *)pgGetVramAddr(x,y);
	for (yy=0; yy<72; yy++) {
		unsigned long *d0=d+(yy*2)*88;
		vptr=vptr0;
		for (xx=0; xx<80; xx++) {
			unsigned long dd1,dd2,dd3,dd4;
			unsigned long dw;
			dw=d0[0];
			dd1=((vptr[0]           =((dw)     & 0x739c))) ;
			dd2=((vptr[2]           =((dw>>16) & 0x739c))) ;
			dw=d0[88];
			dd3=((vptr[0+LINESIZE*2]=((dw)     & 0x739c))) ;
			dd4=((vptr[2+LINESIZE*2]=((dw>>16) & 0x739c))) ;

			vptr++;
			*vptr=(dd1+dd2) >> 1;
			vptr+=(LINESIZE-1);
			*vptr=(dd1+dd3) >> 1;
			vptr++;
			*vptr=(dd1+dd2+dd3+dd4) >> 2;
			vptr++;
			*vptr=(dd2+dd4) >> 1;
			vptr+=(LINESIZE-1);
			*vptr=(dd3+dd4) >> 1;
			vptr+=(2-LINESIZE*2);
			d0+=1;
		}
		vptr0+=LINESIZE*3;
	}
}

//ÇÊÇ≠ÇÌÇ©ÇÒÇ»Ç¢x2 - LCK
void pgBitBltN2(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

//by z-rwt
void pgBitBltStScan(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

void pgBitBltSt2wotop(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

void pgBitBltSt2wobot(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<h-16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
}

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	char *vptr0;		//pointer to vram
	char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	cfont=font+ch*8;
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}

void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}


void pgScreenFlip()
{
  pg_showframe=pg_drawframe;
  pg_drawframe++;
  pg_drawframe&=1;
  sceDisplaySetFrameBuf(pg_vramtop+pg_showframe*FRAMESIZE,LINESIZE,PIXELSIZE,0);
}

void pgDisplayDrawFrame() {
  sceDisplaySetFrameBuf(pg_vramtop+pg_drawframe*FRAMESIZE,LINESIZE,PIXELSIZE,0);
}

void pgDisplayShowFrame() {
  sceDisplaySetFrameBuf(pg_vramtop+pg_showframe*FRAMESIZE,LINESIZE,PIXELSIZE,0);
}

void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}

int get_pad(void)
{
	static int n=0;
	SceCtrlData paddata;

#ifdef PSP_MODE
        sceCtrlReadBufferPositive(&paddata, 1);
        // kmg
        // Analog pad state
        if (paddata.Ly >= 0xD0) paddata.Buttons|=PSP_CTRL_DOWN;  // DOWN
        if (paddata.Ly <= 0x30) paddata.Buttons|=PSP_CTRL_UP;    // UP
        if (paddata.Lx <= 0x30) paddata.Buttons|=PSP_CTRL_LEFT;  // LEFT
        if (paddata.Lx >= 0xD0) paddata.Buttons|=PSP_CTRL_RIGHT; // RIGHT
#else
        sceCtrlReadBufferPositive(&paddata, 1);
#endif

	return paddata.Buttons;
}

void pgwaitPress(void){

		while (get_pad()) ;
		while (!get_pad()) ;
		while (get_pad()) ;
}


/******************************************************************************/


void pgcLocate(unsigned long x, unsigned long y)
{
	if (x>=CMAX_X) x=0;
	if (y>=CMAX_Y) y=0;
	pgc_csr_x[pg_drawframe?1:0]=x;
	pgc_csr_y[pg_drawframe?1:0]=y;
}


void pgcColor(unsigned long fg, unsigned long bg)
{
	pgc_fgcolor[pg_drawframe?1:0]=fg;
	pgc_bgcolor[pg_drawframe?1:0]=bg;
}


void pgcDraw(char drawfg, char drawbg)
{
	pgc_fgdraw[pg_drawframe?1:0]=drawfg;
	pgc_bgdraw[pg_drawframe?1:0]=drawbg;
}


void pgcSetmag(char mag)
{
	pgc_mag[pg_drawframe?1:0]=mag;
}

void pgcCls()
{
	pgFillvram(pgc_bgcolor[pg_drawframe]);
	pgcLocate(0,0);
}

void pgcPutchar_nocontrol(const char ch)
{
	pgPutChar(pgc_csr_x[pg_drawframe]*8, pgc_csr_y[pg_drawframe]*8, pgc_fgcolor[pg_drawframe], pgc_bgcolor[pg_drawframe], ch, pgc_fgdraw[pg_drawframe], pgc_bgdraw[pg_drawframe], pgc_mag[pg_drawframe]);
	pgc_csr_x[pg_drawframe]+=pgc_mag[pg_drawframe];
	if (pgc_csr_x[pg_drawframe]>CMAX_X-pgc_mag[pg_drawframe]) {
		pgc_csr_x[pg_drawframe]=0;
		pgc_csr_y[pg_drawframe]+=pgc_mag[pg_drawframe];
		if (pgc_csr_y[pg_drawframe]>CMAX_Y-pgc_mag[pg_drawframe]) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-pgc_mag[pg_drawframe];
//			pgMoverect(0,pgc_mag[pg_drawframe]*8,SCREEN_WIDTH,SCREEN_HEIGHT-pgc_mag[pg_drawframe]*8,0,0);
		}
	}
}

void pgcPutchar(const char ch)
{
	if (ch==0x0d) {
		pgc_csr_x[pg_drawframe]=0;
		return;
	}
	if (ch==0x0a) {
		if ((++pgc_csr_y[pg_drawframe])>=CMAX_Y) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-1;
//			pgMoverect(0,8,SCREEN_WIDTH,SCREEN_HEIGHT-8,0,0);
		}
		return;
	}
	pgcPutchar_nocontrol(ch);
}

void pgcPuthex2(const unsigned long s)
{
	char ch;
	ch=((s>>4)&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
	ch=(s&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
}


void pgcPuthex8(const unsigned long s)
{
	pgcPuthex2(s>>24);
	pgcPuthex2(s>>16);
	pgcPuthex2(s>>8);
	pgcPuthex2(s);
}

/******************************************************************************/





//////////////////////////
void pgPrintHex(int x,int y,short col,unsigned int hex,int bg)
{
    char string[9],o;
    int i,a;
    memset(string,0,sizeof(string));

    for(i=0;i<8;i++) {
        a = (hex & 0xf0000000)>>28;
        switch(a) {
          case 0x0a: o='A'; break;
          case 0x0b: o='B'; break;
          case 0x0c: o='C'; break;
          case 0x0d: o='D'; break;
          case 0x0e: o='E'; break;
          case 0x0f: o='F'; break;
          default:   o='0'+a; break;
        }
        hex<<=4;
        string[i]=o;
    }
    pgPrint(x,y,col,string,bg);
}

void pgPrintDec(int x,int y,short col,unsigned int dec,int bg)
{
    char string[12];
    int i,a;

    for(i=0;i<11;i++) {
        a = dec % 10;
    	dec/=10;
        string[10-i]=0x30+a;
//        if(dec=0 && a==0) break;
    }
    string[i]=0;
    pgPrint(x,y,col,string,bg);
}

void pgPrintDecTrim(int x,int y,short col,unsigned int dec,int bg)
{
    char string[12],stringfinal[12];
    int i,a;

    for(i=0;i<11;i++) {
        a = dec % 10;
    	dec/=10;
        string[10-i]=0x30+a;
//        if(dec=0 && a==0) break;
    }
    string[i]=0;
    a=0;
    while ((string[a]=='0')&&(a<i-1)) a++;
    pgPrint(x,y,col,string+a,bg);
}
