#include <psppower.h>

//#include "main.h"
#include "psp.h"
#include "blitter.h"
#include "pg.h"
#include "neocd.h"
//#include "syscall.h"


#define TITLE_COL ((31)|(26<<5)|(31<<10))
#define PATH_COL ((31)|(24<<5)|(28<<10))

#define FILE_COL ((20)|(20<<5)|(31<<10))
#define DIR_COL ((8)|(31<<5)|(8<<10))

#define SEL_COL ((30)|(30<<5)|(31<<10))
#define SELDIR_COL ((28)|(31<<5)|(28<<10))


#define INFOBAR_COL ((31)|(24<<5)|(20<<10))

extern int neogeo_region,neogeo_mp3_volume;
extern int MP3_pause,bLoop;
u32 new_pad,old_pad;
extern int neogeo_sound_enable,psp_cpuclock,neogeo_frameskip,neogeo_mp3_enable,psp_vsync;
extern int neogeo_autofireA,neogeo_autofireB,neogeo_autofireC,neogeo_autofireD;
int exit_menu;
extern int neogeo_speedlimit,neogeo_showfps,neogeo_blitter,neo4all_z80_cycles_idx,neo4all_68k_cycles_idx,neogeo_hardrender;
extern int ExitCallback(void);

extern int	neogeo_hreset(void);

int menu_reset(){

	pgFillBox(240-150+1,136-15+1,240+150-1,136+22-1,(16<<10)|(10<<5)|7);
	pgDrawFrame(240-150,136-15,240+150,136+22,(12<<10)|(8<<5)|5);
	pgPrintCenter(16,(15<<10)|(31<<5)|31,"Reset NEOGEO ?",0);
	pgPrintCenter(18,(31<<10)|(31<<5)|31,"\1 to confirm, \2 to cancel",0);
	pgScreenFlipV();
	while (get_pad()) pgWaitV();
	while (1){
				int pad=get_pad();
				if (pad) while (get_pad()) pgWaitV();
				if (pad&PSP_CTRL_CIRCLE) {
					pgFillBox(240-150+1,136-15+1,240+150-1,136+22-1,(16<<10)|(10<<5)|7);
					pgDrawFrame(240-150,136-15,240+150,136+22,(12<<10)|(8<<5)|5);
					pgPrintCenter(16,(15<<10)|(31<<5)|31,"Reseting...",0);
					pgPrintCenter(18,(31<<10)|(31<<5)|31,"Please wait",0);
					pgScreenFlipV();
					neogeo_hreset();
					return 1;

				}
				if (pad&PSP_CTRL_CROSS) return 0;
	}

}

int menu_exitbrowser(){
	bLoop=0;
	return 1;
}


typedef struct {
		const char label[64];
		int (*menu_func)(void);
		int	*value_int;
		int	values_list_size;
		int values_list[16];
		char* values_list_label[16];
} menu_time_t;

menu_time_t psp_menu[]={
	{"Sound : ",NULL,&neogeo_sound_enable,2,{0,1},{"Off","On"}},
	{"Music : ",NULL,&neogeo_mp3_enable,2,{0,1},{"Off","On"}},
	{"MP3 volume % : ",NULL,&neogeo_mp3_volume,10,{10,20,30,40,50,60,70,80,90,100},NULL},
	{"Render : ",NULL,&neogeo_blitter,4,{0,1,2,3},{"1:1","Zoom1","Zoom2","Full"}},
	{"Frameskip : ",NULL,&neogeo_frameskip,7,{0,1,2,3,4,5,6},{"0","1","2","3","4","5","Auto"}},
	{"-----------",NULL,NULL,0,NULL,NULL},
	{"Select lock: ",NULL,&select_lock,2,{0,1},{"Off","On"}},
	{"Autofire A : ",NULL,&neogeo_autofireA,4,{0,1,2,3},{"Off","Slow","Medium","Fast"}},
	{"Autofire B : ",NULL,&neogeo_autofireB,4,{0,1,2,3},{"Off","Slow","Medium","Fast"}},
	{"Autofire C : ",NULL,&neogeo_autofireC,4,{0,1,2,3},{"Off","Slow","Medium","Fast"}},
	{"Autofire D : ",NULL,&neogeo_autofireD,4,{0,1,2,3},{"Off","Slow","Medium","Fast"}},
	{"-----------",NULL,NULL,0,NULL,NULL},
	{"GFX Engine : ",NULL,&neogeo_hardrender,2,{0,1},{"Software","Hardware"}},
	{"Video sync : ",NULL,&psp_vsync,2,{0,1},{"Off","On"}},
	{"Show FPS : ",NULL,&neogeo_showfps,2,{0,1},{"Off","On"}},
	{"60fps limit : ",NULL,&neogeo_speedlimit,2,{0,1},{"Off","On"}},
	{"-----------",NULL,NULL,0,NULL,NULL},
	{"PSP clock : ",NULL,&psp_cpuclock,3,{222,266,333},NULL},
	{"-----------",NULL,NULL,0,NULL,NULL},
	{"NEOGEO Region : ",menu_reset,&neogeo_region,3,{0,1,2},{"Japan","USA","Europe"}},
	{"Reset NEOGEO",menu_reset,NULL,0,NULL,NULL},
	{"Return to GAME browser",menu_exitbrowser,NULL,0,NULL,NULL},
	{"-----------",NULL,NULL,0,NULL,NULL},
	{"Exit emulator",ExitCallback,NULL,0,NULL,NULL}
};

#define MENU_ITEMS 24


int showmenu(void)
{
	int retval;
	unsigned long color=RGB_WHITE;

	static signed int sel=0, rows=25,top=0;
	int x, y, h, i,pad_cnt=5;
	int old_mp3_pause;

	old_mp3_pause=MP3_pause;

	MP3Pause(1);
	neogeo_soundmute=1;
	scePowerSetClockFrequency(222,222,111);
	pgScreenFrame(2,0);
	pgFillAllvram(0);

	old_pad=0;
	for(;;){
  	show_background(192);
    pgDrawFrame(12,12,480-12,272-12,28|(28<<5)|(28<<10));
    pgDrawFrame(11,11,480-11,272-11,30|(30<<5)|(30<<10));

    new_pad=0;
    if (!pad_cnt) new_pad=get_pad();
    else pad_cnt--;
    pgWaitV();

		if (new_pad) {
			 		if (old_pad==new_pad) pad_cnt=2;
			 		else pad_cnt=5;
			 		old_pad=new_pad;
			 	}

		if(new_pad & PSP_CTRL_CIRCLE){
			if (psp_menu[sel].menu_func)
				if ((*psp_menu[sel].menu_func)()) {retval=0;break;}
		}
    else if(new_pad & PSP_CTRL_CROSS)   { retval=0;break; }
    else if(new_pad & PSP_CTRL_UP)      { sel--;    }
    else if(new_pad & PSP_CTRL_DOWN)    { sel++;    }
    else if(new_pad & PSP_CTRL_LEFT)    {
    	if (psp_menu[sel].value_int){
    		int cur=*(psp_menu[sel].value_int);
    			for (i=1;i<psp_menu[sel].values_list_size;i++){
    				if (cur==psp_menu[sel].values_list[i]) {
    					*(psp_menu[sel].value_int)=psp_menu[sel].values_list[i-1];
    					break;
    				}
    		  }
    	}
    }
    else if(new_pad & PSP_CTRL_RIGHT)    {
    	if (psp_menu[sel].value_int){
    		int cur=*(psp_menu[sel].value_int);
    			for (i=0;i<psp_menu[sel].values_list_size-1;i++){
    				if (cur==psp_menu[sel].values_list[i]) {
    					*(psp_menu[sel].value_int)=psp_menu[sel].values_list[i+1];
    					break;
    				}
    		  }
    	}
    }


		if(top > MENU_ITEMS-rows)	top=MENU_ITEMS-rows;
		if(top < 0)				top=0;
		if(sel >= MENU_ITEMS)		sel=0;
		if(sel < 0)				sel=MENU_ITEMS-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;


    pgPrint(1,0,TITLE_COL,"[NEOCDPSP " VERSION_STR "] - Menu",0);
    pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Return game \5,\7 to select ,\6,\b to change value",0);


		if(MENU_ITEMS > rows){
			h = 219;
			pgDrawFrame(445,25,446,248,0x1f<<10);
			pgFillBox(448, h*top/MENU_ITEMS + 27,
				460, h*(top+rows)/MENU_ITEMS + 27,0x1f<<10);
		}

		x=4; y=4;
		for(i=0; i<rows; i++){
			if((top+i) >= MENU_ITEMS) break;
			if((top+i) == sel) 	color = SEL_COL;
			else color = FILE_COL;
			if (color==SEL_COL) pgPrintSel(x, y, color, psp_menu[top+i].label);
			else pgPrint(x, y, color, psp_menu[top+i].label,0);
			if (psp_menu[top+i].value_int) {
				if (psp_menu[top+i].values_list_label[0]){
					if (psp_menu[top+i].values_list_label[*(psp_menu[top+i].value_int)])
						pgPrint(x+strlen(psp_menu[top+i].label), y, color, psp_menu[top+i].values_list_label[*(psp_menu[top+i].value_int)],0);
				}else	pgPrintDecTrim(x+strlen(psp_menu[top+i].label), y, color, *(psp_menu[top+i].value_int),0);
			}

			y+=1;
		}

		pgScreenFlipV();

        if(psp_ExitCheck()) {
            retval=-1;
            break;
        }
	}


	set_cpu_clock();
	init_autoframeskip();
	neogeo_adjust_cycles(neo4all_68k_cycles_idx,neo4all_z80_cycles_idx);
	neogeo_setregion();

	//since we cleared the vram, caches are corrupted
	blit_fix_resetcache();
	blit_spr_resetcache();

	if (!old_mp3_pause) MP3Pause(0);
	neogeo_soundmute=0;
	pgFillAllvram(0);
	pgScreenFrame(1,0);
	if (!neogeo_mp3_enable) cdda_stop();

	while (get_pad()) pgWaitV();
	return retval;

}

int save_settings(void){
	FILE *f;
	int l;
	//remove("ms0:/PSP/GAME/NEOCDPSP/neocdpsp.ini");
	sprintf(tmp_str,"%s%s",launchDir,"neocdpsp.ini");
	f = fopen(tmp_str,"wb");
	if (!f){
		ErrorMsg("cannot save settings");
		return -1;
	}
	l=(VERSION_MAJOR<<16)|VERSION_MINOR;
	fwrite(&l,1,4,f);
	fwrite(&neogeo_sound_enable,1,4,f);
	fwrite(&neogeo_mp3_enable,1,4,f);
	fwrite(&neogeo_frameskip,1,4,f);
	fwrite(&neogeo_blitter,1,4,f);
	fwrite(&psp_vsync,1,4,f);
	fwrite(&neogeo_showfps,1,4,f);
	fwrite(&neogeo_speedlimit,1,4,f);
	fwrite(&psp_cpuclock,1,4,f);
	fwrite(&neo4all_68k_cycles_idx,1,4,f);
	fwrite(&neo4all_z80_cycles_idx,1,4,f);
	fwrite(&neogeo_hardrender,1,4,f);
	fwrite(&neogeo_autofireA,1,4,f);
	fwrite(&neogeo_autofireB,1,4,f);
	fwrite(&neogeo_autofireC,1,4,f);
	fwrite(&neogeo_autofireD,1,4,f);
	fwrite(&neogeo_region,1,4,f);
	fwrite(&neogeo_mp3_volume,1,4,f);
	fwrite(&select_lock,1,4,f);
	fclose(f);
	return 0;
}

int load_settings(void){
	FILE *f;
	int l;
	sprintf(tmp_str,"%s%s",launchDir,"neocdpsp.ini");
	f = fopen(tmp_str,"rb");
	if (!f){
		debug_log("cannot load settings");
		return -1;
	}
	fread(&l,1,4,f);
	if (l>((VERSION_MAJOR<<16)|VERSION_MINOR)){
		debug_log("cannot load settings, too old version");
		fclose(f);
		return -2;
	}

#define READ_SETTING(a) \
if (fread(&l,1,4,f)==4) a=l; \
else {fclose(f);return -3;}

	READ_SETTING(neogeo_sound_enable)
	READ_SETTING(neogeo_mp3_enable)
	READ_SETTING(neogeo_frameskip)
	READ_SETTING(neogeo_blitter)
	READ_SETTING(psp_vsync)
	READ_SETTING(neogeo_showfps)
	READ_SETTING(neogeo_speedlimit)
	READ_SETTING(psp_cpuclock)
	READ_SETTING(neo4all_68k_cycles_idx)
	READ_SETTING(neo4all_z80_cycles_idx)
	READ_SETTING(neogeo_hardrender)
	READ_SETTING(neogeo_autofireA)
	READ_SETTING(neogeo_autofireB)
	READ_SETTING(neogeo_autofireC)
	READ_SETTING(neogeo_autofireD)
	READ_SETTING(neogeo_region)
	READ_SETTING(neogeo_mp3_volume)
	READ_SETTING(select_lock)
	fclose(f);
	return 0;
}
