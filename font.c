#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "font.h"

#define MAX_HEADER 512

void readEntry(const char* data,struct TableEntry *tableEntry, int offset){
    tableEntry->tag[0] = data[offset];
    tableEntry->tag[1] = data[offset + 1];
    tableEntry->tag[2] = data[offset + 2];
    tableEntry->tag[3] = data[offset + 3];
    tableEntry->checksum = 
            (data[offset + 4] & 0xFF) << 24 |
            (data[offset + 5] & 0xFF) << 16 |
            (data[offset + 6] & 0xFF) << 8  |
            (data[offset + 7] & 0xFF); 

    tableEntry->offset= 
            (data[offset + 8] & 0xFF) << 24 |
            (data[offset + 9] & 0xFF) << 16 |
            (data[offset + 10] & 0xFF) << 8  |
            (data[offset + 11] & 0xFF);

//    printf("data[20]: 0x%x, \t data[21]: 0x%x, \t data[22]: 0x%x, \t data[23]: 0x%x\n", data[20], data[21], data[22], data[23]);
//    printf("offset: 0x%x\n", tableEntry.offset);
    tableEntry->length = 
           (data[offset + 12] & 0xFF) << 24 |
           (data[offset + 13] & 0xFF) << 16 |
           (data[offset + 14] & 0xFF) << 8  |
           (data[offset + 15] & 0xFF); 
}

int initFontInfo(){
    int fd = -1;
    unsigned long fileSize = 0;
    fd = open(FONT_DIRECTORY "/" FONT_FILE, O_RDONLY);
    if(fd < 0){
        printf("Error: Cannot open the font file\n");
        return -1;
    }
    if (lseek(fd, 0 , SEEK_SET) < 0){
        printf("Error: Cannot seek to head\n");
        return -2;
    }
    fileSize = lseek(fd, 0, SEEK_END);
    if(fileSize <= 0){
        printf("Error: Cannot get filesize");
        return -2;
    }
    lseek(fd, 0, SEEK_SET);

    int headerLength = sizeof(struct TableEntry) + sizeof(struct TableDirectory);
    printf("headerLength: %d\n", headerLength);

    char *data =(char*)malloc(MAX_HEADER);

    read(fd, data, MAX_HEADER);

    struct TableDirectory tableDirectory;
    tableDirectory.sfntVersion = 
            (data[0] & 0xFF) << 24 |
            (data[1] & 0xFF) << 16 |
            (data[2] & 0xFF) << 8  |
            (data[3] & 0xFF);
    tableDirectory.numTables = 
            (data[4] & 0xFF) << 8 |
            (data[5] & 0xFF);
    tableDirectory.searchRange = 
            (data[6] & 0xFF) << 8 |
            (data[7] & 0xFF);
    tableDirectory.entrySelector = 
            (data[8] & 0xFF) << 8 |
            (data[9] & 0xFF);
    tableDirectory.rangeShift = 
            (data[10] & 0xFF) << 8 |
            (data[11] & 0xFF);

    printf("TableDirectory: \n");
    printf("\t sfntVersion: %d\n", tableDirectory.sfntVersion);
    printf("\t numTables: %u\n", tableDirectory.numTables);
    printf("\t searchRange: %u\n", tableDirectory.searchRange);
    printf("\t entrySelector: %u\n", tableDirectory.entrySelector);
    printf("\t rangeShift: %u\n",tableDirectory.rangeShift);

    struct TableEntry tableEntry;
    int i = 0;
    for(i = 0; i < tableDirectory.numTables; i++){
        readEntry(data, &tableEntry, 12 + i*sizeof(struct TableEntry));
        printf("TableEntry: \n");
        printf("\t tag: %c%c%c%c\n",
                        tableEntry.tag[0], tableEntry.tag[1],
                        tableEntry.tag[2], tableEntry.tag[3]);
        printf("\t checksum: %u\n", tableEntry.checksum);
        printf("\t offset: %u\n", tableEntry.offset);
        printf("\t length: %u\n", tableEntry.length);
    }
    free(data);
    close(fd);
}


int main()
{
    initFontInfo();
    printf("sizeof(unsigned long): %d\n", sizeof(unsigned long));
    printf("sizeof(unsigned shoar): %d\n", sizeof(unsigned short));
    printf("sizeof(int)： %d\n", sizeof(int));
    return 0;
}
