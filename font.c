#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "font.h"
#include "utils.h"
#define MAX_HEADER 512

typedef struct _TableEntryNode{
    struct TableEntry tableEntry;
    struct ListNode listNode;
}TableEntryNode;


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

    tableEntry->length =
           (data[offset + 12] & 0xFF) << 24 |
           (data[offset + 13] & 0xFF) << 16 |
           (data[offset + 14] & 0xFF) << 8  |
           (data[offset + 15] & 0xFF);
}

int initFontInfo(){
    TableEntryNode *tableEntryNode;
    TableEntryNode *tempTableEntryNode;
    struct ListNode tableEntryList;
    struct ListNode *node;

    initListNode(&tableEntryList);

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


    tableEntryNode = malloc(sizeof(TableEntryNode) * tableDirectory.numTables);

    int i = 0;
    for(i = 0; i < tableDirectory.numTables; i++){
        readEntry(data, &(tableEntryNode[i].tableEntry), 12 + i*sizeof(struct TableEntry));
        addTailListNode(&tableEntryList, &(tableEntryNode[i].listNode));
    }

    printf("TableEntry: \n");
    list_for_each(node, &tableEntryList){
        tempTableEntryNode = listEntry(node, TableEntryNode, listNode);
        printf("\t tag: %c%c%c%c\n",
                        tempTableEntryNode->tableEntry.tag[0], tempTableEntryNode->tableEntry.tag[1],
                        tempTableEntryNode->tableEntry.tag[2], tempTableEntryNode->tableEntry.tag[3]);
        printf("\t checksum: %u\n", tempTableEntryNode->tableEntry.checksum);
        printf("\t offset: %u\n", tempTableEntryNode->tableEntry.offset);
        printf("\t length: %u\n", tempTableEntryNode->tableEntry.length);
    }

    free(tableEntryNode);
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
