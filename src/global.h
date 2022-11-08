#include "pad.h"

#define SHIP_VIS_DIST 16000
#define SHIP_LOD_DIST 8000

#define INTERACTIVE 0
#define ATTRACT     1

#define DRTEXT_16   0
#define DRTEXT_12   1
#define DRTEXT_8    2
#define DRTEXT_RED  4

#define TEXT_COLOR_WHITE        0xFF808080
#define TEXT_COLOR_HIGHLIGHT    0xFF19668C

#define BLINK_TEXT (((gFrameIndex & 3) <= 1) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT)
#define BLINK_SLOW ((gFrameIndex & 63) <= 42)

#define CAMERA_FOV          (75 << 12)
#define CAMERA_FOV_EXTRA    (100 << 12)

extern int gWidth;
extern int gHeight;
extern int gBorder;
extern int gCameraFOV;
extern int gTargetFOV;

extern int HACK_LOAD_TRACK;
extern int HACK_LOAD_SHADOW;
extern int HACK_TEX_FILTER;

#define SCREEN_Z 160

extern TrackData* track;
extern int ownShip;
extern void (*CameraUpdate)(TrackCamera*, ShipData*, RescueData*);
extern TrackCamera* cameraPtr;
extern int NoOfMainSections;
extern uint16 pad;
extern uint16 opad, xpad;
extern PadInfo pCon;

extern int trackNo;
extern char NumTracks;
extern char totalTracks;
extern int attractDelay;
extern int raceType;

extern int tempLapTimes[5];

extern Object* startBoom[4];
extern short startBoomCount;
extern int crossedLine;

#define MAX_TEXTURES 512
extern Texture gTextures[MAX_TEXTURES];
extern Texture* gTexturePtr;

extern Object* objectTable[ObjectTableMax];

/* spline stuff */
extern Object* camSpline[];
extern Object* shipSpline[];
extern short finishedExtro;

extern Skeleton gCamPos;
extern Skeleton gCamDir;

extern short trackNum;
extern int32 gMusicTracksCount;
extern int32 gMusicTrack;
extern int32 gMasterBalance;

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

extern const char gTrackList[15];

#define DRAM_BLOCK_SIZE (32 * 1024)
#define VRAM_BLOCK_SIZE (16 * 1024)

#define DRAM_SIZE (33 * DRAM_BLOCK_SIZE)
#define VRAM_SIZE (44 * VRAM_BLOCK_SIZE)

extern char* gDRAM;
extern char* gVRAM;
extern char* pDRAM;
extern char* pVRAM;
char* allocDRAM(int size);
char* allocVRAM(int size);

void memStateSave(void);
void memStateRestore(void);
