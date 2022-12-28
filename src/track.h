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
    vec3s normal;
    uint8 flags;
    uint8 texture;
    uint8* plut;
} Face;
#else
typedef struct Face
{
    int16 vertex[4];
    vec3s normal;
    uint8 texture;
    uint8 flags;
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 padding_unused;
} Face;
#endif

/* TrackSection Flags */
#define Jump            1
#define JunctionEnd     8
#define JunctionStart   16
#define JunctionSection 32

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

    vec3i centre;

    uint16 faceStart;
    uint16 faceCount;

    int16* objects;
    uint32 objectsCount;

    uint16 globalRadius;
    uint16 localRadius;

    uint16 flags;
    uint16 secNo;

    TrackRange visRange[2][2];
} TrackSection;
#else
typedef struct TrackSection
{
    vec3i centre;

    int16 objectsCount;

    int16 northViewCount[3];
    int16 southViewCount[3];
    int16 eastViewCount[3];
    int16 westViewCount[3];
    int16 allViewCount[3];

    uint16 faceStart;
    uint16 faceCount;

    uint16 globalRadius;
    uint16 localRadius;

    uint16 flags;
    uint16 secNo;

    struct TrackSection* junction;
    struct TrackSection* prevSection;
    struct TrackSection* nextSection;
    int16* objects;
    int16* northViewSection[3];
    int16* southViewSection[3];
    int16* eastViewSection[3];
    int16* westViewSection[3];
    int16* allViewSection[3];
} TrackSection;
#endif

typedef struct WeaponGrid
{
    TrackSection* section;
    short attr;
    short cycleCount;
    int count;
    CVECTOR colour;
} WeaponGrid;

typedef struct TrackData
{
    int32 vertexCount;
    int32 faceCount;
    int32 sectionCount;
    int32 weaponsCount;

    int32 objCount;
    int32 startCount;
    int32 lostCount;
    int32 redCount;
    int32 donkeyCount;

    vec3i* vertices;
    Face* faces;
    TrackSection* sections;
    WeaponGrid* weapons;
    int16* viewList;

    Object* objects;
    Object* sky;
    Object* start;
    Object* lost;
    Object* red;
    Object* donkey;
} TrackData;

typedef struct TrackCamera
{
    TrackSection* section;
    int32 hdg, pitch, roll;
    vec3i pos;
    vec3i vel;
} TrackCamera;

TrackSection* NextIndexSection(TrackSection* section);
