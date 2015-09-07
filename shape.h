#ifndef __SHAPE_H__
#define __SHAPE_H__
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#define FRAMEBUFFER_DEVICE "/dev/fb0"

#define MAX_COLOR_RGB565  2<<15
#define MAX_COLOR_RGB888  2<<23
#define MAX_COLOR_RGBA    2<<31

#define RGBA(r, g, b, a) ((r<<24) | (g<<16) | (b<<8) |(a))
#define RGB888(r,g,b) ((r<<16) | (g<<8) |(b))
#define RGB565(r,g,b) ((r<<11) | (g << 5) | (b))

#define RGBA_COLOR_MASK 0x000000FF

#define RGB888_COLOR_MASK   0x0000FF

#define RGB565_RED_MASK   0xF8
#define RGB565_GREEN_MASK 0x07E4
#define RGB565_BLUE_MASK  0x001F

typedef struct {
    int x;
    int y;
}Point;
/*
void setPixel(Point* point){
    printf("Coord(%d, %d)\n", point->x, point->y);
}
*/
int isSamePoint(Point *p1, Point *p2){
    return ((p1->x == p2->x) && (p1->y == p2->y))?
        1 : 0;
}

int isValidPoint(Point* p){
    return ((p->x > 3) && (p->y > 3)) ? 1 : 0;
}

enum colorFormat {
    COLOR_FORMAT_RGB565 = 1,
    COLOR_FORMAT_RGB888,
    COLOR_FORMAT_RGBA
};

struct shape_framebuffer_info {
    char* fbp;
    int framebuffer_fd;
    int initialized;
    int line_size;
    long int screensize;
    unsigned int bpp;
    unsigned int xres;
    unsigned int yres;
    unsigned int xres_virtual;
    unsigned int yres_virtual;
    unsigned int xoffset;
    unsigned int yoffset;

} shape_fb_info = {
    .fbp = 0,
    .framebuffer_fd = -1,
    .initialized = 0,
    .line_size = 0,
    .screensize = 0,
    .bpp = -1,
    .xres = 0,
    .yres = 0,
    .xres_virtual = 0,
    .yres_virtual = 0,
    .xoffset = 0,
    .yoffset = 0
};
int getColorFormat(int color);

void getColorFields(int color, int* red, int* green, int* blue, int* alpha, int inputColorFormat);

unsigned int getLocation(int x, int y);
void setPixel(int x, int y, int color, int colorFormat);

void init_framebuffer_info(struct shape_framebuffer_info *shape_fb_info);
void free_framebuffer_info(struct shape_framebuffer_info *shape_fb_info);
void drawLine_backup(int startx, int starty, int endx, int endy, int color);
void drawLine(int startx, int starty, int endx, int endy, int color);
void drawCircle(int x, int y, int R, int color);
void drawRectangle(int leftTopX, int leftTopY, int rightBottomX, int rightBottomY, int color);
void bezier2(Point *start, Point *end, Point* control1, Point* control2, int shift);
#endif
