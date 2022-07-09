/* (C) Psygnosis 1994. By Jason Carl Denton
*/

/* Skeleton Structure ( Hierarchical Coordinate System )
 */

typedef struct Skeleton
{
    MATRIX relative; /* Relative rotation/translation matrix */
    MATRIX absolute; /* Absolute rotation/translation matrix */

    short update; /* Set if absolute matrix needs updating ( i.e. parent matrix has been changed ) */

    struct Skeleton* super; /* Parent Skeleton */

    /*TODO unused, remove from the custom file format*/
    struct Skeleton* sub; /* First Child Skeleton of this Skeleton */
    struct Skeleton* next; /* Next Child Skeleton of Parent Skeleton */
} Skeleton;

/* Primitive Structure Stub ( Structure varies with primitive type )
 */

typedef struct Primitive
{
    short type; /* Type of Primitive */
} Primitive;

/* Actual Primitive Structures
 */

/* FIXED COLOUR POLYGONS
 */

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
    VECTOR control1;
    VECTOR position;
    VECTOR control2;
    CVECTOR colour;
} Spline;

/* PRIMITIVE FLAGS */

#define SingleSided     0x0001
#define ShipEngine      0x0002
#define Translucent     0x0004

typedef union Poly
{
    char* ptr;
    Primitive* prim;

    F3* f3;
    FT3* ft3;
    F4* f4;
    FT4* ft4;
    G3* g3;
    GT3* gt3;
    G4* g4;
    GT4* gt4;
    SPR* spr;

    Spline* spline;
} Poly;

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

//#define TypeLSF3    12
//#define TypeLSFT3   13
//#define TypeLSF4    14
//#define TypeLSFT4   15
//#define TypeLSG3    16
//#define TypeLSGT3   17
//#define TypeLSG4    18
//#define TypeLSGT4   19

#define TypeSpline  20

/* Main Object Structure
 */

typedef struct Object
{
    char name[16];

    short vertexCount; /* Number of Vertices */
    SVECTOR* vertices; /* Pointer to 3D Points */

    short normalCount; /* Number of Normals */
    SVECTOR* normals; /* Pointer to 3D Normals */

    short primitiveCount; /* Number of Primitives */
    Primitive* primitives; /* Pointer to Z Sort Primitives */

    /*TODO remove unused for the custom file format*/
    long unused_pad;
    long unused_bspTree;

    Skeleton* skeleton; /* Pointer to Local Coordinates */

    long extent; /* Flags for object characteristics */

    /*TODO remove unused for the custom file format*/
    short unused_flags; /* Next object in list */

    struct Object* next; /* Next object in list */
} Object;

typedef union Prm
{
    char* ptr;
    short* sptr;
    long* lptr;
    Object* object;
    Skeleton* skeleton;
    SVECTOR* svector;
    Primitive* primitive;

    F3* f3;
    FT3* ft3;
    F4* f4;
    FT4* ft4;
    G3* g3;
    GT3* gt3;
    G4* g4;
    GT4* gt4;
    SPR* spr;
    Spline* spline;
} Prm;

#define CrtClipped  0x80000000
#define XLarge      0x01000000
#define YLarge      0x00800000
#define ZLarge      0x00400000
#define ZNegative   0x00040000
#define ZSmall      0x00020000
#define XClipped    0x00004000
#define YClipped    0x00002000
#define OutFog      0x00001000

void InitGameDisplay(void);
void InitMenuDisplay(int geomOffset);

void TransformStaticMenuObject(Object* object);
void TransformStaticObjectBodge(Object* object);
void TransformStaticObject(Object* object);
void SetSkeletonTransform(Skeleton* skeleton);
