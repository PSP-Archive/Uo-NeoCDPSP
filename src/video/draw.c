/*******************************************
**** VIDEO.C - Video Hardware Emulation ****
*******************************************/

//-- Include Files -----------------------------------------------------------
//#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "../neocd.h"
#include "video.h"
#include "psp/blitter.h"

#ifdef USE_VIDEO_GL
#include "videogl.h"
#endif

//#include "console.h"
extern int neogeo_hardrender;
extern void guDrawBuffer();

unsigned char	video_shrinky[17];

unsigned eficiencia_media=50;

#ifdef DREAMCAST
#include<kos.h>
#else
#include<string.h>
#define sq_set16(aaa,bbb,ccc)	{ \
				int sqinc; \
				short *sqsrc=(short *)aaa; \
				short sqset=bbb; \
				for(sqinc=0;sqinc<ccc;sqinc++) \
					sqsrc[sqinc]=sqset; \
				}
#endif


static unsigned int   fc=0;

int neogeo_blitter=0;


#ifdef PROFILER
extern Uint32 profiler_media3;
extern Uint32 profiler_media4;
#endif

void blit(void)
{
	if (neogeo_hardrender) return;

	switch (neogeo_blitter){
		case 0:{
			guDrawBuffer(video_buffer+8,304,224,512,304,224);
			break;
		}
		case 1:{
			guDrawBuffer(video_buffer+8,304,224,512,370,272);

			break;
		}
		case 2:{
			guDrawBuffer(video_buffer+8,304,224,512,400,272);
			break;
		}
		case 3:{
			guDrawBuffer(video_buffer+8,304,224,512,480,272);
			break;
		}
	}
}


void video_draw_blank(void)
{
	blit();
}

static void memset16(void *dst0,unsigned data,unsigned len)
{
	int* dst = (int*)dst0;
	data |= data<<16;
	int i;
	for(i=0;i<len/8;i++) {
		dst[0] = data;
		dst[1] = data;
		dst[2] = data;
		dst[3] = data;
		dst+=4;
	}
}


static __inline__ void video_draw_screen_first(void)
{
  int    a;

  for (a=0; a<224; a++) memset16(video_line_ptr[a],video_paletteram_pc[4095],320*2);

  	//sq_set16(video_line_ptr[a], video_paletteram_pc[4095], 320*2);

}

//----------------------------------------------------------------------------
void video_draw_screen1()
{
   int         sx =0,sy =0,oy =0,my =0,zx = 1, rzy = 1;
   int         offs,i,count,y;
   int         tileno,tileatr,t1,t2,t3;
   char         fullmode=0;
   int         dday=0,rzx=15,yskip=0;

   if (neogeo_hardrender){
     if (!neogeo_blitter)
       blit_start_fast(video_paletteram_pc[4095]);
     else
       blit_start(video_paletteram_pc[4095]);
   }else
     video_draw_screen_first();

   for (count=0;count<0x300;count+=2) {
      t3 = *((unsigned short *)( &video_vidram[0x10000 + count] ));
      t1 = *((unsigned short *)( &video_vidram[0x10400 + count] ));
      t2 = *((unsigned short *)( &video_vidram[0x10800 + count] ));

      // If this bit is set this new column is placed next to last one
      if (t1 & 0x40) {
         sx += (rzx + 1);
         if ( sx >= 0x1F0 )
            sx -= 0x200;

         // Get new zoom for this column
         zx = (t3 >> 8)&0x0F;

         sy = oy;
      } else {   // nope it is a new block
         // Sprite scaling
         zx = (t3 >> 8)&0x0F;

         rzy = t3 & 0xff;

         sx = (t2 >> 7);
         if ( sx >= 0x1F0 )
            sx -= 0x200;

         // Number of tiles in this strip
         my = t1 & 0x3f;
         if (my == 0x20)
            fullmode = 1;
         else if (my >= 0x21)
            fullmode = 2;   // most games use 0x21, but
         else
            fullmode = 0;   // Alpha Mission II uses 0x3f

         sy = 0x1F0 - (t1 >> 7);
         if (sy > 0x100) sy -= 0x200;

         if (fullmode == 2 || (fullmode == 1 && rzy == 0xff))
         {
            while (sy < -16) sy += 2 * (rzy + 1);
         }
         oy = sy;

           if(my==0x21) my=0x20;
         else if(rzy!=0xff && my!=0)
            my=((my*16*256)/(rzy+1) + 15)/16;

                  if(my>0x20) my=0x20;
      }

      rzx = zx;

      // No point doing anything if tile strip is 0
      if ((my==0)||(sx>311))
         continue;

      // Setup y zoom
      if(rzy==255)
         yskip=16;
      else
         dday=0;   // =256; NS990105 mslug fix

      offs = count<<6;

      // my holds the number of tiles in each vertical multisprite block
      for (y=0; y < my ;y++) {
         tileno = *((unsigned short *)(&video_vidram[offs]));
         offs+=2;
         tileatr = *((unsigned short *)(&video_vidram[offs]));
         offs+=2;

         if (tileatr&0x8)
            tileno = (tileno&~7)|(neogeo_frame_counter&7);
         else if (tileatr&0x4)
            tileno = (tileno&~3)|(neogeo_frame_counter&3);

//         tileno &= 0x7FFF;
         if (tileno>0x7FFF)
            continue;
         if(rzy!=255)
         {
            yskip=0;
            memset(video_shrinky,0,17);
            for(i=0;i<16;i++)
            {
               dday-=rzy+1;
               if(dday<=0)
               {
                  dday+=256;
                  yskip++;
                  video_shrinky[yskip]++;
               }
               else
                  video_shrinky[yskip]++;
            }
         }

         if (fullmode == 2 || (fullmode == 1 && rzy == 0xff))
         {
            if (sy >= 248) sy -= 2 * (rzy + 1);
         }
         else if (fullmode == 1)
         {
            if (y == 0x10) sy -= 2 * (rzy + 1);
         }
         else if (sy > 0x110) sy -= 0x200;


	 if (((tileatr>>8))&&(sy<224) && video_spr_usage[tileno])
	   {
	     if (neogeo_hardrender)
	       blit_drawspr(sx,sy,rzx+1,yskip,&video_paletteram_pc[(tileatr >> 8)<<4],(u32*)(&neogeo_spr_memory[tileno<<7]),tileno,(tileatr >> 8)<<4,tileatr & 0x03);
	     else {
	       if (video_spr_usage[tileno] == 2) // all solid
		 video_draw_spr_solid(tileno,tileatr >> 8, tileatr & 0x01,tileatr & 0x02, sx,sy,rzx,yskip);
	       else
		 video_draw_spr(tileno,tileatr >> 8, tileatr & 0x01,tileatr & 0x02, sx,sy,rzx,yskip);
	     }
	   }

         sy +=yskip;
      }  // for y
   }  // for count

   if (fc >= neogeo_frame_counter_speed) {
      neogeo_frame_counter++;
      fc=3;
   }

   fc++;

   if (neogeo_hardrender){
     video_draw_fixPSP();
     switch (neogeo_blitter){
     case 0:/*blit_finish(320,224,320,224);*/blit_finish_fast();break;
     case 1:blit_finish(320,224,388,272);break;
     case 2:blit_finish(320,224,420,272);break;
     case 3:blit_finish(320,224,480,272);break;
     }
   }else{
     video_draw_fix();
     blit();
   }
}


//----------------------------------------------------------------------------
void video_draw_screen2()
{
   static int      pass1_start;
   int         sx =0,sy =0,oy =0,my =0,zx = 1, rzy = 1;
   int         offs,i,count,y;
   int         tileno,tileatr,t1,t2,t3;
   char         fullmode=0;
   int         dday=0,rzx=15,yskip=0;

   if (neogeo_hardrender){
     if (!neogeo_blitter)
       blit_start_fast(video_paletteram_pc[4095]);
     else
       blit_start(video_paletteram_pc[4095]);
   } else
     video_draw_screen_first();

   t1 = *((unsigned short *)( &video_vidram[0x10400 + 4] ));

   if ((t1 & 0x40) == 0)
   {
      for (pass1_start=6;pass1_start<0x300;pass1_start+=2)
      {
         t1 = *((unsigned short *)( &video_vidram[0x10400 + pass1_start] ));

         if ((t1 & 0x40) == 0)
            break;
      }

      if (pass1_start == 6)
         pass1_start = 0;
   }
   else
      pass1_start = 0;

   for (count=pass1_start;count<0x300;count+=2) {
      t3 = *((unsigned short *)( &video_vidram[0x10000 + count] ));
      t1 = *((unsigned short *)( &video_vidram[0x10400 + count] ));
      t2 = *((unsigned short *)( &video_vidram[0x10800 + count] ));

      // If this bit is set this new column is placed next to last one
      if (t1 & 0x40) {
         sx += (rzx + 1);
         if ( sx >= 0x1F0 )
            sx -= 0x200;

         // Get new zoom for this column
         zx = (t3 >> 8)&0x0F;

         sy = oy;
      } else {   // nope it is a new block
         // Sprite scaling
         zx = (t3 >> 8)&0x0F;

         rzy = t3 & 0xff;

         sx = (t2 >> 7);
         if ( sx >= 0x1F0 )
            sx -= 0x200;

         // Number of tiles in this strip
         my = t1 & 0x3f;
         if (my == 0x20)
            fullmode = 1;
         else if (my >= 0x21)
            fullmode = 2;   // most games use 0x21, but
         else
            fullmode = 0;   // Alpha Mission II uses 0x3f

         sy = 0x1F0 - (t1 >> 7);
         if (sy > 0x100) sy -= 0x200;

         if (fullmode == 2 || (fullmode == 1 && rzy == 0xff))
         {
            while (sy < -16) sy += 2 * (rzy + 1);
         }
         oy = sy;

           if(my==0x21) my=0x20;
         else if(rzy!=0xff && my!=0)
            my=((my*16*256)/(rzy+1) + 15)/16;

                  if(my>0x20) my=0x20;

      }

      rzx = zx;

      // No point doing anything if tile strip is 0
      if ((my==0)||(sx>311))
         continue;

      // Setup y zoom
      if(rzy==255)
         yskip=16;
      else
         dday=0;   // =256; NS990105 mslug fix

      offs = count<<6;

      // my holds the number of tiles in each vertical multisprite block
      for (y=0; y < my ;y++) {
         tileno = *((unsigned short *)(&video_vidram[offs]));
         offs+=2;
         tileatr = *((unsigned short *)(&video_vidram[offs]));
         offs+=2;

         if (tileatr&0x8)
            tileno = (tileno&~7)|(neogeo_frame_counter&7);
         else if (tileatr&0x4)
            tileno = (tileno&~3)|(neogeo_frame_counter&3);

//         tileno &= 0x7FFF;
         if (tileno>0x7FFF)
            continue;

         if (fullmode == 2 || (fullmode == 1 && rzy == 0xff))
         {
            if (sy >= 224) sy -= 2 * (rzy + 1);
         }
         else if (fullmode == 1)
         {
            if (y == 0x10) sy -= 2 * (rzy + 1);
         }
         else if (sy > 0x100) sy -= 0x200;

         if(rzy!=255)
         {
            yskip=0;
            memset(video_shrinky,0,17);
            for(i=0;i<16;i++)
            {
               dday-=rzy+1;
               if(dday<=0)
               {
                  dday+=256;
                  yskip++;
                  video_shrinky[yskip]++;
               }
               else
                  video_shrinky[yskip]++;
            }
         }

	 if (((tileatr>>8)||(tileno!=0))&&(sy<224) && video_spr_usage[tileno])
	   {
	     if (neogeo_hardrender)
	       blit_drawspr(sx,sy,rzx+1,yskip,&video_paletteram_pc[(tileatr >> 8)<<4],(u32*)(&neogeo_spr_memory[tileno<<7]),tileno,(tileatr >> 8)<<4,tileatr & 0x03);
	     else {
	       if (video_spr_usage[tileno] == 2) // all solid
		 video_draw_spr_solid( tileno, tileatr >> 8, tileatr & 0x01,tileatr & 0x02, sx,sy,rzx,yskip);
	       else
		 video_draw_spr( tileno, tileatr >> 8, tileatr & 0x01,tileatr & 0x02, sx,sy,rzx,yskip);
	     }
	   }

         sy +=yskip;
      }  // for y
   }  // for count

   for (count=0;count<pass1_start;count+=2) {
      t3 = *((unsigned short *)( &video_vidram[0x10000 + count] ));
      t1 = *((unsigned short *)( &video_vidram[0x10400 + count] ));
      t2 = *((unsigned short *)( &video_vidram[0x10800 + count] ));

      // If this bit is set this new column is placed next to last one
      if (t1 & 0x40) {
         sx += (rzx + 1);
         if ( sx >= 0x1F0 )
            sx -= 0x200;

         // Get new zoom for this column
         zx = (t3 >> 8)&0x0F;

         sy = oy;
      } else {   // nope it is a new block
         // Sprite scaling
         zx = (t3 >> 8)&0x0F;

         rzy = t3 & 0xff;

         sx = (t2 >> 7);
         if ( sx >= 0x1F0 )
            sx -= 0x200;

         // Number of tiles in this strip
         my = t1 & 0x3f;
         if (my == 0x20)
            fullmode = 1;
         else if (my >= 0x21)
            fullmode = 2;   // most games use 0x21, but
         else
            fullmode = 0;   // Alpha Mission II uses 0x3f

         sy = 0x1F0 - (t1 >> 7);
         if (sy > 0x100) sy -= 0x200;

         if (fullmode == 2 || (fullmode == 1 && rzy == 0xff))
         {
            while (sy < -16) sy += 2 * (rzy + 1);
         }
         oy = sy;

           if(my==0x21) my=0x20;
         else if(rzy!=0xff && my!=0)
            my=((my*16*256)/(rzy+1) + 15)/16;

                  if(my>0x20) my=0x20;

      }

      rzx = zx;

      // No point doing anything if tile strip is 0
      if ((my==0)||(sx>311))
         continue;

      // Setup y zoom
      if(rzy==255)
         yskip=16;
      else
         dday=0;   // =256; NS990105 mslug fix

      offs = count<<6;

      // my holds the number of tiles in each vertical multisprite block
      for (y=0; y < my ;y++) {
         tileno = *((unsigned short *)(&video_vidram[offs]));
         offs+=2;
         tileatr = *((unsigned short *)(&video_vidram[offs]));
         offs+=2;

         if (tileatr&0x8)
            tileno = (tileno&~7)|(neogeo_frame_counter&7);
         else if (tileatr&0x4)
            tileno = (tileno&~3)|(neogeo_frame_counter&3);

//         tileno &= 0x7FFF;
         if (tileno>0x7FFF)
            continue;

         if (fullmode == 2 || (fullmode == 1 && rzy == 0xff))
         {
            if (sy >= 248) sy -= 2 * (rzy + 1);
         }
         else if (fullmode == 1)
         {
            if (y == 0x10) sy -= 2 * (rzy + 1);
         }
         else if (sy > 0x110) sy -= 0x200;

         if(rzy!=255)
         {
            yskip=0;
            memset(video_shrinky,0,17);
            for(i=0;i<16;i++)
            {
               video_shrinky[i+1]=0;
               dday-=rzy+1;
               if(dday<=0)
               {
                  dday+=256;
                  yskip++;
                  video_shrinky[yskip]++;
               }
               else
                  video_shrinky[yskip]++;
            }
         }

	 if (((tileatr>>8)||(tileno!=0))&&(sy<224) && video_spr_usage[tileno])
	   {
	     if (neogeo_hardrender)
	       blit_drawspr(sx,sy,rzx+1,yskip,&video_paletteram_pc[(tileatr >> 8)<<4],(u32*)(&neogeo_spr_memory[tileno<<7]),tileno,(tileatr >> 8)<<4,tileatr & 0x03);
	     else {
	       if (video_spr_usage[tileno] == 2) // all solid
		 video_draw_spr_solid( tileno, tileatr >> 8, tileatr & 0x01,tileatr & 0x02, sx,sy,rzx,yskip);
	       else
		 video_draw_spr( tileno, tileatr >> 8, tileatr & 0x01,tileatr & 0x02, sx,sy,rzx,yskip);
	     }
	   }

         sy +=yskip;
      }  // for y
   }  // for count

   if (fc >= neogeo_frame_counter_speed) {
      neogeo_frame_counter++;
      fc=3;
   }

   fc++;

   if (neogeo_hardrender){
     video_draw_fixPSP();
     switch (neogeo_blitter){
     case 0:/*blit_finish(320,224,320,224);*/blit_finish_fast();break;
     case 1:blit_finish(320,224,388,272);break;
     case 2:blit_finish(320,224,420,272);break;
     case 3:blit_finish(320,224,480,272);break;
     }
   } else {
     video_draw_fix();
     blit();
   }
}

/*void video_flip(SDL_Surface *surface)
{
#ifndef USE_VIDEO_GL
	SDL_Flip(surface);
#else
#ifdef SHOW_CONSOLE
  if (used_blitter)
  {
	tcache_hash_init();
	fcache_hash_init();
	if (view_console)
		console_draw_all_background();
  }
#endif
#ifdef DREAMCAST
  glKosBeginFrame();
#endif
  glClearColor( 0.0,0.0,0.0, 255.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  extern GLint screen_texture;

  tile_z=0.5;

#ifdef MENU_ALPHA
  video_draw_tile_textures_gl();
  video_draw_font_textures_gl();
#endif

  glBindTexture(GL_TEXTURE_2D,screen_texture);
  loadTextureParams();

#ifdef MENU_ALPHA

#ifndef DREAMCAST
  glTexImage2D(GL_TEXTURE_2D, 0, 4, 512, 512, 0,
    GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, neo4all_texture_surface);
#else
  glKosTex2D(GL_ARGB1555,512,512,neo4all_texture_surface);
#endif

#else

#ifndef DREAMCAST
  glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0,
    GL_RGB, GL_UNSIGNED_SHORT_5_6_5, neo4all_texture_surface);
#else
  glKosTex2D(GL_RGB565,512,512,neo4all_texture_surface);
#endif

#endif

  double t_x1=0.0,t_y1=0.0,t_x2=512.0,t_y2=512.0;
#ifdef DREAMCAST
  t_y1+=4.0; t_y2+=4.0;
#endif

  glBegin(GL_QUADS);
  	glTexCoord2f(0.0,0.0);
	glVertex3f(t_x1,t_y1,tile_z);

  	glTexCoord2f(1.0,0.0);
	glVertex3f(t_x2,t_y1,tile_z);

  	glTexCoord2f(1.0,1.0);
	glVertex3f(t_x2,t_y2,tile_z);

  	glTexCoord2f(0.0,1.0);
	glVertex3f(t_x1,t_y2,tile_z);
  glEnd();

#ifndef DREAMCAST
  SDL_GL_SwapBuffers();
#else
  glKosFinishFrame();
#endif
#ifdef SHOW_CONSOLE
  used_blitter=0;
#endif
#endif
}
*/

