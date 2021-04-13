/***************************************************************************

  streams.c

  Handle general purpose audio streams

***************************************************************************/
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>

#include "../neocd.h"

#define MIXER_MAX_CHANNELS 15
#define MIXER_OUTPUT_SHIFT 0
#define BUFFER_LEN 16384

#define CPU_FPS 60

extern int frame;

extern volatile int neogeo_soundmute;

int SamplePan[] = {//{ 0, 255, 128, 0, 255 };
	0,255,128,0,255,
	0,255,128,0,255,
	0,255,128,0,255
};


static int stream_joined_channels[MIXER_MAX_CHANNELS];

static int stream_vol[MIXER_MAX_CHANNELS];

struct {
    s16 *buffer;
    int param;
    void (*callback) (int param, s16 ** buffer, int length);
} stream[MIXER_MAX_CHANNELS];

void mixer_set_volume(int channel,int volume) {
    stream_vol[channel]=volume;
}

int streams_sh_start(void)
{
    int i;

    for (i = 0; i < MIXER_MAX_CHANNELS; i++) {
	stream_joined_channels[i] = 1;
	stream[i].buffer = 0;
    }
    return 0;
}

static int channel;

void streams_sh_stop(void)
{
    int i;

    for (i = 0; i < MIXER_MAX_CHANNELS; i++) {
	if (stream[i].buffer)
		free(stream[i].buffer);
	stream[i].buffer = 0;
    }
    channel = 0;
}

#define RANGE_CHECK(a) (a>32767?32767:(a<-32768?-32768:a))

void streamupdate(s16 *mix_buffer,int len) {
	/*static int current_pos;*/
  int channel, i,j;
  int there_is_data;
  u16 *bl, *br;
  u16 *pl;

  /* There is a hack here. The audio is actually generated at twice the
     SAMPLE_RATE, probably because lame outputs at 44 Khz.
     It would be nice to check this is the actual reason.
     Anyway, on pc, one would force SAMPLE_RATE*2 for the 2610 and the ay8910
     The problem is that we loose almost 10fps if we do so !
     So the hack is to generate half the samples, and double them.
     It's faster, and you don't really hear the difference */

  memset(mix_buffer,0,len*4);
  there_is_data=0;
  for (channel = 0; channel < MIXER_MAX_CHANNELS;channel += stream_joined_channels[channel]) {
    if (stream[channel].buffer) {
      there_is_data=1;
      if (stream_joined_channels[channel] > 1) {
	s16 *buf[MIXER_MAX_CHANNELS];
	// on update le YM2610
	if (len > 0) {
	  for (i = 0; i < stream_joined_channels[channel]; i++)
	    buf[i] = stream[channel + i].buffer;	// + stream[channel+i].buffer_pos;
	  (*stream[channel].callback) (stream[channel].param,buf, len/2);
	}
      }
    }
  }

  if (there_is_data&&!neogeo_soundmute) {
    /* init output buffers */

    for (channel = 0; channel < MIXER_MAX_CHANNELS;channel += stream_joined_channels[channel]) {
      if (stream[channel].buffer) {
	    	for (i = 0; i < stream_joined_channels[channel]; i++) {
		  int pan = SamplePan[channel + i];
		  if (pan == 0){ // full left
		    int x;s16 *dst;s16 *src;
		    src=(s16 *) stream[channel + i].buffer;
		    dst=mix_buffer;
		    for (x=len/2;x;x--) {
		      s32 l32 = *dst+(*src++);
		      *dst = dst[2] = RANGE_CHECK(l32);
		      dst += 4;
		    }
		  } else if (pan == 128) { // center
		    int x;s16 *dst;s16 *src;
		    src=(s16 *) stream[channel + i].buffer;
		    dst=mix_buffer;
		    for (x=len/2;x;x--) {
		      s32 l32 = *dst+(*src);
		      *dst = dst[2] = RANGE_CHECK(l32);
		      dst++;
		      l32 = *dst+(*src++);
		      *dst = dst[2] = RANGE_CHECK(l32);
		      dst+=3;
		    }
		  } else { // full right
		    int x;s16 *dst;s16 *src;
		    src=(s16 *) stream[channel + i].buffer;
		    dst=mix_buffer+1;
		    for (x=len/2;x;x--) {
		      s32 l32 = *dst+(*src++);
		      *dst = dst[2] = RANGE_CHECK(l32);
		      dst += 4;
		    }
		  }
	    	}
      }
    }
    //SDL_LockAudio();
  }
}

int stream_init_multi(int channels, int param,
		      void (*callback) (int param, s16 ** buffer,
					int length))
{
    int i;

    stream_joined_channels[channel] = channels;

    for (i = 0; i < channels; i++) {

	if ((stream[channel + i].buffer =
	     malloc(sizeof(s16) * BUFFER_LEN)) == 0)
	    return -1;
    }

    stream[channel].param = param;
    stream[channel].callback = callback;
    channel += channels;
    return channel - channels;
}
