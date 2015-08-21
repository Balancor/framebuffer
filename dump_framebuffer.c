#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#define FRAMEBUFFER_DEVICE "/dev/fb0"

void dump_fix_screeninfo(struct fb_fix_screeninfo *finfo){
        printf("smem: %u\n", finfo->smem_len);
        printf("line_length: %u\n", finfo->line_length);
}

void dump_var_screeninof(struct fb_var_screeninfo *vinfo){
        printf("xres: %u\n", vinfo->xres);
        printf("yres: %u\n", vinfo->yres);

        printf("xres_virtual: %u\n", vinfo->xres_virtual);
        printf("yres_virtual: %u\n", vinfo->yres_virtual);

        printf("xoffset: %u\n", vinfo->xoffset);
        printf("yoffset: %u\n", vinfo->yoffset);
        printf("bpp: %u\n", vinfo->bits_per_pixel);

}


int main()
{
        int framebuffer_fd = -1;

        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;

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
        dump_var_screeninof(&vinfo);
        dump_fix_screeninfo(&finfo);

        close(framebuffer_fd);

    return -1;
}
