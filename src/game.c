#include "game.h"

Texture* titleScreen;
int gTimeout;

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

    renderOffset(0, 0);

    renderFrameBegin();
    renderClear();
    renderSetMode(RENDER_MODE_2D);

    renderBackground(loadingScreen);

    renderFrameEnd();
    renderSwap();

    renderDeleteTexture(loadingScreen);

    gTexturePtr = texPtr;
    pVRAM = vramPtr;
}

void resetTimer()
{
    gLastFrameIndex = gFrameIndex - 1;
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

    CentreText(STR_PRESS_ANY_KEY, text_data(0, 200, 8), TEXT_COLOR_WHITE);

    if (gPlayers[0].xpad)
    {
        PlayNote(SFX_MENU_SELECT);
        ctrlGameLoop = GAME_LOOP_MENU_INIT;
    }

    gTimeout -= (gFrameIndex - gLastFrameIndex);
    if (gTimeout <= 0)
    {
        raceDemo();
    }

    renderFrameEnd();
}

void gameInit(void)
{
    int32 time;

    renderInit();

    showLoading(-1); // 1s

    // 3.4s -> 2.3s (uncomp) -> 1.7s (conv tex) -> 1.5s (conv prm)
    time = osGetSystemTimeMS();
    soundInit();
    SfxInit();
    InitNotes();

    gMusicTracksCount = 11;
    gMusicTrack = 0; // random

    SetMasterBalance(160); // ~60%
    HighScore();

    InitGameData();

    /*TODO saves
       LoadCardData(lfilename, 0, &gConfig, (SelectionData *) 0);
    */

    /*TODO cheats*/
#if 1
    gConfig.rapierClass = 1;
    totalTracks = 14;
    NumTracks = 7;
    gConfig.showBonusTrack = 1;
#endif

    InitChampionship();

    ctrlGameLoop = GAME_LOOP_MAIN;

    gTimeout = 60 * 30;

    LoadCommon();

    memStateSave();

    LOG("init main %d ms\n", osGetSystemTimeMS() - time);
}

void gameFree(void)
{
    SfxFree();
    renderFree();
}

void gameCtrl(void)
{
    int32 time;

    UpdateGameDisplay(gWidth, gHeight);

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
            LOG("init menu %d ms\n", osGetSystemTimeMS() - time);
            ctrlGameLoop = GAME_LOOP_MENU;
            resetTimer();
        case GAME_LOOP_MENU:
            menuLoop();
            break;

        case GAME_LOOP_RACE_INIT:
            memStateRestore();
            MusicReset();
            time = osGetSystemTimeMS();
            raceInit(); // 14s -> 10.3s (uncomp) -> 7.9s (conv tex) -> 6.9s (conv prm) -> 1.4s (obj vis opt)
            LOG("race init %d ms\n", osGetSystemTimeMS() - time);
            renderPrepare();
            ctrlGameLoop = GAME_LOOP_RACE;
            resetTimer();
        case GAME_LOOP_RACE:
            raceLoop();
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
