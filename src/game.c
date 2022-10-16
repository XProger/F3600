#include "game.h"

long ctrlmode;
long ctrltimeout;
long ctrlGameLoop;
ConfigData ctrlConfigData;

extern short trackNum; // SJR
extern long ctrlfirsttime;

Texture* titleScreen;
int gTimeout;

extern Object* ctrlshipShapes[(NO_OF_SHIPS * 2) + 1];
extern ShipData ctrlshipIndex[NO_OF_SHIPS];
extern const TrackCamera ctrlcamera;

char* pDRAM_checkpoint;
char* pVRAM_checkpoint;

void showLoading(int32 track)
{
    Texture* loadingScreen;
    char* vramPtr = pVRAM;
    
    if (track == -1)
    {
        loadingScreen = LoadTexture("wipeout/textures/wiptitle.tim");
    }
    else
    {
        char path[32];
        strcpy(path, "wipeout/textures/load");
        if (track <= 9)
        {
            path[21] = '0' + track;
            path[22] = 0;
        }
        else
        {
            path[21] = '1';
            path[22] = '0' + (track - 10);
            path[23] = 0;
        }
        strcat(path, ".tim");

        loadingScreen = LoadTexture(path);
    }

    renderFrameBegin();
    renderSetMode(RENDER_MODE_2D);
    renderBackground(loadingScreen);
    CentreText("LOADING", text_data(0, 200, 8), TEXT_COLOR_WHITE);
    renderFrameEnd();
    renderSwap();

    renderDeleteTexture(loadingScreen);

    pVRAM = vramPtr;
}

void titleInit(void)
{
    titleScreen = LoadTexture("wipeout/textures/wiptitle.tim");
}

void titleLoop(void)
{
    UpdatePad();

    renderFrameBegin();
    renderClear();
    renderSetMode(RENDER_MODE_2D);

    renderBackground(titleScreen);

    CentreText("PRESS ANY KEY", text_data(0, 200, 8), TEXT_COLOR_WHITE);

    if (xpad)
    {
        PlayNote(SFX_MENU_SELECT);
        ctrlGameLoop = GAME_LOOP_MENU_INIT;
    }

    gTimeout -= (gFrameIndex - gLastFrameIndex);
    if (gTimeout <= 0)
    {
        raceDemo(&ctrlConfigData);
    }

    renderFrameEnd();
}

void gameInit(void)
{
    renderInit();

    InitGameDisplay();

    TextureTableCount = 0;
    InitText();

    showLoading(-1);

    soundInit();
    SfxInit();
    InitNotes();

    gMusicTracksCount = 11;
    gMusicTrack = 0; // random

    SetMasterBalance(160); // ~60%
    HighScore(0);

    InitGameData(&ctrlConfigData);

    /*TODO saves
       LoadCardData(lfilename, 0, &ctrlConfigData, (SelectionData *) 0);
    */

    ctrlConfigData.Lives = 3;
    ctrlConfigData.EndOfGame = 0;
    ctrlConfigData.Position = 8;

    /*TODO cheats*/
#if 1
    ctrlConfigData.RapierClass = 1;
    totalTracks = 14;
    NumTracks = 7;
    ctrlConfigData.showBonusTrack = 1;
#endif

    InitPadSettings();
    InitChampionship();

    ctrltimeout = FR30 * 120;

    trackNum = 16; // SJR

    ctrlGameLoop = GAME_LOOP_MAIN;

    if (ctrlfirsttime)
        gTimeout = 60 * 30;
    else
        gTimeout = 60 * 5;

    loadShipData(ctrlshipIndex, ctrlshipShapes, &gCamPos);
    InitSpeedo();
    InitWeaponSprtIcons();
    InitEffects();
    InitWeapons(ctrlweaponShapes);

    gBaseTextureIndex = TextureTableCount;

    pDRAM_checkpoint = pDRAM;
    pVRAM_checkpoint = pVRAM;
}

void gameFree(void)
{
    renderFree();
    SfxFree();
}

void gameCtrl(void)
{
    switch (ctrlGameLoop)
    {
        case GAME_LOOP_TITLE_INIT:
            MusicReset();
            titleInit();
            ctrlGameLoop = GAME_LOOP_TITLE;
        case GAME_LOOP_TITLE:
            titleLoop();
            break;

        case GAME_LOOP_MENU_INIT:
            pDRAM = pDRAM_checkpoint;
            pVRAM = pVRAM_checkpoint;
            MusicReset();
            menuInit();
            ctrlGameLoop = GAME_LOOP_MENU;
        case GAME_LOOP_MENU:
            menuLoop(&ctrlConfigData);
            break;

        case GAME_LOOP_RACE_INIT:
            pDRAM = pDRAM_checkpoint;
            pVRAM = pVRAM_checkpoint;
            MusicReset();
            raceInit(&ctrlConfigData);
            renderPrepare();
            ctrlGameLoop = GAME_LOOP_RACE;
        case GAME_LOOP_RACE:
            raceLoop(&ctrlConfigData);
            break;
    }

    if (!MusicIsPlaying())
    {
        if (gMusicTrack == 0) // random
        {
            MusicPlay(1 + (randy() % gMusicTracksCount));
        }
        else
        {
            MusicPlay(gMusicTrack);
        }
    }
}

