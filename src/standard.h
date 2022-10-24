/* (C) Psygnosis 1994. By Jason Carl Denton
*/
#ifndef _STANDARD_H_INCLUDED_
#define _STANDARD_H_INCLUDED_

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef WIN32
    #include <windows.h>
#endif

#ifdef __3DO__
    #include <displayutils.h>
    #include <debug.h>
    #include <nodes.h>
    #include <kernelnodes.h>
    #include <list.h>
    #include <folio.h>
    #include <audio.h>
    #include <task.h>
    #include <kernel.h>
    #include <mem.h>
    #include <operamath.h>
    #include <semaphore.h>
    #include <io.h>
    #include <event.h>
    #include <controlpad.h>
    #include <graphics.h>
    #include <celutils.h>
    #include <timerutils.h>
    #include <hardware.h>

    #define CPU_BIG_ENDIAN
    #define USE_SHIP_LODS
    #define USE_DIV_TABLE
#else
    #include <stdio.h>

    #define USE_COMPRESSION
    #define USE_SHIP_AMBIENT
    #define MENU_HAS_QUIT
#endif

#ifndef __3DO__
typedef signed char         int8;
typedef unsigned char       uint8;
typedef short int           int16;
typedef unsigned short int  uint16;
typedef int                 int32;
typedef unsigned int        uint32;
#endif

#ifdef __3DO__
    #define ASSERT(x)
#else
    #define ASSERT(x) if (!(x)) __debugbreak()
#endif

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
    uint8 r, g, b, cd;
} CVECTOR;

typedef struct
{ /* 2D short vector */
    short vx, vy;
} DVECTOR;

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

unsigned short GetKeyNoBlock(void);

int32 osGetSystemTimeMS(void);
void osQuit(void);
void osJoyVibrate(int index, int L, int R);
long osLoadFile(const char* name, char* buffer);

void showLoading(int32 track);

enum ViewDir
{
    VIEW_DIR_NORTH,
    VIEW_DIR_SOUTH,
    VIEW_DIR_WEST,
    VIEW_DIR_EAST,
    VIEW_DIR_ALL
};

enum GameLoop
{
    GAME_LOOP_TITLE_INIT,
    GAME_LOOP_TITLE,
    GAME_LOOP_MENU_INIT,
    GAME_LOOP_MENU,
    GAME_LOOP_RACE_INIT,
    GAME_LOOP_RACE
};

#define GAME_LOOP_MAIN  GAME_LOOP_MENU_INIT

typedef struct Texture
{
    uint16 width;
    uint16 height;
} Texture;

#endif