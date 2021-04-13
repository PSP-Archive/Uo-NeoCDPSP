#ifndef _IMAGEIO_H_
#define _IMAGEIO_H_

typedef struct {
	unsigned char r,g,b,a;
} COLOR;

typedef struct {
	int width;
	int height;
	int bit;
	void *pixels;
	int n_palette;
	COLOR* palette;
} IMAGE;

IMAGE* image_alloc(int width,int height,int bit);
void image_free(IMAGE* image);

IMAGE* load_png(int fd);
IMAGE* load_bmp(FILE *fd);
int save_bmp(const char *file,int width,int height,int bit,void *bits,int pitch);
void image_put(int x0,int y0,IMAGE* img,int fade);

#endif
