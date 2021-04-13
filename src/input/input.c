/**************************************
****   INPUT.C  -  Input devices   ****
**************************************/

/*-- Include Files ---------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "../neocd.h"
#include "menu.h"
#include "psp/pg.h"
#ifdef SNAPSHOT_ACTIVE
#include "psp/imageio.h"
#endif
#include "ingame.h"

extern volatile int bLoop;
static int snapshot_cpt=0;
int select_lock;

/*--------------------------------------------------------------------------*/
#define P1UP    0x00000001
#define P1DOWN  0x00000002
#define P1LEFT  0x00000004
#define P1RIGHT 0x00000008
#define P1A     0x00000010
#define P1B     0x00000020
#define P1C     0x00000040
#define P1D     0x00000080

#define P2UP    0x00000100
#define P2DOWN  0x00000200
#define P2LEFT  0x00000400
#define P2RIGHT 0x00000800
#define P2A     0x00001000
#define P2B     0x00002000
#define P2C     0x00004000
#define P2D     0x00008000

#define P1START 0x00010000
#define P1SEL   0x00020000
#define P2START 0x00040000
#define P2SEL   0x00080000

#define SPECIAL 0x01000000


/*--------------------------------------------------------------------------*/
u32 keys   =~0;

extern int neogeo_autofireA,neogeo_autofireB,neogeo_autofireC,neogeo_autofireD;

/*--------------------------------------------------------------------------*/
void input_init(void) {
}


void input_shutdown(void) {
}

#if 0
INLINE void specialKey (SDLKey key) {
    switch(key) {
        case SDLK_F1: video_fullscreen_toggle(); break;
        case SDLK_F2: video_mode_toggle(); break;
        case SDLK_F3: incframeskip(); break;
        case SDLK_F4: sound_toggle(); break;
        case SDLK_F12: video_save_snapshot(); break;
        case SDLK_ESCAPE: exit(0); break;
        default:
            break;
    }
}
#endif

extern int swap_buf;

void processEvents(void) {
	static int cvtbl[][2] = {
	{PSP_CTRL_UP,	P1UP},
	{PSP_CTRL_DOWN,	P1DOWN},
	{PSP_CTRL_LEFT,	P1LEFT},
	{PSP_CTRL_RIGHT,P1RIGHT},
	{PSP_CTRL_CROSS,P1A},
	{PSP_CTRL_CIRCLE,P1B},
	{PSP_CTRL_SQUARE,P1C},
	{PSP_CTRL_TRIANGLE,P1D},
	{PSP_CTRL_START,P1START},
	// {PSP_CTRL_SELECT,P1SEL},
//	{PSP_CTRL_RTRIGGER,P1COIN},
	};
	int i;
	static int buttons = 0;
	static int cpt;
	int newkey = 0;

	cpt++;
	if (cpt&1) return;


	buttons = get_pad();//g_getpad();
	for(i=0;i<sizeof(cvtbl)/sizeof(cvtbl[0]);i++) {
		if (buttons & cvtbl[i][0]) newkey |= cvtbl[i][1];
	}
	if ((buttons & PSP_CTRL_SELECT) &&
	    (!select_lock || (buttons & PSP_CTRL_CROSS)))
	  newkey |= P1SEL;
	if ((buttons & PSP_CTRL_LTRIGGER) &&
	    (!select_lock || (buttons & PSP_CTRL_SELECT))) {
	  showmenu();
	}
#ifdef SNAPSHOT_ACTIVE
	if ((buttons & PSP_CTRL_RTRIGGER) &&
	    (!select_lock || (buttons & PSP_CTRL_SELECT))) {
	  char str[256];
	  sprintf(str,"%ssnapshot%02d.bmp",launchDir,snapshot_cpt++);
	  MP3Pause(1);
	  if (save_bmp(str,480,272,16,pgGetVramAddr(0,swap_buf*272),512) < 0)
	    print_ingame(60,"Can't save snapshot %d",snapshot_cpt);
	  else
	    print_ingame(60,"Snapshot %d saved",snapshot_cpt);
	  MP3Pause(0);
	}
#endif

	if (neogeo_autofireA&&(newkey&P1A)){
		switch (neogeo_autofireA){
			case 1:if (((cpt/2)&15)>=7) newkey&=~P1A;break;
			case 2:if (((cpt/2)&7)>=3) newkey&=~P1A;break;
			case 3:if (((cpt/2)&3)>=1) newkey&=~P1A;break;
		}
	}
	if (neogeo_autofireB&&(newkey&P1B)){
		switch (neogeo_autofireB){
			case 1:if (((cpt/2)&15)>=7) newkey&=~P1B;break;
			case 2:if (((cpt/2)&7)>=3) newkey&=~P1B;break;
			case 3:if (((cpt/2)&3)>=1) newkey&=~P1B;break;
		}
	}
	if (neogeo_autofireC&&(newkey&P1C)){
		switch (neogeo_autofireC){
			case 1:if (((cpt/2)&15)>=7) newkey&=~P1C;break;
			case 2:if (((cpt/2)&7)>=3) newkey&=~P1C;break;
			case 3:if (((cpt/2)&3)>=1) newkey&=~P1C;break;
		}
	}
	if (neogeo_autofireD&&(newkey&P1D)){
		switch (neogeo_autofireD){
			case 1:if (((cpt/2)&15)>=7) newkey&=~P1D;break;
			case 2:if (((cpt/2)&7)>=3) newkey&=~P1D;break;
			case 3:if (((cpt/2)&3)>=1) newkey&=~P1D;break;
		}
	}

	keys = ~newkey;
}

/*--------------------------------------------------------------------------*/
unsigned char read_player1(void) {
    return keys&0xff;
}

/*--------------------------------------------------------------------------*/
unsigned char read_player2(void) {
    return (keys>>8)&0xff;
}

/*--------------------------------------------------------------------------*/
unsigned char read_pl12_startsel(void) {
    return (keys>>16)&0x0f;
}


