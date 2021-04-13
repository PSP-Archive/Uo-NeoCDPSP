#ifndef __PSP_H__
#define __PSP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*#define malloc malloc_psp
#define free free_psp
#define calloc calloc_psp
#define realloc realloc_psp
*/

#define RELEASE
#define SNAPSHOT_ACTIVE

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <string.h>
#include <math.h>

#include "pg.h"
#include "psplib.h"
//#include "malloc.h"

//#define memmove memmove_psp

void ErrorMsg(char *msg);

extern char launchDir[256];
extern char tmp_str[256];
extern volatile int neogeo_soundmute;

#define VRAM_ADDR	(0x04000000)

#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

#define POWER_CB_POWER		0x80000000 
#define POWER_CB_HOLDON		0x40000000 
#define POWER_CB_STANDBY	0x00080000 
#define POWER_CB_RESCOMP	0x00040000 
#define POWER_CB_RESUME		0x00020000 
#define POWER_CB_SUSPEND	0x00010000 
#define POWER_CB_EXT		0x00001000 
#define POWER_CB_BATLOW		0x00000100 
#define POWER_CB_BATTERY	0x00000080 
#define POWER_CB_BATTPOWER	0x0000007F 

#define MAX_PATH 256

  // These version major/minor are used to check the version of the ini file
  // if the version read in the ini file is > than the version in psp.h then
  // the read aborts. It's a very unlikely condition, but anyway...

#define VERSION_MAJOR 0
#define VERSION_MINOR 7

/* Changes since version 0.6.2 :
 *
 * This changelog is mainly to remember the changes for the release.
 * See the web page for older changes.
 *
 * - Fixed crash in breakers : this is once again related to the upload area,
 *   so I can just hope it won't break other games
 * - Try harder to recognise game files from ipl.txt : some originals have
 *   weird filenames in the ipl.txt (a double dragon version is in this case).
 *   Now if I can't find the file type in the extension, I look for it in
 *   the whole filename, which seems to work for this double dragon version,
 *   at least.
 * - Fixed sound effects : they are now much clearer, and in real stereo.
 *   The difference is especially noticeable in bust a move.
 * - Fixed a last possible corruption of the memcard.bin file if quiting with
 *   the home key. Now it should be safe to use the home key to quit at any
 *   time, and there should be no way left to corrupt any file (mp3 or other).
 *   If you use fw 2.0, be sure to use the Exit command from the gui though,
 *   or do otherwise at your own risk (I am not testing neocdpsp with fw 2.0)
 * - The sound effects do not stop anymore after 3 or 4 games (finally !)
 * - Added some game messages at the bottom of the screen (now you know
 *   when an mp3 track is starting to play, if it didn't find it, if there
 *   was an error decoding it, and so on...)
 * - In previous version, if an mp3 had some id3 tags it couldn't be played
 *   in neocdpsp. This is fixed now, sorry for those who encountered this
 *   problem.
 * - Now you can also place your mp3s in PSP/MUSIC. Example :
 *   say you have a game in a directory mslug. The you can put its mp3s
 *   as previously in mslug/neocd.mp3/ but now you can also put them in
 *   /PSP/MUSIC/mslug/ if you prefer. After all these are mp3s, so you should
 *   be able to play them from the psp interface if you want to !
 *   The directory in music where you put the mp3 must be named exactly like
 *   the directory of the game for neocdpsp.
 * - The mp3 decoder is a little more resistant to crashes, that is if you have
 *   an mp3 file at 0 or 1 byte after you have filled your memory card, it
 *   won't crash anymore. I don't say it will not every crash again though,
 *   there are probably files which can make it crash !
 * - Fixed sound and music in top players golf
 * - Fixed Art of Fighting 3 freezing at start of gameplay
 * - The loading of zipped games is even faster than in 0.6.2 !
 * - The ingame sound effects of samurai spirits rpg work now, the game is
 *   still very long to load, but it was already long on the original hardware
 *   and is now limited by the access speed of the memory card.
 * - Now the memory card is writen only when you quit the game, not everytime
 *   the game tries to write to it.
 * - Added "Select lock" option to the gui. When enabled the controls become:
 *   Select + LTrigger instead of LTrigger for the gui
 *   Select + RTrigger instead of RTrigger for snapshots
 *   and Select + Cross instead of Select alone (pause in most games).
 *   This for those who press the triggers by accident !
 *
 * 0.7.1 : fix the stupid bug in neogeo_upload
 */

#define VERSION_STR "0.7.1"

int ExitCallback();
void MP3Pause(int pause);
#ifndef RELEASE
void debug_log( const char* message );
#else
#define debug_log(message)
#endif
void StopSoundThread();
void InitSoundThread();
int PlayMP3(char *name);
void StopMP3();
int InitMP3Thread();
void StopMP3Thread();
void show_background(int fade);
int psp_ExitCheck();
void set_cpu_clock();
void print_center(char *msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PSP_H__ */
