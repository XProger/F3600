#define TRACK_FACE_BASE     (1 << 0)
#define TRACK_FACE_SPEED    (1 << 1)
#define TRACK_FACE_FLIP     (1 << 2)

#define GET_TRACK_FACE_TEXTURE(flags)   ((flags >> 3) & 31)
#define GET_TRACK_FACE_WEAPON(flags)    (flags >> 8)

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
    uint16 flags;
    uint32 color;
} Face;
#endif

/* TrackSection Flags */
#define TRACK_JUMP              (1 << 0)
#define TRACK_JUNCTION_END      (1 << 3)
#define TRACK_JUNCTION_START    (1 << 4)
#define TRACK_JUNCTION          (1 << 5)

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
    int16 faceIndex;
    int16 count;
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
    WeaponGrid weapons[64];
    int16* viewList;

    Object* objects;
    Object* sky;
    Object* start;
    Object* lost;
    Object* red;
    Object* donkey;
} TrackData;

 struct ShipData;
 struct RescueData;

typedef struct TrackCamera
{
    TrackSection* section;
    vec3i vel;

    vec3i pos;
    int32 pitch, hdg, roll;
#ifdef USE_FRAME_LERP
    vec3i _pos;
    int32 _pitch, _hdg, _roll;
#endif

    int32 fov;
    int32 fovTarget;

    void (*update)(struct TrackCamera*, struct ShipData*, struct RescueData*);
} TrackCamera;

TrackSection* NextIndexSection(TrackSection* section);
