#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "font.h"
#include "dump.h"
#define MAX_HEADER 512

char *data;

struct TableDirectory tableDirectory;
TableEntryNode *tableEntryNode;
struct ListNode tableEntryList;

CmapSubtableNode *cmapSubtableNode;
struct ListNode cmapSubtableEntryList;

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

static inline short
readShort(const char* contentPtr){
    short temp  = (contentPtr[0] & 0xFF) << 8 |
                  (contentPtr[1] & 0xFF);
    return (temp & 0xFFFF);
}

static inline unsigned short
readUnsignedShort(const char* contentPtr){
    unsigned short temp  = (contentPtr[0] & 0xFF) << 8 |
                           (contentPtr[1] & 0xFF);
    return (temp & 0xFFFF);
}
static inline unsigned int
readUnsignedInt(const char* contentPtr){
    unsigned int temp  = (contentPtr[0] & 0xFF) << 24 |
                         (contentPtr[1] & 0xFF) << 16 |
                         (contentPtr[2] & 0xFF) << 8  |
                         (contentPtr[3] & 0xFF);
    return (temp & 0xFFFFFFFF);
};
void readEncodingTable(unsigned short platformId, unsigned short encodingId){
    if(platformId < 0 || encodingId < 0)return;
    CmapSubtableNode *tempCmapSubtableNode = getCmapEntry(platformId, encodingId);
    if(tempCmapSubtableNode == NULL) return;

    unsigned int encodingTableOffset = tempCmapSubtableNode->offsetToBeginFile + tempCmapSubtableNode->cmapSubtableEntry.offset;
    char* encodingTablePtr = data + encodingTableOffset;
    unsigned short format = 0;
    format  = (encodingTablePtr[0] & 0xFF) << 8 |
              (encodingTablePtr[1] & 0xFF);
    printf("format: %u\n", format);
    switch(format){
        case CMAP_SUBTABLE_FORMAT_BYTE: // 0
            {
                CmapByteEncodingTable cmapByteEncodingTable;
                cmapByteEncodingTable.format = format;
                cmapByteEncodingTable.length = readUnsignedShort(encodingTablePtr + 2);
                cmapByteEncodingTable.language = readUnsignedShort(encodingTablePtr + 4);
                int i = 0;
                for(i = 0; i < 256; i++){
                    cmapByteEncodingTable.glyIdArray[i] = (*(encodingTablePtr++) & 0xFF);
                }
                break;
            }
        case CMAP_SUBTABLE_FORMAT_HIGH_BYTE: // 2
            {
                CmapHighbyteMappingTable cmapByteEncodingTable;
                break;
            }
        case CMAP_SUBTABLE_FORMAT_SEGMENT: // 4
            {
                unsigned short segCount = 0;
                CmapSegmentMappingToDelta cmapSegment;
                cmapSegment.format = format;
                cmapSegment.length = readUnsignedShort(encodingTablePtr + 2);
                cmapSegment.language = readUnsignedShort(encodingTablePtr + 4);
                cmapSegment.segCountX2 = readUnsignedShort(encodingTablePtr + 6);
                segCount = cmapSegment.segCountX2 >> 1;
                cmapSegment.searchRange = readUnsignedShort(encodingTablePtr + 8);
                cmapSegment.entrySelector = readUnsignedShort(encodingTablePtr + 10);
                cmapSegment.rangeShift = readUnsignedShort(encodingTablePtr + 12);
                int i = 0;
                int tempOffset = 0;
                cmapSegment.endCount = (short*)malloc(sizeof(short) * segCount);
                for(i = 0; i < segCount; i++){
                    tempOffset = 14 + i * 2;
                    cmapSegment.endCount[i] = readShort(encodingTablePtr + tempOffset);
                    //printf("endCount[%d]: 0x%04x\n",i, cmapSegment.endCount[i]);
                }
                cmapSegment.reservedPad = readUnsignedShort(encodingTablePtr + tempOffset + 2);

                tempOffset = tempOffset + 4;
                cmapSegment.startCount = (short*)malloc(sizeof(short) * segCount);
                for(i = 0; i < segCount; i++){
                    tempOffset += i * 2;
                    cmapSegment.startCount[i] = readShort(encodingTablePtr + tempOffset);
                }

                tempOffset = tempOffset + 2;
                cmapSegment.idDelta = (short*)malloc(sizeof(short) * segCount);
                for(i = 0; i < segCount; i++){
                    tempOffset += i * 2;
                    cmapSegment.idDelta[i] = readShort(encodingTablePtr + tempOffset);
                }

                int j = 0;
                initListNode(&(cmapSegment.glyphIndexArray));
                tempOffset = tempOffset + 2;
                //dump(encodingTablePtr+tempOffset, 512);
                cmapSegment.idRangeOffset = (short*)malloc(sizeof(short) * segCount);
                for(i = 0; i < segCount; i++){
                    tempOffset += i * 2;
                    cmapSegment.idRangeOffset[i] = readShort(encodingTablePtr + tempOffset);
                    if((cmapSegment.idRangeOffset[i])){
                        struct UnsignedShorNode unsignedShortNode;
                        unsignedShortNode.data = readUnsignedShort(encodingTablePtr + tempOffset + 2);
                        tempOffset += 2;
                        addTailListNode(&(cmapSegment.glyphIndexArray), &(unsignedShortNode.listNode));
                    }
                }
//                dumpCmapSegmentMappingToDelta(&cmapSegment);
                free(cmapSegment.endCount);
                free(cmapSegment.startCount);
                free(cmapSegment.idDelta);
                free(cmapSegment.idRangeOffset);
                break;
            }
        case CMAP_SUBTABLE_FORMAT_TRIMMED: // 6
            {
                CmapTrimmedTable trimmedTable;
                trimmedTable.format = format;
                trimmedTable.length = readUnsignedShort(encodingTablePtr + 2);
                trimmedTable.language = readUnsignedShort(encodingTablePtr + 4);
                trimmedTable.firstCode = readUnsignedShort(encodingTablePtr + 6);
                trimmedTable.entryCount = readUnsignedShort(encodingTablePtr + 8);
                trimmedTable.glyphIdArray = (unsigned short*)malloc(sizeof(unsigned short) * trimmedTable.entryCount);
                int i = 0;
                int tempOffset = 10;
                for(i = 0; i < trimmedTable.entryCount; i++){
                    tempOffset += i * 2;
                    trimmedTable.glyphIdArray[i] = readUnsignedShort(encodingTablePtr + tempOffset);
                }
                break;
            }
        case CMAP_SUBTABLE_FORMAT_MIXED: // 8
            {
                CmapMixed16Bit32Bit cmapMixer;
                cmapMixer.format = format;
                cmapMixer.reserved = readUnsignedShort(encodingTablePtr+2);
                cmapMixer.length = readUnsignedInt(encodingTablePtr + 4);
                cmapMixer.language = readUnsignedInt(encodingTablePtr + 8);
                int i = 0;
                int tempOffset = 12;
                for(i = 0; i < 8192; i++){
                    tempOffset += i;
                    cmapMixer.is32[i] = ((*(encodingTablePtr + tempOffset)) & 0xFF);
                }
                cmapMixer.nGroups = readUnsignedInt(encodingTablePtr + tempOffset + 1);

                int cmapGroupSize = sizeof(CmapGroup); // should be 12 bytes
                CmapGroup *cmapGroups = (CmapGroup*)malloc(cmapGroupSize * cmapMixer.nGroups);
                tempOffset = tempOffset + 5;
                for(i = 0; i < cmapMixer.nGroups; i++){
                    tempOffset += i * cmapGroupSize;
                    cmapGroups[i].startCharCode =  readUnsignedInt(encodingTablePtr + tempOffset);
                    cmapGroups[i].endCharCode =  readUnsignedInt(encodingTablePtr + tempOffset + 4);
                    cmapGroups[i].startGlyphId =  readUnsignedInt(encodingTablePtr + tempOffset + 8);
                }
                free(cmapGroups);
                break;
            }
        case CMAP_SUBTABLE_FORMAT_TRIMMED_ARRAY: //10
            {
                CmapTrimmedArray trimmedArrayTable;
                trimmedArrayTable.format = format;
                trimmedArrayTable.reserved = readUnsignedShort(encodingTablePtr + 2);
                trimmedArrayTable.length = readUnsignedShort(encodingTablePtr + 4);
                trimmedArrayTable.language = readUnsignedShort(encodingTablePtr + 6);
                trimmedArrayTable.startCharCode = readUnsignedShort(encodingTablePtr + 8);
                trimmedArrayTable.numChars = readUnsignedShort(encodingTablePtr + 10);
                break;
            }
        case CMAP_SUBTABLE_FORMAT_SEGMENT_COVERAGE: //12
            {
                CmapSegmentedCoverage cmapSegmentedCoverage;
                cmapSegmentedCoverage.format = format;
                cmapSegmentedCoverage.reserved = readUnsignedShort(encodingTablePtr+2);
                cmapSegmentedCoverage.length = readUnsignedInt(encodingTablePtr + 4);
                cmapSegmentedCoverage.language = readUnsignedInt(encodingTablePtr + 8);
                cmapSegmentedCoverage.nGroups = readUnsignedInt(encodingTablePtr + 12);

                int cmapGroupSize = sizeof(CmapGroup); // should be 12 bytes
                CmapGroup *cmapGroups = (CmapGroup*)malloc(cmapGroupSize * cmapSegmentedCoverage.nGroups);
                int i = 0;
                int tempOffset = 16;
                for(i = 0; i < cmapSegmentedCoverage.nGroups; i++){
                    tempOffset += i * cmapGroupSize;
                    cmapGroups[i].startCharCode =  readUnsignedInt(encodingTablePtr + tempOffset);
                    cmapGroups[i].endCharCode =  readUnsignedInt(encodingTablePtr + tempOffset + 4);
                    cmapGroups[i].startGlyphId =  readUnsignedInt(encodingTablePtr + tempOffset + 8);
                }
                free(cmapGroups);

                break;
            }
        case CMAP_SUBTABLE_FORMAT_MANY_TO_ONE: //13
            {
                CmapManyToOne cmapManyToOne;
                cmapManyToOne.format = format;
                cmapManyToOne.reserved = readUnsignedShort(encodingTablePtr+2);
                cmapManyToOne.length = readUnsignedInt(encodingTablePtr + 4);
                cmapManyToOne.language = readUnsignedInt(encodingTablePtr + 8);
                cmapManyToOne.nGroups = readUnsignedInt(encodingTablePtr + 12);

                int cmapGroupSize = sizeof(CmapGroup); // should be 12 bytes
                CmapGroup *cmapGroups = (CmapGroup*)malloc(cmapGroupSize * cmapManyToOne.nGroups);
                int i = 0;
                int tempOffset =  16;
                for(i = 0; i < cmapManyToOne.nGroups; i++){
                    tempOffset += i * cmapGroupSize;
                    cmapGroups[i].startCharCode =  readUnsignedInt(encodingTablePtr + tempOffset);
                    cmapGroups[i].endCharCode =  readUnsignedInt(encodingTablePtr + tempOffset + 4);
                    cmapGroups[i].startGlyphId =  readUnsignedInt(encodingTablePtr + tempOffset + 8);
                }
                free(cmapGroups);
                break;
            }
        case CMAP_SUBTABLE_FORMAT_UNICODE: //14
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

//   list_for_each(node, &tableEntryList){
//       tempTableEntryNode = listEntry(node, TableEntryNode, listNode);
//       dumpTableEntry(&(tempTableEntryNode->tableEntry));
//   }
//
   unsigned short platformId = -1, encodingId = -1;
   list_for_each(node, &cmapSubtableEntryList){
       tempCmapSubtableNode = listEntry(node, CmapSubtableNode, listNode);
       platformId = tempCmapSubtableNode->cmapSubtableEntry.platformId;
       encodingId = tempCmapSubtableNode->cmapSubtableEntry.encodingId;
       readEncodingTable(platformId, encodingId);

   }

    free(cmapSubtableNode);
    free(tableEntryNode);
    free(data);
    return 0;
}
