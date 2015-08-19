#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "logs.h"

void log_init(){
    int fd = -1;
    fd = open(LOG_FILE,O_RDWR | O_CREAT, 0666);
    if(fd <= 0){
        printf("Cannot open the log file\n ");
    }
    logInfo.logFd = fd;
    logInfo.logOpened = 1;

};
void write_msg(const char* msg, int length){
    if(lseek(logInfo.logFd, 0, SEEK_END) == -1){
        return;
    }
    if(write(logInfo.logFd, msg, length) != length){
        return;
    }
}
void log_v(const char* msg){
    const char* LOG_TAG = 'V';
    int msg_length = strlen(msg);

    char* tem = (char*)malloc(msg_length + 3);
    tem[0] = 'V';
    tem[1] = ':';
    tem[2] = ' ';
    memcpy(tem+3, msg, msg_length);
    write_msg(tem, msg_length + 3);
    free(tem);

};
void log_i(const char* msg);
void log_d(const char* msg);
void log_w(const char* msg);
void log_e(const char* msg);
void log_close(){
    if(logInfo.logOpened){
        logInfo.logOpened = 0;
    }
    if(logInfo.logFd){
        logInfo.logFd = -1;
        close(logInfo.logFd);
    }
};
