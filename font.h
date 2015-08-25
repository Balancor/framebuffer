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


#define MAX_SIZE 256

#define FONT_DIRECTORY  "data"
#define FONT_FILE "FreeMono.ttf"

struct UnsignedShortList{
    unsigned short data;
    struct UnsignedShortList *next;
    struct UnsignedShortList *prev;
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

typedef struct{
        unsigned short firstCode;
        unsigned short entryCount;
        short idDelta;
        unsigned short idRangeOffset;
}subHeaders;

typedef struct {
    unsigned short format; //should be 2
    unsigned short length;
    unsigned short version;
    unsigned short subHeaderKeys[256];
    subHeaders subHeaders1[4];
    subHeaders subHeaders2[4];
    struct UnsignedShortList glyphIndexArray;
}CmapHighbyteMappingTable;

typedef struct {
    unsigned short format; //should be 4;
    unsigned short length;
    unsigned short version;
    unsigned short segCountX2;
    unsigned short searchRange;
    unsigned short entrySelector;
    unsigned short rangeShift;
    short* endCount; //should have segCount
    unsigned short reservedPad;
    short* startCount; //should have segCount;
    short* idDelta; //should have segCount;
    short* idRangeOffset; //should have segCount;
    struct UnsignedShortList glyphIndexArray;
}CmapSegmentMappingToDelta;

typedef struct{
    unsigned short format; //should be 6;
    unsigned short length;
    unsigned short version;
    unsigned short firstCode;
    unsigned short entryCount;
    unsigned short* glyphIdArray; //should have entryCount
}CmapTrimmedTable;
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
