#include "pad.h"

#define SHIP_LOD_DIST 8000

#define INTERACTIVE 0
#define ATTRACT     1

#define DRTEXT_16   0
#define DRTEXT_12   1
#define DRTEXT_8    2
#define DRTEXT_RED  4

#define TEXT_COLOR_WHITE        0xFFFFFFFF
#define TEXT_COLOR_HIGHLIGHT    0xFF32CCFF

#ifdef __3DO__
#define BLINK_TEXT (((gFrameIndex & 3) <= 1) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT) // ok for 15 fps :)
#else
#define BLINK_TEXT ((gFrameIndex & 1) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT)
#endif

#define BLINK_SLOW ((gFrameIndex & 31) <= 21)

#define CAMERA_FOV          (75 << 12)
#define CAMERA_FOV_EXTRA    (100 << 12)

extern int32 gWidth, gHeight;
extern int32 guiL, guiT, guiR, guiB;

extern int HACK_LOAD_TRACK;
extern int HACK_TEX_FILTER;

#define SCREEN_Z 160

extern TrackData gTrack;
extern int32 gNumPlayers;
extern PlayerData gPlayers[MAX_PLAYERS];

extern int ownShip;
extern int NoOfMainSections;

extern char NumTracks;
extern char totalTracks;
extern int attractDelay;

extern int tempLapTimes[];

extern Object* startBoom[];
extern short startBoomCount;

#define MAX_TEXTURES 512
extern Texture gTextures[];
extern Texture* gTexturePtr;

#define MAX_SPRITES_PER_OBJECT 64
#define MAX_OBJECTS 512
extern Object gObjects[];
extern Object* gObjectPtr;

/* spline stuff */
extern Object* camSpline[];
extern Object* shipSpline[];
extern short finishedExtro;

extern ConfigData gConfig;

extern int32 ctrlmode;
extern int32 ctrlGameLoop;

extern Object* ctrlshipShapes;
extern ShipData ctrlshipIndex[];

extern Object* gWeaponShapes;

extern int32 gMusicTracksCount;
extern int32 gMusicTrack;
extern int32 gMasterBalance;

extern int32 gFrameIndex;
extern int32 gLastFrameIndex;
extern int32 gFrameDelta;

extern int32 gEnableFrameLerp;

#ifdef USE_FRAME_LERP
    #define FRAME_LERP_INIT(obj)\
        if (gEnableFrameLerp) {\
            (obj)->_pos.x = (obj)->pos.x;\
            (obj)->_pos.y = (obj)->pos.y;\
            (obj)->_pos.z = (obj)->pos.z;\
            (obj)->_pitch = (obj)->pitch;\
            (obj)->_hdg = (obj)->hdg;\
            (obj)->_roll = (obj)->roll;\
        }

    #define FRAME_LERP(obj)\
        if (gEnableFrameLerp) {\
            px = FRAME_LERP_POS((obj)->_pos.x, px);\
            py = FRAME_LERP_POS((obj)->_pos.y, py);\
            pz = FRAME_LERP_POS((obj)->_pos.z, pz);\
            rx = FRAME_LERP_ROT((obj)->_pitch, rx);\
            ry = FRAME_LERP_ROT((obj)->_hdg, ry);\
            rz = FRAME_LERP_ROT((obj)->_roll, rz);\
        }

    #define FRAME_LERP_VALUE(a, delta)  ((a) + ((delta) * gFrameDelta >> 8))
    #define FRAME_LERP_POS(a, b)        FRAME_LERP_VALUE(a, (b) - (a))
    #define FRAME_LERP_ROT(a, b)        FRAME_LERP_VALUE(a, ang((b) - (a)))
    #define FRAME_LERP_STEP(step)       FRAME_LERP_VALUE(gFrameIndex * step, step)
#else
    #define FRAME_LERP_INIT(obj)
    #define FRAME_LERP(obj)
    #define FRAME_LERP_STEP(step)   (gFrameIndex * step)
    #define FRAME_LERP_POS(a, b)    b
    #define FRAME_LERP_ROT(a, b)    b
#endif

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

extern const char gTrackList[14];

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
extern int32 dbg_v;
extern int32 dbg_f;
