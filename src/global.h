#include "pad.h"

#define COMBAT 1
#define CD_AUDIO 0

#define INTERACTIVE 0
#define ATTRACT 1

#define RedText     0
#define WhiteText   1

#define DRTEXT_16   0
#define DRTEXT_12   1
#define DRTEXT_8    2
#define DRTEXT_RED  4
#define DRTEXT_CLUT 10

#define FontTextureTableMax 20
#define TextureTableMax 512

#define BLINK_TEXT (((gFrameIndex & 3) <= 1) ? WhiteText : RedText)
#define BLINK_SLOW ((gFrameIndex & 63) <= 42)

extern TrackData* track;
extern int ownShip;
extern int serialShip;
extern char gameType;
extern void (*CameraUpdate)(TrackCamera*, ShipData*, RescueData*);
extern TrackCamera* cameraPtr;
extern int NoOfMainSections;
extern uint16 pad;
extern uint16 opad, xpad;
extern PadInfo pCon;

extern ChanTable chanTable[24];
extern VagTable vagTable[42];
extern short sampleLevel;

extern int trackNo;
extern char NumTracks;
extern char totalTracks;
extern int attractDelay;
extern int raceType;

extern int tempLapTimes[5];
extern int serialLapTimes[5];

extern Object* startBoom[4];
extern short startBoomCount;
extern int crossedLine;

extern Texture* FontTextureTable[];
extern Texture* TextureTable[];
extern short TextureTableCount;
extern Object* objectTable[ObjectTableMax];

/* spline stuff */
extern Object* camSpline[];
extern Object* shipSpline[];
extern short finishedExtro;
extern short extroLevel;

/* fade table references */

#define BLACK_NOT_DRAWN 1
#define BLACK1_TRANSL 2
#define BLACK2_TRANSL 3
#define BLACK3_TRANSL 4
#define BLACK4_TRANSL 5
#define BLACK5_TRANSL 6
#define BLACK6_TRANSL 7
#define WHITE1_TRANSL 8
#define WHITE2_TRANSL 9
#define WHITE3_TRANSL 10
#define WHITE4_TRANSL 11
#define WHITE5_TRANSL 12
#define WHITE6_TRANSL 13
#define WHITE7_TRANSL 14
#define WHITE8_TRANSL 15
#define RED_TRANSL 16
#define BRIGHT_RED_TRANSL 17
#define YELLOW_TRANSL 18
#define YELLOW_WHITE_TRANSL 19
#define BLUE_TRANSL 20
#define PURPLE_TRANSL 21
#define UNUSED1_TRANSL 22
#define UNUSED2_TRANSL 23
#define UNUSED3_TRANSL 24
#define UNUSED4_TRANSL 25

#define BLUE_SPEEDUP 25

#define RED_LIGHT_FADE 16
#define AMBER_LIGHT_FADE 26
#define GREEN_LIGHT_FADE 27

#define frameRate30 30
#define FR30    (frameRate30)
#define FR60    (frameRate30 * 2)
#define FR50    (frameRate30 * 50 / 30)
#define FR6     (frameRate30 * 6 / 30)
#define FR10    (frameRate30 / 3)
#define FR40    (frameRate30 * 40 / 30)
#define FR50U   (50 * 30 / frameRate30)
#define FR15U   (15 * 30 / frameRate30)
#define FR30U   (30 * 30 / frameRate30)
#define FR7U    (7 * 30 / frameRate30)
#define FR32U   (32 * 30 / frameRate30)

extern char trackdir[];
