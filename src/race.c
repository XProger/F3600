#include "game.h"

#define START_EXTRO     0
#define UPDATE_EXTRO    1
#define DONE_EXTRO      2

#define AJSDELAY 60 * 30 // arbitrary number - linked to frame rate ? HOPE SO !

extern short startIceText;
extern short startFireText;
int ajscount = 0; // oh shit another global flag

/**************************************************/
/******           Dave additions              *****/
/**************************************************/

int ctrlchoice;
Face* ctrlfacePtr;
int ctrldoneExtro;
int ctrlplayCredits;
int ctrlgameOverTime;

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

/*          Andy Y's additions*/
int32 ctrlNeedEndText;
int32 ctrlNeedWinLose;
int32 ctrlNeedBackPoly;
int32 ctrlNeedRaceInfo;
int32 ctrlNeedDeSelectScreen;
int32 ctrlNeedNewHighScores;
int32 ctrlNeedRacePoints;
int32 ctrlNeedChampTable;
int32 ctrlNeedAllVenomComplete;
int32 ctrlNeedVenomComplete;
int32 ctrlNeedContinueGameConfirm;
int32 ctrlNeedEndTextVar;
int32 ctrlNeedWinLoseVar;
int32 ctrlNeedInGameReset;
int32 ctrlNeedInSingleGameConfirm;
int32 ctrlNeedInGameConfirm;

int32 ctrl_says_quit;

ShipData* gSortedShips[NO_OF_SHIPS];

const short startLinePos[14] = {
// Venom
    27,     /* Altima VII   */
    16,     /* Karbonis     */
    27,     /* TerrorMax    */
    16,     /* Korrodera    */
    16,     /* Arridos IV   */
    16,     /* SilverStream */
    27,     /* FireStar     */
// Rapier
    27,     /* Altima VII   */
    16,     /* Karbonis     */
    27,     /* TerrorMax    */
    16,     /* Korrodera    */
    16,     /* Arridos IV   */
    16,     /* SilverStream */
    27      /* FireStar     */
};

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
    int32 dx, dz;
    int mag;
    int chosen;
    int smallestMag;
    TrackCamera* camera = &gPlayers[0].camera; // TODO choose nearest camera

#ifdef __3DO__ // TODO 3DO optimize
    return;
#endif

    smallestMag = 0x7FFFFFFF;
    chosen = -1;

    /*TODO audio crowd lerp between two closest*/

    for (i = 0; i < lostCount; i++)
    {
        dx = (lost[i].pos.x - camera->pos.x) >> 3;
        dz = (lost[i].pos.z - camera->pos.z) >> 3;

        mag = GetMagnitudeApprox(dx, dz);

        if (mag < smallestMag)
        {
            smallestMag = mag;
            chosen = i;
        }
    }

    if (smallestMag < 8192)
    {
        AdjustDirectionalNote(SFX_CROWD, lost[chosen].pos.x, lost[chosen].pos.z);
    }
    else
    {
        SetVolume(SFX_CROWD, 0, 0);
    }
}

void Restart(ShipData* shipIndex)
{
    int32 i;
    KillNotes();
    InitShipData(gTrack.sections, shipIndex);

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;
        if (player->ship)
        {
            InitCamera(&player->camera, player->ship->nearTrkSect);
            InitRescueDroid(&player->droid, player->ship);
            memset(player->lapTimes, 0, sizeof(player->lapTimes));
            memset(&player->replay, 0, sizeof(player->replay) - sizeof(player->replay.data));
        }
    }

    ResetWeapons();
    ResetEffects();
    ResetStartBoom();

    gConfig.EndOfGame = 0;
    gConfig.newLapRecord = 0;
}

void QuitGame()
{
    KillNotes();
}

int ComparePosision(ShipData* a, ShipData* b)
{
    if (((a->attr | b->attr) & RACING) == 0)
        return 0;

    if ((a->attr & RACING) == 0)
        return -1;

    if ((b->attr & RACING) == 0)
        return 1;

    if (a->DPASectDiff < b->DPASectDiff)
    {
        return 1;
    }

    if (a->DPASectDiff > b->DPASectDiff)
    {
        return -1;
    }

    // a->DPASectDiff == b->DPASectDiff
    if (a->nearTrkSect == b->nearTrkSect)
    {
        vec3i ca, cb, dir;
        int32 da, db;
        int32 cx, cy, cz;

        cx = a->nearTrkSect->centre.x;
        cy = a->nearTrkSect->centre.y;
        cz = a->nearTrkSect->centre.z;

        //ASSERT(a->nearTrkSect == b->nearTrkSect);

        ca.x = a->pos.x - cx;
        ca.y = a->pos.y - cy;
        ca.z = a->pos.z - cz;

        cb.x = b->pos.x - cx;
        cb.y = b->pos.y - cy;
        cb.z = b->pos.z - cz;

        dir.x = a->nearTrkSect->nextSection->centre.x - cx;
        dir.y = a->nearTrkSect->nextSection->centre.y - cy;
        dir.z = a->nearTrkSect->nextSection->centre.z - cz;

        da = GetMagnitude(&ca);
        db = GetMagnitude(&cb);

        da = da ? (dot(&ca, &dir) / da) : 0;
        db = db ? (dot(&cb, &dir) / db) : 0;

        if (da < db)
            return 1;

        if (da > db)
            return -1;
    }

    return 0;
}

void CalcPosition(ShipData* shipIndex)
{
    int i, swapped;

    do
    {
        swapped = 0;
        for (i = 1; i < NO_OF_SHIPS; i++)
        {
            if (ComparePosision(gSortedShips[i - 1], gSortedShips[i]) > 0)
            {
                ShipData* t = gSortedShips[i - 1];
                gSortedShips[i - 1] = gSortedShips[i];
                gSortedShips[i] = t;
                swapped = 1;
            }
        }
    } while (swapped);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        gSortedShips[i]->racePosition = i + 1;
    }
}

#ifdef USE_SHIP_GHOST
void replayRecord(PlayerData* player)
{
    uint8 ctrl = player->ctrl;
    uint8 stickX = player->stickX - 1;
    uint8 stickY = player->stickY - 1;
    Replay* replay = &player->replay;

    if ((replay->ctrl != ctrl) || (replay->tick == 255) || (replay->stickX != stickX) || (replay->stickY != stickY))
    {
        if (replay->pos + 4 <= MAX_REPLAY_SIZE)
        {
            replay->size += 4;
            LOG("%d\t%d\n", replay->tick, replay->size);
            replay->ctrl = ctrl;
            replay->tick = 0;
            replay->stickX = stickX;
            replay->stickY = stickY;
        }
    }

    replay->tick++;
}
#endif

void UpdateShipSecNo(ShipData* ship)
{
    int32 secNo;
    int32 startSecNo = startLinePos[gConfig.trackNum];

    if (ship->player)
    {
        if ((ship->prevSecNo == (startSecNo + 1)) && (ship->nearSecNo <= startSecNo))
        {
            ship->crossedLine--;
        }
    }

    if ((ship->prevSecNo == startSecNo) && ((ship->nearSecNo > startSecNo) && (ship->nearSecNo < (startSecNo + 10))))
    {
        if (ship->player)
        {
            if (ship->lapNo == ship->crossedLine)
            {
                ship->lapNo++;

                if (ship->player && (ship->lapNo > 1) && (ship->lapNo <= END_LAP + 1))
                {
                    if (CheckLapRecord((PlayerData*)ship->player))
                    {
                        gConfig.newLapRecord = 1; // TODO ship attr?
                    }
                }
            }

            ship->crossedLine++;
            if (gConfig.raceType == TIME_TRIAL)
            {
                ship->weaponType = WPN_TURBO;
            }
        }
        else
        {
            ship->lapNo++;

            if ((ship->lapNo > END_LAP) && (ship->attr & RACING))
            {
                ship->attr &= ~RACING;
            }
        }
    }

    secNo = ship->nearSecNo - (startSecNo + 1);

    if (ship->player)
    {
        if (secNo < 0)
        {
            secNo = secNo + (ship->crossedLine * NoOfMainSections);
        }
        else
        {
            secNo = secNo + ((ship->crossedLine - 1) * NoOfMainSections);
        }
    }
    else
    {
        if (secNo < 0)
        {
            secNo = secNo + (ship->lapNo * NoOfMainSections);
        }
        else
        {
            secNo = secNo + ((ship->lapNo - 1) * NoOfMainSections);
        }
    }

    ship->DPASectDiff = secNo;
}

void UpdateRaceHud(ShipData* shipIndex, int choice)
{
    int32 i, end;

    if (gConfig.raceType != TIME_TRIAL)
    {
        for (i = 0; i < NO_OF_SHIPS; i++)
        {
            UpdateShipSecNo(shipIndex + i);
        }
        CalcPosition(shipIndex);
    }
    else
    {
        for (i = 0; i < MAX_PLAYERS; i++)
        {
            if (gPlayers[i].ship)
            {
                UpdateShipSecNo(gPlayers[i].ship);
            }
        }
    }

    end = 1;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;
        ShipData* ship = player->ship;
        if (!ship)
            continue;

        if (ship->lapNo > 0)
        {
            if ((choice == -1) && (ship->lapNo < END_LAP + 1))
            {
                player->lapTimes[ship->lapNo - 1] += 2;
            }
        }

        if (ship->lapNo > END_LAP)
        {
            if (ship->attr & RACING)
            {
                ship->update = UpdateRemoteShip;
                ship->updateCount = 0;
                ship->remoteMaxThrust = 3160;
                ship->remoteThrustMag = 32;
                ship->speed = 3160;
                ship->attr &= ~(SHIELDED | VIEW_IN | RACING);

                LOG("replaySize: %d\n", player->replay.size);

                player->camera.update = UpdateCameraAttractMode;
            }
        }
        else
        {
            end = 0;
        }
    }

    if (end)
    {
        raceEnd();
    }
}

void raceInit(void)
{
    int32 i;

    LOG("race init\n");

    //raceDemo(); // demo debug

    showLoading(gTrackList[gConfig.trackNum]);

    attractDelay = 0;

    gConfig.newLapRecord = 0;

    ctrl_says_quit = 0;

    ctrlchoice = -1;
    ctrlplayCredits = 0;
    ctrldoneExtro = START_EXTRO;
    ctrlgameOverTime = 0;
    ctrlshowMenu = 1;
    ctrlconfirm = 0;
    ctrllConfirm = 0;
    ctrlchoiceData = -1;
    ctrlshowGameOver = 0;
    finishedExtro = 0;

    LoadScene(gTrackList[gConfig.trackNum]);
    LoadWinLoseTex(PILOT_NAMES[gPlayers[0].shapeIndex]);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ctrlshipIndex[i].player = NULL;
        gSortedShips[i] = ctrlshipIndex + i;
    }

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;

        if (player->shapeIndex != -1)
        {
            player->ship = ctrlshipIndex + i;
            player->ship->player = player;
        }
        else
        {
            player->ship = NULL;
        }
    }

    Restart(ctrlshipIndex);

    /* Initialize hi-score and game */

    InitHiScoreData(&ctrlhiScoreData);

    gConfig.lives = 3;
    gConfig.gameOverCount = 0;

    ctrlhiScoreData.confirm = 2;
    ctrlhiScoreData.lConfirm = 0;

    if (ctrlmode == ATTRACT)
    {
        ShipData* ship = gPlayers[0].ship;
        ASSERT(gNumPlayers == 1);

        ship->update = UpdatePlayerShipNormStart;
        ship->updateCount = START_TIME;
        ship->remoteMaxThrust = 3160;
        ship->remoteThrustMag = 32;
        ship->speed = 3160;
        ship->attr &= ~VIEW_IN;
        ship->attr &= ~RACING;

        gPlayers[0].camera.update = UpdateCameraAttractMode;

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

    if (gConfig.trackNum == 12)
    {
        InitIceCongratulations();
    }

    if (gConfig.trackNum == 13)
    {
        InitFireCongratulations();
    }

    UpdateEffects();
}

void raceCtrl(void)
{
    int32 i;
    PlayerData* player = gPlayers; // TODO other players
    int32 Position = player->ship->racePosition;

    // Reset all of my little Drawing markers // TODO HUD states instead of this shit
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

    //shipIndex[0].weaponType = WPN_ROCKET; // fx debug

    UpdatePad();

    if (gPlayers[0].ship->attr & RACING) // TODO other players
    {
        if ((GetMenuKey(M_START)) && (ctrlchoice == -1))
        {
            if (gConfig.lives)
            {
                PlayNote(SFX_MENU_SELECT); // go boing
                ctrlchoice = 0; // set ctrlchoice to the first on the list
                gPlayers[0].xpad = 0; // clear the status of the pad so that the next bit does not select

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

                    if (gConfig.raceType == CHAMPIONSHIP) // Chanpionship mode, then lose a life for the prividge
                    { // of restarting your race.
                        gConfig.lives--;
                        if (gConfig.lives == 0)
                        {
                            ctrlgameOverTime = 90; // 3 second
                            ctrlshowGameOver = 1;
                            gPlayers[0].xpad = 0;
                            break;
                        }
                    }
                    Restart(ctrlshipIndex);
                    break;

                case 2:
                    ctrlchoice = -1;
                    ctrlchoiceData = 2;
                    QuitGame();
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
            gConfig.newLapRecord = 0;
        }
        ctrlgameOverTime--;

        if (ctrlshowGameOver == 0)
        {
            QuitGame();
            ctrl_says_quit = 1;
            return;
        }
    }

    if ((ctrlchoice == -1) && (ctrlshowGameOver == 0))
    {
        UpdateShips(ctrlshipIndex);

        for (i = 0; i < MAX_PLAYERS; i++)
        {
            PlayerData* player = gPlayers + i;
            if (!player->ship)
                continue;

            { // update droid
                FRAME_LERP_INIT(&player->droid);
                (player->droid.update)(&player->droid);
            }

            { // update camera
                FRAME_LERP_INIT(&player->camera);

                if (ctrldoneExtro == UPDATE_EXTRO)
                {
                    (player->camera.update)(&player->camera, ctrlshipIndex, &player->droid); // TODO just disable other ships at setting extro flag
                }
                else
                {
                    (player->camera.update)(&player->camera, player->ship, &player->droid);
                }
            }

            { // update targeting and shake effects
                ShipData* ship = player->ship;

                if (ship->attr & RACING)
                {
                    replayRecord(player);

                    if ((ship->weaponType == WPN_MISSILE) || (ship->weaponType == WPN_ELEC_BOLT) || (ship->weaponType == WPN_REV_CON))
                    {
                        MissileLock(ship, ctrlshipIndex);
                    }
                    ShakeUpdate(ship);
                }
            }
        }

        UpdateCone(&ctrlcone);

        if (gConfig.raceType != TIME_TRIAL)
        {
            UpdateWeapons(ctrlshipIndex);
            CallShip2ShipCollision(ctrlshipIndex);
            UpdateWeaponGrids(ctrlshipIndex, gTrack.weapons, gTrack.weaponsCount);
        }

        UpdateEffects();

        SetVolume(SFX_SHIELD, SND_MAX_VOLUME, SND_MAX_VOLUME);

        AdjustEngineSound(gPlayers[0].ship, &gPlayers[0].camera);
        AdjustCrowdSound(gTrack.lost, gTrack.lostCount);
        AdjustRemoteEngineSound(ctrlshipIndex);
    }

    if ((attractDelay >= 0) && (ctrlmode == ATTRACT))
    {
        attractDelay--;
        if ((GetMenuKey(M_START)) || (GetMenuKey(M_SELECT)) || (attractDelay == 0))
        {
            QuitGame();
            ctrl_says_quit = 1;
            return;
        }
    }
    else if (attractDelay > 0)
    {
        attractDelay--;

        if (attractDelay == 0 && gConfig.raceType == SINGLE_RACE)
        {
            attractDelay = 1;
            if (gConfig.showPilotCount != 0)
            {
                ctrlNeedWinLose = 1;
                ctrlNeedEndText = 1;
                ctrlNeedBackPoly = 1;
                ctrlNeedRaceInfo = 1;
                ctrlNeedDeSelectScreen = 1;

                if (Position == 1)
                {
                    ctrlNeedEndTextVar = 1;
                    ctrlNeedWinLoseVar = 1;
                }
                else
                {
                    ctrlNeedEndTextVar = 2;
                    ctrlNeedWinLoseVar = 0;
                }

                ajscount++;
                if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                {
                    gConfig.showPilotCount = 0;
                    ajscount = 0;
                }
            }
            else
            {
                gConfig.newLapRecord = 0;

                if (gConfig.EndOfGame)
                {
                    if (Position == 1)
                    {
                        ctrlhiScoreData.highScore = CheckHiScore(&ctrlhiScoreData, player);
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
                    NewHighScore(&ctrlhiScoreData);
                    ctrlNeedDeSelectScreen = 1;
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

                        Restart(ctrlshipIndex);
                    }
                }
            }
        }

        if (attractDelay == 0 && gConfig.raceType == TIME_TRIAL)
        {
            attractDelay = 1;

            if (gConfig.raceType == TIME_TRIAL)
            {

                if (gConfig.showPilotCount != 0)
                {

                    ctrlNeedRaceInfo = 1;
                    ctrlNeedDeSelectScreen = 1;

                    ajscount++;
                    if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                    {
                        gConfig.showPilotCount = 0;
                        ajscount = 0;
                    }
                }
                else
                {
                    gConfig.newLapRecord = 0;
                    if (gConfig.EndOfGame)
                        ctrlhiScoreData.highScore = CheckHiScore(&ctrlhiScoreData, player);

                    if (ctrlhiScoreData.highScore)
                    {
                        ctrlhiScoreData.displayHiScore = 1;
                        ctrlhiScoreData.highScore = 0;
                    }

                    if (ctrlhiScoreData.displayHiScore)
                    {
                        ctrlNeedNewHighScores = 1;
                        NewHighScore(&ctrlhiScoreData);
                        ctrlNeedDeSelectScreen = 1;
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
                            Restart(ctrlshipIndex);
                        }
                    }
                }
            }
        }

        /****************** Do Extro ******************/
        if (attractDelay == 0 && gConfig.raceType == CHAMPIONSHIP)
        {
            attractDelay = 1;

            if (gConfig.raceType == CHAMPIONSHIP)
            {

                if (gConfig.lives == 0) /*0*/
                {
                    if (gConfig.gameOverCount != 0)
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
                            gConfig.showGameOver = 1;
                            ctrlgameOverTime = 90; // 3 seconds
                            gConfig.gameOverCount = 0;
                            gConfig.newLapRecord = 0;
                        }
                    }
                    else if (gConfig.gotoMenu == 1)
                    {
                        gConfig.gotoMenu = 0;
                        gConfig.newLapRecord = 0;
                        QuitGame();
                        ctrl_says_quit = 1;
                        return;
                    }
                    else if (gConfig.showGameOver == 1)
                    {
                        ajscount++;
                        if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                        {
                            ajscount = 0;
                            gConfig.gotoMenu = 1;
                            gConfig.showGameOver = 0;
                            gConfig.newLapRecord = 0;
                        }
                    }
                }
                else
                {
                    if (gConfig.showPilotCount != 0)
                    {
                        ctrlNeedWinLose = 1;
                        ctrlNeedEndText = 1;
                        ctrlNeedBackPoly = 1;
                        ctrlNeedRaceInfo = 1;
                        ctrlNeedDeSelectScreen = 1;

                        if (Position >= 1 && Position <= 3)
                        {
                            ctrlNeedEndTextVar = 1;
                            ctrlNeedWinLoseVar = 1;
                        }
                        else
                        {
                            ctrlNeedEndTextVar = 3;
                            ctrlNeedWinLoseVar = 0;
                        }

                        ajscount++;
                        if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                        {
                            ajscount = 0;
                            gConfig.showPilotCount = 0;
                            gConfig.newLapRecord = 0;
                        }
                    }
                    else if (gConfig.showPointCount != 0)
                    {
                        if (Position >= 1 && Position <= 3)
                        {
                            ctrlNeedRacePoints = 1;
                            ctrlNeedDeSelectScreen = 1;
                        }
                        ajscount++;
                        if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                        {
                            ajscount = 0;
                            gConfig.showPointCount = 0;
                        }
                    }
                    else if (gConfig.showChampCount != 0)
                    {
                        if (Position >= 1 && Position <= 3)
                        {
                            ctrlNeedChampTable = 1;
                            ctrlNeedDeSelectScreen = 1;
                        }
                        ajscount++;
                        if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                        {
                            ajscount = 0;
                            gConfig.showChampCount = 0;
                        }
                    }
                    else if (gConfig.venomComplete != 0)
                    {
                        if (gConfig.showBonusTrack == 1)
                            ctrlNeedAllVenomComplete = 1;
                        else
                            ctrlNeedVenomComplete = 1;

                        ctrlNeedDeSelectScreen = 1;

                        ajscount++;
                        if ((ajscount > (AJSDELAY / 2)))
                        {
                            ajscount = 0;
                            gConfig.venomComplete = 0;
                        }
                    }
                    else
                    {
                        if (Position >= 1 && Position <= 3)
                        {
                            if (gConfig.EndOfGame)
                                ctrlhiScoreData.highScore = CheckHiScore(&ctrlhiScoreData, player);
                        }

                        if (ctrlhiScoreData.highScore)
                        {
                            ctrlhiScoreData.displayHiScore = 1;
                            ctrlhiScoreData.highScore = 0;
                        }

                        if (ctrlhiScoreData.displayHiScore)
                        {
                            ctrlNeedNewHighScores = 1;
                            NewHighScore(&ctrlhiScoreData);
                            ctrlNeedDeSelectScreen = 1;
                        }
                        else
                        {
                            if (gConfig.lostGame == 1)
                            {

                                ajscount++;
                                if ((GetMenuKey(M_SELECT)) || (ajscount > AJSDELAY))
                                {
                                    ajscount = 0;
                                    gConfig.lostGame = 0;
                                    gConfig.gotoMenu = 1;
                                }
                            }
                            else
                            {
                                if (gConfig.gameComplete == 1)
                                {
                                    if (ctrldoneExtro == START_EXTRO)
                                    {
                                        int i;
                                        gPlayers[0].camera.update = UpdateCameraExtroAttract;
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
                                        gConfig.gotoMenu = 0;
                                        InitChampionship();
                                        gConfig.gameComplete = 0;
                                        QuitGame();
                                        ctrl_says_quit = 1;
                                        return;
                                    }
                                }
                                else
                                {

                                    if (gConfig.gotoMenu != 0)
                                    {
                                        gConfig.gotoMenu = 0;
                                        InitChampionship();
                                        gConfig.gameComplete = 0;
                                        QuitGame();
                                        ctrl_says_quit = 1;
                                        return;
                                    }

                                    InitHiScoreData(&ctrlhiScoreData);
                                    if (Position >= 1 && Position <= 3)
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
                                            QuitGame();
                                            ctrl_says_quit = 1;
                                            return;
                                        }

                                        if (ctrlhiScoreData.confirm == 0)
                                        {
                                            attractDelay = 0;
                                            ctrlhiScoreData.confirm = 2;
                                            ctrlhiScoreData.lConfirm = 0;
                                            Restart(ctrlshipIndex);
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
        UpdateRaceHud(ctrlshipIndex, ctrlchoice);
    }
}

void raceLoop(void)
{
    int32 i, viewIndex, count;
    int32 w, h;

    count = gFrameIndex - gLastFrameIndex;

    if (count > 10) // max frame skip
    {
        count = 10;
    }

    for (i = 0; i < count; i++)
    {
        dbg_update = osGetSystemTimeMS();
        raceCtrl();
        dbg_update = osGetSystemTimeMS() - dbg_update;
    }

    if (ctrlNeedInGameReset || ctrlNeedInGameConfirm)
    {
        gFrameDelta = 255;
    }

    renderFrameBegin();
    renderClear();

    if (gConfig.trackNum == 12)
    {
        AuroraBorialis(gTrack.sky);
    }

    if ((gConfig.trackNum == 4) || (gConfig.trackNum == 11))
    {
        ColourRedLight();
        MoveDonkey();
    }
    else if (gConfig.trackNum == 2)
    {
        ColourRedLight();
    }

    ColourDroidLights(ctrlshipShapes + RESCUE_DROID);

    if (gNumPlayers == 1)
    {
        renderSetView(-1);
        UpdateGameDisplay(gWidth, gHeight);
        DrawView(0);
    }
    else
    {
        viewIndex = 0;
        w = gWidth >> 1;
        h = (gNumPlayers > 2) ? (gHeight >> 1) : gHeight;

        for (i = 0; i < MAX_PLAYERS; i++)
        {
            if (!gPlayers[i].ship)
                continue;
            renderSetView(viewIndex++);
            UpdateGameDisplay(w, h);
            DrawView(i);
        }
        renderSetView(-1);
        UpdateGameDisplay(gWidth, gHeight);
    }

    renderSetMode(RENDER_MODE_2D);

    if (ctrlNeedDeSelectScreen)
        renderBackFill(guiL, guiT, guiR - guiL, guiB - guiT);

    if (ctrlNeedBackPoly)
        renderBackFill(180, 50, 128, 140);

    if (ctrlmode == ATTRACT)
    {
        if (ctrlplayCredits == 1)
            ShowCredits(attractDelay);
        else
            CentreText(STR_DEMO_MODE, text_data(0, 14, 8), TEXT_COLOR_HIGHLIGHT);
    }

    // All game overs
    if ((ctrlshowGameOver == 1) || (gConfig.showGameOver == 1) || (gConfig.lostGame == 1))
        CentreText(STR_GAME_OVER, text_data(0, 112, 16), TEXT_COLOR_HIGHLIGHT);

    // Text at end.
    if (ctrlNeedEndText)
    {
        switch (ctrlNeedEndTextVar)
        {
            case 1:
                CentreText(STR_WIN, text_data(0, 20, 8), TEXT_COLOR_HIGHLIGHT);
                break;
            case 2:
                CentreText(STR_LOSE, text_data(0, 20, 8), TEXT_COLOR_HIGHLIGHT);
                break;
            case 3:
                CentreText(STR_FAIL, text_data(0, 20, 8), TEXT_COLOR_HIGHLIGHT);
                break;
        }
    }
    // Pictures at end
    if (ctrlNeedWinLose)
        DisplayWinLose(ctrlNeedWinLoseVar);
    // High score bollox
    if (ctrlNeedRaceInfo)
        DisplayRaceInfo(&ctrlhiScoreData);
    if (ctrlNeedNewHighScores)
        NewHighScoreDraw(&ctrlhiScoreData);
    if (ctrlNeedRacePoints)
        DisplayRacePoints();
    if (ctrlNeedChampTable)
        DisplayChampTable();
    if (ctrlNeedAllVenomComplete)
        DisplayAllVenomComplete();
    if (ctrlNeedVenomComplete)
        DisplayVenomComplete();
    if (ctrlNeedContinueGameConfirm)
        DrawContinueGameConfirm(ctrlhiScoreData.lConfirm);
    if (ctrlNeedInSingleGameConfirm)
        DrawRestartGameConfirm(ctrlhiScoreData.lConfirm);
    if (ctrlNeedInGameReset)
        InGameReset(ctrlchoice);
    if (ctrlNeedInGameConfirm)
        InGameConfirm(ctrlchoice, &ctrllConfirm);

    if (startIceText || startFireText)
    {
        ShowCongratulations();
    }

    renderFrameEnd();

    if (ctrl_says_quit)
    {
        ctrlmode = INTERACTIVE;

        ResetCone(&ctrlcone);
        ctrlGameLoop = GAME_LOOP_MAIN;
        return;
    }
}

void raceDemo(void)
{
    int32 i;

    ctrlmode = ATTRACT;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;
        player->shapeIndex = -1;
    }
    gPlayers[0].shapeIndex = randy() % NO_OF_SHIPS;

    gConfig.raceType = SINGLE_RACE;
    gConfig.trackNum = (randy() % 6); // all venom
    ctrlGameLoop = GAME_LOOP_RACE_INIT;
}

void raceEnd(void)
{
    static const uint8 RACE_POINTS[] = { 0, 9, 7, 5, 3, 2, 1, 0, 0 };
    SortTable sortTable[NO_OF_SHIPS];
    int32 i, j, swapped;
    int32 position;
    ShipData* ship;
    PlayerData* player = gPlayers; // TODO other players

    if (gConfig.EndOfGame)
        return;

    gConfig.EndOfGame = 1;

    gConfig.showPilotCount = 120;
    attractDelay = 300;

    if (gConfig.raceType != CHAMPIONSHIP)
        return;

    position = player->ship->racePosition;

    attractDelay = 1;

    if ((position >= 1 && position <= 3) && gConfig.lives > 0)
    {
        /* Find position during race */
        for (i = 0; i < NO_OF_SHIPS; i++)
        {
            ship = ctrlshipIndex + i;

            ship->racePoints = RACE_POINTS[ship->racePosition];
            ship->totalPoints += ship->racePoints;

            sortTable[i].shipNum = i;
            sortTable[i].points = ship->totalPoints;
        }

        do
        {
            swapped = 0;
            for (i = 1; i < NO_OF_SHIPS; i++)
            {
                if (sortTable[i - 1].points < sortTable[i].points)
                {
                    // swap ship index
                    j = sortTable[i].shipNum;
                    sortTable[i].shipNum = sortTable[i - 1].shipNum;
                    sortTable[i - 1].shipNum = j;
                    // swap points
                    j = sortTable[i].points;
                    sortTable[i].points = sortTable[i - 1].points;
                    sortTable[i - 1].points = j;

                    swapped = 1;
                }
            }
        } while (swapped);

        for (i = 0; i < NO_OF_SHIPS; i++)
        {
            winTable[sortTable[i].shipNum].position = (i + 1);
        }

        gConfig.showPointCount = 120;
        gConfig.showChampCount = 120;
    }

    gConfig.gameOverCount = 120;

    if (position > 3)
    {
        gConfig.lives--;
        gConfig.trackInc = 0;
    }
    else
    {
        if (gConfig.trackNum == NumTracks - 1)
        {
            if (position == 1)
            {
                gConfig.rapierClass = 1;
                gConfig.raceClass = 1;
                if (gConfig.showBonusTrack == 0)
                {
                    gConfig.trackNum += 2;
                    gConfig.trackInc = 2;
                }
                else
                {
                    gConfig.trackNum++;
                    gConfig.trackInc = 1;
                }
                gConfig.venomComplete = 1;
                gConfig.gotoMenu = 1;
            }
            else
            {
                gConfig.lostGame = 1;
                gConfig.trackInc = 0;
            }
        }
        else
        {
            if (gConfig.trackNum == totalTracks - 1)
            {
                if (position == 1)
                {
                    gConfig.gameComplete = 1;
                    totalTracks = 14;
                    NumTracks = 7;
                    gConfig.showBonusTrack = 1;
                }
                else
                {
                    gConfig.lostGame = 1;
                    gConfig.trackInc = 0;
                }
            }
            else
            {
                gConfig.trackInc = 1;
                gConfig.trackNum++;
            }
        }
    }
}
