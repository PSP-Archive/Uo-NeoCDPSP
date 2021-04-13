/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "../neocd.h"
#include "streams.h"

static unsigned char sound_muted=0x3;

#include "sound.h"

#define MIXER_MAX_CHANNELS 16
#define MUSIC_VOLUME 96
#define BUFFER_LEN 16384


#define NB_SAMPLES 512 /* better resolution */

#ifdef MENU_MUSIC
#include <SDL_mixer.h>
enum{
	SAMPLE_LOADING,
	SAMPLE_SAVING,
	SAMPLE_MENU,
	SAMPLE_BYE,
	SAMPLE_BEEP,
	SAMPLE_ERROR,
	NUM_SAMPLES
};

static char *sample_filename[NUM_SAMPLES]={
	DATA_PREFIX "loading.wav",
	DATA_PREFIX "saving.wav",
	DATA_PREFIX "menu.wav",
	DATA_PREFIX "goodbye.wav",
	DATA_PREFIX "beep.wav",
	DATA_PREFIX "error.wav"
};

static Mix_Chunk *sample[NUM_SAMPLES];

#define play_sample(NSAMPLE) Mix_PlayChannel(0,sample[(NSAMPLE)],0)

#endif


int init_sdl_audio(void)
{
#ifdef Z80_EMULATED
/*#ifdef MENU_MUSIC
    unsigned i;
    Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16, 2, NB_SAMPLES);
    for(i=0;i<NUM_SAMPLES;i++)
	    sample[i]=Mix_LoadWAV(sample_filename[i]);
#else
    SDL_AudioSpec desired;

    desired.freq = SAMPLE_RATE;
    desired.samples = NB_SAMPLES;

#ifdef WORDS_BIGENDIAN
    desired.format = AUDIO_S16MSB;
#else
    desired.format = AUDIO_S16;
#endif
    desired.channels = 2;
    desired.callback = update_sdl_stream;
    desired.userdata = NULL;
    SDL_OpenAudio(&desired, NULL);
#endif
    SDL_PauseAudio(1);
*/
    sound_reset();
#endif
    return 1;
}


void sound_play_menu_music(void)
{
#ifdef MENU_MUSIC
    Mix_PlayMusic(Mix_LoadMUS(DATA_PREFIX "music.mod"),-1);
    sound_mute();
#endif
}

void sound_play_loading(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_LOADING);
#endif
}

void sound_play_saving(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_SAVING);
#endif
}

void sound_play_menu(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_MENU);
#endif
}

void sound_play_bye(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_BYE);
#endif
}

void sound_play_beep(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_BEEP);
#endif
}

void sound_play_error(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_ERROR);
#endif
}

static int old_sound,sound_saved = 0;

void sound_enable(void) {
#ifdef Z80_EMULATED
//puts("sound_enable");
	if ((sound_muted&0x1)&&(!(sound_muted&0x2)))
	{
#ifdef MENU_MUSIC
		SDL_PauseAudio(1);
		Mix_HookMusic(&update_sdl_stream,NULL);
#endif
		//SDL_PauseAudio(0);
	}
	if (sound_saved) {
	  sound_saved = 0;
	  neogeo_soundmute = old_sound;
	}
	sound_muted&=0xFE;
#endif
}

void sound_disable(void) {
#ifdef Z80_EMULATED
//puts("sound_disable");
	if (!(sound_muted&0x3))
	{
#ifdef MENU_MUSIC
		SDL_PauseAudio(1);
		Mix_HookMusic(NULL,NULL);
		SDL_PauseAudio(0);
		Mix_VolumeMusic(MUSIC_VOLUME);
#else
		//SDL_PauseAudio(1);
#endif
	}
	sound_muted|=0x1;
	if (!sound_saved) {
	  old_sound = neogeo_soundmute;
	  neogeo_soundmute = 1;
	  sound_saved = 1;
	}
#endif
}

void sound_toggle(void) {
#ifdef Z80_EMULATED
//puts("sound_toggle");
	if (!(sound_muted&0x2))
	{
		if (sound_muted&0x1)
			sound_enable();
		else
			sound_disable();
	}
#endif
}

void sound_shutdown(void) {
#ifdef Z80_EMULATED
//puts("sound_shutdown");
    sound_stop();
//    SDL_CloseAudio();
#endif
}

void sound_stop(void) {
#ifdef Z80_EMULATED
//puts("sound_stop");
    sound_mute();
    streams_sh_stop();
    YM2610_sh_stop();
    //SDL_Delay(100);
#endif
}

void sound_reset(void) {
#ifdef Z80_EMULATED
//puts("sound_reset");
    streams_sh_start();
    YM2610_sh_start();
    sound_unmute();
#endif
}

void sound_mute(void){
#ifdef Z80_EMULATED
//puts("sound_mute");
	if (!(sound_muted&0x2))
	{
#ifdef MENU_MUSIC
		SDL_PauseAudio(1);
		Mix_HookMusic(NULL,NULL);
		SDL_PauseAudio(0);
		Mix_VolumeMusic(MUSIC_VOLUME);
#else
		//SDL_PauseAudio(1);
#endif
	}
	sound_muted=0x3;
	if (!sound_saved) {
	  old_sound = neogeo_soundmute;
	  neogeo_soundmute = 1;
	  sound_saved = 1;
	}
#endif
}

void sound_unmute(void){
#ifdef Z80_EMULATED
//puts("sound_unmute");
	if (sound_muted&0x2)
	{
#ifdef MENU_MUSIC
		SDL_PauseAudio(1);
		Mix_HookMusic(&update_sdl_stream,NULL);
#endif
		//SDL_PauseAudio(0);
	}
	sound_muted=0;
	if (sound_saved) {
	  sound_saved = 0;
	  neogeo_soundmute = old_sound;
	}
#endif
}
