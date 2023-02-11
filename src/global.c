#include "game.h"

int ownShip;
int NoOfMainSections;

PadInfo pCon;

Object* gStartBoom[4];
short gStartBoomCount = 0;

TrackData gTrack;
int32 gNumPlayers;

PlayerData gPlayers[MAX_PLAYERS];

int trackNo;
char NumTracks = 6;
char totalTracks = 13;
int tempLapTimes[5]; // TODO END_LAP
int attractDelay;

int32 gWidth, gHeight;
int32 guiL, guiT, guiR, guiB;

const char** STR = STR_EN;

Texture gTextures[MAX_TEXTURES];
Texture* gTexturePtr = gTextures;

Object gObjects[MAX_OBJECTS];
Object* gObjectPtr = gObjects;

/* spline stuff */
Object* camSpline[20];
Object* shipSpline[20];
short finishedExtro;

ConfigData gConfig;

int32 ctrlmode;
int32 ctrlGameLoop;

Object* ctrlshipShapes[(NO_OF_SHIPS * 2) + 1];
ShipData ctrlshipIndex[NO_OF_SHIPS];

Object* gWeaponShapes[WEAPON_SHAPE_MAX];

int32 gMusicTracksCount;
int32 gMusicTrack;
int32 gMasterBalance;

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

#ifndef DRAM_ALIGN
    #define DRAM_ALIGN  4
#endif

#ifndef VRAM_ALIGN
    #define VRAM_ALIGN  4
#endif

uint8* allocDRAM(int32 size)
{
    uint8* ret = pDRAM;
    size = (size + (DRAM_ALIGN - 1)) & ~(DRAM_ALIGN - 1);
    pDRAM += size;
    ASSERT(pDRAM - gDRAM <= DRAM_SIZE);
//    printf("D alloc %d (%d kb left)\n", size, (DRAM_SIZE - (pDRAM - gDRAM)) / 1024);
    return ret;
}

uint8* allocVRAM(int32 size)
{
    uint8* ret = pVRAM;
    size = (size + (VRAM_ALIGN - 1)) & ~(VRAM_ALIGN - 1);
    pVRAM += size;
    ASSERT(pVRAM - gVRAM <= VRAM_SIZE);
//    printf("V alloc %d (%d kb left)\n", size, (VRAM_SIZE - (pVRAM - gVRAM)) / 1024);
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
