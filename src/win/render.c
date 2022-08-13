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
#define i255    (1.0f / 255.0f)
#define i256    (1.0f / 256.0f)
#define i212    (1.0f / 212.0f)
#define i4096   (1.0f / 4096.0f)
#define IDEG    (360.0f / 4096.0f)

#define PI      3.14159265358979323846f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

typedef struct
{
    uint16 width;
    uint16 height;
    GLuint id;
    float iw;
    float ih;
} TextureGL;

typedef struct
{
    int32 x;
    int32 y;
    int32 z;
    float s, t;
    float r, g, b, a;
} VertexGL;

typedef uint16 IndexGL;

typedef struct
{
    float e[4][4];
} Matrix;

typedef struct Mesh
{
    int32 iStart;
    int32 iCount;
    struct Mesh* next;
} Mesh;

typedef struct
{
    Mesh* mesh;
    const TextureGL* texture;
    int32 projMatrix;
    int32 viewMatrix;
    int32 layer;
    BlendMode blend;
    uint8 depthWrite;
    uint8 depthTest;
} RenderState;

#define MAX_RENDER_VERTICES     (1024 * 16)
#define MAX_RENDER_INDICES      (1024 * 16)
#define MAX_RENDER_MATRICES     (128)
#define MAX_RENDER_MESHES       (1024)
#define MAX_RENDER_STATES       (512)

VertexGL gVertices[MAX_RENDER_VERTICES];
IndexGL gIndices[MAX_RENDER_INDICES];
IndexGL gIndicesMerged[MAX_RENDER_INDICES];
Matrix gMatrices[MAX_RENDER_MATRICES];
Mesh gMeshes[MAX_RENDER_MESHES];
RenderState gCurState;
RenderState gStates[MAX_RENDER_STATES];
RenderState* gStatePtr;

int32 gVerticesCount;
int32 gIndicesCount;
int32 gMatricesCount;
int32 gMeshesCount;
int32 gStatesCount;

uint16 tempData[320 * 240];

extern Texture* TextureTable[];
const TextureGL* gTexture;

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

extern const TrackCamera ctrlcamera;

int gRenderMode;

int32 xOffset;
int32 yOffset;

int gWidth, gHeight, gBorder;

int gCylX, gCylZ;

float gR, gG, gB;

#define MAX_TRACK_TEX   22
TextureGL gTrackTex[MAX_TRACK_TEX];
uint16 gTrackTexData[MAX_TRACK_TEX][128 * 128];

extern TrackData* track;
extern TexTemplate* ctrltexTemplate;
extern short ctrllibraryTextures;
extern short ctrlnumLib;

void matrixIdentity(Matrix* m)
{
    memset(m, 0, sizeof(Matrix));
    m->e[0][0] = m->e[1][1] = m->e[2][2] = m->e[3][3] = 1.0f;
}

void matrixOrtho(Matrix* m, float l, float r, float b, float t, float znear, float zfar)
{
    memset(m, 0, sizeof(Matrix));
    m->e[0][0] = 2.0f / (r - l);
    m->e[1][1] = 2.0f / (t - b);
    m->e[2][2] = 2.0f / (znear - zfar);
    m->e[3][3] = 1.0f;
    m->e[3][0] = (l + r) / (l - r);
    m->e[3][1] = (t + b) / (b - t);
    m->e[3][2] = (znear + zfar) / (znear - zfar);
}

void matrixPerspective(Matrix* m, float fov, float aspect, float xoff, float yoff, float znear, float zfar)
{
    float y = tanf(fov * 0.5f * DEG2RAD) * znear;
    float x = y;

    x = y * aspect;

    memset(m, 0, sizeof(Matrix));
    m->e[0][0] = znear / x;
    m->e[1][1] = znear / y;
    m->e[2][2] = (znear + zfar) / (znear - zfar);
    m->e[3][3] = 0.0f;
    m->e[2][0] = xoff;
    m->e[2][1] = yoff;
    m->e[2][3] = -1.0f;
    m->e[3][2] = 2.0f * zfar * znear / (znear - zfar);

    // wipEout specific axis flip (X, -Y, -Z)
    m->e[1][0] = -m->e[1][0];
    m->e[1][1] = -m->e[1][1];
    m->e[1][2] = -m->e[1][2];
    m->e[1][3] = -m->e[1][3];

    m->e[2][0] = -m->e[2][0];
    m->e[2][1] = -m->e[2][1];
    m->e[2][2] = -m->e[2][2];
    m->e[2][3] = -m->e[2][3];
}

int32 stateCmp(const RenderState* a, const RenderState* b)
{
    return (a->texture == b->texture) && 
           (a->projMatrix == b->projMatrix) &&
           (a->viewMatrix == b->viewMatrix) &&
           (a->layer == b->layer) &&
           (a->blend == b->blend) &&
           (a->depthWrite == b->depthWrite) &&
           (a->depthTest == b->depthTest);
}

void stateCheck()
{
    int32 i;
    Mesh* mesh;

    if (gStatePtr)
        return;

    for (i = 0; i < gStatesCount; i++)
    {
        if (stateCmp(&gCurState, gStates + i))
        {
            gStatePtr = gStates + i;
            break;
        }
    }

    if (!gStatePtr)
    {
        gStatePtr = gStates + gStatesCount++;
        *gStatePtr = gCurState;
        ASSERT(gStatePtr->mesh == NULL);
    }

    mesh = gMeshes + gMeshesCount++;

    mesh->iStart = gIndicesCount;
    mesh->iCount = 0;
    mesh->next = gStatePtr->mesh;

    gStatePtr->mesh = mesh;
}

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
    SwapBuffers(hDC);
}

void renderResize(int width, int height)
{
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
}

void renderOffset(int x, int y)
{
    xOffset = x;
    yOffset = y;
}

void renderSetBlend(int blend)
{
    if (gCurState.blend != blend)
    {
        gCurState.blend = blend;
        gStatePtr = NULL;
    }
}

void renderDepthWrite(int enable)
{
    if (gCurState.depthWrite != enable)
    {
        gCurState.depthWrite = enable;
        gStatePtr = NULL;
    }
}

void renderDepthTest(int enable)
{
    if (gCurState.depthTest != enable)
    {
        gCurState.depthTest = enable;
        gStatePtr = NULL;
    }
}

void renderSetColor(uint32 color)
{
    gR = 0.1f + 2.0f * i256 * (0xFF & (color));
    gG = 0.1f + 2.0f * i256 * (0xFF & (color >> 8));
    gB = 0.1f + 2.0f * i256 * (0xFF & (color >> 16));
    if (gR > 1.0f) gR = 1.0f;
    if (gG > 1.0f) gG = 1.0f;
    if (gB > 1.0f) gB = 1.0f;
    gR *= i256;
    gG *= i256;
    gB *= i256;
}

void renderNextLayer()
{
    gCurState.layer++;
    gStatePtr = NULL;
}

void renderSetTexture(const TextureGL* texture)
{
    if (gCurState.texture != texture)
    {
        gCurState.texture = texture;
        gStatePtr = NULL;
    }
}

void renderClear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void addTriangle()
{
    ASSERT(gIndicesCount + 3 <= MAX_RENDER_INDICES);

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 1;
    gIndices[gIndicesCount++] = gVerticesCount + 2;

    gStatePtr->mesh->iCount += 3;
}

void addQuad()
{
    ASSERT(gIndicesCount + 6 <= MAX_RENDER_INDICES);

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 1;
    gIndices[gIndicesCount++] = gVerticesCount + 2;

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 2;
    gIndices[gIndicesCount++] = gVerticesCount + 3;

    gStatePtr->mesh->iCount += 6;
}

void addVertex(int32 x, int32 y, int32 z, float s, float t, float r, float g, float b, float a)
{
    ASSERT(gVerticesCount < MAX_RENDER_VERTICES);

    VertexGL* v = gVertices + gVerticesCount++;
    v->x = x;
    v->y = y;
    v->z = z;
    v->s = s;
    v->t = t;
    v->r = r;
    v->g = g;
    v->b = b;
    v->a = a;
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

    renderSetTexture(t);
    stateCheck();

    addQuad();
    addVertex(x0, y0, 0, u0, v0, 0.5f, 0.5f, 0.5f, 1.0f);
    addVertex(x1, y0, 0, u1, v0, 0.5f, 0.5f, 0.5f, 1.0f);
    addVertex(x1, y1, 0, u1, v1, 0.5f, 0.5f, 0.5f, 1.0f);
    addVertex(x0, y1, 0, u0, v1, 0.5f, 0.5f, 0.5f, 1.0f);
}

void renderShadow(const Shadow* s)
{
    TextureGL* t = (TextureGL*)s->texture;
    float intensity = s->intensity * i255;

    renderSetTexture(t);
    stateCheck();

    addQuad();
    addVertex(s->x0, s->y0, s->z0, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, intensity);
    addVertex(s->x1, s->y1, s->z1, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, intensity);
    addVertex(s->x2, s->y2, s->z2, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, intensity);
    addVertex(s->x3, s->y3, s->z3, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, intensity);
}

void renderColorQuad(const ColorQuad* q)
{
    renderSetTexture(NULL);
    stateCheck();

    addQuad();
    addVertex(q->x0, q->y0, 0, 0.0f, 0.0f, q->r0 * i255, q->g0 * i255, q->b0 * i255, 1.0f);
    addVertex(q->x1, q->y1, 0, 0.0f, 0.0f, q->r1 * i255, q->g1 * i255, q->b1 * i255, 1.0f);
    addVertex(q->x2, q->y2, 0, 0.0f, 0.0f, q->r2 * i255, q->g2 * i255, q->b2 * i255, 1.0f);
    addVertex(q->x3, q->y3, 0, 0.0f, 0.0f, q->r3 * i255, q->g3 * i255, q->b3 * i255, 1.0f);
}

void renderBillboard(const Billboard* b)
{
    int sx, sy, sz, size;
    TextureGL* t = (TextureGL*)b->texture;

    size = b->size * 8;
    sx = size * gCylX >> 12;
    sy = size;
    sz = size * gCylZ >> 12;

    renderSetTexture(t);
    stateCheck();

    addQuad();
    addVertex(b->x - sx, b->y - sy, b->z - sz, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f);
    addVertex(b->x + sx, b->y - sy, b->z + sz, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f);
    addVertex(b->x + sx, b->y + sy, b->z + sz, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f);
    addVertex(b->x - sx, b->y + sy, b->z - sz, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f);
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
    Matrix* m;

    gBorder = ((gWidth * 240) / gHeight - 320) >> 1;

    gCylX = rcos(-ctrlcamera.hdg);
    gCylZ = rsin(-ctrlcamera.hdg);

    gRenderMode = RENDER_MODE_3D;
    renderSetMode(RENDER_MODE_2D);
    renderSetColor(0xFFFFFFFF);

    // identity matrix
    m = gMatrices + 0;
    matrixIdentity(m);

    // otho projection matrix
    m = gMatrices + 1;
    matrixOrtho(m, -gBorder, 320 + gBorder, 240, 0, 0, 1);

    gMatricesCount = 2;

    gCurState.texture = NULL;
    gCurState.blend = BLEND_NONE;
    gCurState.projMatrix = 1;
    gCurState.viewMatrix = 0;
    gCurState.layer = 0;
    gCurState.blend = BLEND_NONE;
    gCurState.depthWrite = 1;
    gCurState.depthTest = 0;
    gStatePtr = NULL;
}

void renderFrameEnd(void)
{
    int32 i, start, count;
    const RenderState* state;
    const Mesh* mesh;

    const TextureGL* texture = NULL;
    int32 projMatrix = 1;
    int32 viewMatrix = 0;
    int32 depthTest = 0;
    int32 depthWrite = 1;
    BlendMode blend = BLEND_NONE;

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*)(gMatrices + projMatrix));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((GLfloat*)(gMatrices + viewMatrix));

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_INT, sizeof(VertexGL), &gVertices->x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexGL), &gVertices->s);
    glColorPointer(4, GL_FLOAT, sizeof(VertexGL), &gVertices->r);

    count = 0;
    for (i = 0; i < gStatesCount; i++)
    {
        state = gStates + i;

        start = count;

        mesh = state->mesh;
        ASSERT(mesh);
        while (mesh)
        {
            memcpy(gIndicesMerged + count, gIndices + mesh->iStart, mesh->iCount * sizeof(IndexGL));
            count += mesh->iCount;
            mesh = mesh->next;
        }

        state->mesh->next = NULL;
        state->mesh->iStart = start;
        state->mesh->iCount = count - start;
    }

    for (i = 0; i < gStatesCount; i++)
    {
        state = gStates + i;

        // set texture
        if (texture != state->texture)
        {
            texture = state->texture;
            glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
        }

        // set proj matrix
        if (projMatrix != state->projMatrix)
        {
            projMatrix = state->projMatrix;
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf((GLfloat*)(gMatrices + projMatrix));
        }

        if (viewMatrix != state->viewMatrix)
        {
            viewMatrix = state->viewMatrix;
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf((GLfloat*)(gMatrices + viewMatrix));
        }

        // set depth test
        if (depthTest != state->depthTest)
        {
            depthTest = state->depthTest;
            if (depthTest)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
        }

        // set depth write
        if (depthWrite != state->depthWrite)
        {
            depthWrite = state->depthWrite;
            glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);
        }

        // set blending
        if (blend != state->blend)
        {
            blend = state->blend;
            switch (blend)
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

        mesh = state->mesh;
        ASSERT(mesh);
        ASSERT(mesh->next == NULL);
        glDrawElements(GL_TRIANGLES, mesh->iCount, GL_UNSIGNED_SHORT, gIndicesMerged + mesh->iStart);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

#ifdef DEBUG_RENDER
    {
        static int32 dbg_v, dbg_i, dbg_x, dbg_m, dbg_s;
        if (dbg_v < gVerticesCount)
            dbg_v = gVerticesCount;
        if (dbg_i < gIndicesCount)
            dbg_i = gIndicesCount;
        if (dbg_x < gMatricesCount)
            dbg_x = gMatricesCount;
        if (dbg_m < gMeshesCount)
            dbg_m = gMeshesCount;
        if (dbg_s < gStatesCount)
            dbg_s = gStatesCount;

        char buf[256];
        sprintf(buf, "v:%d i:%d x:%d m:%d s:%d\n", dbg_v, dbg_i, dbg_x, dbg_m, dbg_s);
        OutputDebugString(buf);    
    }
#endif

    gVerticesCount = 0;
    gIndicesCount = 0;
    gMatricesCount = 0;
    gMeshesCount = 0;
    gStatesCount = 0;
}

void renderSetMatrix(const MATRIX* matrix, int x, int y, int z)
{
    Matrix* m = gMatrices + gMatricesCount;

    gCurState.viewMatrix = gMatricesCount++;
    gStatePtr = NULL;

    m->e[0][0] = matrix->m[0][0] * i4096;
    m->e[0][1] = matrix->m[1][0] * i4096;
    m->e[0][2] = matrix->m[2][0] * i4096;
    m->e[0][3] = 0.0f;

    m->e[1][0] = matrix->m[0][1] * i4096;
    m->e[1][1] = matrix->m[1][1] * i4096;
    m->e[1][2] = matrix->m[2][1] * i4096;
    m->e[1][3] = 0.0f;

    m->e[2][0] = matrix->m[0][2] * i4096;
    m->e[2][1] = matrix->m[1][2] * i4096;
    m->e[2][2] = matrix->m[2][2] * i4096;
    m->e[2][3] = 0.0f;

    m->e[3][0] = x;
    m->e[3][1] = y;
    m->e[3][2] = z;
    m->e[3][3] = 1.0f;
}

void renderSetSkeleton(Skeleton* skeleton, int x, int y, int z)
{
    renderSetMatrix(GetSkeletonMatrix(skeleton), x, y, z);
}

void renderSection(const TrackSection* section)
{
    const Face* face = track->faces + section->faceStart;
    const VECTOR* vertices = track->vertices;
    const VECTOR* v;
    int i;
    float r, g, b, u0, u1;

    for (i = 0; i < section->faceCount; i++, face++)
    {
        ASSERT(face->texture < ctrlnumLib);

    #ifdef CHECK_UNUSED_TEX
        texUsed[face->texture] = 1;
    #endif

        renderSetTexture(gTrackTex + face->texture);
        stateCheck();

        r = i255 * face->red;
        g = i255 * face->green;
        b = i255 * face->blue;

        if (face->flags & FLIP_TEXTURE)
        {
            u0 = 0.0f;
            u1 = 1.0f;
        }
        else
        {
            u0 = 1.0f;
            u1 = 0.0f;
        }

        addQuad();
        v = vertices + face->vertex[3];
        addVertex(v->vx, v->vy, v->vz, u0, 1.0f, r, g, b, 1.0f);
        v = vertices + face->vertex[2];
        addVertex(v->vx, v->vy, v->vz, u1, 1.0f, r, g, b, 1.0f);
        v = vertices + face->vertex[1];
        addVertex(v->vx, v->vy, v->vz, u1, 0.0f, r, g, b, 1.0f);
        v = vertices + face->vertex[0];
        addVertex(v->vx, v->vy, v->vz, u0, 0.0f, r, g, b, 1.0f);
    }
}

void renderTrack(void)
{
    const VECTOR* vertices = track->vertices;
    const Face* face;
    int i, j, s, id;
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

    section = track->sections + sections[0][0];
    renderSection(section->prevSection.ptr);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < counts[i]; j += 2)
        {
            section = track->sections + sections[i][j];

            for (s = 0; s < sections[i][j + 1]; s++)
            {
                renderSection(section);
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
    const SVECTOR* v;
    int i;
    int cx, cy, cz;
    float r, g, b;

    matrix = GetSkeletonMatrix(object->skeleton);

    cx = matrix->t[0];
    cy = matrix->t[1];
    cz = matrix->t[2];

    renderSetSkeleton(object->skeleton, cx, cy, cz);

    vertices = object->vertices;
    poly.primitive = object->primitives;

    for (i = 0; i < object->primitiveCount; i++)
    {
        switch (poly.primitive->type)
        {
            case TypeF3:
            {
                const F3* f = poly.f3++;

                r = gR * f->colour.r;
                g = gG * f->colour.g;
                b = gB * f->colour.b;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                break;
            }

            case TypeLSF3:
            {
                const LSF3* f = poly.lsf3++;

                r = gR * f->colour.r;
                g = gG * f->colour.g;
                b = gB * f->colour.b;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                break;
            }

            case TypeFT3:
            {
                const FT3* f = poly.ft3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                if (f->flag & ShipEngine)
                {
                    renderSetBlend(BLEND_ADD);
                    r = 1.0f;
                    g = 1.0f;
                    b = 1.0f;
                }
                else
                {
                    r = gR * f->colour.r;
                    g = gG * f->colour.g;
                    b = gB * f->colour.b;
                }

                renderSetTexture(t);
                stateCheck();

                addTriangle();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, r, g, b, 1.0f);

                if (f->flag & ShipEngine)
                {
                    renderSetBlend(BLEND_NONE);
                }
                break;
            }

            case TypeLSFT3:
            {
                const LSFT3* f = poly.lsft3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                r = gR * f->colour.r;
                g = gG * f->colour.g;
                b = gB * f->colour.b;

                renderSetTexture(t);
                stateCheck();

                addTriangle();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, r, g, b, 1.0f);
                break;
            }

            case TypeF4:
            {
                const F4* f = poly.f4++;

                r = gR * f->colour.r;
                g = gG * f->colour.g;
                b = gB * f->colour.b;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                break;
            }

            case TypeLSF4:
            {
                const LSF4* f = poly.lsf4++;

                r = gR * f->colour.r;
                g = gG * f->colour.g;
                b = gB * f->colour.b;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, r, g, b, 1.0f);
                break;
            }

            case TypeFT4:
            {
                const FT4* f = poly.ft4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                r = gR * f->colour.r;
                g = gG * f->colour.g;
                b = gB * f->colour.b;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, f->u3 * t->iw, f->v3 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, r, g, b, 1.0f);
                break;
            }

            case TypeLSFT4:
            {
                const LSFT4* f = poly.lsft4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                r = gR * f->colour.r;
                g = gG * f->colour.g;
                b = gB * f->colour.b;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, f->u3 * t->iw, f->v3 * t->ih, r, g, b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, r, g, b, 1.0f);
                break;
            }

            case TypeG3:
            {
                const G3* f = poly.g3++;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                break;
            }

            case TypeLSG3:
            {
                const LSG3* f = poly.lsg3++;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                break;
            }

            case TypeGT3:
            {
                const GT3* f = poly.gt3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);

                if (f->flag & ShipEngine)
                {
                    renderSetBlend(BLEND_ADD);
                    stateCheck();

                    addTriangle();
                    v = vertices + f->coords[0];
                    addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, 1.0f, 1.0f, 1.0f, 1.0f);
                    v = vertices + f->coords[1];
                    addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, 1.0f, 1.0f, 1.0f, 1.0f);
                    v = vertices + f->coords[2];
                    addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, 1.0f, 1.0f, 1.0f, 1.0f);

                    renderSetBlend(BLEND_NONE);
                }
                else
                {
                    stateCheck();

                    addTriangle();
                    v = vertices + f->coords[0];
                    addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                    v = vertices + f->coords[1];
                    addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                    v = vertices + f->coords[2];
                    addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                }
                break;
            }

            case TypeLSGT3:
            {
                const LSGT3* f = poly.lsgt3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addTriangle();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                break;
            }

            case TypeG4:
            {
                const G4* f = poly.g4++;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[3].r, gG * f->colour[3].g, gB * f->colour[3].b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                break;
            }

            case TypeLSG4:
            {
                const LSG4* f = poly.lsg4++;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[3].r, gG * f->colour[3].g, gB * f->colour[3].b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, 0.0f, 0.0f, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                break;
            }

            case TypeGT4:
            {
                const GT4* f = poly.gt4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, f->u3 * t->iw, f->v3 * t->ih, gR * f->colour[3].r, gG * f->colour[3].g, gB * f->colour[3].b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                break;
            }

            case TypeLSGT4:
            {
                const LSGT4* f = poly.lsgt4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addQuad();
                v = vertices + f->coords[0];
                addVertex(v->vx, v->vy, v->vz, f->u0 * t->iw, f->v0 * t->ih, gR * f->colour[0].r, gG * f->colour[0].g, gB * f->colour[0].b, 1.0f);
                v = vertices + f->coords[1];
                addVertex(v->vx, v->vy, v->vz, f->u1 * t->iw, f->v1 * t->ih, gR * f->colour[1].r, gG * f->colour[1].g, gB * f->colour[1].b, 1.0f);
                v = vertices + f->coords[3];
                addVertex(v->vx, v->vy, v->vz, f->u3 * t->iw, f->v3 * t->ih, gR * f->colour[3].r, gG * f->colour[3].g, gB * f->colour[3].b, 1.0f);
                v = vertices + f->coords[2];
                addVertex(v->vx, v->vy, v->vz, f->u2 * t->iw, f->v2 * t->ih, gR * f->colour[2].r, gG * f->colour[2].g, gB * f->colour[2].b, 1.0f);
                break;
            }

            case TypeTSPR:
            case TypeBSPR:
            {
                const SPR* f = poly.spr;

                int w, h0, h1;
                int sx, sz;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                w = f->width >> 1;
                if (poly.primitive->type == TypeTSPR)
                {
                    h0 = 0;
                    h1 = f->height;
                }
                else
                {
                    h0 = f->height;
                    h1 = 0;
                }
                poly.spr++;

                v = vertices + f->coord;
                sx = gCylX * w >> 12;
                sz = gCylZ * w >> 12;

                r = i255 * f->colour.r;
                g = i255 * f->colour.g;
                b = i255 * f->colour.b;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertex(v->vx - sx, v->vy - h0, v->vz - sz, 0.0f, 0.0f, r, g, b, 1.0f);
                addVertex(v->vx + sx, v->vy - h0, v->vz + sz, 1.0f, 0.0f, r, g, b, 1.0f);
                addVertex(v->vx + sx, v->vy + h1, v->vz + sz, 1.0f, 1.0f, r, g, b, 1.0f);
                addVertex(v->vx - sx, v->vy + h1, v->vz - sz, 0.0f, 1.0f, r, g, b, 1.0f);
                break;
            }

            case TypeInfiniteLight:
                poly.infiniteLight++;
                break;

            case TypePointLight:
                poly.pointLight++;
                break;

            case TypeSpotLight:
                poly.spotLight++;
                break;

            default:
                ASSERT(0);
                break;
        }
    }
}

void renderSky(const Object* object)
{
    renderDepthTest(0);
    renderObject(object);
    renderDepthTest(1);
}

void renderSetMode(int renderMode)
{
    if (gRenderMode == renderMode)
        return;
    gRenderMode = renderMode;

    if (renderMode == RENDER_MODE_2D)
    {
        renderDepthTest(0);
        gCurState.viewMatrix = 0;
        gCurState.projMatrix = 1;
        gStatePtr = NULL;
    }
    else
    {
        renderDepthTest(1);

        matrixPerspective(gMatrices + gMatricesCount, 75.0f, (float)gWidth / (float)gHeight, -xOffset / 160.0f, yOffset / 120.f, (float)64, (float)(64 * 1024));

        gCurState.viewMatrix = 0;
        gCurState.projMatrix = gMatricesCount++;
        gStatePtr = NULL;
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
