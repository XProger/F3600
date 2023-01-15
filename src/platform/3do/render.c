#include "game.h"

//#define USE_TEST_TEX_TRACK
#define USE_TEST_TEX_OBJ

typedef struct vec2p
{
    int32 xy;
    uint32 zp;
} vec2p;

extern int32 screenPage;
extern ScreenContext screen;

Item irqVBL;
Item irqVRAM;

Item screenItem;
int32 screenPage;

int32 xOffset;
int32 yOffset;

int gCylX, gCylZ;

#define MAX_CEL_FACES       2048

#define FRAME_WIDTH         320
#define FRAME_HEIGHT        240

#define MAX_VERTICES_BUFFER 4096

vec3i vBuffer[MAX_VERTICES_BUFFER];
vec2p pBuffer[MAX_VERTICES_BUFFER];

#define OT_SHIFT    4   // actually 7 because Z already shr by 3
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

#if defined(USE_TEST_TEX_TRACK) || defined(USE_TEST_TEX_OBJ)
uint8* gTestTexData;
#endif

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

void EnableFeatures()
{
    int32 i;
    for (i = 0; i < 2; i++)
    {
        //SetCEControl(screen.sc_BitmapItems[i], 0xFFFFFFFF, ASCALL); // perf spikes and total blackout :(
        EnableHAVG(screen.sc_BitmapItems[i]);
        EnableVAVG(screen.sc_BitmapItems[i]);
    }
}

#define RGB555(r, g, b) (b >> 3) | ((g >> 3) << 5) | ((r >> 3) << 10)

void renderInit(void)
{
    static const uint16 HIGHLIGHT_TEXT_PLUT[] = {
        0x0000, 0x0C40, 0x1461, 0x1C81,
        0x28C1, 0x3102, 0x3922, 0x4162,
        0x49A3, 0x51C3, 0x5A03, 0x5E43,
        0x6663, 0x6A83, 0x72C3, 0x7B03,
    // test
        RGB555(31, 132, 255), RGB555(174, 180, 181), 0, 0
    };

    irqVBL = GetVBLIOReq();
    irqVRAM = GetVRAMIOReq();
    screenItem = screen.sc_ScreenItems[screenPage];

    EnableFeatures(); 

    gHighlightTextPLUT = (uint16*)allocVRAM(sizeof(HIGHLIGHT_TEXT_PLUT));
    memcpy(gHighlightTextPLUT, HIGHLIGHT_TEXT_PLUT, sizeof(HIGHLIGHT_TEXT_PLUT));

#if defined(USE_TEST_TEX_TRACK) || defined(USE_TEST_TEX_OBJ)
    { // generate test texture
        int32 x, y, idx = 1;

        gTestTexData = (uint8*)allocVRAM(8 * 8);


        for (x = 0; x < 8 * 8; x++)
            gTestTexData[x] = 0;


        for (y = 0; y < 8; y++) 
        {
            gTestTexData[8 * y] = idx << 4;
            gTestTexData[8 * y + 3] = idx;
        }

        for (x = 0; x < 4; x++)
        {
            gTestTexData[x] = gTestTexData[x + 7 * 8] = idx | (idx << 4);
        }
    }
#endif
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
    xOffset = (x + (FRAME_WIDTH >> 1)) << 16;
    yOffset = (y + (FRAME_HEIGHT >> 1)) << 16;
}

void renderClear(void) {}

FaceCEL* faceAdd(int32 depth)
{
    FaceCEL* face = gFacesBase++;
    ListOT* list;

    if (depth >= OT_SIZE)
    {
        depth = OT_SIZE - 1;
    }

    list = gOT + depth;

    if (list->tail)
        list->tail->ccb_NextPtr = face;
    else
        list->head = face;
    list->tail = face;

    return face;
}

#define TRACK_TEX_SHIFT 5   // 1 << 5 = 32 px
#define OBJ_TEX_SHIFT   3   // 1 << 3 = 8 px
#define SUBPX_SHIFT     2   // 1/4 px
#define Z_NEAR          (1 << 6)
#define Z_FAR           (1 << 15)
#define VP_W            ((FRAME_WIDTH >> 1) << SUBPX_SHIFT)
#define VP_H            ((FRAME_HEIGHT >> 1) << SUBPX_SHIFT)
#define GUARDBAND       (1024 << SUBPX_SHIFT)

#define DP3_XYZ(row) ((gMatrix[row][0] * vx + gMatrix[row][1] * vy + gMatrix[row][2] * vz) >> FIXED_SHIFT)

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

void ccbMap4_track(FaceCEL* face, const vec2p* v0, const vec2p* v1, const vec2p* v2, const vec2p* v3, uint8* data, uint8* plut)
{
    int32 x0 = v0->xy >> 16;
    int32 y0 = v0->xy << 16 >> 16;
    int32 x1 = v1->xy >> 16;
    int32 y1 = v1->xy << 16 >> 16;
    int32 x2 = v2->xy >> 16;
    int32 y2 = v2->xy << 16 >> 16;
    int32 x3 = v3->xy >> 16;
    int32 y3 = v3->xy << 16 >> 16;

    int32 hdx0 = (x1 - x0) << (20 - TRACK_TEX_SHIFT - SUBPX_SHIFT);
    int32 hdy0 = (y1 - y0) << (20 - TRACK_TEX_SHIFT - SUBPX_SHIFT);
    int32 hdx1 = (x2 - x3) << (20 - TRACK_TEX_SHIFT - SUBPX_SHIFT);
    int32 hdy1 = (y2 - y3) << (20 - TRACK_TEX_SHIFT - SUBPX_SHIFT);
    int32 vdx0 = (x3 - x0) << (16 - TRACK_TEX_SHIFT - SUBPX_SHIFT);
    int32 vdy0 = (y3 - y0) << (16 - TRACK_TEX_SHIFT - SUBPX_SHIFT);

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

#ifdef USE_TEST_TEX_TRACK
    face->ccb_SourcePtr = gTestTexData;
    face->ccb_PLUTPtr = gHighlightTextPLUT + 16;
#else
    face->ccb_SourcePtr = data;
    face->ccb_PLUTPtr = plut;
#endif

    face->ccb_XPos = xOffset + (x0 << (16 - SUBPX_SHIFT));
    face->ccb_YPos = yOffset + (y0 << (16 - SUBPX_SHIFT));
    face->ccb_HDX = hdx0;
    face->ccb_HDY = hdy0;
    face->ccb_VDX = vdx0;
    face->ccb_VDY = vdy0;
    face->ccb_HDDX = (hdx1 - hdx0) >> 5;
    face->ccb_HDDY = (hdy1 - hdy0) >> 5;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;
    face->ccb_PRE0 = (((1 << TRACK_TEX_SHIFT) - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    face->ccb_PRE1 = (((1 << TRACK_TEX_SHIFT) - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | (( (1 << (TRACK_TEX_SHIFT - 3)) - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
}

void ccbMap4_obj(const vec2p* vertices, uint32 indices, const Texture* texture)
{
    const vec2p *p0, *p1, *p2, *p3;
    int32 x0, y0, x1, y1, x2, y2, x3, y3;
    uint32 z0, z1, z2, z3, avgZ;
    int32 hdx0, hdy0, vdx0, vdy0, hdx1, hdy1;
    FaceCEL* face;
    //int32 xy10, xy30;

    p0 = vertices + (indices & 0xFF);
    p1 = vertices + ((indices >> 8) & 0xFF);
    p2 = vertices + ((indices >> 16) & 0xFF);
    p3 = vertices + (indices >> 24);

    x0 = p0->xy >> 16;
    y0 = p0->xy << 16 >> 16;
    z0 = p0->zp;

    x1 = p1->xy >> 16;
    y1 = p1->xy << 16 >> 16;
    z1 = p1->zp;

    x2 = p2->xy >> 16;
    y2 = p2->xy << 16 >> 16;
    z2 = p2->zp;

    x3 = p3->xy >> 16;
    y3 = p3->xy << 16 >> 16;
    z3 = p3->zp;

    if ((z0 & z1 & z2 & z3) & 1) // check for clip flag
        return;

    avgZ = (z0 + z1 + z2 + z3) >> (16 + 2 + OT_SHIFT);

/* TODO this trick has gaps
    xy10 = p1->xy - p0->xy;
    xy30 = p3->xy - p0->xy;
    hdx0 = xy10 >> 16;
    hdy0 = xy10 << 16 >> 16;
    vdx0 = xy30 >> 16;
    vdy0 = xy30 << 16 >> 16;
*/
        
    hdx0 = x1 - x0;
    hdy0 = y1 - y0;
    vdx0 = x3 - x0;
    vdy0 = y3 - y0;

    if (hdx0 * vdy0 - vdx0 * hdy0 <= 0)
        return;

    hdx1 = x2 - x3;
    hdy1 = y2 - y3;

    hdx0 <<= 20 - OBJ_TEX_SHIFT - SUBPX_SHIFT;
    hdy0 <<= 20 - OBJ_TEX_SHIFT - SUBPX_SHIFT;
    hdx1 <<= 20 - OBJ_TEX_SHIFT - SUBPX_SHIFT;
    hdy1 <<= 20 - OBJ_TEX_SHIFT - SUBPX_SHIFT;
    vdx0 <<= 16 - OBJ_TEX_SHIFT - SUBPX_SHIFT;
    vdy0 <<= 16 - OBJ_TEX_SHIFT - SUBPX_SHIFT;

    face = faceAdd(avgZ);

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

#ifdef USE_TEST_TEX_OBJ
    face->ccb_SourcePtr = gTestTexData;
    face->ccb_PLUTPtr = gHighlightTextPLUT + 16;
#else
    face->ccb_SourcePtr = texture->data;
    face->ccb_PLUTPtr = texture->plut;
#endif

    face->ccb_XPos = xOffset + (x0 << (16 - SUBPX_SHIFT));
    face->ccb_YPos = yOffset + (y0 << (16 - SUBPX_SHIFT));
    face->ccb_HDX = hdx0;
    face->ccb_HDY = hdy0;
    face->ccb_VDX = vdx0;
    face->ccb_VDY = vdy0;
    face->ccb_HDDX = (hdx1 - hdx0) >> 3;
    face->ccb_HDDY = (hdy1 - hdy0) >> 3;
    face->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;// | PPMPC_2D_2;
    face->ccb_PRE0 = (((1 << OBJ_TEX_SHIFT) - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    face->ccb_PRE1 = (((1 << OBJ_TEX_SHIFT) - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((2 - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
}

void ccbMap4_shadow(FaceCEL* face, const vec3i* v0, const vec3i* v1, const vec3i* v2, const vec3i* v3)
{
    int32 x0 = v0->x;
    int32 y0 = v0->y;
    int32 x1 = v1->x;
    int32 x2 = v2->x;
    int32 y2 = v2->y;
    int32 y1 = v1->y;
    int32 x3 = v3->x;
    int32 y3 = v3->y;

    int32 hdx0 = (x1 - x0) << 20;
    int32 hdy0 = (y1 - y0) << 20;
    int32 hdx1 = (x2 - x3) << 20;
    int32 hdy1 = (y2 - y3) << 20;
    int32 vdx0 = (x3 - x0) << 16;
    int32 vdy0 = (y3 - y0) << 16;
    int32 hddx = hdx1 - hdx0;
    int32 hddy = hdy1 - hdy0;

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

    face->ccb_SourcePtr = (uint8*)(gHighlightTextPLUT);
    face->ccb_PLUTPtr = gHighlightTextPLUT;
    face->ccb_XPos = xOffset + (x0 << 16);
    face->ccb_YPos = yOffset + (y0 << 16);
    face->ccb_HDX = hdx0;
    face->ccb_HDY = hdy0;
    face->ccb_VDX = vdx0;
    face->ccb_VDY = vdy0;
    face->ccb_HDDX = hddx;
    face->ccb_HDDY = hddy;
    face->ccb_PIXC = PPMPC_1S_CFBD | PPMPC_SF_8 | PPMPC_2S_PDC | (3 << PPMPC_MF_SHIFT);
    face->ccb_PRE0 = PRE0_BGND | PRE0_BPP_4;
    face->ccb_PRE1 = PRE1_TLLSB_PDC0;
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
/*
typedef struct ShadowVertex
{
    int32 x, y;
} ShadowVertex;

const ShadowVertex vShadow[20] = { { -2, 26 }, { 2, 26 }, { 11, -17 }, { -7, -20 }, { 7, -20 }, { -4, -26 }, { 4, -26 }, { 1, -24 }, { -2, -26 }, { 11, -20 }, { 7, -22 }, { -11, -20 }, { -7, -22 }, { -6, -7 }, { 6, -7 }, { -11, -17 }, { -6, -20 }, { 6, -20 }, { 2, -26 }, { -1, -24 } };
const int32 fShadow[9*4] = { 11, 15, 3, 12, 10, 4, 2, 9, 0, 13, 14, 1, 17, 14, 2, 4, 16, 17, 14, 13, 3, 15, 13, 16, 7, 17, 16, 19, 6, 17, 7, 18, 19, 16, 5, 8 };
*/
void renderShadow(int32 type, int32 intensity, int32 px, int32 py, int32 pz, int32 dx, int32 dy, int32 dirz, int32 rx, int32 ry, int32 rz)
{
    /*
    int32 i, vx, vy, vz, x, y, z, dz, avgZ;
    vec3i* dst = vBuffer;
    const ShadowVertex* v = vShadow;
    const int32* index = fShadow;
    vec3i *p0, *p1, *p2, *p3;

    int32 tx = gMatrixPtr->e03;
    int32 ty = gMatrixPtr->e13;
    int32 tz = gMatrixPtr->e23;

    return;
    for (i = 0; i < 20; i++)
    {
        // shifted down by 4
        vx = px + v->x;
        vy = py;
        vz = pz + v->y;
        v++;

        x = vx;//TODO DP3_XYZ(0) + tx;
        y = vx;//TODO DP3_XYZ(1) + ty;
        z = vx;//TODO DP3_XYZ(2) + tz;

        if (z <= ZNEAR)
        {
            z = ZNEAR;
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

    for (i = 0; i < 9; i++)
    {
        int32 i0 = *index++;
        int32 i1 = *index++;
        int32 i2 = *index++;
        int32 i3 = *index++;

        p0 = vBuffer + i0;
        p1 = vBuffer + i1;
        p2 = vBuffer + i2;
        p3 = vBuffer + i3;

        if (p0->z <= ZNEAR || p1->z <= ZNEAR || p2->z <= ZNEAR || p3->z <= ZNEAR)
            return;

        avgZ = (p0->z + p1->z + p2->z + p3->z) >> 2;

        ccbMap4_shadow(faceAdd(avgZ >> OT_SHIFT), p0, p1, p2, p3);
    }
    */
}

void renderColorQuad(const ColorQuad* q) {}
void renderBillboard(const Billboard* b) {}

void renderBackFill(int x, int y, int w, int h)
{
    // TODO perf compare with 16-bpp
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

void renderFrameBegin(void)
{
    dbg_render = osGetSystemTimeMS();
    gFacesBase = gFaces;

    dbg_v = 0;
}

void renderFrameEnd(void)
{
    FaceCEL* facesHead = NULL;
    FaceCEL* facesTail = NULL;
    ListOT* list = gOT + OT_SIZE - 1;
    int32 i;

    if (gFaces != gFacesBase)
    {
        dbg_f = gFacesBase - gFaces;

        for (i = OT_SIZE - 1; i >= 0; i--, list--)
        {
            if (!list->tail) continue;

            if (facesTail)
            {
                facesTail->ccb_NextPtr = list->head;
            }
            else
            {
                facesHead = list->head;
            }

            facesTail = list->tail;
            list->tail = NULL;
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

void vertexTrans(const vec3i* src, int32 start, int32 count)
{
    dbg_v += count;
    MulManyVec3Mat33_F16((vec3f16*)vBuffer + start, (vec3f16*)src + start, *(mat33f16*)gMatrixPtr, count);
}

#define vertexProj vertexProj_asm
void vertexProj_asm(const vec3i* src, vec2p* dst, int32 count);
void vertexProj_c(const vec3i* src, vec2p* dst, int32 count)
{
    const vec2p* last = dst + count;
    int32 x, y, z, dz, clip;

    int32 tx = gMatrixPtr->e03;
    int32 ty = gMatrixPtr->e13;
    int32 tz = gMatrixPtr->e23;

    while (dst < last)
    {
        x = src->x;
        y = src->y;
        z = src->z;
        src++;

        x += tx;
        y += ty;
        z += tz;

        clip = 0;

        if (z < Z_NEAR)
        {
            z = Z_NEAR;
            clip = 1;
        }

        if (z > Z_FAR)
        {
            z = Z_FAR;
            clip = 1;
        }

        z >>= 3;

        dz = FixedInvU(z);

        x *= dz;
        y *= dz;

        x >>= (12 - SUBPX_SHIFT);
        y >>= (12 - SUBPX_SHIFT);

        if (x < -VP_W || x > VP_W || y < -VP_H || y > VP_H)
        {
            clip = 1;
        }

    #if 1
        if (x < -GUARDBAND - 1) x = -GUARDBAND - 1;
        if (y < -GUARDBAND - 1) y = -GUARDBAND - 1;
        if (x >  GUARDBAND) x =  GUARDBAND;
        if (y >  GUARDBAND) y =  GUARDBAND;
    #else
        if (x < -VP_W) x = -VP_W;
        if (y < -VP_H) y = -VP_H;
        if (x >  VP_W) x =  VP_W;
        if (y >  VP_H) y =  VP_H;
    #endif

        dst->xy = (x << 16) | (uint16)y;
        dst->zp = (z << 16) | clip;
        dst++;
    }
}

void renderSectionsMed(const TrackRange* ranges)
{
    const Face* trackFace;
    vec3i *v, *v0, *v1, *v2, *v3, *v01, *v12, *v23, *v30, *vM;
    vec2p *p, *p0, *p1, *p2, *p3, *p01, *p12, *p23, *p30, *pM;
    int32 i, j;
    uint32 z0, z1, z2, z3, maxZ;

    for (j = 0; j < 2; j++, ranges++)
    {
        if (!ranges->fCount)
            break;

        vertexTrans(gTrack.vertices, ranges->vStart, ranges->vCount);

        v = vBuffer + ranges->vStart + ranges->vCount;
        trackFace = gTrack.faces + ranges->fStart;
        for (i = 0; i < ranges->fCount; i++, trackFace++, v += 5)
        {
            uint32 index01 = ((uint32*)trackFace->vertex)[0];
            uint32 index23 = ((uint32*)trackFace->vertex)[1];

            v0 = vBuffer + (index01 >> 16);
            v1 = vBuffer + (index01 << 16 >> 16);
            v2 = vBuffer + (index23 >> 16);
            v3 = vBuffer + (index23 << 16 >> 16);

            v01 = v + 0;
            v12 = v + 1;
            v23 = v + 2;
            v30 = v + 3;
            vM = v + 4;

            AVG(v01, v0, v1);
            AVG(v12, v1, v2);
            AVG(v23, v2, v3);
            AVG(v30, v3, v0);
            AVG(vM, v01, v23);
        }
        
        vertexProj(vBuffer + ranges->vStart, pBuffer + ranges->vStart, ranges->vCount + ranges->fCount * 5);

        p = pBuffer + ranges->vStart + ranges->vCount;
        trackFace = gTrack.faces + ranges->fStart;
        for (i = 0; i < ranges->fCount; i++, trackFace++, p += 5)
        {
            uint32 index01 = ((uint32*)trackFace->vertex)[0];
            uint32 index23 = ((uint32*)trackFace->vertex)[1];

            p0 = pBuffer + (index01 >> 16);
            p1 = pBuffer + (index01 << 16 >> 16);
            p2 = pBuffer + (index23 >> 16);
            p3 = pBuffer + (index23 << 16 >> 16);

            z0 = p0->zp;
            z1 = p1->zp;
            z2 = p2->zp;
            z3 = p3->zp;

            if ((z0 & z1 & z2 & z3) & 1) // check for clip flag
                continue;

            //if (cross(p0, p1, p3) > 0)
            //    continue;

            maxZ = z0;
            if (maxZ < z1) maxZ = z1;
            if (maxZ < z2) maxZ = z2;
            if (maxZ < z3) maxZ = z3;
            maxZ >>= (16 + OT_SHIFT);

            p01 = p + 0;
            p12 = p + 1;
            p23 = p + 2;
            p30 = p + 3;
            pM = p + 4;

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
    vec2p *p0, *p1, *p2, *p3;
    FaceCEL* face;
    int32 i, j;
    uint32 maxZ;
    const Texture* texture;

    for (j = 0; j < 2; j++, ranges++)
    {
        if (!ranges->fCount)
            break;

        vertexTrans(gTrack.vertices, ranges->vStart, ranges->vCount);
        vertexProj(vBuffer + ranges->vStart, pBuffer + ranges->vStart, ranges->vCount);

        trackFace = gTrack.faces + ranges->fStart;

        for (i = 0; i < ranges->fCount; i++, trackFace++)
        {
            uint32 index01 = ((uint32*)trackFace->vertex)[0];
            uint32 index23 = ((uint32*)trackFace->vertex)[1];

            p0 = pBuffer + (index01 >> 16);
            p1 = pBuffer + (index01 << 16 >> 16);
            p2 = pBuffer + (index23 >> 16);
            p3 = pBuffer + (index23 << 16 >> 16);

            //if (cross(p0, p1, p3) > 0)
            //    continue;

            maxZ = p0->zp >> (16 + OT_SHIFT);
            //if (maxZ < p1->vz) maxZ = p1->vz;
            //if (maxZ < p2->vz) maxZ = p2->vz;
            //if (maxZ < p3->vz) maxZ = p3->vz;

            face = faceAdd(maxZ);
        
            texture = gTextures + ctrltexTemplate[trackFace->texture].tex1[0] + ctrllibraryTextures;

            // TODO pre-compute flipped indices
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
    // TODO pre-shift track coords?
    gMatrixPtr->e00 <<= 4;
    gMatrixPtr->e10 <<= 4;
    gMatrixPtr->e20 <<= 4;
    gMatrixPtr->e01 <<= 4;
    gMatrixPtr->e11 <<= 4;
    gMatrixPtr->e21 <<= 4;
    gMatrixPtr->e02 <<= 4;
    gMatrixPtr->e12 <<= 4;
    gMatrixPtr->e22 <<= 4;

    renderSectionsMed(gCamera.section->visRange[0]);
    yOffset += (1 << 15); // HACK to hide gaps between LODs (precision problem)
    renderSectionsLow(gCamera.section->visRange[1]);
    yOffset -= (1 << 15);

    gMatrixPtr->e00 >>= 4;
    gMatrixPtr->e10 >>= 4;
    gMatrixPtr->e20 >>= 4;
    gMatrixPtr->e01 >>= 4;
    gMatrixPtr->e11 >>= 4;
    gMatrixPtr->e21 >>= 4;
    gMatrixPtr->e02 >>= 4;
    gMatrixPtr->e12 >>= 4;
    gMatrixPtr->e22 >>= 4;
}

void renderEffects(const Effect* fx)
{
    // TODO
}

void renderMesh(const Object* obj)
{
    int32 i;
    const ObjFace* f = obj->faces;

    vertexTrans(obj->vertices, 0, obj->vCount);
    vertexProj(vBuffer, pBuffer, obj->vCount);

    for (i = 0; i < obj->fCount; i++, f++)
    {
        uint32 texIndex = FACE_TEXTURE(f);

        if (texIndex == NO_TEXTURE)
        {
            texIndex = 0;
        }

        ccbMap4_obj(pBuffer, f->index, gTextures + texIndex);
    }
}

void renderObject(const Object* object)
{
    renderMesh(object);
}

void renderShip(const Object* object, uint32 color)
{
    renderMesh(object);
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
