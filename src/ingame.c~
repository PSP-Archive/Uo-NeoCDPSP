#include <stdarg.h>
#include "neocd.h"
#include "video/video.h"

/**[Message List Stuff]*************************************/

#define MSG_LIST_SIZE	10	// maximum messages onscreen

typedef struct MESSAGE
{
   s32  messagetime;		// time before message expires
   char message[256];		// message string
} MESSAGE;

static struct MESSAGE MsgList[MSG_LIST_SIZE];

static int mbase;		// Which message is top of the list

typedef void text_func( u16 x, u16 y, const char *STR);
static text_func *textout_fast;

// print_ingame():
// Add Message to Ingame Message List, using a printf() style format string.

void print_ingame(int showtime, const char *format, ...)
{
   va_list ap;
   va_start(ap,format);
   vsprintf(MsgList[mbase].message,format,ap);
   va_end(ap);

   MsgList[mbase].messagetime = showtime;

   mbase++;
   if(mbase>=MSG_LIST_SIZE) mbase=0;
}

// clear_ingame_message_list():
// Clear the ingame message list

void clear_ingame_message_list(void)
{
   int ta;

   mbase = 0;
   for(ta = 0; ta < MSG_LIST_SIZE; ta ++){
      MsgList[ta].messagetime = 0;
      sprintf(MsgList[ta].message," ");
   }
}

#ifndef round
#define     round(a)    (int)(((a)<0.0)?(a)-.5:(a)+.5)
#endif

void overlay_ingame_interface(void)
{
  int ta,tb;

   /*

   print ingame message list

   */

   if (neogeo_hardrender)
     textout_fast = &fixputsPSP;
   else
     textout_fast = &fixputs;

   for(tb=0;tb<MSG_LIST_SIZE;tb++){

      ta=tb+mbase;

      if(ta>=MSG_LIST_SIZE)
	 ta-=MSG_LIST_SIZE;

      if(MsgList[ta].messagetime>0){

	 MsgList[ta].messagetime --;

	 textout_fast(1,27-((MSG_LIST_SIZE-1)-tb),MsgList[ta].message);
      }

   }

   /*

   print speed profile (fps)

   */

   switch(neogeo_showfps){
   case 0x00:				// Show nothing
   break;
   case 0x01:				// Show Average FPS (takes a while to adapt to changes)
     textout_fast( 39-strlen(buf_fps), 0, buf_fps);
     break;
   case 0x02:				// Show Profile results (percent)
/*       for(ta=0;ta<PRO_COUNT;ta++){ */
/*       sprintf(fps_buff,"%s: %2d%%",profile_results[ta].name, profile_results[ta].percent); */
/*       textout_fast(fps_buff,xoff2+xxx-(10*6),yoff2+(ta*8),get_white_pen()); */
     break;
   }

}
