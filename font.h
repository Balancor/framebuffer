#include "utils.h"

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

#define CMAP_SUBTABLE_FORMAT_BYTE 0
#define CMAP_SUBTABLE_FORMAT_HIGH_BYTE 2
#define CMAP_SUBTABLE_FORMAT_SEGMENT 4
#define CMAP_SUBTABLE_FORMAT_TRIMMED 6
#define CMAP_SUBTABLE_FORMAT_MIXED 8
#define CMAP_SUBTABLE_FORMAT_TRIMMED_ARRAY 10
#define CMAP_SUBTABLE_FORMAT_SEGMENT_COVERAGE 12
#define CMAP_SUBTABLE_FORMAT_MANY_TO_ONE 13
#define CMAP_SUBTABLE_FORMAT_UNICODE 14

#define MAX_SIZE 256

#define FONT_DIRECTORY  "data"
#define FONT_FILE "FreeMono.ttf"

struct UnsignedShorNode{
    unsigned short data;
    struct ListNode listNode;
};

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


struct TableEntry{
    char tag[4]; // four character for tag, the end character for '\0'
    unsigned int checksum;
    unsigned int offset;
    unsigned int length;
};

struct TableDirectory {
    int sfntVersion;
    unsigned short numTables;
    unsigned short searchRange;
    unsigned short entrySelector;
    unsigned short rangeShift;
    struct TableEntry entries[1];
};


unsigned long calcTableCheckSum(unsigned long* data, unsigned long length){
    unsigned long sum = 0L;
    unsigned long *endPtr = data + ((length + 3) & 3) / sizeof(unsigned long);
    while(data < endPtr){
        sum += *data++;
    }
    return sum;
};

struct FontHeader {
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
};

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
}CmapMixed16Bit32BitGroup;

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

typedef struct {
    typedef struct{
        unsigned short highBytes;
        char lowByte;
    }varSelector;
    unsigned int defaultUVSOffset;
    unsigned int nonDefaultUVSOffset;
}VariationSelector;

struct CMAP { };

struct GLFY { };

struct TableMaxp {
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
};

struct Mmtx { };

struct Loca { };

struct Name { };

struct Hmtx { };

struct Kerm { };

struct PostScriptInfo { };

struct PCLT { };
