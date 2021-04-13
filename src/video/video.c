/*******************************************
**** VIDEO.C - Video Hardware Emulation ****
*******************************************/

//-- Include Files -----------------------------------------------------------
//#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>


#include "neocd.h"
#include "video.h"


//#include "console.h"


//-- Defines -----------------------------------------------------------------
#define VIDEO_TEXT		0
#define VIDEO_NORMAL	1
#define	VIDEO_SCANLINES	2



//-- Global Variables --------------------------------------------------------
//unsigned	video_palette_selected=0;
char *          video_vidram;
unsigned short	*video_paletteram_ng;

unsigned short	*video_paletteram_pc;
unsigned short	video_palette_bank0_ng[4096];
unsigned short	video_palette_bank1_ng[4096];
unsigned short	video_palette_bank0_pc[4096];
unsigned short	video_palette_bank1_pc[4096];


unsigned short	video_color_lut[32768];
//char	video_palette_use[0x200];
//int	video_palette_dirty=0;

short			video_modulo;
unsigned short	video_pointer;
int				video_dos_cx;
int				video_dos_cy;
int				video_mode = 0;
unsigned short	*video_line_ptr[224];
unsigned char	video_fix_usage[4096];
unsigned char   rom_fix_usage[4096];

unsigned      neogeo_nb_of_tiles;

unsigned char	video_spr_usage[32768];
//unsigned char   rom_spr_usage[32768];

/*SDL_Surface		*screen;
SDL_Surface		*video_buffer;
SDL_Surface		*game_title;*/
u16 *video_buffer;

unsigned int	neogeo_frame_counter = 0;
unsigned int	neogeo_frame_counter_speed = 4;

unsigned int	video_hide_fps=1;
int				video_scanlines_capable = 1;
double			gamma_correction = 1.0;

int				fullscreen_flag=0;
int				display_mode=1;
int				snap_no;
int				frameskip=0;

//SDL_Rect		src,dest;

//-- Function Prototypes -----------------------------------------------------
void	video_shutdown(void);
void	video_draw_line(int);
int	video_set_mode(int);
void	incframeskip(void);
void	video_precalc_lut(void);

void	video_draw_screen1(void);
void	video_draw_screen2(void);
void	snapshot_init(void);
void	video_save_snapshot(void);
void	video_setup(void);



void video_reset(void)
{
	unsigned i;

	video_precalc_lut();

	memset(video_palette_bank0_ng, 0, 8192);
	memset(video_palette_bank1_ng, 0, 8192);
	memset(video_palette_bank0_pc, 0, 8192);
	memset(video_palette_bank1_pc, 0, 8192);
	for (i=0;i<4096/16;i++) {
		video_palette_bank0_pc[i*16]=0x8000;
		video_palette_bank1_pc[i*16]=0x8000;
	}

	video_paletteram_ng = video_palette_bank0_ng;
	video_paletteram_pc = video_palette_bank0_pc;

	video_modulo = 0;
	video_pointer = 0;

	//video_palette_dirty=0;
	//memset(video_palette_use,0,0x200);
	//video_palette_selected=0;

#ifdef USE_VIDEO_GL
	video_reset_gl();
#endif
	init_autoframeskip();
}

//----------------------------------------------------------------------------
int	video_init(void)
{
	int		y;
	unsigned short	*ptr;



	video_precalc_lut();



	video_vidram = calloc(1,131072);



	if (video_vidram==NULL) {
		strcpy(global_error, "VIDEO: Could not allocate vidram (128k)");
		return	0;
	}







	ptr = video_buffer = pgGetVramAddr(0,272*2);//calloc(1,512*272*2);//pgGetVramAddr(0,512);
	for(y=0;y<224;y++) {
		video_line_ptr[y] = ptr;
		//ptr += (video_buffer->pitch/2);
		ptr += VIDEOBUF_PITCH;
	}




	video_reset();

	if (video_set_mode(VIDEO_NORMAL)==0)
		return 0;





	snapshot_init();

	return 1;
}

//----------------------------------------------------------------------------
void	video_shutdown(void)
{

	//if (video_buffer != NULL) SDL_FreeSurface( video_buffer );
	//if (game_title != NULL)   SDL_FreeSurface( game_title );
	//free(video_buffer);
	free(video_vidram);

}
#ifdef DREAMCAST
extern void __sdl_dc_set_blit(int width,int height);
extern int __sdl_dc_wait_vblank;
extern int __sdl_dc_no_ask_60hz;
#endif

//----------------------------------------------------------------------------
int	video_set_mode(int mode)
{
#ifdef DREAMCAST
	__sdl_dc_no_ask_60hz=1;
#endif
#ifndef USE_VIDEO_GL
#ifdef DREAMCAST
//	if ((screen=SDL_SetVideoMode(320, 240, 16, SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF))==NULL) {
	if ((screen=SDL_SetVideoMode(320, 240, 16, SDL_HWPALETTE|SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF))==NULL) {
//	if ((screen=SDL_SetVideoMode(320, 240, 16, SDL_HWPALETTE|SDL_HWSURFACE|SDL_FULLSCREEN))==NULL) {
//	if ((screen=SDL_SetVideoMode(512, 256, 16, SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF))==NULL) {
#else
	//if ((screen=SDL_SetVideoMode(320, 240, 16, SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF))==NULL) {
#endif

#else
	if (!init_video_gl()) {
#endif
		/*fprintf( stderr, "Could not set video mode: %s\n", SDL_GetError() );

        	/SDL_Quit();
        	exit( -1 );
		return 0;
	}*/
#ifdef DREAMCAST
	__sdl_dc_no_ask_60hz=1;
#ifndef USE_VIDEO_GL
	__sdl_dc_set_blit(320,240);
	__sdl_dc_wait_vblank=0;
#endif
#endif

	video_mode = VIDEO_NORMAL;

	return 1;
}


//----------------------------------------------------------------------------
void	video_precalc_lut(void)
{
	int	ndx, rr, rg, rb;
	int 	r1=10,r2=5,r3=0,mr=0,mult=31;
#ifdef USE_VIDEO_GL
#ifdef DREAMCAST
	r1=0;
	r3=10;
#else
	r1=10;
	r3=0;
#endif
	r2=5;
	mult=31;
	mr=0x8000;
#endif

	memset(video_color_lut,1,32768);
	for(rr=0;rr<32;rr++) {
		for(rg=0;rg<32;rg++) {
			for(rb=0;rb<32;rb++) {
				ndx = ((rr&1)<<14)|((rg&1)<<13)|((rb&1)<<12)|((rr&30)<<7)
					|((rg&30)<<3)|((rb&30)>>1);
				video_color_lut[ndx] =
				     ((int)( 31 * pow( (double)rb / 31, 1 / gamma_correction ) )<<r1)
				        | mr
					|((int)( mult * pow( (double)rg / 31, 1 / gamma_correction ) )<<r2)
					|((int)( 31 * pow( (double)rr / 31, 1 / gamma_correction ) )<<r3);
			}
		}
	}
}


/*void	video_precalc_lut(void)
{
	int	ndx, rr, rg, rb;

	for(rr=0;rr<32;rr++) {
		for(rg=0;rg<32;rg++) {
			for(rb=0;rb<32;rb++) {
				ndx = ((rr&1)<<14)|((rg&1)<<13)|((rb&1)<<12)|((rr&30)<<7)
					|((rg&30)<<3)|((rb&30)>>1);
				video_color_lut[ndx] = (rb<<10) | (rg<<5) | (rr);
			}
		}
	}

}
*/
void snapshot_init(void)
{
	/* This function sets up snapshot number */
	char name[30];
	FILE *fp;

	snap_no=0;

	sprintf(name,"snap%04d.bmp", snap_no);

	while((fp=fopen(convert_path(name),"r"))!=NULL)
	{
		fclose(fp);
		snap_no++;
		sprintf(name,"snap%04d.bmp", snap_no);
	}
}




void video_save_snapshot(void)
{
	char name[30];
	sprintf(name,"snap%04d.bmp", snap_no++);
	//SDL_SaveBMP(screen,name);
}

void video_fullscreen_toggle(void)
{
#ifndef DREAMCAST
#ifndef USE_VIDEO_GL
	//SDL_WM_ToggleFullScreen(screen);
#else
	video_fullscreen_toggle_gl();
#endif
#endif

}

