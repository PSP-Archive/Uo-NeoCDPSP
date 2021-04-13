#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <pspgu.h>
#include <pspaudio.h>
#include <psppower.h>
#include <pspdisplay.h>

#include "psp.h"
#include "imageio.h"
#include "mad.h"
#include "blitter.h"
#include "menu.h"
#include "filer.h"
#include "pg.h"
#include "sound/streams.h"
#include "neocd.h"
#include "ingame.h"

#define SLICE_SIZE 64 // change this to experiment with different page-cache sizes
//static unsigned short __attribute__((aligned(16))) pixels[512*272];
struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};

/* Define the module info section */
PSP_MODULE_INFO("NEOCDPSP", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);


#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define NEO_WIDTH 320

IMAGE* bg_img;

extern volatile int			cdda_playing;
volatile int neogeo_soundmute;
volatile int SndThread_Active;
volatile int MP3Thread_Active;

char launchDir[256];
char tmp_str[256];

extern volatile int cdda_autoloop;

int psp_cpuclock;
int psp_vsync;

char MP3_file[256];
int MP3_newfile,MP3_playing;
int MP3_pause;


int snd_thread=-1;
int mp3_thread=-1;


volatile int bLoop,main_emu_loop;
volatile int bSleep;

char ziparchive_filename[256];
char shortzip_filename[64];


#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))

#define BUFFER_LEN 16384
extern u16 play_buffer[BUFFER_LEN];


#define OUTPUT_BUFFER_SIZE	1024*4 /* Must be an integer multiple of 4. */
#define INPUT_BUFFER_SIZE	(2*OUTPUT_BUFFER_SIZE)


#define MAXVOLUME	0x8000
int neogeo_mp3_volume = 100;
int snd_buffer_size=1024;
int mp3_buffer_size=OUTPUT_BUFFER_SIZE/4;
static int snd_handle,mp3_handle;
static s16 SoundBuffer[2][BUFFER_LEN];
volatile int SoundBuffer_flip=0;

extern char LastPath[MAX_PATH];



static unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
						OutputBuffer[2][OUTPUT_BUFFER_SIZE],
						*OutputPtr=OutputBuffer[0],
						*GuardPtr=NULL;
volatile int OutputBuffer_flip=0;
volatile static const unsigned char	*OutputBufferEnd=OutputBuffer[0]+OUTPUT_BUFFER_SIZE;
static struct mad_stream	Stream;
static struct mad_frame	Frame;
static struct mad_synth	Synth;
static mad_timer_t			Timer;

static int SoundThread(SceSize args, void *argp){
  debug_log( "Thrd strt:snd" );
  SoundBuffer_flip=0;
  do{
    streamupdate(SoundBuffer[SoundBuffer_flip],snd_buffer_size);
    sceAudioOutputPannedBlocking( snd_handle, MAXVOLUME, MAXVOLUME, (char*)SoundBuffer[SoundBuffer_flip] );
    SoundBuffer_flip^=1;

  }
  while (SndThread_Active);
  debug_log( "Thrd stop:snd" );
  return (0);
}

void InitSoundThread(){
	if (snd_thread!=-1) return;
	debug_log( "Create Thread : snd" );
	SndThread_Active=1;
	snd_thread = sceKernelCreateThread( "sound thread", SoundThread, 0x8, 0x40000, 0, NULL );
	if ( snd_thread < 0 ){
		ErrorMsg( "Thread failed : snd" );
		return;
	}

	sceKernelStartThread( snd_thread, 0, 0 );
	debug_log( "Thread ok : snd" );
}

void StopSoundThread(){
	SndThread_Active=0;
	if ( snd_thread !=-1 ){
		sceKernelWaitThreadEnd( snd_thread, NULL );
		sceKernelDeleteThread( snd_thread );
		snd_thread=-1;
	}
}


void psp_OpenAudio(){
	mp3_handle = sceAudioChReserve( -1, mp3_buffer_size, 0 );
	snd_handle = sceAudioChReserve( -1, snd_buffer_size, 0 );
}
void psp_CloseAudio(){
	sceAudioChRelease( snd_handle );
	sceAudioChRelease( mp3_handle );

	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);

}

int psp_ExitCheck(){
	return !bLoop;
}

int ExitCallback(){
	// Cleanup the games resources etc (if required)
	debug_log("Exit Callback");

	neogeo_shutdown();
	save_settings();

	bLoop=0;bSleep=0;main_emu_loop=0;

	debug_log("mp3");
	StopMP3Thread();
	debug_log("snd");
	StopSoundThread();
	debug_log("audio");
	psp_CloseAudio();

	scePowerSetClockFrequency(222,222,111);
	debug_log("exit");
	// Exit game
	sceKernelExitGame();
	return 0;
}

void set_cpu_clock(){
#ifdef PSP_MODE
		switch (psp_cpuclock){
			case 266:scePowerSetClockFrequency(266,266,133);break;
			case 333:scePowerSetClockFrequency(333,333,166);break;
			default :scePowerSetClockFrequency(222,222,111);
		}
#endif
}

void PowerCallback(int unknown, int pwrflags){
	static int old_snd_thread,old_mp3_thread;
	debug_log("Power Callback");

	if(pwrflags & POWER_CB_POWER){
		bSleep=1;
		old_mp3_thread=mp3_thread;
		StopMP3Thread();
		old_snd_thread=snd_thread;
		StopSoundThread();
		scePowerSetClockFrequency(222,222,111);

	}else if(pwrflags & POWER_CB_RESCOMP){
		bSleep=0;
		set_cpu_clock();
		if (old_mp3_thread!=-1)	InitMP3Thread();
		if (old_snd_thread!=-1)InitSoundThread();
	}
	int cbid = sceKernelCreateCallback("Power Callback", (void*)PowerCallback,NULL);
	scePowerRegisterCallback(0, cbid);
}

// Thread to create the callbacks and then begin polling
int CallbackThread(SceSize args, void *arg){
	int cbid;
	cbid = sceKernelCreateCallback( "Exit Callback", (void*)ExitCallback,NULL );
	sceKernelRegisterExitCallback( cbid );
	cbid = sceKernelCreateCallback( "Power Callback", (void*)PowerCallback,NULL );
	scePowerRegisterCallback( 0, cbid );
	sceKernelSleepThreadCB();
	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks()
{
	int thid = 0;

	thid = sceKernelCreateThread( "update_thread", CallbackThread, 0x11, 0xFA0, 0, 0 );
	if( thid >= 0 ){
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

#ifndef RELEASE
void debug_log( const char* message ){
	static int	sy = 1;

	pgPrint( 0, sy, 0xffff, message ,1);
	sy++;

	if ( sy >= CMAX_Y ){ /*pgWaitVn(180);*/pgFillvram(0);sy=0;}
		/*int 	x, y;
		u16*	dest;

		dest = (u16*)pgGetVramAddr( 0, 0 );

		for ( y = 0; y < SCREEN_HEIGHT; y++ ){
			for ( x = 0; x < (SCREEN_WIDTH - NEO_WIDTH); x++ ){
				*dest++ = 0;
			}
			dest += (512 - (SCREEN_WIDTH - NEO_WIDTH));
		}
		sy = 1;
	}*/
}
#endif // RELEASE

static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The signed short value is formed, after clipping, by the least
	 * significant whole part bit, followed by the 15 most significant
	 * fractional part bits. Warning: this is a quick and dirty way to
	 * compute the 16-bit number, madplay includes much better
	 * algorithms.
	 */

	/* Clipping */
	if(Fixed>=MAD_F_ONE)
		return(32767);
	if(Fixed<=-MAD_F_ONE)
		return(-32767);

	/* Conversion. */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}

static char *short_name(char *s) {
  char *d = strrchr(s,'/');
  if (d)
    return d;
  return s;
}

int MP3Thread(SceSize args, void *argp){
	FILE *f;
	int filesize=0;
	int					Status,i;
	unsigned long		FrameCount;
	debug_log( "Thread start! : mp3" );


	/* First the structures used by libmad must be initialized. */


	/* Decoding options can here be set in the options field of the
	 * Stream structure.
	 */

	/* {1} When decoding from a file we need to know when the end of
	 * the file is reached at the same time as the last bytes are read
	 * (see also the comment marked {3} bellow). Neither the standard
	 * C fread() function nor the POSIX read() system call provides
	 * this feature. We thus need to perform our reads through an
	 * interface having this feature, this is implemented here by the
	 * bstdfile.c module.
	 */
	f=NULL;
	while (MP3Thread_Active){
		// Is there a new file to open
		if (MP3_newfile){
			MP3_newfile=0;
			f=fopen(convert_path(MP3_file),"rb");
			if(f==NULL) {
					MP3_playing=0;
					print_ingame(180,"Can't open %s",short_name(MP3_file));
					debug_log(MP3_file);
					continue;

			}
			debug_log("mp3 file opened");
			print_ingame(180,"%s",short_name(MP3_file));

			fseek(f,0,SEEK_END);
			filesize=ftell(f);
			fseek(f,0,SEEK_SET);
			/*init mad*/
			mad_stream_init(&Stream);
			mad_frame_init(&Frame);
			mad_synth_init(&Synth);
			mad_timer_reset(&Timer);
			/*var*/
			FrameCount=0;
			Status=0;
			OutputBuffer_flip=0;
			OutputPtr=OutputBuffer[0];
			OutputBufferEnd=OutputBuffer[0]+OUTPUT_BUFFER_SIZE;
			MP3_playing=1;

		}
		/* This is the decoding loop. */
		while (MP3_playing&&MP3Thread_Active)	{
			// input
			if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)	{
				size_t			ReadSize,	Remaining;
				unsigned char	*ReadStart;
				if(Stream.next_frame!=NULL)	{
					Remaining=Stream.bufend-Stream.next_frame;
					memmove(InputBuffer,Stream.next_frame,Remaining);
					ReadStart=InputBuffer+Remaining;
					ReadSize=INPUT_BUFFER_SIZE-Remaining;
				}	else {
					ReadSize=INPUT_BUFFER_SIZE;
					ReadStart=InputBuffer;
					Remaining=0;
				}
				// Fill-in the buffer.
				ReadSize=fread(ReadStart,1,ReadSize,f);
				if (ReadSize >= 0) filesize-=ReadSize;
				if(filesize==0)	{
					if (cdda_autoloop){
						debug_log("loop");
						fseek(f,0,SEEK_END);
						filesize=ftell(f);
						fseek(f,0,SEEK_SET);
					}else{//debug_log("end of input");
						cdda_playing=0;
						MP3_playing=0;  //no break to decode last frame
						//break;
					}
				}
			/* {3} When decoding the last frame of a file, it must be
			 * followed by MAD_BUFFER_GUARD zero bytes if one wants to
			 * decode that last frame. When the end of file is
			 * detected we append that quantity of bytes at the end of
			 * the available data. Note that the buffer can't overflow
			 * as the guard size was allocated but not used the the
			 * buffer management code. (See also the comment marked
			 * {1}.)
			 *
			 * In a message to the mad-dev mailing list on May 29th,
			 * 2001, Rob Leslie explains the guard zone as follows:
			 *
			 *    "The reason for MAD_BUFFER_GUARD has to do with the
			 *    way decoding is performed. In Layer III, Huffman
			 *    decoding may inadvertently read a few bytes beyond
			 *    the end of the buffer in the case of certain invalid
			 *    input. This is not detected until after the fact. To
			 *    prevent this from causing problems, and also to
			 *    ensure the next frame's main_data_begin pointer is
			 *    always accessible, MAD requires MAD_BUFFER_GUARD
			 *    (currently 8) bytes to be present in the buffer past
			 *    the end of the current frame in order to decode the
			 *    frame."
			 */

				if(!filesize && ReadSize > 0)	{
					GuardPtr=ReadStart+ReadSize;
					memset(GuardPtr,0,MAD_BUFFER_GUARD);
					ReadSize+=MAD_BUFFER_GUARD;
				}
				// decode

				if (ReadSize + Remaining > 0)
				  mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);

				Stream.error=0;
			}

			if(mad_frame_decode(&Frame,&Stream)) {
				if(MAD_RECOVERABLE(Stream.error)){
					if(Stream.error!=MAD_ERROR_LOSTSYNC)	{
						debug_log("recov. error");
						print_ingame(600,mad_stream_errorstr(&Stream));
						cdda_playing = 0;
						MP3_playing = 0;
						// alert("error",mad_stream_errorstr(&Stream));
					}
					continue;
				}
				else if(Stream.error==MAD_ERROR_BUFLEN) {
				  if (FrameCount)
				    continue;
				  else
				    break;
				} else	{
								ErrorMsg("unrecov. error");
								Status=1;
								break;
					 	}
	  	}

		/* The characteristics of the stream's first frame is printed
		 * on stderr. The first frame is representative of the entire
		 * stream.
		 */
		/*if(FrameCount==0)
			if(PrintFrameInfo(stderr,&Frame.header))
			{
				Status=1;
				break;
			}*/

		/* Accounting. The computed frame duration is in the frame
		 * header structure. It is expressed as a fixed point number
		 * whole data type is mad_timer_t. It is different from the
		 * samples fixed point format and unlike it, it can't directly
		 * be added or subtracted. The timer module provides several
		 * functions to operate on such numbers. Be careful there, as
		 * some functions of libmad's timer module receive some of
		 * their mad_timer_t arguments by value!
		 */

			FrameCount++;
			mad_timer_add(&Timer,Frame.header.duration);

		/* Between the frame decoding and samples synthesis we can
		 * perform some operations on the audio data. We do this only
		 * if some processing was required. Detailed explanations are
		 * given in the ApplyFilter() function.
		 */
		//if(DoFilter) ApplyFilter(&Frame);

		/* Once decoded the frame is synthesized to PCM samples. No errors
		 * are reported by mad_synth_frame();
		 */

			mad_synth_frame(&Synth,&Frame);

		/* Synthesized samples must be converted from libmad's fixed
		 * point number to the consumer format. Here we use unsigned
		 * 16 bit big endian integers on two channels. Integer samples
		 * are temporarily stored in a buffer that is flushed when
		 * full.
		 */

			for(i=0;i<Synth.pcm.length;i++)	{
				signed short	Sample;
				/* Left channel */
				Sample=MadFixedToSshort(Synth.pcm.samples[0][i]);
				*(OutputPtr++)=Sample&0xff;
				*(OutputPtr++)=(Sample>>8);
				/* Right channel. If the decoded stream is monophonic then
			 	* the right output channel is the same as the left one.
			 	*/
				if(MAD_NCHANNELS(&Frame.header)==2)	Sample=MadFixedToSshort(Synth.pcm.samples[1][i]);
				*(OutputPtr++)=Sample&0xff;
				*(OutputPtr++)=(Sample>>8);

				/* Flush the output buffer if it is full. */
				if(OutputPtr==OutputBufferEnd){
					sceAudioOutputPannedBlocking(mp3_handle, MAXVOLUME*neogeo_mp3_volume/100, MAXVOLUME*neogeo_mp3_volume/100, (char*)OutputBuffer[OutputBuffer_flip] );
					OutputBuffer_flip^=1;
					OutputPtr=OutputBuffer[OutputBuffer_flip];
					OutputBufferEnd=OutputBuffer[OutputBuffer_flip]+OUTPUT_BUFFER_SIZE;
				}
			}
		}
	 if (f) {fclose(f);f=NULL;}
	 if (!MP3Thread_Active) break;
	 sceKernelSleepThread();
	}
	debug_log( "Thread stopped! : mp3" );

	return (0);
}

void MP3Pause(int pause){
	if (mp3_thread==-1) return;
	MP3_pause=pause;
	if (pause) {debug_log("pause");sceKernelSuspendThread(mp3_thread);}
	else {debug_log("resume");sceKernelResumeThread(mp3_thread);}
}

int PlayMP3(char *name){
		strcpy(MP3_file,name);

		MP3_newfile=1;
#ifdef PSP_MODE
		sceKernelWakeupThread(mp3_thread);
#endif
		MP3Pause(0);
		return 0;
}
void StopMP3(){
		MP3_playing=0;
		MP3Pause(0);
}

int InitMP3Thread(){
	if (mp3_thread!=-1) return -1;
	debug_log( "Create Thread : mp3" );
	MP3Thread_Active=1;
	MP3_pause=0;

	mp3_thread = sceKernelCreateThread( "mp3 thread", MP3Thread, 0x8, 0x40000, 0, NULL );
	if ( mp3_thread < 0 ){
		ErrorMsg( "Thread failed : mp3" );
		return -1;
	}
	sceKernelStartThread( mp3_thread, 0, 0 );
	debug_log( "Thread ok : mp3" );
	return 0;
}

void StopMP3Thread(){
	if ( mp3_thread !=-1 ){
		MP3Thread_Active=0;
		MP3_playing=0;
		if (MP3_pause) MP3Pause(0);
		sceKernelWakeupThread(mp3_thread);

		sceKernelWaitThreadEnd( mp3_thread, NULL );
		sceKernelDeleteThread( mp3_thread );
		mp3_thread=-1;
	}
}

void load_background(){

	sprintf(tmp_str,"%s%s",launchDir,"logo.bmp");
	FILE *fd = fopen(tmp_str,"rb");
	if (fd==NULL) {
		debug_log("can't load background image\n");
		bg_img=NULL;
		return;
	}
	bg_img = load_bmp(fd);
	fclose(fd);
}

void show_background(int fade){
	if (bg_img) image_put((SCREEN_WIDTH-bg_img->width)/2,(SCREEN_HEIGHT-bg_img->height)/2,bg_img,fade);
	else pgFillvram(0);
}


int main(int argc, char **argv){
	//malloc_psp_init();
	//pspDebugScreenInit();

	strncpy(launchDir,argv[0],sizeof(launchDir)-1);
	launchDir[sizeof(launchDir)-1]=0;
	char *str_ptr=strrchr(launchDir,'/');
	if (str_ptr){
		str_ptr++;
		*str_ptr=0;
	}



	psp_cpuclock=222;
	psp_vsync=0;

	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	//sceDisplaySetFrameBuf( (char*)VRAM_ADDR, 512, 1, 1 );
	pgScreenFrame(1,0);
	//debug_log( argv );
	SetupCallbacks();
	sceCtrlSetSamplingCycle( 0 );
	sceCtrlSetSamplingMode( 1 );

	set_cpu_clock();


	SndThread_Active=0;

	/* init audio */
	psp_OpenAudio();
	/* mad stuff */
	MP3_newfile=0;
	InitMP3Thread();
	// InitSoundThread();

	main_emu_loop=1;
	InitFiler("[NEOCDPSP " VERSION_STR "] - Choose a ZIP file",launchDir);
	load_background();
	show_background(0);

	pgFillBox(240-150+1,136-60+1,240+150-1,136+50-1,(16<<10)|(10<<5)|7);
	pgDrawFrame(240-150,136-60,240+150,136+50,(12<<10)|(8<<5)|5);
	pgPrintCenter(10,(15<<10)|(31<<5)|31,"NEOCDPSP " VERSION_STR " by YoyoFR",0);
	pgPrintCenter(12,(31<<10)|(31<<5)|31,"based on NEOCD 0.1,",0);
	pgPrintCenter(13,(31<<10)|(31<<5)|31,"NEO4ALL 0.2beta & NeoDC2.3",0);
	pgPrintCenter(15,(31<<10)|(31<<5)|31,"Powered by unofficial PSPSDK",0);
	pgPrintCenter(16,(31<<10)|(31<<5)|31,"http://www.ps2dev.org",0);
	pgPrintCenter(18,(31<<10)|(31<<5)|31,"Nice gfx by Pochii & SNK",0);
	pgPrintCenter(19,(31<<10)|(31<<5)|31,"http://pochistyle.pspwire.net/",0);

	pgPrintCenter(20,(31<<10)|(31<<5)|31,"--------",0);
	pgPrintCenter(21,(31<<10)|(26<<5)|30,"http://yoyofr.fr.st",0);



	while (get_pad());
	{int i;for (i=240;i&&!get_pad();i--){pgWaitV();}}
	while (get_pad()) pgWaitV();



	load_settings();

while (main_emu_loop){
		pgFillAllvram(0);
		pgScreenFrame(2,0);

		//strcpy(ziparchive_filename,"ms0:/PSP/GAME/NEOCDPSP/mslug");
		//strcpy(LastPath,"ms0:/PSP/GAME/NEOCDPSP/mslug/");
		main_emu_loop=getFilePath(ziparchive_filename);
		if (!main_emu_loop) ExitCallback();
		chdir(LastPath);


		pgFillAllvram(0);
		pgScreenFrame(1,0);


		set_cpu_clock();




		bLoop=1;

	/* configure input, output, and error functions */
/*	InitMP3Thread("ms0:/PSP/GAME/NEOCDPSP/mslug/neocd.mp3/test.mp3");
	pgwaitPress();
	StopMP3Thread();
	*/

	blit_init();
	neo_main();
	blit_deinit();

	if (main_emu_loop==2) {//a dir was choosen, so update so that after returning to browser we'll be at parent dir
			char *last_slash=strrchr(LastPath,'/');
			if (last_slash) {
				*last_slash=0;
				last_slash=strrchr(LastPath,'/');
				if (last_slash) last_slash[1]=0;
			}
		}

	}
	if (bg_img) image_free(bg_img);



	return 0;
}


void ErrorMsg(char *msg){
	pgFillAllvram(0);
	pgScreenFrame(1,0);
	pgPrintCenter(11,(15<<10)|(31<<5)|31,msg,1);
	pgwaitPress();
}
#define printf	pspDebugScreenPrintf

void print_center(char *msg) {
  // pgScreenFrame(0,0);
  pgFillBox(0,0,480,8,(0<<10)|(0<<5)|0);
  pgPrint(11,0,(15<<10)|(31<<5)|31,msg,0);
}
