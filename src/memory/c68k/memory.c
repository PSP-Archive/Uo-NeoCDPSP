/********************************************
*   NeoCD Memory Mapping (C version)        *
*********************************************
* Fosters(2001,2003)                        *
********************************************/

//#include <SDL/SDL.h>
//#include <SDL/SDL_endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "neocd.h"
#include "psp/blitter.h"

#ifdef DEBUG_MEMORY
extern int trazando;
#endif

extern int nb_interlace;

char mem_str[256];

extern unsigned short    *video_paletteram_ng;

#if 0
    #define logaccess(...) printf(__VA_ARGS__)
#else
    #define logaccess(...)
#endif

#define RASTER_COUNTER_START 0x1f0	/* value assumed right after vblank */
#define RASTER_COUNTER_RELOAD 0x0f8	/* value assumed after 0x1ff */
#define RASTER_LINE_RELOAD (0x200 - RASTER_COUNTER_START)

/*** Globals ***************************************************************************************/
int watchdog_counter=-1;
int memcard_write=0;

void   initialize_memmap(void);
unsigned int m68k_read_memory_8(unsigned int address);
unsigned int m68k_read_memory_16(unsigned int address);
unsigned int m68k_read_memory_32(unsigned int address);
void m68k_write_memory_8(unsigned int address, unsigned int value);
void m68k_write_memory_16(unsigned int address, unsigned int value);
void m68k_write_memory_32(unsigned int address, unsigned int value);


/***Helper Functions********************************************************************************/
static int    read_upload(int);
static void   write_upload(int, int);
static void   write_upload_word(int, int);
static void   write_upload_dword(int, int);

static int    cpu_readvidreg(int);
static void   cpu_writevidreg(int, int);
static void   cpu_writeswitch(int, int);
static int    cpu_readcoin(int);

static void   write_memcard(int,int);

void cdda_control(void);

static void   watchdog_reset_w(void);


/***************************************************************************************************/
void initialize_memmap(void) {
    return;
}


/***************************************************************************************************/

void	swab( const void* src1, const void* src2, int isize)
{
	char*	ptr1;
	char*	ptr2;
	char	tmp;
	int		ic1;


	ptr1 = (char*)src1;
	ptr2 = (char*)src2;
	for ( ic1=0 ; ic1<isize-1 ; ic1+=2){
		tmp = ptr1[ic1+0];
		ptr2[ic1+0] = ptr1[ic1+1];
		ptr2[ic1+1] = tmp;
	}


}

unsigned int  FASTCALL m68k_read_memory_8(const unsigned int off) {
#ifdef DEBUG_MEMORY
if (trazando) printf("m68k_read_memory_8(0x%X)\n",off); fflush(stdout);
#endif
    unsigned int offset = off & 0xffffff;
    if(offset<0x200000)
        return neogeo_prg_memory[offset^1];

    switch(offset>>16)
    {
        case    0x30:    return read_player1();
    case 0x33:
    case    0x32:    return cpu_readcoin(offset);
        case    0x34:    return read_player2();
        case    0x38:    return read_pl12_startsel();
        case    0x80:    if(offset&0x01) return neogeo_memorycard[(offset&0x3fff)>>1];
                         else return -1;

        /* BIOS ROM */
        case    0xc0:
        case    0xc1:
        case    0xc2:
        case    0xc3:
        case    0xc4:
        case    0xc5:
        case    0xc6:
        case    0xc7:    return neogeo_rom_memory[(offset^1)&0x0fffff];

        case    0xe0:
        case    0xe1:
        case    0xe2:
        case    0xe3:    return read_upload(offset&0xfffff);

        default:
          //  logaccess("m68k_read_memory_8(0x%x) PC=%x\n",offset,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
    return 0xff;
}

/***************************************************************************************************/
unsigned int  FASTCALL m68k_read_memory_16(const unsigned int off) {
#ifdef DEBUG_MEMORY
if (trazando) printf("m68k_read_memory_16f(0x%X)\n",off); fflush(stdout);
#endif
    unsigned int offset = off & 0xffffff;
    unsigned int off16;
    if(offset<0x200000)
        return *((u16*)&neogeo_prg_memory[offset]);
    off16 = offset >> 16;

    switch(off16)
    {
    case 0x33:
    case    0x32:    return cpu_readcoin(offset);
        case    0x3c:    return cpu_readvidreg(offset);
        case    0x80:    return 0xff00|neogeo_memorycard[(offset&0x3fff)>>1];

        /* BIOS ROM */
        case    0xc0:
        case    0xc1:
        case    0xc2:
        case    0xc3:
        case    0xc4:
        case    0xc5:
        case    0xc6:
        case    0xc7:    return *(u16*)&neogeo_rom_memory[offset&0x0fffff];

		case	0xff:	 break;


        default:
          //  logaccess("m68k_read_memory_16(0x%x) PC=%x\n",offset,m68k_get_reg(NULL,M68K_REG_PPC));
	  if (off16 >= 0x40 && off16 <= 0x7f) {
	    // mirrored palette
	    return video_paletteram_ng[offset&0x1fff];
	  }

	  break;
    }
    return 0xffff;
}


/***************************************************************************************************/
unsigned int  FASTCALL m68k_read_memory_32f(const unsigned int off) {
#ifdef DEBUG_MEMORY
if (trazando) printf("m68k_read_memory_32f(0x%X)\n",off); fflush(stdout);
#endif
    unsigned int offset = off & 0xffffff;
    unsigned int data;
    data=m68k_read_memory_16(offset)<<16;
    data|=m68k_read_memory_16(offset+2);
    return data;
}

unsigned int  m68k_read_memory_32(unsigned int offset) {
#ifdef DEBUG_MEMORY
if (trazando) printf("m68k_read_memory_32(0x%X)\n",offset); fflush(stdout);
#endif
    return m68k_read_memory_32f(offset);
}



/***************************************************************************************************/
void FASTCALL m68k_write_memory_8(const unsigned int off, unsigned int data) {
#ifdef DEBUG_MEMORY
if (trazando) printf("m68k_write_memory_8(0x%X,0x%X)\n",off,data); fflush(stdout);
#endif
    int temp;
    unsigned int offset = off & 0xffffff;
    data&=0xff;
    if(offset<0x200000) {
        neogeo_prg_memory[offset^1]=(char)data;
	if ((offset == ((0x108000+30455)^1)) && data <= 7) {
	  neogeo_do_cdda(data,neogeo_prg_memory[(0x108000+30456)^1]);
	}
        return;
    }

    switch(offset>>16)
    {
        case    0x30:    watchdog_reset_w(); break;
        case    0x32:

	  if (neogeo_sound_enable) {
	    sound_code=data;
	    pending_command=1;
	    mz80nmi();
	    ExitOnEI = 1;
	    mz80exec(60000);
	  }

	  break;
		case	0x38:	break;
        case    0x3a:    cpu_writeswitch(offset, data); break;
        case    0x3c:    temp=cpu_readvidreg(offset);
            if(offset&0x01) cpu_writevidreg(offset, (temp&0xff)|(data<<8));
            else cpu_writevidreg(offset, (temp&0xff00)|data);
            break;
        case    0x80:    if(offset&0x01) write_memcard(offset,data); break;


        /* upload */
        case    0xe0:
        case    0xe1:
        case    0xe2:
        case    0xe3:   write_upload(offset&0xfffff,data); break;
		/* cdrom */
		case	0xff:	break;

        default:
       //     logaccess("m68k_write_memory_8(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
}


/***************************************************************************************************/
void FASTCALL m68k_write_memory_16(const unsigned int off, unsigned int data) {
#ifdef DEBUG_MEMORY
if (trazando) printf("m68k_write_memory_16(0x%X,0x%X)\n",off,data); fflush(stdout);
#endif
    unsigned int offset = off & 0xffffff;
    unsigned int off16;
    data&=0xffff;

    if(offset<0x200000) {
        *(u16*)&neogeo_prg_memory[offset]=(u16)data;
        return;
    }
    off16 = offset >> 16;

    switch(off16)
    {
	case	0x2f:	break;
        case    0x32:

	  if (neogeo_sound_enable) {
	    sound_code=(data >> 8);
	    pending_command=1;
	    mz80nmi();
	    ExitOnEI = 1;
	    mz80exec(60000);
	  }

	  break;
        case    0x3a:    cpu_writeswitch(offset, data); break;
        case    0x3c:    cpu_writevidreg(offset, data); break;
        case    0x80:    write_memcard(offset,data); break;

        /* upload */
        case    0xe0:
        case    0xe1:
        case    0xe2:
        case    0xe3:    write_upload_word(offset&0xfffff,data); break;
		case	0xff: break;

        default:
	  if (off16 >= 0x40 && off16 <= 0x7f) {
	    // mirrored palette
            offset =(offset&0x1fff)>>1;
	    data  &=0x7fff;
	    blit_fix_resetcache();blit_spr_resetcache();
	    video_paletteram_ng[offset]=(u16)data;
	    video_paletteram_pc[offset]=video_color_lut[data]|(offset&15?0:0x8000);
	  }
          //  logaccess("m68k_write_memory_16(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
}


/***************************************************************************************************/
void FASTCALL m68k_write_memory_32(const unsigned int off, unsigned int data) {
#ifdef DEBUG_MEMORY
if (trazando) printf("m68k_write_memory_32(0x%X,0x%X)\n",off,data); fflush(stdout);
#endif
      unsigned int offset = off & 0xffffff;
	unsigned int word1=(data>>16)&0xffff;
	unsigned int word2=data&0xffff;
	m68k_write_memory_16(offset,word1);
	m68k_write_memory_16(offset+2,word2);
}


/***************************************************************************************************/
static int    cpu_readvidreg(int offset)
{
#ifdef DEBUG_MEMORY
if (trazando) printf("cpu_readvidreg(0x%X)\n",offset); fflush(stdout);
#endif
    switch(offset)
    {
        case    0x3c0000:    return *(u16*)&video_vidram[video_pointer<<1]; break;
        case    0x3c0002:    return *(u16*)&video_vidram[video_pointer<<1]; break;
        case    0x3c0004:    return video_modulo; break;
        case    0x3c0006:
	  {
	    /* Determine the rastercounter based on the number of executed
	       cycles */
	    int cycles = C68k_Get_CycleDone(&C68K);
	    int scanline = cycles*264.0/NEO4ALL_Z80_NORMAL_CYCLES;
	    int rastercounter;
	    if (scanline < RASTER_LINE_RELOAD)
	      rastercounter = RASTER_COUNTER_START + scanline;
	    else
	      rastercounter = RASTER_COUNTER_RELOAD + scanline - RASTER_LINE_RELOAD;
	    return ((rastercounter<<7)&0xff80)|
	    (neogeo_frame_counter&7); break;
	  }
        case    0x3c0008:    return *(u16*)&video_vidram[video_pointer<<1]; break;
        case    0x3c000a:    return *(u16*)&video_vidram[video_pointer<<1]; break;

        default:
         //   logaccess("cpu_readvidreg(0x%x) PC=%x\n",offset,m68k_get_reg(NULL,M68K_REG_PC));
            return 0;
            break;
    }
}


/***************************************************************************************************/
static void    cpu_writevidreg(int offset, int data)
{
#ifdef DEBUG_MEMORY
if (trazando) printf("cpu_writevidreg(0x%X,0x%X)\n",offset,data); fflush(stdout);
#endif
    switch(offset)
    {
        case    0x3c0000:    video_pointer=(u16)data; break;
        case    0x3c0002:    *(u16*)&video_vidram[video_pointer<<1]=(u16)data;
                             video_pointer+=video_modulo;
                             //blit_spr_resetcache();
                             break;
        case    0x3c0004:    video_modulo=(s16)data; break;

        case    0x3c0006:    neogeo_frame_counter_speed=((data>>8)&0xff)+1; break;

        case    0x3c0008:    /* IRQ position */    break;
        case    0x3c000a:    /* IRQ position */    break;
        case    0x3c000c:    /* IRQ acknowledge */ break;

        default:
        //    logaccess("cpu_writevidreg(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
}

/***************************************************************************************************/

static void     neogeo_setpalbank0 (void) {
#ifdef DEBUG_MEMORY
if (trazando) puts("neogeo_setpalbank0()"); fflush(stdout);
#endif
    video_paletteram_ng=video_palette_bank0_ng;
    video_paletteram_pc=video_palette_bank0_pc;

}


static void     neogeo_setpalbank1 (void) {
#ifdef DEBUG_MEMORY
if (trazando) puts("neogeo_setpalbank1()"); fflush(stdout);
#endif
    video_paletteram_ng=video_palette_bank1_ng;
    video_paletteram_pc=video_palette_bank1_pc;
}


static void    neogeo_select_bios_vectors (void) {
#ifdef DEBUG_MEMORY
if (trazando) puts("neogeo_select_bios_vectors()"); fflush(stdout);
#endif
    memcpy(neogeo_prg_memory, neogeo_rom_memory, 0x80);
}


static void    neogeo_select_game_vectors (void) {
#ifdef DEBUG_MEMORY
if (trazando) puts("neogeo_select_game_vectors()"); fflush(stdout);
#endif
    logaccess("write game vectors stub\n");
}


static void    cpu_writeswitch(int offset, int data)
{
#ifdef DEBUG_MEMORY
if (trazando) printf("cpu_writeswitch(0x%X,0x%X)\n",offset,data); fflush(stdout);
#endif
 offset &= 0x1e;
    switch(offset)
    {
        case 0: /* NOP */ break;

        case 2: neogeo_select_bios_vectors(); break;

        case 0xe: neogeo_setpalbank1(); break;

        case 0x10: /* NOP */ break;

        case 0x12: neogeo_select_game_vectors(); break;

        case 0x1e: neogeo_setpalbank0(); break;

        default:
       //     logaccess("cpu_writeswitch(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
}

/***************************************************************************************************/

void neogeo_sound_irq(int irq)
{
#ifdef DEBUG_MEMORY
if (trazando) printf("neogeo_sound_irq(0x%X)\n",irq); fflush(stdout);
#endif

    if (neogeo_sound_enable)
    {
    	if (irq) {
        	//logaccess("neogeo_sound_irq %d\n",irq);
        	mz80int(0);
    	} else {
		mz80ClearPendingInterrupt();
        	//mz80ClearPendingInterrupt();
        	//logaccess("neogeo_sound_end %d\n",irq);
	}
    }

}



static int cpu_readcoin(int addr)
{
#ifdef DEBUG_MEMORY
if (trazando) printf("cpu_readcoin(0x%X)\n",addr); fflush(stdout);
#endif
    addr &= 0xFFFF;
    if (addr & 0x1) {
        int coinflip = pd4990a_testbit_r();
        int databit = pd4990a_databit_r();
        return 0xff ^ (coinflip << 6) ^ (databit << 7);
    }
    {
      int res = result_code;

      if (neogeo_sound_enable)
	{
        	if (pending_command)
           		 res &= 0x7f;
        }

        return res;
    }
}


static void watchdog_reset_w (void)
{
#ifdef DEBUG_MEMORY
if (trazando) puts("watchdog_reset_w()"); fflush(stdout);
#endif
    //if (watchdog_counter == -1) logaccess("Watchdog Armed!\n");
    watchdog_counter=3 * 60;  /* 3s * 60 fps */
}

static int read_upload(int offset) {
    int zone = m68k_read_memory_8(0x10FEDA);
   // int bank = m68k_read_memory_8(0x10FEDB);
    // I am not sure wether the read is really usefull...
    // anyway it doesn't seem to do any harm...

    /* The upload area is still pure guessing stuff.
       What we know for sure :
        - the fix area seems correct, see metal slug in write_upload
	- for zone 1, 0xff seems to work everywhere, and king of monsters 2
	  hangs if we return the z80 memory instead
        - for zone 0, if we return the 68k memory, then breakers crashes
	  as soon as a hit is exchanged in the game. Since the offsets for
	  this zone look very much like z80 offsets, we will try the z80
	  memory here. Also if we disable the writes in this area, the kof97
	  hangs with the message "z80 ioerror, io=0, data=30", so it seems
	  *REALLY* related to the z80 !
	Now I still don't know wether the offsets should be allowed to read/
	write everywhere or should be restricted ? */

    switch (zone) {
    case 0x00:  // /* 68000 */          return neogeo_prg_memory[offset^1];
      return subcpu_memspace[(offset>>1)^1];

    case 0x01: /* Z80 */
      // if (offset < 0xf800)
      return 0xff;
      // return subcpu_memspace[(offset>>1) ^ 1];
    case 0x11: /* FIX */
      return neogeo_fix_memory[offset>>1];
    default:
           //sprintf(mem_str,"read_upload unimplemented zone %x\n",zone);
           //debug_log(mem_str);
	  // This is probably not the z80, or at least not this way.
	  // try master of monsters 2 for a clear demonstration that
	  // it can't possibly work this way
            return -1;
    }
}


static void write_upload(int offset, int data) {
    int zone = m68k_read_memory_8(0x10FEDA);
    /* int bank = m68k_read_memory_8(0x10FEDB); */
    // This write is really used at least by metal slug to update the
    // fix area when choosing "Exit" from the main screen. (cd screen)

    switch (zone) {
        case 0x00: /* 68000 */
	  /* It this really 68k write ?
	     anyway, this breaks breakers, just start to play and hit
	     the opponent (or be hit), it hangs the game if this write
	     is authorized */
	  // neogeo_prg_memory[offset^1]=(char)data;
	  offset >>=1;
	  // if (offset >= 0xf800) // ???
	    subcpu_memspace[(offset)^1]=(char)data;
	  break;
        case 0x01: /* Z80 */
	  // if (offset >= 0xf800)
	  // subcpu_memspace[(offset>>1)^1]=(char)data;
	  // disabled, see king of monsters 2
	  break;
        case 0x11: /* FIX */
            neogeo_fix_memory[offset>>1]=(char)data;

            blit_fix_resetcache();
            break;
        default:
         //sprintf(mem_str,"write_upload unimplemented zone %x\n",zone);
         //debug_log(mem_str);
			break;
    }
}


static void write_upload_word(int offset, int data) {
    int zone = m68k_read_memory_8(0x10FEDA);
    int bank = m68k_read_memory_8(0x10FEDB);
    int offset2;
    char *dest;
    char sprbuffer[4];

  	data&=0xffff;

    switch (zone) {
        case 0x12: /* SPR */

            offset2=offset & ~0x02;

            offset2+=(bank<<20);

            if((offset2&0x7f)<64)
               offset2=(offset2&0xfff80)+((offset2&0x7f)<<1)+4;
            else
               offset2=(offset2&0xfff80)+((offset2&0x7f)<<1)-128;

            dest=&neogeo_spr_memory[offset2];
            blit_spr_resetcache();

            if (offset & 0x02) {
               /* second word */
			   *(u16*)(&dest[2])=(u16)data;
			   /* reformat sprite data */
               swab(dest, sprbuffer, sizeof(sprbuffer));
               extract8(sprbuffer, dest);
			} else {
			   /* first word */
			   *(u16*)(&dest[0])=(u16)data;
            }
            break;

        case 0x13: /* Z80 */
	  if (offset >= 0xf800)
            subcpu_memspace[(offset>>1) ^ 1]=(char)data;
            break;
        case 0x14: /* PCM */
            neogeo_pcm_memory[(offset>>1)+(bank<<19)]=(char)data;
            break;
        default:
         //sprintf(mem_str,"write_upload_word unimplemented zone %x\n",zone);
         //debug_log(mem_str);
			break;
    }
}


static void write_upload_dword(int offset, int data) {
    int zone = m68k_read_memory_8(0x10FEDA);
    int bank = m68k_read_memory_8(0x10FEDB);
    char *dest;
    char sprbuffer[4];

    switch (zone) {
        case 0x12: /* SPR */
            offset+=(bank<<20);

            if((offset&0x7f)<64)
               offset=(offset&0xfff80)+((offset&0x7f)<<1)+4;
            else
               offset=(offset&0xfff80)+((offset&0x7f)<<1)-128;

            dest=&neogeo_spr_memory[offset];
            swab((char*)&data, sprbuffer, sizeof(sprbuffer));
            extract8(sprbuffer, dest);

            blit_spr_resetcache();

            break;
        default:
         //sprintf(mem_str,"write_upload_dword unimplemented zone %x\n",zone);
         //debug_log(mem_str);
			break;
    }
}


static void write_memcard(int offset, int data) {
#ifdef DEBUG_MEMORY
if (trazando) printf("write_memcard(0x%X,0x%X)\n",offset,data); fflush(stdout);
#endif
    data&=0xff;
    neogeo_memorycard[(offset&0x3fff)>>1]=(char)data;

    /* signal that memory card has been written */
    memcard_write=3;
}

