#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_

#include "standard.h"

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
    int16 vCount;
    int16 fCount;
    int16 sCount;
    int16 extent;
    vec3i pos;

    vec3s* vertices;
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
    int16 type;
    int16 size;
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
    uint32 id;
    uint32 flags;
} TimHeader;

typedef struct Tim4
{
    TimHeader header;

    uint32 clutBytes;
    int16 clutX;
    int16 clutY;
    int16 clutW;
    int16 clutH;
    uint16 clut[16];

    uint32 textureBytes;
    int16 textureX;
    int16 textureY;
    int16 textureW;
    int16 textureH;
} Tim4;

typedef struct Tim8
{
    TimHeader header;

    uint32 clutBytes;
    int16 clutX;
    int16 clutY;
    int16 clutW;
    int16 clutH;
    uint16 clut[256];

    uint32 textureBytes;
    int16 textureX;
    int16 textureY;
    int16 textureW;
    int16 textureH;
} Tim8;

typedef struct Tim16
{
    TimHeader header;

    uint32 textureBytes;
    int16 textureX;
    int16 textureY;
    int16 textureW;
    int16 textureH;
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

void renderInit(void);
void renderFree(void);
void renderSetVSync(int32 enable);
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
void renderTargetIcon(Texture* tex);
void renderSetMode(int renderMode);
void renderSetBlend(int blend);
void renderDepthWrite(int enable);
void renderNextLayer(void);
Texture* renderCreateTexture(uint8* ptr);
void renderDeleteTexture(Texture* texture);

#endif
