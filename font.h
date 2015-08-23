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
}
