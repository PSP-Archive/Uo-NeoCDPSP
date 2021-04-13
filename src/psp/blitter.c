#include <pspdisplay.h>

#include "psp.h"
#include <pspgu.h>
#include "video.h"

extern int psp_vsync;
extern s32 neogeo_frame_count;

/*static */unsigned int __attribute__((aligned(16))) gulist[262144];
#define SLICE_SIZE 64 // change this to experiment with different page-cache sizes
//static unsigned short __attribute__((aligned(16))) pixels[512*272];
int swap_buf;
struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};
/*struct Vertex
{
	float u, v;
	unsigned int color;
	float x,y,z;
};*/


#define SWIZZLE 0

#define FIX_CACHE_SIZE 1024

#define FIX_HASH_MASK 255
#define FIX_HASH_SIZE 256
#define FIX_MAX_SPRITES 40*28
#define FIX_CACHE_MAX_DELAY 4

#define SPR_CACHE_SIZE 2048
#define SPR_HASH_MASK 511
#define SPR_HASH_SIZE 512
#define SPR_MAX_SPRITES 4096
#define SPR_CACHE_MAX_DELAY 1

typedef struct tile_cache_s{
	u32 key;s32 used;
	u16 text_idx;
	struct tile_cache_s *next;
}tile_cache_t;


tile_cache_t *tile_fix_cache_ptr[FIX_HASH_SIZE];
tile_cache_t tile_fix_cache_data[FIX_CACHE_SIZE];
tile_cache_t *tile_fix_cache_first_free;


u16 *text_fix;
//u16 __attribute__((aligned(16))) text_fix[512*512*2];
u32 tile_fix_nb;
u16 tile_fix_x[FIX_MAX_SPRITES];
u16 tile_fix_y[FIX_MAX_SPRITES];
u16 tile_fix_idx[FIX_MAX_SPRITES];
u8 text_idx_map_fix[FIX_CACHE_SIZE];
int tile_fix_cached;
int tile_fix_cache_reset;
u32 video_pal_bank_fix;

tile_cache_t *tile_spr_cache_ptr[SPR_HASH_SIZE];
tile_cache_t tile_spr_cache_data[SPR_CACHE_SIZE];
tile_cache_t *tile_spr_cache_first_free;

u16 *text_spr1,*text_spr2;

u32 tile_spr_nb;
u16 tile_spr_x[SPR_MAX_SPRITES];
u16 tile_spr_y[SPR_MAX_SPRITES];
u8 tile_spr_w[SPR_MAX_SPRITES];
u8 tile_spr_h[SPR_MAX_SPRITES];
u16 tile_spr_idx[SPR_MAX_SPRITES];
u8 tile_spr_att[SPR_MAX_SPRITES];
u8 text_idx_map_spr[SPR_CACHE_SIZE];
int tile_spr_cached=0;
int tile_spr_cache_reset;
u32 video_pal_bank_spr;

// ask for a reset at next rendering pass
void blit_fix_resetcache(){
	tile_fix_cache_reset=1;
}

// ask for a reset at next rendering pass
void blit_spr_resetcache(){
	tile_spr_cache_reset=1;
}

// get tile with hashkey=key
// return texture index if found, 0 if not
INLINE s32 blit_fix_get_cache(int key){
	 tile_cache_t *p=tile_fix_cache_ptr[key&FIX_HASH_MASK];
	 for (;;){
	 	if (!p) return -1;
	 	if (p->key==key) {
	 		p->used=neogeo_frame_count;
	 		return p->text_idx;
	 	}
	 	p=p->next;
	 }
}
INLINE s32 blit_spr_get_cache(int key){
	 tile_cache_t *p=tile_spr_cache_ptr[key&SPR_HASH_MASK];
	 for (;;){
	 	if (!p) return -1;
	 	if (p->key==key) {
	 		p->used=neogeo_frame_count;
	 		return p->text_idx;
	 	}
	 	p=p->next;
	 }
}

// insert a new tile in cache
INLINE int blit_fix_insert_cache(u32 key){
	tile_cache_t *p=tile_fix_cache_ptr[key&FIX_HASH_MASK];
	tile_cache_t *q;
	//get first free cache entry
	q=tile_fix_cache_first_free;
	if (!q) {//error cache full
		debug_log("**fix cache full!");
		return -1;
	}
	//update first free
	tile_fix_cache_first_free=tile_fix_cache_first_free->next;

	//update new cached tile data
	q->next=NULL;
	q->key=key;
	q->used=neogeo_frame_count;


	if (!p) {//insert first
		tile_fix_cache_ptr[key&FIX_HASH_MASK]=q;
	}else{//insert at end
		for (;;){
			if (p->next) p=p->next;
			else {p->next=q;break;}
		}
	}
	return q->text_idx;
}
INLINE int blit_spr_insert_cache(u32 key){
	tile_cache_t *p=tile_spr_cache_ptr[key&SPR_HASH_MASK];
	tile_cache_t *q;
	//get first free cache entry
	q=tile_spr_cache_first_free;
	if (!q) {//error cache full
		debug_log("**spr cache full!");
		return -1;
	}
	//update first free
	tile_spr_cache_first_free=tile_spr_cache_first_free->next;

	//update new cached tile data
	q->next=NULL;
	q->key=key;
	q->used=neogeo_frame_count;


	if (!p) {//insert first
		tile_spr_cache_ptr[key&SPR_HASH_MASK]=q;
	}else{//insert at end
		for (;;){
			if (p->next) p=p->next;
			else {p->next=q;break;}
		}
	}
	return q->text_idx;
}


// clean used cached tile & recount total
INLINE void blit_fix_clean_cache(int delay){
	int i;
	tile_cache_t *p,*prev_p;

	debug_log("fix clean");
	/*{  char str[64];
		sprintf(str,"bef:%d",tile_fix_cached);
		debug_log(str);
	}*/

	for (i=0;i<FIX_HASH_SIZE;i++){
		prev_p=NULL;
		p=tile_fix_cache_ptr[i];
		while (p) {
			if ((neogeo_frame_count-p->used)>delay) {//delete entry
				tile_fix_cached--;
				if (!prev_p) {//free tile in first pos
					tile_fix_cache_ptr[i]=p->next;
					p->next=tile_fix_cache_first_free;
					tile_fix_cache_first_free=p;
					p=tile_fix_cache_ptr[i];
				}else{//free tile at n-th pos
					prev_p->next=p->next;
					p->next=tile_fix_cache_first_free;
					tile_fix_cache_first_free=p;
					p=prev_p->next;
				}
			}else{
				prev_p=p;
				p=p->next;
			}
		}
	}
	/*debug_log("clean fix cache");

	{  char str[64];
		sprintf(str,"aft:%d",tile_fix_cached);
		debug_log(str);
	}*/
}
INLINE void blit_spr_clean_cache(int delay){
	int i;
	tile_cache_t *p,*prev_p;

	/*{  char str[64];
		sprintf(str,"bef:%d",tile_fix_cached);
		debug_log(str);
	}*/

	for (i=0;i<SPR_HASH_SIZE;i++){
		prev_p=NULL;
		p=tile_spr_cache_ptr[i];
		while (p) {
			if ((neogeo_frame_count-p->used)>delay) {//delete entry
				tile_spr_cached--;
				if (!prev_p) {//free tile in first pos
					tile_spr_cache_ptr[i]=p->next;
					p->next=tile_spr_cache_first_free;
					tile_spr_cache_first_free=p;
					p=tile_spr_cache_ptr[i];
				}else{//free tile at n-th pos
					prev_p->next=p->next;
					p->next=tile_spr_cache_first_free;
					tile_spr_cache_first_free=p;
					p=prev_p->next;
				}
			}else{
				prev_p=p;
				p=p->next;
			}
		}
	}
	/*debug_log("clean spr cache");
	{  char str[64];
		sprintf(str,"aft:%d",tile_fix_cached);
		debug_log(str);
	}*/
}


// reset tile cache
INLINE void blit_reset_fix_cache(){
	int i;
	for (i=0;i<FIX_CACHE_SIZE-1;i++){
		tile_fix_cache_data[i].next=&tile_fix_cache_data[i+1];
	}
	tile_fix_cache_data[i].next=NULL;
	tile_fix_cache_first_free=&tile_fix_cache_data[0];
	memset(tile_fix_cache_ptr,0,sizeof(tile_cache_t*)*FIX_HASH_SIZE);
	tile_fix_cached=0;

}
INLINE void blit_reset_spr_cache(){
	int i;
	for (i=0;i<SPR_CACHE_SIZE-1;i++){
		tile_spr_cache_data[i].next=&tile_spr_cache_data[i+1];
	}
	tile_spr_cache_data[i].next=NULL;
	tile_spr_cache_first_free=&tile_spr_cache_data[0];
	memset(tile_spr_cache_ptr,0,sizeof(tile_cache_t*)*SPR_HASH_SIZE);
	tile_spr_cached=0;
}


//init the blitter
void blit_init(void){


	//init cache texture map location

  text_fix=(u16*)pgGetVramAddr(0,272*3);
	text_spr2=text_fix+(FIX_CACHE_SIZE*8*8);
	text_spr1=text_spr2+512*512;


	//init both cache
	int i;
	for (i=0;i<FIX_CACHE_SIZE-1;i++){
		tile_fix_cache_data[i].text_idx=i;
		tile_fix_cache_data[i].next=&tile_fix_cache_data[i+1];
	}
	tile_fix_cache_data[i].text_idx=i;
	tile_fix_cache_data[i].next=NULL;
	tile_fix_cache_first_free=&tile_fix_cache_data[0];
	memset(tile_fix_cache_ptr,0,sizeof(tile_cache_t*)*FIX_HASH_SIZE);
	tile_fix_cached=0;

	for (i=0;i<SPR_CACHE_SIZE-1;i++){
		tile_spr_cache_data[i].text_idx=i;
		tile_spr_cache_data[i].next=&tile_spr_cache_data[i+1];
	}
	tile_spr_cache_data[i].text_idx=i;
	tile_spr_cache_data[i].next=NULL;
	tile_spr_cache_first_free=&tile_spr_cache_data[0];
	memset(tile_spr_cache_ptr,0,sizeof(tile_cache_t*)*SPR_HASH_SIZE);
	tile_spr_cached=0;


	// setup GU
	sceGuInit();
	sceGuStart(0,gulist);

	swap_buf=1;
	sceGuDispBuffer(480,272,(void*)(FRAMESIZE*0),512);
	sceGuDrawBuffer(GU_PSM_5551,(void*)(FRAMESIZE*1),512);


	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);

	//sceGuDepthRange(SPR_CACHE_SIZE+FIX_CACHE_SIZE,0);

	sceGuEnable(GU_SCISSOR_TEST);

	sceGuAlphaFunc(GU_LEQUAL,0,0x1);
	sceGuEnable(GU_ALPHA_TEST);

	sceGuEnable(GU_TEXTURE_2D);

	//sceGuDepthRange(0,SPR_CACHE_SIZE+FIX_CACHE_SIZE);
	//sceGuDepthFunc(GU_GEQUAL);
	//sceGuEnable(GU_DEPTH_TEST);

	//sceGuClearDepth(0);
	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT/*|GU_DEPTH_BUFFER_BIT*/);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
}

void guDrawBuffer(u16* video_buffer,int src_w,int src_h,int src_pitch,int dst_w,int dst_h){
  unsigned int j,cx,cy;
  struct Vertex* vertices;

  cx=(480-dst_w)/2;
  cy=(272-dst_h)/2;

  sceGuStart(0,gulist);
  // clear screen

  sceGuOffset(2048 - (480/2),2048 - (272/2));
  sceGuViewport(2048,2048,480,272);
  sceGuScissor(cx,cy,dst_w,dst_h);

	sceGuDrawBufferList(GU_PSM_5551,(void*)(FRAMESIZE*swap_buf),512);
	if (!video_buffer) video_buffer=(u16*)pgGetVramAddr(8,272*2);

	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT);



	//sceGuDisable(GU_DEPTH_TEST);
	sceGuDisable(GU_ALPHA_TEST);

  sceGuTexMode(GU_PSM_5551,0,0,SWIZZLE);
  sceGuTexImage(0,src_pitch,src_pitch,src_pitch,video_buffer);
  sceGuTexFunc(GU_TFX_REPLACE,0);
  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
  sceGuTexScale(1.0f/src_pitch,1.0f/src_pitch); // scale UVs to 0..1
  sceGuTexOffset(0,0);
  sceGuAmbientColor(0xffffffff);



  for (j = 0; (j+SLICE_SIZE) < src_w; j = j+SLICE_SIZE) {
    vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

    vertices[0].u = j; vertices[0].v = 0;

    vertices[0].x = cx+j*dst_w/src_w; vertices[0].y = cy+0; vertices[0].z = 0;

    vertices[1].u = j+SLICE_SIZE; vertices[1].v = src_h;

    vertices[1].x = cx+(j+SLICE_SIZE)*dst_w/src_w; vertices[1].y = cy+dst_h; vertices[1].z = 0;

    //sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2,0,vertices);
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
  }
  if (j<src_w){
    vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

    vertices[0].u = j; vertices[0].v = 0;

    vertices[0].x = cx+j*dst_w/src_w; vertices[0].y = cy+0; vertices[0].z = 0;
    vertices[1].u = src_w; vertices[1].v = src_h;

    vertices[1].x = cx+dst_w; vertices[1].y = cy+dst_h; vertices[1].z = 0;

    //sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2,0,vertices);
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
  }

/*
  sceGuScissor(0,0,480,272);
  sceGuTexImage(0,512,512,512,text_spr1);
  sceGuTexScale(1.0f/512.0f,1.0f/512.0f); // scale UVs to 0..1
  vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  vertices[0].u = 0; vertices[0].v = 0;
  vertices[0].x = 0; vertices[0].y = 272-512/4; vertices[0].z = 0;
  vertices[0].color = 0xffffffff;
  vertices[1].u = 512; vertices[1].v = 512;
  vertices[1].x = 512/4; vertices[1].y = 272; vertices[1].z = 0;
  vertices[1].color = 0xffffffff;
  sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2,0,vertices);

  sceGuTexImage(0,512,512,512,text_spr2);
  vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  vertices[0].u = 0; vertices[0].v = 0;
  vertices[0].x = 512/4+8; vertices[0].y = 272-512/4; vertices[0].z = 0;
  vertices[0].color = 0xffffffff;
  vertices[1].u = 512; vertices[1].v = 512;
  vertices[1].x = 512/4+8+512/4; vertices[1].y = 272; vertices[1].z = 0;
  vertices[1].color = 0xffffffff;
  sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2,0,vertices);
*/
  sceGuFinish();
  sceGuSync(0,0);
  if (psp_vsync) sceDisplayWaitVblankStart();
  sceGuSwapBuffers();
	swap_buf^=1;

}


void blit_deinit(){
  sceGuTerm();
}


void blit_start(u16 col ){


	if (tile_fix_cache_reset){
		blit_reset_fix_cache();
		tile_fix_cache_reset=0;
	}// else blit_fix_clean_cache(FIX_CACHE_MAX_DELAY);

	if (tile_spr_cache_reset){
		blit_reset_spr_cache();
		tile_spr_cache_reset=0;
	}// else blit_spr_clean_cache(SPR_CACHE_MAX_DELAY);

	tile_fix_nb=0;
	tile_spr_nb=0;

	video_pal_bank_fix=(video_paletteram_pc==video_palette_bank0_pc?0:0x80000000);
	video_pal_bank_spr=(video_paletteram_pc==video_palette_bank0_pc?0:0x80000000);
	sceGuStart(0,gulist);


	sceGuOffset(2048 - (320/2),2048 - (224/2));
	sceGuViewport(2048,2048,320,224);
	sceGuScissor(0,0,320,224);

	//sceGuDepthBuffer((void*)(FRAMESIZE*swap_buf),320);//512);
	sceGuDrawBufferList(GU_PSM_5551,(void*)(FRAMESIZE*2),512);


	// clear screen
	int r,g,b;
	b=(col>>10)&31;
	g=(col>>5)&31;
	r=col&31;
	sceGuClearColor((r<<19)|(g<<11)|(b<<3));
	//sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT/*|GU_DEPTH_BUFFER_BIT*/);
	//sceGuDepthFunc(GU_GEQUAL);
	//sceGuEnable(GU_DEPTH_TEST);
	sceGuEnable(GU_ALPHA_TEST);
}



void blit_finish(s32 sw,s32 sh,s32 dw,s32 dh){
	 //sceGuClutLoad(2,pal); // upload 8 entries by block
  int i,u,v;
  s32 cx,cy;


  cx=0;//(480-dw);
  cy=0;//(272-dh);

  sceGuTexMode(GU_PSM_5551,0,0,SWIZZLE);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA); // NOTE: this enables reads of the alpha-component from the texture, otherwise blend/test won't work
	//sceGuTexFunc(GU_TFX_REPLACE,0);
	//sceGuTexFilter(GU_NEAREST,GU_NEAREST);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);


  sceGuTexOffset(0,0);
  sceGuAmbientColor(0xffffffff);
  sceGuTexScale(1.0f/512.0f,1.0f/512.0f);

  struct Vertex* vertices_spr = (struct Vertex*)sceGuGetMemory(tile_spr_nb*2 * sizeof(struct Vertex));
  if (!vertices_spr){
  	debug_log("not enough guram spr");
  }else{
  	struct Vertex* vertices_spr_tmp=vertices_spr;
  	int total_spr;

  	total_spr=0;
  	int cur_text=0;
  	sceGuTexImage(0,512,512,512,text_spr1);
  	for (i=0;i<tile_spr_nb;i++){
  		v=(tile_spr_idx[i]>>5)<<4;
  		//if (v&512) continue;
  		if ((v&512)!=cur_text){
  			cur_text^=512;
  			//sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*total_spr,0,vertices_spr);
  			total_spr=0;
  			vertices_spr=vertices_spr_tmp;
  			sceGuTexImage(0,512,512,512,(cur_text?text_spr2:text_spr1));
  		}
  		v&=511;

  		u=(tile_spr_idx[i]&31)<<4;
  		if (tile_spr_att[i]&0x1) {//flipx
	  		vertices_spr_tmp[0].u = u+16;
  			vertices_spr_tmp[1].u = u;
  		}else{
	  		vertices_spr_tmp[0].u = u;
  			vertices_spr_tmp[1].u = u+16;
    	}
	    if (tile_spr_att[i]&0x2) {//flipy
  			vertices_spr_tmp[0].v = v+16;
  	  	vertices_spr_tmp[1].v = v;
  		}else{
	  		vertices_spr_tmp[0].v = v;
  	  	vertices_spr_tmp[1].v = v+16;
    	}
  		vertices_spr_tmp[0].x = cx+tile_spr_x[i]; vertices_spr_tmp[0].y = cy+tile_spr_y[i]; vertices_spr_tmp[0].z = (i+1);
  		vertices_spr_tmp[0].color = 0xffff;
  		vertices_spr_tmp[1].x = cx+(tile_spr_x[i]+tile_spr_w[i]); vertices_spr_tmp[1].y = cy+(tile_spr_y[i]+tile_spr_h[i]); vertices_spr_tmp[1].z = (i+1);
  		vertices_spr_tmp[1].color = 0xffff;
  		vertices_spr_tmp+=2;
  		total_spr++;
  	}
  	if (total_spr) //sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*total_spr,0,vertices_spr);


  	/*vertices_spr=vertices_spr_tmp;
  	total_spr=0;
  	sceGuTexImage(0,512,512,512,text_spr2);
  	for (i=0;i<tile_spr_nb;i++){
  		v=(tile_spr_idx[i]>>5)<<4;
  		if (!(v&512)) continue;
  		v&=511;
  		u=(tile_spr_idx[i]&31)<<4;
  		if (tile_spr_att[i]&0x1) {//flipx
	  		vertices_spr_tmp[0].u = u+16;
  			vertices_spr_tmp[1].u = u;
  		}else{
	  		vertices_spr_tmp[0].u = u;
  			vertices_spr_tmp[1].u = u+16;
    	}
	    if (tile_spr_att[i]&0x2) {//flipy
  			vertices_spr_tmp[0].v = v+16;
  	  	vertices_spr_tmp[1].v = v;
  		}else{
	  		vertices_spr_tmp[0].v = v;
  	  	vertices_spr_tmp[1].v = v+16;
    	}
  		vertices_spr_tmp[0].x = cx+tile_spr_x[i]; vertices_spr_tmp[0].y = cy+tile_spr_y[i]; vertices_spr_tmp[0].z = (i+1);
  		vertices_spr_tmp[0].color = 0xffffffff;
  		vertices_spr_tmp[1].x = cx+tile_spr_x[i]+tile_spr_w[i]; vertices_spr_tmp[1].y = cy+tile_spr_y[i]+tile_spr_h[i]; vertices_spr_tmp[1].z = (i+1);
  		vertices_spr_tmp[1].color = 0xffffffff;
  		vertices_spr_tmp+=2;
  		total_spr++;
  	}
  	sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  	*/
	}

  // sceGuTexFunc(GU_TFX_REPLACE,0);
  sceGuTexImage(0,512,512,512,text_fix);
  struct Vertex* vertices_fix = (struct Vertex*)sceGuGetMemory(tile_fix_nb*2 * sizeof(struct Vertex));
  if (!vertices_fix){
  	debug_log("not enough guram fix");
  }else{
  	struct Vertex* vertices_fix_tmp=vertices_fix;
  	for (i=0;i<tile_fix_nb;i++){
  		u=(tile_fix_idx[i]&63)<<3;
  		v=(tile_fix_idx[i]>>6)<<3;
	  	vertices_fix_tmp[0].u = u; vertices_fix_tmp[0].v = v;
  		vertices_fix_tmp[0].x = cx+tile_fix_x[i]; vertices_fix_tmp[0].y = cy+tile_fix_y[i]; vertices_fix_tmp[0].z = FIX_CACHE_SIZE+SPR_CACHE_SIZE;
	  	vertices_fix_tmp[0].color = 0xffff;
  		vertices_fix_tmp[1].u = u+8; vertices_fix_tmp[1].v = v+8;
  		vertices_fix_tmp[1].x = cx+tile_fix_x[i]+8; vertices_fix_tmp[1].y = cy+tile_fix_y[i]+8; vertices_fix_tmp[1].z = FIX_CACHE_SIZE+SPR_CACHE_SIZE;
  		vertices_fix_tmp[1].color = 0xffff;
  		vertices_fix_tmp+=2;
  	}
  	//sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*tile_fix_nb,0,vertices_fix);
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*tile_fix_nb,0,vertices_fix);
	}

sceGuFinish();
sceGuSync(0,0);


guDrawBuffer(NULL,304,224,512,dw,dh);

  /*if (psp_vsync) sceDisplayWaitVblankStart();
  sceGuSwapBuffers();*/
#ifndef RELEASE
  pgPrintHex(0,0,(31<<10)|(31<<5)|31,tile_fix_cached,1);
  pgPrintHex(10,0,(31<<10)|(31<<5)|31,tile_fix_nb,1);
  pgPrintHex(20,0,(31<<10)|(31<<5)|31,tile_spr_cached,1);
  pgPrintHex(30,0,(31<<10)|(31<<5)|31,tile_spr_nb,1);
#endif

}



void blit_start_fast2(u16 col ){
	s32 cx,cy;
  cx=(480-320)/2;
  cy=(272-224)/2;



  if (tile_fix_cache_reset){
		blit_reset_fix_cache();
		tile_fix_cache_reset=0;
	}// else blit_fix_clean_cache(FIX_CACHE_MAX_DELAY);

	if (tile_spr_cache_reset){
		blit_reset_spr_cache();
		tile_spr_cache_reset=0;
	}//else blit_spr_clean_cache(SPR_CACHE_MAX_DELAY);


	tile_fix_nb=0;
	tile_spr_nb=0;

	video_pal_bank_fix=(video_paletteram_pc==video_palette_bank0_pc?0:0x800000);
	video_pal_bank_spr=(video_paletteram_pc==video_palette_bank0_pc?0:0x800000);
	sceGuStart(0,gulist);

	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	sceGuScissor(cx+8,cy,304,224);

	sceGuDrawBufferList(GU_PSM_5551,(void*)(FRAMESIZE*swap_buf),512);
	//sceGuDepthBuffer((void*)(FRAMESIZE*2),320/*512*/);

	// clear screen
	int r,g,b;
	b=(col>>10)&31;
	g=(col>>5)&31;
	r=col&31;
	sceGuClearColor((r<<19)|(g<<11)|(b<<3));
	//sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT/*|GU_DEPTH_BUFFER_BIT*/);
	//sceGuDepthFunc(GU_GEQUAL);
	//sceGuEnable(GU_DEPTH_TEST);
	sceGuEnable(GU_ALPHA_TEST);
}



void blit_finish_fast2(){
  int i,u,v;
  s32 cx,cy;
  cx=(480-320)/2;
  cy=(272-224)/2;

  sceGuTexMode(GU_PSM_5551,0,0,SWIZZLE);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA); // NOTE: this enables reads of the alpha-component from the texture, otherwise blend/test won't work
	//sceGuTexFunc(GU_TFX_REPLACE,0);
	//sceGuTexFilter(GU_NEAREST,GU_NEAREST);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);


  sceGuTexOffset(0,0);
  sceGuAmbientColor(0xffffffff);
  sceGuTexScale(1.0f/512.0f,1.0f/512.0f);

  struct Vertex* vertices_spr = (struct Vertex*)sceGuGetMemory(tile_spr_nb*2 * sizeof(struct Vertex));
  if (!vertices_spr){
  	debug_log("not enough guram spr");
  }else{
  	struct Vertex* vertices_spr_tmp=vertices_spr;
  	int total_spr;

  	total_spr=0;
  	int cur_text=0;
  	sceGuTexImage(0,512,512,512,text_spr1);
  	for (i=0;i<tile_spr_nb;i++){
  		v=(tile_spr_idx[i]>>5)<<4;
  		//if (v&512) continue;
  		if ((v&512)!=cur_text){
  			cur_text^=512;
  			//sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*total_spr,0,vertices_spr);
  			total_spr=0;
  			vertices_spr=vertices_spr_tmp;
  			sceGuTexImage(0,512,512,512,(cur_text?text_spr2:text_spr1));
  		}
  		v&=511;

  		u=(tile_spr_idx[i]&31)<<4;
  		if (tile_spr_att[i]&0x1) {//flipx
	  		vertices_spr_tmp[0].u = u+16;
  			vertices_spr_tmp[1].u = u;
  		}else{
	  		vertices_spr_tmp[0].u = u;
  			vertices_spr_tmp[1].u = u+16;
    	}
	    if (tile_spr_att[i]&0x2) {//flipy
  			vertices_spr_tmp[0].v = v+16;
  	  	vertices_spr_tmp[1].v = v;
  		}else{
	  		vertices_spr_tmp[0].v = v;
  	  	vertices_spr_tmp[1].v = v+16;
    	}
  		vertices_spr_tmp[0].x = cx+tile_spr_x[i]; vertices_spr_tmp[0].y = cy+tile_spr_y[i]; vertices_spr_tmp[0].z = (i+1);
  		vertices_spr_tmp[0].color = 0xffff;
  		vertices_spr_tmp[1].x = cx+(tile_spr_x[i]+tile_spr_w[i]); vertices_spr_tmp[1].y = cy+(tile_spr_y[i]+tile_spr_h[i]); vertices_spr_tmp[1].z = (i+1);
  		vertices_spr_tmp[1].color = 0xffff;
  		vertices_spr_tmp+=2;
  		total_spr++;
  	}
  	if (total_spr) //sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*total_spr,0,vertices_spr);
  	//sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_32BITF,GE_CT_5551,0,GE_MT_32BITF,0,0,0,0,GE_BM_2D),total_spr * 2,0,vertices_spr);


  	/*vertices_spr=vertices_spr_tmp;
  	total_spr=0;
  	sceGuTexImage(0,512,512,512,text_spr2);
  	for (i=0;i<tile_spr_nb;i++){
  		v=(tile_spr_idx[i]>>5)<<4;
  		if (!(v&512)) continue;
  		u=(tile_spr_idx[i]&31)<<4;
  		v&=511;
  		if (tile_spr_att[i]&0x1) {//flipx
	  		vertices_spr_tmp[0].u = u+16;
  			vertices_spr_tmp[1].u = u;
  		}else{
	  		vertices_spr_tmp[0].u = u;
  			vertices_spr_tmp[1].u = u+16;
    	}
	    if (tile_spr_att[i]&0x2) {//flipy
  			vertices_spr_tmp[0].v = v+16;
  	  	vertices_spr_tmp[1].v = v;
  		}else{
	  		vertices_spr_tmp[0].v = v;
  	  	vertices_spr_tmp[1].v = v+16;
    	}
  		vertices_spr_tmp[0].x = cx+tile_spr_x[i]; vertices_spr_tmp[0].y = cy+tile_spr_y[i]; vertices_spr_tmp[0].z = (i+1);
  		vertices_spr_tmp[0].color = 0xffffffff;
  		vertices_spr_tmp[1].x = cx+tile_spr_x[i]+tile_spr_w[i]; vertices_spr_tmp[1].y = cy+tile_spr_y[i]+tile_spr_h[i]; vertices_spr_tmp[1].z = (i+1);
  		vertices_spr_tmp[1].color = 0xffffffff;
  		vertices_spr_tmp+=2;
  		total_spr++;
  	}
  	sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  	*/
	}

  sceGuTexImage(0,512,512,512,text_fix);
  struct Vertex* vertices_fix = (struct Vertex*)sceGuGetMemory(tile_fix_nb*2 * sizeof(struct Vertex));
  if (!vertices_fix){
  	debug_log("not enough guram fix");
  }else{
  	struct Vertex* vertices_fix_tmp=vertices_fix;
  	for (i=0;i<tile_fix_nb;i++){
  		u=(tile_fix_idx[i]&63)<<3;
  		v=(tile_fix_idx[i]>>6)<<3;
	  	vertices_fix_tmp[0].u = u; vertices_fix_tmp[0].v = v;
  		vertices_fix_tmp[0].x = cx+tile_fix_x[i]; vertices_fix_tmp[0].y = cy+tile_fix_y[i]; vertices_fix_tmp[0].z = FIX_CACHE_SIZE+SPR_CACHE_SIZE;
	  	vertices_fix_tmp[0].color = 0xffff;
  		vertices_fix_tmp[1].u = u+8; vertices_fix_tmp[1].v = v+8;
  		vertices_fix_tmp[1].x = cx+tile_fix_x[i]+8; vertices_fix_tmp[1].y = cy+tile_fix_y[i]+8; vertices_fix_tmp[1].z = FIX_CACHE_SIZE+SPR_CACHE_SIZE;
  		vertices_fix_tmp[1].color = 0xffff;
  		vertices_fix_tmp+=2;
  	}
  	//sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*tile_fix_nb,0,vertices_fix);
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*tile_fix_nb,0,vertices_fix);
	}

	/*
  sceGuScissor(0,0,480,272);
  sceGuTexImage(0,512,512,512,text_spr1);
  struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  vertices[0].u = 0; vertices[0].v = 0;
  vertices[0].x = 0; vertices[0].y = 272-512/4; vertices[0].z = 0;
  vertices[0].color = 0xffffffff;
  vertices[1].u = 512; vertices[1].v = 512;
  vertices[1].x = 512/4; vertices[1].y = 272; vertices[1].z = 0;
  vertices[1].color = 0xffffffff;
  sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2,0,vertices);
  sceGuTexImage(0,512,512,512,text_spr2);

  vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  vertices[0].u = 0; vertices[0].v = 0;
  vertices[0].x = 512/4+8; vertices[0].y = 272-512/4; vertices[0].z = 0;
  vertices[0].color = 0xffffffff;
  vertices[1].u = 512; vertices[1].v = 512;
  vertices[1].x = 512/4+8+512/4; vertices[1].y = 272; vertices[1].z = 0;
  vertices[1].color = 0xffffffff;
  sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2,0,vertices);
  */


	sceGuFinish();
	sceGuSync(0,0);


  if (psp_vsync) sceDisplayWaitVblankStart();
  sceGuSwapBuffers();
	swap_buf^=1;
#ifndef RELEASE
  pgPrintHex(0,0,(31<<10)|(31<<5)|31,tile_fix_cached,1);
  pgPrintHex(10,0,(31<<10)|(31<<5)|31,tile_fix_nb,1);
  pgPrintHex(20,0,(31<<10)|(31<<5)|31,tile_spr_cached,1);
  pgPrintHex(30,0,(31<<10)|(31<<5)|31,tile_spr_nb,1);
#endif

}


void blit_spritemode(int sw,int sh,int dw,int dh){
	//sceGuSpriteMode(sw,sh,dw,dh);
}




void blit_drawfix(int x,int y,u16 *pal,u32 *text,u32 code,u32 colour){
	int u,v,tile_key,i;
	s32 idx;

  if (tile_fix_nb==FIX_MAX_SPRITES) return;



  tile_key=code|(colour<<8)|video_pal_bank_fix; //0x1ffff;

  idx=blit_fix_get_cache(tile_key);
  if (idx<0) {
  	if (tile_fix_cached==(FIX_CACHE_SIZE-1)) {
  		//to much tile cached, clean
  		blit_fix_clean_cache(0);
  		//still to much, exit
  		if (tile_fix_cached==(FIX_CACHE_SIZE-1)) return;
  	}

  	idx=blit_fix_insert_cache(tile_key);
  	u=(idx&63)<<3;
  	v=(idx>>6)<<3;
  	tile_fix_cached++;


  	int mydword,col;u16 *dest=&(text_fix[u+(v<<9)]);
   	for (i=8;i;i--){
	   	mydword  = *text++;
			col = (mydword>> 0)&0x0f; dest[0] = pal[col];
			col = (mydword>> 4)&0x0f; dest[1] = pal[col];
			col = (mydword>> 8)&0x0f; dest[2] = pal[col];
			col = (mydword>>12)&0x0f; dest[3] = pal[col];
			col = (mydword>>16)&0x0f; dest[4] = pal[col];
			col = (mydword>>20)&0x0f; dest[5] = pal[col];
			col = (mydword>>24)&0x0f; dest[6] = pal[col];
			col = (mydword>>28)&0x0f; dest[7] = pal[col];

			dest += 512;
  	}
  }
  tile_fix_idx[tile_fix_nb]=idx;
  tile_fix_x[tile_fix_nb]=x;
  tile_fix_y[tile_fix_nb]=y;
  tile_fix_nb++;


}

void blit_drawfix_key(int x,int y,u16 *pal,u8 *text,u32 code,u32 colour){
	int u,v,tile_key,i;
	s32 idx;

  if (tile_fix_nb==FIX_MAX_SPRITES) return;



  tile_key=(code-0x300)|(colour<<8)|0x1000; //0x1ffff;

  idx=blit_fix_get_cache(tile_key);
  if (idx<0) {
  	if (tile_fix_cached==(FIX_CACHE_SIZE-1)) {
  		//to much tile cached, clean
  		blit_fix_clean_cache(0);
  		//still to much, exit
  		if (tile_fix_cached==(FIX_CACHE_SIZE-1)) return;
  	}

  	idx=blit_fix_insert_cache(tile_key);
  	u=(idx&63)<<3;
  	v=(idx>>6)<<3;
  	tile_fix_cached++;


  	u16 *dest=&(text_fix[u+(v<<9)]);
   	for (i=8;i;i--){
	  memcpy(dest,text,8*2);
	  text += 8*2;
	  dest += 512;
  	}
  }
  tile_fix_idx[tile_fix_nb]=idx;
  tile_fix_x[tile_fix_nb]=x;
  tile_fix_y[tile_fix_nb]=y;
  tile_fix_nb++;


}

void blit_drawspr(s32 x,s32 y,s32 w,s32 h,u16 *pal,u32 *text,u32 code,u32 colour,u32 att){
	s32 u,v,tile_key,i,idx;
	if (x <= -8) return;
	if (w<=0 || h<=0) return;


	if (tile_spr_nb==SPR_MAX_SPRITES) {
		debug_log("spr overrun");
		return;
	}

  tile_key=code|(colour<<16)|video_pal_bank_spr; //0xffffff;
  idx=blit_spr_get_cache(tile_key);
  if (idx<0) {
  	if (tile_spr_cached==(SPR_CACHE_SIZE-1)) {
  			//to much tile cached, clean
  			blit_spr_clean_cache(0);
  			//tile_spr_cache_reset=1;
  			//still to much, exit
  			if (tile_spr_cached==(SPR_CACHE_SIZE-1)) {debug_log("sprcache full");return;}
  	}

  	idx=blit_spr_insert_cache(tile_key);
  	tile_spr_cached++;


  	u=(idx&31)<<4;
  	v=(idx>>5)<<4;


  	u32 mydword,col;u16 *dest;
  	if (v<511) dest=&(text_spr1[u|(v<<9)]);
  	else dest=&(text_spr2[u|((v&511)<<9)]);
   	for (i=16;i;i--){
	   	mydword  = *text++;
			col = (mydword>> 0)&0x0f; dest[0] = pal[col];
			col = (mydword>> 4)&0x0f; dest[1] = pal[col];
			col = (mydword>> 8)&0x0f; dest[2] = pal[col];
			col = (mydword>>12)&0x0f; dest[3] = pal[col];
			col = (mydword>>16)&0x0f; dest[4] = pal[col];
			col = (mydword>>20)&0x0f; dest[5] = pal[col];
			col = (mydword>>24)&0x0f; dest[6] = pal[col];
			col = (mydword>>28)&0x0f; dest[7] = pal[col];
			mydword  = *text++;
			col = (mydword>> 0)&0x0f; dest[8] = pal[col];
			col = (mydword>> 4)&0x0f; dest[9] = pal[col];
			col = (mydword>> 8)&0x0f; dest[10] = pal[col];
			col = (mydword>>12)&0x0f; dest[11] = pal[col];
			col = (mydword>>16)&0x0f; dest[12] = pal[col];
			col = (mydword>>20)&0x0f; dest[13] = pal[col];
			col = (mydword>>24)&0x0f; dest[14] = pal[col];
			col = (mydword>>28)&0x0f; dest[15] = pal[col];

			dest += 512;
  	}
  }
  tile_spr_idx[tile_spr_nb]=idx;
  tile_spr_x[tile_spr_nb]=x;
  tile_spr_y[tile_spr_nb]=y;
  tile_spr_w[tile_spr_nb]=w;
  tile_spr_h[tile_spr_nb]=h;
  tile_spr_att[tile_spr_nb]=att;
  tile_spr_nb++;

}




void blit_start_fast(u16 col ){


	if (tile_fix_cache_reset){
		blit_reset_fix_cache();
		tile_fix_cache_reset=0;
	}// else blit_fix_clean_cache(FIX_CACHE_MAX_DELAY);

	if (tile_spr_cache_reset){
		blit_reset_spr_cache();
		tile_spr_cache_reset=0;
	}// else blit_spr_clean_cache(SPR_CACHE_MAX_DELAY);

	tile_fix_nb=0;
	tile_spr_nb=0;

	video_pal_bank_fix=(video_paletteram_pc==video_palette_bank0_pc?0:0x80000000);
	video_pal_bank_spr=(video_paletteram_pc==video_palette_bank0_pc?0:0x80000000);
	sceGuStart(0,gulist);


	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	sceGuScissor((480-304)/2,(272-224)/2,304,224);

	//sceGuDepthBuffer((void*)(FRAMESIZE*swap_buf),320);//512);
	sceGuDrawBufferList(GU_PSM_5551,(void*)(FRAMESIZE*swap_buf),512);


	// clear screen
	int r,g,b;
	b=(col>>10)&31;
	g=(col>>5)&31;
	r=col&31;
	sceGuClearColor((r<<19)|(g<<11)|(b<<3));
	//sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT/*|GU_DEPTH_BUFFER_BIT*/);
	//sceGuDepthFunc(GU_GEQUAL);
	//sceGuEnable(GU_DEPTH_TEST);
	sceGuEnable(GU_ALPHA_TEST);
}



void blit_finish_fast(){
	 //sceGuClutLoad(2,pal); // upload 8 entries by block
  int i,u,v;
  s32 cx,cy;


  cx=(480-320)/2;
  cy=(272-224)/2;

  sceGuTexMode(GU_PSM_5551,0,0,SWIZZLE);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA); // NOTE: this enables reads of the alpha-component from the texture, otherwise blend/test won't work
	//sceGuTexFunc(GU_TFX_REPLACE,0);
	//sceGuTexFilter(GU_NEAREST,GU_NEAREST);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);


  sceGuTexOffset(0,0);
  sceGuAmbientColor(0xffffffff);
  sceGuTexScale(1.0f/512.0f,1.0f/512.0f);

  struct Vertex* vertices_spr = (struct Vertex*)sceGuGetMemory(tile_spr_nb*2 * sizeof(struct Vertex));
  if (!vertices_spr){
  	debug_log("not enough guram spr");
  }else{
  	struct Vertex* vertices_spr_tmp=vertices_spr;
  	int total_spr;

  	total_spr=0;
  	int cur_text=0;
  	sceGuTexImage(0,512,512,512,text_spr1);
  	for (i=0;i<tile_spr_nb;i++){
  		v=(tile_spr_idx[i]>>5)<<4;
  		//if (v&512) continue;
  		if ((v&512)!=cur_text){
  			cur_text^=512;
  			//sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*total_spr,0,vertices_spr);
  			total_spr=0;
  			vertices_spr=vertices_spr_tmp;
  			sceGuTexImage(0,512,512,512,(cur_text?text_spr2:text_spr1));
  		}
  		v&=511;

  		u=(tile_spr_idx[i]&31)<<4;
  		if (tile_spr_att[i]&0x1) {//flipx
	  		vertices_spr_tmp[0].u = u+16;
  			vertices_spr_tmp[1].u = u;
  		}else{
	  		vertices_spr_tmp[0].u = u;
  			vertices_spr_tmp[1].u = u+16;
    	}
	    if (tile_spr_att[i]&0x2) {//flipy
  			vertices_spr_tmp[0].v = v+16;
  	  	vertices_spr_tmp[1].v = v;
  		}else{
	  		vertices_spr_tmp[0].v = v;
  	  	vertices_spr_tmp[1].v = v+16;
    	}
  		vertices_spr_tmp[0].x = cx+tile_spr_x[i]; vertices_spr_tmp[0].y = cy+tile_spr_y[i]; vertices_spr_tmp[0].z = (i+1);
  		vertices_spr_tmp[0].color = 0xffff;
  		vertices_spr_tmp[1].x = cx+(tile_spr_x[i]+tile_spr_w[i]); vertices_spr_tmp[1].y = cy+(tile_spr_y[i]+tile_spr_h[i]); vertices_spr_tmp[1].z = (i+1);
  		vertices_spr_tmp[1].color = 0xffff;
  		vertices_spr_tmp+=2;
  		total_spr++;
  	}
  	if (total_spr) //sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*total_spr,0,vertices_spr);


  	/*vertices_spr=vertices_spr_tmp;
  	total_spr=0;
  	sceGuTexImage(0,512,512,512,text_spr2);
  	for (i=0;i<tile_spr_nb;i++){
  		v=(tile_spr_idx[i]>>5)<<4;
  		if (!(v&512)) continue;
  		v&=511;
  		u=(tile_spr_idx[i]&31)<<4;
  		if (tile_spr_att[i]&0x1) {//flipx
	  		vertices_spr_tmp[0].u = u+16;
  			vertices_spr_tmp[1].u = u;
  		}else{
	  		vertices_spr_tmp[0].u = u;
  			vertices_spr_tmp[1].u = u+16;
    	}
	    if (tile_spr_att[i]&0x2) {//flipy
  			vertices_spr_tmp[0].v = v+16;
  	  	vertices_spr_tmp[1].v = v;
  		}else{
	  		vertices_spr_tmp[0].v = v;
  	  	vertices_spr_tmp[1].v = v+16;
    	}
  		vertices_spr_tmp[0].x = cx+tile_spr_x[i]; vertices_spr_tmp[0].y = cy+tile_spr_y[i]; vertices_spr_tmp[0].z = (i+1);
  		vertices_spr_tmp[0].color = 0xffffffff;
  		vertices_spr_tmp[1].x = cx+tile_spr_x[i]+tile_spr_w[i]; vertices_spr_tmp[1].y = cy+tile_spr_y[i]+tile_spr_h[i]; vertices_spr_tmp[1].z = (i+1);
  		vertices_spr_tmp[1].color = 0xffffffff;
  		vertices_spr_tmp+=2;
  		total_spr++;
  	}
  	sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*total_spr,0,vertices_spr);
  	*/
	}

  sceGuTexImage(0,512,512,512,text_fix);
  struct Vertex* vertices_fix = (struct Vertex*)sceGuGetMemory(tile_fix_nb*2 * sizeof(struct Vertex));
  if (!vertices_fix){
  	debug_log("not enough guram fix");
  }else{
  	struct Vertex* vertices_fix_tmp=vertices_fix;
  	for (i=0;i<tile_fix_nb;i++){
  		u=(tile_fix_idx[i]&63)<<3;
  		v=(tile_fix_idx[i]>>6)<<3;
	  	vertices_fix_tmp[0].u = u; vertices_fix_tmp[0].v = v;
  		vertices_fix_tmp[0].x = cx+tile_fix_x[i]; vertices_fix_tmp[0].y = cy+tile_fix_y[i]; vertices_fix_tmp[0].z = FIX_CACHE_SIZE+SPR_CACHE_SIZE;
	  	vertices_fix_tmp[0].color = 0xffff;
  		vertices_fix_tmp[1].u = u+8; vertices_fix_tmp[1].v = v+8;
  		vertices_fix_tmp[1].x = cx+tile_fix_x[i]+8; vertices_fix_tmp[1].y = cy+tile_fix_y[i]+8; vertices_fix_tmp[1].z = FIX_CACHE_SIZE+SPR_CACHE_SIZE;
  		vertices_fix_tmp[1].color = 0xffff;
  		vertices_fix_tmp+=2;
  	}
  	//sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2*tile_fix_nb,0,vertices_fix);
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*tile_fix_nb,0,vertices_fix);
	}

sceGuFinish();
sceGuSync(0,0);


//guDrawBuffer(NULL,304,224,512,dw,dh);

  if (psp_vsync) sceDisplayWaitVblankStart();
  sceGuSwapBuffers();
  swap_buf^=1;
#ifndef RELEASE
  pgPrintHex(0,0,(31<<10)|(31<<5)|31,tile_fix_cached,1);
  pgPrintHex(10,0,(31<<10)|(31<<5)|31,tile_fix_nb,1);
  pgPrintHex(20,0,(31<<10)|(31<<5)|31,tile_spr_cached,1);
  pgPrintHex(30,0,(31<<10)|(31<<5)|31,tile_spr_nb,1);
#endif

}
