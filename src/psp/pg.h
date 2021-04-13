// primitive graphics for Hello World PSP
#ifndef __PG_H__
#define __PG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#include "syscall.h"

extern long pg_screenmode;

#define RGB(r,g,b) (short)((((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|(((r>>3) & 0x1F)<<0)|0x8000)
#define RGB_WHITE  RGB(255,255,255)
#define RGB_BLACK  RGB(0,0,0)
#define RGB_BLUE   RGB(0,0,255)
#define RGB_GREEN  RGB(0,255,0)
#define RGB_RED    RGB(255,0,0)
#define RGB_YELLOW RGB(255,255,0)

extern u32 new_pad, now_pad;
extern SceCtrlData paddata;

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define		PIXELSIZE	1				//in short
#define		LINESIZE	512				//in short
#define		FRAMESIZE	0x44000			//in byte

//480*272 = 60*34
#define CMAX_X 60
#define CMAX_Y 34
#define CMAX2_X 30
#define CMAX2_Y 17
#define CMAX4_X 15
#define CMAX4_Y 8

void pgInit();
void pgWaitV();
void pgWaitVn(unsigned long count);
void pgScreenFrame(long mode,long frame);
void pgScreenFlip();
void pgScreenFlipV();
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str,int bg);
void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgFillvram(unsigned long color);

void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d);
void pgBitBltN1(unsigned long x,unsigned long y,unsigned long *d);
void pgBitBltN15(unsigned long x,unsigned long y,unsigned long *d);
void pgBitBltN2(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgBitBltSt(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgBitBltStScan(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgBitBltSt2wotop(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);
void pgBitBltSt2wobot(unsigned long x,unsigned long y,unsigned long h,unsigned long *d);

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);
void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);

char *pgGetVramAddr(unsigned long x,unsigned long y);


void pgMain(unsigned long args, void *argp);

void readpad(void);

void pgcPuthex2(const unsigned long s);
void pgcPuthex8(const unsigned long s);


extern u32 now_tick;

//optimize
#ifndef WIN32
//long�z����R�s�[�B�z�񋫊E��4�o�C�g�A���C������Ă���K�v����
static inline void __memcpy4(void *d, void *s, unsigned long c)
{
	unsigned long *dl=(unsigned long *)d;
	unsigned long *sl=(unsigned long *)s;
	for (; c; --c) *dl++=*sl++;
}


//long�z��ɃZ�b�g�B�z�񋫊E��4�o�C�g�A���C������Ă���K�v����
static inline void __memset4(void *d, unsigned long v, unsigned long c)
{
	unsigned long *dl=(unsigned long *)d;
	for (; c; --c) *dl++=v;
}


//long�z����R�s�[�B�z�񋫊E��4�o�C�g�A���C������Ă���K�v����
//�R���p�C���̍œK���ɂ���ė\�����Ȃ��R�[�h����������邽�߁A�\���ɒ��ӂ̂��ƁB
static inline void __memcpy4aa(void *d, void *s, unsigned long c)
{
	unsigned long wk0,wk1,wk2,wk3;

	asm volatile (
		"		.set	push"				"\n"
		"		.set	noreorder"			"\n"

		"		move	%1,%4 "				"\n"
		"		move	%2,%5 "				"\n"
		"		move	%3,%6 "				"\n"

		"1:		lw		%0,0(%2) "			"\n"
		"		addiu	%3,%3,-1 "			"\n"
		"		addiu	%2,%2,4 "			"\n"
		"		sw		%0,0(%1) "			"\n"
		"		bnez	%3,1b "				"\n"
		"		addiu	%1,%1,4 "			"\n"

		"		.set	pop"				"\n"

			:	"=&r" (wk0),	// %0
				"=&r" (wk1),	// %1
				"=&r" (wk2),	// %2
				"=&r" (wk3)		// %3
			:	"r" (d),		// %4
				"r" (s),		// %5
				"r" (c)			// %6
	);
}

//long�z����R�s�[�B�z�񋫊E��4�o�C�g�A���C������Ă���K�v����
//s,d�͎Q�Ɠn�������ɂȂ�̂ŁA���^�[����͕ύX����Ă���ƍl�����ق����ǂ�
//�R���p�C���̍œK���ɂ���ė\�����Ȃ��R�[�h����������邽�߁A�\���ɒ��ӂ̂��ƁB__memcpy4�̂ق������S�B
//�Ƃ����܂���c�ŏ����Ă��S�R�ς��Ȃ��悤�ȁB��ݶޯ��̂ɁB
static inline void __memcpy4a(unsigned long *d, unsigned long *s, unsigned long c)
{
	unsigned long wk,counter;

	asm volatile (
		"		.set	push"				"\n"
		"		.set	noreorder"			"\n"

		"		move	%1,%4 "				"\n"
		"1:		lw		%0,0(%3) "			"\n"
		"		addiu	%1,%1,-1 "			"\n"
		"		addiu	%3,%3,4 "			"\n"
		"		sw		%0,0(%2) "			"\n"
		"		bnez	%1,1b "				"\n"
		"		addiu	%2,%2,4 "			"\n"

		"		.set	pop"				"\n"

			:	"=&r" (wk),		// %0
				"=&r" (counter)	// %1
			:	"r" (d),		// %2
				"r" (s),		// %3
				"r" (c)			// %4
	);
}

#endif


void pgPrintHex(int x,int y,short col,unsigned int hex,int bg);
void pgPrintDec(int x,int y,short col,unsigned int dec,int bg);
void pgPrintDecTrim(int x,int y,short col,unsigned int dec,int bg);


void pgMemClr(void* ptr,int size);
void pgMemCpy(void* dst,void* src,int size);
void pgwaitPress(void);
int get_pad(void);
void pgPrintCenter(unsigned long y,unsigned long color,const char *str,int bg);
void pgFillAllvram(unsigned long color);
void pgPrintSel(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgDisplayDrawFrame();
void pgDisplayShowFrame();
u8 *pgGetVramShowAddr(unsigned long x,unsigned long y);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
