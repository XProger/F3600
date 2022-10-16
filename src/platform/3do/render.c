#include "game.h"

extern int32 screenPage;
extern ScreenContext screen;
extern TrackCamera ctrlcamera;
extern TexTemplate* ctrltexTemplate;
extern short ctrllibraryTextures;

Item irqVBL;
Item irqVRAM;

Item screenItem;
int32 screenPage;

int32 xOffset;
int32 yOffset;

int gWidth, gHeight, gBorder;
int gCylX, gCylZ;

#define MAX_CEL_FACES       1024
#define MAX_CEL_TEXTURES    512

#define FRAME_WIDTH         320
#define FRAME_HEIGHT        240

#define MAX_VERTICES_BUFFER 96

VECTOR vBuffer[MAX_VERTICES_BUFFER];

#define OT_SHIFT    4
#define OT_SIZE     4096

typedef struct CCB FaceCEL;

typedef struct {
    FaceCEL* head;
    FaceCEL* tail;
} ListOT;

ListOT gOT[OT_SIZE];

typedef struct
{
    Texture tex;
    uint16* plut;
    uint8* data;
} TextureCEL;

FaceCEL gFaces[MAX_CEL_FACES];
FaceCEL* gFacesBase;

TextureCEL gTextures[MAX_CEL_TEXTURES];
TextureCEL* gTextureBase = gTextures;

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
        0x6663, 0x6A83, 0x72C3, 0x7B03 };

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
    xOffset = (FRAME_WIDTH >> 1) + x;
    yOffset = (FRAME_HEIGHT >> 1) + y;
}

void renderClear(void) {}

#define RGB16(r, g, b) (r | (g << 5) | (b << 10))

const uint16 gPalette[] = 
{
    RGB16(31, 0, 0),
    RGB16(0, 31, 0),
    RGB16(0, 0, 31),
    RGB16(31, 0, 31),
    RGB16(31, 31, 0),
    RGB16(0, 31, 31),
    RGB16(31, 31, 31),
    RGB16(15, 15, 15),
// dummy
    RGB16(15, 15, 15),
    RGB16(15, 15, 15),
    RGB16(15, 15, 15),
    RGB16(15, 15, 15)
};

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
    TextureCEL* t = (TextureCEL*)g->texture;

    if (!t->plut)
        return; // TODO temporary for 8-bpp and 16-bpp

    woffset = ((t->tex.width << 2) + 31) >> 5;
    skipx = g->x & 7;

    face = faceAdd(0);
    face->ccb_SourcePtr = (CelData*)((uint32*)t->data + (g->x >> 3) + g->y * woffset);
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
    // Texture must be 4-bpp >= 16 px width
    int32 woffset;
    FaceCEL* face;

    if (!((TextureCEL*)t)->plut)
        return; // TODO temporary for 8-bpp and 16-bpp

    woffset = ((t->width << 2) + 31) >> 5;

    face = faceAdd(0);
    face->ccb_SourcePtr = (CelData*)(((TextureCEL*)t)->data);
    face->ccb_PLUTPtr = ((TextureCEL*)t)->plut;
    face->ccb_PRE0 = ((t->height - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    face->ccb_PRE1 = ((t->width - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);

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

void renderBackground(const Texture* t)
{
    // Texture must be 320x240 16-bpp
    // TODO use SPORT
    FaceCEL* face = faceAdd(OT_SIZE - 1);
    face->ccb_SourcePtr = (CelData*)(((TextureCEL*)t)->data);
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
void renderBackFill(int x, int y, int w, int h) {}

void renderPrepare(void) {}

void renderFrameBegin(void)
{
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

        if (facesHead)
        {
            LAST_CEL(facesTail);
            DrawScreenCels(screenItem, (CCB*)facesHead);
            UNLAST_CEL(facesTail);
        }
    }

    gFacesBase = gFaces;
}

MATRIX* gMatrix;
int cx, cy, cz;

#define DP3(row, v) ((gMatrix->m[row][0] * v->vx + gMatrix->m[row][1] * v->vy + gMatrix->m[row][2] * v->vz) >> 12)

#define TRANSFORM(p, v) {\
        p.vx = DP3(0, v) + cx;\
        p.vy = DP3(1, v) + cy;\
        p.vz = DP3(2, v) + cz;\
    }

#define PROJECT(p) {\
        int32 z = p.vz >> 3;\
        if (z >= DIV_TABLE_SIZE) z = DIV_TABLE_SIZE - 1;\
        z = FixedInvU(z);\
        p.vx = (p.vx * z) >> 12;\
        p.vy = (p.vy * z) >> 12;\
    }

#define AVG(p, a, b) {\
        p.vx = (a.vx + b.vx) >> 1;\
        p.vy = (a.vy + b.vy) >> 1;\
        p.vz = (a.vz + b.vz) >> 1;\
    }

void ccbMap4(FaceCEL* f, const VECTOR* v0, const VECTOR* v1, const VECTOR* v2, const VECTOR* v3, uint32 texIndex)
{
    int32 x0 = v0->vx;
    int32 y0 = v0->vy;
    int32 x1 = v1->vx;
    int32 x2 = v2->vx;
    int32 y2 = v2->vy;
    int32 y1 = v1->vy;
    int32 x3 = v3->vx;
    int32 y3 = v3->vy;

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

    const TextureCEL* texture = (TextureCEL*)TextureTable[texIndex];

    hs = 16 - hs;
    hddx = (hdx1 - hdx0) >> hs;
    hddy = (hdy1 - hdy0) >> hs;

    f->ccb_XPos = (x0 + xOffset) << 16;
    f->ccb_YPos = (y0 + yOffset) << 16;
    f->ccb_HDX = hdx0;
    f->ccb_HDY = hdy0;
    f->ccb_VDX = vdx0;
    f->ccb_VDY = vdy0;
    f->ccb_HDDX = hddx;
    f->ccb_HDDY = hddy;

    f->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;

    f->ccb_Flags =
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

    f->ccb_PRE0 = ((32 - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    f->ccb_PRE1 = ((32 - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
    f->ccb_SourcePtr = (CelData*)texture->data;
    f->ccb_PLUTPtr = texture->plut;
}

void ccbMap3(FaceCEL* f, const VECTOR* v0, const VECTOR* v1, const VECTOR* v2, uint32 texIndex)
{
    int32 x0 = v0->vx;
    int32 y0 = v0->vy;
    int32 x1 = v1->vx;
    int32 y1 = v1->vy;
    int32 x2 = v2->vx;
    int32 y2 = v2->vy;

    uint32 ws = 20 - 5;
    uint32 hs = 16 - 5;

    int32 hdx0 = (x1 - x0) << ws;
    int32 hdy0 = (y1 - y0) << ws;
    int32 hdx1 = (x2 - x0) << ws;
    int32 hdy1 = (y2 - y0) << ws;
    int32 hddx;
    int32 hddy;
    uint32 woffset = ((32 << 2) + 31) >> 5;

    const TextureCEL* texture = (TextureCEL*)TextureTable[texIndex];

    hs = 16 - hs;
    hddx = (hdx1 - hdx0) >> hs;
    hddy = (hdy1 - hdy0) >> hs;

    f->ccb_XPos = (x0 + xOffset) << 16;
    f->ccb_YPos = (y0 + yOffset) << 16;
    f->ccb_HDX = hdx0;
    f->ccb_HDY = hdy0;
    f->ccb_VDX = 0;
    f->ccb_VDY = 0;
    f->ccb_HDDX = hddx;
    f->ccb_HDDY = hddy;

    f->ccb_PIXC = PPMPC_MF_8 | PPMPC_SF_8;

    f->ccb_Flags =
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

    f->ccb_PRE0 = ((32 - PRE0_VCNT_PREFETCH) << PRE0_VCNT_SHIFT) | PRE0_BPP_4;
    f->ccb_PRE1 = ((32 - PRE1_TLHPCNT_PREFETCH) << PRE1_TLHPCNT_SHIFT) | PRE1_TLLSB_PDC0 | ((woffset - PRE1_WOFFSET_PREFETCH) << PRE1_WOFFSET8_SHIFT);
    f->ccb_SourcePtr = (CelData*)texture->data;
    f->ccb_PLUTPtr = texture->plut;
}

void renderSetSkeleton(Skeleton* skeleton, int x, int y, int z)
{
    gMatrix = GetSkeletonMatrix(skeleton);
    cx = x;
    cy = y;
    cz = z;
}

#define ZNEAR 128

void renderSectionLow(const TrackSection* section, uint32 color)
{
    const Face* trackFace = track->faces + section->faceStart;
    const VECTOR* vertices = track->vertices;
    const VECTOR *v0, *v1, *v2, *v3;
    VECTOR p0, p1, p2, p3;
    FaceCEL* face;
    int i, maxZ;

    for (i = 0; i < section->faceCount; i++, trackFace++)
    {
        v0 = vertices + trackFace->vertex[0];
        v1 = vertices + trackFace->vertex[1];
        v2 = vertices + trackFace->vertex[2];
        v3 = vertices + trackFace->vertex[3];

        TRANSFORM(p0, v0);
        TRANSFORM(p1, v1);
        TRANSFORM(p2, v2);
        TRANSFORM(p3, v3);

#if 0
        if (p0.vz < ZNEAR) p0.vz = ZNEAR;
        if (p1.vz < ZNEAR) p1.vz = ZNEAR;
        if (p2.vz < ZNEAR) p2.vz = ZNEAR;
        if (p3.vz < ZNEAR) p3.vz = ZNEAR;
#else
        if (p0.vz <= ZNEAR || p1.vz <= ZNEAR || p2.vz <= ZNEAR || p3.vz <= ZNEAR)
            continue;
#endif

        PROJECT(p0);
        PROJECT(p1);
        PROJECT(p2);
        PROJECT(p3);

        maxZ = p0.vz;
        if (maxZ < p1.vz) maxZ = p1.vz;
        if (maxZ < p2.vz) maxZ = p2.vz;
        if (maxZ < p3.vz) maxZ = p3.vz;

        face = faceAdd(maxZ >> OT_SHIFT);
        
        if (trackFace->flags & FLIP_TEXTURE)
        {
            ccbMap4(face, &p0, &p1, &p2, &p3, ctrltexTemplate[trackFace->texture].tex1[0] + ctrllibraryTextures);
        }
        else
        {
            ccbMap4(face, &p1, &p0, &p3, &p2, ctrltexTemplate[trackFace->texture].tex1[0] + ctrllibraryTextures);
        }
    }
}

void renderSectionMed(const TrackSection* section, uint32 color)
{
    const Face* trackFace = track->faces + section->faceStart;
    const VECTOR* vertices = track->vertices;
    const VECTOR *v0, *v1, *v2, *v3;
    VECTOR p0, p1, p2, p3, p01, p12, p23, p30, pM;
    int i, maxZ;
    int mask;

    for (i = 0; i < section->faceCount; i++, trackFace++)
    {
        v0 = vertices + trackFace->vertex[0];
        v1 = vertices + trackFace->vertex[1];
        v2 = vertices + trackFace->vertex[2];
        v3 = vertices + trackFace->vertex[3];

        TRANSFORM(p0, v0);
        TRANSFORM(p1, v1);
        TRANSFORM(p2, v2);
        TRANSFORM(p3, v3);

        mask = 0;

        if (p0.vz <= ZNEAR)
        {
            mask |= 1;
            p0.vz = ZNEAR;
        }

        if (p1.vz <= ZNEAR)
        {
            mask |= 2;
            p1.vz = ZNEAR;
        }

        if (p2.vz <= ZNEAR)
        {
            mask |= 4;
            p2.vz = ZNEAR;
        }

        if (p3.vz <= ZNEAR)
        {
            mask |= 8;
            p3.vz = ZNEAR;
        }

        AVG(p01, p0, p1);
        AVG(p12, p1, p2);
        AVG(p23, p2, p3);
        AVG(p30, p3, p0);
        AVG(pM, p01, p23);

        PROJECT(p0);
        PROJECT(p1);
        PROJECT(p2);
        PROJECT(p3);

        PROJECT(p01);
        PROJECT(p12);
        PROJECT(p23);
        PROJECT(p30);
        PROJECT(pM);
        
        maxZ = p0.vz;
        if (maxZ < p1.vz) maxZ = p1.vz;
        if (maxZ < p2.vz) maxZ = p2.vz;
        if (maxZ < p3.vz) maxZ = p3.vz;

        maxZ >>= OT_SHIFT;

        if (trackFace->flags & FLIP_TEXTURE)
        {
            if (!(mask & 1)) ccbMap4(faceAdd(maxZ), &p0, &p01, &pM, &p30, ctrltexTemplate[trackFace->texture].tex4[0] + ctrllibraryTextures);
            if (!(mask & 2)) ccbMap4(faceAdd(maxZ), &p01, &p1, &p12, &pM, ctrltexTemplate[trackFace->texture].tex4[1] + ctrllibraryTextures);
            if (!(mask & 4)) ccbMap4(faceAdd(maxZ), &pM, &p12, &p2, &p23, ctrltexTemplate[trackFace->texture].tex4[3] + ctrllibraryTextures);
            if (!(mask & 8)) ccbMap4(faceAdd(maxZ), &p30, &pM, &p23, &p3, ctrltexTemplate[trackFace->texture].tex4[2] + ctrllibraryTextures);
        }
        else
        {
            if (!(mask & 1)) ccbMap4(faceAdd(maxZ), &p01, &p0, &p30, &pM, ctrltexTemplate[trackFace->texture].tex4[1] + ctrllibraryTextures);
            if (!(mask & 2)) ccbMap4(faceAdd(maxZ), &p1, &p01, &pM, &p12, ctrltexTemplate[trackFace->texture].tex4[0] + ctrllibraryTextures);
            if (!(mask & 4)) ccbMap4(faceAdd(maxZ), &p12, &pM, &p23, &p2, ctrltexTemplate[trackFace->texture].tex4[2] + ctrllibraryTextures);
            if (!(mask & 8)) ccbMap4(faceAdd(maxZ), &pM, &p30, &p3, &p23, ctrltexTemplate[trackFace->texture].tex4[3] + ctrllibraryTextures);
        }
    }
}

void renderTrack(void)
{
    int i, j, s;
    int dir;
    short* counts;
    const short* const * sections;
    TrackSection* section;
    uint32 color;

    dir = TrackDirection(&ctrlcamera);

    renderSetSkeleton(
        ctrlcamera.camPos->super,
        ctrlcamera.camPos->absolute.t[0],
        ctrlcamera.camPos->absolute.t[1],
        ctrlcamera.camPos->absolute.t[2]);

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

    color = sections[0][0];

    section = track->sections + sections[0][0];
    //renderSectionLow(section->prevSection.ptr, cx, cy, cz, color++);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 2 /* counts[i]*/; j += 2) // TODO better visibility indexing
        {
            section = track->sections + sections[i][j];

            for (s = 0; s < sections[i][j + 1]; s++)
            {
                if (i == 0)
                {
                    renderSectionMed(section, color++);
                }
                else
                {
                    renderSectionLow(section, color++);
                }
                section = section->nextSection.ptr;
            }
        }
    }
}


#define DP3_XYZ(row) ((gMatrix->m[row][0] * x + gMatrix->m[row][1] * y + gMatrix->m[row][2] * z) >> 12)

#define PROJECT(p) {\
        int32 z = p.vz >> 3;\
        if (z >= DIV_TABLE_SIZE) z = DIV_TABLE_SIZE - 1;\
        z = FixedInvU(z);\
        p.vx = (p.vx * z) >> 12;\
        p.vy = (p.vy * z) >> 12;\
    }

void transformMesh(const SVECTOR* vertices, int32 count)
{
    int32 i;
    VECTOR* dst = vBuffer;
    const SVECTOR* end = vertices + count;
    int32 mx = cx;
    int32 my = cy;
    int32 mz = cz;

    while (vertices < end)
    {
        int32 xy = ((uint32*)vertices)[0];
        int32 zp = ((uint32*)vertices)[1];

        int32 x = (xy >> 16);
        int32 y = (xy << 16) >> 16;
        int32 z = (zp >> 16);

        int32 px = DP3_XYZ(0) + mx;
        int32 py = DP3_XYZ(1) + my;
        int32 pz = DP3_XYZ(2) + mz;

        if (pz <= 0)
        {
            pz = 0;
        }

        z = pz >> 3;
        if (z >= DIV_TABLE_SIZE)
        {
            z = DIV_TABLE_SIZE - 1;
        }
        z = FixedInvU(z);

        dst->vx = (px * z) >> 12;
        dst->vy = (py * z) >> 12;
        dst->vz = pz;
        dst++;

        vertices++;
    }
}

int32 cross(const VECTOR *a, const VECTOR *b, const VECTOR *c)
{
    return (b->vx - a->vx) * (c->vy - a->vy) -
           (c->vx - a->vx) * (b->vy - a->vy);
}

void renderQuadFace(const int16* indices, int32 texIndex)
{
    const VECTOR *p0, *p1, *p2, *p3;
    int32 avgZ;

    p0 = vBuffer + indices[0];
    p1 = vBuffer + indices[1];
    p2 = vBuffer + indices[2];
    p3 = vBuffer + indices[3];

    if (p0->vz <= ZNEAR || p1->vz <= ZNEAR || p2->vz <= ZNEAR || p3->vz <= ZNEAR)
        return;

    if (cross(p0, p1, p3) <= 0)
        return;

    avgZ = (p0->vz + p1->vz + p2->vz + p3->vz) >> 2;

    ccbMap4(faceAdd(avgZ >> OT_SHIFT), p0, p1, p3, p2, texIndex);
}

void renderTriFace(const int16* indices, int32 texIndex)
{
    const VECTOR *p0, *p1, *p2;
    int32 avgZ;

    p0 = vBuffer + indices[0];
    p1 = vBuffer + indices[1];
    p2 = vBuffer + indices[2];

    if (p0->vz <= ZNEAR || p1->vz <= ZNEAR || p2->vz <= ZNEAR)
        return;

    if (cross(p0, p1, p2) <= 0)
        return;

    avgZ = (p0->vz + p1->vz + p2->vz + p2->vz) >> 2;

    ccbMap3(faceAdd(avgZ >> OT_SHIFT), p0, p1, p2, texIndex);
}

void renderMesh(Primitive* prim, int32 count)
{
    int i;
    Prm poly;
    const VECTOR* v;

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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];
                /*
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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];
                /*
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
                TextureCEL* t = (TextureCEL*)TextureTable[];

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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];
                /*
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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];
                /*
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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];
                /*
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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];

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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];
                /*
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
                TextureCEL* t = (TextureCEL*)TextureTable[f->texture];

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

                v = vBuffer + f->coord;
                sx = gCylX * w >> 12;
                sz = gCylZ * w >> 12;
                /*
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

void renderObject(const Object* object) {}

void renderShip(const Object* object, uint32 color)
{
    const MATRIX* matrix;

    matrix = GetSkeletonMatrix(object->skeleton);

    renderSetSkeleton(object->skeleton, matrix->t[0], matrix->t[1], matrix->t[2]);

    transformMesh(object->vertices, object->vertexCount);

    renderMesh(object->primitives, object->primitiveCount);
}


void renderShield(const Object* object, int32 counter) {}
void renderSky(const Object* object) {}
void renderSetMode(int renderMode) {}
void renderSetBlend(int blend) {}
void renderDepthWrite(int enable) {}
void renderNextLayer(void) {}

Texture* renderCreateTexture(Tim* tim)
{
    int32 i;
    TextureCEL* tex = gTextureBase++;

    tim->header.flags = SwapLE32(tim->header.flags);

    if (TimType(tim) == TIM_4)
    {
        uint8* src;

        Tim4* t = &tim->tim4;
        tex->tex.width = SwapLE16(t->textureW) * 4;
        tex->tex.height = SwapLE16(t->textureH);
        tex->plut = t->clut;
        tex->data = (uint8*)(t + 1);

        for (i = 0; i < 16; i++)
        {
            uint16 p = SwapLE16(tex->plut[i]);
            uint8 r = p & 31;
            uint8 g = (p >> 5) & 31;
            uint8 b = (p >> 10) & 31;
            tex->plut[i] = b | (g << 5) | (r << 10);
        }

        src = tex->data;
        for (i = 0; i < tex->tex.width * tex->tex.height / 2; i++)
        {
            uint8 p = *src;
            *src++ = ((p & 15) << 4) | (p >> 4);
        }
    }
    else if (TimType(tim) == TIM_8)
    {
        Tim8* t = &tim->tim8;
        tex->tex.width = SwapLE16(t->textureW) * 2;
        tex->tex.height = SwapLE16(t->textureH);
        tex->plut = NULL;//t->clut;
        tex->data = NULL;//(uint8*)(t + 1);
    }
    else if (TimType(tim) == TIM_16)
    {
        uint16* src;

        Tim16* t = &tim->tim16;
        tex->tex.width = SwapLE16(t->textureW);
        tex->tex.height = SwapLE16(t->textureH);
        tex->plut = NULL;
        tex->data = (uint8*)(t + 1);

        src = (uint16*)tex->data;
        for (i = 0; i < tex->tex.width * tex->tex.height; i++)
        {
            uint16 p = SwapLE16(*src);
            uint8 r = p & 31;
            uint8 g = (p >> 5) & 31;
            uint8 b = (p >> 10) & 31;
            *src++ = b | (g << 5) | (r << 10);
        }
    }

    return &tex->tex;
}

void renderDeleteTexture(Texture* texture)
{
    // nothing to do
}