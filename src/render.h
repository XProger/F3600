#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_

#include "standard.h"

/* PrimitivePSX Structure Stub ( Structure varies with primitive type )
 */

typedef struct PrimitivePSX
{
    short type; /* Type of PrimitivePSX */
} PrimitivePSX;

/* Actual PrimitivePSX Structures
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

/* PRIMITIVE FLAGS */

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
    MATRIX absolute; /* Absolute rotation/translation matrix */

    short update; /* Set if absolute matrix needs updating ( i.e. parent matrix has been changed ) */

    struct SkeletonPSX* super; /* Parent Skeleton */

    /*TODO unused, remove from the custom file format*/
    struct SkeletonPSX* sub; /* First Child Skeleton of this Skeleton */
    struct SkeletonPSX* next; /* Next Child Skeleton of Parent Skeleton */
} SkeletonPSX;

/* TODO dumbest thing I've ever seen */
typedef struct ObjectPSX
{
   char              name[16];
   short             vertexCount;      /* Number of Vertices */
   vec4s*            vertices;         /* Pointer to 3D Points */
   short             normalCount;      /* Number of Normals */
   vec4s*            normals;          /* Pointer to 3D Normals */
   short             primitiveCount;   /* Number of Primitives */
   PrimitivePSX*     primitives;       /* Pointer to Z Sort Primitives */
   long              pad;
   void*             bspTree;          /* Pointer to BSP Tree Primitives */
   SkeletonPSX*      skeleton;         /* Pointer to Local Coordinates */
   long              extent;           /* Flags for object characteristics */
   short             flags;            /* Next object in list */
   struct ObjectPSX* next;             /* Next object in list */
} ObjectPSX;

typedef union PrmPSX
{
    char* ptr;
    short* sptr;
    long* lptr;
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

// optimized Object & Primitive
#define NO_TEXTURE 0xFFFF

#define FACE_TEXTURE(face) (face->flags & 0xFFFF)
#ifdef __3DO__
#define FACE_QUAD(face) ((face->index >> 24) != ((face->index >> 16) & 0xFF))
#else
#define FACE_QUAD(face) (face->index[2] != face->index[3])
#endif
#define FACE_FLAG_ENGINE (1 << 31)

#define SPRITE_TEXTURE(sprite) (sprite->flags & 0xFFFF)
#define SPRITE_FLIP(face) (face->flags & (1 << 31))

enum ObjType
{
    OBJ_TYPE_SKY,
    OBJ_TYPE_START,
    OBJ_TYPE_LOST,
    OBJ_TYPE_RED,
    OBJ_TYPE_DONKEY,
    OBJ_TYPE_MAX
};

typedef struct ObjFace
{
#ifdef __3DO__
    uint32 flags;
    uint32 index;
#else
    uint32 flags;
    uint16 index[4];
    uint8 uv[4][2];
    uint32 color[4];
#endif
} ObjFace;

typedef struct ObjSprite
{
#ifdef __3DO__
    uint32 flags;
    uint16 index;
    uint16 size;
#else
    uint32 flags;
    uint32 index;
    int16 width;
    int16 height;
    uint32 color;
#endif
} ObjSprite;

typedef struct Object
{
    int32 type;
    int32 vCount;
    int32 fCount;
    int32 sCount;
    vec3i pos;
    int32 extent;

    vec3i* vertices;
    struct ObjFace* faces;
    struct ObjSprite* sprites;
} Object;

enum FxType
{
    FX_FIRE,
    FX_FIRE_WHITE,
    FX_SMOKE,
    FX_EBOLT,
    FX_HALO,
    FX_GREENY,
    FX_MAX
};

typedef struct Effect
{
    vec3i pos;
    vec3i vel;
    int32 count;
    short type;
    short size;
    struct Effect* next;
    struct Effect* prev;
} Effect;

#define CrtClipped  0x80000000
#define XLarge      0x01000000
#define YLarge      0x00800000
#define ZLarge      0x00400000
#define ZNegative   0x00040000
#define ZSmall      0x00020000
#define XClipped    0x00004000
#define YClipped    0x00002000
#define OutFog      0x00001000

// Tim
typedef struct TimHeader
{
    long id;
    long flags;
} TimHeader;

typedef struct Tim4
{
    TimHeader header;

    long clutBytes;
    short clutX;
    short clutY;
    short clutW;
    short clutH;
    uint16 clut[16];

    long textureBytes;
    short textureX;
    short textureY;
    short textureW;
    short textureH;
} Tim4;

typedef struct Tim8
{
    TimHeader header;

    long clutBytes;
    short clutX;
    short clutY;
    short clutW;
    short clutH;
    uint16 clut[256];

    long textureBytes;
    short textureX;
    short textureY;
    short textureW;
    short textureH;
} Tim8;

typedef struct Tim16
{
    TimHeader header;

    long textureBytes;
    short textureX;
    short textureY;
    short textureW;
    short textureH;
} Tim16;

typedef union Tim
{
    TimHeader header;
    Tim4 tim4;
    Tim8 tim8;
    Tim16 tim16;
} Tim;

#define TIM_4    0
#define TIM_8    1
#define TIM_16   2

#define TimType(t) (t->header.flags & 7)

// textures format for 3DO
#define TEX_TYPE_4  0
#define TEX_TYPE_16 1

#define INIT_TEX_INFO(w, h, t)  ((w) | ((h) << 9) | ((t) << 18))
#define GET_TEX_WIDTH(info)     ((info) & 0x1FF)
#define GET_TEX_HEIGHT(info)    (((info) >> 9) & 0x1FF)
#define GET_TEX_TYPE(info)      ((info) >> 18)

typedef struct Texture
{
    uint16 width;
    uint16 height;
#ifdef __3DO__
    uint16* plut;
    uint8* data;
#else
    uint32 id;
    float iw;
    float ih;
    #ifdef _DEBUG
        Tim* tim;
    #endif
#endif
} Texture;

typedef struct
{
    int32 x;
    int32 y;
    int32 w;
    int32 h;
    const Texture* texture;
} Glyph;

typedef struct
{
    int32 x, y;
    int32 w, h;
    uint16 u0, v0;
    uint16 u1, v1;
    uint32 color;
    const Texture* texture;
} Quad;

typedef struct
{
    vec3i v[3];
    const Texture* texture;
    int32 intensity;
} Shadow;

typedef struct
{
    int16 x0, y0;
    int16 x1, y1;
    int16 x2, y2;
    int16 x3, y3;
    uint32 color[4];
} ColorQuad;

typedef struct
{
    int32 x, y, z;
    int32 size;
    const Texture* texture;
} Billboard;

typedef enum
{
    BLEND_NONE,
    BLEND_ALPHA,
    BLEND_ADD,
    BLEND_SUB,
    BLEND_MUL
} BlendMode;

typedef enum
{
    RENDER_MODE_2D,
    RENDER_MODE_3D
} RenderMode;

extern int gFrameIndex;
extern int gLastFrameIndex;

void renderInit(void);
void renderFree(void);
void renderSwap(void);
void renderResize(int width, int height);
void renderOffset(int x, int y);
void renderClear(void);
void renderGlyph(const Glyph* g, int32 x, int32 y, uint32 color);
void renderImage(const Texture* t, int32 x, int32 y);
void renderBackground(const Texture* t);
void renderShadow(int32 type, int32 intensity, int32 px, int32 py, int32 pz, int32 dx, int32 dy, int32 dz, int32 rx, int32 ry, int32 rz);
void renderColorQuad(const ColorQuad* q);
void renderBillboard(const Billboard* b);
void renderBackFill(int x, int y, int w, int h);

void renderPrepare(void);
void renderFrameBegin(void);
void renderFrameEnd(void);
void renderTrack(void);
void renderEffects(const Effect* fx);
void renderObject(const Object* object);
void renderShip(const Object* object, uint32 color);
void renderShield(const Object* object, int32 counter);
void renderSky(const Object* object);
void renderSetMode(int renderMode);
void renderSetBlend(int blend);
void renderDepthWrite(int enable);
void renderNextLayer(void);
Texture* renderCreateTexture(uint8* ptr);
void renderDeleteTexture(Texture* texture);

#endif
