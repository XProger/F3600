typedef struct PrimitivePSX
{
    short type; /* Type of PrimitivePSX */
} PrimitivePSX;

typedef struct F3
{
    short type; /* Type of primitive */
    short flag;
    short coords[3]; /* Indices of the coords */
    short pad1;
    CVECTOR colour;
} F3;

typedef struct FT3
{
    short type; /* Type of primitive */
    short flag;
    short coords[3]; /* Indices of the coords */
    short texture;
    short cba;
    short tsb;
    unsigned char u0;
    unsigned char v0;
    unsigned char u1;
    unsigned char v1;
    unsigned char u2;
    unsigned char v2;
    short pad1;
    CVECTOR colour;
} FT3;

typedef struct F4
{
    short type; /* Type of primitive */
    short flag;
    short coords[4]; /* Indices of the coords */
    CVECTOR colour;
} F4;

typedef struct FT4
{
    short type; /* Type of primitive */
    short flag;
    short coords[4]; /* Indices of the coords */
    short texture;
    short cba;
    short tsb;
    unsigned char u0;
    unsigned char v0;
    unsigned char u1;
    unsigned char v1;
    unsigned char u2;
    unsigned char v2;
    unsigned char u3;
    unsigned char v3;
    short pad1;
    CVECTOR colour;
} FT4;

typedef struct G3
{
    short type; /* Type of primitive */
    short flag;
    short coords[3]; /* Indices of the coords */
    short pad1;
    CVECTOR colour[3];
} G3;

typedef struct GT3
{
    short type; /* Type of primitive */
    short flag;
    short coords[3]; /* Indices of the coords */
    short texture;
    short cba;
    short tsb;
    unsigned char u0;
    unsigned char v0;
    unsigned char u1;
    unsigned char v1;
    unsigned char u2;
    unsigned char v2;
    short pad1;
    CVECTOR colour[3];
} GT3;

typedef struct G4
{
    short type; /* Type of primitive */
    short flag;
    short coords[4]; /* Indices of the coords */
    CVECTOR colour[4];
} G4;

typedef struct GT4
{
    short type; /* Type of primitive */
    short flag;
    short coords[4]; /* Indices of the coords */
    short texture;
    short cba;
    short tsb;
    unsigned char u0;
    unsigned char v0;
    unsigned char u1;
    unsigned char v1;
    unsigned char u2;
    unsigned char v2;
    unsigned char u3;
    unsigned char v3;
    short pad1;
    CVECTOR colour[4];
} GT4;

typedef struct LSF3
{
    short type;
    short flag;
    short coords[3];
    short normal;
    CVECTOR colour;
} LSF3;

typedef struct LSFT3
{
    short type;
    short flag;
    short coords[3];
    short normal;
    short texture;
    short cba;
    short tsb;
    char u0;
    char v0;
    char u1;
    char v1;
    char u2;
    char v2;
    CVECTOR colour;
} LSFT3;

typedef struct LSF4
{
    short type;
    short flag;
    short coords[4];
    short normal;
    short pad1;
    CVECTOR colour;
} LSF4;

typedef struct LSFT4
{
    short type;
    short flag;
    short coords[4];
    short normal;
    short texture;
    short cba;
    short tsb;
    char u0;
    char v0;
    char u1;
    char v1;
    char u2;
    char v2;
    char u3;
    char v3;
    CVECTOR colour;
} LSFT4;

typedef struct LSG3
{
    short type;
    short flag;
    short coords[3];
    short normals[3];
    CVECTOR colour[3];
} LSG3;

typedef struct LSGT3
{
    short type;
    short flag;
    short coords[3];
    short normals[3];
    short texture;
    short cba;
    short tsb;
    char u0;
    char v0;
    char u1;
    char v1;
    char u2;
    char v2;
    CVECTOR colour[3];
} LSGT3;

typedef struct LSG4
{
    short type;
    short flag;
    short coords[4];
    short normals[4];
    CVECTOR colour[4];
} LSG4;

typedef struct LSGT4
{
    short type;
    short flag;
    short coords[4];
    short normals[4];
    short texture;
    short cba;
    short tsb;
    char u0;
    char v0;
    char u1;
    char v1;
    char u2;
    char v2;
    char u3;
    char v3;
    short pad1;
    CVECTOR colour[4];
} LSGT4;

typedef struct SPR
{
    short type;
    short flag;
    short coord;
    short width;
    short height;
    short texture;
    CVECTOR colour;
} SPR;

typedef struct Spline
{
    short type; /* Type of primitive */
    short flag;
    vec4i control1;
    vec4i position;
    vec4i control2;
    CVECTOR colour;
} Spline;

typedef struct PointLight
{
    short type;
    short flag;
    vec4i position;
    CVECTOR colour;
    short startFalloff;
    short endFalloff;
} PointLight;

typedef struct SpotLight
{
    short type;
    short flag;
    vec4i position;
    vec4s direction;
    CVECTOR colour;
    short startFalloff;
    short endFalloff;
    short coneAngle;
    short spreadAngle;
} SpotLight;

typedef struct InfiniteLight
{
    short type;
    short flag;
    vec4s direction;
    CVECTOR colour;
} InfiniteLight;

#define SingleSided     0x0001
#define ShipEngine      0x0002
#define Translucent     0x0004

#define TypeF3      1
#define TypeFT3     2
#define TypeF4      3
#define TypeFT4     4
#define TypeG3      5
#define TypeGT3     6
#define TypeG4      7
#define TypeGT4     8

//#define TypeLF2     9
#define TypeTSPR    10
#define TypeBSPR    11

#define TypeLSF3    12
#define TypeLSFT3   13
#define TypeLSF4    14
#define TypeLSFT4   15
#define TypeLSG3    16
#define TypeLSGT3   17
#define TypeLSG4    18
#define TypeLSGT4   19

#define TypeSpline  20

#define TypeInfiniteLight 21
#define TypePointLight    22
#define TypeSpotLight     23

/* Main Object Structure
 */

typedef struct SkeletonPSX
{
    MATRIX relative; /* Relative rotation/translation matrix */
    MATRIX absolute_dummy; /* Absolute rotation/translation matrix */

    short update_dummy; /* Set if absolute matrix needs updating ( i.e. parent matrix has been changed ) */
    short padding_dummy;

    int super_dummy; /* Parent Skeleton */

    /*TODO unused, remove from the custom file format*/
    int sub_dummy; /* First Child Skeleton of this Skeleton */
    int next_dummy; /* Next Child Skeleton of Parent Skeleton */
} SkeletonPSX;

/* TODO dumbest thing I've ever seen */
typedef struct ObjectPSX
{
   char  name[16];
   short vertexCount;      /* Number of Vertices */
   int   vertices_dummy;   /* Pointer to 3D Points */
   short normalCount;      /* Number of Normals */
   int   normals_dummy;    /* Pointer to 3D Normals */
   short primitiveCount;   /* Number of Primitives */
   int   primitives_dummy; /* Pointer to Z Sort Primitives */
   int   padding_dummy;
   int   bspTree_dummy;    /* Pointer to BSP Tree Primitives */
   int   skeleton_dummy;   /* Pointer to Local Coordinates */
   int   extent;           /* Flags for object characteristics */
   short flags_dummy;      /* Next object in list */
   int   next_dummy;       /* Next object in list */
} ObjectPSX;

typedef union PrmPSX
{
    ObjectPSX* object;
    SkeletonPSX* skeleton;
    vec4s* svector;
    PrimitivePSX* primitive;

    F3* f3;
    FT3* ft3;
    F4* f4;
    FT4* ft4;
    G3* g3;
    GT3* gt3;
    G4* g4;
    GT4* gt4;

    SPR* spr;

    LSF3* lsf3;
    LSFT3* lsft3;
    LSF4* lsf4;
    LSFT4* lsft4;
    LSG3* lsg3;
    LSGT3* lsgt3;
    LSG4* lsg4;
    LSGT4* lsgt4;

    Spline* spline;

    PointLight* pointLight;
    SpotLight* spotLight;
    InfiniteLight* infiniteLight;
} PrmPSX;

typedef struct TrackSectionPSX
{
    int32 junctionIndex;
    int32 prevSectionIndex;
    int32 nextSectionIndex;

    vec3i centre;

    int16 unused_version;
    int16 unused_pad;

    int32 objects;
    int16 objectsCount;
    int16 unused_padding_0;

    int32 northViewSection[3];
    int32 southViewSection[3];
    int32 eastViewSection[3];
    int32 westViewSection[3];
    int32 allViewSection[3];

    int16 northViewCount[3];
    int16 southViewCount[3];
    int16 eastViewCount[3];
    int16 westViewCount[3];
    int16 allViewCount[3];

    int16 unused[8];

    uint16 faceStart;
    uint16 faceCount;

    uint16 globalRadius;
    uint16 localRadius;

    uint16 flags;
    uint16 secNo;

    uint16 unused_padding_1;
} TrackSectionPSX;

typedef struct
{
    short tex16[16];
    short tex4[4];
    short tex1[1];
} TexTemplate;

extern TexTemplate* ctrltexTemplate;
extern int32 ctrlnumLib;
extern int32 ctrllibraryTextures;

extern Texture* gEffectTextures;

void LoadCompressedTextureSequence(const char* fileName, int filter);
Texture* LoadTexture(const char* fileName);

Object* LoadPrm(const char* name, Texture* texturePtr, int32* count, uint8* types);

void LoadCommon(void);
void LoadMainMenu(void);
void LoadScene(int32 trackNum);
void LoadWinLoseTex(const char* fileName);