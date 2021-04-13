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

int sound=1;

#include "sound.h"

#define MIXER_MAX_CHANNELS 16

//#define CPU_FPS 60
#define BUFFER_LEN 16384
extern u16 play_buffer[BUFFER_LEN];

//#define NB_SAMPLES 512 /* better resolution */
#define NB_SAMPLES 1024

void update_sdl_stream(void *userdata, u8 * stream, int len)
{
    streamupdate(len);
    memcpy(stream, (u8 *) play_buffer, len);
}

int init_sdl_audio(void)
{
    
    //init rest of audio
    streams_sh_start();
	YM2610_sh_start();
//	SDL_PauseAudio(0);


    
    return 1;
}

void sound_toggle(void) {
//	SDL_PauseAudio(sound);
	sound^=1;
}

void sound_shutdown(void) {
    
    streams_sh_stop();
    YM2610_sh_stop();
    
}



