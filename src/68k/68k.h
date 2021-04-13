#ifndef INTERFACE_68K_H
#define INTERFACE_68K_H

#ifdef USE_FAME_CORE

#include "fame/fame.h"
#define _68K_REG_D0 M68K_REG_D0
#define _68K_REG_D1 M68K_REG_D1
#define _68K_REG_D2 M68K_REG_D2
#define _68K_REG_D3 M68K_REG_D3
#define _68K_REG_D4 M68K_REG_D4
#define _68K_REG_D5 M68K_REG_D5
#define _68K_REG_D6 M68K_REG_D6
#define _68K_REG_D7 M68K_REG_D7
#define _68K_REG_A0 M68K_REG_A0
#define _68K_REG_A1 M68K_REG_A1
#define _68K_REG_A2 M68K_REG_A2
#define _68K_REG_A3 M68K_REG_A3
#define _68K_REG_A4 M68K_REG_A4
#define _68K_REG_A5 M68K_REG_A5
#define _68K_REG_A6 M68K_REG_A6
#define _68K_REG_A7 M68K_REG_A7
#define _68K_REG_ASP M68K_REG_ASP
#define _68K_REG_PC M68K_REG_PC
#define _68K_REG_SR M68K_REG_SR

#define _68k_init m68k_init
#define _68k_reset m68k_reset
#define _68k_emulate m68k_emulate
#define _68k_get_pc m68k_get_pc
#define _68k_get_cpu_state m68k_get_cpu_state
#define _68k_fetch m68k_fetch
#define _68k_raise_irq m68k_raise_irq
#define _68k_lower_irq m68k_lower_irq
#define _68k_get_irq_vector m68k_get_irq_vector
#define _68k_change_irq_vector m68k_change_irq_vector
#define _68k_get_context_size m68k_get_context_size
#define _68k_get_context m68k_get_context
#define _68k_set_context m68k_set_context
#define _68k_get_register m68k_get_register
#define _68k_set_register m68k_set_register
#define _68k_get_cycles_counter m68k_get_cycles_counter
#define _68k_trip_cycles_counter m68k_trip_cycles_counter
#define _68k_control_cycles_counter m68k_control_cycles_counter
#define _68k_release_timeslice m68k_release_timeslice
#define _68k_add_cycles m68k_add_cycles
#define _68k_release_cycles m68k_release_cycles

#define _68k_interrupt(N_IRQ) (m68k_raise_irq(N_IRQ,M68K_AUTOVECTORED_IRQ))


#else

#include <malloc.h>
#include "c68k/types.h"
#include "c68k/c68k.h"
#include "c68k/m68k.h"
#include "c68k/cpu68k.h"

#define _68K_REG_D0 M68K_REG_D0
#define _68K_REG_D1 M68K_REG_D1
#define _68K_REG_D2 M68K_REG_D2
#define _68K_REG_D3 M68K_REG_D3
#define _68K_REG_D4 M68K_REG_D4
#define _68K_REG_D5 M68K_REG_D5
#define _68K_REG_D6 M68K_REG_D6
#define _68K_REG_D7 M68K_REG_D7
#define _68K_REG_A0 M68K_REG_A0
#define _68K_REG_A1 M68K_REG_A1
#define _68K_REG_A2 M68K_REG_A2
#define _68K_REG_A3 M68K_REG_A3
#define _68K_REG_A4 M68K_REG_A4
#define _68K_REG_A5 M68K_REG_A5
#define _68K_REG_A6 M68K_REG_A6
#define _68K_REG_A7 M68K_REG_A7
#define _68K_REG_ASP M68K_REG_SP
#define _68K_REG_PC M68K_REG_PC
#define _68K_REG_SR M68K_REG_SR


static __inline__  unsigned _68k_get_register(unsigned r)
{
	if (r<=M68K_REG_D7)
		return M68K_GetDReg(r);
	else if (r<=M68K_REG_A7)
		return M68K_GetAReg(r-M68K_REG_A0);
	else if (r==M68K_REG_SP)
		return M68K_GetSP();
	else if (r==M68K_REG_PC)
		return M68K_GetPC();
	return M68K_GetSR();
}

static __inline__ void _68k_set_register(unsigned r, unsigned d)
{
	if (r<=M68K_REG_D7)
		return M68K_SetDReg(r,d);
	else if (r<=M68K_REG_A7)
		return M68K_SetAReg(r-M68K_REG_A0,d);
	else if (r==M68K_REG_SP)
		return M68K_SetSP(d);
	else if (r==M68K_REG_PC)
		return M68K_SetPC(d);
	return M68K_SetSR(d);
}

static __inline__ unsigned __CPU68K_READ(unsigned d)
{
	unsigned ret;
	void *buff=calloc(64,1024);
	m68k_get_context(buff);
	ret=m68k_read_memory_16(d);
	free(buff);
	return ret;
}


#define _68k_init M68K_Init
#define _68k_reset M68K_Reset
#define _68k_emulate M68K_Exec
#define _68k_interrupt M68K_SetIRQ

#define _68k_get_pc M68K_GetPC
#define _68k_fetch(FDIR,FT) (__CPU68K_READ(FDIR))


#endif
#endif

