#include "game.h"

int ownShip;
void (*CameraUpdate)(TrackCamera*, ShipData*, RescueData*);
int NoOfMainSections;

uint16 pad;
uint16 opad, xpad;
PadInfo pCon;

Object* gStartBoom[4];
short gStartBoomCount = 0;
int crossedLine;

TrackData gTrack;
TrackCamera gCamera;

int trackNo;
char NumTracks = 6;
char totalTracks = 13;
int tempLapTimes[5];
int serialLapTimes[5];
int attractDelay;

int gCameraFOV = CAMERA_FOV;
int gTargetFOV = CAMERA_FOV;

Texture gTextures[MAX_TEXTURES];
Texture* gTexturePtr = gTextures;

/* spline stuff */
Object* camSpline[20];
Object* shipSpline[20];
short finishedExtro;

int32 gMusicTracksCount;
int32 gMusicTrack;
int32 gMasterBalance;

int32 dbg_update;
int32 dbg_render;
int32 dbg_frameTime;
int32 dbg_v;
int32 dbg_f;

const char gTrackList[15] = { 2, 4, 1, 12, 8, 9, 10, 3, 5, 6, 7, 11, 13, 14, 15 }; // TODO 15?

char* gDRAM;
char* gVRAM;
char* pDRAM;
char* pVRAM;

char* allocDRAM(int size)
{
    char* ret = pDRAM;
    size = (size + 3) & ~3;
    pDRAM += size;
    ASSERT(pDRAM - gDRAM <= DRAM_SIZE);
#ifdef __3DO__
    printf("D alloc %d (%d kb left)\n", size, (DRAM_SIZE - (pDRAM - gDRAM)) / 1024);
#endif
    return ret;
}

char* allocVRAM(int size)
{
    char* ret = pVRAM;
    size = (size + 3) & ~3;
    pVRAM += size;
    ASSERT(pVRAM - gVRAM <= VRAM_SIZE);
#ifdef __3DO__
    printf("V alloc %d (%d kb left)\n", size, (VRAM_SIZE - (pVRAM - gVRAM)) / 1024);
#endif
    return ret;
}

typedef struct MemState
{
    char* pDRAM;
    char* pVRAM;
    Texture* pTexture;
} MemState;

MemState gMemState;

void memStateSave(void)
{
    gMemState.pDRAM = pDRAM;
    gMemState.pVRAM = pVRAM;
    gMemState.pTexture = gTexturePtr;
}

void memStateRestore(void)
{
    Texture* tex = gMemState.pTexture;
    Texture* end = gTexturePtr;

    while (tex < end)
    {
        renderDeleteTexture(tex);
        tex++;
    }

    pDRAM = gMemState.pDRAM;
    pVRAM = gMemState.pVRAM;
    gTexturePtr = gMemState.pTexture;
}
