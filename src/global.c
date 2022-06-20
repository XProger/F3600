#include "standard.h"
#include "malloc.h"
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

int            copyright_Psygnosis_1995 = 1;

int            ownShip;
int            serialShip;
char           gameType;
int            textStart;
int            textPrim;
short          DRTEXT_16;
short          DRTEXT_12;
short          DRTEXT_8;
short          DRTEXT_RED;
// POLY_FT4       display_text[600] ;
SPRT           display_text[600] ;
void           (*CameraUpdate)(TrackCamera *, ShipData*, RescueData*);
TrackCamera    *cameraPtr;
int            NoOfMainSections;
DR_MODE        shieldParams[64 * 3][2];
DR_MODE        dmode;   // PCwipeout
CdlLOC         loc[20];
int            ntoc;
int            actualCdTrack;

ushort         pad;
ushort         opad, xpad;
PadInfo        pCon;


/*** Sound Variables required for libSpu ***/
long           soundHead[ SoundHeapSize ];
SpuCommonAttr  c_attr;
SpuReverbAttr  r_attr;

ChanTable      chanTable[24];
VagTable       vagTable[42];
short          sampleLevel;

char           trackPath[64];
Object*        startBoom[4];
short          startBoomCount = 0;
int            crossedLine;
int            crossedLineSerial;

TrackData*     track;
int            trackNo;
int            raceType;
char           NumTracks = 6 ;
char           totalTracks = 13;
int            tempLapTimes[5];
int            serialLapTimes[5];
int            attractDelay;
int            slowCraft = 0;

long           primCount;
short          numTex;
DynamicHeap*   heap;

Texture*       TextureTable[ TextureTableMax ];
short          TextureTableCount = 0;

P_TAG*         OT[ 2 ];

P_TAG          ot0[ OT_SIZE ];   //PCWipeout
P_TAG          ot1[ OT_SIZE ];   //PCWipeout

short          edDebug;

char           errorString[ 256 ];


short          textureX;
short          textureY;

short          clutX;
short          clutY;


char*          sharedPrims[ MaxUnique ][ MaxObjects ];
short          sharedPrimCount[ MaxUnique ];

long           spareMax = 0;

short          showFaceFlags = 0;



Object         **objectTable;

uchar          PrimitiveBuffer0[ PrimitiveBufferSize ];
uchar          PrimitiveBuffer1[ PrimitiveBufferSize ];
uchar*         PrimitiveBuffer[ 2 ];
StdPoly        prims;


long           SlowMem[ SlowMemSize ];
/* spline stuff */
Object             *camSpline[ 20 ];
Object             *shipSpline[ 20 ];
short          finishedExtro;
short          extroLevel;

unsigned int   this_frame = 0, last_frame = 0;
long        DrawnShieldBefore;

long        TextureTrack = 1;
long        SkyTrackFlags = 3;

unsigned short JoyXVal;
unsigned short JoyYVal;

char        JoyButtons;
int            WinPitch;
short       WinWidth;
short       WinHeight;
short       WinHeightX2;
