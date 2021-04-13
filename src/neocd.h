/**
 * NeoCD/SDL main header file
 **
 * 2003 Fosters
 */

#ifndef NEOCD_H
#define NEOCD_H

#include "psp.h"

#define REFRESHTIME 1000/60
#define FPS 60


//#define NEO4ALL_Z80_UNDER_CYCLES	 66666
#define NEO4ALL_Z80_UNDER_CYCLES	 33333
//#define NEO4ALL_Z80_UNDER_CYCLES	 22222
#define NEO4ALL_68K_UNDER_CYCLES	166666

//#define NEO4ALL_Z80_NORMAL_CYCLES	100000
#define NEO4ALL_Z80_NORMAL_CYCLES	 66666
//#define NEO4ALL_Z80_NORMAL_CYCLES	 44444
#define NEO4ALL_68K_NORMAL_CYCLES	200000

//#define NEO4ALL_Z80_OVER_CYCLES	133333
#define NEO4ALL_Z80_OVER_CYCLES	 	 99999
//#define NEO4ALL_Z80_OVER_CYCLES	 88888
#define NEO4ALL_68K_OVER_CYCLES		233333

#define SAMPLE_RATE    22050

#define REGION_JAPAN  0
#define REGION_USA    1
#define REGION_EUROPE 2

#define REGION REGION_JAPAN //REGION_USA

//#include <SDL.h>

#include "cdaudio/cdaudio.h"
#include "cdrom/cdrom.h"
#include "68k.h"
#include "memory/memory.h"
#include "video/video.h"
#include "input/input.h"
#include "z80/z80intrf.h"
#include "sound/sound.h"
#include "sound/streams.h"
// #include "sound/2610intf.h"
#include "sound/timer.h"
#include "pd4990a.h"

/*-- Version, date & time to display on startup ----------------------------*/

#define VERSION1 "NEOCDPSP based on NEO4ALL 0.2beta"
#define VERSION2 "Compiled on: "__DATE__" "__TIME__

/*-- functions -------------------------------------------------------------*/


/*-- globals ---------------------------------------------------------------*/
extern char	global_error[80];

extern char	*neogeo_rom_memory;
extern char	*neogeo_prg_memory;
extern char	*neogeo_fix_memory;
extern char	*neogeo_spr_memory;
extern char	*neogeo_pcm_memory;
extern int neogeo_pcm_size;

extern unsigned char neogeo_memorycard[];
void memcard_init(void);
void memcard_shutdown(void);
void memcard_update(void);

extern int      neogeo_ipl_done;
extern int	neogeo_sound_enable;
extern int	neogeo_frameskip;

extern u32 neo4all_z80_cycles;
extern u32 neo4all_68k_cycles;
extern unsigned z80_cycles_inited;

int	neogeo_hreset(void);

void neogeo_adjust_frameskip(int new_frameskip);
void neogeo_adjust_cycles(int new_68k, int new_z80);
void print_frameskip(void);
void show_icon(void);
int	neo_main();

extern int neogeo_region,neogeo_hardrender;
extern int neogeo_showfps;
extern char buf_fps[12];

extern u32 neocd_time;
extern int neo4all_skip_next_frame;
void alert(char *titre, char *msg);
void init_autoframeskip(void);
void neogeo_setregion();
void	neogeo_do_cdda( int command, int trck_number_bcd);
void	neogeo_shutdown(void);

#endif /* NEOCD_H */
