#include <malloc.h>
#include <unistd.h>
#include <sysutil/sysutil.h>
#include <sysutil/video.h>
#include <rsx/rsx.h>
#include <rsx/gcm_sys.h>

#include "game.h"

#define DECL_ENUM(v) v,
#define DECL_STR(v)  #v,

#define SHADER_UNIFORMS(E) \
    E( uProjMatrix      ) \
    E( uViewMatrix      ) \
    E( uColor           )

#define aCoord      GCM_VERTEX_ATTRIB_POS
#define aNormal     GCM_VERTEX_ATTRIB_NORMAL
#define aColor      GCM_VERTEX_ATTRIB_COLOR0
#define aTexCoord   GCM_VERTEX_ATTRIB_TEX0

typedef enum
{
    SHADER_UNIFORMS(DECL_ENUM)
    uMAX
} UniformType;

const char* const SHADER_UNIFORM_NAME[uMAX] = { SHADER_UNIFORMS(DECL_STR) };

typedef struct
{
    rsxVertexProgram* vp;
    rsxFragmentProgram* fp;
    void* fp_ucode;
    void* vp_ucode;
    uint32 fp_offset;
    rsxProgramConst* uIndex[uMAX];
} Shader;

#define SHADER_VP(name)     name ## _vpo
#define SHADER_FP(name)     name ## _fpo

#define SHADER_IMPORT(name)\
    Shader shader_ ## name;\
    extern const u8 SHADER_VP(name)[];\
    extern const u8 SHADER_FP(name)[]

#define SHADER_INIT(name)   initShader(&shader_ ## name, (void*)SHADER_VP(name), (void*)SHADER_FP(name))

SHADER_IMPORT(gui);
SHADER_IMPORT(track);
SHADER_IMPORT(object);
SHADER_IMPORT(ship);
SHADER_IMPORT(shield);
SHADER_IMPORT(blob);

typedef struct
{
    float x, y, z;
    float s, t;
    uint32 color;
} Vertex;

#define HOST_SIZE   (1024 << 10)
#define CB_SIZE     (256 << 10)
#define VB_SIZE     ((16 << 10) * sizeof(Vertex))

gcmContextData* gContext;

Vertex* gVertices[2];
Vertex* gVerticesPtr;
uint32 gAttribOffset;

int32 xOffset;
int32 yOffset;

int32 gCylX, gCylZ;

#define MAX_TRACK_TEX   22
Texture gTrackTex[MAX_TRACK_TEX];
uint16 gTrackTexData[MAX_TRACK_TEX][128 * 128];
uint16 tempData[320 * 240];
uint8* gTextureAddr;
uint32 whiteTexOffset;

float gAspect;

gcmSurface gSurface[2];
int32 gSurfaceIndex;

#define i255    (1.0f / 255.0f)
#define i4096   (1.0f / 4096.0f)

#define Z_NEAR  128
#define Z_FAR   65535

typedef struct
{
    float e[4][4];
} mat4;

mat4 mView, mProj, mProj2D, mProj3D;

void matrixOrtho(mat4* m, float l, float r, float b, float t, float znear, float zfar)
{
    memset(m, 0, sizeof(mat4));
    m->e[0][0] = 2.0f / (r - l);
    m->e[1][1] = 2.0f / (t - b);
    m->e[2][2] = 2.0f / (znear - zfar);
    m->e[3][3] = 1.0f;
    m->e[3][0] = (l + r) / (l - r);
    m->e[3][1] = (t + b) / (b - t);
    m->e[3][2] = (znear + zfar) / (znear - zfar);
}

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

void initShader(Shader *shader, void* vp, void* fp)
{
    uint32 i, size;
    void* ucode;

    shader->vp = (rsxVertexProgram*)vp;
    shader->fp = (rsxFragmentProgram*)fp;

    rsxVertexProgramGetUCode(shader->vp, &ucode, &size);
    shader->vp_ucode = ucode;

    for (i = 0; i < uMAX; i++)
    {
        shader->uIndex[i] = rsxVertexProgramGetConst(shader->vp, SHADER_UNIFORM_NAME[i]);
    }

    rsxFragmentProgramGetUCode(shader->fp, &ucode, &size);
    shader->fp_ucode = allocVRAM(size);
    memcpy(shader->fp_ucode, ucode, size);
    rsxAddressToOffset(shader->fp_ucode, &shader->fp_offset);
}

void setShader(const Shader* shader)
{
    Vertex* v = (Vertex*)gAttribOffset;

    rsxLoadVertexProgram(gContext, shader->vp, shader->vp_ucode);
    rsxLoadFragmentProgramLocation(gContext, shader->fp, shader->fp_offset, GCM_LOCATION_RSX);

    // uniforms
    if (shader->uIndex[uProjMatrix])
    {
        rsxSetVertexProgramParameter(gContext, shader->vp, shader->uIndex[uProjMatrix], (float*)&mProj);
    }

    if (shader->uIndex[uViewMatrix])
    {
        rsxSetVertexProgramParameter(gContext, shader->vp, shader->uIndex[uViewMatrix], (float*)&mView);
    }

    // attribs
    rsxBindVertexArrayAttrib(gContext, aCoord, 0, (uint32)&v->x, sizeof(Vertex), 3, GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(gContext, aTexCoord, 0, (uint32)&v->s, sizeof(Vertex), 2, GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(gContext, aColor, 0, (uint32)&v->color, sizeof(Vertex), 4, GCM_VERTEX_DATA_TYPE_U8, GCM_LOCATION_RSX);
}

#define VERTEX_ALLOC(count) gVerticesPtr; gVerticesPtr += count
#define VERTEX_SET(v, _x, _y, _z, _s, _t, _color) v.x = _x; v.y = _y; v.z = _z; v.s = _s; v.t = _t; v.color = _color;
#define VERTEX_SET_FACE(i, face, vertices, v) VERTEX_SET(v[i], vertices[face->index[i]].x, vertices[face->index[i]].y, vertices[face->index[i]].z, face->uv[i][0] * iw, face->uv[i][1] * ih, face->color[i])

void drawPrim(int32 type, const Vertex* v)
{
    rsxDrawVertexArray(gContext, type, v - gVertices[gSurfaceIndex], gVerticesPtr - v);
}

void setFilter(int32 enable)
{
    uint32 mode = enable ? GCM_TEXTURE_LINEAR : GCM_TEXTURE_NEAREST;
    rsxTextureFilter(gContext, 0, 0, mode, mode, GCM_TEXTURE_CONVOLUTION_QUINCUNX);
}

void setTexture(const Texture* texture, int32 swizzle)
{
    gcmTexture t;

    t.format = GCM_TEXTURE_FORMAT_A1R5G5B5 | (swizzle ? GCM_TEXTURE_FORMAT_SWZ : GCM_TEXTURE_FORMAT_LIN);
    t.mipmap = 1;
    t.dimension = GCM_TEXTURE_DIMS_2D;
    t.cubemap = GCM_FALSE;
    t.remap = (
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_A << GCM_TEXTURE_REMAP_COLOR_A_SHIFT));
    t.depth = 1;
    t.location = GCM_LOCATION_RSX;
    if (texture)
    {
        t.width = texture->width;
        t.height = texture->height;
        t.pitch = texture->width * sizeof(uint16);
        t.offset = texture->offset;
    }
    else
    {
        t.width = 1;
        t.height = 1;
        t.pitch = 1 * sizeof(uint16);
        t.offset = whiteTexOffset;
    }

    rsxInvalidateTextureCache(gContext, GCM_INVALIDATE_TEXTURE);
    rsxLoadTexture(gContext, 0, &t);
}

void renderInit()
{
    videoState state;
    videoResolution resolution;
    videoConfiguration config;
    gcmConfiguration gcmConfig;

    uint32 colorOffset[2];
    uint32 depthOffset;

    void* ioAddr = (void*)memalign(1024 * 1024, HOST_SIZE);
    rsxInit(&gContext, CB_SIZE, HOST_SIZE, ioAddr);

    videoGetState(0, 0, &state);

    videoGetResolution(state.displayMode.resolution, &resolution);
    gWidth = resolution.width;
    gHeight = resolution.height;

    memset(&config, 0, sizeof(config));
    config.resolution = state.displayMode.resolution;
    config.format = VIDEO_BUFFER_FORMAT_XRGB;
    config.pitch = gWidth * 4;
    videoConfigure(0, &config, NULL, 0);

    videoGetState(0, 0, &state);
    gAspect = (state.displayMode.aspect == VIDEO_ASPECT_4_3) ? (4.0f / 3.0f) : (16.0f / 9.0f);

    gcmGetConfiguration(&gcmConfig);

    gDRAM = pDRAM = (uint8*)memalign(64, DRAM_SIZE);
    pVRAM = (uint8*)gcmConfig.localAddress;

    rsxAddressToOffset(pVRAM, &colorOffset[0]);
    pVRAM += gWidth * gHeight * 4;
    rsxAddressToOffset(pVRAM, &colorOffset[1]);
    pVRAM += gWidth * gHeight * 4;
    rsxAddressToOffset(pVRAM, &depthOffset);
    pVRAM += gWidth * gHeight * 4;

    gVertices[0] = (Vertex*)pVRAM;
    pVRAM += VB_SIZE;
    gVertices[1] = (Vertex*)pVRAM;
    pVRAM += VB_SIZE;

    SHADER_INIT(gui);
    SHADER_INIT(track);
    SHADER_INIT(object);
    SHADER_INIT(ship);
    SHADER_INIT(shield);
    SHADER_INIT(blob);

    pVRAM = (uint8*)(((uint32)pVRAM + 127) & ~127); // align
    gVRAM = pVRAM;

    gTextureAddr = pVRAM + VRAM_SIZE;

    // init whiteTex
    *(uint16*)gTextureAddr = 0xFFFF; // whiteTex
    rsxAddressToOffset(gTextureAddr, &whiteTexOffset);
    gTextureAddr += 128;

    gcmSetDisplayBuffer(0, colorOffset[0], gWidth * 4, gWidth, gHeight);
    gcmSetDisplayBuffer(1, colorOffset[1], gWidth * 4, gWidth, gHeight);

    gSurface[0].type                = GCM_SURFACE_TYPE_LINEAR;
    gSurface[0].antiAlias           = GCM_SURFACE_CENTER_1;
    gSurface[0].colorFormat         = GCM_SURFACE_X8R8G8B8;
    gSurface[0].colorTarget         = GCM_SURFACE_TARGET_0;
    gSurface[0].colorLocation[0]    = GCM_LOCATION_RSX;
    gSurface[0].colorLocation[1]    = GCM_LOCATION_RSX;
    gSurface[0].colorLocation[2]    = GCM_LOCATION_RSX;
    gSurface[0].colorLocation[3]    = GCM_LOCATION_RSX;
    gSurface[0].colorOffset[0]      = colorOffset[0];
    gSurface[0].colorOffset[1]      = 0;
    gSurface[0].colorOffset[2]      = 0;
    gSurface[0].colorOffset[3]      = 0;
    gSurface[0].colorPitch[0]       = gWidth * 4;
    gSurface[0].colorPitch[1]       = 64;
    gSurface[0].colorPitch[2]       = 64;
    gSurface[0].colorPitch[3]       = 64;
    gSurface[0].depthFormat         = GCM_SURFACE_ZETA_Z24S8;
    gSurface[0].depthLocation       = GCM_LOCATION_RSX;
    gSurface[0].depthOffset         = depthOffset;
    gSurface[0].depthPitch          = gWidth * 4;
    gSurface[0].width               = gWidth;
    gSurface[0].height              = gHeight;
    gSurface[0].x                   = 0;
    gSurface[0].y                   = 0;
    gSurface[1] = gSurface[0];
    gSurface[1].colorOffset[0]      = colorOffset[1];

    renderSetVSync(1);

    rsxSetSurface(gContext, gSurface + gSurfaceIndex);
}

void renderFree(void)
{
    gcmSetWaitFlip(gContext);
    rsxFinish(gContext, 1);
}

void renderSetVSync(int32 enable)
{
    gcmSetFlipMode(enable ? GCM_FLIP_VSYNC : GCM_FLIP_HSYNC);
}

void renderSwap(void)
{
    while (gcmGetFlipStatus())
    {
        usleep(300);
    }

    gcmResetFlipStatus();
    gcmSetFlip(gContext, gSurfaceIndex);
    gSurfaceIndex ^= 1;
    rsxFlushBuffer(gContext);
    gcmSetWaitFlip(gContext);
    rsxSetSurface(gContext, gSurface + gSurfaceIndex);
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
            rsxSetBlendFunc(gContext, GCM_ONE, GCM_ONE, GCM_ONE, GCM_ONE);
            break;
        case BLEND_MUL:
            rsxSetBlendFunc(gContext, GCM_DST_COLOR, GCM_ZERO, GCM_ONE, GCM_ZERO);
            break;
        default:
            rsxSetBlendEnable(gContext, GCM_FALSE);
            return;
    }
    rsxSetBlendEnable(gContext, GCM_TRUE);
}

void setDepthWrite(int enable)
{
    rsxSetDepthWriteEnable(gContext, enable ? GCM_TRUE : GCM_FALSE);
}

void setDepthTest(int enable)
{
    rsxSetDepthTestEnable(gContext, enable ? GCM_TRUE : GCM_FALSE);
}

void renderNextLayer()
{
    //
}

void renderClear(void)
{
    rsxClearSurface(gContext, GCM_CLEAR_M);
}

int32 renderGlyph(const Glyph* g, int32 x, int32 y, int32 size, uint32 color)
{
#ifndef USE_FONT_SCALE
    size = 16;
#endif
    const Texture* t = g->texture;

    float u0 = g->x * t->iw;
    float v0 = g->y * t->ih;
    float u1 = (g->x + g->w) * t->iw;
    float v1 = (g->y + g->h) * t->ih;

    int32 x0 = x;
    int32 y0 = y;
    int32 x1 = x + g->w * size / 16;
    int32 y1 = y + g->h * size / 16;

    Vertex* v = VERTEX_ALLOC(4);
    VERTEX_SET(v[0], x0, y0, 0, u0, v0, color);
    VERTEX_SET(v[1], x1, y0, 0, u1, v0, color);
    VERTEX_SET(v[2], x1, y1, 0, u1, v1, color);
    VERTEX_SET(v[3], x0, y1, 0, u0, v1, color);

    setTexture(t, 0);
    drawPrim(GCM_TYPE_QUADS, v);

    return g->w * size / 16;
}

void renderImage(const Texture* t, int32 x, int32 y)
{
    int32 x0 = x;
    int32 y0 = y;
    int32 x1 = x + t->width;
    int32 y1 = y + t->height;

    Vertex* v = VERTEX_ALLOC(4);
    VERTEX_SET(v[0], x0, y0, 0, 0.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[1], x1, y0, 0, 1.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[2], x1, y1, 0, 1.0f, 1.0f, 0xFFFFFFFF);
    VERTEX_SET(v[3], x0, y1, 0, 0.0f, 1.0f, 0xFFFFFFFF);

    setTexture(t, 0);
    drawPrim(GCM_TYPE_QUADS, v);
}

void renderBackground(const Texture* t)
{
    float f = (guiB - guiT) / (t->height * 2.0f);
    float x0 = 160 - t->width * f;
    float y0 = guiT;
    float x1 = 160 + t->width * f;
    float y1 = guiB;

    Vertex* v = VERTEX_ALLOC(4);
    VERTEX_SET(v[0], x0, y0, 0, 0.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[1], x1, y0, 0, 1.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[2], x1, y1, 0, 1.0f, 1.0f, 0xFFFFFFFF);
    VERTEX_SET(v[3], x0, y1, 0, 0.0f, 1.0f, 0xFFFFFFFF);

    setTexture(t, 0);
    drawPrim(GCM_TYPE_QUADS, v);
}

void renderColorQuad(const ColorQuad* q)
{
    Vertex* v = VERTEX_ALLOC(4);
    VERTEX_SET(v[0], q->x0, q->y0, 0, 0.0f, 0.0f, q->color[0]);
    VERTEX_SET(v[1], q->x1, q->y1, 0, 0.0f, 0.0f, q->color[1]);
    VERTEX_SET(v[2], q->x2, q->y2, 0, 0.0f, 0.0f, q->color[2]);
    VERTEX_SET(v[3], q->x3, q->y3, 0, 0.0f, 0.0f, q->color[3]);

    setTexture(NULL, 0);
    drawPrim(GCM_TYPE_QUADS, v);
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
    Vertex* v = VERTEX_ALLOC(4);

    uint32 color = (intensity << 24) | 0xFFFFFF;

    dx >>= SHADOW_SHIFT_N;
    dy >>= SHADOW_SHIFT_N;
    dz >>= SHADOW_SHIFT_N;

    rx >>= SHADOW_SHIFT_W;
    ry >>= SHADOW_SHIFT_W;
    rz >>= SHADOW_SHIFT_W;

    VERTEX_SET(v[0], px + dx - rx, py + dy - ry, pz + dz - rz, 0.0f, 0.0f, color);
    VERTEX_SET(v[1], px + dx + rx, py + dy + ry, pz + dz + rz, 1.0f, 0.0f, color);
    VERTEX_SET(v[2], px - dx + rx, py - dy + ry, pz - dz + rz, 1.0f, 1.0f, color);
    VERTEX_SET(v[3], px - dx - rx, py - dy - ry, pz - dz - rz, 0.0f, 1.0f, color);

    renderSetBlend(BLEND_MUL);
    setDepthWrite(0);
    setShader(&shader_blob);

    setTexture(gShadowTex, 0);
    drawPrim(GCM_TYPE_QUADS, v);

    setDepthWrite(1);
    renderSetBlend(BLEND_NONE);
}

uint32 Part1By1(uint32 n)
{
    n = (n ^ (n << 8)) & 0x00ff00ff;
    n = (n ^ (n << 4)) & 0x0f0f0f0f;
    n = (n ^ (n << 2)) & 0x33333333;
    n = (n ^ (n << 1)) & 0x55555555;
    return n;
}

uint32 Part1By2(uint32 n)
{
    n = (n ^ (n << 16)) & 0xff0000ff;
    n = (n ^ (n <<  8)) & 0x0300f00f;
    n = (n ^ (n <<  4)) & 0x030c30c3;
    n = (n ^ (n <<  2)) & 0x09249249;
    return n;
}

uint16* swizzle(uint16* src, int32 width, int32 height)
{
    int32 x, y;
    uint16* dst = tempData;

    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 128; x++)
        {
            dst[(Part1By1(y) << 1) + Part1By1(x)] = *src++;
        }
    }

    return tempData;
}

void prepareTrack(void)
{
    int32 i;

    ASSERT(ctrlnumLib <= MAX_TRACK_TEX);
    ASSERT(sizeof(gTrackTexData[0]) == 128 * 128 * sizeof(uint16));

    for (i = 0; i < ctrlnumLib; i++)
    {
        Texture* tex = gTrackTex + i;
        tex->width = tex->height = 128;
        tex->iw = tex->ih = 1.0f / 128.0f;
        tex->addr = gTextureAddr;
        gTextureAddr += sizeof(gTrackTexData[0]);

        memcpy(tex->addr, swizzle(gTrackTexData[i], 128, 128), sizeof(gTrackTexData[0]));
        rsxAddressToOffset(tex->addr, &tex->offset);
    }
}

void fillTrackTex(uint16* data)
{
    int32 id, i, j, k, x, y;

    id = (gTexturePtr - gTextures) - ctrllibraryTextures;

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

void setMatrix(const Matrix* matrix)
{
    mView.e[0][0] = matrix->e00 * i4096;
    mView.e[0][1] = matrix->e10 * i4096;
    mView.e[0][2] = matrix->e20 * i4096;
    mView.e[0][3] = 0.0f;

    mView.e[1][0] = matrix->e01 * i4096;
    mView.e[1][1] = matrix->e11 * i4096;
    mView.e[1][2] = matrix->e21 * i4096;
    mView.e[1][3] = 0.0f;

    mView.e[2][0] = matrix->e02 * i4096;
    mView.e[2][1] = matrix->e12 * i4096;
    mView.e[2][2] = matrix->e22 * i4096;
    mView.e[2][3] = 0.0f;

    mView.e[3][0] = matrix->e03;
    mView.e[3][1] = matrix->e13;
    mView.e[3][2] = matrix->e23;
    mView.e[3][3] = 1.0f;
}

void renderFrameBegin(void)
{
    float vpScale[4] = { gWidth * 0.5f, -gHeight * 0.5f, 0.5f, 0.0f };
    float vpOffset[4] = { gWidth * 0.5f, gHeight * 0.5f, 0.5f, 0.0f };

    // get fixed right vector for sprites
    rsincos(gCamera.hdg, gCylZ, gCylX);

    rsxSetColorMask(gContext, GCM_COLOR_MASK_B | GCM_COLOR_MASK_G | GCM_COLOR_MASK_R | GCM_COLOR_MASK_A);
    rsxSetColorMaskMrt(gContext, 0);
    rsxSetClearColor(gContext, 0);
    rsxSetDepthFunc(gContext, GCM_LEQUAL);
    rsxSetShadeModel(gContext, GCM_SHADE_MODEL_SMOOTH);

    rsxSetBlendEquation(gContext, GCM_FUNC_ADD, GCM_FUNC_ADD);

    rsxSetCullFaceEnable(gContext, GCM_TRUE);
    rsxSetFrontFace(gContext, GCM_FRONTFACE_CW);
    rsxSetCullFace(gContext, GCM_CULL_BACK);
    rsxSetZMinMaxControl(gContext, GCM_TRUE, GCM_TRUE, GCM_FALSE);

    rsxSetViewport(gContext, 0, 0, gWidth, gHeight, 0.0f, 1.0f, vpScale, vpOffset);
    rsxSetScissor(gContext, 0, 0, gWidth, gHeight);

    gVerticesPtr = gVertices[gSurfaceIndex];
    rsxAddressToOffset(gVerticesPtr, &gAttribOffset);

    rsxTextureControl(gContext, 0, GCM_TRUE, 0, 0, GCM_TEXTURE_MAX_ANISO_1);
    rsxTextureWrapMode(gContext, 0, GCM_TEXTURE_CLAMP_TO_EDGE, GCM_TEXTURE_CLAMP_TO_EDGE, GCM_TEXTURE_CLAMP_TO_EDGE, 0, GCM_TEXTURE_ZFUNC_LESS, 0);

    renderSetMode(RENDER_MODE_2D);
}

void renderFrameEnd(void)
{
}

void drawSection(const TrackSection* section)
{
    const Face* face = gTrack.faces + section->faceStart;
    const Face* last = face + section->faceCount;
    const vec3i* vertices = gTrack.vertices;
    float u0, u1;
    Vertex* v;

    while (face < last)
    {
        int32 flip = (face->flags & FLIP_TEXTURE);

        const vec3i* v0 = vertices + face->vertex[0];
        const vec3i* v1 = vertices + face->vertex[1];
        const vec3i* v2 = vertices + face->vertex[2];
        const vec3i* v3 = vertices + face->vertex[3];

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

        setTexture(gTrackTex + face->texture, 1);

        v = VERTEX_ALLOC(4);
        VERTEX_SET(v[0], v3->x, v3->y, v3->z, u0, 1.0f, face->color);
        VERTEX_SET(v[1], v2->x, v2->y, v2->z, u1, 1.0f, face->color);
        VERTEX_SET(v[2], v1->x, v1->y, v1->z, u1, 0.0f, face->color);
        VERTEX_SET(v[3], v0->x, v0->y, v0->z, u0, 0.0f, face->color);

        drawPrim(GCM_TYPE_QUADS, v);
        face++;
    }
}

void renderTrack(void)
{
    int32 i, j, s;
    int16* counts;
    int16** sections;
    TrackSection* section;
    int32 dir = TrackDirection(&gCamera);

    setMatrix(gMatrixPtr);
    setShader(&shader_track);

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
    drawSection(section->prevSection);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < counts[i]; j += 2)
        {
            section = gTrack.sections + sections[i][j];

            for (s = 0; s < sections[i][j + 1]; s++)
            {
                drawSection(section);
                section = section->nextSection;
            }
        }
    }
}

void renderEffects(const Effect* fx)
{
    int32 x, y, z, rx, ry, rz, ux, uy, uz, size;
    Vertex* v;

    renderSetBlend(BLEND_ADD);
    setDepthWrite(0);
    setMatrix(gMatrixPtr);
    setShader(&shader_object);

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

        setTexture(gEffectTextures + fx->type, 0);

        v = VERTEX_ALLOC(4);
        VERTEX_SET(v[0], x - rx - ux, y - ry - uy, z - rz - uz, 0.0f, 0.0f, 0xFF808080);
        VERTEX_SET(v[1], x + rx - ux, y - ry - uy, z + rz - uz, 1.0f, 0.0f, 0xFF808080);
        VERTEX_SET(v[2], x + rx + ux, y + ry + uy, z + rz + uz, 1.0f, 1.0f, 0xFF808080);
        VERTEX_SET(v[3], x - rx + ux, y + ry + uy, z - rz + uz, 0.0f, 1.0f, 0xFF808080);

        drawPrim(GCM_TYPE_QUADS, v);

        fx = fx->next;
    }

    setDepthWrite(1);
    renderSetBlend(BLEND_NONE);
}

void drawMesh(const Object* obj)
{
    Vertex* v;
    float iw, ih;
    const vec3s* vertices = obj->vertices;
    const ObjFace* face = obj->faces;
    const ObjFace* fLast = face + obj->fCount;
    const ObjSprite* sprite = obj->sprites;
    const ObjSprite* sLast = sprite + obj->sCount;

    while (face < fLast)
    {
        uint32 texIndex = FACE_TEXTURE(face);

        // TODO precalc float uv's
        if (texIndex != NO_TEXTURE)
        {
            Texture* t = gTextures + FACE_TEXTURE(face);
            setTexture(t, 0);
            iw = t->iw;
            ih = t->ih;
        }
        else
        {
            setTexture(NULL, 0);
            iw = 0.0f;
            ih = 0.0f;
        }

        if (FACE_QUAD(face))
        {
            v = VERTEX_ALLOC(4);
            VERTEX_SET_FACE(0, face, vertices, v);
            VERTEX_SET_FACE(1, face, vertices, v);
            VERTEX_SET_FACE(2, face, vertices, v);
            VERTEX_SET_FACE(3, face, vertices, v);
            drawPrim(GCM_TYPE_QUADS, v);
        }
        else
        {
            v = VERTEX_ALLOC(3);
            VERTEX_SET_FACE(0, face, vertices, v);
            VERTEX_SET_FACE(1, face, vertices, v);
            VERTEX_SET_FACE(2, face, vertices, v);
            drawPrim(GCM_TYPE_TRIANGLES, v);
        }

        face++;
    }

    while (sprite < sLast)
    {
        const vec3s* p = vertices + sprite->index;
        const Texture* t = gTextures + SPRITE_TEXTURE(sprite);
        int32 w = sprite->width >> 1;
        int32 wx = gCylX * w >> FIXED_SHIFT;
        int32 wz = gCylZ * w >> FIXED_SHIFT;
        int32 h0, h1;

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

        setTexture(t, 0);

        v = VERTEX_ALLOC(4);
        VERTEX_SET(v[0], p->x - wx, p->y - h0, p->z - wz, 0.0f, 0.0f, sprite->color);
        VERTEX_SET(v[1], p->x + wx, p->y - h0, p->z + wz, 1.0f, 0.0f, sprite->color);
        VERTEX_SET(v[2], p->x + wx, p->y + h1, p->z + wz, 1.0f, 1.0f, sprite->color);
        VERTEX_SET(v[3], p->x - wx, p->y + h1, p->z - wz, 0.0f, 1.0f, sprite->color);

        drawPrim(GCM_TYPE_QUADS, v);

        sprite++;
    }
}

void drawMeshShip(const Object* obj, uint32 color)
{
    Vertex* v;
    float iw, ih;
    const vec3s* vertices = obj->vertices;
    const ObjFace* face = obj->faces;
    const ObjFace* fLast = face + obj->fCount;
    float colorParam[4];

// opaque
    colorParam[0] = i255 * (0xFF & (color));
    colorParam[1] = i255 * (0xFF & (color >> 8));
    colorParam[2] = i255 * (0xFF & (color >> 16));
    colorParam[3] = 1.0f;
    if (shader_ship.uIndex[uColor])
    {
        rsxSetVertexProgramParameter(gContext, shader_ship.vp, shader_ship.uIndex[uColor], colorParam);
    }

    while (face < fLast)
    {
        if ((face->flags & FACE_FLAG_ENGINE) == 0)
        {
            uint32 texIndex = FACE_TEXTURE(face);

            // TODO precalc float uv's
            if (texIndex != NO_TEXTURE)
            {
                Texture* t = gTextures + FACE_TEXTURE(face);
                setTexture(t, 0);
                iw = t->iw;
                ih = t->ih;
            }
            else
            {
                setTexture(NULL, 0);
                iw = 0.0f;
                ih = 0.0f;
            }

            if (FACE_QUAD(face))
            {
                v = VERTEX_ALLOC(4);
                VERTEX_SET_FACE(0, face, vertices, v);
                VERTEX_SET_FACE(1, face, vertices, v);
                VERTEX_SET_FACE(2, face, vertices, v);
                VERTEX_SET_FACE(3, face, vertices, v);
                drawPrim(GCM_TYPE_QUADS, v);
            }
            else
            {
                v = VERTEX_ALLOC(3);
                VERTEX_SET_FACE(0, face, vertices, v);
                VERTEX_SET_FACE(1, face, vertices, v);
                VERTEX_SET_FACE(2, face, vertices, v);
                drawPrim(GCM_TYPE_TRIANGLES, v);
            }
        }

        face++;
    }

// semi-transparent
    colorParam[0] = colorParam[1] = colorParam[2] = 0.125f;
    if (shader_ship.uIndex[uColor])
    {
        rsxSetVertexProgramParameter(gContext, shader_ship.vp, shader_ship.uIndex[uColor], colorParam);
    }
    renderSetBlend(BLEND_ADD);

    face = obj->faces;
    while (face < fLast)
    {
        if ((face->flags & FACE_FLAG_ENGINE) != 0)
        {
            uint32 texIndex = FACE_TEXTURE(face);

            // TODO precalc float uv's
            if (texIndex != NO_TEXTURE)
            {
                Texture* t = gTextures + FACE_TEXTURE(face);
                setTexture(t, 0);
                iw = t->iw;
                ih = t->ih;
            }
            else
            {
                setTexture(NULL, 0);
                iw = 0.0f;
                ih = 0.0f;
            }

            ASSERT(FACE_QUAD(face) == 0); // triangles only

            v = VERTEX_ALLOC(3);
            VERTEX_SET_FACE(0, face, vertices, v);
            VERTEX_SET_FACE(1, face, vertices, v);
            VERTEX_SET_FACE(2, face, vertices, v);
            drawPrim(GCM_TYPE_TRIANGLES, v);
        }

        face++;
    }

    renderSetBlend(BLEND_NONE);
}

void renderObject(const Object* object)
{
    setMatrix(gMatrixPtr);
    setShader(&shader_object);
    drawMesh(object);
}

void renderShip(const Object* object, uint32 color)
{
    setMatrix(gMatrixPtr);
    setShader(&shader_ship);
    drawMeshShip(object, color);
}

void renderShield(const Object* object, int32 counter)
{
    float colorParam[4] = { i255 * (0xFF & (counter)), 0, 0, 0 };

    setMatrix(gMatrixPtr);
    renderSetBlend(BLEND_ADD);
    setShader(&shader_shield);
    if (shader_shield.uIndex[uColor])
    {
        rsxSetVertexProgramParameter(gContext, shader_shield.vp, shader_shield.uIndex[uColor], colorParam);
    }

    drawMesh(object);

    renderSetBlend(BLEND_NONE);
}

void renderSky(const Object* object)
{
    setDepthTest(0);
    setMatrix(gMatrixPtr);
    setShader(&shader_object);
    drawMesh(object);
    setDepthTest(1);
}

void renderTargetIcon(Texture* tex)
{
    Vertex* v;
    float px, py, pz, size;
    Matrix* m = gMatrixPtr;

    pz = mProj3D.e[0][2] * m->e03 + mProj3D.e[1][2] * m->e13 + mProj3D.e[2][2] * m->e23 + mProj3D.e[3][2];

    if (pz <= Z_NEAR)
        return;

    px = mProj3D.e[0][0] * m->e03 + mProj3D.e[1][0] * m->e13 + mProj3D.e[2][0] * m->e23 + mProj3D.e[3][0];
    py = mProj3D.e[0][1] * m->e03 + mProj3D.e[1][1] * m->e13 + mProj3D.e[2][1] * m->e23 + mProj3D.e[3][1];

    px = guiL + (px / pz * 0.5f + 0.5f) * (guiR - guiL);
    py = guiT + (0.5f - py / pz * 0.5f) * (guiB - guiT);

    size = tex->width >> 1;

    renderSetBlend(BLEND_ADD);

    setTexture(tex, 0);
    setFilter(1);

    v = VERTEX_ALLOC(4);
    VERTEX_SET(v[0], px - size, py - size, 0, 0.0f, 1.0f, 0xFF808080);
    VERTEX_SET(v[1], px + size, py - size, 0, 1.0f, 1.0f, 0xFF808080);
    VERTEX_SET(v[2], px + size, py + size, 0, 1.0f, 0.0f, 0xFF808080);
    VERTEX_SET(v[3], px - size, py + size, 0, 0.0f, 0.0f, 0xFF808080);
    drawPrim(GCM_TYPE_QUADS, v);

    setFilter(0);
    renderSetBlend(BLEND_NONE);
}

void renderSetMode(int renderMode)
{
    if (renderMode == RENDER_MODE_2D)
    {
        setFilter(0);

        matrixOrtho(&mProj, guiL, guiR, guiB, guiT, 0, 1);
        mProj2D = mProj;

        setDepthTest(0);
        renderSetBlend(BLEND_NONE);

        setShader(&shader_gui);
    }
    else
    {
        setFilter(1);

        matrixSetViewMenu();
        matrixPerspective(&mProj, gCameraFOV / 4096.0f, gAspect, -xOffset / 160.0f, yOffset / 120.f, Z_NEAR, Z_FAR);
        mProj3D = mProj;

        setDepthTest(1);
        renderSetBlend(BLEND_NONE);
    }
}

uint16 RGB5551(uint16 c)
{
    char r, g, b;
    c = swap16(c);
    r = (c) & 0x1F;
    g = (c >> 5) & 0x1F;
    b = (c >> 10) & 0x1F;
    c = (r << 10) | (g << 5) | (b);

    if (c != 0)
    {
        c |= (1 << 15);
    }

    return c;
}

uint16 RGB555(uint16 c)
{
    char r, g, b;
    c = swap16(c);
    r = (c) & 0x1F;
    g = (c >> 5) & 0x1F;
    b = (c >> 10) & 0x1F;
    c = (r << 10) | (g << 5) | (b) | (1 << 15);
    return c;
}

Texture* renderCreateTexture(uint8* ptr)
{
    Tim* tim = (Tim*)ptr;
    Texture* tex;
    int32 w, h;
    int32 i, j;

    swap32p(&tim->header.flags);

    if (TimType(tim) == TIM_4)
    {
        uint16 clut[16];
        Tim4* t = &tim->tim4;
        uint8* src = (uint8*)(t + 1);
        uint16* dst = tempData;

        swap16p(&t->textureW);
        swap16p(&t->textureH);

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
        uint16 clut[256];
        Tim8* t = &tim->tim8;
        uint8* src = (uint8*)(t + 1);
        uint16* dst = tempData;

        swap16p(&t->textureW);
        swap16p(&t->textureH);

        w = t->textureW * 2;
        h = t->textureH;

        for (i = 0; i < 256; i++)
        {
            clut[i] = RGB5551(t->clut[i]);
        }

        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                *dst++ = clut[*src++];
            }
        }
    }
    else if (TimType(tim) == TIM_16)
    {
        Tim16* t = &tim->tim16;
        uint16* src = (uint16*)(t + 1);
        uint16* dst = tempData;

        swap16p(&t->textureW);
        swap16p(&t->textureH);

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
        return NULL;
    }

    // TODO remove shitty texture from resources
    if (w < 1)
    {
        w = 1;
    }

    if (h < 1)
    {
        h = 1;
    }

    tex = gTexturePtr;
    tex->width = w;
    tex->height = h;
    tex->iw = 1.0f / w;
    tex->ih = 1.0f / h;
    tex->addr = gTextureAddr;
    rsxAddressToOffset(tex->addr, &tex->offset);

    if (HACK_LOAD_TRACK)
    {
        fillTrackTex(tempData);
    }
    else
    {
        int32 size = w * h * sizeof(uint16);
        memcpy(tex->addr, tempData, size);
        gTextureAddr += (size + 127) & ~127;
    }

    gTexturePtr++;

    return tex;
}

void renderDeleteTexture(Texture* texture)
{
    if (texture && texture->addr < gTextureAddr)
    {
        gTextureAddr = texture->addr;
    }
}
