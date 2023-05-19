/* (C) Psygnosis 1994. By Jason Carl Denton
*/
#ifndef _STANDARD_H_INCLUDED_
#define _STANDARD_H_INCLUDED_

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define USE_FRAME_LERP

#if defined(__3DO__)
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

    #define SHIP_VIS_DIST       (32 << 10)
    #define USE_FONT_SCALE
    #define USE_VERTEX_COLOR
    #define USE_COMPRESSION
    #define USE_SHIP_AMBIENT
    #define USE_SHIP_GHOST
    #define MENU_HAS_QUIT
#endif

#ifdef __PSP__
    #define USE_OLD_SHIELD
    #undef USE_FONT_SCALE
    #undef MENU_HAS_QUIT
#endif

#ifdef __PS3__
    #undef MENU_HAS_QUIT
    #define CPU_BIG_ENDIAN
    #define USE_WIDE_STRETCH

    #define VRAM_ALIGN  128
#endif

#ifdef __X360__
    #undef MENU_HAS_QUIT
    #define CPU_BIG_ENDIAN
    #define USE_WIDE_STRETCH
#endif

#ifndef SHIP_VIS_DIST
    #define SHIP_VIS_DIST (16 << 10)
#endif

#ifndef MAX_PLAYERS
    #define MAX_PLAYERS 4
#endif

#if MAX_PLAYERS > 1
    #define MENU_HAS_MULTIPLAYER
#endif

#ifndef __3DO__
typedef signed char         int8;
typedef unsigned char       uint8;
typedef short int           int16;
typedef unsigned short int  uint16;
typedef int                 int32;
typedef unsigned int        uint32;
#endif

#ifdef _DEBUG
    #if defined(__LINUX__) || defined(__MAC__)
        #include <signal.h>
        #define __debugbreak() raise(SIGTRAP)
    #endif

    #define LOG(...)        printf(__VA_ARGS__)
    #define ASSERT(expr)    if (!(expr)) { LOG("ASSERT:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, #expr); __debugbreak(); }
#else
    #define LOG(a)          //printf(__VA_ARGS__); fflush(stdout)
    #define ASSERT(expr)    //if (!(expr)) { LOG("ASSERT:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, #expr); }
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

#define PI      3.14159265358979323846f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

#ifdef __cplusplus
extern "C" {
#endif

int32 osGetSystemTimeMS(void);
void osQuit(void);
void osJoyVibrate(int32 index, int32 L, int32 R);
int32 osLoadFile(const char* name, void* buffer);
void soundInit(void);

#ifdef __cplusplus
}
#endif

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
    GAME_LOOP_RACE,
    GAME_LOOP_MAIN = GAME_LOOP_MENU_INIT
};

#endif
