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
#define TRACK_BASE          1
#define WEAPON_GRID_LEFT    2
#define FLIP_TEXTURE        4
#define WEAPON_GRID_RIGHT   8
#define START_GRID          16
#define SPEED_UP            32

#ifdef __3DO__
typedef struct Face
{
    int16 vertex[4];
    SVector normal;
    uint8 flags;
    uint8 texture;
    uint8* plut;
} Face;
#else
typedef struct Face
{
    int16 vertex[4];
    SVector normal;
    uint8 texture;
    uint8 flags;
    uint8 red;
    uint8 green;
    uint8 blue;
} Face;
#endif

/* TrackSection Flags */
#define Jump            1
#define JunctionEnd     8
#define JunctionStart   16
#define JunctionSection 32

#define ObjectTableMax  512 // 464
#define TrackTextures   24

typedef struct TrackRange
{
    uint16 vStart;
    uint16 vCount;
    uint16 fStart;
    uint16 fCount;
} TrackRange;

#ifdef __3DO__
typedef struct TrackSection
{
    struct TrackSection* junction;
    struct TrackSection* prevSection;
    struct TrackSection* nextSection;

    Vector centre;

    uint16 faceStart;
    uint16 faceCount;

    int16* objects;
    uint32 objectCount;

    uint16 globalRadius;
    uint16 localRadius;

    uint16 flags;
    uint16 secNo;

    TrackRange visRange[2][2];
} TrackSection;
#else
typedef struct TrackSection
{
    struct TrackSection* junction;
    struct TrackSection* prevSection;
    struct TrackSection* nextSection;

    Vector centre;

    short unused_version;
    short unused_pad;

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

    short unused[8];

    uint16 faceStart;
    uint16 faceCount;

    uint16 globalRadius;
    uint16 localRadius;

    uint16 flags;
    uint16 secNo;
} TrackSection;
#endif

typedef struct TrackData
{
    long vertexCount;
    long faceCount;
    long sectionCount;

    VECTOR* vertices;
    Face* faces;
    TrackSection* sections;
    int16* viewList;
} TrackData;

typedef struct TrackCamera
{
    Skeleton* camPos;
    TrackSection* section;
    short hdg, pitch, roll;
    short vhdg, vpitch, vroll;
    SVECTOR vel;
} TrackCamera;

#ifdef __3DO__
void InitTrackShade(char* file);
#endif

TrackSection* NextIndexSection(TrackSection* section);

void LoadTrack(int32 trackNum);

#ifdef _DEBUG
void convertTrack3DO(int32 trackNum);
#endif