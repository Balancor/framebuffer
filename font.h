#include "utils.h"

#ifndef _FONT_H
#define _FONT_H

#define DIRECTORY_TAG_CMAP "cmap" //character to glyph mapping
#define DIRECTORY_TAG_GLFY "glyf" // glyph data
#define DIRECTORY_TAG_HEAD "head" //font header
#define DIRECTORY_TAG_HHEA "hhea" //horizontal header
#define DIRECTORY_TAG_HMTX "hmtx" //horizontal metrics
#define DIRECTORY_TAG_LOCA "loca" //index to location
#define DIRECTORY_TAG_MAXP "maxp" //maximum profile
#define DIRECTORY_TAG_NAME "name" //naming table
#define DIRECTORY_TAG_POST "post" //PostScript information
#define DIRECTORY_TAG_OS_2 "OS/2" //OS/2 and Windows specific metrics

#define DIRECTORY_TAG_CVT  "cvt " //Control Value Table
#define DIRECTORY_TAG_EBDT "EBDT" //Embedded bitmap data
#define DIRECTORY_TAG_EBLC "EBLC" //Embedded bitmap location data
#define DIRECTORY_TAG_EBSC "EBSC" //Embedded bitmap scaling data
#define DIRECTORY_TAG_FPGM "fpgm" //font program
#define DIRECTORY_TAG_GASP "gasp" //grid-fitting and scan conversion procedure (grayscale)
#define DIRECTORY_TAG_HDMX "hdmx" //horizontal device metrics
#define DIRECTORY_TAG_KERN "kern" //kerning
#define DIRECTORY_TAG_LTSH "LTSH" //Linear threshold table
#define DIRECTORY_TAG_PREP "prep" //CVT Program
#define DIRECTORY_TAG_PCLT "PCLT" //PCL5
#define DIRECTORY_TAG_VDMX "VDMX" //Vertical Device Metrics table
#define DIRECTORY_TAG_VHEA "vhea" //Vertical Metrics header
#define DIRECTORY_TAG_VMTX "vmtx" //Vertical Metrics

/* Glyph flags  START
 */
#define ARG_1_AND_2_ARE_WORDS 1 << 0
#define ARGS_ARE_XY_VALUES 1 << 1
#define ROUND_XY_TO_GRID 1 << 2
#define WE_HAVE_A_SCALE 1 << 3
#define RESERVED  1 << 4
#define MORE_COMPONENTS 1 << 5
#define WE_HAVE_AN_X_AND_Y_SCALE 1 << 6
#define WE_HAVE_A_TWO_BY_TWO 1 << 7
#define WE_HAVE_INSTRUCTIONS 1 << 8
#define USE_MY_METRICS 1 << 9
#define OVERLAP_COMPOUND 1 << 10
#define SCALED_COMPONENT_OFFSET 1 << 11
#define UNSCALED_COMPONENT_OFFSET 1 << 12
/* Glyph flags  END
 */

/*
 *Glyph Data Simple Glyph Description Flags
 START
 * */
#define SIMPLE_GLYPH_FLAG_ON_CURVE 1 << 0
#define SIMPLE_GLYPH_FLAG_X_SHORT_VECTOR 1 << 1
#define SIMPLE_GLYPH_FLAG_Y_SHORT_VECTOR 1 << 2
#define SIMPLE_GLYPH_FLAG_REPEAT 1 << 3
#define SIMPLE_GLYPH_FLAG_X_SAME 1 << 4
#define SIMPLE_GLYPH_FLAG_Y_SAME 1 << 5
#define SIMPLE_GLYPH_FLAG_RESERVED1 1 << 6
#define SIMPLE_GLYPH_FLAG_RESERVED2 1 << 7
/*
 *Glyph Data Simple Glyph Description Flags
 END
 * */
#define CMAP_SUBTABLE_FORMAT_BYTE 0
#define CMAP_SUBTABLE_FORMAT_HIGH_BYTE 2
#define CMAP_SUBTABLE_FORMAT_SEGMENT 4
#define CMAP_SUBTABLE_FORMAT_TRIMMED 6
#define CMAP_SUBTABLE_FORMAT_MIXED 8
#define CMAP_SUBTABLE_FORMAT_TRIMMED_ARRAY 10
#define CMAP_SUBTABLE_FORMAT_SEGMENT_COVERAGE 12
#define CMAP_SUBTABLE_FORMAT_MANY_TO_ONE 13
#define CMAP_SUBTABLE_FORMAT_UNICODE 14

#define TABALE_VERSION_5 0x00005000
#define TABALE_VERSION1  0x00010000

#define MAX_SIZE 256

#define FONT_DIRECTORY  "data"
#define FONT_FILE "FreeMono.ttf"



typedef struct{
        unsigned short firstCode;
        unsigned short entryCount;
        short idDelta;
        unsigned short idRangeOffset;
}SubHeaders;

struct SubHeadersNode{
    SubHeaders subHeaders;
    struct ListNode listNode;
};


typedef struct TableEntry{
    char tag[4]; // four character for tag, the end character for '\0'
    unsigned int checksum;
    unsigned int offset;
    unsigned int length;
}TableEntry;

typedef struct TableDirectory {
    int sfntVersion;
    unsigned short numTables;
    unsigned short searchRange;
    unsigned short entrySelector;
    unsigned short rangeShift;
    struct TableEntry entries[1];
}TableDirectory;


unsigned long calcTableCheckSum(unsigned long* data, unsigned long length){
    unsigned long sum = 0L;
    unsigned long *endPtr = data + ((length + 3) & 3) / sizeof(unsigned long);
    while(data < endPtr){
        sum += *data++;
    }
    return sum;
};

typedef struct FontHeader {
    unsigned int tableVersion;
    unsigned int fontVersion;
    unsigned int checkSumAdjestment;
    unsigned int magicNumber;
    unsigned short flags;
    unsigned short unitsPerEm;
    char created;
    char modified;

    short xMin;
    short yMin;
    short xMax;
    short yMax;

    unsigned short macStyle;
    unsigned short lowestRecPPEM;
    unsigned short fontDirectionHint;
    unsigned short indexToLocationFormat;
    unsigned short glyphDataFormat;
}FontHeader;

typedef struct {
    unsigned short tableVersion;
    unsigned short numOfTable;
}CmapHeader;

typedef struct {
    unsigned short platformId;
    unsigned short encodingId;
    unsigned int offset;
}CmapEntry;

typedef struct {
    unsigned short format; //should be 0
    unsigned short length;
    unsigned short language;
    char glyIdArray[256];
}CmapByteEncodingTable;


typedef struct {
    unsigned short format; //should be 2
    unsigned short length;
    unsigned short language;
    unsigned short subHeaderKeys[256];
    struct ListNode subHeadersList; //List of SubHeadersNode;
    struct ListNode glyphIndexArray; //List of UnsignedShorNode
}CmapHighbyteMappingTable;

typedef struct {
    unsigned short format; //should be 4;
    unsigned short length;
    unsigned short language;
    unsigned short segCountX2;
    unsigned short searchRange;
    unsigned short entrySelector;
    unsigned short rangeShift;
    short* endCount; //should have segCount
    unsigned short reservedPad;
    short* startCount; //should have segCount;
    short* idDelta; //should have segCount;
    short* idRangeOffset; //should have segCount;
    struct ListNode glyphIndexArray; // List of UnsignedShorNode
}CmapSegmentMappingToDelta;

typedef struct{
    unsigned short format; //should be 6;
    unsigned short length;
    unsigned short language;
    unsigned short firstCode;
    unsigned short entryCount;
    unsigned short* glyphIdArray; //should have entryCount
}CmapTrimmedTable;

typedef struct {
    unsigned short format; // should be 8
    unsigned short reserved; //set to 0
    unsigned int length;
    unsigned int  language;
    char is32[8192];
    unsigned int nGroups;
}CmapMixed16Bit32Bit;

typedef struct {
    unsigned int startCharCode;
    unsigned int endCharCode;
    unsigned int startGlyphId;
}CmapGroup;

typedef struct {
    unsigned short format; //should be 10
    unsigned short reserved;
    unsigned int length;
    unsigned int language;
    unsigned int startCharCode;
    unsigned int numChars;
    struct ListNode glyphs; //List of UnsignedShorNode
}CmapTrimmedArray;

typedef struct {
    unsigned short format; //should be 12
    unsigned short reserved;
    unsigned int length;
    unsigned int language;
    unsigned int nGroups;
}CmapSegmentedCoverage;

typedef struct {
    unsigned short format; //should be 13
    unsigned short reserved;
    unsigned int length;
    unsigned int language;
    unsigned int nGroups;
}CmapManyToOne;

typedef struct {
    unsigned short format; //should be 14
    unsigned int length;
    unsigned int numVarSelectorRecords;
}CmapUnicodeVariation;

typedef struct{
        unsigned short highBytes;
        char lowByte;
}VarSelector;

typedef struct {
    unsigned int defaultUVSOffset;
    VarSelector varSelector;
    unsigned int nonDefaultUVSOffset;
}VariationSelector;

typedef struct{
    unsigned int version;
    unsigned short numGlypha;
}MaximumProfile_5;

typedef struct{
    unsigned int version;
    unsigned short numGlypha;
    unsigned short maxPoints;
    unsigned short maxContours;
    unsigned short maxCompositePoints;
    unsigned short maxCompositeContours;
    unsigned short maxZones;
    unsigned short maxTwilightPoints;
    unsigned short maxStorage;
    unsigned short maxFunctionDefs;
    unsigned short maxStackElements;
    unsigned short maxSizeOfInstructions;
    unsigned short maxComponentElements;
    unsigned short maxComponentDepth;
}MaximumProfile1;

typedef struct _HorizontalHeader{
    unsigned int tableVersion;
    short ascender;
    short descender;
    unsigned short advanceWidthMax;
    short minLeftSideBearing;
    short minRightSideBearing;
    short xMaxExtent;
    short caretSlopeRise;
    short caretSlopeRun;
    short caretOffset;
    short reserved1;
    short reserved2;
    short reserved3;
    short reserved4;
    short merticDataFormat;
    unsigned short numberOfHMetrics;
}HorizontalHeader;
struct Mmtx { };

struct Loca { };

struct Name { };

typedef struct _longHorMetric {
    unsigned short advanceWidth;
    short lsb;
}LongHorMetric;
typedef struct {
    LongHorMetric longHorMetric;
    struct ListNode listNode;
}LongHorMetricNode;

typedef struct Hmtx {
    struct ListNode longHorMetric; //List of LongHorMetricNode
    struct ListNode leftSideBearing; //List of UnsignedShorNode
}Hmtx;

typedef struct {
    unsigned int tableVersion;
    unsigned short numGlyphs;
}MaxpOlder;

typedef struct{
    unsigned int tableVersion;
    unsigned short numGlyphs;
    unsigned short maxPoints;
    unsigned short maxContours;
    unsigned short maxCompositePoints;
    unsigned short maxCompositeCountours;
    unsigned short maxZones;
    unsigned short maxTwillightPoints;
    unsigned short maxStorage;
    unsigned short maxFunctionDefs;
    unsigned short maxInstructionDefs;
    unsigned short maxStackElements;
    unsigned short maxSizeOfInstructions;
    unsigned short maxComponentElements;
    unsigned short maxComponentDepth;
}Maxp1;

typedef struct{
    unsigned short version;
    short xAvgCharWidth;
    unsigned short usWeightClass;
    unsigned short usWidthClass;
    unsigned short fsType;
    short ySubscriptXSize;
    short ySubscriptYSize;
    short ySubscriptXOffset;
    short ySubscriptYOffset;
    short ySuperscriptXSize;
    short ySuperscriptYSize;
    short ySuperscriptXOffset;
    short ySuperscriptYOffset;
    short yStrikeoutSize;
    short yStrikeoutPosition;
    short sFamilyClass;
    char panose[10];
    unsigned int ulUnicodeRange1;
    unsigned int ulUnicodeRange2;
    unsigned int ulUnicodeRange3;
    unsigned int ulUnicodeRange4;
    char achVend[4];
    unsigned short fsSelection;
    unsigned short usFirstCharIndex;
    unsigned short usLastCharIndex;
    short sTypoAscender;
    short sTypoDescender;
    short sTypoLineGap;
    unsigned short usWinAscent;
    unsigned short usWinDescent;
    unsigned int ulCodePageRange1;
    unsigned int ulCodePageRange2;
    short sxHeight;
    short sCapHeight;
    unsigned short usDefaultChar;
    unsigned short usBreakChar;
    unsigned short usMaxContext;
    unsigned short usLowerOpticalPointSize;
    unsigned short usUpperOpticalPointSize;
}OS2;

typedef struct UnsignedShorNode{
    unsigned short data;
    struct ListNode listNode;
}UnsignedShorNode;

typedef struct _CmapSubtableEntryNode {
    CmapEntry cmapSubtableEntry;
    struct ListNode listNode;
    unsigned int offsetToBeginFile;
}CmapSubtableNode;

typedef struct _TableEntryNode{
    struct TableEntry tableEntry;
    struct ListNode listNode;
}TableEntryNode;

typedef struct {
    short numberOfContours;
    short xMin;
    short yMin;
    short xMax;
    short yMax;
}GlyfData;

typedef struct{
    unsigned short* endPtsOfContours;
    unsigned short instructionLength;
    char* instructions;
    char* flags;
    union{
        char xCoorByte;
        short xCoorShort
    }xCoordinates;

    union{
        char yCoorByte;
        short yCoorShort
    }yCoordinates;

}SimpleGlyphDescription;

typedef struct{
    unsigned short flags;
    unsigned short glyphIndex;
    /* the type of arguments is unknown.
       so, JUST assume the type as short
     */
    short argument1;
    short argument2;
}CompositeGlyphDescription;

#endif
