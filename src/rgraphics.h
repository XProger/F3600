//================================================================================================================================
// rGraphics.h
// -----------
//
//================================================================================================================================

#ifndef _BS_rGraphics
#define _BS_rGraphics

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

struct {
   float x0,y0,z0;
   float x1,y1,z1;
   float x2,y2,z2;
} rTriangle;

#define MAT_ENVIRONMENT (1<<25)
#define MAT_ALPHA       (1<<26)
#define MAT_COLLIDABLE  (1<<27)
#define MAT_DOUBLESIDED (1<<28)
#define MAT_WALL        (1<<29)
#define MAT_GRASS       (1<<30)
#define MAT_TRACK       (1<<31)


#define MAT_TCLAMP 1
#define MAT_SCLAMP 2
#define MAT_DECAL  4

struct {
   DWORD flags;
   DWORD textureHandle;
} BorisMaterial;

#define D3DFVF_TLVERTEX2 ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | \
                          D3DFVF_TEX2)

typedef struct _D3DTLVERTEX2 {
    D3DVALUE    sx;
    D3DVALUE    sy;
    D3DVALUE    sz;
    D3DVALUE    rhw;        /* Reciprocal of homogeneous w */
    D3DCOLOR    color;          /* Vertex color */
    D3DCOLOR    specular;       /* Specular component of vertex */
    D3DVALUE    tu1;             /* Texture coordinates */
    D3DVALUE    tv1;
    D3DVALUE    tu2;             /* Texture coordinates */
    D3DVALUE    tv2;
} D3DTLVERTEX2, *LPD3DTLVERTEX2;

BOOL renderInitialise(HWND hWnd, BOOL b_fullScreen, BOOL Voodoo, BOOL softwareOnly);
void renderFlipSurface(void);
HRESULT Cleanup3DEnvironment();

#endif   // _BS_rGraphics

//================================================================================================================================
//END OF FILE
//================================================================================================================================



