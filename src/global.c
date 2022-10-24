#include "game.h"

int ownShip;
void (*CameraUpdate)(TrackCamera*, ShipData*, RescueData*);
TrackCamera* cameraPtr;
int NoOfMainSections;

uint16 pad;
uint16 opad, xpad;
PadInfo pCon;

Object* startBoom[4];
short startBoomCount = 0;
int crossedLine;

TrackData* track;
int trackNo;
int raceType;
char NumTracks = 6;
char totalTracks = 13;
int tempLapTimes[5];
int serialLapTimes[5];
int attractDelay;

int gCameraFOV = CAMERA_FOV;
int gTargetFOV = CAMERA_FOV;

Texture* FontTextureTable[FontTextureTableMax];
Texture* TextureTable[TextureTableMax];
short TextureTableCount;
short gBaseTextureIndex;

Object* objectTable[ObjectTableMax];

/* spline stuff */
Object* camSpline[20];
Object* shipSpline[20];
short finishedExtro;
short extroLevel;

Skeleton gCamPos;
Skeleton gCamDir;

short trackNum;
int32 gMusicTracksCount;
int32 gMusicTrack;
int32 gMasterBalance;

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
    printf("D alloc %d (%d kb left)\n", size, (DRAM_SIZE - (pDRAM - gDRAM)) / 1024);
    return ret;
}

char* allocVRAM(int size)
{
    char* ret = pVRAM;
    size = (size + 3) & ~3;
    pVRAM += size;
    ASSERT(pVRAM - gVRAM <= VRAM_SIZE);
    printf("V alloc %d (%d kb left)\n", size, (VRAM_SIZE - (pVRAM - gVRAM)) / 1024);
    return ret;
}
