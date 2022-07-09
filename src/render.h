#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_

#include "standard.h"
#include "tim.h"

typedef struct Quad
{
    signed short x, y;

    unsigned short w, h;

    unsigned short u0, v0;
    unsigned short u1, v1;

    const Texture* texture;
} Quad;

extern int gFrameIndex;
extern int gLastFrameIndex;

void SetGeomOffset(long ofx, long ofy);

void renderInit(void);
void renderFree(void);
void renderSwap(void);
void renderResize(int width, int height);
void renderOffset(int x, int y);
void renderClear(void);
void renderQuad(const Quad* spr);

void renderPrepare(void);
void renderTrack(void);
void renderObject(const Object* object);
void renderSky(const Object* object);
void renderBegin3D(void);
void renderEnd3D(void);
void renderTransparent(int enable);
Texture* renderCreateTexture(Tim* tim);

#endif
