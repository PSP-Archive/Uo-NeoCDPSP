
#ifdef __cplusplus
extern "C" {
#endif
#ifndef TIMER_H
#define TIMER_H

// The interface tries to be similar to the one used by mame
// The internals are not taken from mame, though.

#define TIME_IN_HZ(hz)        (1.0 / (double)(hz))
#define TIME_IN_USEC(us)      ((double)(us) * (1.0 / 1000000.0))
#define TIME_IN_NSEC(us)      ((double)(us) * (1.0 / 1000000000.0))

void setup_z80_frame(u32 cpu,u32 cycles);
void execute_z80_audio_frame(void);
void *timer_adjust(double duration, int param, double period, void (*callback)(int));

#define timer_set(duration, param, callback) \
  timer_adjust(duration, param, 0, callback)
void timer_remove(void *timer);
void reset_timers(void);
int execute_one_z80_audio_frame(u32 frame);
void triger_timers(void);
void update_timers(void);
void z80_irq_handler(int irq);
double emu_get_time(void);
double pos_in_frame(void);
s32 get_min_cycles(u32 frame);
void execute_z80_audio_frame_with_nmi(int nb);

#endif

#ifdef __cplusplus
}
#endif
