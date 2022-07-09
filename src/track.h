

#define NEW_TRACK 1

typedef struct Normal
{
    signed char vx;
    signed char vy;
    signed char vz;
} Normal;

typedef struct Vector
{
    long vx;
    long vy;
    long vz;
} Vector;

typedef struct SVector
{
    short vx;
    short vy;
    short vz;
} SVector;

/* Face flags */
#define TRACK_BASE 1
#define WEAPON_GRID_LEFT 2
#define FLIP_TEXTURE 4
#define WEAPON_GRID_RIGHT 8
#define START_GRID 16
#define SPEED_UP 32

typedef struct Face
{
    short vertex[4];
    SVector normal;
    char texture;
    char flags;
    uint8 red;
    uint8 green;
    uint8 blue;
} Face;

typedef union
{
    struct TrackSection* ptr;
    long index;
} SectionPtr;

/* TrackSection Flags */
#define Jump 1
#define JunctionEnd 8
#define JunctionStart 16
#define JunctionSection 32

typedef struct TrackSection
{
    SectionPtr junction;

    SectionPtr prevSection;
    SectionPtr nextSection;

    Vector centre;

    short version;
    short pad;

    short* objects;
    short objectCount;

    short* northViewSection[3];
    short* southViewSection[3];
    short* eastViewSection[3];
    short* westViewSection[3];
    short* allViewSection[3];
    short northViewCount[3];
    short southViewCount[3];
    short eastViewCount[3];
    short westViewCount[3];
    short allViewCount[3];

    /*TODO unused*/
    short unused_high[4];
    short unused_med[4];

    short faceStart;
    short faceCount;

    short globalRadius;
    short localRadius;

    short flags;
    short secNo;
} TrackSection;

typedef struct colors
{
    char red;
    char green;
    char blue;
} Colors;

#define MAX_VERTICES    (32 * 104)
#define MAX_FACES       (32 * 88)
#define MAX_SECTIONS    (32 * 19)
#define MAX_VIEWLIST    (32 * 933)

typedef struct TrackData
{
    long vertexCount;
    long faceCount;
    long sectionCount;

    VECTOR vertices[MAX_VERTICES];
    Face faces[MAX_FACES];
    TrackSection sections[MAX_SECTIONS];
    short viewList[MAX_VIEWLIST];
} TrackData;

typedef struct TrackCamera
{
    Skeleton* camPos;
    TrackSection* section;
    short hdg, pitch, roll;
    short vhdg, vpitch, vroll;
    SVECTOR vel;
} TrackCamera;

#define MaxAutos 256

void IntelVertex(void);
void IntelFace(void);
void IntelSection(void);

void CheckVersion(void);

void IndexToPointers(void);
void PointersToIndex(void);

void InitVertices(char* file);
void InitFaces(char* file);
void InitViewList(char* file);
void InitSections(char* file);

TrackSection* NextIndexSection(TrackSection* section);
TrackSection* PrevIndexSection(TrackSection* section);
