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

const char gTrackList[15] = { 2, 4, 1, 12, 8, 9, 10, 3, 5, 6, 7, 11, 13, 14, 15 };
