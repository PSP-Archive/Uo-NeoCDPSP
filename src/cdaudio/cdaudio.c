/**************************************
****   CDAUDIO.C  -  CD-DA Player  ****
**************************************/

//-- Include files -----------------------------------------------------------
//#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../neocd.h"
#include "cdaudio.h"
#include "../psp/filer.h"

extern int neogeo_mp3_enable;
extern int PlayMP3(char *name);
extern void StopMP3();
extern char LastPath[MAX_PATH];




//-- Private Variables -------------------------------------------------------
//static int			cdda_min_track;
//static int			cdda_max_track;
//static int			cdda_disk_length;
//static int			cdda_track_end;
//static int			cdda_loop_counter;
//static SDL_CD			*cdrom;

//-- Public Variables --------------------------------------------------------
int			cdda_first_drive=0;
int			cdda_current_drive=0;
int			cdda_current_track=0;
int			cdda_current_frame=0;
volatile int			cdda_playing=0;
volatile int		cdda_autoloop=0;
int			cdda_volume=0;
int			cdda_disabled=0;

//-- Function Prototypes -----------------------------------------------------
int			cdda_init(void);
int			cdda_play(int);
void			cdda_stop(void);
void			cdda_resume(void);
void			cdda_shutdown(void);
void			cdda_loop_check(void);
int 			cdda_get_disk_info(void);




//----------------------------------------------------------------------------
int	cdda_init(void)
{

	//cdda_min_track = cdda_max_track = 0;
	cdda_current_track = 0;
	cdda_playing = 0;
	//cdda_loop_counter = 0;

	/* Open the default drive */
/*	cdrom=SDL_CDOpen(cdda_current_drive);

	// Did if open? Check if cdrom is NULL
	if(cdrom == NULL){
		printf("Couldn't open drive %s for audio.  %s\n", SDL_CDName(cdda_current_drive), SDL_GetError());
		cdda_disabled=1;
		return 1;
	} else {
		cdda_disabled=0;
		printf("CD Audio OK!\n");
	}*/
	cdda_disabled=0;
	//debug_log("CD Audio OK!\n");

	cdda_get_disk_info();
	return	1;
}

//----------------------------------------------------------------------------
int cdda_get_disk_info(void)
{
    if(cdda_disabled) return 1;

    //if( CD_INDRIVE(SDL_CDStatus(cdrom)) ) {
        //cdda_min_track = 0;
        //cdda_max_track = 40;//cdrom->numtracks;
        //cdda_disk_length = 40;//cdrom->numtracks;
/*        return 1;
    }
    else
    {
        printf("Error: No Disc in drive\n");
        cdda_disabled=1;
        return 1;
    }*/
    return 1;
}


//----------------------------------------------------------------------------
int cdda_play(int track)
{
	char str[256];char str2[256];char *str3;;


	if (!neogeo_mp3_enable) return 1;

    // if(cdda_disabled) {debug_log("disabled");return 1;}

    //if (track==2) return 1;


    sprintf(str,"pl.trk.%d",track);
    debug_log(str);




    if(cdda_playing && cdda_current_track==track) {
    	if (cdda_playing==2) {//was in pause, so resume & let stop/play again
    		MP3Pause(0);
				cdda_playing = 1;
    	}else return 1;
    }

    StopMP3();


    	cdda_current_track = track;
    	//cdda_loop_counter=0;
    	//cdda_track_end=0;//(cdrom->track[track-1].length*60)/CD_FPS;//Length in 1/60s of second
    	cdda_playing = 1;

    	strcpy(str,LastPath);
    	strcat(str,"neocd.mp3/");

    	strcpy(str2,"xx.mp3");
    	str2[0]=(((track)/10)%10)+48;
    	str2[1]=((track)%10)+48;

    	str3=find_file(str2,str);
	if (!str3) { // try in the psp/music directory...
	  strcpy(str,LastPath);
	  str[strlen(str)-1] = 0; // remove the trailing /
	  char *s = strrchr(str,'/');
	  if (s) { // this is the game directory then
	    strcpy(str2,s+1);
	    strcpy(&str[5],"PSP/MUSIC/");
	    strcat(str,str2);
	    strcat(str,"/");

	    strcpy(str2,"xx.mp3");
	    str2[0]=(((track)/10)%10)+48;
	    str2[1]=((track)%10)+48;

	    str3=find_file(str2,str);
	  }
	}

    	if (str3) {
    		debug_log("cdda play - mp3 found");
    		strcat(str,str3);
    		cdda_disabled = PlayMP3(str);
    	}
    	else {
	  print_ingame(180,"didn't find mp3 track %d",track);
	  cdda_disabled = 1;
    	}

	/*	{
		SceCtrlData pad;
		do {
			sceCtrlReadBufferPositive(&pad, 1);
		} while ((pad.Buttons & PSP_CTRL_TRIANGLE));
		} */
    	return 1;
/*    }
    else
    {
        cdda_disabled = 1;
        return 1;
    }*/
}

//----------------------------------------------------------------------------
void	cdda_pause(void)
{
	if(cdda_disabled) return;
	//SDL_CDPause(cdrom);
	//debug_log("pause");
	MP3Pause(1);
	cdda_playing = 2;
}


void	cdda_stop(void)
{
	if(cdda_disabled) return;
	//SDL_CDStop(cdrom);
	debug_log("cdda stop");
	StopMP3();
	cdda_playing = 0;
}

//----------------------------------------------------------------------------
void	cdda_resume(void)
{
	if(cdda_disabled || (cdda_playing==1)) return;
	//SDL_CDResume(cdrom);
	debug_log("resume");
	MP3Pause(0);
	cdda_playing = 1;
}

//----------------------------------------------------------------------------
void	cdda_shutdown(void)
{
	if(cdda_disabled) return;
	/*SDL_CDStop(cdrom);
	SDL_CDClose(cdrom);*/
	//debug_log("cdda shutdown");
	StopMP3();
}

//----------------------------------------------------------------------------
void	cdda_loop_check(void)
{
	/*if(cdda_disabled) return;
	if (cdda_playing==1) {
		cdda_loop_counter++;
		if (cdda_loop_counter>=cdda_track_end) {
			if (cdda_autoloop)
				cdda_play(cdda_current_track);
			else
				cdda_stop();
		}
	}*/
}

