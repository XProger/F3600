/* (C) Psygnosis 1994. By Jason Carl Denton
*/
#ifndef _STANDARD_H_INCLUDED_
#define _STANDARD_H_INCLUDED_

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef __WIN32__
    #include <windows.h>
#endif

#define USE_FRAME_LERP

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
#else
    #include <stdio.h>

    #define USE_VERTEX_COLOR
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

#if 1
    #if __LINUX__
        #include <signal.h>
        #define __debugbreak() raise(SIGTRAP)
    #endif

    #define LOG(...)        printf(__VA_ARGS__)
    #define ASSERT(expr)    if (!(expr)) { LOG("ASSERT:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, #expr); __debugbreak(); }
#else
    #define LOG(...)
    #define ASSERT(x)
#endif

#define COUNT(x) (sizeof(x) / sizeof(x[0]))

typedef struct
{
    int16 m[3][3];
    int32 t[3];
} MATRIX;

typedef struct
{
    uint8 r, g, b, cd;
} CVECTOR;

typedef struct vec2s
{
    int16 x, y;
} vec2s;

typedef struct vec3s
{
    int16 x, y, z;
} vec3s;

typedef struct vec4s
{
    int16 x, y, z, w;
} vec4s;

typedef struct vec3i
{
    int32 x, y, z;
} vec3i;

typedef struct vec4i
{
    int32 x, y, z, w;
} vec4i;

#ifndef NULL
    #define NULL 0
#endif

#define randy() (rand() & 0x7FFF)

unsigned short GetKeyNoBlock(void);

int32 osGetSystemTimeMS(void);
void osQuit(void);
void osJoyVibrate(int index, int L, int R);
int32 osLoadFile(const char* name, void* buffer);

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

#endif
