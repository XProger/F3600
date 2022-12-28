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

#ifdef __3DO__
#define BLINK_TEXT (((gFrameIndex & 7) <= 3) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT) // ok for 15 fps :)
#else
#define BLINK_TEXT (((gFrameIndex & 3) <= 1) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT)
#endif

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

extern TrackData gTrack;
extern TrackCamera gCamera;

extern int ownShip;
extern void (*CameraUpdate)(TrackCamera*, ShipData*, RescueData*);
extern int NoOfMainSections;
extern uint16 pad;
extern uint16 opad, xpad;
extern PadInfo pCon;

extern int trackNo;
extern char NumTracks;
extern char totalTracks;
extern int attractDelay;

extern int tempLapTimes[5];

extern Object* startBoom[4];
extern short startBoomCount;
extern int crossedLine;

#define MAX_TEXTURES 512
extern Texture gTextures[MAX_TEXTURES];
extern Texture* gTexturePtr;

/* spline stuff */
extern Object* camSpline[];
extern Object* shipSpline[];
extern short finishedExtro;

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

#define DRAM_SIZE (32 * DRAM_BLOCK_SIZE)
#define VRAM_SIZE (44 * VRAM_BLOCK_SIZE)

extern uint8* gDRAM;
extern uint8* gVRAM;
extern uint8* pDRAM;
extern uint8* pVRAM;
uint8* allocDRAM(int32 size);
uint8* allocVRAM(int32 size);

void memStateSave(void);
void memStateRestore(void);

void getTrackStrNo(char* trackStr, char trackNo);
void getPath(char* trackPath, char trackNo, char* name);

extern int32 dbg_update;
extern int32 dbg_render;
extern int32 dbg_frameTime;
extern int32 dbg_update;
extern int32 dbg_v;
extern int32 dbg_f;
