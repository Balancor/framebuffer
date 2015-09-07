#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/cdefs.h>
#include <stdarg.h>

#include "logs.h"
static int log_fd = -1;
static int log_level = LOG_DEFAULT_LEVEL;

int log_get_level(void){
    return log_level;
}

void log_set_level(int level){
    log_level = level;
}

void log_init(){
    int fd = -1;
    if(access(LOG_DIRE, F_OK) < 0){
        if(mkdir(LOG_DIRE, 0777) < 0){
            printf("Error: Cannot creat the directory\n");
        }
    }
    fd = open(LOG_DIRE "/" LOG_FILE,O_RDWR | O_CREAT, 0666);
    if(fd <= 0){
        printf("Cannot open the log file\n ");
    }
    log_fd = fd;

};
#define LOG_BUF_MAX 512
void log_vwrite(int level, const char* fmt, va_list ap){
    char buf[LOG_BUF_MAX];

    if(level > log_level) return;
    if(log_fd < 0) log_init();
    if(log_fd < 0) return;

    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX -1] = 0;

    if(lseek(log_fd, 0, SEEK_END) == -1){
        return;
    }
    write(log_fd, buf, strlen(buf));
}
void log_write(int level, const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    log_vwrite(level, fmt, ap);
    va_end(ap);
}

void log_close(){
    if(log_fd) close(log_fd);
};
/*
int main(){
    log_init();
    int integer = 10;
    log_write(3, "Hello, world, int: %d\n", integer);
    log_close();
    return 0;
}
*/
