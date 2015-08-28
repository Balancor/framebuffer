#include "font.h"
#include "utils.h"

#ifndef _DUMP_H
#define _DUMP_H
void dumpTableEntry(struct TableEntry *tableEntry){
    printf("TableEntry:\n");
    printf("\t tag: %c%c%c%c\n", tableEntry->tag[0], tableEntry->tag[1],
                                 tableEntry->tag[2], tableEntry->tag[3]);
    printf("\t checksum: %u\n", tableEntry->checksum);
    printf("\t offset: %u\n", tableEntry->offset);
    printf("\t length: %u\n", tableEntry->length);
}

void dump(const char* data, int length){
    printf("\n");
    int i = 0;
    for(i = 0; i < length; i++){
        printf("%02x", (data[i] & 0xFF));
        if((i + 1) % 2 == 0) printf(" ");
        if((i + 1) % 16 == 0)printf("\n");
    }
    printf("\n");
}


void dumpCmapSegmentMappingToDelta (CmapSegmentMappingToDelta* table){
    int segCount = ((table->segCountX2) >> 1);
    printf("CmapSegmentMappingToDelta:\n");
    printf("\t format: %u\n", table->format);
    printf("\t length: %u\n", table->length);
    printf("\t language: %u\n", table->language);
    printf("\t segCountX2: %u\n", table->segCountX2);
    printf("\t searchRange: %u\n", table->searchRange);
    printf("\t entrySelector: %u\n", table->entrySelector);
    printf("\t rangeShift: %u\n", table->rangeShift);
    int i = 0;
    printf("\t Segment:\n");
    printf("\t\t Segment[i], EndCharactorCode, \tStartCharactorCode, \tidDelta, \tidRangeOffset\n");
    for(i = 0; i < segCount; i++){
        printf("\t\t Segment[%d]:\t0x%04x, \t\t0x%04x, \t\t0x%04x, \t\t0x%04x\n", i,
                table->endCount[i], table->startCount[i], table->idDelta[i], table->idRangeOffset[i]);
    }

    printf("\tglyphIndexArray:\n");
    i = 0;
    struct UnsignedShorNode *tempUnsignedShortNode;
    struct ListNode *node;
    list_for_each(node, &(table->glyphIndexArray)){
        tempUnsignedShortNode = listEntry(node, UnsignedShorNode, listNode);
        printf("\t glyphIndexArray[%d]: %u\n", i++, tempUnsignedShortNode->data);
    }

}


#endif
