#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_

#include "standard.h"
#include "tim.h"

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
    int32 x0, y0, z0;
    int32 x1, y1, z1;
    int32 x2, y2, z2;
    int32 x3, y3, z3;
    const Texture* texture;
    int32 intensity;
} Shadow;

typedef struct
{
    int16 x0, y0;
    int16 x1, y1;
    int16 x2, y2;
    int16 x3, y3;
    CVECTOR color[4];
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
void renderShadow(const Shadow* s);
void renderColorQuad(const ColorQuad* q);
void renderBillboard(const Billboard* b);
void renderBackFill(int x, int y, int w, int h);

void renderPrepare(void);
void renderFrameBegin(void);
void renderFrameEnd(void);
void renderSetMatrix(const MATRIX* matrix, int x, int y, int z);
void renderSetSkeleton(Skeleton* skeleton, int x, int y, int z);
void renderTrack(void);
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
