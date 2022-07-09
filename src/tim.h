#ifndef _TIM_H_INCLUDED_
#define _TIM_H_INCLUDED_

#define CLUT4   0x00
#define CLUT8   0x01
#define TRUE15  0x10

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

#endif
