/**** MEMORY DEFINITIONS for NEOCD emulator ****/

#ifndef	MEMORY_H
#define MEMORY_H

/*-- Exported Functions ------------------------------------------------------*/

void	initialize_memmap(void);

void	cpu_setOPbase(int);

unsigned int m68k_read_memory_8(unsigned int address);
unsigned int m68k_read_memory_16(unsigned int address);
unsigned int m68k_read_memory_32(unsigned int address);
void m68k_write_memory_8(unsigned int address, unsigned int value);
void m68k_write_memory_16(unsigned int address, unsigned int value);
void m68k_write_memory_32(unsigned int address, unsigned int value);

void    neogeo_sound_irq(int irq);
void	swab( const void* src1, const void* src2, int isize);

extern int watchdog_counter;
extern int memcard_write;
extern int irq2start, irq2pos;

#endif
