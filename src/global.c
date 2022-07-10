#include "game.h"

int ownShip;
int serialShip;
char gameType;
void (*CameraUpdate)(TrackCamera*, ShipData*, RescueData*);
TrackCamera* cameraPtr;
int NoOfMainSections;

uint16 pad;
uint16 opad, xpad;
PadInfo pCon;

/*** Sound Variables required for libSpu ***/
ChanTable chanTable[24];
VagTable vagTable[42];
short sampleLevel;

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
short TextureTableCount = 0;

Object* objectTable[ObjectTableMax];

/* spline stuff */
Object* camSpline[20];
Object* shipSpline[20];
short finishedExtro;
short extroLevel;
