
/* (C) Psygnosis 1994. By Jason Carl Denton
*/


#include "wipesrc\psx26\include\sys\types.h"
#include "wipesrc\psx26\include\libgte.h"
#include "wipesrc\psx26\include\libgpu.h"
#include "wipesrc\psx26\include\libsn.h"
#include "wipesrc\psx26\include\libetc.h"
#include "wipesrc\psx26\include\libcd.h"
#include "wipesrc\psx26\include\libspu.h"

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;

typedef char* Ptr;

#define PAL       0

#ifndef NULL
#define  NULL     0
#endif

extern   ulong FastRam; // ajs (well ajy actually)

/* Functions in the library but not defined in the headers! */


/*
   rsin() and rcos()

   angle  : 4096 == 360 degrees
   result : 4096 == 1


long rsin( long angle );
long rcos( long angle );
*/

int   JJSLoadSprites(void);
int   randy(void);
void  centreMouse(void);
void  changeScreen(void);
int   LockBackBuffer(void);
void  UnlockBackBuffer(void);
void  setRenderBegin(void);
void  setRenderEnd(void);
int   ProcessMessages(void);
void  sendList(void);
void  JJSDrawPolyF3(POLY_F4* p);
void  JJSDrawPolyFT3(POLY_FT3* p);
void  JJSDrawPolyG3(POLY_G3* p);
void  JJSDrawPolyGT3(POLY_GT3* p);
void  JJSDrawPolyGT4(POLY_GT4* p);
void  JJSDrawPolyF4(POLY_F4* p);
void  JJSDrawPolyFT4(POLY_FT4* p);
void  JJSDrawPolyG4(POLY_G4* p);
void  WinQuit(void);
int   JJSJoyConfig(void);
