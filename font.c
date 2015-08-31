#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "font.h"
#include "dump.h"
#define MAX_HEADER 512

/*
 Global varibal START
 */
char *data;
struct TableDirectory tableDirectory;
TableEntryNode *tableEntryNode;
struct ListNode tableEntryList;

CmapSubtableNode *cmapSubtableNode;
struct ListNode cmapSubtableEntryList;
/*
 Global varibal END
 */

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

char* getTablePtr(int tableTag){
    TableEntry* tempTableEntry = getTableEntry(tableTag);
    if(tempTableEntry == NULL) return;
    char* tablePtr = data + tempTableEntry->offset;
    return tablePtr;
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


void readCmapSubtables(CmapHeader* cmapHeader){
    TableEntry* tempTableEntry = getTableEntry(DIRECTORY_TAG_CMAP);
    if(tempTableEntry == NULL) return;
    char* tablePtr = data + tempTableEntry->offset;

    cmapHeader->tableVersion = (tablePtr[0] & 0xFF) << 8 |
                              (tablePtr[1] & 0xFF);
    cmapHeader->numOfTable =   (tablePtr[2] & 0xFF) << 8 |
                              (tablePtr[3] & 0xFF);

    initListNode(&cmapSubtableEntryList);
    int subTableLength = sizeof(CmapEntry);

    cmapSubtableNode = (CmapSubtableNode *)malloc(sizeof(CmapSubtableNode) * cmapHeader->numOfTable);

    if(!cmapSubtableNode){
        printf("Error: Cannot get enough memory!\n");
    }

    int i = 0;
    CmapEntry cmapEntry;
    char* subTableEntryPtr = tablePtr + 4;
    for (i = 0; i < cmapHeader->numOfTable; i++){
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

void readFontHeaderTable(FontHeader* fontHeader){
    char* tablePtr = getTablePtr(DIRECTORY_TAG_HEAD);
    if(NULL == tablePtr) return;

    fontHeader->tableVersion = readUnsignedInt(tablePtr);
    fontHeader->fontVersion = readUnsignedInt(tablePtr + 4);
    fontHeader->checkSumAdjestment = readUnsignedInt(tablePtr + 8);
    fontHeader->magicNumber = readUnsignedInt(tablePtr + 12);
    fontHeader->flags = readUnsignedShort(tablePtr + 16);
    fontHeader->unitsPerEm = readUnsignedShort(tablePtr + 18);
    fontHeader->created = readUnsignedShort(tablePtr + 20);
    fontHeader->modified = readUnsignedShort(tablePtr + 21);
    fontHeader->xMin = readShort(tablePtr + 22);
    fontHeader->yMin = readShort(tablePtr + 24);
    fontHeader->xMax = readShort(tablePtr + 26);
    fontHeader->yMax = readShort(tablePtr + 28);
    fontHeader->macStyle = readUnsignedShort(tablePtr +30);
    fontHeader->lowestRecPPEM = readUnsignedShort(tablePtr +32);
    fontHeader->fontDirectionHint = readUnsignedShort(tablePtr +34);
    fontHeader->indexToLocationFormat = readUnsignedShort(tablePtr +36);
    fontHeader->glyphDataFormat = readUnsignedShort(tablePtr +38);
}

void readHorizontalHeader(HorizontalHeader* hheader){
    char* tablePtr = getTablePtr(DIRECTORY_TAG_HHEA);
    if(NULL == tablePtr) return;

    hheader->tableVersion = readUnsignedInt(tablePtr);
    hheader->ascender = readShort(tablePtr + 4);
    hheader->descender = readShort(tablePtr + 6);
    hheader->advanceWidthMax = readUnsignedShort(tablePtr + 8);
    hheader->minLeftSideBearing = readShort(tablePtr + 10);
    hheader->minRightSideBearing = readShort(tablePtr + 12);
    hheader->xMaxExtent = readShort(tablePtr + 14);
    hheader->caretSlopeRise = readShort(tablePtr + 16);
    hheader->caretSlopeRun = readShort(tablePtr + 18);
    hheader->reserved1 = readShort(tablePtr + 20);
    hheader->reserved2 = readShort(tablePtr + 22);
    hheader->reserved3 = readShort(tablePtr + 24);
    hheader->reserved4 = readShort(tablePtr + 26);
    hheader->merticDataFormat = readShort(tablePtr + 28);
    hheader->numberOfHMetrics = readShort(tablePtr + 30);
}

void readMaximumProfile(MaximumProfile_5* maxProfile_5,
        MaximumProfile1* maxProfile1){
    char* tablePtr = getTablePtr(DIRECTORY_TAG_MAXP);
    if(NULL == tablePtr) return;

    unsigned int version = readUnsignedInt(tablePtr);
    if(TABALE_VERSION_5 == version){
        maxProfile_5->version = version;
        maxProfile_5->numGlypha = readUnsignedShort(tablePtr + 4);
    } else if(TABALE_VERSION1 == version){
        maxProfile1->version = version;
        maxProfile1->numGlypha = readUnsignedShort(tablePtr + 4);
        maxProfile1->maxPoints = readUnsignedShort(tablePtr + 6);
        maxProfile1->maxContours = readUnsignedShort(tablePtr + 8);
        maxProfile1->maxCompositePoints = readUnsignedShort(tablePtr + 10);
        maxProfile1->maxCompositeContours = readUnsignedShort(tablePtr + 12);
        maxProfile1->maxZones = readUnsignedShort(tablePtr + 14);
        maxProfile1->maxTwilightPoints = readUnsignedShort(tablePtr + 16);
        maxProfile1->maxStorage = readUnsignedShort(tablePtr + 18);
        maxProfile1->maxFunctionDefs = readUnsignedShort(tablePtr + 20);
        maxProfile1->maxStackElements = readUnsignedShort(tablePtr + 22);
        maxProfile1->maxSizeOfInstructions = readUnsignedShort(tablePtr + 24);
        maxProfile1->maxComponentElements = readUnsignedShort(tablePtr + 26);
        maxProfile1->maxComponentDepth = readUnsignedShort(tablePtr + 28);
    } else {
        printf("Error: Cannot get version\n");
    }
};

void readOSTable(OS2* osTablePtr) {
    char* tablePtr = getTablePtr(DIRECTORY_TAG_OS_2);
    if(NULL == tablePtr) return;

    osTablePtr->version = readUnsignedShort(tablePtr);
    osTablePtr->xAvgCharWidth = readShort(tablePtr + 2);
    osTablePtr->usWeightClass = readUnsignedShort(tablePtr + 4);
    osTablePtr->usWidthClass = readUnsignedShort(tablePtr + 6);
    osTablePtr->fsType = readUnsignedShort(tablePtr + 8);

    osTablePtr->ySubscriptXSize = readShort(tablePtr + 10);
    osTablePtr->ySubscriptYSize = readShort(tablePtr + 12);
    osTablePtr->ySubscriptXOffset = readShort(tablePtr + 14);
    osTablePtr->ySubscriptYOffset = readShort(tablePtr + 16);
    osTablePtr->ySuperscriptXSize = readShort(tablePtr + 18);
    osTablePtr->ySuperscriptYSize = readShort(tablePtr + 20);
    osTablePtr->ySuperscriptXOffset = readShort(tablePtr + 22);
    osTablePtr->ySuperscriptYOffset = readShort(tablePtr + 24);
    osTablePtr->yStrikeoutSize = readShort(tablePtr + 26);
    osTablePtr->yStrikeoutPosition = readShort(tablePtr + 28);
    osTablePtr->sFamilyClass = readShort(tablePtr + 30);

    int tempOffset = 32;
    int i = 0;
    for(i = 0; i < 10; i++){
        osTablePtr->panose[i] = ((*(tablePtr + tempOffset)) & 0xFF);
        tempOffset++;
    }
    osTablePtr->panose[0] = ((*(tablePtr + 32)) & 0xFF);
    osTablePtr->panose[1] = ((*(tablePtr + 33)) & 0xFF);
    osTablePtr->panose[2] = ((*(tablePtr + 34)) & 0xFF);
    osTablePtr->panose[3] = ((*(tablePtr + 35)) & 0xFF);
    osTablePtr->panose[4] = ((*(tablePtr + 36)) & 0xFF);
    osTablePtr->panose[5] = ((*(tablePtr + 37)) & 0xFF);
    osTablePtr->panose[6] = ((*(tablePtr + 38)) & 0xFF);
    osTablePtr->panose[7] = ((*(tablePtr + 39)) & 0xFF);
    osTablePtr->panose[8] = ((*(tablePtr + 40)) & 0xFF);
    osTablePtr->panose[9] = ((*(tablePtr + 41)) & 0xFF);

    osTablePtr->ulUnicodeRange1 = readUnsignedInt(tablePtr + 42);
    osTablePtr->ulUnicodeRange2 = readUnsignedInt(tablePtr + 46);
    osTablePtr->ulUnicodeRange3 = readUnsignedInt(tablePtr + 50);
    osTablePtr->ulUnicodeRange4 = readUnsignedInt(tablePtr + 54);
    osTablePtr->achVend[0] = ((*(tablePtr + 58)) & 0xFF);
    osTablePtr->achVend[1] = ((*(tablePtr + 59)) & 0xFF);
    osTablePtr->achVend[2] = ((*(tablePtr + 60)) & 0xFF);
    osTablePtr->achVend[3] = ((*(tablePtr + 61)) & 0xFF);
    osTablePtr->fsSelection = readUnsignedShort(tablePtr + 62);
    osTablePtr->usFirstCharIndex = readUnsignedShort(tablePtr + 64);;
    osTablePtr->usLastCharIndex = readUnsignedShort(tablePtr + 66);
    osTablePtr->sTypoAscender = readShort(tablePtr + 68);
    osTablePtr->sTypoDescender = readShort(tablePtr + 70);
    osTablePtr->sTypoLineGap = readShort(tablePtr + 72);
    osTablePtr->usWinAscent = readUnsignedShort(tablePtr + 74);
    osTablePtr->usWinDescent = readUnsignedShort(tablePtr + 76);
    osTablePtr->ulCodePageRange1 = readUnsignedInt(tablePtr + 78);
    osTablePtr->ulCodePageRange2 = readUnsignedInt(tablePtr + 82);
    osTablePtr->sxHeight = readShort(tablePtr + 86);
    osTablePtr->sCapHeight = readShort(tablePtr + 88);
    osTablePtr->usDefaultChar = readUnsignedShort(tablePtr + 90);
    osTablePtr->usBreakChar = readUnsignedShort(tablePtr + 92);
    osTablePtr->usMaxContext = readUnsignedShort(tablePtr + 94);
    osTablePtr->usLowerOpticalPointSize = readUnsignedShort(tablePtr + 96);
    osTablePtr->usUpperOpticalPointSize = readUnsignedShort(tablePtr + 98);
};

void readGlyfData(GlyfData* glyfData,
        SimpleGlyphDescription* simpleGlyphDescription,
        CompositeGlyphDescription* compositeGlyphDescription){
    char* tablePtr = getTablePtr(DIRECTORY_TAG_GLFY);
    if(NULL == tablePtr) return;

    glyfData->numberOfContours = readShort(tablePtr);
    glyfData->xMin = readShort(tablePtr + 2);
    glyfData->yMin = readShort(tablePtr + 4);
    glyfData->xMax = readShort(tablePtr + 6);
    glyfData->yMax = readShort(tablePtr + 8);

    if(glyfData->numberOfContours > 0){
        /*Simple GlyphDescription*/
        int i = 0;
        int tempOffset = 10;
        for(i = 0; i < glyfData->numberOfContours; i++){
            simpleGlyphDescription->endPtsOfContours = readUnsignedShort(tablePtr + tempOffset);
            tempOffset += 2;
        }

        simpleGlyphDescription->instructionLength = readUnsignedShort(tablePtr + tempOffset);
        tempOffset += 2;
        for(i = 0; i < glyfData->numberOfContours; i++){
            simpleGlyphDescription->instructions[i] = ((*(tablePtr + tempOffset)) & 0xFF);
            tempOffset ++;
        }

        for(i = 0; i < glyfData->numberOfContours; i++){
            simpleGlyphDescription->flags[i] = ((*(tablePtr + tempOffset)) & 0xFF);
            tempOffset ++;
        }

        for(i = 0; i < glyfData->numberOfContours; i++){
            if(simpleGlyphDescription->flags[i]){
                simpleGlyphDescription->xCoordinates.xCoorByte = ((*(tablePtr + tempOffset)) & 0xFF);
                tempOffset ++;
            } else {
                simpleGlyphDescription->xCoordinates.xCoorShort = readUnsignedShort(tablePtr + tempOffset);
                tempOffset += 2;
            }
        }

        for(i = 0; i < glyfData->numberOfContours; i++){
            if(simpleGlyphDescription->flags[i]){
                simpleGlyphDescription->yCoordinates.yCoorByte = ((*(tablePtr + tempOffset)) & 0xFF);
                tempOffset ++;
            } else {
                simpleGlyphDescription->yCoordinates.yCoorShort = readUnsignedShort(tablePtr + tempOffset);
                tempOffset += 2;
            }
        }
    } else if(glyfData->numberOfContours <= 0){
        /*
         * PASS
         * */
    }

};

int main()
{
    TableEntryNode *tempTableEntryNode;
    CmapSubtableNode *tempCmapSubtableNode;
    struct ListNode *node;
    initFontInfo();
    CmapHeader cmapHeader;
    readCmapSubtables(&cmapHeader);

   list_for_each(node, &tableEntryList){
       tempTableEntryNode = listEntry(node, TableEntryNode, listNode);
       dumpTableEntry(&(tempTableEntryNode->tableEntry));
   }
//
//  unsigned short platformId = -1, encodingId = -1;
//  list_for_each(node, &cmapSubtableEntryList){
//      tempCmapSubtableNode = listEntry(node, CmapSubtableNode, listNode);
//      platformId = tempCmapSubtableNode->cmapSubtableEntry.platformId;
//      encodingId = tempCmapSubtableNode->cmapSubtableEntry.encodingId;
//      readEncodingTable(platformId, encodingId);
//
//  }

    free(cmapSubtableNode);
    free(tableEntryNode);
    free(data);
    return 0;
}
