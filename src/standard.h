
/* (C) Psygnosis 1994. By Jason Carl Denton
*/

#include "wipesrc\psx26\include\sys\types.h"
#include "wipesrc\psx26\include\libgte.h"
#include "wipesrc\psx26\include\libgpu.h"
#include "wipesrc\psx26\include\libsn.h"
#include "wipesrc\psx26\include\libetc.h"
#include "wipesrc\psx26\include\libcd.h"
#include "wipesrc\psx26\include\libspu.h"


#define  SLAVE       1
#define  MASTER      0
#define  SINGLE      2

#define  PAUSE    0x30
#define  CONT     0x31
#define  RETIRE   0x32
#define  QUIT     0x33
#define  NULLDATA 0x34

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;

typedef char* Ptr;

#ifndef NULL
#define  NULL     0
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

typedef struct {
   int   width;
   int   height;
   int   vram_xpos;
   int   vram_ypos;
   char  *bitmap;
   char  *palette;
} bm;


void  osQuit(void);

int   JJSLoadSprites(void);
void  changeScreen(void);
void  setRenderBegin(void);
void  setRenderEnd(void);
int   ProcessMessages(void);
void  JJSDrawPolyF3(POLY_F4* p);
void  JJSDrawPolyFT3(POLY_FT3* p);
void  JJSDrawPolyG3(POLY_G3* p);
void  JJSDrawPolyGT3(POLY_GT3* p);
void  JJSDrawPolyGT4(POLY_GT4* p);
void  JJSDrawPolyF4(POLY_F4* p);
void  JJSDrawPolyFT4(POLY_FT4* p);
void  JJSDrawPolyG4(POLY_G4* p);

unsigned short GetKeyNoBlock();
