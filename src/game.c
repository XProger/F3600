#include "game.h"

int32 ctrlmode;
int32 ctrltimeout;
int32 ctrlGameLoop;
ConfigData ctrlConfigData;

extern int32 ctrlfirsttime;

Texture* titleScreen;
int gTimeout;

extern Object* ctrlshipShapes[(NO_OF_SHIPS * 2) + 1];
extern ShipData ctrlshipIndex[NO_OF_SHIPS];

void showLoading(int32 track)
{
    Texture* loadingScreen;
    Texture* texPtr = gTexturePtr;
    uint8* vramPtr = pVRAM;

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
    if (track != -1)
    {
        CentreText("LOADING", text_data(0, 200, 8), TEXT_COLOR_WHITE);
    }
    renderFrameEnd();
    renderSwap();

    renderDeleteTexture(loadingScreen);

    gTexturePtr = texPtr;
    pVRAM = vramPtr;
}

void resetTimer()
{
    gLastFrameIndex = gFrameIndex - 2;
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
    int32 time;
#
    renderInit();

    renderOffset(0, 0);

    showLoading(-1); // 1s

    // 3.4s -> 2.3s (uncomp) -> 1.7s (conv tex) -> 1.5s (conv prm)
    time = osGetSystemTimeMS();
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

    ctrlGameLoop = GAME_LOOP_MAIN;

    if (ctrlfirsttime)
        gTimeout = 60 * 30;
    else
        gTimeout = 60 * 5;

    LoadCommon();

    memStateSave();

    printf("init main %d ms\n", osGetSystemTimeMS() - time);
}

void gameFree(void)
{
    SfxFree();
    renderFree();
}

void gameCtrl(void)
{
    int32 time;

    switch (ctrlGameLoop)
    {
        case GAME_LOOP_TITLE_INIT:
            MusicReset();
            titleInit();
            ctrlGameLoop = GAME_LOOP_TITLE;
            resetTimer();
        case GAME_LOOP_TITLE:
            titleLoop();
            break;

        case GAME_LOOP_MENU_INIT:
            memStateRestore();
            MusicReset();
            time = osGetSystemTimeMS();
            menuInit(); // 4.7s -> 3.1s (uncomp) -> 2.1s (conv tex) -> 2.2s (conv prm)
            printf("init menu %d ms\n", osGetSystemTimeMS() - time);
            ctrlGameLoop = GAME_LOOP_MENU;
            resetTimer();
        case GAME_LOOP_MENU:
            menuLoop(&ctrlConfigData);
            break;

        case GAME_LOOP_RACE_INIT:
            memStateRestore();
            MusicReset();
            time = osGetSystemTimeMS();
            raceInit(&ctrlConfigData); // 14s -> 10.3s (uncomp) -> 7.9s (conv tex) -> 6.9s (conv prm) -> 1.4s (obj vis opt)
            printf("race init %d ms\n", osGetSystemTimeMS() - time);
            renderPrepare();
            ctrlGameLoop = GAME_LOOP_RACE;
            resetTimer();
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
