#ifndef _blit_h_
#define _blit_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void blit_init(void);
void blit_deinit(void);
void guDrawBuffer(u16* video_buffer,int src_w,int src_h,int src_pitch,int dst_w,int dst_h);
void blit_fix_resetcache();
void blit_spr_resetcache();

void blit_start();
void blit_finish(s32 sw,s32 sh,s32 dw,s32 dh);
void blit_drawfix(int x,int y,u16 *pal,u32 *text,u32 code,u32 colour);
void blit_start_fast(u16 col );
void blit_drawspr(s32 x,s32 y,s32 w,s32 h,u16 *pal,u32 *text,u32 code,u32 colour,u32 att);
void blit_finish_fast();
void blit_drawfix_key(int x,int y,u16 *pal,u8 *text,u32 code,u32 colour);

extern unsigned int __attribute__((aligned(16))) gulist[262144];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
