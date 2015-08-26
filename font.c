#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "font.h"
#define MAX_HEADER 512

char *data;
typedef struct _TableEntryNode{
    struct TableEntry tableEntry;
    struct ListNode listNode;
}TableEntryNode;


struct TableDirectory tableDirectory;
TableEntryNode *tableEntryNode;
struct ListNode tableEntryList;

typedef struct _CmapSubtableEntryNode {
    CmapEntry cmapSubtableEntry;
    struct ListNode listNode;
    unsigned int offsetToBeginFile;
}CmapSubtableNode;

CmapSubtableNode *cmapSubtableNode;
struct ListNode cmapSubtableEntryList;

void dumpTableEntry(struct TableEntry *tableEntry){
    printf("TableEntry:\n");
    printf("\t tag: %c%c%c%c\n", tableEntry->tag[0], tableEntry->tag[1],
                                 tableEntry->tag[2], tableEntry->tag[3]);
    printf("\t checksum: %d\n", tableEntry->checksum);
    printf("\t offset: %d\n", tableEntry->offset);
    printf("\t length: %d\n", tableEntry->length);
}
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

    data =(char*)malloc(fileSize);
    if(!data){
        printf("Error: Cannot malloc the memory\n");
    }
    if( fileSize != read(fd, data, fileSize)) {
        printf("Error: Cannot read the content of the font file\n");
    }

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

    initListNode(&tableEntryList);
    tableEntryNode = malloc(sizeof(TableEntryNode) * tableDirectory.numTables);

    int i = 0;
    for(i = 0; i < tableDirectory.numTables; i++){
        readEntry(data, &(tableEntryNode[i].tableEntry), 12 + i*sizeof(struct TableEntry));
        addTailListNode(&tableEntryList, &(tableEntryNode[i].listNode));
    }
    close(fd);
}

struct TableEntry* getTableEntry(const char* tag){
    TableEntryNode *tempTableEntryNode = 0;
    struct ListNode *node;
    int found = 0;
    list_for_each(node, &tableEntryList){
        tempTableEntryNode = listEntry(node, TableEntryNode, listNode);
        if(!strncmp(tempTableEntryNode->tableEntry.tag, tag, 4)){
            found = 1;
            break;
        }
    }
    if(found) return &(tempTableEntryNode->tableEntry);
}

void readCmapSubtables(){
    struct TableEntry *tempTableEntry;
    tempTableEntry = getTableEntry("cmap");
    if(tempTableEntry == NULL) return;
    char* tablePtr = data + tempTableEntry->offset;
    char* endTablePrt = tablePtr + tempTableEntry->length;

    CmapHeader cmapHeader;
    cmapHeader.tableVersion = (tablePtr[0] & 0xFF) << 8 |
                              (tablePtr[1] & 0xFF);
    cmapHeader.numOfTable =   (tablePtr[2] & 0xFF) << 8 |
                              (tablePtr[3] & 0xFF);

    initListNode(&cmapSubtableEntryList);
    int subTableLength = sizeof(CmapEntry);

    cmapSubtableNode = (CmapSubtableNode *)malloc(sizeof(CmapSubtableNode) * cmapHeader.numOfTable);

    if(!cmapSubtableNode){
        printf("Error: Cannot get enough memory!\n");
    }

    int i = 0;
    CmapEntry cmapEntry;
    char* subTableEntryPtr = tablePtr + 4;
    for (i = 0; i < cmapHeader.numOfTable; i++){
        cmapSubtableNode[i].offsetToBeginFile = tempTableEntry->offset;
        cmapSubtableNode[i].cmapSubtableEntry.platformId =
                                         (subTableEntryPtr[subTableLength * i] & 0xFF) << 8 |
                                         (subTableEntryPtr[subTableLength * i + 1] & 0xFF);

        cmapSubtableNode[i].cmapSubtableEntry.encodingId =
                                         (subTableEntryPtr[subTableLength * i + 2] & 0xFF) << 8 |
                                         (subTableEntryPtr[subTableLength * i + 3] & 0xFF);

        cmapSubtableNode[i].cmapSubtableEntry.offset     =
                                         (subTableEntryPtr[subTableLength * i + 4] & 0xFF) << 24 |
                                         (subTableEntryPtr[subTableLength * i + 5] & 0xFF) << 16 |
                                         (subTableEntryPtr[subTableLength * i + 6] & 0xFF) << 8 |
                                         (subTableEntryPtr[subTableLength * i + 7] & 0xFF);
        addTailListNode(&cmapSubtableEntryList , &(cmapSubtableNode[i].listNode));
    }
}

CmapSubtableNode* getCmapEntry(unsigned short platformId, unsigned short encodingId){
    if(platformId < 0 || encodingId < 0)return (CmapSubtableNode*)0;
    CmapSubtableNode *tempCmapSubtableNode;
    struct ListNode *node;
    list_for_each(node, &cmapSubtableEntryList){
        tempCmapSubtableNode = listEntry(node, CmapSubtableNode, listNode);
        if ( (tempCmapSubtableNode->cmapSubtableEntry.platformId == platformId) &&
             (tempCmapSubtableNode->cmapSubtableEntry.encodingId == encodingId)){
            break;
        }
    }
    return tempCmapSubtableNode;
}

static inline unsigned short
readUnsignedShort(const char* contentPtr){
    unsigned short temp  = ((*(contentPtr++)) & 0xFF) << 8;
                   temp |= ((*(contentPtr++)) & 0xFF);
    return temp;
}
void readEncodingTable(unsigned short platformId, unsigned short encodingId){
    if(platformId < 0 || encodingId < 0)return;
    CmapSubtableNode *tempCmapSubtableNode = getCmapEntry(platformId, encodingId);
    if(tempCmapSubtableNode == NULL) return;
    unsigned int encodingTableOffset = tempCmapSubtableNode->offsetToBeginFile + tempCmapSubtableNode->cmapSubtableEntry.offset;
    char* encodingTablePtr = data + encodingTableOffset;
    unsigned short format = 0;
    format  = ((*(encodingTablePtr++)) & 0xFF) << 8; 
    format |= ((*(encodingTablePtr++)) & 0xFF);
//    format = ((*(encodingTablePtr)) & 0xFF) << 8 |
//             ((*(++encodingTablePtr)) & 0xFF);
//    format = ((encodingTablePtr[0]) & 0xFF) << 8 |
//             ((encodingTablePtr[1]) & 0xFF);
    printf("format: %u\n", format);
    switch(format){
        case CMAP_SUBTABLE_FORMAT_BYTE:
            {
                CmapByteEncodingTable cmapByteEncodingTable;
                cmapByteEncodingTable.format = format;
                cmapByteEncodingTable.length = readUnsignedShort(encodingTablePtr);
                cmapByteEncodingTable.language = readUnsignedShort(encodingTablePtr);
                int i = 0;
                for(i = 0; i < 256; i++){
                    cmapByteEncodingTable.glyIdArray[i] = (*(encodingTablePtr++) & 0xFF);
                }
                printf("length: %d\n", cmapByteEncodingTable.length);
                printf("language: %d\n", cmapByteEncodingTable.language);
                break;
            }
        case CMAP_SUBTABLE_FORMAT_HIGH_BYTE:
            {
                printf("CmapHighbyteMappingTable empty\n");
                break;
            }
        case CMAP_SUBTABLE_FORMAT_SEGMENT:
            {
                printf("CmapSegmentMappingToDelta empty\n");
                break;
            }
        case CMAP_SUBTABLE_FORMAT_TRIMMED:
            {
                printf("CmapTrimmedTable empty\n");
                break;
            }
        case CMAP_SUBTABLE_FORMAT_MIXED:
            {
                break;
            }
        case CMAP_SUBTABLE_FORMAT_TRIMMED_ARRAY:
            {
                break;
            }
        case CMAP_SUBTABLE_FORMAT_SEGMENT_COVERAGE:
            {
                break;
            }
        case CMAP_SUBTABLE_FORMAT_MANY_TO_ONE:
            {
                break;
            }
        case CMAP_SUBTABLE_FORMAT_UNICODE:
            {
                break;
            }
        default:
            {
                break;
            }
    }
}

int main()
{
    TableEntryNode *tempTableEntryNode;
    CmapSubtableNode *tempCmapSubtableNode;
    struct ListNode *node;
    initFontInfo();
    readCmapSubtables();


    unsigned short platformId = -1, encodingId = -1;
    list_for_each(node, &cmapSubtableEntryList){
        tempCmapSubtableNode = listEntry(node, CmapSubtableNode, listNode);
//        printf("tempCmapSubtableNode: offsetToBeginFile: %d\n", tempCmapSubtableNode->offsetToBeginFile);
//        printf("CmapEntry: \n");
        platformId = tempCmapSubtableNode->cmapSubtableEntry.platformId;
        encodingId = tempCmapSubtableNode->cmapSubtableEntry.encodingId;
//        printf("\t platformId: %u\n", platformId);
//        printf("\t encodingId: %u\n", encodingId);
//        printf("\t offset: %u\n", tempCmapSubtableNode->cmapSubtableEntry.offset);
        readEncodingTable(platformId, encodingId);

    }

    int i  = 10;
//    printf("i++: %d\n", i++);
//    printf("++i: %d\n", ++i);
    free(cmapSubtableNode);
    free(tableEntryNode);
    free(data);
    return 0;
}
