#if defined(__WIN32__)
    #include <d3d9.h>
#elif defined(__X360__)
    #include <xtl.h>
    #include <xbox.h>
    #include <xgraphics.h>
#endif

#include "game.h"

D3DPRESENT_PARAMETERS d3dpp;
D3DDevice* gDevice;
D3DVertexDeclaration* gVertexDecl;
float gAspect;

#define i255    (1.0f / 255.0f)
#define i4096   (1.0f / 4096.0f)

#define Z_NEAR  64
#define Z_FAR   65535

typedef struct
{
    float x, y, z;
    float s, t;
    uint32 color;
} Vertex;

typedef struct
{
    float e[4][4];
} mat4;

mat4 mView, mProj, mProj2D, mProj3D;

typedef struct
{
    D3DVertexShader* vs;
    D3DPixelShader* ps;
} Shader;

#include "shaders/out/shaders.h"

#define SHADER_VS(name)     name ## _vs
#define SHADER_PS(name)     name ## _ps
#define SHADER_IMPORT(name) Shader shader_ ## name;
#define SHADER_INIT(name)   initShader(&shader_ ## name, SHADER_VS(name), SHADER_PS(name))

SHADER_IMPORT(gui);
SHADER_IMPORT(track);
SHADER_IMPORT(object);
SHADER_IMPORT(ship);
SHADER_IMPORT(shield);
SHADER_IMPORT(blob);

uint16 tempData[320 * 240];

int32 xOffset;
int32 yOffset;

int32 gCylX, gCylZ;

D3DTexture* whiteTex;

const Texture* gCurTexture;

#define MAX_TRACK_TEX   22
uint16 gTrackTexData[MAX_TRACK_TEX][128 * 128];

Texture gTrackTexArray;
extern Texture* gEffectTextures;

void matrixIdentity(mat4* m)
{
    memset(m, 0, sizeof(mat4));
    m->e[0][0] = m->e[1][1] = m->e[2][2] = m->e[3][3] = 1.0f;
}

void matrixOrtho(mat4* m, float l, float r, float b, float t, float znear, float zfar)
{
    memset(m, 0, sizeof(mat4));
    m->e[0][0] = 2.0f / (r - l);
    m->e[1][1] = 2.0f / (t - b);
    m->e[2][2] = 2.0f / (znear - zfar);
    m->e[3][3] = 1.0f;
    m->e[3][0] = (l + r) / (l - r);
    m->e[3][1] = (t + b) / (b - t);
    m->e[3][2] = znear / (znear - zfar);
}

void matrixPerspective(mat4* m, float fov, float aspect, float xoff, float yoff, float znear, float zfar)
{
    float y = tanf(fov * 0.5f * DEG2RAD) * znear;
    float x = y;

    x = y * aspect;

    memset(m, 0, sizeof(mat4));
    m->e[0][0] = znear / x;
    m->e[1][1] = znear / y;
    m->e[2][2] = zfar / (znear - zfar);
    m->e[3][3] = 0.0f;
    m->e[2][0] = xoff;
    m->e[2][1] = yoff;
    m->e[2][3] = -1.0f;
    m->e[3][2] = znear * (zfar / (znear - zfar));

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

void initShader(Shader *shader, const DWORD* vs, const DWORD* ps)
{
    IDirect3DDevice9_CreateVertexShader(gDevice, vs, &shader->vs);
    IDirect3DDevice9_CreatePixelShader(gDevice, ps, &shader->ps);
}

void setShader(const Shader* shader, float* color)
{
    IDirect3DDevice9_SetVertexShader(gDevice, shader->vs);
    IDirect3DDevice9_SetPixelShader(gDevice, shader->ps);

    IDirect3DDevice9_SetVertexShaderConstantF(gDevice, 0, (FLOAT*)&mProj, 4);
    IDirect3DDevice9_SetVertexShaderConstantF(gDevice, 4, (FLOAT*)&mView, 4);
    if (color)
    {
        IDirect3DDevice9_SetVertexShaderConstantF(gDevice, 8, (FLOAT*)color, 1);
    }
}

#define DRAW_BEGIN(v,t,c)   IDirect3DDevice9_BeginVertices(gDevice, t, c, sizeof(*v), (void**)&v)
#define DRAW_END()          IDirect3DDevice9_EndVertices(gDevice)
#define VERTEX_SET(v, _x, _y, _z, _s, _t, _color) v.x = (float)_x; v.y = (float)_y; v.z = (float)_z; v.s = (float)_s; v.t = (float)_t; v.color = _color
#define VERTEX_SET_FACE(i, face, vertices, v) VERTEX_SET(v[i], vertices[face->index[i]].x, vertices[face->index[i]].y, vertices[face->index[i]].z, face->uv[i][0] * iw, face->uv[i][1] * ih, face->color[i])

void setFilter(int32 enable)
{
    IDirect3DDevice9_SetSamplerState(gDevice, 0, D3DSAMP_MINFILTER, enable ? D3DTEXF_LINEAR : D3DTEXF_POINT);
    IDirect3DDevice9_SetSamplerState(gDevice, 0, D3DSAMP_MAGFILTER, enable ? D3DTEXF_LINEAR : D3DTEXF_POINT);
    IDirect3DDevice9_SetSamplerState(gDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    IDirect3DDevice9_SetSamplerState(gDevice, 0, D3DSAMP_MAXANISOTROPY, 1);

    IDirect3DDevice9_SetSamplerState(gDevice, 1, D3DSAMP_MINFILTER, enable ? D3DTEXF_LINEAR : D3DTEXF_POINT);
    IDirect3DDevice9_SetSamplerState(gDevice, 1, D3DSAMP_MAGFILTER, enable ? D3DTEXF_LINEAR : D3DTEXF_POINT);
    IDirect3DDevice9_SetSamplerState(gDevice, 1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    IDirect3DDevice9_SetSamplerState(gDevice, 1, D3DSAMP_MAXANISOTROPY, 1);
}

void setTexture(const Texture* texture)
{
    if (gCurTexture == texture)
        return;
    gCurTexture = texture;

    if (texture)
    {
        IDirect3DDevice9_SetTexture(gDevice, 0, (D3DBaseTexture*)texture->obj);
    }
    else
    {
        IDirect3DDevice9_SetTexture(gDevice, 0, (D3DBaseTexture*)whiteTex);
    }
}

void renderInit()
{
    Direct3D* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

#ifdef __X360__
    XVIDEO_MODE mode;
    XGetVideoMode(&mode);
    gAspect = mode.fIsWideScreen ? (16.0f / 9.0f) : (4.0f / 3.0f);
    gWidth = mode.dwDisplayWidth;
    gHeight = mode.dwDisplayHeight;
#endif

    memset(&d3dpp, 0, sizeof(d3dpp));
    // TODO 1080p support (tiling)
    d3dpp.BackBufferWidth = min(gWidth, 1280);
    d3dpp.BackBufferHeight = min(gHeight, 720);
    d3dpp.BackBufferFormat = D3DFMT_LE_X8R8G8B8;
    d3dpp.BackBufferCount = 2;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // enable vsync

#ifdef __WIN32__
    params.Windowed = TRUE;
    params.hDeviceWindow = hWnd;
#endif

    IDirect3D9_CreateDevice(pD3D, 0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &gDevice);
    ASSERT(gDevice != NULL);

    // init shaders
    SHADER_INIT(gui);
    SHADER_INIT(track);
    SHADER_INIT(object);
    SHADER_INIT(ship);
    SHADER_INIT(shield);
    SHADER_INIT(blob);

    // init vertex declaration
    {
        static const D3DVERTEXELEMENT9 VERTEX_DECL[] =
        {
            { 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, // aCoord
            { 0, 12, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // aTexCoord
            { 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 }, // aColor
            D3DDECL_END()
        };
        IDirect3DDevice9_CreateVertexDeclaration(gDevice, VERTEX_DECL, &gVertexDecl);
    }

    // pre-allocate track textures
    IDirect3DDevice9_CreateArrayTexture(gDevice, 128, 128, MAX_TRACK_TEX, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &(D3DArrayTexture*)gTrackTexArray.obj, NULL);
    IDirect3DDevice9_SetTexture(gDevice, 1, (D3DBaseTexture*)gTrackTexArray.obj); // use s1 for sampler3D

    // init white texture for non-textured primitives
    {
        D3DLOCKED_RECT rect;
        IDirect3DDevice9_CreateTexture(gDevice, 1, 1, 1, 0, D3DFMT_LIN_A1R5G5B5, D3DPOOL_DEFAULT, &whiteTex, NULL);
        D3DTexture_LockRect(whiteTex, 0, &rect, NULL, 0);
        ((uint16*)rect.pBits)[0] = 0xFFFF;
        D3DTexture_UnlockRect(whiteTex, 0);
    }

    renderSetVSync(1);
}

void renderFree(void)
{
    IDirect3DDevice9_Release(gDevice);
}

void deviceReset()
{
#ifdef __WIN32__
    IDirect3DDevice9_Reset(gDevice, &d3dpp)
#endif
}

void renderSetVSync(int32 enable)
{
#ifdef __WIN32__
    UINT interval = enable ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
    if (interval == d3dpp.PresentationInterval)
        return;
    d3dpp.PresentationInterval = interval;
    deviceReset();
#endif
}

void renderSwap(void)
{
    IDirect3DDevice9_Present(gDevice, NULL, NULL, NULL, NULL);
}

void renderResize(int width, int height)
{
#ifdef __WIN32__
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    if (gWidth == width && gHeight == height)
        return;

    gWidth = width;
    gHeight = height;
    d3dpp.BackBufferWidth = gWidth;
    d3dpp.BackBufferHeight = gHeight;
    deviceReset();
#endif
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
        case BLEND_NONE:
            IDirect3DDevice9_SetRenderState(gDevice, D3DRS_ALPHABLENDENABLE, FALSE);
            return;
        case BLEND_ADD:
            IDirect3DDevice9_SetRenderState(gDevice, D3DRS_SRCBLEND,  D3DBLEND_ONE);
            IDirect3DDevice9_SetRenderState(gDevice, D3DRS_DESTBLEND, D3DBLEND_ONE);
            break;
        case BLEND_MUL:
            IDirect3DDevice9_SetRenderState(gDevice, D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR);
            IDirect3DDevice9_SetRenderState(gDevice, D3DRS_DESTBLEND, D3DBLEND_ZERO);
            break;
    }
    IDirect3DDevice9_SetRenderState(gDevice, D3DRS_ALPHABLENDENABLE, TRUE);
}

void setDepthWrite(int enable)
{
    IDirect3DDevice9_SetRenderState(gDevice, D3DRS_ZWRITEENABLE, enable);
}

void setDepthTest(int enable)
{
    IDirect3DDevice9_SetRenderState(gDevice, D3DRS_ZENABLE, enable);
}

void renderNextLayer()
{
}

void renderClear(void)
{
    IDirect3DDevice9_Clear(gDevice, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0);
}

int32 renderGlyph(const Glyph* g, int32 x, int32 y, int32 size, uint32 color)
{
    const Texture* t = g->texture;

    float u0 = g->x * t->iw;
    float v0 = g->y * t->ih;
    float u1 = (g->x + g->w) * t->iw;
    float v1 = (g->y + g->h) * t->ih;
    int32 x0, y0, x1, y1;
    volatile Vertex* v;

#ifndef USE_FONT_SCALE
    size = 16;
#endif

    x0 = x;
    y0 = y;
    x1 = x + g->w * size / 16;
    y1 = y + g->h * size / 16;

    setTexture(t);

    DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
    VERTEX_SET(v[0], x0, y0, 0, u0, v0, color);
    VERTEX_SET(v[1], x1, y0, 0, u1, v0, color);
    VERTEX_SET(v[2], x1, y1, 0, u1, v1, color);
    VERTEX_SET(v[3], x0, y1, 0, u0, v1, color);
    DRAW_END();

    return g->w * size / 16;
}

void renderImage(const Texture* t, int32 x, int32 y)
{
    int32 x0 = x;
    int32 y0 = y;
    int32 x1 = x + t->width;
    int32 y1 = y + t->height;
    volatile Vertex* v;

    setTexture(t);

    DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
    VERTEX_SET(v[0], x0, y0, 0, 0.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[1], x1, y0, 0, 1.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[2], x1, y1, 0, 1.0f, 1.0f, 0xFFFFFFFF);
    VERTEX_SET(v[3], x0, y1, 0, 0.0f, 1.0f, 0xFFFFFFFF);
    DRAW_END();
}

void renderBackground(const Texture* t)
{
    float f = (guiB - guiT) / (t->height * 2.0f);
    float x0 = 160 - t->width * f;
    float y0 = (float)guiT;
    float x1 = 160 + t->width * f;
    float y1 = (float)guiB;
    volatile Vertex* v;

    setTexture(t);

    DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
    VERTEX_SET(v[0], x0, y0, 0, 0.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[1], x1, y0, 0, 1.0f, 0.0f, 0xFFFFFFFF);
    VERTEX_SET(v[2], x1, y1, 0, 1.0f, 1.0f, 0xFFFFFFFF);
    VERTEX_SET(v[3], x0, y1, 0, 0.0f, 1.0f, 0xFFFFFFFF);
    DRAW_END();
}

void renderColorQuad(const ColorQuad* q)
{
    volatile Vertex* v;

    setTexture(NULL);

    DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
    VERTEX_SET(v[0], q->x0, q->y0, 0, 0.0f, 0.0f, q->color[0]);
    VERTEX_SET(v[1], q->x1, q->y1, 0, 0.0f, 0.0f, q->color[1]);
    VERTEX_SET(v[2], q->x2, q->y2, 0, 0.0f, 0.0f, q->color[2]);
    VERTEX_SET(v[3], q->x3, q->y3, 0, 0.0f, 0.0f, q->color[3]);
    DRAW_END();
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
    volatile Vertex* v;

    uint32 color = (intensity << 24) | 0xFFFFFF;

    dx >>= SHADOW_SHIFT_N;
    dy >>= SHADOW_SHIFT_N;
    dz >>= SHADOW_SHIFT_N;

    rx >>= SHADOW_SHIFT_W;
    ry >>= SHADOW_SHIFT_W;
    rz >>= SHADOW_SHIFT_W;

    renderSetBlend(BLEND_MUL);
    setDepthWrite(0);
    setShader(&shader_blob, NULL);

    setTexture(gShadowTex);

    DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
    VERTEX_SET(v[0], px + dx - rx, py + dy - ry, pz + dz - rz, 0.0f, 0.0f, color);
    VERTEX_SET(v[1], px + dx + rx, py + dy + ry, pz + dz + rz, 1.0f, 0.0f, color);
    VERTEX_SET(v[2], px - dx + rx, py - dy + ry, pz - dz + rz, 1.0f, 1.0f, color);
    VERTEX_SET(v[3], px - dx - rx, py - dy - ry, pz - dz - rz, 0.0f, 1.0f, color);
    DRAW_END();

    setDepthWrite(1);
    renderSetBlend(BLEND_NONE);
}

void prepareTrack(void)
{
    int32 i;

    ASSERT(ctrlnumLib <= MAX_TRACK_TEX);
    for (i = 0; i < ctrlnumLib; i++)
    {
        D3DLOCKED_RECT rect;
        D3DArrayTexture_LockRect((D3DArrayTexture*)gTrackTexArray.obj, i, 0, &rect, NULL, 0);
        XGTileSurface(rect.pBits, 128, 128, NULL, gTrackTexData[i], rect.Pitch, NULL, 2);
        D3DArrayTexture_UnlockRect((D3DArrayTexture*)gTrackTexArray.obj, i, 0);
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

    mView.e[3][0] = (float)matrix->e03;
    mView.e[3][1] = (float)matrix->e13;
    mView.e[3][2] = (float)matrix->e23;
    mView.e[3][3] = 1.0f;
}

void renderFrameBegin(void)
{
    // get fixed right vector for sprites
    rsincos(gCamera.hdg, gCylZ, gCylX);

#ifdef __WIN32__
    IDirect3DDevice9_BeginScene(gDevice);
#endif
    IDirect3DDevice9_SetVertexDeclaration(gDevice, gVertexDecl);
    IDirect3DDevice9_SetStreamSource(gDevice, 0, NULL, 0, sizeof(Vertex));

    IDirect3DDevice9_SetSamplerState(gDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    IDirect3DDevice9_SetSamplerState(gDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    IDirect3DDevice9_SetSamplerState(gDevice, 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);

    IDirect3DDevice9_SetSamplerState(gDevice, 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    IDirect3DDevice9_SetSamplerState(gDevice, 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    IDirect3DDevice9_SetSamplerState(gDevice, 1, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);

    IDirect3DDevice9_SetRenderState(gDevice, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    IDirect3DDevice9_SetRenderState(gDevice, D3DRS_CULLMODE, D3DCULL_CCW);

    gCurTexture = gTextures;
    setTexture(NULL); // will reset gCurTexture to NULL

    renderSetMode(RENDER_MODE_2D);
}

void renderFrameEnd(void)
{
#ifdef __WIN32__
    IDirect3DDevice9_EndScene(gDevice);
#endif
}

volatile Vertex* gTrackVertices;

void renderSection(const TrackSection* section)
{
    const Face* face = gTrack.faces + section->faceStart;
    const Face* last = face + section->faceCount;
    const vec3i* vertices = gTrack.vertices;
    float u0, u1;
    uint32 color;
    volatile Vertex* v = gTrackVertices;

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

        color = face->color;

        color = (color & 0x00FFFFFF) | (face->texture << 24); // pack texture indect into color.a
        VERTEX_SET(v[0], v3->x, v3->y, v3->z, u0, 1.0f, color);
        VERTEX_SET(v[1], v2->x, v2->y, v2->z, u1, 1.0f, color);
        VERTEX_SET(v[2], v1->x, v1->y, v1->z, u1, 0.0f, color);
        VERTEX_SET(v[3], v0->x, v0->y, v0->z, u0, 0.0f, color);
        v += 4;

        face++;
    }

    gTrackVertices = v;
}

void renderTrack(void)
{
    int32 i, j, s, fCount;
    int32 dir;
    int16* counts;
    int16** sections;
    TrackSection* section;

    dir = TrackDirection(&gCamera);

    setMatrix(gMatrixPtr);
    setShader(&shader_track, NULL);

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

    // calc total number of faces
    section = gTrack.sections + sections[0][0];
    fCount = section->prevSection->faceCount;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < counts[i]; j += 2)
        {
            section = gTrack.sections + sections[i][j];

            for (s = 0; s < sections[i][j + 1]; s++)
            {
                fCount += section->faceCount;
                section = section->nextSection;
            }
        }
    }

    // allocate quad list
    DRAW_BEGIN(gTrackVertices, D3DPT_QUADLIST, fCount * 4);

    section = gTrack.sections + sections[0][0];
    renderSection(section->prevSection);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < counts[i]; j += 2)
        {
            section = gTrack.sections + sections[i][j];

            for (s = 0; s < sections[i][j + 1]; s++)
            {
                renderSection(section);
                section = section->nextSection;
            }
        }
    }

    DRAW_END();
}

void renderEffects(const Effect* fx)
{
    int32 x, y, z, rx, ry, rz, ux, uy, uz, size;
    volatile Vertex* v;

    renderSetBlend(BLEND_ADD);
    setDepthWrite(0);
    setMatrix(gMatrixPtr);
    setShader(&shader_object, NULL);

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

        setTexture(gEffectTextures + fx->type);

        DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
        VERTEX_SET(v[0], x - rx - ux, y - ry - uy, z - rz - uz, 0.0f, 0.0f, 0xFF808080);
        VERTEX_SET(v[1], x + rx - ux, y - ry - uy, z + rz - uz, 1.0f, 0.0f, 0xFF808080);
        VERTEX_SET(v[2], x + rx + ux, y + ry + uy, z + rz + uz, 1.0f, 1.0f, 0xFF808080);
        VERTEX_SET(v[3], x - rx + ux, y + ry + uy, z - rz + uz, 0.0f, 1.0f, 0xFF808080);
        DRAW_END();

        fx = fx->next;
    }

    setDepthWrite(1);
    renderSetBlend(BLEND_NONE);
}

void drawMesh(const Object* obj)
{
    volatile Vertex* v;
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
            setTexture(t);
            iw = t->iw;
            ih = t->ih;
        }
        else
        {
            setTexture(NULL);
            iw = 0.0f;
            ih = 0.0f;
        }

        if (FACE_QUAD(face))
        {
            DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
            VERTEX_SET_FACE(0, face, vertices, v);
            VERTEX_SET_FACE(1, face, vertices, v);
            VERTEX_SET_FACE(2, face, vertices, v);
            VERTEX_SET_FACE(3, face, vertices, v);
            DRAW_END();
        }
        else
        {
            DRAW_BEGIN(v, D3DPT_TRIANGLELIST, 3);
            VERTEX_SET_FACE(0, face, vertices, v);
            VERTEX_SET_FACE(1, face, vertices, v);
            VERTEX_SET_FACE(2, face, vertices, v);
            DRAW_END();
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

        setTexture(t);

        DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
        VERTEX_SET(v[0], p->x - wx, p->y - h0, p->z - wz, 0.0f, 0.0f, sprite->color);
        VERTEX_SET(v[1], p->x + wx, p->y - h0, p->z + wz, 1.0f, 0.0f, sprite->color);
        VERTEX_SET(v[2], p->x + wx, p->y + h1, p->z + wz, 1.0f, 1.0f, sprite->color);
        VERTEX_SET(v[3], p->x - wx, p->y + h1, p->z - wz, 0.0f, 1.0f, sprite->color);
        DRAW_END();

        sprite++;
    }
}

void drawMeshShip(const Object* obj, uint32 color)
{
    volatile Vertex* v;
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
    setShader(&shader_ship, colorParam);

    while (face < fLast)
    {
        if ((face->flags & FACE_FLAG_ENGINE) == 0)
        {
            uint32 texIndex = FACE_TEXTURE(face);

            // TODO precalc float uv's
            if (texIndex != NO_TEXTURE)
            {
                Texture* t = gTextures + FACE_TEXTURE(face);
                setTexture(t);
                iw = t->iw;
                ih = t->ih;
            }
            else
            {
                setTexture(NULL);
                iw = 0.0f;
                ih = 0.0f;
            }

            if (FACE_QUAD(face))
            {
                DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
                VERTEX_SET_FACE(0, face, vertices, v);
                VERTEX_SET_FACE(1, face, vertices, v);
                VERTEX_SET_FACE(2, face, vertices, v);
                VERTEX_SET_FACE(3, face, vertices, v);
                DRAW_END();
            }
            else
            {
                DRAW_BEGIN(v, D3DPT_TRIANGLELIST, 3);
                VERTEX_SET_FACE(0, face, vertices, v);
                VERTEX_SET_FACE(1, face, vertices, v);
                VERTEX_SET_FACE(2, face, vertices, v);
                DRAW_END();
            }
        }

        face++;
    }

// semi-transparent
    colorParam[0] = colorParam[1] = colorParam[2] = 0.125f;
    setShader(&shader_ship, colorParam);

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
                setTexture(t);
                iw = t->iw;
                ih = t->ih;
            }
            else
            {
                setTexture(NULL);
                iw = 0.0f;
                ih = 0.0f;
            }

            ASSERT(FACE_QUAD(face) == 0); // triangles only

            DRAW_BEGIN(v, D3DPT_TRIANGLELIST, 3);
            VERTEX_SET_FACE(0, face, vertices, v);
            VERTEX_SET_FACE(1, face, vertices, v);
            VERTEX_SET_FACE(2, face, vertices, v);
            DRAW_END();
        }

        face++;
    }

    renderSetBlend(BLEND_NONE);
}

void renderObject(const Object* object)
{
    setMatrix(gMatrixPtr);
    setShader(&shader_object, NULL);

    drawMesh(object);
}

void renderShip(const Object* object, uint32 color)
{
    setMatrix(gMatrixPtr);
    drawMeshShip(object, color);
}

void renderShield(const Object* object, int32 counter)
{
    float colorParam[4] = { i255 * (0xFF & (counter)), 0, 0, 0 };

    setMatrix(gMatrixPtr);
    renderSetBlend(BLEND_ADD);
    setShader(&shader_shield, colorParam);

    drawMesh(object);

    renderSetBlend(BLEND_NONE);
}

void renderSky(const Object* object)
{
    setMatrix(gMatrixPtr);
    setShader(&shader_object, NULL);
    setDepthTest(0);

    drawMesh(object);

    setDepthTest(1);
}

void renderTargetIcon(Texture* tex)
{
    volatile Vertex* v;
    float px, py, pz;
    int32 size;
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

    setTexture(tex);
    setFilter(1);

    DRAW_BEGIN(v, D3DPT_QUADLIST, 4);
    VERTEX_SET(v[0], px - size, py - size, 0, 0.0f, 1.0f, 0xFF808080);
    VERTEX_SET(v[1], px + size, py - size, 0, 1.0f, 1.0f, 0xFF808080);
    VERTEX_SET(v[2], px + size, py + size, 0, 1.0f, 0.0f, 0xFF808080);
    VERTEX_SET(v[3], px - size, py + size, 0, 0.0f, 0.0f, 0xFF808080);
    DRAW_END();

    setFilter(0);
    renderSetBlend(BLEND_NONE);
}

void renderSetMode(int renderMode)
{
    if (renderMode == RENDER_MODE_2D)
    {
        setFilter(0);

        matrixOrtho(&mProj, (float)guiL, (float)guiR, (float)guiB, (float)guiT, 0, 1);
        mProj2D = mProj;

        setDepthTest(0);
        renderSetBlend(BLEND_NONE);

        setShader(&shader_gui, NULL);
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
    tex->obj = NULL;

    if (HACK_LOAD_TRACK)
    {
        fillTrackTex(tempData);
    }
    else
    {
        D3DLOCKED_RECT rect;
        int32 y;
        uint8* dst;
        uint8* src = (uint8*)tempData;

        IDirect3DDevice9_CreateTexture(gDevice, w, h, 1, 0, D3DFMT_LIN_A1R5G5B5, D3DPOOL_DEFAULT, &(D3DTexture*)tex->obj, NULL);
        D3DTexture_LockRect((D3DTexture*)tex->obj, 0, &rect, NULL, 0);
        dst = (uint8*)rect.pBits;
        for (y = 0; y < h; y++)
        {
            memcpy(dst, src, w * sizeof(uint16));
            src += w * sizeof(uint16);
            dst += rect.Pitch;
        }
        D3DTexture_UnlockRect((D3DTexture*)tex->obj, 0);
    }

    gTexturePtr++;

    return tex;
}

void renderDeleteTexture(Texture* texture)
{
    if (!texture->obj)
        return;
    setTexture(NULL);
    D3DTexture_Release((D3DTexture*)texture->obj);
    texture->obj = NULL;
}
