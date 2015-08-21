#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#include "logs.h"
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
/*
void dump_var_screeninfo(struct fb_var_screeninfo *vinfo){
    INFO("resolution: %u x %u\n", vinfo->xres, vinfo->yres);
    INFO("virtual-resolution: %u x %u\n", vinfo->xres_virtual, vinfo->yres_virtual);
    INFO("xoffset: %u, xoffset: %u\n", vinfo->xoffset, vinfo->yoffset);
    INFO("bits_per_pixel: %u\n", vinfo->bits_per_pixel);

}

void dump_fix_screeninfo(struct fb_fix_screeninfo *finfo){
    INFO("smem_len: %lu\n", finfo->smem_len);
    INFO("line_length: %u\n", finfo->line_length);
}
*/
int getColorFormat(int color) {
    if(color <= MAX_COLOR_RGB565) {
        return COLOR_FORMAT_RGB565;
    } else if (color <= MAX_COLOR_RGB888) {
        return COLOR_FORMAT_RGB888;
    } else if (color <= MAX_COLOR_RGBA) {
        return COLOR_FORMAT_RGBA;
    } else {
        return -1;
    }
}

void getColorFields(int color, int* red, int* green, int* blue, int* alpha, int inputColorFormat){
    int colorFormat = inputColorFormat; //getColorFormat(color);
    if (colorFormat < 0){
        printf("Error: It is not a avaliable color\n");
        return ;
    }
    switch(colorFormat){
        case COLOR_FORMAT_RGB565:
//            printf("Color Format RGB565\n");
            *red   = (color >> 11) & RGB565_RED_MASK;
            *green = (color >>  5) & RGB565_GREEN_MASK;
            *blue  = color & RGB565_BLUE_MASK;
            alpha  = 0xFF;
            break;
        case COLOR_FORMAT_RGB888:
//            printf("color: 0x%6x, Color Format RGB888\n",color);
            *red   = (color >> 16) & RGB888_COLOR_MASK;
            *green = (color >> 8) & RGB888_COLOR_MASK;
            *blue  = color & RGB888_COLOR_MASK;
            alpha  = 0xFF;
//            printf("red: 0x%2x, green: 0x%2x, blue: 0x%2x\n", *red, *green, *blue);
            break;
        case COLOR_FORMAT_RGBA:
//            printf("Color Format RGBA\n");
            *red   = (color >> 24) & RGBA_COLOR_MASK;
            *green = (color >> 16) & RGBA_COLOR_MASK;
            *blue  = (color >> 8 ) & RGBA_COLOR_MASK;
            alpha  = color & RGBA_COLOR_MASK;
            break;
        default:
            break;

    }
}
/*
int RGB888ToRGB565(int rgb888_color){
    int rgb565_red = 0;
    int rgb565_green = 0;
    int rgb565_blue = 0;
    int rgb888_red = 0,  rgb888_green = 0, rgb888_blue = 0;
    int alpha = 0xFF;
    getColorFields(rgb888_color, &rgb888_red, &rgb888_green, &rgb888_blue,&alpha);
    rgb565_red = (int)((rgb888_red - 7) / 8);
    rgb565_green = (int)((rgb888_green - 3) / 4);
    rgb565_blue = (int)((rgb888_blue - 7) / 8);
    return (rgb565_red<<11) |
           (rgb565_green<<6)|
           (rgb565_blue);

}
*/

unsigned int getLocation(int x, int y){
    unsigned int location = -1;
    if(shape_fb_info.initialized){
        location = (y + shape_fb_info.yoffset - 1) * (shape_fb_info.line_size) +
                   (x + shape_fb_info.xoffset) * (shape_fb_info.bpp / 8);
    } else {
        printf("Error:Framebuffer has not been initialized\n");
    }
    return location;
}
void setPixel(int x, int y, int color, int colorFormat){
    if(!shape_fb_info.initialized){
        printf("Error:Framebuffer has not been initialized\n");
        return;
    }
    unsigned int location = getLocation(x, y);
    INFO("x: %d, y: %d, location: %lu\n", x, y, location);
    if(location < 0) {
        printf("Error: Cannot get the location\n");
        return;
    }
    char* fbp = shape_fb_info.fbp;
    int r  = 0, g = 0, b = 0;
    int alpha = 0xff;
    getColorFields(color, &r, &g, &b, &alpha, colorFormat);

    unsigned int bpp = shape_fb_info.bpp;
    INFO("bpp: %u\n", shape_fb_info.bpp);
    INFO("red: %d, green: %d, blue: %d, alpha: %d, bpp: %u\n", r, g, b, alpha, bpp);
    if(bpp == 16){
       INFO("Screen Color Format RGB565\n");
       unsigned short int t = r << 16 |
                              g << 5  |
                              b;
     *((unsigned short int*)(fbp + location)) = t;
    } else if(bpp == 24) {
        INFO("Screen Color Format RGB888\n");
        *(fbp + location)     = 0xFF & b;
        *(fbp + location + 1) = 0xFF & g;
        *(fbp + location + 2) = 0xFF & r;
    } else if(bpp == 32){
        INFO("Screen Color Format RGBA\n");
        *(fbp + location)     = 0xFF & b;
        *(fbp + location + 1) = 0xFF & g;
        *(fbp + location + 2) = 0xFF & r;
        *(fbp + location + 3) = 0xFF & alpha;
    }

};

void init_framebuffer_info(struct shape_framebuffer_info *shape_fb_info){
    if(!shape_fb_info->initialized){
        int framebuffer_fd = -1;

        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;

        char* fbp = 0;
        long int screensize = 0;
        framebuffer_fd = open(FRAMEBUFFER_DEVICE, O_RDWR);
        if(framebuffer_fd < 0){
            perror("Error: Canot open the framebuffer device : " FRAMEBUFFER_DEVICE );
            exit(-1);
        }

        if(ioctl(framebuffer_fd, FBIOGET_VSCREENINFO, &vinfo) == -1 ){
            perror("Error: Canot get variable screen infomation");
            exit(-2);
        }
        if(ioctl(framebuffer_fd, FBIOGET_FSCREENINFO, &finfo) == -1 ){
            perror("Error: Canot get fix screen infomation");
            exit(-3);
        }
        screensize = vinfo.yres * finfo.line_length;
        fbp = (char*)mmap(NULL, screensize,PROT_READ | PROT_WRITE,  MAP_SHARED, framebuffer_fd, 0);
        if(fbp == -1){
            perror("Error: Canot mmap");
            exit(-4);
        }
//        int line_size = vinfo.xres * vinfo.bits_per_pixel / 8;
        int line_size = finfo.line_length; 
        shape_fb_info->framebuffer_fd = framebuffer_fd;
        shape_fb_info->fbp = fbp;
        shape_fb_info->screensize = screensize;
        shape_fb_info->initialized = 1;
        shape_fb_info->line_size = line_size;
        shape_fb_info->bpp  = vinfo.bits_per_pixel;
        shape_fb_info->xres =vinfo.xres;
        shape_fb_info->yres =vinfo.yres;
        shape_fb_info->xres_virtual = vinfo.xres_virtual;
        shape_fb_info->yres_virtual = vinfo.yres_virtual;
        shape_fb_info->xoffset = vinfo.xoffset;
        shape_fb_info->yoffset = vinfo.yoffset;
    }else {
            printf("Havd initialized the framebuffer info\n");
            return;
    }
}
void free_framebuffer_info(struct shape_framebuffer_info *shape_fb_info){
    if(shape_fb_info->initialized){
        shape_fb_info->initialized = 0;
        shape_fb_info->screensize = 0;
        munmap(shape_fb_info->fbp, shape_fb_info->screensize);
        if(shape_fb_info->framebuffer_fd)
            close(shape_fb_info->framebuffer_fd);
    } else {
        printf("Had not initialized the framebuffer info\n");
        return;
    }

}
void drawLine_backup(int startx, int starty, int endx, int endy, int color){
    int dx = endx - startx;
    int dy = endy - starty;
    float k = 0.0, c = 0.0;
    k = (float)dx / (float)dy;
    c = (starty * dy - startx * dx) / dy;
    INFO("dx: %d, dy: %d, k: %.2f, c: %.2f\n", dx, dy, k, c);
    if(abs(k) < 1){
        int x = 0;
        float y = 0.0;
        for(x = startx; x < startx + dx; x++){
            y = k * x + c;
            if( (y - (int)y) >= 0.5) setPixel(x, (int)(y+1), color, COLOR_FORMAT_RGB888);
            else setPixel(x, (int)y, color, COLOR_FORMAT_RGB888);
        }
    } else {
        int y = 0;
        float x = 0.0;
        for(y = starty; y < starty + dy; y++){
            x = (y - c) / k;
            if(x - (int)x >= 0.5) setPixel((int)(x+1), y, color, COLOR_FORMAT_RGB888);
            else setPixel((int)x, y, color, COLOR_FORMAT_RGB888);
        }
    }
}
void drawLine(int startx, int starty, int endx, int endy, int color){
    int dx = endx - startx;
    int dy = endy - starty;
    int ux = ((dx > 0) << 1) - 1;
    int uy = ((dy > 0) << 1) - 1;
    int x = startx, y = starty, eps;

    eps = 0;
    dx = abs(dx);
    dy = abs(dy);
    if(dx > dy){
        for(x = startx; x != endx + ux; x+=ux){
            setPixel(x, y, color, COLOR_FORMAT_RGB888);
            eps += dy;
            if((eps << 1) >= dx){
                y += uy;
                eps -= dx;
            }
        }
    } else {
        for(y = starty; y != endy + uy; y+=uy){
            setPixel(x, y, color, COLOR_FORMAT_RGB888);
            eps += dx;
            if((eps << 1) >= dy){
                x += ux;
                eps -= dy;
            }
        }
    }
}
void drawCircle(int x, int y, int R, int color){
        int stepx, stepy, p;
        stepx = 0;
        stepy = R;
        p = 3 - (R << 1);
        for(;stepx <= stepy; stepx++){
            setPixel( stepx + x, stepy + y, color, COLOR_FORMAT_RGB888);
            setPixel(-stepx + x, stepy + y, color, COLOR_FORMAT_RGB888);
            setPixel( stepx + x,-stepy + y, color, COLOR_FORMAT_RGB888);
            setPixel(-stepx + x,-stepy + y, color, COLOR_FORMAT_RGB888);

            setPixel( stepy + x, stepx + y, color, COLOR_FORMAT_RGB888);
            setPixel(-stepy + x, stepx + y, color, COLOR_FORMAT_RGB888);
            setPixel( stepy + x,-stepx + y, color, COLOR_FORMAT_RGB888);
            setPixel(-stepy + x,-stepx + y, color, COLOR_FORMAT_RGB888);
            if(p < 0){
                p += ((stepx<<2) + 6);
            } else {
                p += (((stepx - stepy)<<2) + 10);
                stepy--;
            }
        }
};
int main()
{
    if(!shape_fb_info.initialized){
        init_framebuffer_info(&shape_fb_info);
    }
    log_init();

//   setPixel(600, 1, 0x00FF00, COLOR_FORMAT_RGB888);
//   setPixel(600, 2, 0x00FF00, COLOR_FORMAT_RGB888);
//   setPixel(600, 3, 0x00FF00, COLOR_FORMAT_RGB888);
//   setPixel(600, 4, 0x00FF00, COLOR_FORMAT_RGB888);
//    setPixel(1365, 767, 0x00FF00, COLOR_FORMAT_RGB888);
//      dump_var_screeninfo(shape_fb_info.vinfo);
//      dump_fix_screeninfo(shape_fb_info.finfo);
//    drawLine(600,300, 800, 500, 0x00FF00);
    drawCircle(500, 500, 50, 0x00FF00);
    log_close();
    if(shape_fb_info.initialized){
        free_framebuffer_info(&shape_fb_info);
    }

    return -1;
}
