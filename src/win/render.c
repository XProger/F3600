#include <windows.h>
#include <gl/GL.h>
#include <gl/glu.h>

#include "game.h"

uint16 tempData[320 * 240];

extern Texture* TextureTable[];

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

extern short WinWidth;
extern short WinHeight;

extern const TrackCamera ctrlcamera;

long xOffset;
long yOffset;

int gWidth, gHeight, gBorder;

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

typedef struct TextureGL
{
    uint16 width;
    uint16 height;
    GLuint id;
    float iw;
    float ih;
} TextureGL;

#define GL_COMBINE      0x8570
#define GL_COMBINE_RGB  0x8571
#define GL_SOURCE0_RGB  0x8580
#define GL_SOURCE1_RGB  0x8581
#define GL_PREVIOUS     0x8578
#define GL_RGB_SCALE    0x8573

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

int UnloadTexture(TextureGL* pTex)
{
#if 0
    if (pTex->Texture == 0)
        return FALSE;

    glDeleteTextures(1, &pTex->Texture);
    pTex->Texture = 0;
#endif
    return TRUE;
}

void SetGeomOffset(long ofx, long ofy)
{
    xOffset = ofx;
    yOffset = ofy;
}

void renderInit(void)
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

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

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
}

void renderFree(void)
{
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

void renderSwap(void)
{
    SwapBuffers(hDC);
}

void renderResize(int width, int height)
{
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
}

void renderOffset(int x, int y)
{
    glViewport(x * gWidth / 320, y * gHeight / 240, gWidth, gHeight);
}

void renderClear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void setTexture(const TextureGL* tex)
{
    if (tex)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
}

#define i32     (1.0f / 32.0f)
#define i64     (1.0f / 64.0f)
#define i256    (1.0f / 256.0f)
#define i212    (1.0f / 212.0f)
#define i4096   (1.0f / 4096.0f)
#define IDEG    (360.0f / 4096.0f)

void renderQuad(const Quad* q)
{
    TextureGL* t = (TextureGL*)q->texture;
    float u0 = q->u0 * t->iw;
    float v0 = q->v0 * t->ih;
    float u1 = q->u1 * t->iw;
    float v1 = q->v1 * t->ih;
    short x0 = q->x;
    short y0 = q->y;
    short x1 = q->x + q->w;
    short y1 = q->y + q->h;

    setTexture(t);

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

void prepareTrack(void)
{
}

void renderPrepare(void)
{
    prepareTrack();
}

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

void renderTrack(void)
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
            const TextureGL* t = (TextureGL*)TextureTable[ctrltexTemplate[face->texture].tex1[0]];

            setTexture(t);

            glColor3ub(face->red, face->green, face->blue);

            glBegin(GL_QUADS);
            if (face->flags & FLIP_TEXTURE)
            {
                glTexCoord2f(0, 1);
                glVertex3iv((GLint*)(vertices + face->vertex[3]));
                glTexCoord2f(1, 1);
                glVertex3iv((GLint*)(vertices + face->vertex[2]));
                glTexCoord2f(1, 0);
                glVertex3iv((GLint*)(vertices + face->vertex[1]));
                glTexCoord2f(0, 0);
                glVertex3iv((GLint*)(vertices + face->vertex[0]));
            }
            else
            {
                glTexCoord2f(1, 1);
                glVertex3iv((GLint*)(vertices + face->vertex[3]));
                glTexCoord2f(0, 1);
                glVertex3iv((GLint*)(vertices + face->vertex[2]));
                glTexCoord2f(0, 0);
                glVertex3iv((GLint*)(vertices + face->vertex[1]));
                glTexCoord2f(1, 0);
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

                setTexture(NULL);

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
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                setTexture(t);

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
                    glTexCoord2f(f->u0 * t->iw, f->v0 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[0]));

                    glTexCoord2f(f->u1 * t->iw, f->v1 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[1]));

                    glTexCoord2f(f->u2 * t->iw, f->v2 * t->ih);
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

                setTexture(NULL);

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
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                setTexture(t);

                glColor3ub(f->colour.r, f->colour.g, f->colour.b);
                glBegin(GL_QUADS);
                    glTexCoord2f(f->u0 * t->iw, f->v0 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[0]));

                    glTexCoord2f(f->u1 * t->iw, f->v1 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[1]));

                    glTexCoord2f(f->u3 * t->iw, f->v3 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[3]));

                    glTexCoord2f(f->u2 * t->iw, f->v2 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.ft4++;
                break;
            }

            case TypeG3:
            {
                const G3* f = poly.g3;

                setTexture(NULL);

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
                TextureGL* t = (TextureGL*)TextureTable[f->texture];
                c = f->colour;

                setTexture(t);

                if (f->flag & ShipEngine)
                {
                    renderTransparent(1);
                    c = &colorWhite3[0];
                }

                glBegin(GL_TRIANGLES);
                    glColor3ubv((GLubyte*)(c + 0));
                    glTexCoord2f(f->u0 * t->iw, f->v0 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[0]));

                    glColor3ubv((GLubyte*)(c + 1));
                    glTexCoord2f(f->u1 * t->iw, f->v1 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[1]));

                    glColor3ubv((GLubyte*)(c + 2));
                    glTexCoord2f(f->u2 * t->iw, f->v2 * t->ih);
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

                setTexture(NULL);

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
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                setTexture(t);

                glBegin(GL_QUADS);
                    glColor3ub(f->colour[0].r, f->colour[0].g, f->colour[0].b);
                    glTexCoord2f(f->u0 * t->iw, f->v0 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[0]));

                    glColor3ub(f->colour[1].r, f->colour[1].g, f->colour[1].b);
                    glTexCoord2f(f->u1 * t->iw, f->v1 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[1]));

                    glColor3ub(f->colour[3].r, f->colour[3].g, f->colour[3].b);
                    glTexCoord2f(f->u3 * t->iw, f->v3 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[3]));

                    glColor3ub(f->colour[2].r, f->colour[2].g, f->colour[2].b);
                    glTexCoord2f(f->u2 * t->iw, f->v2 * t->ih);
                    glVertex3sv((GLshort*)(vertices + f->coords[2]));
                glEnd();

                poly.gt4++;
                break;
            }

            case TypeBSPR:
            {
                const SPR* f = poly.spr;
                const SVECTOR* v = vertices + f->coord;
                int w = f->width >> 1;
                int h = f->height;
                int sx, sz;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                sx = rcos(-ctrlcamera.hdg) * w >> 12;
                sz = rsin(-ctrlcamera.hdg) * w >> 12;

                setTexture(t);

                glColor3ub(f->colour.r, f->colour.g, f->colour.b);

                glBegin(GL_QUADS);
                    glTexCoord2f(0, 0);
                    glVertex3s(v->vx - sx, v->vy - h, v->vz - sz);

                    glTexCoord2f(1, 0);
                    glVertex3s(v->vx + sx, v->vy - h, v->vz + sz);

                    glTexCoord2f(1, 1);
                    glVertex3s(v->vx + sx, v->vy, v->vz + sz);

                    glTexCoord2f(0, 1);
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

void renderBegin3D(void)
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

void renderEnd3D(void)
{
    in3D--;
    if (in3D > 0)
        return;

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gBorder = ((gWidth * 240) / gHeight - 320) >> 1;
    
    glOrtho(-gBorder, 320 + gBorder, 240, 0, 0, 1);

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

uint16 RGB5551(uint16 c)
{
    char r = (c) & 0x1F;
    char g = (c >> 5) & 0x1F;
    char b = (c >> 10) & 0x1F;
    c = (r << 11) | (g << 6) | (b << 1);

    if (c != 0)
    {
        c |= 1;
    }

    return c;
}

uint16 RGB555(uint16 c)
{
    char r = (c) & 0x1F;
    char g = (c >> 5) & 0x1F;
    char b = (c >> 10) & 0x1F;
    c = (r << 11) | (g << 6) | (b << 1) | 1;
    return c;
}

Texture* renderCreateTexture(Tim* tim)
{
    TextureGL* tex;
    int32 w, h;
    int i, j;

    if (TimType(tim) == TIM_4)
    {
        uint16 clut[16];
        Tim4* t = &tim->tim4;
        uint8* src = (uint8*)(t + 1);
        uint16* dst = tempData;

        w = t->textureW * 4;
        h = t->textureH;

        for (i = 0; i < 16; i++)
        {
            clut[i] = RGB5551(t->clut[i]);
        }

        for (i = 0; i < h; i++)
        {
            for (j = 0; j < (w >> 1); j++)
            {
                uint8 indexAB = *src++;
                uint8 indexA = (indexAB & 0x0F);
                uint8 indexB = (indexAB & 0xF0) >> 4;

                *dst++ = clut[indexA];
                *dst++ = clut[indexB];
            }
        }
    }
    else if (TimType(tim) == TIM_8)
    {
        Tim8* t = &tim->tim8;
        uint8* src = (uint8*)(t + 1);
        uint16* dst = tempData;
        w = t->textureW * 2;
        h = t->textureH;

        for (i = 0; i < 256; i++)
        {
            t->clut[i] = RGB555(t->clut[i]);
        }

        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                *dst++ = t->clut[*src++];
            }
        }
    }
    else if (TimType(tim) == TIM_16)
    {
        Tim16* t = &tim->tim16;
        uint16* src = (uint16*)(t + 1);
        uint16* dst = tempData;
        w = t->textureW;
        h = t->textureH;

        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                *dst++ = RGB555(*src++);
            }
        }
    }
    else
    {
        ASSERT(0);
    }

    tex = (TextureGL*)malloc(sizeof(TextureGL));
    tex->width = w;
    tex->height = h;
    tex->iw = 1.0f / w;
    tex->ih = 1.0f / h;

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, tempData);
    setTexFilter();

    return (Texture*)tex;
}
