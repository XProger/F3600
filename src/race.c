#include "game.h"

#define START_EXTRO     0
#define UPDATE_EXTRO    1
#define DONE_EXTRO      2

#define PrmMax 64
#define AJSDELAY 60 * 30 // arbitrary number - linked to frame rate ? HOPE SO !

extern Cone ctrlcone;

extern short startIceText;
extern short startFireText;
int ajscount = 0; // oh shit another global flag

// All race locals now globals, as RaceCtrl has to be re-entrant
TexTemplate* ctrltexTemplate;

/**************************************************/
/******           Dave additions              *****/
/**************************************************/

Object* ctrlshipShapes[(NO_OF_SHIPS * 2) + 1];
ShipData ctrlshipIndex[NO_OF_SHIPS];
RescueData ctrlrescueDroid;

Object* ctrlweaponShapes[WEAPON_SHAPE_NAX];

int ctrlchoice;
Face* ctrlfacePtr;
short ctrlbuttonPressed;
int ctrldoneExtro;
int ctrlplayCredits;
int ctrlgameOverTime;
int ctrlnearShip;

int HACK_LOAD_TRACK;
int HACK_LOAD_SHADOW;
int HACK_TEX_FILTER;

/**************************************************/
/******           Stu additions               *****/
/**************************************************/
char ctrlshowMenu;
char ctrlconfirm;
char ctrllConfirm;
HiScoreData ctrlhiScoreData;
int ctrlchoiceData;
char ctrlshowGameOver;

extern long ctrlmode;
extern long ctrltimeout;
extern long ctrlGameLoop;
extern ConfigData ctrlConfigData;

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
long ctrlfirsttime;

long ctrl_says_quit;
unsigned long ctrlFramesCount;

extern short trackNum; // SJR

const char* const PILOT_NAMES[] =
{
    "dekka",
    "chang",
    "arial",
    "anast",
    "solar",
    "arian",
    "sophi",
    "paul"
};

void AdjustCrowdSound(Object* lost, int lostCount)
{
    int i;
    vec3i distance;
    int magSq;
    int chosen;
    int smallestMagSq;
    vec3i pos;

#ifdef __3DO__ // TODO optimize
    return;
#endif

    smallestMagSq = 0x7FFFFFFF;
    chosen = -1;

    /*TODO audio crowd lerp between two closest*/

    for (i = 0; i < lostCount; i++)
    {
        distance.x = (lost[i].pos.x + gCamera.pos.x) >> 3;
        distance.y = (lost[i].pos.y + gCamera.pos.y) >> 3;
        distance.z = (lost[i].pos.z + gCamera.pos.z) >> 3;

        magSq = GetMagnitudeSqr(&distance);

        if (magSq < smallestMagSq)
        {
            smallestMagSq = magSq;
            chosen = i;
        }
    }

    if (smallestMagSq < (8192 * 8192))
    {
        pos.x = lost[chosen].pos.x;
        pos.y = lost[chosen].pos.y;
        pos.z = lost[chosen].pos.z;

        AdjustDirectionalNote(SFX_CROWD, &pos);
    }
    else
    {
        SetVolume(SFX_CROWD, 0, 0);
    }
}

void Restart(ShipData* shipIndex, ConfigData* gameData, Object** weaponShapes, RescueData* rescueDroid, Object** shipShapes)
{
    KillNotes();
    initShipData(gTrack.sections, shipIndex, gameData);
    InitRescueDroid(rescueDroid, shipShapes, &(shipIndex[ownShip]));
    gCamera.section = shipIndex[ownShip].nearTrkSect;

    ResetWeapons(ctrlweaponShapes);
    ResetEffects();
    ResetStartBoom();

    CameraUpdate = UpdateCameraRaceStart;
}

void QuitGame()
{
    KillNotes();
}

void showLoadingScreen()
{
    char loadPath[48];
    char loadNo[3];

    /* load screen stuff */
    strcpy(loadPath, "wipeout/textures/load");
    getTrackStrNo(loadNo, ctrlConfigData.actTrackNum);
    strcat(loadPath, loadNo);
    strcat(loadPath, ".tim");

    //TODO render/swap/render/swap
    //JJSLoad16BitTexture(loadPath, SCREEN);
}

void raceInit(ConfigData* gameData)
{
    printf("race init\n");

    renderOffset(0, 0);

    if (ctrlmode != ATTRACT) // not demo mode
    {
        trackNo = gameData->actTrackNum;
    }

    showLoading(ctrlConfigData.actTrackNum);

    trackNum = gameData->trackNum; // for restoring - SJR

    attractDelay = 0;

    tempLapTimes[0] = 0;
    tempLapTimes[1] = 0;
    tempLapTimes[2] = 0;
    tempLapTimes[3] = 0;
    tempLapTimes[4] = 0;

    gameData->newLapRecord = 0;

    ctrl_says_quit = 0;

    ctrlchoice = -1;
    ctrlbuttonPressed = 0;
    ctrlplayCredits = 0;
    ctrldoneExtro = START_EXTRO;
    ctrlgameOverTime = 0;
    ctrlshowMenu = 1;
    ctrlconfirm = 0;
    ctrllConfirm = 0;
    ctrlchoiceData = -1;
    ctrlshowGameOver = 0;
    ctrlfirsttime = 1;
    finishedExtro = 0;

    /**********************************************************/
    /***                 Initialise Track                   ***/
    /**********************************************************/
    /* firestar */
    if (gameData->actTrackNum == 14)
    {
        camSpline[0] = LoadPrm("wipeout/common/cam1s.prm", 0, NULL);
        shipSpline[0] = LoadPrm("wipeout/common/shp1s.prm", 0, NULL);

        camSpline[1] = LoadPrm("wipeout/common/cam2s.prm", 0, NULL);
        shipSpline[1] = LoadPrm("wipeout/common/shp2s.prm", 0, NULL);

        camSpline[2] = LoadPrm("wipeout/common/cam3s.prm", 0, NULL);
        shipSpline[2] = LoadPrm("wipeout/common/shp3s.prm", 0, NULL);

        camSpline[3] = LoadPrm("wipeout/common/cam4s.prm", 0, NULL);
        shipSpline[3] = LoadPrm("wipeout/common/shp4s.prm", 0, NULL);
    }

    /* silverstream */
    if (gameData->actTrackNum == 13)
    {
        camSpline[4] = LoadPrm("wipeout/common/cam5s.prm", 0, NULL);
        shipSpline[4] = LoadPrm("wipeout/common/shp5s.prm", 0, NULL);

        camSpline[5] = LoadPrm("wipeout/common/cam6s.prm", 0, NULL);
        shipSpline[5] = LoadPrm("wipeout/common/shp6s.prm", 0, NULL);

        camSpline[6] = LoadPrm("wipeout/common/cam7s.prm", 0, NULL);
        shipSpline[6] = LoadPrm("wipeout/common/shp7s.prm", 0, NULL);
    }

    InitScene(gameData->actTrackNum);

    InitCamera(&gCamera, gTrack.sections);

    InitShadow();

    InitWinLoseTex(PILOT_NAMES[gameData->shipNum]);

    initShipData(gTrack.sections, ctrlshipIndex, gameData);
    InitRescueDroid(&ctrlrescueDroid, ctrlshipShapes, &(ctrlshipIndex[ownShip]));

    ResetWeapons(ctrlweaponShapes);
    ResetEffects();
    ResetStartBoom();

    CameraUpdate = UpdateCameraRaceStart;

    /* Initialize hi-score and game */

    InitHiScoreData(&ctrlhiScoreData);

    gameData->Lives = 3;
    gameData->Position = NO_OF_SHIPS;
    gameData->EndOfGame = 0;
    gameData->gameOverCount = 0;

    ctrlhiScoreData.confirm = 2;
    ctrlhiScoreData.lConfirm = 0;

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
    }

    ctrlshipIndex[ownShip].haveFired = 0;

    SetNormPadPresets(gameData);

    if (gameData->actTrackNum == 13)
    {
        InitIceCongratulations();
    }

    if (gameData->actTrackNum == 14)
    {
        InitFireCongratulations();
    }

    UpdateEffects();

    ctrlFramesCount = 1;

#ifdef _DEBUG
    convertTrack3DO(gameData->actTrackNum);
#endif
}

void raceCtrl(ConfigData* gameData)
{
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

    //ctrlshipIndex[ownShip].weaponType = WPN_ROCKET; // fx debug

    if (gTargetFOV > CAMERA_FOV)
        gTargetFOV -= (1 << 12);
    gCameraFOV = gCameraFOV + ((gTargetFOV - gCameraFOV) >> 3);

    UpdatePad();

    if (ctrlfirsttime)
    {
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
                PlayNote(SFX_MENU_SELECT); // go boing
                ctrlchoice = 0; // set ctrlchoice to the first on the list
                xpad = 0; // clear the status of the pad so that the next bit does not select

                SetVolume(SFX_REMOTE_ENG, 0, 0);
                SetVolume(SFX_ENG_RUMBLE, 0, 0);
                SetVolume(SFX_ENG_AIR_INTAKE, 0, 0);
                SetVolume(SFX_ENG_THRUST, 0, 0);
                SetVolume(SFX_TURBULENCE, 0, 0);
                SetVolume(SFX_CROWD, 0, 0);
                SetVolume(SFX_SHIELD, 0, 0);
            }
        }
    }

    if (ctrlchoice > -1) // we are in pause state
    {
        if ((GetMenuKey(M_UP)) && ctrlshowMenu) // hit up key
        {
            ctrlchoice--;
            if (ctrlchoice < 0)
                ctrlchoice = 3; // hard code no of entries to 3!
            PlayNote(SFX_MENU_MOVE); // Play a boing
        }
        else if ((GetMenuKey(M_DOWN)) && ctrlshowMenu)
        { // ditto but down
            ctrlchoice++;
            if (ctrlchoice > 3)
                ctrlchoice = 0;
            PlayNote(SFX_MENU_MOVE);
        }

        if (ctrlshowMenu)
        {
            ctrlNeedInGameReset = 1;
            ctrlNeedDeSelectScreen = 1;
            ctrllConfirm = 0;

            if (ctrlchoice == 3)
            {
                if (GetMenuKey(M_LEFT))
                {
                    GoPrevMusic();
                }
                else if (GetMenuKey(M_RIGHT))
                {
                    GoNextMusic();
                }
                else if (GetMenuKey(M_SELECT) && !gMusicTrack)
                {
                    MusicStop();
                }
            }
        }
        else
        {
            ctrlNeedInGameConfirm = 1;
            if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
            {
                ctrllConfirm ^= 1;
                PlayNote(SFX_MENU_MOVE);
            }

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

        if (GetMenuKey(M_SELECT) && (!ctrlshowMenu) && (ctrlconfirm == 1))
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
                    Restart(ctrlshipIndex, gameData, ctrlweaponShapes, &ctrlrescueDroid, ctrlshipShapes);
                    break;

                case 2:
                    ctrlchoice = -1;
                    ctrlchoiceData = 2;
                    QuitGame();
                    gameData->gameType = NO_GAME_SET;
                    ctrl_says_quit = 1;
                    return;
            } /*end switch*/
        } /*end if (load a bollox*/

        if (GetMenuKey(M_SELECT) && (ctrlshowMenu))
        {
            PlayNote(SFX_MENU_SELECT);
            if (ctrlchoice == 1 || ctrlchoice == 2)
            {
                ctrlshowMenu = 0;
            }
            else if (ctrlchoice == 0)
            {
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
            gameData->gameType = NO_GAME_SET;
            ctrl_says_quit = 1;
            return;
        }
    }

    if ((ctrlchoice == -1) && (ctrlshowGameOver == 0))
    {
        if (ctrlmode != ATTRACT)
        {
            if (pad == 0)
                ctrlbuttonPressed++;
            else
                ctrlbuttonPressed = 0;
        }

        UpdateShips(ctrlshipIndex);

        (ctrlrescueDroid.droidUpdate)(&ctrlrescueDroid, &(ctrlshipIndex[ownShip]), ctrlshipShapes);

        if (ctrldoneExtro == UPDATE_EXTRO)
        {
            (CameraUpdate)(&gCamera, ctrlshipIndex, &ctrlrescueDroid);
        }
        else
        {
            (CameraUpdate)(&gCamera, &(ctrlshipIndex[ownShip]), &ctrlrescueDroid);
        }

        UpdateCone(&ctrlcone);

        if (ctrlshipIndex[ownShip].attr & RACING)
        {
            if (((ctrlshipIndex[ownShip].weaponType == WPN_MISSILE) || (ctrlshipIndex[ownShip].weaponType == WPN_ELEC_BOLT) || (ctrlshipIndex[ownShip].weaponType == WPN_REV_CON)) && (ctrlshipIndex[ownShip].attr & RACING))
            {
                MissileLock(&(ctrlshipIndex[ownShip]), ctrlshipIndex, ctrlshipShapes);
            }

            ShakeScreen();
        }

        UpdateWeapons(ctrlshipIndex, ctrlweaponShapes);

        CallShip2ShipCollision(ctrlshipIndex, ctrlshipShapes);

        UpdateEffects();

        SetVolume(SFX_SHIELD, SND_MAX_VOLUME, SND_MAX_VOLUME);

        AdjustEngineSound(&(ctrlshipIndex[ownShip]), &gCamera);
        AdjustCrowdSound(gTrack.lost, gTrack.lostCount);
        AdjustRemoteEngineSound(ctrlshipIndex, ctrlshipShapes);

        if (raceType != TIME_TRIAL)
        {
            CycleWeaponGrids(ctrlshipIndex, gTrack.weapons, gTrack.weaponsCount);
        }
    }

    if ((attractDelay >= 0) && (ctrlmode == ATTRACT))
    {
        attractDelay--;
        if ((GetMenuKey(M_START)) || (GetMenuKey(M_SELECT)) || (attractDelay == 0))
        {
            gameData->gameType = NO_GAME_SET;
            QuitGame();
            ctrl_says_quit = 1;
            return;
        }
    }
    else if (attractDelay > 0)
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
                        PlayNote(SFX_MENU_MOVE);
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
                        ctrlNeedDeSelectScreen = 0;
                        InitHiScoreData(&ctrlhiScoreData);

                        Restart(ctrlshipIndex, gameData, ctrlweaponShapes, &ctrlrescueDroid, ctrlshipShapes);
                    }
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
                            PlayNote(SFX_MENU_MOVE);
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
                            InitHiScoreData(&ctrlhiScoreData);
                            Restart(ctrlshipIndex, gameData, ctrlweaponShapes, &ctrlrescueDroid, ctrlshipShapes);
                        }
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
                                        int i;
                                        CameraUpdate = UpdateCameraExtroAttract;
                                        for (i = 0; i < NO_OF_SHIPS; i++)
                                        {
                                            ctrlshipIndex[i].remoteMaxThrust = 3160;
                                            ctrlshipIndex[i].remoteThrustMag = 32;
                                            ctrlshipIndex[i].speed = 3160;
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
                                        gameData->gameType = NO_GAME_SET;
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
                                            PlayNote(SFX_MENU_MOVE);
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
                                            Restart(ctrlshipIndex, gameData, ctrlweaponShapes, &ctrlrescueDroid, ctrlshipShapes);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ((ctrlmode != ATTRACT))
    {
        UpdateRaceHud(ctrlshipIndex, ctrlchoice, gameData);
    }
}

void raceLoop(ConfigData* gameData)
{
    int i, count;
    
    ctrlFramesCount += gFrameIndex - gLastFrameIndex;

    count = ctrlFramesCount >> 1; // 60 -> 30 Hz
    ctrlFramesCount -= count << 1;

    if (count > 10) // max frame skip
    {
        count = 10;
    }

    for (i = 0; i < count; i++)
    {
        dbg_update = osGetSystemTimeMS();
        raceCtrl(gameData);
        dbg_update = osGetSystemTimeMS() - dbg_update;
    }

    renderFrameBegin();
    renderClear();

    if (trackNo == 13)
    {
        AuroraBorialis(gTrack.sky);
    }

    if ((trackNo == 8) || (trackNo == 11))
    {
        ColourRedLight();
        MoveDonkey();
    }
    else if ((trackNo == 1) /* || (trackNo == 6)*/)
    {
        ColourRedLight();
    }
    
    ColourDroidLights(ctrlshipShapes[RESCUE_DROID]);

    renderSetMode(RENDER_MODE_3D);

    if (CameraUpdate == UpdateCameraRaceInternal)
    {
        matrixSetViewInternal(&gCamera.pos, gCamera.pitch, gCamera.hdg, gCamera.roll);
    }
    else
    {
        matrixSetViewExternal(&gCamera.pos, gCamera.pitch, gCamera.hdg);
    }

    DrawSky(gTrack.sky);
    DrawLandscape(&gCamera);
    DrawTrack();
    DrawShadows(ctrlshipIndex);
    DrawShips(ctrlshipIndex, ctrlshipShapes, ownShip);
    DrawDroid(ctrlshipShapes[RESCUE_DROID], &ctrlrescueDroid);
    DrawWeapons(ctrlshipIndex, ctrlweaponShapes);
    DrawEffects();

    renderSetMode(RENDER_MODE_2D);

    if (ctrlNeedTargetIcon == 1)
        DrawTargetIcon(ctrlshipIndex, ctrlshipShapes);

    if (ctrlshipIndex[ownShip].attr & RACING)
        DrawHud(ctrlshipIndex, ctrlchoice, gameData);

    if (ctrlNeedDeSelectScreen)
        renderBackFill(-gBorder, 0, 320 + gBorder * 2, 240);

    if (ctrlNeedBackPoly)
        renderBackFill(180, 50, 128, 140);
    
    if (ctrlmode == ATTRACT)
    {
        if (ctrlplayCredits == 1)
            ShowCredits(attractDelay);
        else
            CentreText("DEMO MODE", text_data(60, 14, 8), TEXT_COLOR_HIGHLIGHT);
    }

    // All game overs
    if ((ctrlshowGameOver == 1) || (gameData->showGameOver == 1) || (gameData->lostGame == 1))
        CentreText("GAME OVER", text_data(60, 112, 16), TEXT_COLOR_HIGHLIGHT);

    // Text at end.
    if (ctrlNeedEndText)
    {
        switch (ctrlNeedEndTextVar)
        {
            case 1:
                CentreText("CONGRATULATIONS", text_data(15, 20, 8), TEXT_COLOR_HIGHLIGHT);
                break;
            case 2:
                CentreText("BAD LUCK", text_data(15, 20, 8), TEXT_COLOR_HIGHLIGHT);
                break;
            case 3:
                CentreText("FAILED TO QUALIFY", text_data(15, 20, 8), TEXT_COLOR_HIGHLIGHT);
                break;
            default:
                CentreText("LOST THE VARIABLE", text_data(15, 20, 8), TEXT_COLOR_HIGHLIGHT);
                break;
        }
    }
    // Pictures at end
    if (ctrlNeedWinLose)
        DisplayWinLose(ctrlNeedWinLoseVar);
    // High score bollox
    if (ctrlNeedRaceInfo)
        DisplayRaceInfo(gameData, &ctrlhiScoreData);
    if (ctrlNeedNewHighScores)
        NewHighScoreDraw(gameData, &ctrlhiScoreData);
    if (ctrlNeedRacePoints)
        DisplayRacePoints();
    if (ctrlNeedChampTable)
        DisplayChampTable();
    if (ctrlNeedAllVenomComplete)
        DisplayAllVenomComplete();
    if (ctrlNeedVenomComplete)
        DisplayVenomComplete();
    if (ctrlNeedContinueGameConfirm)
        ContinueGameConfirm(&(ctrlhiScoreData.confirm), xpad, &(ctrlhiScoreData.lConfirm));
    if (ctrlNeedInSingleGameConfirm)
        RestartGameConfirm(&(ctrlhiScoreData.confirm), xpad, &(ctrlhiScoreData.lConfirm));
    if (ctrlNeedInGameReset)
        InGameReset(ctrlchoice);
    if (ctrlNeedInGameConfirm)
        InGameConfirm(&ctrlconfirm, ctrlchoice, xpad, &ctrllConfirm);

    if (startIceText)
    {
        ShowIceCongratulations();
    }
    if (startFireText)
    {
        ShowFireCongratulations();
    }

    renderFrameEnd();

    if (ctrl_says_quit)
    {
        ctrlmode = INTERACTIVE;

        ResetCone(&ctrlcone);
        ctrlGameLoop = (gameData->gameType == CHAMPIONSHIP) ? GAME_LOOP_RACE_INIT : GAME_LOOP_MAIN;
        return;
    }
}

void raceDemo(ConfigData* gameData)
{
    ctrlmode = ATTRACT;
    ownShip = randy() % 8;

    gameData->gameType = SINGLE_RACE;
    gameData->trackNum = 1 + (randy() % 6); // all venom
    gameData->actTrackNum = trackNo = gTrackList[gameData->trackNum - 1];
    ctrlGameLoop = GAME_LOOP_RACE_INIT;
}
