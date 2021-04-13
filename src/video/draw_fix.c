/******************************************
**** Fixed Text Layer Drawing Routines ****
******************************************/

#include <stdio.h>
#include <string.h>
//#include <ctype.h>


#include "neocd.h"
#include "video.h"
#include "psp/blitter.h"
#include "ingame.h"

static u8 *font_area = NULL;

/* Draw Single FIX character */
INLINE void draw_fix_font(u16 code, u16 colour, u16 sx, u16 sy, u16 * palette, u8 *font_area)
{
  u8 y;
  u8 * fix=&font_area[(code-32)<<7];
  u16 * dest = video_line_ptr[sy] + sx;

  for(y=0;y<8;y++)
    {
      memcpy(dest,fix,8*2);
      dest += VIDEOBUF_PITCH;
      fix += 8*2;
    }
}

INLINE void draw_fix(u16 code, u16 colour, u16 sx, u16 sy, u16 * palette, char * fix_memory)
{
	u8 y;
	u32 mydword;
	u32 * fix=(u32*)&(fix_memory[code<<5]);
	u16 * dest = video_line_ptr[sy] + sx;
	u16 * paldata=&palette[colour];
	u16 col;

	for(y=0;y<8;y++)
	{
		mydword  = *fix++;

		col = (mydword>> 0)&0x0f; if (col) dest[0] = paldata[col];
		col = (mydword>> 4)&0x0f; if (col) dest[1] = paldata[col];
		col = (mydword>> 8)&0x0f; if (col) dest[2] = paldata[col];
		col = (mydword>>12)&0x0f; if (col) dest[3] = paldata[col];
		col = (mydword>>16)&0x0f; if (col) dest[4] = paldata[col];
		col = (mydword>>20)&0x0f; if (col) dest[5] = paldata[col];
		col = (mydword>>24)&0x0f; if (col) dest[6] = paldata[col];
		col = (mydword>>28)&0x0f; if (col) dest[7] = paldata[col];
		dest += VIDEOBUF_PITCH;
	}
}


/* FIX palette for fixputs*/
// For the hardware palette, the most significant bit toggles transparency
// So I have set it for color 0, and cleared it for all the while colors...
// the difference is not noticeable even in software mode
u16 palette[16]={0x0000,0x7fff,0x0000,0x0000,
		    0x0000,0x0000,0x0000,0x0000,
		    0x7fff,0x0000,0x0000,0x0000,
		    0x0000,0x0000,0x0000,0x7fff};

static void reserve_font_area() {
  int code;
  u8 *fix_memory = (u8*)&neogeo_rom_memory[458752+(0x300<<5)];

  font_area = malloc((']'-' '+1)*8*8*2);
  if (!font_area) return;

  for (code = ' '; code <= ']'; code++) {
    u32 * fix=(u32*)&(fix_memory[code<<5]);
    u32 mydword;
    int y;
    u16 *dest = (u16*)(font_area + (code-' ')*8*8*2);
    u16 col;

    for(y=0;y<8;y++)
      {
	mydword  = *fix++;

	col = (mydword>> 0)&0x0f; dest[0] = palette[col];
	col = (mydword>> 4)&0x0f; dest[1] = palette[col];
	col = (mydword>> 8)&0x0f; dest[2] = palette[col];
	col = (mydword>>12)&0x0f; dest[3] = palette[col];
	col = (mydword>>16)&0x0f; dest[4] = palette[col];
	col = (mydword>>20)&0x0f; dest[5] = palette[col];
	col = (mydword>>24)&0x0f; dest[6] = palette[col];
	col = (mydword>>28)&0x0f; dest[7] = palette[col];
	dest += 8;
    }
  }
}

void fixputs( u16 x, u16 y, const char * string )
{
  u8 i;
  int length=strlen(string);

  if ( y>27 ) return;

  if ( x+length > 40 ) {
    length=40-x;
  }

  if (length<0) return;


  y<<=3;

  for (i=0; i<length; i++) {
    char code = toupper(string[i]);
    if (code > 32)
      draw_fix_font(code,0,(x+i)<<3,y,palette,font_area);
  }
}

void fixputsPSP( u16 x, u16 y, const char * string )
{
  u8 i;
  int length=strlen(string);

  if ( y>27 ) return;

  if ( x+length > 40 ) {
    length=40-x;
  }

  if (length<0) return;


  y<<=3;

  for (i=0; i<length; i++) {
    u16 code = toupper(string[i]);
    if (code > 32) {
      code -= 32;
    // draw_fix(code,0,(x+i)<<3,y,palette, &neogeo_rom_memory[458752]);
      blit_drawfix_key((x+i)<<3,y,&palette[0],&font_area[code<<7],code,0);
    }
  }
}


/* Draw entire Character Foreground */
void video_draw_fix(void)
{
  u16 x, y;
  u16 code, colour;
  u16 * fixarea=(u16 *)&video_vidram[0xe004];

  for (y=0; y < 28; y++)
    {
      for (x = 0; x < 40; x++)
	{
	  code = fixarea[x << 5];

	  colour = (code&0xf000)>>8;
	  code  &= 0xfff;

	  if(video_fix_usage[code]) draw_fix(code,colour,(x<<3),(y<<3), video_paletteram_pc, neogeo_fix_memory);
	}
      fixarea++;
    }
  overlay_ingame_interface();
}

/* Draw entire Character Foreground */
void video_draw_fixPSP(void)
{
  u16 x, y;
  u16 code, colour;
  u16 * fixarea=(u16 *)&video_vidram[0xe004];

  //blit_spritemode(8,8,8,8);

  for (y=0; y < 28; y++)
    {
      for (x = 0; x < 40; x++)
	{
	  code = fixarea[x << 5];

	  colour = (code&0xf000)>>8;
	  code  &= 0xfff;

	  if(video_fix_usage[code])
	    blit_drawfix(x<<3,y<<3,&video_paletteram_pc[colour],(u32*)&(neogeo_fix_memory[code<<5]),code,colour);
	}
      fixarea++;
    }
  overlay_ingame_interface();
}

void init_draw_fix(void) {
  if (!font_area) reserve_font_area();
}
