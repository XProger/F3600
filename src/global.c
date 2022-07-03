#include "standard.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "droid.h"
#include "camera.h"
#include "scene.h"
#include "sound.h"
#include "global.h"

int ownShip;
int serialShip;
char gameType;
short DRTEXT_16;
short DRTEXT_12;
short DRTEXT_8;
short DRTEXT_RED;
void (*CameraUpdate)(TrackCamera*, ShipData*, RescueData*);
TrackCamera* cameraPtr;
int NoOfMainSections;

ushort pad;
ushort opad, xpad;
PadInfo pCon;

/*** Sound Variables required for libSpu ***/
ChanTable chanTable[24];
VagTable vagTable[42];
short sampleLevel;

char trackPath[64];
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

short numTex;

Texture* TextureTable[TextureTableMax];
short TextureTableCount = 0;

short textureX;
short textureY;

short clutX;
short clutY;

Object** objectTable;

/* spline stuff */
Object* camSpline[20];
Object* shipSpline[20];
short finishedExtro;
short extroLevel;

long DrawnShieldBefore;

short WinWidth;
short WinHeight;
