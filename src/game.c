#include <windows.h>
#include "game.h"

long ctrlmode;
long ctrltimeout;
long ctrlGameLoop;
ConfigData ctrlConfigData;

extern short trackNum; // SJR
extern int menuTexturesIn;
extern long ctrlfirsttime;

char trackdir[21];

Texture* titleScreen;
int gTimeout;

void AddQuad(int x, int y, int w, int h, int u0, int v0, int u1, int v1, int clut, int page);

void titleInit()
{
    titleScreen = LoadTexture("wipeout\\textures\\wiptitle.tim", 0);
    if (ctrlfirsttime)
        gTimeout = 60 * 5;
    else
        gTimeout = 60 * 30;
}

void titleLoop()
{
    int i;

    /*TODO title screen*/
    // AddGlyph(0, 0, 320, 240, titleScreen->u0, titleScreen->v0, titleScreen->cba);
    CentreText("PRESS START", text_data(0, 200, 8), RedText);

    if (GetMenuKey(M_SELECT))
    {
        PlayNote(MENU_SELECT, 1, 20);
        for (i = 0; i < 10; i++)
        {
            UpdateNotes(NOT_PAUSED);
        }
        ctrlmode = INTERACTIVE;
        ctrlGameLoop = GAME_LOOP_MENU_INIT;
    }

    gTimeout -= (gFrameIndex - gLastFrameIndex);
    if (gTimeout <= 0)
    {
        raceDemo(&ctrlConfigData);
    }
}

void gameInit()
{
    InitGameDisplay();

    InitSpuSound();

    SetCharTextures(8);
    SetCharTextures(12);
    SetCharTextures(16);
    HighScore(0);

    TextureTableCount = 0;
    InitText();

    InitGameData(&ctrlConfigData);

    /*TODO saves
       LoadCardData(lfilename, 0, &ctrlConfigData, (SelectionData *) 0);
    */

    gameType = SINGLE;

    ctrlConfigData.Lives = 3;
    ctrlConfigData.EndOfGame = 0;
    ctrlConfigData.continueConfirm = 1;
    ctrlConfigData.Position = 8;

    /*TODO cheats*/
    /* Enable cheat permanently */
    // gameData.RapierClass = 1 ;
    // totalTracks = 14 ;
    // NumTracks = 7 ;
    // gameData.showBonusTrack = 1 ; //SJR

    SfxSetVolume((char)((0x3fff - (75 * ctrlConfigData.currSliderPos)) >> 6));

    InitPadSettings();
    InitChampionship();

    ctrltimeout = FR30 * 120;

    ctrlmode = ATTRACT;

    SetCharTextures(8);
    SetCharTextures(12);
    SetCharTextures(16);

    trackNum = 16; // SJR

    ctrlGameLoop = GAME_LOOP_TITLE_INIT;
}

void gameCtrl()
{
    opad = pad;
    pad = gPad;
    xpad = (pad ^ opad) & pad;

    switch (ctrlGameLoop)
    {
        case GAME_LOOP_TITLE_INIT:
            titleInit();
            ctrlGameLoop = GAME_LOOP_TITLE;
        case GAME_LOOP_TITLE:
            titleLoop();
            break;

        case GAME_LOOP_MENU_INIT:
            menuInit();
            ctrlGameLoop = GAME_LOOP_MENU;
        case GAME_LOOP_MENU:
            menuLoop(&ctrlConfigData);
            break;

        case GAME_LOOP_RACE_INIT:
            raceInit(&ctrlConfigData);
            renderPrepare();
            ctrlGameLoop = GAME_LOOP_RACE;
        case GAME_LOOP_RACE:
            raceLoop(&ctrlConfigData);
            break;
    }
}

