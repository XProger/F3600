#include "standard.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "tilelib.h"
#include "ttf.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "droid.h"
#include "camera.h"
#include "draw.h"
#include "scene.h"
#include "dynam.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "sound.h"
#include "effects.h"
#include "fileops.h"
#include "options.h"
#include "global.h"
#include "cone.h"
#include "race.h"
#include "spline.h"
#include "credits.h"
#include "init.h"
#include "collide.h"

#define START_EXTRO 0
#define UPDATE_EXTRO 1
#define DONE_EXTRO 2

#define PrmMax 64
#define AJSDELAY 60 * 30 // arbitrary number - linked to frame rate ? HOPE SO !
//#define   AJSDELAY     20 // arbitrary number - linked to frame rate ? HOPE SO !

extern long fr;
extern ChampionShip winTable[];
extern short starttrack, endtrack, CurrTrkNo;

extern short startIceText;
extern short startFireText;
int ajscount = 0; // oh shit another global flag

// All race locals now globals, as RaceCtrl has to be re-entrant
TrackData ctrltrackStart;
TexTemplate* ctrltexTemplate;
Object* ctrlprm[PrmMax];
Object* ctrlobj;
short ctrllibraryTextures;
short ctrlprmCount;
long ctrl_i, ctrl_j, ctrl_k;
short ctrlnumLib;
Cone ctrlcone;
long ctrlbefore;
short ctrlcount;
short ctrlweapCount;
TrackSection* ctrlnextSection;

/**************************************************/
/******           Dave additions              *****/
/**************************************************/

TrackCamera ctrlcamera;
Skeleton* ctrlcamDir;

Object* ctrlshipShapes[(NO_OF_SHIPS * 2) + 1];
ShipData* ctrlshipIndex;
RescueData ctrlrescueDroid;

WeaponShape* ctrlweaponShapes;
WeaponData* ctrlweaponIndex;

WeaponGrid* ctrlweaponGrid;

int ctrlnoWeaponGrids;
Texture* ctrlweaponIconTextures[NO_OF_WEAPON_TYPES];

short ctrlsky;
int ctrlchoice;
TrackSection* ctrlsection;
Face* ctrlfacePtr;
VECTOR ctrlfacePoint;
Effect* ctrleffects;
Texture* ctrleffectTextures[NO_OF_EFFECT_TEXTURES];
int ctrlcdTrack;
int ctrlnoCdTracks;
TIMlist* ctrltimPtr;
Shadow* ctrlShadow;
Object* ctrllostad[20]; /* It wouldn`t suprise me ! */
int ctrlstadCount;
short ctrlsectCount;
short ctrlcheck;
short ctrljcheck;
short ctrlji;
TrackSection* ctrljsection;
short ctrlbuttonPressed;
Object* ctrlredLight;
Object* ctrldonkey[2];
int ctrldoneExtro;
int ctrlplayCredits;
int ctrlgameOverTime;
int ctrlcdPause;
int ctrlnearShip;

/**************************************************/
/******           Stu additions               *****/
/**************************************************/
Speedo ctrlspeedo;
int ctrlframe_cnt;
ulong ctrltoggleDisplay;
char ctrlshowMenu;
char ctrlconfirm;
char ctrllConfirm;
HiScoreData ctrlhiScoreData;
char ctrlrec[8];
char ctrlmachinePaused;
char ctrlactivated;
int ctrlchoiceData;
char ctrlquitFlag;
Texture* ctrlwinLose[2];
int ctrlquitTwoPlayer;
char ctrlshowGameOver;

ConfigData* ctrlgameData;
long ctrlmode;
long ctrltimeout;

/*          Andy Y's additions*/
long ctrlNeedEndText;
long ctrlNeedWinLose;
long ctrlNeedBackPoly;
long ctrlNeedRaceInfo;
long ctrlNeedDeSelectScreen;
long ctrlNeedNewHighScores;
long ctrlNeedRacePoints;
long ctrlNeedChampTable;
long ctrlNeedAllVenomComplete;
long ctrlNeedVenomComplete;
long ctrlNeedContinueGameConfirm;
long ctrlNeedEndTextVar;
long ctrlNeedWinLoseVar;
long ctrlNeedInGameReset;
long ctrlNeedInSingleGameConfirm;
long ctrlNeedInGameConfirm;
long ctrlNeedTargetIcon;
long ctrlCDPaused;
long ctrlfirsttime;
extern long CDRestartNeeded;

extern long ctrl_init;
extern long ctrl_says_quit;

char winLoseIn;

void setWinLose(void) { winLoseIn = 1; }

void raceinit(void)
{
    ConfigData* gameData;
    CVECTOR colvector;

    gameData = ctrlgameData;
    if (ctrl_init == 0)
    {
        ctrl_says_quit = 0;
        ctrlprmCount = 0;
        ctrlnumLib = 0;
        ctrlCDPaused = 0;
        ctrlnoWeaponGrids = 0;
        ctrlchoice = -1;
        ctrlcdTrack = 1;
        ctrlcheck = 1;
        ctrljcheck = 1;
        ctrlbuttonPressed = 0;
        ctrlstadCount = 0;
        ctrlplayCredits = 0;
        ctrldoneExtro = START_EXTRO;
        ctrlgameOverTime = 0;
        ctrlcdPause = 0;
        ctrlframe_cnt = 1;
        ctrltoggleDisplay = 0;
        ctrlshowMenu = 1;
        ctrlconfirm = 0;
        ctrllConfirm = 0;
        ctrlmachinePaused = SINGLE;
        ctrlactivated = 0;
        ctrlchoiceData = -1;
        ctrlquitFlag = 0;
        ctrlquitTwoPlayer = 0;
        ctrlshowGameOver = 0;
        ctrlfirsttime = 1;
        finishedExtro = 0;

        /**********************************************************/
        /***                 Initialise Camera                  ***/
        /**********************************************************/

        ctrlcamera.camPos = NewSkeleton();
        SetSkeletonPosition(ctrlcamera.camPos, 0, 0, 0);
        SetSkeletonDirection(ctrlcamera.camPos, 0, 0, 0);
        ctrlcamDir = NewSkeleton();
        SetSkeletonPosition(ctrlcamDir, 0, 0, 0);
        SetSkeletonDirection(ctrlcamDir, 0, 0, 0);
        ctrlcamera.camPos->super = ctrlcamDir;

        cameraPtr = &ctrlcamera;

        track = &ctrltrackStart;

        /**********************************************************/
        /***                 Initialise Track                   ***/
        /**********************************************************/
        //   printf( "Initialise Track\n" );
        getPath(trackPath, gameData->actTrackNum, "\\track.trv");
        InitVertices(trackPath);
        getPath(trackPath, gameData->actTrackNum, "\\track.trf");
        InitFaces(trackPath);
        getPath(trackPath, gameData->actTrackNum, "\\track.trs");
        InitSections(trackPath);
        getPath(trackPath, gameData->actTrackNum, "\\track.vew");
        InitViewList(trackPath);

        extroLevel = 0;

        /* firestar */
        if (gameData->actTrackNum == 14)
        {
            //      printf ( "loading firestar extro\n" );
            extroLevel = 14;

            camSpline[0] = LoadPrm("wipeout\\common\\cam1s.prm", 0);
            camSpline[0]->skeleton->super = ctrlcamera.camPos;

            shipSpline[0] = LoadPrm("wipeout\\common\\shp1s.prm", 0);
            shipSpline[0]->skeleton->super = ctrlcamera.camPos;

            camSpline[1] = LoadPrm("wipeout\\common\\cam2s.prm", 0);
            camSpline[1]->skeleton->super = ctrlcamera.camPos;

            shipSpline[1] = LoadPrm("wipeout\\common\\shp2s.prm", 0);
            shipSpline[1]->skeleton->super = ctrlcamera.camPos;

            camSpline[2] = LoadPrm("wipeout\\common\\cam3s.prm", 0);
            camSpline[2]->skeleton->super = ctrlcamera.camPos;

            shipSpline[2] = LoadPrm("wipeout\\common\\shp3s.prm", 0);
            shipSpline[2]->skeleton->super = ctrlcamera.camPos;

            camSpline[3] = LoadPrm("wipeout\\common\\cam4s.prm", 0);
            camSpline[3]->skeleton->super = ctrlcamera.camPos;

            shipSpline[3] = LoadPrm("wipeout\\common\\shp4s.prm", 0);
            shipSpline[3]->skeleton->super = ctrlcamera.camPos;
        }

        /* silverstream */
        if (gameData->actTrackNum == 13)
        {
            //      printf ( "loading silverstream extro\n" );
            extroLevel = 13;

            camSpline[4] = LoadPrm("wipeout\\common\\cam5s.prm", 0);
            camSpline[4]->skeleton->super = ctrlcamera.camPos;

            shipSpline[4] = LoadPrm("wipeout\\common\\shp5s.prm", 0);
            shipSpline[4]->skeleton->super = ctrlcamera.camPos;

            camSpline[5] = LoadPrm("wipeout\\common\\cam6s.prm", 0);
            camSpline[5]->skeleton->super = ctrlcamera.camPos;

            shipSpline[5] = LoadPrm("wipeout\\common\\shp6s.prm", 0);
            shipSpline[5]->skeleton->super = ctrlcamera.camPos;

            camSpline[6] = LoadPrm("wipeout\\common\\cam7s.prm", 0);
            camSpline[6]->skeleton->super = ctrlcamera.camPos;

            shipSpline[6] = LoadPrm("wipeout\\common\\shp7s.prm", 0);
            shipSpline[6]->skeleton->super = ctrlcamera.camPos;
        }

        //   printf( "Initialise track library Textures\n" );
        ctrllibraryTextures = TextureTableCount;
        getPath(trackPath, gameData->actTrackNum, "\\library.cmp");
        ctrltimPtr = LoadCompressedTextureSequence(trackPath);
        LoadCmpFiles(ctrltimPtr);

        //   printf( "Initialise track library Texture templates\n" );
        getPath(trackPath, gameData->actTrackNum, "\\library.ttf");
        ctrltexTemplate = LoadTtfFile(trackPath, &ctrlnumLib, ctrllibraryTextures);

        ctrlsection = track->sections;

        ctrl_i = 0;
        NoOfMainSections = 0;

        /*** Reserve memory for weapons functions ***/
        ctrlweaponGrid = (WeaponGrid*)malloc((sizeof(WeaponGrid) * 64) + 32);
        ctrlweaponShapes = (WeaponShape*)malloc((sizeof(WeaponShape) * NO_OF_WEAPONS) + 32);
        ctrlweaponIndex = (WeaponData*)malloc((sizeof(WeaponData) * NO_ACTIVE_WEAPONS) + 32);

        while (ctrlcheck)
        {
            ctrlnoWeaponGrids = SetTrackAttributes(ctrlsection, ctrl_i, ctrlweaponGrid, ctrlnoWeaponGrids);
            ctrlsection->secNo = ctrl_i;
            if (ctrlsection->junction.index != -1)
            {
                ctrljsection = ctrlsection;
                ctrlji = ctrl_i;
                ctrlsection = ctrlsection->junction.ptr;
                while (ctrljcheck)
                {
                    ctrl_i++;
                    ctrlnoWeaponGrids = SetTrackAttributes(ctrlsection, ctrl_i, ctrlweaponGrid, ctrlnoWeaponGrids);
                    ctrlsection->secNo = ctrl_i;
                    ctrlsection = ctrlsection->nextSection.ptr;
                    if (ctrlsection->junction.index != -1)
                        ctrljcheck = 0;
                }
                ctrljcheck = 1;
                ctrlsection = ctrljsection;
                ctrl_i = ctrlji;
            }
            ctrl_i++;
            NoOfMainSections++;
            ctrlsection = ctrlsection->nextSection.ptr;
            if (ctrlsection == track->sections)
                ctrlcheck = 0;
        }

        InitSpeedo(&ctrlspeedo);
        InitText();

        ctrlshipIndex = (ShipData*)malloc(((sizeof(ShipData) + 32) * (NO_OF_SHIPS)));
        loadShipData(track->sections, ctrlshipIndex, ctrlshipShapes, ctrlcamera.camPos);
        ctrlsky = InitScene(ctrlprm, &ctrlprmCount, ctrlcamera.camPos, gameData);

        InitStartBoom(ctrlprm, ctrlprmCount);

        // ctrlstadCount = InitCrowdSound(ctrlprm, ctrlprmCount, ctrllostad, ctrlstadCount);

        if ((trackNo == 8) || (trackNo == 11))
        {
            InitRedLight(ctrlprm, ctrlprmCount, &ctrlredLight);
            InitDonkey(ctrlprm, ctrlprmCount, ctrldonkey);
        }
        else if ((trackNo == 1) /* || (trackNo == 6)*/)
        {
            InitRedLight(ctrlprm, ctrlprmCount, &ctrlredLight);
        }

        InitCamera(&ctrlcamera, track->sections);

        ctrlShadow = InitShadow(ctrlcamera.camPos, ctrleffectTextures);

        if (trackNo == 13)
            InitAuroraBorialis(ctrlprm[ctrlsky]);

        InitWeaponSprtIcons(ctrlweaponIconTextures);
        //   printf("after wicons\n");
        switch (gameData->shipNum)
        {
            case 0:
                InitScreenTex(ctrlwinLose, "dekka");
                break;

            case 1:
                InitScreenTex(ctrlwinLose, "chang");
                break;

            case 2:
                InitScreenTex(ctrlwinLose, "arial");
                break;

            case 3:
                InitScreenTex(ctrlwinLose, "anast");
                break;

            case 4:
                InitScreenTex(ctrlwinLose, "solar");
                break;

            case 5:
                InitScreenTex(ctrlwinLose, "arian");
                break;

            case 6:
                InitScreenTex(ctrlwinLose, "sophi");
                break;

            case 7:
                InitScreenTex(ctrlwinLose, "paul");
                break;
        }
        //   printf("after win lose\n");

        initShipData(track->sections, ctrlshipIndex, gameData);
        InitRescueDroid(&ctrlrescueDroid, ctrlshipShapes, &(ctrlshipIndex[ownShip]));

        initWeapons(ctrlweaponShapes, ctrlcamera.camPos, ctrlweaponIndex);

        ctrleffects = (Effect*)malloc((sizeof(Effect) * NO_OF_EFFECTS) + 32);
        InitEffects(ctrleffects, ctrlcamera.camPos, ctrleffectTextures);

        CameraUpdate = UpdateCameraRaceStart;

        /* Initialize hi-score and game */

        InitHiScoreData(&ctrlhiScoreData);

        gameData->defaultMenu = -2;
        gameData->Lives = 3;
        gameData->Position = 8;
        gameData->EndOfGame = 0;
        gameData->gameOverCount = 0;

        ctrlhiScoreData.confirm = 2;
        ctrlhiScoreData.lConfirm = 0;

        for (ctrl_i = 0; ctrl_i < 8; ctrl_i++)
        {
            ctrlcone.object[ctrl_i] = ctrlshipShapes[ctrlshipIndex[ctrl_i].highResTableIndex];
            ctrlcone.ship[ctrl_i] = &ctrlshipIndex[ctrl_i];
        }

        InitCone(&ctrlcone);

        if (ctrlmode == ATTRACT)
        {
            ctrlshipIndex[ownShip].update = UpdatePlayerShipNormStart;
            ctrlshipIndex[ownShip].updateCount = START_TIME;
            ctrlshipIndex[ownShip].remoteMaxThrust = 3160;
            ctrlshipIndex[ownShip].remoteThrustMag = 32;
            ctrlshipIndex[ownShip].speed = 3160;
            ctrlshipIndex[ownShip].attr &= ~VIEW_IN;
            ctrlshipIndex[ownShip].attr &= ~RACING;
            CameraUpdate = UpdateCameraAttractMode;

            if ((rand() % 3) == 0)
            {
                ctrlplayCredits = 1;
                attractDelay = 75 * FR30; // PCWipeout
            }
            else
            {
                attractDelay = 60 * FR30; // was 30*30 but this was too short I think
            }
            //          if(ctrlplayCredits == 1)
        }

        ctrlnextSection = ctrlcamera.section;

        ctrlshipIndex[ownShip].haveFired = NON_ACTIVE;

        if (gameType != SINGLE)
            ctrlshipIndex[serialShip].haveFired = NON_ACTIVE;

        SetNormPadPresets(gameData);

        if (gameData->actTrackNum == 13)
        {
            InitIceCongratulations();
        }

        if (gameData->actTrackNum == 14)
        {
            InitFireCongratulations();
        }

        inmenu = 0;
    }
}

void racectrl(void)
{
    ConfigData* gameData;
    int mode, timeout;

    gameData = ctrlgameData;
    mode = ctrlmode;
    timeout = ctrltimeout;

    // Reset all of my little Drawing markers
    ctrlNeedEndText = 0;
    ctrlNeedWinLose = 0;
    ctrlNeedBackPoly = 0;
    ctrlNeedRaceInfo = 0;
    ctrlNeedDeSelectScreen = 0;
    ctrlNeedNewHighScores = 0;
    ctrlNeedRacePoints = 0;
    ctrlNeedChampTable = 0;
    ctrlNeedAllVenomComplete = 0;
    ctrlNeedVenomComplete = 0;
    ctrlNeedContinueGameConfirm = 0;
    ctrlNeedInGameReset = 0;
    ctrlNeedInGameConfirm = 0;
    ctrlNeedInSingleGameConfirm = 0;
    ctrlNeedTargetIcon = 0;

    opad = pad;
    //    archive_pad = pad;
    pad = gPad;
    //    archive_xpad = xpad;          //preserve xpad from the drawing side
    xpad = (pad ^ opad) & pad; // xpad is difference between this frames
    // pad and the last frames pad.

    //    AJSVSync(2);
    if (ctrlfirsttime)
    {
        PlayNote(CROWD, 0, -1);
        PlayNote(CROWD2, 0, -1);
        ctrlfirsttime = 0;
    }

    if ((ctrlbuttonPressed == ctrltimeout) && // If no buttons have hit for timeout, and we were racing
        (ctrlshipIndex[ownShip].attr & RACING)) // set up for attract mode.
    {
        ctrlshipIndex[ownShip].update = UpdateRemoteShip;
        ctrlshipIndex[ownShip].updateCount = START_TIME;
        ctrlshipIndex[ownShip].remoteMaxThrust = 3160;
        ctrlshipIndex[ownShip].remoteThrustMag = 32;
        ctrlshipIndex[ownShip].speed = 3160;
        ctrlshipIndex[ownShip].attr &= ~VIEW_IN;
        ctrlshipIndex[ownShip].attr &= ~RACING;
        CameraUpdate = UpdateCameraAttractMode;
        attractDelay = 30 * FR30;
        ctrlmode = ATTRACT;
        ctrlbuttonPressed = 0;
    }

    if (ctrlshipIndex[ownShip].attr & RACING)
    {
        if ((GetMenuKey(M_START)) && (ctrlchoice == -1))
        {
            if (gameData->Lives)
            {
                ctrlCDPaused = 1; // If we're racing, and start key hit pause CD
                PlayNote(MENU_SELECT, 0, 20); // go boing
                ctrlchoice = 0; // set ctrlchoice to the first on the list
                xpad = 0; // clear the status of the pad so that the next bit does not select
            }
        }
    }

    if (ctrlchoice > -1) // we are in pause state
    {
        UpdateNotes(PAUSED); // service the held notes
        if ((GetMenuKey(M_UP)) && ctrlshowMenu) // hit up key
        {
            ctrlchoice--;
            if (ctrlchoice < 0)
                ctrlchoice = 3; // hard code no of entries to 3!
            PlayNote(MENU_MOVE, 0, 20); // Play a boing
        }
        else if ((GetMenuKey(M_DOWN)) && ctrlshowMenu)
        { // ditto but down
            ctrlchoice++;
            if (ctrlchoice > 3)
                ctrlchoice = 0;
            PlayNote(MENU_MOVE, 0, 20);
        }

        if (ctrlshowMenu)
        {
            ctrlNeedInGameReset = 1;
            ctrlNeedDeSelectScreen = 1;
            ctrllConfirm = 0;
            // Keyboard stuff
            if (ctrlchoice == 3)
            {
                if (GetMenuKey(M_LEFT))
                {
                    PlayNote(MENU_MOVE, 1, 2);
                    gameData->cdTrack--;
                    if (gameData->cdTrack < 1)
                        gameData->cdTrack = endtrack;
                    CDRestartNeeded = 1;
                    ctrlcdPause = 1;
                }
                else if (GetMenuKey(M_RIGHT))
                {
                    PlayNote(MENU_MOVE, 1, 2);
                    gameData->cdTrack++;
                    if (gameData->cdTrack > endtrack)
                        gameData->cdTrack = 1;
                    CDRestartNeeded = 1;
                    ctrlcdPause = 1;
                }
                CurrTrkNo = gameData->cdTrack;
            }
        }
        else
        {
            ctrlNeedInGameConfirm = 1;
            if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
            {
                ctrllConfirm ^= 1;
                PlayNote(MENU_MOVE, 1, 20);
            }
            //          if ((GetMenuKey(M_SELECT)) || (GetMenuKey(M_START)))
            if (GetMenuKey(M_SELECT))
                ctrlconfirm = ctrllConfirm;

            ctrlNeedDeSelectScreen = 1;
        }

        if (ctrlconfirm == 0)
        {
            ctrlshowMenu = 1;

            ctrlconfirm = 2;
            ctrlchoice = 0;
        }

        //        if( ((GetMenuKey(M_SELECT)) || (GetMenuKey(M_START))) && (!ctrlshowMenu) && (ctrlconfirm == 1) && (ctrlmachinePaused != SLAVE))
        if (GetMenuKey(M_SELECT) && (!ctrlshowMenu) && (ctrlconfirm == 1) && (ctrlmachinePaused != SLAVE))
        {
            ctrlshowMenu = 1;
            ctrlconfirm = 2;

            switch (ctrlchoice)
            {
                case 0:
                    ctrlchoice = -1; // Continue with the game
                    ctrlchoiceData = 0; // no new data
                    break;

                case 1:
                    ctrlchoice = -1; /* RESET */
                    ctrlchoiceData = 1;

                    if (gameData->gameType == CHAMPIONSHIP) // Chanpionship mode, then lose a life for the prividge
                    { // of restarting your race.
                        gameData->Lives--;
                        if (gameData->Lives == 0)
                        {
                            ctrlgameOverTime = 90; // 3 second
                            ctrlshowGameOver = 1;
                            xpad = 0;
                            break;
                        }
                    }
                    Restart(ctrlshipIndex, gameData, ctrlweaponIndex, ctrlweaponShapes, ctrleffects, &ctrlrescueDroid, ctrlshipShapes);

                    PlayNote(CROWD, 0, -1);
                    PlayNote(CROWD2, 0, -1);
                    break;

                case 2:
                    ctrlchoice = -1;
                    ctrlchoiceData = 2;
                    if (ctrlmachinePaused == SINGLE)
                    {
                        QuitGame();
                        gameData->defaultMenu = -2;
                        gameData->gameType = NO_GAME_SET;
                        ctrl_says_quit = 1;
                        return;
                    }
                    break;

            } /*end switch*/
        } /*end if (load a bollox*/

        //          if ( ((GetMenuKey(M_SELECT)) || (GetMenuKey(M_START))) && (ctrlshowMenu) && ctrlmachinePaused != SLAVE)
        if (GetMenuKey(M_SELECT) && (ctrlshowMenu) && ctrlmachinePaused != SLAVE)
        {
            PlayNote(MENU_SELECT, 0, 20);
            if (ctrlchoice == 1 || ctrlchoice == 2)
            {
                ctrlshowMenu = 0;
            }
            else if (ctrlchoice == 0)
            {
                ctrlCDPaused = 2;
                ctrlchoiceData = 0;
                ctrlchoice = -1;
            }
        }
    } // endif ctrlchoice > -1

    if (ctrlshowGameOver == 1)
    {

        /*       DeSelectScreen (ctrlselectBox, ctrlselectTrans) ;
         */
        if (ctrlgameOverTime == 0)
        {
            ctrlshowGameOver = 0;
            gameData->newLapRecord = 0;
        }
        ctrlgameOverTime--;

        /*       if (GetMenuKey(M_SELECT))
        {
        ctrlshowGameOver = 0 ;
        gameData->newLapRecord = 0 ;
        }
        */
        if (ctrlshowGameOver == 0)
        {
            QuitGame();
            gameData->defaultMenu = MAIN_MENU;
            gameData->gameType = NO_GAME_SET;
            ctrl_says_quit = 1;
            return;
        }
    }

    if ((ctrlchoice == -1) && (ctrlshowGameOver == 0))
    {
        UpdateNotes(NOT_PAUSED);

        if (ctrlmode != ATTRACT)
        {
            if (pad == 0)
                ctrlbuttonPressed++;
            else
                ctrlbuttonPressed = 0;
        }

        UpdateShips(ctrlshipIndex, ctrlweaponShapes, ctrlshipShapes, ctrlweaponIndex);

        (ctrlrescueDroid.droidUpdate)(&ctrlrescueDroid, &(ctrlshipIndex[ownShip]), ctrlshipShapes);

        if (ctrldoneExtro == UPDATE_EXTRO)
        {
            (CameraUpdate)(&ctrlcamera, ctrlshipIndex, &ctrlrescueDroid);
        }
        else
        {
            (CameraUpdate)(&ctrlcamera, &(ctrlshipIndex[ownShip]), &ctrlrescueDroid);
        }

        UpdateCone(&ctrlcone);

        if (ctrlshipIndex[ownShip].attr & RACING)
        {
            if (((ctrlshipIndex[ownShip].weaponType == MISSILE) || (ctrlshipIndex[ownShip].weaponType == ELEC_BOLT) || (ctrlshipIndex[ownShip].weaponType == REV_CON)) && (ctrlshipIndex[ownShip].attr & RACING))
            {
                MissileLock(&(ctrlshipIndex[ownShip]), ctrlshipIndex, ctrlshipShapes);
            }

            ShakeScreen();
        }

        UpdateWeapons(ctrlshipIndex, ctrlweaponShapes, ctrlweaponIndex);

        if (gameType == SINGLE)
            CallShip2ShipCollision(ctrlshipIndex, ctrlshipShapes);

        // AdjustEngineSound(&(ctrlshipIndex[ownShip]), &ctrlcamera);

        // AdjustCrowdSound(ctrllostad, ctrlstadCount);
        // AdjustRemoteEngineSound(ctrlshipIndex, ctrlshipShapes, &ctrlcamera );

        UpdateEffects(ctrleffects);

        if (raceType != TIME_TRIAL)
        {
            CycleWeaponGrids(ctrlshipIndex, ctrlweaponGrid, ctrlnoWeaponGrids);
        }
    }

    if ((attractDelay >= 0) && (ctrlmode == ATTRACT))
    {
        attractDelay--;
        if ((GetMenuKey(M_START)) || (GetMenuKey(M_SELECT)) || (attractDelay == 0))
        {
            gameData->defaultMenu = -2;
            gameData->gameType = NO_GAME_SET;
            QuitGame();
            ctrl_says_quit = 1;
            return;
        }
    }

    else if (attractDelay > 0)
    {
        if (gameType == SINGLE)
        {
            attractDelay--;

            if (attractDelay == 0 && gameData->gameType == SINGLE_RACE)
            {
                attractDelay = 1;
                if (gameData->showPilotCount != 0)
                {
                    if (gameData->Position == 1)
                    {
                        ctrlNeedWinLose = 1;
                        ctrlNeedEndText = 1;
                        ctrlNeedBackPoly = 1;
                        ctrlNeedRaceInfo = 1;
                        ctrlNeedDeSelectScreen = 1;
                        ctrlNeedEndTextVar = 1;
                        ctrlNeedWinLoseVar = 1;
                    }
                    else
                    {
                        ctrlNeedWinLose = 1;
                        ctrlNeedEndText = 1;
                        ctrlNeedBackPoly = 1;
                        ctrlNeedRaceInfo = 1;
                        ctrlNeedDeSelectScreen = 1;
                        ctrlNeedEndTextVar = 2;
                        ctrlNeedWinLoseVar = 0;
                    }

                    ajscount++;
                    if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                    {
                        gameData->showPilotCount = 0;
                        ajscount = 0;
                    }
                }
                else
                {
                    gameData->newLapRecord = 0;

                    if (gameData->EndOfGame)
                    {
                        if (gameData->Position == 1)
                        {
                            ctrlhiScoreData.highScore = CheckHiScore(gameData, &ctrlhiScoreData);
                        }
                    }

                    if (ctrlhiScoreData.highScore)
                    {
                        ctrlhiScoreData.displayHiScore = 1;
                        ctrlhiScoreData.highScore = 0;
                    }
                    if (ctrlhiScoreData.displayHiScore)
                    {
                        ctrlNeedNewHighScores = 1;
                        NewHighScore(gameData, &ctrlhiScoreData);
                        ctrlNeedDeSelectScreen = 1;
                        gameData->EndOfGame = 0;
                    }
                    else
                    {
                        ajscount++;

                        ctrlNeedInSingleGameConfirm = 1;
                        if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
                        {
                            ctrlhiScoreData.lConfirm ^= 1;
                            PlayNote(MENU_MOVE, 1, 20);
                        }
                        ajscount++;
                        if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                        {
                            if (ajscount > AJSDELAY)
                                ctrlhiScoreData.lConfirm = 1;
                            ctrlhiScoreData.confirm = ctrlhiScoreData.lConfirm;
                            ajscount = 0;
                        }

                        ctrlNeedDeSelectScreen = 1;

                        if (ctrlhiScoreData.confirm == 1)
                        {
                            ctrlhiScoreData.confirm = 2;
                            ctrlhiScoreData.lConfirm = 0;
                            attractDelay = 0;
                            gameData->defaultMenu = -2;
                            gameData->gameType = NO_GAME_SET;
                            QuitGame();
                            ctrl_says_quit = 1;
                            return;
                        }

                        if (ctrlhiScoreData.confirm == 0)
                        {
                            attractDelay = 0;
                            ctrlhiScoreData.confirm = 2;
                            ctrlhiScoreData.lConfirm = 0;
                            CDRestartNeeded = 1;
                            ctrlNeedDeSelectScreen = 0;
                            InitHiScoreData(&ctrlhiScoreData);

                            Restart(ctrlshipIndex, gameData, ctrlweaponIndex, ctrlweaponShapes, ctrleffects, &ctrlrescueDroid, ctrlshipShapes);
                            PlayNote(CROWD, 0, -1);
                            PlayNote(CROWD2, 0, -1);
                        }

                        //                   if ((GetMenuKey(M_SELECT))||(ajscount>AJSDELAY))
                        //                   {
                        //                      ajscount=0;
                        //                      gameData->newLapRecord = 0 ;
                        //                      ctrlhiScoreData.displayHiScore = 0 ;
                        //                      attractDelay = 0 ;
                        //                      gameData->defaultMenu = TRACK_MENU ;
                        //                      QuitGame();
                        //                      ctrl_says_quit = 1;
                        //                      return ;
                        //                   }
                    }
                }
            }

            if (attractDelay == 0 && gameData->gameType == TIME_TRIAL)
            {
                attractDelay = 1;

                if (gameData->gameType == TIME_TRIAL)
                {

                    if (gameData->showPilotCount != 0)
                    {

                        ctrlNeedRaceInfo = 1;
                        ctrlNeedDeSelectScreen = 1;

                        ajscount++;
                        if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                        {
                            gameData->showPilotCount = 0;
                            ajscount = 0;
                        }
                    }
                    else
                    {
                        gameData->newLapRecord = 0;
                        if (gameData->EndOfGame)
                            ctrlhiScoreData.highScore = CheckHiScore(gameData, &ctrlhiScoreData);

                        if (ctrlhiScoreData.highScore)
                        {
                            ctrlhiScoreData.displayHiScore = 1;
                            ctrlhiScoreData.highScore = 0;
                        }

                        if (ctrlhiScoreData.displayHiScore)
                        {
                            ctrlNeedNewHighScores = 1;
                            NewHighScore(gameData, &ctrlhiScoreData);
                            ctrlNeedDeSelectScreen = 1;
                            gameData->EndOfGame = 0;
                        }
                        else
                        {
                            ajscount++;

                            ctrlNeedInSingleGameConfirm = 1;
                            if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
                            {
                                ctrlhiScoreData.lConfirm ^= 1;
                                PlayNote(MENU_MOVE, 1, 20);
                            }
                            ajscount++;
                            if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                            {
                                if (ajscount > AJSDELAY)
                                    ctrlhiScoreData.lConfirm = 1;
                                ctrlhiScoreData.confirm = ctrlhiScoreData.lConfirm;
                                ajscount = 0;
                            }

                            ctrlNeedDeSelectScreen = 1;

                            if (ctrlhiScoreData.confirm == 1)
                            {
                                ctrlhiScoreData.confirm = 2;
                                ctrlhiScoreData.lConfirm = 0;
                                attractDelay = 0;
                                gameData->defaultMenu = -2;
                                gameData->gameType = NO_GAME_SET;
                                QuitGame();
                                ctrl_says_quit = 1;
                                return;
                            }

                            if (ctrlhiScoreData.confirm == 0)
                            {
                                attractDelay = 0;
                                ctrlhiScoreData.confirm = 2;
                                ctrlhiScoreData.lConfirm = 0;
                                CDRestartNeeded = 1;
                                InitHiScoreData(&ctrlhiScoreData);
                                Restart(ctrlshipIndex, gameData, ctrlweaponIndex, ctrlweaponShapes, ctrleffects, &ctrlrescueDroid, ctrlshipShapes);
                                PlayNote(CROWD, 0, -1);
                                PlayNote(CROWD2, 0, -1);
                            }

                            //                      ajscount++;
                            //                      if ((GetMenuKey(M_SELECT))||(ajscount>AJSDELAY))
                            //                      {
                            //                         ajscount=0;
                            //                         gameData->newLapRecord = 0 ;
                            //                         ctrlhiScoreData.displayHiScore = 0 ;
                            //                         attractDelay = 0 ;
                            //                         gameData->defaultMenu = TRACK_MENU ;
                            //                         QuitGame();
                            //                         ctrl_says_quit = 1;
                            //                         return ;
                            //
                            //                      }
                        }
                    }
                }
            }

            /****************** Do Extro ******************/
            if (attractDelay == 0 && gameData->gameType == CHAMPIONSHIP)
            {
                attractDelay = 1;

                if (gameData->gameType == CHAMPIONSHIP)
                {

                    if (gameData->Lives == 0) /*0*/
                    {
                        if (gameData->gameOverCount != 0)
                        {
                            ctrlNeedWinLose = 1;
                            ctrlNeedEndText = 1;
                            ctrlNeedBackPoly = 1;
                            ctrlNeedRaceInfo = 1;
                            ctrlNeedDeSelectScreen = 1;
                            ctrlNeedEndTextVar = 3;
                            ctrlNeedWinLoseVar = 0;

                            ajscount++;
                            if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                            {
                                ajscount = 0;
                                gameData->showGameOver = 1;
                                ctrlgameOverTime = 90; // 3 seconds
                                gameData->gameOverCount = 0;
                                gameData->newLapRecord = 0;
                            }
                        }
                        else if (gameData->gotoMenu == 1)
                        {
                            gameData->gotoMenu = 0;
                            gameData->gameType = NO_GAME_SET;
                            gameData->defaultMenu = -2;
                            gameData->newLapRecord = 0;
                            QuitGame();
                            ctrl_says_quit = 1;
                            return;
                        }
                        else

                            if (gameData->showGameOver == 1)
                        {

                            ajscount++;
                            if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                            {
                                ajscount = 0;
                                gameData->gotoMenu = 1;
                                gameData->showGameOver = 0;
                                gameData->newLapRecord = 0;
                            }
                        }
                    }
                    else
                    {
                        if (gameData->showPilotCount != 0)
                        {
                            if (gameData->Position >= 1 && gameData->Position <= 3)
                            {
                                ctrlNeedWinLose = 1;
                                ctrlNeedEndText = 1;
                                ctrlNeedBackPoly = 1;
                                ctrlNeedRaceInfo = 1;
                                ctrlNeedDeSelectScreen = 1;
                                ctrlNeedEndTextVar = 1;
                                ctrlNeedWinLoseVar = 1;
                            }
                            else
                            {
                                ctrlNeedWinLose = 1;
                                ctrlNeedEndText = 1;
                                ctrlNeedBackPoly = 1;
                                ctrlNeedRaceInfo = 1;
                                ctrlNeedDeSelectScreen = 1;
                                ctrlNeedEndTextVar = 3;
                                ctrlNeedWinLoseVar = 0;
                            }

                            ajscount++;
                            if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                            {
                                ajscount = 0;
                                gameData->showPilotCount = 0;
                                gameData->newLapRecord = 0;
                            }
                        }
                        else if (gameData->showPointCount != 0)
                        {
                            if (gameData->Position >= 1 && gameData->Position <= 3)
                            {
                                ctrlNeedRacePoints = 1;
                                ctrlNeedDeSelectScreen = 1;
                            }
                            ajscount++;
                            if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                            {
                                ajscount = 0;
                                gameData->showPointCount = 0;
                            }
                        }
                        else if (gameData->showChampCount != 0)
                        {
                            if (gameData->Position >= 1 && gameData->Position <= 3)
                            {
                                ctrlNeedChampTable = 1;
                                ctrlNeedDeSelectScreen = 1;
                            }
                            ajscount++;
                            if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                            {
                                ajscount = 0;
                                gameData->showChampCount = 0;
                            }
                        }
                        else if (gameData->venomComplete != 0)
                        {
                            if (gameData->showBonusTrack == 1)
                                ctrlNeedAllVenomComplete = 1;
                            else
                                ctrlNeedVenomComplete = 1;

                            ctrlNeedDeSelectScreen = 1;

                            ajscount++;
                            if ((ajscount > (AJSDELAY / 2)))
                            {
                                ajscount = 0;
                                gameData->venomComplete = 0;
                            }
                        }
                        else
                        {
                            if (gameData->Position >= 1 && gameData->Position <= 3)
                            {
                                if (gameData->EndOfGame)
                                    ctrlhiScoreData.highScore = CheckHiScore(gameData, &ctrlhiScoreData);
                            }

                            if (ctrlhiScoreData.highScore)
                            {
                                ctrlhiScoreData.displayHiScore = 1;
                                ctrlhiScoreData.highScore = 0;
                            }

                            if (ctrlhiScoreData.displayHiScore)
                            {
                                ctrlNeedNewHighScores = 1;
                                NewHighScore(gameData, &ctrlhiScoreData);
                                ctrlNeedDeSelectScreen = 1;

                                gameData->EndOfGame = 0;
                            }
                            else
                            {
                                if (gameData->lostGame == 1)
                                {

                                    ajscount++;
                                    if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                                    {
                                        ajscount = 0;
                                        gameData->lostGame = 0;
                                        gameData->gotoMenu = 1;
                                    }
                                }
                                else
                                {

                                    if (gameData->gameComplete == 1)
                                    {
                                        if (ctrldoneExtro == START_EXTRO)
                                        {
                                            //                                       printf( "init extro\n" );
                                            CameraUpdate = UpdateCameraExtroAttract;
                                            for (ctrl_i = 0; ctrl_i < NO_OF_SHIPS; ctrl_i++)
                                            {
                                                ctrlshipIndex[ctrl_i].remoteMaxThrust = 3160;
                                                ctrlshipIndex[ctrl_i].remoteThrustMag = 32;
                                                ctrlshipIndex[ctrl_i].speed = 3160;
                                            }
                                            ctrldoneExtro = UPDATE_EXTRO;
                                        }

                                        if (ctrldoneExtro == UPDATE_EXTRO)
                                        {
                                            ajscount++;
                                            if (GetMenuKey(M_SELECT) || GetMenuKey(M_START) || (ajscount > AJSDELAY))
                                            {
                                                ajscount = 0;
                                                finishedExtro = 1;
                                            }
                                        }

                                        if (finishedExtro)
                                        {
                                            //                                       printf( "\nif finished extro \n" );
                                            gameData->gameType = NO_GAME_SET;
                                            gameData->defaultMenu = OPTION_MENU;
                                            gameData->gotoMenu = 0;
                                            InitChampionship();
                                            gameData->gameComplete = 0;
                                            QuitGame();
                                            ctrl_says_quit = 1;
                                            return;

                                            /* gameData->gameComplete = 0;
                                            gameData->gotoMenu = 1;  */
                                        }
                                    }
                                    else
                                    {

                                        if (gameData->gotoMenu != 0)
                                        {
                                            gameData->gameType = NO_GAME_SET;
                                            gameData->defaultMenu = OPTION_MENU;
                                            gameData->gotoMenu = 0;
                                            InitChampionship();
                                            gameData->gameComplete = 0;
                                            QuitGame();
                                            ctrl_says_quit = 1;
                                            return;
                                        }

                                        gameData->EndOfGame = 0;
                                        InitHiScoreData(&ctrlhiScoreData);
                                        gameData->newLapRecord = 0;
                                        if (gameData->Position >= 1 && gameData->Position <= 3)
                                        {
                                            //                                     printf ("Number of track = %d \n", gameData->NumTracks) ;
                                            QuitGame();
                                            ctrl_says_quit = 1;
                                            return;
                                        }
                                        else
                                        {
                                            ctrlNeedContinueGameConfirm = 1;
                                            if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
                                            {
                                                ctrlhiScoreData.lConfirm ^= 1;
                                                PlayNote(MENU_MOVE, 1, 20);
                                            }
                                            ajscount++;
                                            if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                                            {
                                                if (ajscount > AJSDELAY)
                                                    ctrlhiScoreData.lConfirm = 1;
                                                ctrlhiScoreData.confirm = ctrlhiScoreData.lConfirm;
                                                ajscount = 0;
                                            }

                                            ctrlNeedDeSelectScreen = 1;

                                            if (ctrlhiScoreData.confirm == 1)
                                            {
                                                ctrlhiScoreData.confirm = 2;
                                                ctrlhiScoreData.lConfirm = 0;
                                                attractDelay = 0;
                                                gameData->defaultMenu = -2;
                                                gameData->gameType = NO_GAME_SET;
                                                QuitGame();
                                                ctrl_says_quit = 1;
                                                return;
                                            }

                                            if (ctrlhiScoreData.confirm == 0)
                                            {
                                                attractDelay = 0;
                                                ctrlhiScoreData.confirm = 2;
                                                ctrlhiScoreData.lConfirm = 0;
                                                Restart(ctrlshipIndex, gameData, ctrlweaponIndex, ctrlweaponShapes, ctrleffects, &ctrlrescueDroid, ctrlshipShapes);
                                                PlayNote(CROWD, 0, -1);
                                                PlayNote(CROWD2, 0, -1);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
                gameData->continueConfirm = 1;
        }
    }

    if (ctrlhiScoreData.displayHiScore == 1) { }

    /*          else if((shipIndex[ownShip].attr & RACING) ||
    (CameraUpdate == UpdateCameraAttractInternal))
    */
    {
        if ((ctrlmode != ATTRACT))
        {
            UpdateRaceHud(ctrlshipIndex, &ctrlspeedo, ctrlchoice, gameData);
        }
    }

    /*********************************************************************/
    /**** Draw, Mesh, AutoMesh, Reject, Don`t reject, Nurse, Bend Over  **/
    /**** Abuse & Call a Naughty boy  TRACK CODE                        **/
    /*********************************************************************/
    if (ctrlshipIndex[serialShip].haveFired == PAUSE && !ctrlactivated)
    {
        ctrlmachinePaused = SLAVE;
        ctrlactivated = 1;
        ctrlchoice = 0;
    }

    if ((ctrlshipIndex[serialShip].haveFired == QUIT) || (ctrlshipIndex[ownShip].haveFired == QUIT))
    {
        ctrlquitTwoPlayer = 0;
        ctrlactivated = 0;
        ctrlmachinePaused = SINGLE;
        ctrlchoice = -1;
        gameData->defaultMenu = -2;
        QuitGame();
        ctrl_says_quit = 1;
        return;
    }

    if (ctrlactivated && ctrlmachinePaused == SLAVE)
    {

        if (ctrlshipIndex[serialShip].haveFired == CONT)
        {
            ctrlactivated = 0;
            ctrlmachinePaused = SINGLE;
            ctrlchoice = -1;
        }

        if (ctrlshipIndex[serialShip].haveFired == RETIRE)
        {
            //          printf ("Am retiring \n") ;
            ctrlactivated = 0;
            ctrlmachinePaused = SINGLE;
            ctrlchoice = -1;
            gameData->Lives--;

            Restart(ctrlshipIndex, gameData, ctrlweaponIndex, ctrlweaponShapes, ctrleffects, &ctrlrescueDroid, ctrlshipShapes);
            PlayNote(CROWD, 0, -1);
            PlayNote(CROWD2, 0, -1);
        }

        if (ctrlshipIndex[serialShip].haveFired == QUIT)
        {
            ctrlactivated = 0;
            ctrlmachinePaused = SINGLE;
            ctrlchoice = -1;
            gameData->defaultMenu = -2;
            QuitGame();
            ctrl_says_quit = 1;
            return;
        }
    }

    if (ctrlmachinePaused == MASTER && ctrlchoiceData == 2)
    {
        ctrlactivated = 0;
        ctrlmachinePaused = SINGLE;
        ctrlchoiceData = -1;
        gameData->defaultMenu = -2;
        QuitGame();
        ctrl_says_quit = 1;
        return;
    }

    //    copy_glob_to_spare(xpad);
    //    xpad = archive_xpad; //bodge to restore the xpad of the current drawing frame
    //    pad = archive_pad;   //bodge to restore the xpad of the current drawing frame
    ctrl_init = 1; // OK to start the drawing now, once we've been around once.
} /*end of race*/
