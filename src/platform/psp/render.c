#include <pspgu.h>
#include <pspgum.h>

#include "game.h"

#define GU_TFX_MODULATE2X   (GU_TFX_MODULATE | (1 << 16))

#define BUF_WIDTH   512
#define SCR_WIDTH   480
#define SCR_HEIGHT  272

#define i255    (1.0f / 255.0f)
#define i4096   (1.0f / 4096.0f)

#define Z_NEAR  128
#define Z_FAR   65535

typedef struct VT2D
{
    uint16 s, t;
    int16 x, y, z;
} VT2D;

typedef struct VC2D
{
    uint32 color;
    int16 x, y, z;
} VC2D;

typedef struct V3D_TRACK
{
    float s, t;
    uint32 color;
    float x, y, z;
} V3D_TRACK;

typedef struct V3D_OBJ
{
    float s, t; // use int16 fixed point UVs (pre-process)
    uint32 color;
    int16 x, y, z;
} V3D_OBJ;

#define VT2D_FMT        (GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D)
#define VC2D_FMT        (GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D)
#define V3D_TRACK_FMT   (GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D)
#define V3D_OBJ_FMT     (GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_3D)

#define GU_ALLOC(type, count) (type*)sceGuGetMemory(count * sizeof(type))
#define VT2D_SET(v, _s, _t, _x, _y) v.s = _s; v.t = _t; v.x = _x; v.y = _y; v.z = 0
#define VC2D_SET(v, _color, _x, _y) v.color = _color; v.x = _x; v.y = _y; v.z = 0
#define V3D_SET(v, _s, _t, _color, _x, _y, _z) v.s = _s; v.t = _t; v.color = _color; v.x = _x; v.y = _y; v.z = _z
#define V3D_SET_FACE(i, face, vertices, V3D) V3D_SET(V3D[i], face->uv[i][0] * iw, face->uv[i][1] * ih, face->color[i], vertices[face->index[i]].x, vertices[face->index[i]].y, vertices[face->index[i]].z)
#define VC3D_SET_FACE(i, _color, face, vertices, V3D) V3D_SET(V3D[i], face->uv[i][0] * iw, face->uv[i][1] * ih, _color, vertices[face->index[i]].x, vertices[face->index[i]].y, vertices[face->index[i]].z)

int32 xOffset;
int32 yOffset;

int32 gCylX, gCylZ;
int32 gBorderX, gBorderY;

uint16* gHightlightTextCLUT;

uint32 __attribute__((aligned(16))) gCmdList[256 << 10];

uint8* pEDRAM;

typedef struct
{
    float e[4][4];
} mat4;

mat4 mProj;

const mat4 mIdentity = {{
    { 1, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 0, 1 }
}};

const mat4 mScale = {{
    { 32767, 0, 0, 0 },
    { 0, 32767, 0, 0 },
    { 0, 0, 32767, 0 },
    { 0, 0, 0, 1 }
}};

void matrixPerspective(mat4* m, float fov, float aspect, float xoff, float yoff, float znear, float zfar)
{
    float y = tanf(fov * 0.5f * DEG2RAD) * znear;
    float x = y;

    x = y * aspect;

    memset(m, 0, sizeof(mat4));
    m->e[0][0] = znear / x;
    m->e[1][1] = znear / y;
    m->e[2][2] = (znear + zfar) / (znear - zfar);
    m->e[3][3] = 0.0f;
    m->e[2][0] = xoff;
    m->e[2][1] = yoff;
    m->e[2][3] = 1.0f;
    m->e[3][2] = 2.0f * zfar * znear / (znear - zfar);

    // wipEout specific axis flip (X, -Y, -Z)
    m->e[1][0] = -m->e[1][0];
    m->e[1][1] = -m->e[1][1];
    m->e[1][2] = -m->e[1][2];
    m->e[1][3] = -m->e[1][3];

    m->e[2][0] = -m->e[2][0];
    m->e[2][1] = -m->e[2][1];
    m->e[2][2] = -m->e[2][2];
    //m->e[2][3] = -m->e[2][3]; // already inverted
}

uint32 nextPow2(uint32 x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

void renderInit()
{
    const uint16 HIGHLIGHT_TEXT_CLUT[] = {
        0x0000, 0x8442, 0x8445, 0x8487,
        0x80CB, 0x890C, 0x890E, 0x8950,
        0x8993, 0x89D5, 0x8A17, 0x8E57,
        0x8E59, 0x8A9B, 0x8ADD, 0x8B1F
    };

    gWidth = SCR_WIDTH;
    gHeight = SCR_HEIGHT;

    gBorderX = (SCR_WIDTH - 320) >> 1;
    gBorderY = (SCR_HEIGHT - 240) >> 1;

    sceGuInit();
    sceGuStart(GU_DIRECT, gCmdList);

    sceGuDrawBuffer(GU_PSM_5650, (void*)0, BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)(BUF_WIDTH * SCR_HEIGHT * 2), BUF_WIDTH);
    sceGuDepthBuffer((void*)(BUF_WIDTH * SCR_HEIGHT * 2 * 2), BUF_WIDTH);

    sceGuOffset(2048 - SCR_WIDTH / 2, 2048 - SCR_HEIGHT / 2);
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);

    sceGuEnable(GU_SCISSOR_TEST);
    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);

    sceGuClearDepth(0xFFFF);
    sceGuClearColor(0x0000); // TODO fix screen flickering (use 0xFF00 for debug)

    sceGuDepthFunc(GU_LEQUAL);
    sceGuDepthRange(Z_NEAR, Z_FAR);

    sceGuEnable(GU_CLIP_PLANES);
    sceGuEnable(GU_CULL_FACE);
    sceGuFrontFace(GU_CW);

    sceGuShadeModel(GU_SMOOTH);

    sceGuEnable(GU_ALPHA_TEST);
    sceGuAlphaFunc(GU_GREATER, 0, 0xFF);

    sceGuAmbient(0xFF800000);
    sceGuAmbientColor(0xFF800000);
    sceGuColor(0xFFFFFFFF);
    sceGuColorMaterial(GU_AMBIENT | GU_DIFFUSE);

    sceGuDisable(GU_BLEND);

    //sceGuEnable(GU_TEXTURE_2D);
    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuTexScale(1.0f, 1.0f);
    sceGuTexOffset(0.0f, 0.0f);
    //sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuClutMode(GU_PSM_5551, 0, 0xFF, 0);

    sceGuFinish();
    sceGuSync(GU_SYNC_WAIT, GU_SYNC_FINISH);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadMatrix((ScePspFMatrix4*)&mScale);

    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);

    sceKernelDcacheWritebackAll();

    pEDRAM = (uint8*)sceGeEdramGetAddr();
    pEDRAM += (512 * 272 * 2 * 3); // double color buffer + depth buffer

    // upload yellow text CLUT
    gHightlightTextCLUT = (uint16*)pEDRAM;
    memcpy(pEDRAM, HIGHLIGHT_TEXT_CLUT, sizeof(HIGHLIGHT_TEXT_CLUT));
    pEDRAM += sizeof(HIGHLIGHT_TEXT_CLUT);
}

void renderFree(void)
{
    sceGuTerm();
}

void renderSetVSync(int32 enable)
{
    //
}

void renderSwap(void)
{
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}

void renderResize(int width, int height)
{
    //
}

void renderOffset(int x, int y)
{
    xOffset = x;
    yOffset = y;
}

void renderSetBlend(int blend)
{
    switch (blend)
    {
        case BLEND_ADD:
            sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0xFF808080, 0xFFFFFFFF);
            break;
        case BLEND_SUB:
            sceGuBlendFunc(GU_REVERSE_SUBTRACT, GU_FIX, GU_FIX, 0xFF808080, 0xFFFFFFFF); // TODO shadow intensity
            break; 
        case BLEND_MUL:
            sceGuBlendFunc(GU_ADD, GU_DST_COLOR, GU_FIX, 0, 0);
            break;
        default:
            sceGuDisable(GU_BLEND);
            return;
    }
    sceGuEnable(GU_BLEND);
}

void renderDepthWrite(int enable)
{
    sceGuDepthMask(enable ? 0 : 1);
}

void renderDepthTest(int enable)
{
    if (enable)
        sceGuEnable(GU_DEPTH_TEST);
    else
        sceGuDisable(GU_DEPTH_TEST);
}

void renderNextLayer()
{
    //
}

void renderSetTexture(const Texture* texture)
{
    uint8* data;

    if (!texture)
    {
        sceGuDisable(GU_TEXTURE_2D);
        return;
    }

    sceGuEnable(GU_TEXTURE_2D);

    data = texture->addr;

    if (texture->type == TIM_4)
    {
        sceGuTexMode(GU_PSM_T4, 0, 0, GU_FALSE);
        sceGuClutLoad(1, data);
        data += sizeof(uint16) * 16;
    }
    else if (texture->type == TIM_8)
    {
        sceGuTexMode(GU_PSM_T8, 0, 0, GU_FALSE);
        sceGuClutLoad(16, data);
        data += sizeof(uint16) * 256;
    }
    else if (texture->type == TIM_16)
    {
        sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
    }

    sceGuTexImage(0, texture->widthPOT, texture->heightPOT, texture->widthPOT, data);
    sceGuTexFlush();
}

void renderClear(void)
{
//    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_FAST_CLEAR_BIT);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_STENCIL_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
}

void renderGlyph(const Glyph* g, int32 x, int32 y, uint32 color)
{
    const Texture* t = g->texture;

    uint16 u0 = g->x;
    uint16 v0 = g->y;
    uint16 u1 = (g->x + g->w);
    uint16 v1 = (g->y + g->h);

    int16 x0 = gBorderX + x;
    int16 y0 = gBorderY + y;
    int16 x1 = x0 + g->w;
    int16 y1 = y0 + g->h;

    VT2D* v = GU_ALLOC(VT2D, 2);
    VT2D_SET(v[0], u0, v0, x0, y0);
    VT2D_SET(v[1], u1, v1, x1, y1);

    renderSetTexture(t);
    if (color == TEXT_COLOR_HIGHLIGHT)
    {
        sceGuClutLoad(1, gHightlightTextCLUT);
    }

    sceGumDrawArray(GU_SPRITES, VT2D_FMT, 2, NULL, v);
}

void renderImage(const Texture* t, int32 x, int32 y)
{
    uint16 u0 = 0;
    uint16 v0 = 0;
    uint16 u1 = t->width;
    uint16 v1 = t->height;
    int16 x0 = gBorderX + x;
    int16 y0 = gBorderY + y;
    int16 x1 = x0 + t->width;
    int16 y1 = y0 + t->height;

    VT2D* v = GU_ALLOC(VT2D, 2);
    VT2D_SET(v[0], u0, v0, x0, y0);
    VT2D_SET(v[1], u1, v1, x1, y1);

    renderSetTexture(t);
    sceGumDrawArray(GU_SPRITES, VT2D_FMT, 2, NULL, v);
}

void renderBackground(const Texture* t)
{
    int32 w = t->width * SCR_HEIGHT / t->height;
    uint16 u0 = 0.0f;
    uint16 v0 = 0.0f;
    uint16 u1 = t->width;
    uint16 v1 = t->height;
    int16 x0 = (SCR_WIDTH - w) >> 1;
    int16 y0 = 0;
    int16 x1 = x0 + w;
    int16 y1 = SCR_HEIGHT;

    VT2D* v = GU_ALLOC(VT2D, 2);
    VT2D_SET(v[0], u0, v0, x0, y0);
    VT2D_SET(v[1], u1, v1, x1, y1);

    renderSetTexture(t);
    sceGumDrawArray(GU_SPRITES, VT2D_FMT, 2, NULL, v);
}

void renderColorQuad(const ColorQuad* q)
{
    VC2D* v = GU_ALLOC(VC2D, 4);
    VC2D_SET(v[0], q->color[0], q->x0 + gBorderX, q->y0 + gBorderY);
    VC2D_SET(v[1], q->color[1], q->x1 + gBorderX, q->y1 + gBorderY);
    VC2D_SET(v[2], q->color[2], q->x2 + gBorderX, q->y2 + gBorderY);
    VC2D_SET(v[3], q->color[3], q->x3 + gBorderX, q->y3 + gBorderY);

    renderSetTexture(NULL);
    sceGumDrawArray(GU_TRIANGLE_FAN, VC2D_FMT, 4, NULL, v);
}

void renderBackFill(int x, int y, int w, int h)
{
    ColorQuad q;
    q.x0 = x;
    q.y0 = y;
    q.x1 = x + w;
    q.y1 = y;
    q.x2 = x + w;
    q.y2 = y + h;
    q.x3 = x;
    q.y3 = y + h;
    q.color[0] =
    q.color[1] =
    q.color[2] =
    q.color[3] = 0xFF808080;

    renderNextLayer();
    renderSetBlend(BLEND_MUL);
    renderColorQuad(&q);
    renderSetBlend(BLEND_NONE);
}

#define SHADOW_SHIFT_N  3
#define SHADOW_SHIFT_W  4

extern Texture* gShadowTex;

void renderShadow(int32 type, int32 intensity, int32 px, int32 py, int32 pz, int32 dx, int32 dy, int32 dz, int32 rx, int32 ry, int32 rz)
{
    V3D_TRACK* v = GU_ALLOC(V3D_TRACK, 4);

    uint32 color = (intensity << 24) | 0xFFFFFF;

    dx >>= SHADOW_SHIFT_N;
    dy >>= SHADOW_SHIFT_N;
    dz >>= SHADOW_SHIFT_N;

    rx >>= SHADOW_SHIFT_W;
    ry >>= SHADOW_SHIFT_W;
    rz >>= SHADOW_SHIFT_W;

    V3D_SET(v[0], 0.0f, 0.0f, color, px + dx - rx, py + dy - ry, pz + dz - rz);
    V3D_SET(v[1], 1.0f, 0.0f, color, px + dx + rx, py + dy + ry, pz + dz + rz);
    V3D_SET(v[2], 1.0f, 1.0f, color, px - dx + rx, py - dy + ry, pz - dz + rz);
    V3D_SET(v[3], 0.0f, 1.0f, color, px - dx - rx, py - dy - ry, pz - dz - rz);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadMatrix((ScePspFMatrix4*)&mIdentity);
    sceGumMatrixMode(GU_VIEW);

    renderSetTexture(gShadowTex);
    sceGumDrawArray(GU_TRIANGLE_FAN, V3D_TRACK_FMT, 4, NULL, v);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadMatrix((ScePspFMatrix4*)&mScale);
    sceGumMatrixMode(GU_VIEW);
}

void renderPrepare(void)
{
    //
}

void renderSetMatrix(const Matrix* matrix)
{
    mat4 m;

    m.e[0][0] = matrix->e00 * i4096;
    m.e[0][1] = matrix->e10 * i4096;
    m.e[0][2] = matrix->e20 * i4096;
    m.e[0][3] = 0.0f;

    m.e[1][0] = matrix->e01 * i4096;
    m.e[1][1] = matrix->e11 * i4096;
    m.e[1][2] = matrix->e21 * i4096;
    m.e[1][3] = 0.0f;

    m.e[2][0] = matrix->e02 * i4096;
    m.e[2][1] = matrix->e12 * i4096;
    m.e[2][2] = matrix->e22 * i4096;
    m.e[2][3] = 0.0f;

    m.e[3][0] = matrix->e03;
    m.e[3][1] = matrix->e13;
    m.e[3][2] = matrix->e23;
    m.e[3][3] = 1.0f;

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadMatrix((ScePspFMatrix4*)&m);
}

void renderFrameBegin(void)
{
    // get fixed right vector for sprites
    rsincos(gCamera.hdg, gCylZ, gCylX);    

    sceGuStart(GU_DIRECT, gCmdList);

    renderSetMode(RENDER_MODE_2D);
}

void renderFrameEnd(void)
{
    sceGuFinish();
    sceGuSync(GU_SYNC_WAIT, GU_SYNC_FINISH);
}

#define AVG(p, a, b)\
    p.x = ((a)->x + (b)->x) >> 1;\
    p.y = ((a)->y + (b)->y) >> 1;\
    p.z = ((a)->z + (b)->z) >> 1

void addTrackFace(V3D_TRACK* v, const vec3i* v0, const vec3i* v1, const vec3i* v2, const vec3i* v3, int32 texIndex, uint32 color, int32 flip)
{
    float u0, u1;

    if (flip)
    {
        u0 = 0.0f;
        u1 = 1.0f;
    }
    else
    {
        u0 = 1.0f;
        u1 = 0.0f;
    }

    V3D_SET(v[0], u0, 1.0f, color, v3->x, v3->y, v3->z);
    V3D_SET(v[1], u1, 1.0f, color, v2->x, v2->y, v2->z);
    V3D_SET(v[2], u1, 0.0f, color, v1->x, v1->y, v1->z);
    V3D_SET(v[3], u0, 0.0f, color, v0->x, v0->y, v0->z);

    renderSetTexture(gTextures + ctrllibraryTextures + texIndex);
    sceGumDrawArray(GU_TRIANGLE_FAN, V3D_TRACK_FMT, 4, NULL, v);
}

void drawSection1(const TrackSection* section)
{
    const Face* face = gTrack.faces + section->faceStart;
    const vec3i* vertices = gTrack.vertices;
    const vec3i *v0, *v1, *v2, *v3;
    int32 i;
    int32 flip;
    uint32 color;

    V3D_TRACK* v = GU_ALLOC(V3D_TRACK, section->faceCount * 4);

    for (i = 0; i < section->faceCount; i++, face++)
    {
        flip = (face->flags & FLIP_TEXTURE);
        color = face->color;

        v0 = vertices + face->vertex[0];
        v1 = vertices + face->vertex[1];
        v2 = vertices + face->vertex[2];
        v3 = vertices + face->vertex[3];

        addTrackFace(v, v0, v1, v2, v3, ctrltexTemplate[face->texture].tex1[0], color, flip);
        v += 4;
    }
}

void drawSection4(const TrackSection* section)
{
    const Face* face = gTrack.faces + section->faceStart;
    const vec3i* vertices = gTrack.vertices;
    const vec3i *v0, *v1, *v2, *v3;
    vec3i v01, v12, v23, v30, vM;
    int32 i;
    int32 flip;
    uint32 color;

    V3D_TRACK* v = GU_ALLOC(V3D_TRACK, section->faceCount * 4 * 4);

    for (i = 0; i < section->faceCount; i++, face++)
    {
        flip = (face->flags & FLIP_TEXTURE);
        color = face->color;

        v0 = vertices + face->vertex[0];
        v1 = vertices + face->vertex[1];
        v2 = vertices + face->vertex[2];
        v3 = vertices + face->vertex[3];

        AVG(v01, v0, v1);
        AVG(v12, v1, v2);
        AVG(v23, v2, v3);
        AVG(v30, v3, v0);
        AVG(vM, &v01, &v23);

        if (flip)
        {
            addTrackFace(v, v0, &v01, &vM, &v30, ctrltexTemplate[face->texture].tex4[0], color, flip); v += 4;
            addTrackFace(v, &v01, v1, &v12, &vM, ctrltexTemplate[face->texture].tex4[1], color, flip); v += 4;
            addTrackFace(v, &v30, &vM, &v23, v3, ctrltexTemplate[face->texture].tex4[2], color, flip); v += 4;
            addTrackFace(v, &vM, &v12, v2, &v23, ctrltexTemplate[face->texture].tex4[3], color, flip); v += 4;
        }
        else
        {
            addTrackFace(v, v0, &v01, &vM, &v30, ctrltexTemplate[face->texture].tex4[1], color, flip); v += 4;
            addTrackFace(v, &v01, v1, &v12, &vM, ctrltexTemplate[face->texture].tex4[0], color, flip); v += 4;
            addTrackFace(v, &v30, &vM, &v23, v3, ctrltexTemplate[face->texture].tex4[3], color, flip); v += 4;
            addTrackFace(v, &vM, &v12, v2, &v23, ctrltexTemplate[face->texture].tex4[2], color, flip); v += 4;
        }
    }
}

void drawSection16(const TrackSection* section)
{
    const Face* face = gTrack.faces + section->faceStart;
    const vec3i* vertices = gTrack.vertices;
    const vec3i *v0, *v1, *v2, *v3;
    vec3i v01, v12, v23, v30, vM;
    vec3i v001, v011, v112, v122, v223, v233, v330, v300, vM01, vM12, vM23, vM30, vM0, vM1, vM2, vM3;
    int32 i;
    int32 flip;
    uint32 color;

    V3D_TRACK* v = GU_ALLOC(V3D_TRACK, section->faceCount * 4 * 16);

    for (i = 0; i < section->faceCount; i++, face++)
    {
        flip = (face->flags & FLIP_TEXTURE);
        color = face->color;

        v0 = vertices + face->vertex[0];
        v1 = vertices + face->vertex[1];
        v2 = vertices + face->vertex[2];
        v3 = vertices + face->vertex[3];

        AVG(v01, v0, v1);
        AVG(v12, v1, v2);
        AVG(v23, v2, v3);
        AVG(v30, v3, v0);
        AVG(vM, &v01, &v23);

        AVG(v001, v0, &v01);
        AVG(v011, &v01, v1);
        AVG(v112, v1, &v12);
        AVG(v122, &v12, v2);
        AVG(v223, v2, &v23);
        AVG(v233, &v23, v3);
        AVG(v330, v3, &v30);
        AVG(v300, &v30, v0);
        AVG(vM01, &vM, &v01);
        AVG(vM12, &vM, &v12);
        AVG(vM23, &vM, &v23);
        AVG(vM30, &vM, &v30);
        AVG(vM0, &vM01, &v300);
        AVG(vM1, &vM01, &v112);
        AVG(vM2, &vM23, &v330);
        AVG(vM3, &vM23, &v122);

        if (flip)
        {
            addTrackFace(v, v0, &v001, &vM0, &v300,   ctrltexTemplate[face->texture].tex16[0], color, flip); v += 4;
            addTrackFace(v, &v001, &v01, &vM01, &vM0, ctrltexTemplate[face->texture].tex16[1], color, flip); v += 4;
            addTrackFace(v, &v01, &v011, &vM1, &vM01, ctrltexTemplate[face->texture].tex16[2], color, flip); v += 4;
            addTrackFace(v, &v011, v1, &v112, &vM1,   ctrltexTemplate[face->texture].tex16[3], color, flip); v += 4;

            addTrackFace(v, &v300, &vM0, &vM30, &v30, ctrltexTemplate[face->texture].tex16[4], color, flip); v += 4;
            addTrackFace(v, &vM0, &vM01, &vM, &vM30,  ctrltexTemplate[face->texture].tex16[5], color, flip); v += 4;
            addTrackFace(v, &vM01, &vM1, &vM12, &vM,  ctrltexTemplate[face->texture].tex16[6], color, flip); v += 4;
            addTrackFace(v, &vM1, &v112, &v12, &vM12, ctrltexTemplate[face->texture].tex16[7], color, flip); v += 4;

            addTrackFace(v, &v30, &vM30, &vM2, &v330, ctrltexTemplate[face->texture].tex16[8], color, flip); v += 4;
            addTrackFace(v, &vM30, &vM, &vM23, &vM2,  ctrltexTemplate[face->texture].tex16[9], color, flip); v += 4;
            addTrackFace(v, &vM, &vM12, &vM3, &vM23,  ctrltexTemplate[face->texture].tex16[10], color, flip); v += 4;
            addTrackFace(v, &vM12, &v12, &v122, &vM3, ctrltexTemplate[face->texture].tex16[11], color, flip); v += 4;

            addTrackFace(v, &v330, &vM2, &v233, v3,   ctrltexTemplate[face->texture].tex16[12], color, flip); v += 4;
            addTrackFace(v, &vM2, &vM23, &v23, &v233, ctrltexTemplate[face->texture].tex16[13], color, flip); v += 4;
            addTrackFace(v, &vM23, &vM3, &v223, &v23, ctrltexTemplate[face->texture].tex16[14], color, flip); v += 4;
            addTrackFace(v, &vM3, &v122, v2, &v223,   ctrltexTemplate[face->texture].tex16[15], color, flip); v += 4;
        }
        else
        {
            addTrackFace(v, v0, &v001, &vM0, &v300,   ctrltexTemplate[face->texture].tex16[3], color, flip); v += 4;
            addTrackFace(v, &v001, &v01, &vM01, &vM0, ctrltexTemplate[face->texture].tex16[2], color, flip); v += 4;
            addTrackFace(v, &v01, &v011, &vM1, &vM01, ctrltexTemplate[face->texture].tex16[1], color, flip); v += 4;
            addTrackFace(v, &v011, v1, &v112, &vM1,   ctrltexTemplate[face->texture].tex16[0], color, flip); v += 4;

            addTrackFace(v, &v300, &vM0, &vM30, &v30, ctrltexTemplate[face->texture].tex16[7], color, flip); v += 4;
            addTrackFace(v, &vM0, &vM01, &vM, &vM30,  ctrltexTemplate[face->texture].tex16[6], color, flip); v += 4;
            addTrackFace(v, &vM01, &vM1, &vM12, &vM,  ctrltexTemplate[face->texture].tex16[5], color, flip); v += 4;
            addTrackFace(v, &vM1, &v112, &v12, &vM12, ctrltexTemplate[face->texture].tex16[4], color, flip); v += 4;

            addTrackFace(v, &v30, &vM30, &vM2, &v330, ctrltexTemplate[face->texture].tex16[11], color, flip); v += 4;
            addTrackFace(v, &vM30, &vM, &vM23, &vM2,  ctrltexTemplate[face->texture].tex16[10], color, flip); v += 4;
            addTrackFace(v, &vM, &vM12, &vM3, &vM23,  ctrltexTemplate[face->texture].tex16[9], color, flip); v += 4;
            addTrackFace(v, &vM12, &v12, &v122, &vM3, ctrltexTemplate[face->texture].tex16[8], color, flip); v += 4;

            addTrackFace(v, &v330, &vM2, &v233, v3,   ctrltexTemplate[face->texture].tex16[15], color, flip); v += 4;
            addTrackFace(v, &vM2, &vM23, &v23, &v233, ctrltexTemplate[face->texture].tex16[14], color, flip); v += 4;
            addTrackFace(v, &vM23, &vM3, &v223, &v23, ctrltexTemplate[face->texture].tex16[13], color, flip); v += 4;
            addTrackFace(v, &vM3, &v122, v2, &v223,   ctrltexTemplate[face->texture].tex16[12], color, flip); v += 4;
        }
    }
}

void renderTrack(void)
{
    int32 i, j, s;
    int32 dir;
    int16* counts;
    int16** sections;
    TrackSection* section;

    dir = TrackDirection(&gCamera);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadMatrix((ScePspFMatrix4*)&mIdentity);
    sceGumMatrixMode(GU_VIEW);

    renderSetMatrix(gMatrixPtr);

    switch (dir)
    {
        case VIEW_DIR_NORTH:
            counts = gCamera.section->northViewCount;
            sections = gCamera.section->northViewSection;
            break;
        case VIEW_DIR_SOUTH:
            counts = gCamera.section->southViewCount;
            sections = gCamera.section->southViewSection;
            break;
        case VIEW_DIR_WEST:
            counts = gCamera.section->westViewCount;
            sections = gCamera.section->westViewSection;
            break;
        default: // VIEW_DIR_EAST
            counts = gCamera.section->eastViewCount;
            sections = gCamera.section->eastViewSection;
    }

    section = gTrack.sections + sections[0][0];
    drawSection16(section->prevSection);

    i = 2;
    for (j = 0; j < counts[i]; j += 2)
    {
        section = gTrack.sections + sections[i][j];

        for (s = 0; s < sections[i][j + 1]; s++)
        {
            drawSection1(section);
            section = section->nextSection;
        }
    }

    i = 1;
    for (j = 0; j < counts[i]; j += 2)
    {
        section = gTrack.sections + sections[i][j];

        for (s = 0; s < sections[i][j + 1]; s++)
        {
            drawSection4(section);
            section = section->nextSection;
        }
    }

    i = 0;
    for (j = 0; j < counts[i]; j += 2)
    {
        section = gTrack.sections + sections[i][j];

        for (s = 0; s < sections[i][j + 1]; s++)
        {
            drawSection16(section);
            section = section->nextSection;
        }
    }

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadMatrix((ScePspFMatrix4*)&mScale);
    sceGumMatrixMode(GU_VIEW);
}

int32 getEffectsCount(const Effect* fx)
{
    int32 count = 0;
    while (fx)
    {
        count++;
        fx = fx->next;
    }
    return count;
}

void renderEffects(const Effect* fx)
{
    int32 x, y, z, rx, ry, rz, ux, uy, uz, size;
    V3D_OBJ* v = GU_ALLOC(V3D_OBJ, getEffectsCount(fx) * 2);

    sceGuDisable(GU_ALPHA_TEST);
    renderSetMatrix(gMatrixPtr);
    renderDepthWrite(0);
    renderSetBlend(BLEND_ADD);

    while (fx)
    {
        x = fx->pos.x - gCameraViewPos.x;
        y = fx->pos.y - gCameraViewPos.y;
        z = fx->pos.z - gCameraViewPos.z;
        size = fx->size * 8;

        rx = gMatrixPtr->e00 * size >> FIXED_SHIFT;
        ry = gMatrixPtr->e01 * size >> FIXED_SHIFT;
        rz = gMatrixPtr->e02 * size >> FIXED_SHIFT;

        ux = gMatrixPtr->e10 * size >> FIXED_SHIFT;
        uy = gMatrixPtr->e11 * size >> FIXED_SHIFT;
        uz = gMatrixPtr->e12 * size >> FIXED_SHIFT;

        V3D_SET(v[0], 0.0f, 0.0f, 0xFFFFFFFF, x - rx - ux, y - ry - uy, z - rz - uz);
        V3D_SET(v[1], 1.0f, 1.0f, 0xFFFFFFFF, x + rx + ux, y + ry + uy, z + rz + uz);

        renderSetTexture(gEffectTextures + fx->type);
        sceGumDrawArray(GU_SPRITES, V3D_OBJ_FMT, 2, NULL, v); // TODO batch by texture
        v += 2;

        fx = fx->next;
    }

    renderDepthWrite(1);
    renderSetBlend(BLEND_NONE);
    sceGuEnable(GU_ALPHA_TEST);
}

void renderMesh(const Object* obj)
{
    V3D_OBJ* v;
    int32 i;
    float iw, ih;
    const vec3s* vertices = obj->vertices;
    const ObjFace* face;
    const ObjSprite* sprite;

    int32 count = obj->sCount * 2;

    face = obj->faces;
    for (i = 0; i < obj->fCount; i++, face++)
    {
        count += (FACE_QUAD(face)) ? 4 : 3;
    }

    v = GU_ALLOC(V3D_OBJ, count);

    face = obj->faces;
    for (i = 0; i < obj->fCount; i++, face++)
    {
        uint32 texIndex = FACE_TEXTURE(face);

        // TODO precalc float uv's
        if (texIndex != NO_TEXTURE)
        {
            Texture* t = gTextures + FACE_TEXTURE(face);
            renderSetTexture(t);
            iw = 1.0f / t->widthPOT;
            ih = 1.0f / t->heightPOT;
        }
        else
        {
            renderSetTexture(NULL);
            iw = 0.0f;
            ih = 0.0f;
        }

        V3D_SET_FACE(0, face, vertices, v);
        V3D_SET_FACE(1, face, vertices, v);
        V3D_SET_FACE(2, face, vertices, v);

        if (FACE_QUAD(face))
        {
            V3D_SET_FACE(3, face, vertices, v);
            sceGumDrawArray(GU_TRIANGLE_FAN, V3D_OBJ_FMT, 4, NULL, v);
            v += 4;
        }
        else
        {
            sceGumDrawArray(GU_TRIANGLES, V3D_OBJ_FMT, 3, NULL, v);
            v += 3;
        }
    }

    sprite = obj->sprites;
    for (i = 0; i < obj->sCount; i++, sprite++)
    {
        const vec3s* p = vertices + sprite->index;
        const Texture* t = gTextures + SPRITE_TEXTURE(sprite);
        int32 w = sprite->width >> 1;
        int32 wx = gCylX * w >> FIXED_SHIFT;
        int32 wz = gCylZ * w >> FIXED_SHIFT;
        int32 h0, h1;
        float iw = (float)t->width / (float)t->widthPOT;
        float ih = (float)t->height / (float)t->heightPOT;

        if (SPRITE_FLIP(sprite))
        {
            h0 = sprite->height;
            h1 = 0;
        }
        else
        {
            h0 = 0;
            h1 = sprite->height;
        }

        V3D_SET(v[0], 0.0f, 0.0f, sprite->color, p->x - wx, p->y - h0, p->z - wz);
        V3D_SET(v[1], iw, ih, sprite->color, p->x + wx, p->y + h1, p->z + wz);

        renderSetTexture(t);
        sceGumDrawArray(GU_SPRITES, V3D_OBJ_FMT, 2, NULL, v);
        v += 2;
    }
}

void renderMeshShip(const Object* obj, uint32 color)
{
    V3D_OBJ* v;
    int32 i;
    float iw, ih;
    const vec3s* vertices = obj->vertices;
    const ObjFace* face;
    uint32 cr = 32 + (0xFF & (color));
    uint32 cg = 32 + (0xFF & (color >> 8));
    uint32 cb = 32 + (0xFF & (color >> 16));
    uint32 fr, fg, fb;
    uint32 faceColor;

    int32 count = 0;

    face = obj->faces;
    for (i = 0; i < obj->fCount; i++, face++)
    {
        count += (FACE_QUAD(face)) ? 4 : 3;
    }

    v = GU_ALLOC(V3D_OBJ, count);

    face = obj->faces;
    for (i = 0; i < obj->fCount; i++, face++)
    {
        uint32 texIndex = FACE_TEXTURE(face);

        // TODO precalc float uv's
        if (texIndex != NO_TEXTURE)
        {
            Texture* t = gTextures + FACE_TEXTURE(face);
            renderSetTexture(t);
            iw = 1.0f / t->widthPOT;
            ih = 1.0f / t->heightPOT;
        }
        else
        {
            renderSetTexture(NULL);
            iw = 0.0f;
            ih = 0.0f;
        }

        if (face->flags & FACE_FLAG_ENGINE) // triangles only
        {
            faceColor = 0xFFFFFFFF;

            VC3D_SET_FACE(0, faceColor, face, vertices, v);
            VC3D_SET_FACE(1, faceColor, face, vertices, v);
            VC3D_SET_FACE(2, faceColor, face, vertices, v);

            renderSetBlend(BLEND_ADD);
            sceGumDrawArray(GU_TRIANGLES, V3D_OBJ_FMT, 3, NULL, v);
            renderSetBlend(BLEND_NONE);
            v += 3;
        }
        else
        {
            // ships use monochrome flat vertex colors
            faceColor = (0xFF & (face->color[0])) << 1;
            fr = (faceColor * cr) >> 8;
            fg = (faceColor * cg) >> 8;
            fb = (faceColor * cb) >> 8;
            if (fr > 255) fr = 255;
            if (fg > 255) fg = 255;
            if (fb > 255) fb = 255;
            faceColor = fr | (fg << 8) | (fb << 16) | 0xFF000000;

            VC3D_SET_FACE(0, faceColor, face, vertices, v);
            VC3D_SET_FACE(1, faceColor, face, vertices, v);
            VC3D_SET_FACE(2, faceColor, face, vertices, v);

            if (FACE_QUAD(face))
            {
                VC3D_SET_FACE(3, faceColor, face, vertices, v);
                sceGumDrawArray(GU_TRIANGLE_FAN, V3D_OBJ_FMT, 4, NULL, v);
                v += 4;
            }
            else
            {
                sceGumDrawArray(GU_TRIANGLES, V3D_OBJ_FMT, 3, NULL, v);
                v += 3;
            }
        }
    }
}

void renderObject(const Object* object)
{
    renderSetMatrix(gMatrixPtr);
    renderMesh(object);
}

void renderShip(const Object* object, uint32 color)
{
    sceGuDisable(GU_ALPHA_TEST);
    renderSetMatrix(gMatrixPtr);
    renderMeshShip(object, color);
    sceGuEnable(GU_ALPHA_TEST);
}

void renderShield(const Object* object, int32 counter)
{
    sceGuDisable(GU_ALPHA_TEST);
    renderSetMatrix(gMatrixPtr);
    renderSetBlend(BLEND_ADD);
    renderMesh(object);
    renderSetBlend(BLEND_NONE);
    sceGuEnable(GU_ALPHA_TEST);
}

void renderSky(const Object* object)
{
    sceGuDisable(GU_ALPHA_TEST);
    renderSetMatrix(gMatrixPtr);
    renderDepthTest(0);
    renderMesh(object);
    renderDepthTest(1);
    sceGuEnable(GU_ALPHA_TEST);
}

void renderTargetIcon(Texture* tex)
{
    VT2D* v;
    float px, py, pz;
    int32 ix, iy, size;
    Matrix* m = gMatrixPtr;

    pz = mProj.e[0][2] * m->e03 + mProj.e[1][2] * m->e13 + mProj.e[2][2] * m->e23 + mProj.e[3][2];

    if (pz <= Z_NEAR)
        return;

    px = mProj.e[0][0] * m->e03 + mProj.e[1][0] * m->e13 + mProj.e[2][0] * m->e23 + mProj.e[3][0];
    py = mProj.e[0][1] * m->e03 + mProj.e[1][1] * m->e13 + mProj.e[2][1] * m->e23 + mProj.e[3][1];

    px = (px / pz * 0.5f + 0.5f) * SCR_WIDTH;
    py = (0.5f - py / pz * 0.5f) * SCR_HEIGHT;

    ix = (int16)px;
    iy = (int16)py;
    size = tex->width >> 1;

    v = GU_ALLOC(VT2D, 2);
    VT2D_SET(v[0], 0, 0, ix - size, iy - size);
    VT2D_SET(v[1], tex->width, tex->height, ix + size, iy + size);

    sceGuDisable(GU_ALPHA_TEST);
    renderSetBlend(BLEND_ADD);
    renderDepthTest(0);
    renderSetTexture(tex);
    sceGumDrawArray(GU_SPRITES, VT2D_FMT, 2, NULL, v);
    renderDepthTest(1);
    renderSetBlend(BLEND_NONE);
    sceGuEnable(GU_ALPHA_TEST);
}

void renderSetMode(int renderMode)
{
    //mModel.e[0][0] = mModel.e[1][1] = mModel.e[2][2] = 32767.0f; // TODO GU_VERTEX_16BIT

    if (renderMode == RENDER_MODE_2D)
    {
        sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
        sceGuTexFilter(GU_NEAREST, GU_NEAREST);

        renderDepthTest(0);
        renderSetBlend(BLEND_NONE);
    }
    else
    {
        sceGuTexFunc(GU_TFX_MODULATE2X, GU_TCC_RGBA);
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);

        renderDepthTest(1);
        renderSetBlend(BLEND_NONE);

        matrixSetViewMenu();
        matrixPerspective(&mProj, gCameraFOV / 4096.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, -xOffset / 160.0f, yOffset / 120.f, Z_NEAR, Z_FAR);

        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadMatrix((ScePspFMatrix4*)&mProj);
        sceGumMatrixMode(GU_VIEW);
    }
}

Texture* renderCreateTexture(uint8* ptr)
{
    int32 i, rowBytes, rowPitch;
    uint8* data;
    Texture* tex;
    int32 type;
    Tim* tim = (Tim*)ptr;

    tex = gTexturePtr++;
    tex->addr = pEDRAM;
    tex->type = TimType(tim);
    
    if (tex->type == TIM_4)
    {
        Tim4* t = &tim->tim4;
        data = (uint8*)(t + 1);

        tex->width = t->textureW * 4;
        tex->height = t->textureH;
        tex->widthPOT = nextPow2(tex->width);
        tex->heightPOT = nextPow2(tex->height);

        if (tex->widthPOT < 32)
        {
            tex->widthPOT = 32;
        }

        rowBytes = tex->width / 2;
        rowPitch = tex->widthPOT / 2;

        // TODO pre-process
        for (i = 0; i < 16; i++)
        {
            if (t->clut[i] != 0)
            {
                t->clut[i] |= (1 << 15);
            }
        }

        memcpy(pEDRAM, t->clut, sizeof(t->clut)); 
        pEDRAM += sizeof(t->clut);
    }
    else if (tex->type == TIM_8)
    {
        Tim8* t = &tim->tim8;
        data = (uint8*)(t + 1);

        tex->width = t->textureW * 2;
        tex->height = t->textureH;
        tex->widthPOT = nextPow2(tex->width);
        tex->heightPOT = nextPow2(tex->height);

        if (tex->widthPOT < 16)
        {
            tex->widthPOT = 16;
        }

        rowBytes = tex->width;
        rowPitch = tex->widthPOT;

        // TODO pre-process
        for (i = 0; i < 256; i++)
        {
            if (t->clut[i] != 0)
            {
                t->clut[i] |= (1 << 15);
            }
        }

        memcpy(pEDRAM, t->clut, sizeof(t->clut)); 
        pEDRAM += sizeof(t->clut);
    }
    else // tex->type == TIM_16
    {
        Tim16* t = &tim->tim16;
        data = (uint8*)(t + 1);

        tex->width = t->textureW;
        tex->height = t->textureH;
        tex->widthPOT = nextPow2(tex->width);
        tex->heightPOT = nextPow2(tex->height);

        if (tex->widthPOT < 8)
        {
            tex->widthPOT = 8;
        }
        
        rowBytes = tex->width * 2;
        rowPitch = tex->widthPOT * 2;

        // TODO pre-process
        for (i = 0; i < tex->width * tex->height; i++)
        {
            ((uint16*)data)[i] |= (1 << 15);
        }
    }

    if (tex->width <= 0)
    {
        tex->width = 1;
    }

    if (tex->height <= 0)
    {
        tex->height = 1;
    }

    memset(pEDRAM, 0, rowBytes * tex->height);

    if (rowBytes != rowPitch)
    {
        for (i = 0; i < tex->height; i++)
        {
            memcpy(pEDRAM, data, rowBytes);
            pEDRAM += rowPitch;
            data += rowBytes;
        }
    }
    else
    {
        memcpy(pEDRAM, data, rowBytes * tex->height);
        pEDRAM += rowBytes * tex->height;
    }

    return tex;
}

void renderDeleteTexture(Texture* texture)
{
    if (texture && texture->addr < pEDRAM)
    {
        pEDRAM = texture->addr;
    }
}
