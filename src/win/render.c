#include <windows.h>
#include <gl/GL.h>
#include <gl/glu.h>

#include "game.h"

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

#define GL_COMBINE      0x8570
#define GL_COMBINE_RGB  0x8571
#define GL_SOURCE0_RGB  0x8580
#define GL_SOURCE1_RGB  0x8581
#define GL_PREVIOUS     0x8578
#define GL_RGB_SCALE    0x8573

#define i32     (1.0f / 32.0f)
#define i64     (1.0f / 64.0f)
#define i256    (1.0f / 256.0f)
#define i212    (1.0f / 212.0f)
#define i4096   (1.0f / 4096.0f)
#define IDEG    (360.0f / 4096.0f)

typedef struct TextureGL
{
    uint16 width;
    uint16 height;
    GLuint id;
    float iw;
    float ih;
} TextureGL;

uint16 tempData[320 * 240];

extern Texture* TextureTable[];
const TextureGL* gTexture;

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

extern const TrackCamera ctrlcamera;

int gRenderMode;

long xOffset;
long yOffset;

int gWidth, gHeight, gBorder;

int gCylX, gCylZ;

#ifdef DEBUG_RENDER
    int dbg_trackPolys;
    int dbg_objectPolys;
    int dbg_texSwitch;
#endif

#define MAX_TRACK_TEX   22
TextureGL gTrackTex[MAX_TRACK_TEX];
uint16 gTrackTexData[MAX_TRACK_TEX][128 * 128];

extern TrackData* track;
extern TexTemplate* ctrltexTemplate;
extern short ctrllibraryTextures;
extern short ctrlnumLib;

void setTexFilter(int bilinear)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bilinear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bilinear ? GL_LINEAR : GL_NEAREST);
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

    glClearColor(0, 0, 0, 1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glEnable(GL_TEXTURE_2D);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void renderFree(void)
{
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

void renderSwap(void)
{
#ifdef DEBUG_RENDER
    char buf[64];
    sprintf(buf, "t:%d, o:%d, ts:%d\n", dbg_trackPolys, dbg_objectPolys, dbg_texSwitch);
    dbg_trackPolys = dbg_objectPolys = dbg_texSwitch = 0;
    OutputDebugString(buf);
#endif
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

void renderFlush()
{

}

void setTexture(const TextureGL* tex)
{
    if (gTexture == tex)
        return;

    gTexture = tex;
    renderFlush();

#ifdef DEBUG_RENDER
    dbg_texSwitch++;
#endif

    if (tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void renderQuad(const Quad* q)
{
    TextureGL* t = (TextureGL*)q->texture;
    float u0 = q->u0 * t->iw;
    float v0 = q->v0 * t->ih;
    float u1 = q->u1 * t->iw;
    float v1 = q->v1 * t->ih;
    int x0 = q->x;
    int y0 = q->y;
    int x1 = q->x + q->w;
    int y1 = q->y + q->h;

    setTexture(t);

    glColor3ub(128, 128, 128);
    glBegin(GL_QUADS);
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

void renderShadow(const Shadow* s)
{
    TextureGL* t = (TextureGL*)s->texture;

    setTexture(t);

    glColor4ub(255, 255, 255, s->intensity);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3i(s->x0, s->y0, s->z0);
        glTexCoord2f(1, 0);
        glVertex3i(s->x1, s->y1, s->z1);
        glTexCoord2f(1, 1);
        glVertex3i(s->x2, s->y2, s->z2);
        glTexCoord2f(0, 1);
        glVertex3i(s->x3, s->y3, s->z3);
    glEnd();
}

void renderColorQuad(const ColorQuad* q)
{
    setTexture(NULL);

    glBegin(GL_QUADS);
        glColor3ubv((GLubyte*)&q->r0);
        glVertex2sv((GLshort*)&q->x0);
        glColor3ubv((GLubyte*)&q->r1);
        glVertex2sv((GLshort*)&q->x1);
        glColor3ubv((GLubyte*)&q->r2);
        glVertex2sv((GLshort*)&q->x2);
        glColor3ubv((GLubyte*)&q->r3);
        glVertex2sv((GLshort*)&q->x3);
    glEnd();
}

void renderBillboard(const Billboard* b)
{
    int sx, sy, sz, size;
    TextureGL* t = (TextureGL*)b->texture;

    size = b->size * 8;
    sx = size * gCylX >> 12;
    sy = size;
    sz = size * gCylZ >> 12;

    setTexture(t);

    glColor3ub(128, 128, 128);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3i(b->x - sx, b->y - sy, b->z - sz);
        glTexCoord2f(1, 0);
        glVertex3i(b->x + sx, b->y - sy, b->z + sz);
        glTexCoord2f(1, 1);
        glVertex3i(b->x + sx, b->y + sy, b->z + sz);
        glTexCoord2f(0, 1);
        glVertex3i(b->x - sx, b->y + sy, b->z - sz);
    glEnd();
}

void prepareTrack(void)
{
    int i, j, x, y;

    ASSERT(ctrlnumLib <= MAX_TRACK_TEX);
    for (i = 0; i < ctrlnumLib; i++)
    {
        glGenTextures(1, &gTrackTex[i].id);
        glBindTexture(GL_TEXTURE_2D, gTrackTex[i].id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, gTrackTexData[i]);
        setTexFilter(0);
    }
}

void FillTrackTex(uint16* data)
{
    int id, i, j, k, x, y;

    id = TextureTableCount - ctrllibraryTextures;

    for (i = 0; i < ctrlnumLib; i++)
    {
        for (j = 0; j < 16; j++)
        {
            if (ctrltexTemplate[i].tex16[j] != id)
                continue;

            x = (j % 4) * 32;
            y = (j / 4) * 32;

            for (k = 0; k < 32; k++)
            {
                memcpy(gTrackTexData[i] + (k + y) * 128 + x, data + k * 32, 32 * sizeof(uint16));
            }
        }
    }
}

void renderPrepare(void)
{
    prepareTrack();
}

void renderFrameBegin(void)
{
    gBorder = ((gWidth * 240) / gHeight - 320) >> 1;

    gCylX = rcos(-ctrlcamera.hdg);
    gCylZ = rsin(-ctrlcamera.hdg);

    gRenderMode = RENDER_MODE_3D;
    renderSetMode(RENDER_MODE_2D);
}

void renderFrameEnd(void)
{
    //
}

void renderSetMatrix(const MATRIX* matrix, int x, int y, int z)
{
    float m[4][4];

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

void renderSetSkeleton(Skeleton* skeleton, int x, int y, int z)
{
    float m[4][4];
    renderSetMatrix(GetSkeletonMatrix(skeleton), x, y, z);
}

void renderTrack(void)
{
    const VECTOR* vertices = track->vertices;
    const Face* face;
    int i, j, s, k, id;
    int cx, cy, cz;
    int dir;
    short* counts;
    const short** sections;
    TrackSection* section;

//    #define CHECK_UNUSED_TEX

#ifdef CHECK_UNUSED_TEX
    uint8 texUsed[MAX_TRACK_TEX];
    memset(texUsed, 0, MAX_TRACK_TEX);
#endif

    dir = TrackDirection(&ctrlcamera);

    cx = ctrlcamera.camPos->absolute.t[0];
    cy = ctrlcamera.camPos->absolute.t[1];
    cz = ctrlcamera.camPos->absolute.t[2];

    renderSetSkeleton(ctrlcamera.camPos->super, cx, cy, cz);

    switch (dir)
    {
        case VIEW_DIR_NORTH:
            counts = ctrlcamera.section->northViewCount;
            sections = ctrlcamera.section->northViewSection;
            break;
        case VIEW_DIR_SOUTH:
            counts = ctrlcamera.section->southViewCount;
            sections = ctrlcamera.section->southViewSection;
            break;
        case VIEW_DIR_WEST:
            counts = ctrlcamera.section->westViewCount;
            sections = ctrlcamera.section->westViewSection;
            break;
        default: // VIEW_DIR_EAST
            counts = ctrlcamera.section->eastViewCount;
            sections = ctrlcamera.section->eastViewSection;
    }

    for (k = 0; k < 3; k++)
    {
        for (j = 0; j < counts[k]; j += 2)
        {
            section = track->sections + sections[k][j];

            for (s = 0; s < sections[k][j + 1]; s++)
            {
                face = track->faces + section->faceStart;

                for (i = 0; i < section->faceCount; i++, face++)
                {
                    ASSERT(face->texture < ctrlnumLib);

                #ifdef CHECK_UNUSED_TEX
                    texUsed[face->texture] = 1;
                #endif

                    setTexture(gTrackTex + face->texture);

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

                #ifdef DEBUG_RENDER
                    dbg_trackPolys++;
                #endif
                }

                section = section->nextSection.ptr;
            }
        }
    }

#ifdef CHECK_UNUSED_TEX
    for (i = 0; i < ctrlnumLib; i++)
    {
        ASSERT(texUsed[i]);
    }
#endif
}

void renderObject(const Object* object)
{
    const MATRIX* matrix;
    Prm poly;
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

    renderSetSkeleton(object->skeleton, cx, cy, cz);

    vertices = object->vertices;
    poly.primitive = object->primitives;

#ifdef DEBUG_RENDER
    dbg_objectPolys += object->primitiveCount;
#endif

    for (i = 0; i < object->primitiveCount; i++)
    {
        switch (poly.primitive->type)
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
                    renderSetBlend(BLEND_ADD);
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
                    renderSetBlend(BLEND_NONE);
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
                    renderSetBlend(BLEND_ADD);
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
                    renderSetBlend(BLEND_NONE);
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

            case TypeTSPR:
            case TypeBSPR:
            {
                const SPR* f = poly.spr;
                const SVECTOR* v = vertices + f->coord;
                int w = f->width >> 1;
                int h = f->height;
                int sx, sz;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                sx = gCylX * w >> 12;
                sz = gCylZ * w >> 12;

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
                ASSERT(0);
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

void renderSetMode(int renderMode)
{
    if (gRenderMode == renderMode)
        return;
    gRenderMode = renderMode;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (renderMode == RENDER_MODE_2D)
    {
        glDisable(GL_DEPTH_TEST);
        glOrtho(-gBorder, 320 + gBorder, 240, 0, 0, 1);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        glTranslatef((xOffset - 160) / 160.0f, -(yOffset - 120) / 120.f, 0);
        gluPerspective(75.0f, (float)gWidth / (float)gHeight, (float)64, (float)(64 * 1024));
        glScalef(1, -1, -1);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderSetBlend(enum BlendMode mode)
{
    switch (mode)
    {
        case BLEND_NONE:
            glDisable(GL_BLEND);
            break;
        case BLEND_ADD:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            break;
        case BLEND_SUB:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BLEND_MUL:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
    }
}

void renderDepthWrite(int enable)
{
    glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

void renderSetColor(uint32 color)
{
    glColor3ubv((GLubyte*)&color);
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
            t->clut[i] = RGB5551(t->clut[i]);
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

    if (HACK_LOAD_TRACK)
    {
        ASSERT(w == 32);
        ASSERT(h == 32);
        FillTrackTex(tempData);
        return NULL;
    }
    else
    {
        tex = (TextureGL*)malloc(sizeof(TextureGL));
        tex->width = w;
        tex->height = h;
        tex->iw = 1.0f / w;
        tex->ih = 1.0f / h;

        glGenTextures(1, &tex->id);
        glBindTexture(GL_TEXTURE_2D, tex->id);

        if (HACK_LOAD_SHADOW)
        {
            uint16* src = tempData;
            uint8* dst = (uint8*)tempData;
            for (i = 0; i < w * h; i++)
            {
                int32 alpha = ((*src++ >> 1) & 31) << 3;
                alpha <<= 2;
                if (alpha > 255)
                    alpha = 255;
                *dst++ = alpha;
            }
            // create alpha only texture for the shadow
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tempData);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, tempData);
        }

        setTexFilter(HACK_LOAD_SHADOW);

        return (Texture*)tex;
    }
}
