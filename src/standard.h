/* (C) Psygnosis 1994. By Jason Carl Denton
*/
#ifndef _STANDARD_H_INCLUDED_
#define _STANDARD_H_INCLUDED_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
typedef unsigned short ushort; /* sys III compat */

typedef struct
{
    short m[3][3]; /* 3x3 rotation matrix */
    long t[3]; /* transfer vector */
} MATRIX;

typedef struct
{ /* long word type 3D vector */
    long vx, vy;
    long vz, pad;
} VECTOR;

typedef struct
{ /* short word type 3D vector */
    short vx, vy;
    short vz, pad;
} SVECTOR;

typedef struct
{ /* color type vector */
    u_char r, g, b, cd;
} CVECTOR;

typedef struct
{ /* 2D short vector */
    short vx, vy;
} DVECTOR;

#define MASTER  0
#define SLAVE   1
#define SINGLE  2

#define PAUSE       0x30
#define CONT        0x31
#define RETIRE      0x32
#define QUIT        0x33
#define NULLDATA    0x34

typedef unsigned char   uchar;
typedef unsigned long   ulong;
typedef unsigned int    uint;

typedef signed char         int8;
typedef unsigned char       uint8;
typedef short int           int16;
typedef unsigned short int  uint16;
typedef int                 int32;
typedef unsigned int        uint32;

typedef char* Ptr;

#ifndef NULL
    #define NULL 0
#endif

#define randy() rand()

extern int gPad;

/* Functions in the library but not defined in the headers! */

/*
   rsin() and rcos()

   angle  : 4096 == 360 degrees
   result : 4096 == 1


long rsin( long angle );
long rcos( long angle );
*/

typedef struct
{
    int width;
    int height;
    int vram_xpos;
    int vram_ypos;
    char* bitmap;
    char* palette;
} bm;

int JJSLoadSprites(void);
void changeScreen(void);
void setRenderBegin(void);
void setRenderEnd(void);
int ProcessMessages(void);

unsigned short GetKeyNoBlock();

void osQuit(void);
void osJoyVibrate(int index, int L, int R);

typedef struct
{
    int x, y, w, h;
} GRECT;

typedef struct
{
    unsigned short red   : 5;
    unsigned short green : 5;
    unsigned short blue  : 5;
    unsigned short trans : 1;
} CLUT;

#define ShipForwards    1
#define ShipBackwards   2
#define ShipRight       4
#define ShipLeft        8

enum GameLoop
{
    GAME_LOOP_TITLE_INIT,
    GAME_LOOP_TITLE,
    GAME_LOOP_MENU_INIT,
    GAME_LOOP_MENU,
    GAME_LOOP_RACE_INIT,
    GAME_LOOP_RACE
};

#endif