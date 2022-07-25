#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_

#include "standard.h"
#include "tim.h"

typedef struct
{
    int32 x, y;
    int32 w, h;
    uint16 u0, v0;
    uint16 u1, v1;
    const Texture* texture;
} Quad;

typedef struct
{
    int16 x0, y0;
    int16 x1, y1;
    int16 x2, y2;
    int16 x3, y3;
    uint8 r0, g0, b0;
    uint8 r1, g1, b1;
    uint8 r2, g2, b2;
    uint8 r3, g3, b3;
} ColorQuad;

typedef struct
{
    int32 x, y, z;
    int32 size;
    const Texture* texture;
} Billboard;

extern int gFrameIndex;
extern int gLastFrameIndex;

void SetGeomOffset(long ofx, long ofy);

void renderInit(void);
void renderFree(void);
void renderSwap(void);
void renderResize(int width, int height);
void renderOffset(int x, int y);
void renderClear(void);
void renderQuad(const Quad* q);
void renderColorQuad(const ColorQuad* q);
void renderBillboard(const Billboard* b);

void renderPrepare(void);
void renderFrameBegin(void);
void renderFrameEnd(void);
void renderSetMatrix(Skeleton* skeleton, int x, int y, int z);
void renderTrack(void);
void renderObject(const Object* object);
void renderSky(const Object* object);
void renderBegin3D(void);
void renderEnd3D(void);
void renderTransparent(int enable);
void renderDepthWrite(int enable);
Texture* renderCreateTexture(Tim* tim);

#endif
