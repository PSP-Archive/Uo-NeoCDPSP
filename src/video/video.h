/*******************************************
**** VIDEO.H - Video Hardware Emulation ****
****            Header File             ****
*******************************************/

#ifndef	VIDEO_H
#define VIDEO_H

//#include <SDL.h>

/*-- Defines ---------------------------------------------------------------*/
#define VIDEO_TEXT	0
#define VIDEO_NORMAL	1
#define	VIDEO_SCANLINES	2

#define VIDEOBUF_PITCH 512





/*-- Global Variables ------------------------------------------------------*/
//extern unsigned	video_palette_selected;
extern char		*video_vidram;
extern unsigned short	*video_paletteram_ng;
extern unsigned short	video_palette_bank0_ng[4096];
extern unsigned short	video_palette_bank1_ng[4096];
extern unsigned short	*video_paletteram_pc;
extern unsigned short	video_palette_bank0_pc[4096];
extern unsigned short	video_palette_bank1_pc[4096];

extern short		video_modulo;
extern unsigned short	video_pointer;
extern unsigned short	*video_paletteram;
extern unsigned short	*video_paletteram_pc;
extern unsigned short	video_palette_bank0[4096];
extern unsigned short	video_palette_bank1[4096];
extern unsigned short	*video_line_ptr[224];
extern unsigned char	video_fix_usage[4096];
extern unsigned char	rom_fix_usage[4096];
extern unsigned char	video_spr_usage[32768];
//extern unsigned char	rom_spr_usage[32768];
extern unsigned int	video_hide_fps;
extern unsigned short	video_color_lut[32768];
//extern char	video_palette_use[0x200];
//extern int	video_palette_dirty;
extern int	used_blitter;

/*extern SDL_Surface	*screen;
extern SDL_Surface	*video_buffer;
extern SDL_Surface	*game_title;*/
extern u16 *video_buffer;
extern int		video_mode;
extern double		gamma_correction;
extern int		frameskip;

extern unsigned int	neogeo_frame_counter;
extern unsigned int	neogeo_frame_counter_speed;

/*-- video.c functions ----------------------------------------------------*/
int  video_init(void);
void video_reset(void);
void video_shutdown(void);
int  video_set_mode(int);
void video_draw_screen1(void);
void video_draw_screen2(void);
void video_save_snapshot(void);
void video_draw_spr(unsigned int code, unsigned int color, int flipx,
			int flipy, int sx, int sy, int zx, int zy);
void video_draw_spr_solid(unsigned int code, unsigned int color, int flipx,
			int flipy, int sx, int sy, int zx, int zy);
void video_setup(void);
void video_fullscreen_toggle(void);
void video_mode_toggle(void);
void incframeskip(void);
//void video_flip(SDL_Surface *surface);

/*-- draw_fix.c functions -------------------------------------------------*/
void video_draw_fix(void);
void fixputs(u16 x, u16 y, const char * string);
void fixputsPSP(u16 x, u16 y, const char * string);
void video_draw_fixPSP(void);

// draw.c
void blit(void);
void video_draw_blank(void);
void init_draw_fix(void);

#endif /* VIDEO_H */

