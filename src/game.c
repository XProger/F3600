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

void titleInit()
{
    titleScreen = LoadTexture("wipeout\\textures\\wiptitle.tim");
    if (ctrlfirsttime)
        gTimeout = 60 * 30;
    else
        gTimeout = 60 * 5;
}

void titleLoop()
{
    int i;
    UpdatePad();

    renderFrameBegin();
    renderClear();
    renderSetMode(RENDER_MODE_2D);

    AddGlyph(0, 0, 320, 240, 0, 0, titleScreen);
    CentreText("PRESS ANY KEY", text_data(0, 200, 8), WhiteText);

    if (pad)
    {
        PlayNote(SFX_MENU_SELECT);
        ctrlmode = INTERACTIVE;
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
    InitGameDisplay();

    SfxInit();
    InitSound();

    gMusicTracksCount = 11;
    gMusicTrack = 0; // random
    gMasterBalance = 160; // ~60%

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

    ctrlmode = ATTRACT;

    SetCharTextures(8);
    SetCharTextures(12);
    SetCharTextures(16);

    trackNum = 16; // SJR

    ctrlGameLoop = GAME_LOOP_TITLE_INIT;

    loadShipData(ctrlshipIndex, ctrlshipShapes, &gCamPos);

    gBaseTextureIndex = TextureTableCount;
}

void gameCtrl(void)
{
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

