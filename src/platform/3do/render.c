#include "game.h"

#define USE_SWI

extern int32 screenPage;
extern ScreenContext screen;
extern TrackCamera ctrlcamera;

Item irqVBL;
Item irqVRAM;

Item screenItem;
int32 screenPage;

int32 xOffset;
int32 yOffset;

int gWidth, gHeight, gBorder;
int gCylX, gCylZ;

#define MAX_CEL_FACES       2048

#define FRAME_WIDTH         320
#define FRAME_HEIGHT        240

#define MAX_VERTICES_BUFFER 4096

vec3i vBuffer[MAX_VERTICES_BUFFER];

#define OT_SHIFT    7
#define OT_SIZE     512

typedef struct FaceCEL
{
    uint32 ccb_Flags;

    struct FaceCEL* ccb_NextPtr;
    void* ccb_SourcePtr;
    void* ccb_PLUTPtr;

    int32 ccb_XPos;
    int32 ccb_YPos;
    int32 ccb_HDX;
    int32 ccb_HDY;
    int32 ccb_VDX;
    int32 ccb_VDY;
    int32 ccb_HDDX;
    int32 ccb_HDDY;
    int32 ccb_PIXC;
    uint32 ccb_PRE0;
    uint32 ccb_PRE1;
    // we don't need this
    //int32 ccb_Width;
    //int32 ccb_Height;
} FaceCEL;

typedef struct {
    FaceCEL* head;
    FaceCEL* tail;
} ListOT;

ListOT gOT[OT_SIZE];

FaceCEL gFaces[MAX_CEL_FACES];
FaceCEL* gFacesBase;

extern Item screenItem;

uint16* gHighlightTextPLUT;

IOInfo clearInfo = {
    FLASHWRITE_CMD, IO_QUICK, 0, 0, -1, 0, 0,
    { NULL, 0 },
    { NULL, 320 * 240 * 2 }
};

void clearFast(int32 page)
{
    clearInfo.ioi_Recv.iob_Buffer = screen.sc_Bitmaps[page]->bm_Buffer;
    SendIO(irqVRAM, &clearInfo);
}

const int chars[10][20] = {
    {
        0,1,1,0,
        1,0,0,1,
        1,0,0,1,
        1,0,0,1,
        0,1,1,0
    }, {
        0,0,1,0,
        0,1,1,0,
        0,0,1,0,
        0,0,1,0,
        0,0,1,0
    }, {
        0,1,1,0,
        1,0,0,1,
        0,0,1,1,
        0,1,0,0,
        1,1,1,1
    }, {
        1,1,1,0,
        0,0,0,1,
        0,0,1,0,
        1,0,0,1,
        0,1,1,0
    }, {
        0,0,0,1,
        0,0,1,1,
        0,1,0,1,
        1,1,1,1,
        0,0,0,1
    }, {
        1,1,1,1,
        1,0,0,0,
        1,1,1,0,
        0,0,0,1,
        1,1,1,0
    }, {
        0,1,1,0,
        1,0,0,0,
        1,1,1,0,
        1,0,0,1,
        0,1,1,0
    }, {
        1,1,1,1,
        0,0,0,1,
        0,0,1,0,
        0,1,0,0,
        0,1,0,0
    }, {
        0,1,1,0,
        1,0,0,1,
        0,1,1,0,
        1,0,0,1,
        0,1,1,0
    }, {
        0,1,1,0,
        1,0,0,1,
        0,1,1,1,
        0,0,0,1,
        0,1,1,0
    }
};

void drawChar(int32 x, int32 y, int32 c)
{
    int32 i, j;
    uint16* ptr = (uint16*)screen.sc_Bitmaps[screenPage]->bm_Buffer;

    ptr += y / 2 * 2 * 320;
    ptr += x * 2;
    ptr += y & 1;

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 4; j++)
        {                                                       
            if (chars[c][i * 4 + j])
            {
                ptr[j * 2] = 0xFFFF;
            }
        }

        if ((y + i) & 1) {
            ptr += 320 * 2 - 1;
        } else {
            ptr += 1;
        }
    }
}

void drawInt(int32 x, int32 y, int32 number)
{
    while (number)
    {
        drawChar(x, y, number % 10);
        number /= 10;
        x -= 6;
    }
}

void EnableSuperClipping()
{
    int32 i;
    for (i = 0; i < 2; i++)
    {
        SetCEControl(screen.sc_BitmapItems[i], 0xFFFFFFFF, ASCALL);
        DisableHAVG(screen.sc_BitmapItems[i]);
        DisableVAVG(screen.sc_BitmapItems[i]);
    }
}

void renderInit(void)
{
    static const uint16 HIGHLIGHT_TEXT_PLUT[] = {
        0x0000, 0x0C40, 0x1461, 0x1C81,
        0x28C1, 0x3102, 0x3922, 0x4162,
        0x49A3, 0x51C3, 0x5A03, 0x5E43,
        0x6663, 0x6A83, 0x72C3, 0x7B03 }; // shadow color

    irqVBL = GetVBLIOReq();
    irqVRAM = GetVRAMIOReq();
    screenItem = screen.sc_ScreenItems[screenPage];

    //EnableSuperClipping(); // perf spikes and total blackout :(

    gHighlightTextPLUT = (uint16*)allocVRAM(sizeof(HIGHLIGHT_TEXT_PLUT));
    memcpy(gHighlightTextPLUT, HIGHLIGHT_TEXT_PLUT, sizeof(HIGHLIGHT_TEXT_PLUT));
}

void renderFree(void) {}

void renderSwap(void)
{
    DisplayScreen(screen.sc_Screens[screenPage], NULL);
    screenPage ^= 1;

    WaitVBL(irqVBL, 1);
    clearFast(screenPage); // TODO remove
    screenItem = screen.sc_ScreenItems[screenPage];
}

void renderResize(int width, int height) {}

void renderOffset(int x, int y)
{
    xOffset = x + (FRAME_WIDTH >> 1);
    yOffset = y + (FRAME_HEIGHT >> 1);
}

void renderClear(void) {}

FaceCEL* faceAdd(int32 depth)
{
    FaceCEL* face = gFacesBase++;

    if (depth >= OT_SIZE)
    {
        depth = OT_SIZE - 1;
    }

    if (gOT[depth].head) {
        gOT[depth].tail->ccb_NextPtr = face;
    } else {
        gOT[depth].head = face;
    }

    gOT[depth].tail = face;

    return face;
}

void renderGlyph(const Glyph* g, int32 x, int32 y, uint32 color)
{
    // Texture must be 4-bpp >= 16 px width
    int32 woffset, skipx;
    FaceCEL* face;
    const Texture* t = g->texture;

    if (!t->plut)
        return; // TODO temporary for 8-bpp and 16-bpp

    woffset = ((t->width << 2) + 31) >> 5; // bitmap pitch in 32-bit words
    skipx = g->x & 7;

    face = faceAdd(0);
    face->ccb_SourcePtr = (uint8*)((uint32*)t->data + (g->x >> 3) + g->y * woffset);
    face->ccb_PLUTPtr = (color == TEXT_COLOR_HIGHLIGHT) ? gHighlightTextPLUT : t->plut;
    face->ccb_PRE0 = ((g->h - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4 | (skipx << PRE0_SKIPX_SHIFT);
    face->ccb_PRE1 = ((g->w + skipx - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);

    face->ccb_Flags =
        CCB_NPABS  |
        CCB_SPABS  |
        CCB_PPABS  |
        CCB_LDSIZE |
        CCB_LDPRS  |
        CCB_LDPPMP |
        CCB_LDPLUT |
        CCB_CCBPRE |
        CCB_YOXY   |
        CCB_ACSC   | CCB_ALSC |
        CCB_ACW    | CCB_ACCW |
        CCB_ACE    |
        CCB_NOBLK;

    face->ccb_XPos = x << 16;
    face->ccb_YPos = y << 16;
    face->ccb_HDX = 1 << 20;
    face->ccb_HDY = 0;
    face->ccb_VDX = 0;
    face->ccb_VDY = 1 << 16;
    face->ccb_HDDX = 0;
    face->ccb_HDDY = 0;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;
}

void renderImage(const Texture* t, int32 x, int32 y)
{
    uint32 woffset, flags, pre0, pre1;
    FaceCEL* face;

    face = faceAdd(0);

    face->ccb_SourcePtr = t->data;

    flags =
        CCB_NPABS  |
        CCB_SPABS  |
        CCB_PPABS  |
        CCB_LDSIZE |
        CCB_LDPRS  |
        CCB_LDPPMP |
        CCB_CCBPRE |
        CCB_YOXY   |
        CCB_ACSC   | CCB_ALSC |
        CCB_ACW    | CCB_ACCW |
        CCB_ACE    |
        CCB_NOBLK;

    if (t->plut)
    {
        face->ccb_PLUTPtr = t->plut;
        flags |= CCB_LDPLUT;

        woffset = t->width >> 3; // 8 px per word
        pre0 = ((t->height - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
        pre1 = ((t->width - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
    }
    else
    {
        woffset = t->width >> 1; // 2 px per word
        pre0 = ((t->height - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_16 | PRE0_LINEAR;
        pre1 = ((t->width - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET10_SHIFT);
    }

    face->ccb_Flags = flags;
    face->ccb_XPos = x << 16;
    face->ccb_YPos = y << 16;
    face->ccb_HDX = 1 << 20;
    face->ccb_HDY = 0;
    face->ccb_VDX = 0;
    face->ccb_VDY = 1 << 16;
    face->ccb_HDDX = 0;
    face->ccb_HDDY = 0;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;
    face->ccb_PRE0 = pre0;
    face->ccb_PRE1 = pre1;
}

void renderBackground(const Texture* t)
{
    // Texture must be 320x240 16-bpp
    FaceCEL* face = faceAdd(OT_SIZE - 1);
    face->ccb_SourcePtr = t->data;
    face->ccb_PRE0 = ((240 - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_16 | PRE0_LINEAR | PRE0_BGND;
    face->ccb_PRE1 = ((320 - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((160 - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET10_SHIFT);

    face->ccb_Flags =
        CCB_NPABS  |
        CCB_SPABS  |
        CCB_PPABS  |
        CCB_LDSIZE |
        CCB_LDPRS  |
        CCB_LDPPMP |
        CCB_CCBPRE |
        CCB_YOXY   |
        CCB_ACSC   | CCB_ALSC |
        CCB_ACW    | CCB_ACCW |
        CCB_ACE    |
        CCB_NOBLK  |
        CCB_BGND;

    face->ccb_XPos = 0;
    face->ccb_YPos = 0;
    face->ccb_HDX = 1 << 20;
    face->ccb_HDY = 0;
    face->ccb_VDX = 0;
    face->ccb_VDY = 1 << 16;
    face->ccb_HDDX = 0;
    face->ccb_HDDY = 0;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;
}

void renderShadow(const Shadow* s) {}
void renderColorQuad(const ColorQuad* q) {}
void renderBillboard(const Billboard* b) {}

void renderBackFill(int x, int y, int w, int h)
{
    // TODO compare with 16-bpp
    FaceCEL* face;

    face = faceAdd(0);
    face->ccb_SourcePtr = (uint8*)(gHighlightTextPLUT);
    face->ccb_PLUTPtr = gHighlightTextPLUT;

    face->ccb_Flags =
        CCB_NPABS  |
        CCB_SPABS  |
        CCB_PPABS  |
        CCB_LDSIZE |
        CCB_LDPRS  |
        CCB_LDPPMP |
        CCB_LDPLUT |
        CCB_CCBPRE |
        CCB_YOXY   |
        CCB_ACSC   | CCB_ALSC |
        CCB_ACW    | CCB_ACCW |
        CCB_ACE    |
        CCB_NOBLK  |
        CCB_BGND;

    face->ccb_XPos = x << 16;
    face->ccb_YPos = y << 16;
    face->ccb_HDX = w << 20;
    face->ccb_HDY = 0;
    face->ccb_VDX = 0;
    face->ccb_VDY = h << 16;
    face->ccb_HDDX = 0;
    face->ccb_HDDY = 0;
    face->ccb_PIXC = PPMPC_1S_CFBD | PPMPC_SF_8 | PPMPC_2S_PDC | (3 << PPMPC_MF_SHIFT);
    face->ccb_PRE0 = PRE0_BGND | PRE0_BPP_4;
    face->ccb_PRE1 = PRE1_TLLSB_PDC0;
}

void renderPrepare(void) {}

int32 dbg_render;

void renderFrameBegin(void)
{
    dbg_render = osGetSystemTimeMS();
    gFacesBase = gFaces;
}

void renderFrameEnd(void)
{
    FaceCEL* facesHead = NULL;
    FaceCEL* facesTail = NULL;
    FaceCEL* face;
    int32 i;

    if (gFaces != gFacesBase)
    {
        for (i = OT_SIZE - 1; i >= 0; i--)
        {
            if (!gOT[i].head) continue;

            face = gOT[i].head;
            gOT[i].head = NULL;

            if (face)
            {
                if (facesTail)
                {
                    facesTail->ccb_NextPtr = face;
                } else if (!facesHead) {
                    facesHead = face;
                }

                facesTail = gOT[i].tail;
            }
        }

        dbg_render = osGetSystemTimeMS() - dbg_render;

        if (facesHead)
        {
            LAST_CEL(facesTail);
            DrawScreenCels(screenItem, (CCB*)facesHead);
            UNLAST_CEL(facesTail);
        }
    }

    gFacesBase = gFaces;
}

int32 gMatrix[3][3];
int cx, cy, cz;

#define DP3(row, v) ((gMatrix[row][0] * v->x + gMatrix[row][1] * v->y + gMatrix[row][2] * v->z) >> 12)
#define DP3_XYZ(row) ((gMatrix[row][0] * vx + gMatrix[row][1] * vy + gMatrix[row][2] * vz) >> 12)

#define TRANSFORM(p, v) {\
        p.x = DP3(0, v) + cx;\
        p.y = DP3(1, v) + cy;\
        p.z = DP3(2, v) + cz;\
    }

#define PROJECT(p) {\
        int32 z = p.z >> 3;\
        if (z >= DIV_TABLE_SIZE) z = DIV_TABLE_SIZE - 1;\
        z = FixedInvU(z);\
        p.x = (p.x * z) >> 12;\
        p.y = (p.y * z) >> 12;\
    }

#define AVG(p, a, b) {\
        p->x = (a->x + b->x) >> 1;\
        p->y = (a->y + b->y) >> 1;\
        p->z = (a->z + b->z) >> 1;\
    }

int32 cross(const vec3i *a, const vec3i *b, const vec3i *c)
{
    return (b->x - a->x) * (c->y - a->y) -
           (c->x - a->x) * (b->y - a->y);
}

void ccbMap4_track(FaceCEL* face, const vec3i* v0, const vec3i* v1, const vec3i* v2, const vec3i* v3, uint8* data, uint8* plut)
{
    int32 x0 = v0->x;
    int32 y0 = v0->y;
    int32 x1 = v1->x;
    int32 x2 = v2->x;
    int32 y2 = v2->y;
    int32 y1 = v1->y;
    int32 x3 = v3->x;
    int32 y3 = v3->y;

    int32 hdx0 = (x1 - x0) << (20 - 5);
    int32 hdy0 = (y1 - y0) << (20 - 5);
    int32 hdx1 = (x2 - x3) << (20 - 5);
    int32 hdy1 = (y2 - y3) << (20 - 5);
    int32 vdx0 = (x3 - x0) << (16 - 5);
    int32 vdy0 = (y3 - y0) << (16 - 5);

    face->ccb_Flags =
        CCB_NPABS  |
        CCB_SPABS  |
        CCB_PPABS  |
        CCB_LDSIZE |
        CCB_LDPRS  |
        CCB_LDPPMP |
        CCB_LDPLUT |
        CCB_CCBPRE |
        CCB_YOXY   |
        CCB_ACSC   | CCB_ALSC |
        CCB_ACW    | CCB_ACCW |
        CCB_ACE    |
        CCB_NOBLK  |
        CCB_BGND;

    face->ccb_SourcePtr = data;
    face->ccb_PLUTPtr = plut;
    face->ccb_XPos = (x0 + xOffset) << 16;
    face->ccb_YPos = (y0 + yOffset) << 16;
    face->ccb_HDX = hdx0;
    face->ccb_HDY = hdy0;
    face->ccb_VDX = vdx0;
    face->ccb_VDY = vdy0;
    face->ccb_HDDX = (hdx1 - hdx0) >> 5;
    face->ccb_HDDY = (hdy1 - hdy0) >> 5;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;// | PPMPC_2S_PDC;
    face->ccb_PRE0 = ((32 - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    face->ccb_PRE1 = ((32 - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((4 - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
}

void ccbMap4(FaceCEL* face, const vec3i* v0, const vec3i* v1, const vec3i* v2, const vec3i* v3, uint32 texIndex)
{
    int32 x0 = v0->x;
    int32 y0 = v0->y;
    int32 x1 = v1->x;
    int32 x2 = v2->x;
    int32 y2 = v2->y;
    int32 y1 = v1->y;
    int32 x3 = v3->x;
    int32 y3 = v3->y;

    uint32 ws = 20 - 5;
    uint32 hs = 16 - 5;

    int32 hdx0 = (x1 - x0) << ws;
    int32 hdy0 = (y1 - y0) << ws;
    int32 hdx1 = (x2 - x3) << ws;
    int32 hdy1 = (y2 - y3) << ws;
    int32 vdx0 = (x3 - x0) << hs;
    int32 vdy0 = (y3 - y0) << hs;
    int32 hddx;
    int32 hddy;
    uint32 woffset = ((32 << 2) + 31) >> 5;

    const Texture* texture = gTextures + texIndex;

    hs = 16 - hs;
    hddx = (hdx1 - hdx0) >> hs;
    hddy = (hdy1 - hdy0) >> hs;

    face->ccb_Flags =
        CCB_NPABS  |
        CCB_SPABS  |
        CCB_PPABS  |
        CCB_LDSIZE |
        CCB_LDPRS  |
        CCB_LDPPMP |
        CCB_LDPLUT |
        CCB_CCBPRE |
        CCB_YOXY   |
        CCB_ACSC   | CCB_ALSC |
        CCB_ACW    | CCB_ACCW |
        CCB_ACE    |
        CCB_NOBLK  |
        CCB_BGND;
    face->ccb_SourcePtr = texture->data;
    face->ccb_PLUTPtr = texture->plut;
    face->ccb_XPos = (x0 + xOffset) << 16;
    face->ccb_YPos = (y0 + yOffset) << 16;
    face->ccb_HDX = hdx0;
    face->ccb_HDY = hdy0;
    face->ccb_VDX = vdx0;
    face->ccb_VDY = vdy0;
    face->ccb_HDDX = hddx;
    face->ccb_HDDY = hddy;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8 | PPMPC_2D_2;
    face->ccb_PRE0 = ((32 - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    face->ccb_PRE1 = ((32 - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
}

void ccbMap3(FaceCEL* face, const vec3i* v0, const vec3i* v1, const vec3i* v2, uint32 texIndex)
{
    int32 x0 = v0->x;
    int32 y0 = v0->y;
    int32 x1 = v1->x;
    int32 y1 = v1->y;
    int32 x2 = v2->x;
    int32 y2 = v2->y;

    uint32 ws = 20 - 5;
    uint32 hs = 16 - 5;

    int32 hdx0 = (x1 - x0) << ws;
    int32 hdy0 = (y1 - y0) << ws;
    int32 hdx1 = (x2 - x0) << ws;
    int32 hdy1 = (y2 - y0) << ws;
    int32 hddx;
    int32 hddy;
    uint32 woffset = ((32 << 2) + 31) >> 5;

    const Texture* texture = gTextures + texIndex;

    hs = 16 - hs;
    hddx = (hdx1 - hdx0) >> hs;
    hddy = (hdy1 - hdy0) >> hs;

    face->ccb_Flags =
        CCB_NPABS  |
        CCB_SPABS  |
        CCB_PPABS  |
        CCB_LDSIZE |
        CCB_LDPRS  |
        CCB_LDPPMP |
        CCB_LDPLUT |
        CCB_CCBPRE |
        CCB_YOXY   |
        CCB_ACSC   | CCB_ALSC |
        CCB_ACW    | CCB_ACCW |
        CCB_ACE    |
        CCB_NOBLK  |
        CCB_BGND;

    face->ccb_SourcePtr = texture->data;
    face->ccb_PLUTPtr = texture->plut;
    face->ccb_XPos = (x0 + xOffset) << 16;
    face->ccb_YPos = (y0 + yOffset) << 16;
    face->ccb_HDX = hdx0;
    face->ccb_HDY = hdy0;
    face->ccb_VDX = 0;
    face->ccb_VDY = 0;
    face->ccb_HDDX = hddx;
    face->ccb_HDDY = hddy;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8 | PPMPC_2D_2;
    face->ccb_PRE0 = ((32 - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    face->ccb_PRE1 = ((32 - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
}

void renderSetSkeleton(Skeleton* skeleton, int x, int y, int z)
{
    MATRIX* m = GetSkeletonMatrix(skeleton);

#ifdef USE_SWI
    // extra shift fo make f16 from f12
    gMatrix[0][0] = m->m[0][0] << 4;
    gMatrix[0][1] = m->m[1][0] << 4;
    gMatrix[0][2] = m->m[2][0] << 4;
    gMatrix[1][0] = m->m[0][1] << 4;
    gMatrix[1][1] = m->m[1][1] << 4;
    gMatrix[1][2] = m->m[2][1] << 4;
    gMatrix[2][0] = m->m[0][2] << 4;
    gMatrix[2][1] = m->m[1][2] << 4;
    gMatrix[2][2] = m->m[2][2] << 4;
#else
    gMatrix[0][0] = m->m[0][0];
    gMatrix[0][1] = m->m[0][1];
    gMatrix[0][2] = m->m[0][2];
    gMatrix[1][0] = m->m[1][0];
    gMatrix[1][1] = m->m[1][1];
    gMatrix[1][2] = m->m[1][2];
    gMatrix[2][0] = m->m[2][0];
    gMatrix[2][1] = m->m[2][1];
    gMatrix[2][2] = m->m[2][2];
#endif

    cx = x;
    cy = y;
    cz = z;
}

void renderSetSkeletonOLD(Skeleton* skeleton, int x, int y, int z)
{
    MATRIX* m = GetSkeletonMatrix(skeleton);

    gMatrix[0][0] = m->m[0][0];
    gMatrix[0][1] = m->m[0][1];
    gMatrix[0][2] = m->m[0][2];
    gMatrix[1][0] = m->m[1][0];
    gMatrix[1][1] = m->m[1][1];
    gMatrix[1][2] = m->m[1][2];
    gMatrix[2][0] = m->m[2][0];
    gMatrix[2][1] = m->m[2][1];
    gMatrix[2][2] = m->m[2][2];

    cx = x;
    cy = y;
    cz = z;
}

#define ZNEAR 64

void trackTransProj(const vec3i* src, int32 start, int32 count)
{
    vec3i* dst = vBuffer + start;
    vec3i* end = dst + count;
    int32 x, y, z;
    int32 tx, ty, tz;
    int32 dz;

    tx = cx;
    ty = cy;
    tz = cz;

    src += start;

#ifdef USE_SWI
    MulManyVec3Mat33_F16((vec3f16*)dst, (vec3f16*)src, *(mat33f16*)&gMatrix, count);

    while (dst < end)
    {
        x = dst->x;
        y = dst->y;
        z = dst->z;

        x += tx;
        y += ty;
        z += tz;

        if (z < ZNEAR)
        {
            z = ZNEAR;
        }

        dz = z >> 3;
        if (dz >= DIV_TABLE_SIZE) dz = DIV_TABLE_SIZE - 1;
        dz = FixedInvU(dz);

        dst->x = (x * dz) >> 12;
        dst->y = (y * dz) >> 12;
        dst->z = z;
        dst++;
    }
#else
    while (dst < end)
    {
        x = DP3(0, src) + tx;
        y = DP3(1, src) + ty;
        z = DP3(2, src) + tz;
        src++;

        if (z < ZNEAR)
        {
            z = ZNEAR;
        }

        dz = z >> 3;
        if (dz >= DIV_TABLE_SIZE) dz = DIV_TABLE_SIZE - 1;
        dz = FixedInvU(dz);

        dst->x = (x * dz) >> 12;
        dst->y = (y * dz) >> 12;
        dst->z = z;

        dst++;
    }
#endif
}

void trackTrans(const vec3i* src, int32 start, int32 count)
{
    vec3i* dst = vBuffer + start;
    vec3i* end = dst + count;
    int32 x, y, z;
    int32 px, py, pz;

    px = cx;
    py = cy;
    pz = cz;
    
    src += start;

#ifdef USE_SWI
    MulManyVec3Mat33_F16((vec3f16*)dst, (vec3f16*)src, *(mat33f16*)&gMatrix, count);
#endif

    while (dst < end)
    {
#ifdef USE_SWI
        x = dst->x;
        y = dst->y;
        z = dst->z;
#else
        x = DP3(0, src);
        y = DP3(1, src);
        z = DP3(2, src);
        src++;
#endif
        x += px;
        y += py;
        z += pz;

        dst->x = x;
        dst->y = y;
        dst->z = z;
        dst++;
    }
}

void trackProj(vec3i* dst, int32 start, int32 count)
{
    vec3i* end = dst + start + count;
    int32 x, y, z;
    int32 dz;

    dst += start;

    while (dst < end)
    {
        x = dst->x;
        y = dst->y;
        z = dst->z;

        if (z < ZNEAR)
        {
            z = ZNEAR;
        }

        dz = z >> 3;
        if (dz >= DIV_TABLE_SIZE) dz = DIV_TABLE_SIZE - 1;
        dz = FixedInvU(dz);

        x = (x * dz) >> 12;
        y = (y * dz) >> 12;

        dst->x = x;
        dst->y = y;
        dst->z = z;
        dst++;
    }
}

void transMesh(const vec4s* src, int32 count)
{
    // TODO USE_SWI
    vec3i* dst = vBuffer;
    vec3i* end = dst + count;
    int32 xy, zp, x, y, z, vx, vy, vz, dz;
    int32 tx = cx;
    int32 ty = cy;
    int32 tz = cz;

    while (dst < end)
    {
        xy = ((uint32*)src)[0];
        zp = ((uint32*)src)[1];
        src++;

        vx = (xy >> 16);
        vy = (xy << 16) >> 16;
        vz = (zp >> 16);

        x = DP3_XYZ(0) + tx;
        y = DP3_XYZ(1) + ty;
        z = DP3_XYZ(2) + tz;

        if (z <= 0)
        {
            z = 0;
        }

        dz = z >> 3;
        if (dz >= DIV_TABLE_SIZE)
        {
            dz = DIV_TABLE_SIZE - 1;
        }
        dz = FixedInvU(dz);

        x = (x * dz) >> 12;
        y = (y * dz) >> 12;

        dst->x = x;
        dst->y = y;
        dst->z = z;
        dst++;
    }
}

void renderSectionsMed(const TrackRange* ranges)
{
    const Face* trackFace;
    vec3i *p0, *p1, *p2, *p3, *p01, *p12, *p23, *p30, *pM;
    int i, j, maxZ;
    vec3i* p;

    for (j = 0; j < 2; j++, ranges++)
    {
        if (!ranges->fCount)
            break;

        trackTrans((vec3i*)track->vertices, ranges->vStart, ranges->vCount);
        
        p = vBuffer + ranges->vStart + ranges->vCount;
        trackFace = track->faces + ranges->fStart;
        for (i = 0; i < ranges->fCount; i++, trackFace++, p += 5)
        {
            uint32 index01 = ((uint32*)trackFace->vertex)[0];
            uint32 index23 = ((uint32*)trackFace->vertex)[1];

            p0 = vBuffer + (index01 >> 16);
            p1 = vBuffer + (index01 << 16 >> 16);
            p2 = vBuffer + (index23 >> 16);
            p3 = vBuffer + (index23 << 16 >> 16);

            p01 = p + 0;
            p12 = p + 1;
            p23 = p + 2;
            p30 = p + 3;
            pM = p + 4;

            AVG(p01, p0, p1);
            AVG(p12, p1, p2);
            AVG(p23, p2, p3);
            AVG(p30, p3, p0);
            AVG(pM, p01, p23);
        }
        
        trackProj(vBuffer, ranges->vStart, ranges->vCount + ranges->fCount * 5);

        p = vBuffer + ranges->vStart + ranges->vCount;
        trackFace = track->faces + ranges->fStart;
        for (i = 0; i < ranges->fCount; i++, trackFace++, p += 5)
        {
            uint32 index01 = ((uint32*)trackFace->vertex)[0];
            uint32 index23 = ((uint32*)trackFace->vertex)[1];

            p0 = vBuffer + (index01 >> 16);
            p1 = vBuffer + (index01 << 16 >> 16);
            p2 = vBuffer + (index23 >> 16);
            p3 = vBuffer + (index23 << 16 >> 16);

            if (p0->z <= ZNEAR && p1->z <= ZNEAR && p2->z <= ZNEAR && p3->z <= ZNEAR)
                continue;

            p01 = p + 0;
            p12 = p + 1;
            p23 = p + 2;
            p30 = p + 3;
            pM = p + 4;

            //if (cross(p0, p1, p3) > 0)
            //    continue;

            maxZ = p0->z;
            if (maxZ < p1->z) maxZ = p1->z;
            if (maxZ < p2->z) maxZ = p2->z;
            if (maxZ < p3->z) maxZ = p3->z;

            maxZ >>= OT_SHIFT;

            if (trackFace->flags & FLIP_TEXTURE)
            {
                ccbMap4_track(faceAdd(maxZ), p0, p01, pM, p30, gTextures[ctrltexTemplate[trackFace->texture].tex4[0] + ctrllibraryTextures].data, trackFace->plut);
                ccbMap4_track(faceAdd(maxZ), p01, p1, p12, pM, gTextures[ctrltexTemplate[trackFace->texture].tex4[1] + ctrllibraryTextures].data, trackFace->plut);
                ccbMap4_track(faceAdd(maxZ), pM, p12, p2, p23, gTextures[ctrltexTemplate[trackFace->texture].tex4[3] + ctrllibraryTextures].data, trackFace->plut);
                ccbMap4_track(faceAdd(maxZ), p30, pM, p23, p3, gTextures[ctrltexTemplate[trackFace->texture].tex4[2] + ctrllibraryTextures].data, trackFace->plut);
            }
            else
            {
                ccbMap4_track(faceAdd(maxZ), p01, p0, p30, pM, gTextures[ctrltexTemplate[trackFace->texture].tex4[1] + ctrllibraryTextures].data, trackFace->plut);
                ccbMap4_track(faceAdd(maxZ), p1, p01, pM, p12, gTextures[ctrltexTemplate[trackFace->texture].tex4[0] + ctrllibraryTextures].data, trackFace->plut);
                ccbMap4_track(faceAdd(maxZ), p12, pM, p23, p2, gTextures[ctrltexTemplate[trackFace->texture].tex4[2] + ctrllibraryTextures].data, trackFace->plut);
                ccbMap4_track(faceAdd(maxZ), pM, p30, p3, p23, gTextures[ctrltexTemplate[trackFace->texture].tex4[3] + ctrllibraryTextures].data, trackFace->plut);
            }
        }
    }
}

void renderSectionsLow(const TrackRange* ranges)
{
    const Face* trackFace;
    vec3i *p0, *p1, *p2, *p3;
    FaceCEL* face;
    int i, j, maxZ;
    const Texture* texture;

    for (j = 0; j < 2; j++, ranges++)
    {
        if (!ranges->fCount)
            break;

        trackTransProj((vec3i*)track->vertices, ranges->vStart, ranges->vCount);

        trackFace = track->faces + ranges->fStart;

        for (i = 0; i < ranges->fCount; i++, trackFace++)
        {
            uint32 index01 = ((uint32*)trackFace->vertex)[0];
            uint32 index23 = ((uint32*)trackFace->vertex)[1];

            p0 = vBuffer + (index01 >> 16);
            p1 = vBuffer + (index01 << 16 >> 16);
            p2 = vBuffer + (index23 >> 16);
            p3 = vBuffer + (index23 << 16 >> 16);

            //if (p0->vz <= ZNEAR || p1->vz <= ZNEAR || p2->vz <= ZNEAR || p3->vz <= ZNEAR)
            //    continue;

            //if (cross(p0, p1, p3) > 0)
            //    continue;

            maxZ = p0->z;
            //if (maxZ < p1->vz) maxZ = p1->vz;
            //if (maxZ < p2->vz) maxZ = p2->vz;
            //if (maxZ < p3->vz) maxZ = p3->vz;

            face = faceAdd(maxZ >> OT_SHIFT);
        
            texture = gTextures + ctrltexTemplate[trackFace->texture].tex1[0] + ctrllibraryTextures;

            if (trackFace->flags & FLIP_TEXTURE)
            {
                ccbMap4_track(face, p0, p1, p2, p3, texture->data, trackFace->plut);
            }
            else
            {
                ccbMap4_track(face, p1, p0, p3, p2, texture->data, trackFace->plut);
            }
        }
    }
}

void renderTrack(void)
{
    renderSetSkeleton(
        ctrlcamera.camPos->super,
        ctrlcamera.camPos->absolute.t[0],
        ctrlcamera.camPos->absolute.t[1],
        ctrlcamera.camPos->absolute.t[2]);

    renderSectionsMed(ctrlcamera.section->visRange[0]);
    yOffset++; // HACK to hide gaps between LODs (precision problem)
    renderSectionsLow(ctrlcamera.section->visRange[1]);
    yOffset--;
}

void renderQuadFace(const int16* indices, int32 texIndex)
{
    const vec3i *p0, *p1, *p2, *p3;
    int32 avgZ;

    uint32 index01 = ((uint32*)indices)[0];
    uint32 index23 = ((uint32*)indices)[1];

    p0 = vBuffer + (index01 >> 16);
    p1 = vBuffer + (index01 << 16 >> 16);
    p2 = vBuffer + (index23 >> 16);
    p3 = vBuffer + (index23 << 16 >> 16);

    if (p0->z <= ZNEAR || p1->z <= ZNEAR || p2->z <= ZNEAR || p3->z <= ZNEAR)
        return;

    if (cross(p0, p1, p3) <= 0)
        return;

    avgZ = (p0->z + p1->z + p2->z + p3->z) >> 2;

    ccbMap4(faceAdd(avgZ >> OT_SHIFT), p0, p1, p3, p2, texIndex);
}

void renderTriFace(const int16* indices, int32 texIndex)
{
    const vec3i *p0, *p1, *p2;
    int32 avgZ;

    uint32 index01 = ((uint32*)indices)[0];
    uint32 index23 = ((uint32*)indices)[1];

    p0 = vBuffer + (index01 >> 16);
    p1 = vBuffer + (index01 << 16 >> 16);
    p2 = vBuffer + (index23 >> 16);

    if (p0->z <= ZNEAR || p1->z <= ZNEAR || p2->z <= ZNEAR)
        return;

    if (cross(p0, p1, p2) <= 0)
        return;

    avgZ = (p0->z + p1->z + p2->z + p2->z) >> 2;

    ccbMap3(faceAdd(avgZ >> OT_SHIFT), p0, p1, p2, texIndex);
}

void renderMesh(Primitive* prim, int32 count)
{
    int i;
    Prm poly;

    poly.primitive = prim;

    for (i = 0; i < count; i++)
    {
        switch (poly.primitive->type)
        {
            case TypeF3:
            {
                const F3* f = poly.f3++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                */
                break;
            }

            case TypeLSF3:
            {
                const LSF3* f = poly.lsf3++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                */
                break;
            }

            case TypeFT3:
            {
                const FT3* f = poly.ft3++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);

                if (f->flag & ShipEngine)
                {
                    uint32 color = gCurState.color;

                    renderSetColor(0xFFFFFFFF);
                    renderSetBlend(BLEND_ADD);
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &colorWhite);

                    renderSetBlend(BLEND_NONE);
                    renderSetColor(color);
                }
                else
                {
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                }
                */
                renderTriFace(f->coords, f->texture);
                break;
            }

            case TypeLSFT3:
            {
                const LSFT3* f = poly.lsft3++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                */
                break;
            }

            case TypeF4:
            {
                const F4* f = poly.f4++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                */
                break;
            }

            case TypeLSF4:
            {
                const LSF4* f = poly.lsf4++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                */
                break;
            }

            case TypeFT4:
            {
                const FT4* f = poly.ft4++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                */
                renderQuadFace(f->coords, f->texture);
                break;
            }

            case TypeLSFT4:
            {
                const LSFT4* f = poly.lsft4++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                */
                break;
            }

            case TypeG3:
            {
                const G3* f = poly.g3++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                */
                break;
            }

            case TypeLSG3:
            {
                const LSG3* f = poly.lsg3++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                */
                break;
            }

            case TypeGT3:
            {
                const GT3* f = poly.gt3++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);

                if (f->flag & ShipEngine)
                {
                    uint32 color = gCurState.color;

                    renderSetColor(0xFFFFFFFF);
                    renderSetBlend(BLEND_ADD);
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &colorWhite);

                    renderSetBlend(BLEND_NONE);
                    renderSetColor(color);
                }
                else
                {
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                }
                */
                renderTriFace(f->coords, f->texture);
                break;
            }

            case TypeLSGT3:
            {
                const LSGT3* f = poly.lsgt3++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                */
                break;
            }

            case TypeG4:
            {
                const G4* f = poly.g4++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, f->colour + 3);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                */
                break;
            }

            case TypeLSG4:
            {
                const LSG4* f = poly.lsg4++;
                /*
                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, f->colour + 3);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                */
                break;
            }

            case TypeGT4:
            {
                const GT4* f = poly.gt4++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, f->colour + 3);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                */
                renderQuadFace(f->coords, f->texture);
                break;
            }

            case TypeLSGT4:
            {
                const LSGT4* f = poly.lsgt4++;
                /*
                Texture* t = gTextures + f->texture;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, f->colour + 3);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                */
                break;
            }

            case TypeTSPR:
            case TypeBSPR:
            {
                const SPR* f = poly.spr;

                int w, h0, h1;
                int sx, sz;
                Texture* t = gTextures + f->texture;

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
                /*
                v = vBuffer + f->coord;
                sx = gCylX * w >> 12;
                sz = gCylZ * w >> 12;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertex(v->vx - sx, v->vy - h0, v->vz - sz, 0.0f, 0.0f, &f->colour);
                addVertex(v->vx + sx, v->vy - h0, v->vz + sz, 1.0f, 0.0f, &f->colour);
                addVertex(v->vx + sx, v->vy + h1, v->vz + sz, 1.0f, 1.0f, &f->colour);
                addVertex(v->vx - sx, v->vy + h1, v->vz - sz, 0.0f, 1.0f, &f->colour);
                */
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

void renderObject(const Object* object)
{
    const MATRIX* matrix;

    matrix = GetSkeletonMatrix(object->skeleton);

    renderSetSkeletonOLD(object->skeleton, matrix->t[0], matrix->t[1], matrix->t[2]);

    transMesh(object->vertices, object->vertexCount);

    renderMesh(object->primitives, object->primitiveCount);
}

void renderShip(const Object* object, uint32 color)
{
    const MATRIX* matrix;

    matrix = GetSkeletonMatrix(object->skeleton);

    renderSetSkeletonOLD(object->skeleton, matrix->t[0], matrix->t[1], matrix->t[2]);

    transMesh(object->vertices, object->vertexCount);

    renderMesh(object->primitives, object->primitiveCount);
}

void renderShield(const Object* object, int32 counter) {}

void renderSky(const Object* object)
{
    //renderObject(object);
}

void renderSetMode(int renderMode) {}
void renderSetBlend(int blend) {}
void renderDepthWrite(int enable) {}
void renderNextLayer(void) {}

Texture* renderCreateTexture(uint8* ptr)
{
    Texture* tex = gTexturePtr++;
    uint32 info = *(uint32*)ptr;

    tex->width = GET_TEX_WIDTH(info);
    tex->height = GET_TEX_HEIGHT(info);

    info = GET_TEX_TYPE(info);

    if (info == TEX_TYPE_4)
    {
        tex->plut = (uint16*)(ptr + 4);
        tex->data = ptr + sizeof(uint32) + 16 * sizeof(uint16);
    }
    else if (info == TEX_TYPE_16)
    {
        tex->plut = NULL;
        tex->data = ptr + sizeof(uint32);
    }

    return tex;
}

void renderDeleteTexture(Texture* texture)
{
    // nothing to do
}