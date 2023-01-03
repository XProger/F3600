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

Object gObjects[MAX_OBJECTS];
Object* gObjectPtr = gObjects;

/* spline stuff */
Object* camSpline[20];
Object* shipSpline[20];
short finishedExtro;

ConfigData gConfig;

Object* ctrlshipShapes[(NO_OF_SHIPS * 2) + 1];
ShipData ctrlshipIndex[NO_OF_SHIPS];
RescueData ctrlrescueDroid;

int32 ctrlNeedTargetIcon;

Object* ctrlweaponShapes[WEAPON_SHAPE_NAX];

int32 gMusicTracksCount;
int32 gMusicTrack;
int32 gMasterBalance;

int32 gPad;
int32 gPadStickX;
int32 gPadStickY;
int32 gLastKey;

int32 gFrameIndex;
int32 gLastFrameIndex;
int32 gFrameDelta;

#ifdef USE_FRAME_LERP
int32 gEnableFrameLerp = 1;
#endif

int32 dbg_update;
int32 dbg_render;
int32 dbg_frameTime;
int32 dbg_v;
int32 dbg_f;

const char gTrackList[15] = { 2, 4, 1, 12, 8, 9, 10, 3, 5, 6, 7, 11, 13, 14, 15 }; // TODO 15?

uint8* gDRAM;
uint8* gVRAM;
uint8* pDRAM;
uint8* pVRAM;

uint8* allocDRAM(int32 size)
{
    uint8* ret = pDRAM;
    size = (size + 3) & ~3;
    pDRAM += size;
    ASSERT(pDRAM - gDRAM <= DRAM_SIZE);
#ifdef __3DO__
    printf("D alloc %d (%d kb left)\n", size, (DRAM_SIZE - (pDRAM - gDRAM)) / 1024);
#endif
    return ret;
}

uint8* allocVRAM(int32 size)
{
    uint8* ret = pVRAM;
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
    uint8* pDRAM;
    uint8* pVRAM;
    Texture* pTexture;
    Object* pObject;
} MemState;

MemState gMemState;

void memStateSave(void)
{
    gMemState.pDRAM = pDRAM;
    gMemState.pVRAM = pVRAM;
    gMemState.pTexture = gTexturePtr;
    gMemState.pObject = gObjectPtr;
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
    gObjectPtr = gMemState.pObject;
}

void getTrackStrNo(char* trackStr, char trackNo)
{
    if (trackNo <= 9)
    {
        trackStr[0] = '0';
        trackStr[1] = '0' + trackNo;
    }
    else
    {
        trackStr[0] = '1';
        trackStr[1] = '0' + (trackNo - 10);
    }
    trackStr[2] = 0;
}

void getPath(char* trackPath, char trackNo, char* name)
{
    char No[3];

    strcpy(trackPath, "wipeout/track");
    getTrackStrNo(No, trackNo);
    strcat(trackPath, No);
    strcat(trackPath, name);
}
