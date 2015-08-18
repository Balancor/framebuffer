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

enum colorFormat {
    COLOR_FORMAT_RGB565 = 1,
    COLOR_FORMAT_RGB888,
    COLOR_FORMAT_RGBA
};

struct shape_framebuffer_info {
    int framebuffer_fd;
    char* fbp;
    int initialized;
    long int screensize;
    struct fb_var_screeninfo *vinfo ;
    struct fb_fix_screeninfo *finfo ;

} shape_fb_info = {
    .framebuffer_fd = -1,
    .fbp = 0,
    .initialized = 0,
    .screensize = 0,
    .vinfo = 0,
    .finfo = 0
};

void dump_var_screeninfo(struct fb_var_screeninfo *vinfo){
    printf("resolution: %u x %u\n", vinfo->xres, vinfo->yres);
    printf("virtual-resolution: %u x %u\n", vinfo->xres_virtual, vinfo->yres_virtual);
    printf("xoffset: %u, xoffset: %u\n", vinfo->xoffset, vinfo->yoffset);
    printf("bits_per_pixel: %u\n", vinfo->bits_per_pixel);

}

void dump_fix_screeninfo(struct fb_fix_screeninfo *finfo){
    printf("smem_len: %lu\n", finfo->smem_len);
    printf("line_length: %u\n", finfo->line_length);
}
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
        location = (y + shape_fb_info.vinfo->yoffset) * (shape_fb_info.finfo->line_length) +
                   (x + shape_fb_info.vinfo->xoffset) * (shape_fb_info.vinfo->bits_per_pixel / 8);
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
    if(location < 0) {
        printf("Error: Cannot get the location\n");
        return;
    }
    char* fbp = shape_fb_info.fbp;
    int r  = 0, g = 0, b = 0;
    int alpha = 0xff;
    getColorFields(color, &r, &g, &b, &alpha, colorFormat);

//    printf("red: %d, green: %d, blue: %d\n", r, g, b);
    int bpp = shape_fb_info.vinfo->bits_per_pixel;
    if(bpp == 16){
//       printf("Screen Color Format RGB565\n");
       unsigned short int t = r << 16 |
                              g << 5  |
                              b;
     *((unsigned short int*)(fbp + location)) = t;
    } else if(bpp == 24) {
//        printf("Screen Color Format RGB888\n");
        *(fbp + location)     = 0xFF & b;
        *(fbp + location + 1) = 0xFF & g;
        *(fbp + location + 2) = 0xFF & r;
    } else if(bpp == 32){
//        printf("Screen Color Format RGBA\n");
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
        shape_fb_info->framebuffer_fd = framebuffer_fd;
        shape_fb_info->fbp = fbp;
        shape_fb_info->vinfo = &vinfo;
        shape_fb_info->finfo = &finfo;
        shape_fb_info->screensize = screensize;
        shape_fb_info->initialized = 1;
    } else {
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
void drawLine(int startx, int starty, int xoffset, int yoffset, int color){
    int x = 0, y = 0;
    for(x = startx; x <= startx + xoffset; x++)
        for(y = starty; y <=starty + yoffset; y++){
            setPixel(x, y, color, COLOR_FORMAT_RGB888);
        }
}
int main()
{
    if(!shape_fb_info.initialized){
        init_framebuffer_info(&shape_fb_info);
    }

//    setPixel(600, 200, 0x00FF00, COLOR_FORMAT_RGB888);
      dump_var_screeninfo(shape_fb_info.vinfo);
      dump_fix_screeninfo(shape_fb_info.finfo);
//    drawLine(400,200, 20, 20, 0x00FF00);
    if(shape_fb_info.initialized){
        free_framebuffer_info(&shape_fb_info);
    }

    return -1;
}
