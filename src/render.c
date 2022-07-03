#include <windows.h>
#include <gl/GL.h>
#include <gl/glu.h>

#include "standard.h"
#include "decl.h"
#include "object.h"
#include "skeleton.h"
#include "track.h"
#include "tim.h"
#include "ttf.h"
#include "render.h"
#include "utils.h"

extern Texture* TextureTable[];

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

extern short WinWidth;
extern short WinHeight;

extern const TrackCamera ctrlcamera;

long xOffset;
long yOffset;

int gWidth, gHeight;

#ifndef GL_UNSIGNED_SHORT_5_6_5
    #define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif

#ifndef GL_UNSIGNED_SHORT_5_5_5_1
    #define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#endif

#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
    #define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#endif

#ifndef GL_CLAMP_TO_EDGE
    #define GL_CLAMP_TO_EDGE 0x812F
#endif

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

short texpage = -1;

extern short trackNum;

void LoadPilotIntoVram(char* filename);

char globalPilot[255] = { 0 };

#define TEXTURESJR 2621440

typedef struct
{
    GLuint Texture;
} TextureGL;

TextureGL gTex[500] = { 0 };

char tempTex[320 * 240 * 2]; /*TODO*/
char* TextureBuffer;

/*TODO WTF? swap 3 and 10? */
char JJSTextureTable[] = { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 16, 17, 18, 19, 20, 11, 20, 12, 13, 21, 14, 15, 16, 17, 18, 19 };
char JJSTextureMenuTable[] = { 0, 1, 2, 3, 4, 0, 1, 2, 10, 4, 5, 6, 7, 8, 9, 3, 16, 17, 18, 19, 20, 11, 20, 12, 13, 21, 14, 15, 16, 17, 18, 19 };

int menuTexturesIn = 0;
short PageXOff, PageYOff;

typedef struct
{
    long id;
    long flags;
    long bnum;
    short textureX, textureY;
    short textureH, textureW;
} TimData;

#define BLACK_NOT_DRAWN 1
#define BLACK1_TRANSL 2
#define BLACK2_TRANSL 3
#define BLACK3_TRANSL 4
#define BLACK4_TRANSL 5
#define BLACK5_TRANSL 6
#define BLACK6_TRANSL 7
#define WHITE1_TRANSL 8
#define WHITE2_TRANSL 9
#define WHITE3_TRANSL 10
#define WHITE4_TRANSL 11
#define WHITE5_TRANSL 12
#define WHITE6_TRANSL 13
#define WHITE7_TRANSL 14
#define WHITE8_TRANSL 15
#define RED_TRANSL 16
#define RED_LIGHT_FADE 16
#define BRIGHT_RED_TRANSL 17
#define YELLOW_TRANSL 18
#define YELLOW_WHITE_TRANSL 19
#define BLUE_TRANSL 20
#define PURPLE_TRANSL 21
#define UNUSED1_TRANSL 22
#define UNUSED2_TRANSL 23
#define UNUSED3_TRANSL 24
#define UNUSED4_TRANSL 25
#define ATTRACT 1

void JJSReadTIM(char* filename, char* location, TimData* tim);

short GetThePage(short page, short u, short v)
{
    short newpage;
    char* tex = menuTexturesIn ? JJSTextureMenuTable : JJSTextureTable;

    newpage = tex[page & 0x1f];
    if (newpage > 4)
        newpage += 400;
    else if (!menuTexturesIn)
    {
        PageXOff = u >> 5;
        PageYOff = v >> 5;
        newpage *= 64;
        newpage += PageXOff;
        newpage += PageYOff << 3;
        PageXOff <<= 5;
        PageYOff <<= 5;
    }
    return (newpage);
}

typedef struct Vertex
{
    float sx; /* Screen coordinates */
    float sy;
    float sz;
    float rhw; /* Reciprocal of homogeneous w */
    DWORD color; /* Vertex color */
    DWORD specular; /* Specular component of vertex */
    float tu; /* Texture coordinates */
    float tv;
} Vertex;

#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_PREVIOUS 0x8578
#define GL_RGB_SCALE 0x8573

#define getTPage(tp, abr, x, y) (((tp & 0x3) << 7) | ((abr & 0x3) << 5) | ((y & 0x100) >> 4) | ((x & 0x3ff) >> 6))

u_short GetTPage(int tp, int abr, int x, int y) { return (getTPage(tp, abr, x, y)); }

int LoadImage4(GRECT* recp, unsigned long* p, CLUT* cluts) { return (1); }

int LoadImage8(GRECT* recp, unsigned long* p, CLUT* cluts) { return (1); }

void LoadClut4(GRECT* rect, u_short* clutptr, CLUT* cluts)
{
    int loop;
    u_short* ptr;

    ptr = (u_short*)cluts;
    for (loop = 0; loop < 16; loop++)
    {
        *ptr++ = *clutptr++;
    }
}

void LoadClut8(GRECT* rect, u_short* clutptr, CLUT* cluts)
{
    int loop;
    u_short* ptr;

    ptr = (u_short*)cluts;
    for (loop = 0; loop < 256; loop++)
    {
        *ptr++ = *clutptr++;
    }
}

void LoadPilotIntoVram(char* filename) { }

void setTexFilter()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
}

/*TODO
void SetColor(Vertex *v,char r, char g, char b)
{
   if(g)
   {
      int r;
      if(g==25)
      {
         v->color=RGB_MAKE(40,40,255);    // Speed up
      }
      else
      {
         r=DepthFadeTble[(g<<8) + WhitePal];
         v->color=RGB_MAKE(ddPalette[r].peRed,ddPalette[r].peGreen,ddPalette[r].peBlue);
      }
   }
   else
   if(b)
   {
      switch(b)
      {
         case BLACK2_TRANSL:
            v->color=RGB_MAKE(180,180,180);
            break;
         case RED_LIGHT_FADE:
            v->color=RGB_MAKE(255,10,0);
            break;
         case WHITE2_TRANSL:
         default:
            v->color=RGB_MAKE(100,100,100);
            break;
      }
   }
   else
   {
      v->color=RGB_MAKE(ddPalette[r].peRed,ddPalette[r].peGreen,ddPalette[r].peBlue);

   }
}
*/

BOOL UnloadTexture(TextureGL* pTex)
{
    if (pTex->Texture == 0)
        return FALSE;

    glDeleteTextures(1, &pTex->Texture);
    pTex->Texture = 0;
    return TRUE;
}

char* LockPilot(void) { return tempTex; }

void UnlockPilot(void)
{
    glBindTexture(GL_TEXTURE_2D, gTex[420].Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tempTex);
}

void ChopUp(char* to, void* from, int which)
{
    char* p1 = (char*)from;
    int i, x, y;

    which &= 63;
    x = which & 7;
    y = which >> 3;
    p1 += y * (512 * 32);
    p1 += x * 64;
    for (i = 0; i < 32; ++i)
    {
        memcpy(to, p1, 64);
        p1 += 512;
        to += 64;
    }
}

/*
 *  LoadTexture - load a texture map from a file into a texture surface
 */
BOOL SJRLoadTextureBMPmenu(const char* lpszTexFilename)
{
    FILE* fp;
    int i;
    long texsize;
    char* ptr;

    texsize = 32768; // 128*128 * sizeof (C3D_UINT16)

    fp = fopen(lpszTexFilename, "rb");
    if (!fp)
    {
        // ASSERT(FALSE);
        return FALSE;
    }

    fread((void*)TextureBuffer, TEXTURESJR, 1, fp);
    fclose(fp);

    { /*TODO WTF?*/
        WORD *tp, temp;

        tp = (WORD*)TextureBuffer;
        for (i = 0; i < TEXTURESJR / 2; ++i)
        {
            temp = *tp << 1;
            // black -> transparent
            if (temp != 0)
            {
                temp |= 1;
            }
            *tp++ = temp;
        }
    }

    ptr = TextureBuffer;

    for (i = 0; i < 420; i++)
    {
        if (i >= 5 && i < 405)
            continue;

        glGenTextures(1, &gTex[i].Texture);
        glBindTexture(GL_TEXTURE_2D, gTex[i].Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, ptr);
        setTexFilter();

        ptr += texsize * 4;
    }

    return TRUE;
}

/*
 *  LoadTexture - load a texture map from a file into a texture surface
 */
BOOL SJRLoadTextureBMP(const char* lpszTexFilename)
{
    FILE* fp;
    int i;
    long texsize;
    char* ptr;
    char* data;
    short track, other, size;

    track = 32;
    other = 256;

    texsize = 32768; // 128*128 * sizeof (C3D_UINT16)

    fp = fopen(lpszTexFilename, "rb");
    if (!fp)
    {
        // ASSERT(FALSE);
        return FALSE;
    }

    fread((void*)TextureBuffer, TEXTURESJR, 1, fp);
    fclose(fp);

    {
        WORD *tp, temp;
        int r, g, b;

        tp = (WORD*)TextureBuffer;
        for (i = 0; i < TEXTURESJR / 2; ++i)
        {
            temp = *tp << 1;
            // black -> transparent
            if (temp != 0)
            {
                temp |= 1;
            }
            *tp++ = temp;
        }
    }

    ptr = TextureBuffer;

    for (i = 0; i < 420; i++)
    {
        if (i >= 320 && i < 405)
            continue;

        if (i < 320)
        {
            ChopUp(tempTex, ptr, i);
            data = tempTex;
        }
        else
        {
            data = ptr;
        }

        size = (i < 320) ? track : other;
        glGenTextures(1, &gTex[i].Texture);
        glBindTexture(GL_TEXTURE_2D, gTex[i].Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, data);
        setTexFilter();

        if (i > 320 || ((i & 63) == 63))
            ptr += texsize * 4;
    }

    return TRUE;
}

/*
 *  LoadTexture - load a texture map from a file into a texture surface
 */
int JJSLoadSprites(void)
{
    FILE* fp;
    int i, sp;

    static char* lpszTexFilename[3] = { "wipeout\\newgraph\\menu\\page12.sjr", "wipeout\\newgraph\\menu\\page15.sjr", "wipeout\\newgraph\\menu\\page17.sjr" };

    for (sp = 0; sp < 3; sp++)
    {
        fp = fopen(lpszTexFilename[sp], "rb");
        if (!fp)
        {
            // ASSERT(FALSE);
            return FALSE;
        }
        fread(tempTex, 256 * 256 * 2, 1, fp);
        fclose(fp);

        {
            short *tp, temp;

            tp = (short*)tempTex;
            for (i = 0; i < 256 * 256; i++)
            {
                temp = *tp << 1;
                // black -> transparent
                if (temp != 0)
                {
                    temp |= 1;
                }
                *tp++ = temp;
            }
        }

        glGenTextures(1, &gTex[420 + sp].Texture);
        glBindTexture(GL_TEXTURE_2D, gTex[420 + sp].Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, tempTex);
        setTexFilter();
    }

    return TRUE;
}

BOOL SJRNewTexture(char* filename)
{
    int i;

    // unload all textures
    for (i = 0; i < 420; i++)
    {
        UnloadTexture(gTex + i);
    }

    if (menuTexturesIn)
        return (SJRLoadTextureBMPmenu(filename));
    else
        return (SJRLoadTextureBMP(filename));
}

TimData* JJSLoad16BitTexture(char* filename, char location)
{
    TimData* timData;
    DWORD bufsize;
    long texsize;
    char* bmpdata;

    texsize = 320 * 240 * sizeof(WORD);
    bufsize = texsize;

    /*TODO 16-bit load*/
    // timData = (TimData*)DAlloc((DynamicHeap*)heap, sizeof(TimData));
    // JJSReadTIM(filename, tempTex, timData);

    return NULL; // timData;
}

void renderBegin(void)
{
    texpage = 420;
    renderClear();
}

void renderEnd(void) { }

void SetGeomOffset(long ofx, long ofy)
{
    xOffset = ofx;
    yOffset = ofy;
}

void renderInit()
{
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;

    hDC = GetDC(hWnd);

    int format = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, format, &pfd);
    hRC = wglCreateContext(hDC);

    wglMakeCurrent(hDC, hRC);

    glClearColor(0, 0, 0, 1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    // use additive blending
    glBlendFunc(GL_ONE, GL_ONE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 320, 240, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    WinWidth = 320;
    WinHeight = 240;

    JJSLoadSprites();

    TextureBuffer = (char*)malloc(TEXTURESJR);
}

void renderFree()
{
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

void renderSwap()
{
    SwapBuffers(hDC);
}

void renderResize(int width, int height)
{
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
}

void renderOffset(int x, int y) { glViewport(x * gWidth / 320, y * gHeight / 240, gWidth, gHeight); }

void renderClear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }

#if 0
BOOL SetState(C3D_EPRIM Quad, C3D_ESHADE Shade, BOOL BTexEn, BOOL Trans, C3D_ETLIGHT Light)
{
    BOOL change = FALSE, tranfix = FALSE;
    C3D_ETEXOP chkey = C3D_ETEXOP_NONE;
    static BOOL FiltOn = FALSE;

    if (Quad != quad)
    {
        sendList();
        quad = Quad;
        ATI3DCIF_ContextSetState(ghRC, C3D_ERS_PRIM_TYPE,
            &quad);
        change = TRUE;
    }
    if (Shade != shade)
    {
        if (!change)
            sendList();
        shade = Shade;
        ATI3DCIF_ContextSetState(ghRC, C3D_ERS_SHADE_MODE,
            &shade);
        change = TRUE;
    }
    if (BTexEn != bTexEn)
    {
        if (!change)
            sendList();
        bTexEn = BTexEn;
        ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_EN, &bTexEn);
        change = TRUE;
    }
    if (Light != light)
    {
        if (!change)
            sendList();
        light = Light;
        ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_LIGHT, &light);
        change = TRUE;
    }
    if (Trans != trans)
    {
        if (!change)
            sendList();
        trans = Trans;
        if (trans)
        {
            switch (trans)
            {
            case BLACK_NOT_DRAWN:
                chkey = C3D_ETEXOP_CHROMAKEY;//SJR - make 0,0,0 trans
                eDstAlpha = C3D_EADST_ZERO;
                eSrcAlpha = C3D_EASRC_ONE;
                break;
            case BLACK5_TRANSL:
            case RED_TRANSL:
            case BLACK1_TRANSL:
            case BLACK2_TRANSL:
            case BLACK4_TRANSL:
            case BLACK6_TRANSL:
                tranfix = TRUE;
                chkey = C3D_ETEXOP_CHROMAKEY;//SJR - make 0,0,0 trans
            case BLACK3_TRANSL:
                eDstAlpha = C3D_EADST_ZERO;
                eSrcAlpha = C3D_EASRC_DSTCLR;
                break;
            case WHITE1_TRANSL:
            case WHITE2_TRANSL:
            case WHITE3_TRANSL:
            case WHITE4_TRANSL:
            case WHITE5_TRANSL:
            case WHITE6_TRANSL:
            case WHITE7_TRANSL:
            case WHITE8_TRANSL:
            case UNUSED1_TRANSL:
            default:
                eDstAlpha = C3D_EADST_ONE;
                eSrcAlpha = C3D_EASRC_INVDSTCLR;

                break;
            }
        }
        else
        {
            eDstAlpha = C3D_EADST_ZERO;
            eSrcAlpha = C3D_EASRC_ONE;
        }
        if (tranfix && ft && FiltOn)
        {
            ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_FILTER, &eTexFilteroff);
            FiltOn = FALSE;
        }
        if (!tranfix && ft && !FiltOn)
        {
            ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_FILTER, &eTexFilteron);
            FiltOn = TRUE;
        }
        ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_TEXOP, &chkey);
        ATI3DCIF_ContextSetState(ghRC, C3D_ERS_ALPHA_DST, &eDstAlpha);
        ATI3DCIF_ContextSetState(ghRC, C3D_ERS_ALPHA_SRC, &eSrcAlpha);

        change = TRUE;
    }
    return(change);
}
#endif

void setTexture(int tpage)
{
    if (tpage >= 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gTex[tpage].Texture);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
}

#define i32 (1.0f / 32.0f)
#define i256 (1.0f / 256.0f)
#define i4096 (1.0f / 4096.0f)
#define IDEG (360.0f / 4096.0f)

extern short DRTEXT_RED;

void renderQuad(const Quad* q)
{
    float u0 = q->u0 * i256;
    float v0 = q->v0 * i256;
    float u1 = q->u1 * i256;
    float v1 = q->v1 * i256;
    short x0 = q->x;
    short y0 = q->y;
    short x1 = q->x + q->w;
    short y1 = q->y + q->h;

    short tpage = GetThePage(q->page, q->u0, q->v0);

    // TODO WTF?
    if (tpage == 415)
        tpage = 422;

    if ((q->clut == TextureTable[DRTEXT_RED]->cba) && tpage == 421)
    {
        tpage = 422;
        v0 += 0.5f;
        v1 += 0.5f;
    }

    setTexture(tpage);

    glColor3ub(128, 128, 128);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(u0, v0);
    glVertex2s(x0, y0);
    glTexCoord2f(u1, v0);
    glVertex2s(x1, y0);
    glTexCoord2f(u1, v1);
    glVertex2s(x1, y1);
    glTexCoord2f(u0, v1);
    glVertex2s(x0, y1);
    glEnd();
}

void prepareTrack() { }

void renderPrepare() { prepareTrack(); }

void setMatrix(Skeleton* skeleton, int x, int y, int z)
{
    float m[4][4];
    const MATRIX* matrix = GetSkeletonMatrix(skeleton);

    m[0][0] = matrix->m[0][0] * i4096;
    m[0][1] = matrix->m[1][0] * i4096;
    m[0][2] = matrix->m[2][0] * i4096;
    m[0][3] = 0.0f;

    m[1][0] = matrix->m[0][1] * i4096;
    m[1][1] = matrix->m[1][1] * i4096;
    m[1][2] = matrix->m[2][1] * i4096;
    m[1][3] = 0.0f;

    m[2][0] = matrix->m[0][2] * i4096;
    m[2][1] = matrix->m[1][2] * i4096;
    m[2][2] = matrix->m[2][2] * i4096;
    m[2][3] = 0.0f;

    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
    m[3][3] = 1.0f;

    glLoadMatrixf((GLfloat*)&m);
}

void renderTrack()
{
    extern TrackData* track;
    extern TexTemplate* ctrltexTemplate;

    const VECTOR* vertices = track->vertices;
    const Face* face = track->faces;
    int i, j;
    int cx, cy, cz;

    cx = ctrlcamera.camPos->absolute.t[0]; // - ctrlcamera.camPos->absolute.t[0] / 8;
    cy = ctrlcamera.camPos->absolute.t[1]; // - ctrlcamera.camPos->absolute.t[1] / 8;
    cz = ctrlcamera.camPos->absolute.t[2]; // - ctrlcamera.camPos->absolute.t[2] / 8;

    setMatrix(ctrlcamera.camPos->super, cx, cy, cz);

    for (j = 0; j < track->sectionCount; j++)
    {
        const TrackSection* section = track->sections + j;

        for (i = section->faceStart; i < section->faceStart + section->faceCount; i++, face++)
        {
            const Texture* tex = TextureTable[ctrltexTemplate[face->texture].tex1[0]];

            short tpage = GetThePage(tex->tsb, (tex->u0 + tex->u1 + tex->u2 + tex->u3) / 4, (tex->v0 + tex->v1 + tex->v2 + tex->v3) / 4);
            setTexture(tpage);

            glColor3ub(face->red, face->green, face->blue);

            glBegin(GL_QUADS);
            if (face->flags & FLIP_TEXTURE)
            {
                glTexCoord2f((tex->u2 - PageXOff) * i32, (tex->v2 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[3]));
                glTexCoord2f((tex->u3 - PageXOff) * i32, (tex->v3 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[2]));
                glTexCoord2f((tex->u1 - PageXOff) * i32, (tex->v1 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[1]));
                glTexCoord2f((tex->u0 - PageXOff) * i32, (tex->v0 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[0]));
            }
            else
            {
                glTexCoord2f((tex->u3 - PageXOff) * i32, (tex->v3 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[3]));
                glTexCoord2f((tex->u2 - PageXOff) * i32, (tex->v2 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[2]));
                glTexCoord2f((tex->u0 - PageXOff) * i32, (tex->v0 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[1]));
                glTexCoord2f((tex->u1 - PageXOff) * i32, (tex->v1 - PageYOff) * i32);
                glVertex3iv((GLint*)(vertices + face->vertex[0]));
            }
            glEnd();
        }
    }
}

void renderObject(const Object* object)
{
    const MATRIX* matrix;
    Poly poly;
    const SVECTOR* vertices;
    short tpage;
    int i;
    int cx, cy, cz;
    const CVECTOR* c;

    const CVECTOR colorWhite3[] = { { 255, 255, 255, 0 }, { 255, 255, 255, 0 }, { 255, 255, 255, 0 } };

    matrix = GetSkeletonMatrix(object->skeleton);

    cx = matrix->t[0];
    cy = matrix->t[1];
    cz = matrix->t[2];
    /*
    if (ctrlcamera.camPos)
    {
       cx -= ctrlcamera.camPos->absolute.t[0] / 8;
       cy -= ctrlcamera.camPos->absolute.t[1] / 8;
       cz -= ctrlcamera.camPos->absolute.t[2] / 8;
    }
    */
    setMatrix(object->skeleton, cx, cy, cz);

    vertices = object->vertices;
    poly.prim = object->primitives;

    for (i = 0; i < object->primitiveCount; i++)
    {
        switch (poly.prim->type)
        {
            case TypeF3:
            {
                const F3* f = poly.f3;

                setTexture(-1);

                glColor3ub(f->colour.r, f->colour.g, f->colour.b);
                glBegin(GL_TRIANGLES);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.f3++;
                break;
            }

            case TypeFT3:
            {
                const FT3* f = poly.ft3;

                tpage = GetThePage(f->tsb, (f->u0 + f->u1 + f->u2) / 3, (f->v0 + f->v1 + f->v2) / 3);
                setTexture(tpage);

                if (f->flag & ShipEngine)
                {
                    renderTransparent(1);
                    glColor3ub(255, 255, 255);
                }
                else
                {
                    glColor3ub(f->colour.r, f->colour.g, f->colour.b);
                }

                glBegin(GL_TRIANGLES);
                glTexCoord2f(f->u0 * i256, f->v0 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glTexCoord2f(f->u1 * i256, f->v1 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glTexCoord2f(f->u2 * i256, f->v2 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                if (f->flag & ShipEngine)
                {
                    renderTransparent(0);
                }

                poly.ft3++;
                break;
            }

            case TypeF4:
            {
                const F4* f = poly.f4;

                setTexture(-1);

                glColor3ub(f->colour.r, f->colour.g, f->colour.b);
                glBegin(GL_QUADS);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glVertex3sv((GLshort*)(vertices + f->coords[3]));

                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.f4++;
                break;
            }

            case TypeFT4:
            {
                const FT4* f = poly.ft4;

                tpage = GetThePage(f->tsb, (f->u0 + f->u1 + f->u2 + f->u3) / 4, (f->v0 + f->v1 + f->v2 + f->v3) / 4);
                setTexture(tpage);

                glColor3ub(f->colour.r, f->colour.g, f->colour.b);
                glBegin(GL_QUADS);
                glTexCoord2f(f->u0 * i256, f->v0 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glTexCoord2f(f->u1 * i256, f->v1 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glTexCoord2f(f->u3 * i256, f->v3 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[3]));

                glTexCoord2f(f->u2 * i256, f->v2 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.ft4++;
                break;
            }

            case TypeG3:
            {
                const G3* f = poly.g3;

                setTexture(-1);

                glBegin(GL_TRIANGLES);
                glColor3ub(f->colour[0].r, f->colour[0].g, f->colour[0].b);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glColor3ub(f->colour[1].r, f->colour[1].g, f->colour[1].b);
                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glColor3ub(f->colour[2].r, f->colour[2].g, f->colour[2].b);
                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.g3++;
                break;
            }

            case TypeGT3:
            {
                const GT3* f = poly.gt3;
                c = f->colour;

                tpage = GetThePage(f->tsb, (f->u0 + f->u1 + f->u2) / 3, (f->v0 + f->v1 + f->v2) / 3);
                setTexture(tpage);

                if (f->flag & ShipEngine)
                {
                    renderTransparent(1);
                    c = &colorWhite3[0];
                }

                glBegin(GL_TRIANGLES);
                glColor3ubv((GLubyte*)(c + 0));
                glTexCoord2f(f->u0 * i256, f->v0 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glColor3ubv((GLubyte*)(c + 1));
                glTexCoord2f(f->u1 * i256, f->v1 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glColor3ubv((GLubyte*)(c + 2));
                glTexCoord2f(f->u2 * i256, f->v2 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                if (f->flag & ShipEngine)
                {
                    renderTransparent(0);
                }

                poly.gt3++;
                break;
            }

            case TypeG4:
            {
                const G4* f = poly.g4;

                setTexture(-1);

                glBegin(GL_QUADS);
                glColor3ub(f->colour[0].r, f->colour[0].g, f->colour[0].b);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glColor3ub(f->colour[1].r, f->colour[1].g, f->colour[1].b);
                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glColor3ub(f->colour[3].r, f->colour[3].g, f->colour[3].b);
                glVertex3sv((GLshort*)(vertices + f->coords[3]));

                glColor3ub(f->colour[2].r, f->colour[2].g, f->colour[2].b);
                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.g4++;
                break;
            }

            case TypeGT4:
            {
                const GT4* f = poly.gt4;

                tpage = GetThePage(f->tsb, (f->u0 + f->u1 + f->u2 + f->u3) / 4, (f->v0 + f->v1 + f->v2 + f->v3) / 4);
                setTexture(tpage);

                glBegin(GL_QUADS);
                glColor3ub(f->colour[0].r, f->colour[0].g, f->colour[0].b);
                glTexCoord2f(f->u0 * i256, f->v0 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[0]));

                glColor3ub(f->colour[1].r, f->colour[1].g, f->colour[1].b);
                glTexCoord2f(f->u1 * i256, f->v1 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[1]));

                glColor3ub(f->colour[3].r, f->colour[3].g, f->colour[3].b);
                glTexCoord2f(f->u3 * i256, f->v3 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[3]));

                glColor3ub(f->colour[2].r, f->colour[2].g, f->colour[2].b);
                glTexCoord2f(f->u2 * i256, f->v2 * i256);
                glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.gt4++;
                break;
            }

            case TypeBSPR:
            {
                const SPR* f = poly.spr;
                const SVECTOR* v = vertices + f->coord;
                const Texture* t = TextureTable[f->texture];
                int w = f->width >> 1;
                int h = f->height;
                int sx, sz;

                sx = rcos(-ctrlcamera.hdg) * w >> 12;
                sz = rsin(-ctrlcamera.hdg) * w >> 12;

                tpage = GetThePage(t->tsb, (t->u0 + t->u1 + t->u2 + t->u3) / 4, (t->v0 + t->v1 + t->v2 + t->v3) / 4);
                setTexture(tpage);

                glColor3ub(f->colour.r, f->colour.g, f->colour.b);
                // glColor3ub(255, 255, 255);
                glBegin(GL_QUADS);
                glTexCoord2f(t->u0 * i256, t->v0 * i256);
                glVertex3s(v->vx - sx, v->vy - h, v->vz - sz);

                glTexCoord2f(t->u1 * i256, t->v1 * i256);
                glVertex3s(v->vx + sx, v->vy - h, v->vz + sz);

                glTexCoord2f(t->u3 * i256, t->v3 * i256);
                glVertex3s(v->vx + sx, v->vy, v->vz + sz);

                glTexCoord2f(t->u2 * i256, t->v2 * i256);
                glVertex3s(v->vx - sx, v->vy, v->vz - sz);
                glEnd();

                poly.spr++;
                break;
            }

            default:
                break;
        }
    }

    glPopMatrix();
}

void renderSky(const Object* object)
{
    glDisable(GL_DEPTH_TEST);
    renderObject(object);
    glEnable(GL_DEPTH_TEST);
}

int in3D = 0;

void renderBegin3D()
{
    in3D++;

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef((xOffset - 160) / 160.0f, -(yOffset - 120) / 120.f, 0);
    gluPerspective(75.0f, (float)gWidth / (float)gHeight, (float)64, (float)(64 * 1024));
    glScalef(1, -1, -1);

    glMatrixMode(GL_MODELVIEW);
}

void renderEnd3D()
{
    in3D--;
    if (in3D > 0)
        return;

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 320, 240, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderTransparent(int enable)
{
    if (enable)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}
