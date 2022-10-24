#include "game.h"

extern short trackNum; // SJR
extern long ctrlmode;
extern long ctrlGameLoop;

Texture* menuScreen;

extern HiScores scoreTable[];

extern const short shipShapeOrder[];
const short pilotShapeOrder[8] = { 0, 4, 6, 7, 2, 5, 1, 3 };
const short teamShapeOrder[4] = { 2, 3, 1, 0 }; // jed
ChampionShip winTable[NO_OF_SHIPS];
int modelHdg = 0;
int zoomInOut = 0;
int downUp = 0;

Texture* buttonTex[18];
Texture* slideTex;
Texture* sliderTex;

char CheckKey(unsigned short code);

#define NONE        0
#define LEFT        1
#define RIGHT       2
#define UP          3
#define DOWN        4
#define BRAKEL      5
#define BRAKER      6
#define ACCEL       7
#define FIRE        8
#define VIEW_KEY    9
#define PAUSE_KEY   10

char KeyToChange = NONE;

int gCurrMenu;
int gCurrOption;

extern long ctrlfirsttime;
extern Object* ctrlshipShapes[(NO_OF_SHIPS * 2) + 1];

Object* audioShapes[3];
Object* pilotShapes[8];
Object* teamShapes[4];

Object* clsShapes[3];
Object* gameShapes[4];
Object* optShapes[7];
Object* padShapes[3];
SelectionData selectData;

Texture* trackTex[7];

void DrawControllerMenu(const Menu* menu, int shapeNo);
void DrawControllerEdit(const Menu* menu, int shapeNo);
void DrawGraphicDetail(const Menu* menu, int shapeNo);
void DisplayPadMenu1(const Menu* menu, int shapeNo);
void DisplayPadMenu2(const Menu* menu, int shapeNo);
void DrawPilot(const Menu* menu, int shapeNo);
void DrawTeams(const Menu* menu, int shapeNo);
void DrawClass(const Menu* menu, int shapeNo);
void DrawMainMenu(const Menu* menu, int shapeNo);
void DrawPlayPad(const Menu* menu, int shapeNo);
void DrawOptionMenus(const Menu* menu, int shapeNo);
void DisplayPadMenu(const Menu* menu, int shapeNo);
void DrawGames(const Menu* menu, int shapeNo);
void DrawHiScoreMenu(const Menu* menu, int shapeNo);

void KeyConfig(void);
void ResetKeys(void);

void CallMenuSelector(Object**, ConfigData*, SelectionData*);
void CallMenuActivator(ConfigData*, SelectionData*);
int LoadModel(Object** modelShapes, char* fileName);
int LoadTexModel(Object** modelShapes, char* fileName);
void MusicVSfx(SelectionData*, ConfigData*);
void ShowHiScores(SelectionData*, ConfigData*);
void InitSelectData(SelectionData*);

void DrawTeamShips(int, int, int, int, Object*);
void InitButtons(char*);
void ShowTimeTrialScores(SelectionData*, ConfigData*);
void QuitYesNo(SelectionData*, ConfigData*);
void GraphicOptions(SelectionData*, ConfigData*);
void ControllerOptions(SelectionData*, ConfigData*);
void ControllerEdit(SelectionData*, ConfigData*);

#define MAX_MENU_HISTORY 8

typedef struct
{
    uint8 menu;
    uint8 option;
} MenuHistory;

int gMenuHistoryIndex;
MenuHistory gMenuHistory[MAX_MENU_HISTORY];

const Menu gMenuSystem[] = {
// MAIN_MENU
    { "WIPEOUT MAIN MENU", ctrlshipShapes, gameShapes, DrawMainMenu, 0, 
    #ifdef MENU_HAS_QUIT
        3,
    #else
        2,
    #endif
        { 
            "START GAME",
            "OPTIONS",
            "QUIT"
        }
    },
// TRACK_MENU
    { "SELECT RACING CIRCUIT", 0, 0, 0, 0, 7,
        {
            "ALTIMA VII",
            "KARBONIS V",
            "TERRAMAX",
            "KORODERA",
            "ARRIDOS IV",
            "SILVERSTREAM",
            "FIRESTAR"
        }
    },
// AUDIO_MENU
    { "AUDIO", 0, 0, 0, MusicVSfx, 0 },
// OPTION_MENU
    { "OPTIONS", optShapes, padShapes, DrawOptionMenus, 0, 4,
        { 
            "CONTROLS",
            "GRAPHICS",
            "AUDIO",
            "BEST TIMES"
        }
    },
// CLASS_MENU
    { "SELECT RACING CLASS", clsShapes, 0, DrawClass, 0, 2,
        { 
            "VENOM CLASS",
            "RAPIER CLASS"
        }
    },
// TEAM_MENU
    { "SELECT YOUR TEAM", teamShapes, ctrlshipShapes, DrawTeams, 0, 4,
        { 
            "AG SYSTEMS",
            "AURICOM",
            "QIREX",
            "FEISAR"
        }
    },
// PILOT_MENU_1 - AG_SYSTEMS
    { "CHOOSE YOUR PILOT", pilotShapes, ctrlshipShapes, DrawPilot, 0, 2,
        { 
            "JOHN DEKKA",
            "DANIEL CHANG"
        }
    },
// PILOT_MENU_2 - AURICOM
    { "CHOOSE YOUR PILOT", pilotShapes, ctrlshipShapes, DrawPilot, 0, 2,
        { 
            "ARIAL TETSUO",
            "ANASTASIA CHEROVOSKI"
        }
    },
// PILOT_MENU_3 - QIREX
    { "CHOOSE YOUR PILOT", pilotShapes, ctrlshipShapes, DrawPilot, 0, 2,
        { 
            "KEL SOLAAR",
            "ARIAN TETSUO"
        }
    },
// PILOT_MENU_4 - FEISAR
    { "CHOOSE YOUR PILOT", pilotShapes, ctrlshipShapes, DrawPilot, 0, 2,
        { 
            "SOFIA DE LA RENTE",
            "PAUL JACKSON"
        }
    },
// RACE_TYPE_MENU
    { "SELECT RACE TYPE", gameShapes, optShapes, DrawGames, 0, 3,
        { 
            "CHAMPIONSHIP RACE",
            "SINGLE RACE",
            "TIME TRIAL"
        }
    },
// HIGH_SCORES_MENU
    { "BEST RACE TIMES", 0, 0, 0, ShowHiScores, 0 },
// BEST_TIMES_MENU
    { "BEST TIMES", optShapes, 0, DrawHiScoreMenu, 0, 2,
        { 
            "BEST RACE TIMES",
            "BEST TIME TRIAL TIMES"
        }
    },
// TIME_TRIAL_MENU
    { "BEST TIME TRIAL TIMES", 0, 0, 0, ShowTimeTrialScores, 0 },
// QUIT_MENU
    { "", 0, 0, 0, QuitYesNo, 0 },
    // DETAILS_MENU
    { "GRAPHICS", ctrlshipShapes, 0, DrawGraphicDetail, GraphicOptions, 0 },
// CONTROLLER_MENU
    { "CONTROLS", padShapes, padShapes, DrawControllerMenu, ControllerOptions, 0 },
// CONTROL_EDIT_MENU
    { "CONFIGURE KEYBOARD", padShapes, padShapes, DrawControllerEdit, ControllerEdit, 0 }
};

//#define VER_PSX

void menuInit(void)
{
    LoadTexModel(pilotShapes, "pilot");
    LoadTexModel(audioShapes, "cdobj");

#ifdef VER_PSX
    LoadModel(teamShapes, "teams");
    LoadModel(clsShapes, "class");
    LoadTexModel(padShapes, "pad1");
    LoadModel(gameShapes, "champ");
    LoadTexModel(optShapes, "alopt");
#else
    LoadTexModel(teamShapes, "team");
    LoadTexModel(clsShapes, "leeg");
    LoadTexModel(padShapes, "cntrl");
    LoadTexModel(gameShapes, "msdos");
    LoadTexModel(optShapes, "misc");
#endif

    InitTrackTex(trackTex);

    InitButtons("wipeout/textures/buttons.cmp");
    slideTex = LoadTexture("wipeout/textures/slider1.tim");
    sliderTex = LoadTexture("wipeout/textures/slide1.tim");
    menuScreen = LoadTexture("wipeout/textures/wipeout1.tim");

    gMenuHistoryIndex = 0;

    trackNum = 15; // pretend track for restoring - SJR

    InitMenuDisplay(90);

    if (ctrlfirsttime)
        InitSelectData(&selectData);

    selectData.select = 0;

    gCurrMenu = 0;
    gCurrOption = 0;

    InitSkeleton(&gCamPos);
    InitSkeleton(&gCamDir);
}

void menuLoop(ConfigData* gameData)
{
    UpdatePad();

    renderFrameBegin();
    renderClear();
    renderSetMode(RENDER_MODE_2D);

    renderBackground(menuScreen);

    if (gCurrMenu >= 0)
        CallMenuSelector(ctrlshipShapes, gameData, &selectData);

    if (selectData.select)
    {
        if (gCurrMenu == MAIN_MENU && gCurrOption == 0)
        {
            // CHEAT ACTIVATION BITS - need to be changed for PCwipeout
            /*TODO cheats*/

            if ((pad & IN_LEFT) && (pad & IN_A))
            {
                gameData->RapierClass = 1;
            }

            if ((pad & IN_RIGHT) && (pad & IN_Y))
            {
                totalTracks = 14;
                NumTracks = 7;
                gameData->showBonusTrack = 1;
            }
        }

        CallMenuActivator(gameData, &selectData);
    }

    if (gCurrMenu == CLASS_MENU && gCurrOption == 1 && (!gameData->RapierClass))
        CentreText("NOT AVAILABLE", text_data(20, 190, 12), TEXT_COLOR_HIGHLIGHT);

    if (gCurrMenu == -1)
    {
        ctrlmode = INTERACTIVE;
        ctrlGameLoop = GAME_LOOP_RACE_INIT;
        return;
    }

    if (selectData.select == 1)
    {
        selectData.select = 0;
    }

    renderFrameEnd();
}

void GoMenu(int menu)
{
    MenuHistory* h = gMenuHistory + (gMenuHistoryIndex++);
 
    ASSERT(gMenuHistoryIndex <= MAX_MENU_HISTORY);

    h->menu = gCurrMenu;
    h->option = gCurrOption;

    gCurrMenu = menu;
    gCurrOption = 0;

    PlayNote(SFX_MENU_SELECT);
    InitMenuDisplay(90);
}

void GoBack(void)
{
    MenuHistory* h = gMenuHistory + (--gMenuHistoryIndex);

    ASSERT(gMenuHistoryIndex >= 0);

    gCurrMenu = h->menu;
    gCurrOption = h->option;

    KillNote(SFX_SHIELDS);

    PlayNote(SFX_MENU_2_MENU);
    InitMenuDisplay(90);
}

void GoPrevMusic(void)
{
    PlayNote(SFX_MENU_MOVE);
    gMusicTrack--;
    if (gMusicTrack < 0)
        gMusicTrack = gMusicTracksCount;
    MusicStop();
}

void GoNextMusic(void)
{
    PlayNote(SFX_MENU_MOVE);
    gMusicTrack++;
    if (gMusicTrack > gMusicTracksCount)
        gMusicTrack = 0;
    MusicStop();
}

void DisplayButtons(int x, int y, int buttType)
{
    renderImage(buttonTex[buttType], x, y);
}

void DrawButtons(void)
{
    if (gMenuSystem[gCurrMenu].numOptions > 0)
        DisplayButtons(30, 210, PSELECT);

    if (gCurrMenu != MAIN_MENU)
        DisplayButtons(240, 210, PBACK);
}

void CallMenuActivator(ConfigData* gameData, SelectionData* selectData)
{
    switch (gCurrMenu)
    {
        case MAIN_MENU:
        {
            switch (gCurrOption)
            {
                case SINGLE_GAME:
                    GoMenu(CLASS_MENU);
                    break;
                case GAME_OPTIONS:
                    GoMenu(OPTION_MENU);
                    break;
                case QUIT_GAME:
                    selectData->confirm = 0;
                    GoMenu(QUIT_MENU);
                    break;
            }
            break;
        }

        case TRACK_MENU:
        {
            gameData->trackNum = (gCurrOption + 1) + gameData->Class * 7;
            gameData->actTrackNum = gTrackList[(gameData->trackNum - 1)];
            GoMenu(-1);
            break;
        }

        case OPTION_MENU:
        {
            switch (gCurrOption)
            {
                case CONTROLS:
                    GoMenu(CONTROLLER_MENU);
                    break;
                case DETAILS:
                    GoMenu(DETAILS_MENU);
                    break;
                case AUDIO_CONFIG:
                    GoMenu(AUDIO_MENU);
                    InitMenuDisplay(180);
                    gameData->oldSliderPos = gameData->currSliderPos;
                    break;
                case HIGH_SCORES:
                    GoMenu(BEST_TIMES_MENU);
                    selectData->hiscoreTrack = 0;
                    break;
            }
            break;
        }

        case CLASS_MENU:
        {
            gameData->Class = gCurrOption;
            if (gameData->RapierClass == 0 && gCurrOption == RAPIER_CLASS)
                return;

            if (gCurrOption == CHAMPIONSHIP)
            {
                gameData->trackNum = 1;
            }

            if (gCurrOption == SINGLE_RACE)
            {
                if (gameData->showBonusTrack == 0)
                    gameData->trackNum = NumTracks + 2;
                else
                    gameData->trackNum = NumTracks + 1;
            }

            GoMenu(RACE_TYPE_MENU);
            break;
        }

        case TEAM_MENU:
        {
            gameData->team = gCurrOption;
            GoMenu(PILOT_MENU_1 + gCurrOption);
            break;
        }

        case PILOT_MENU_1:
        case PILOT_MENU_2:
        case PILOT_MENU_3:
        case PILOT_MENU_4:
        {
            gameData->newLapRecord = 0;
            gameData->shipNum = (gameData->team * 2) + gCurrOption;
            ownShip = gameData->shipNum;

            if (gameData->gameType == CHAMPIONSHIP)
            {
                GoMenu(-1);
            }
            else
            {
                GoMenu(TRACK_MENU);
            }
            break;
        }

        case RACE_TYPE_MENU:
        {
            raceType = 0; // TODO gameData->gameType ?

            switch (gCurrOption)
            {
                case CHAMPIONSHIP:
                    gameData->gameType = CHAMPIONSHIP;
                    gameData->actTrackNum = gTrackList[gameData->trackNum - 1];
                    InitChampTable();
                    break;
                case SINGLE_RACE:
                    gameData->gameType = SINGLE_RACE;
                    break;
                case TIME_TRIAL:
                    gameData->gameType = TIME_TRIAL;
                    raceType = TIME_TRIAL;
                    break;
            }

            GoMenu(TEAM_MENU);
            break;
        }

        case BEST_TIMES_MENU:
        {
            switch (gCurrOption)
            {
                case 0:
                    selectData->hiscoreTrack = 0;
                    GoMenu(HIGH_SCORES_MENU);
                    break;
                case 1:
                    selectData->hiscoreTrack = 14;
                    GoMenu(TIME_TRIAL_MENU);
                    break;
            }
            break;
        }

        case QUIT_MENU:
        {
            if (selectData->confirm)
            {
                osQuit();
            }
            else
            {
                GoBack();
            }
            break;
        }

        case DETAILS_MENU:
        {
            GoMenu(OPTION_MENU);
            break;
        }

        case CONTROL_EDIT_MENU:
        {
            GoMenu(CONTROLLER_MENU);
            break;
        }

        case CONTROLLER_MENU:
        {
            GoMenu(OPTION_MENU);
            break;
        }
    }
}

void CallMenuSelector(Object** shipShapes, ConfigData* gameData, SelectionData* selectData)
{
    int i;
    const Menu* menu = gMenuSystem + gCurrMenu;
    int numOptions = menu->numOptions;

    if (gCurrMenu == TRACK_MENU)
    {
        renderImage(trackTex[gCurrOption], 96, 50);

        if (gameData->Class == 0)
            CentreText("VENOM CLASS", text_data(20, 40, 8), TEXT_COLOR_HIGHLIGHT);
        else
            CentreText("RAPIER CLASS", text_data(20, 40, 8), TEXT_COLOR_HIGHLIGHT);

        numOptions = NumTracks;
    }

    if (GetMenuKey(M_UP))
    {
        PlayNote(SFX_MENU_MOVE);
        gCurrOption--;
        if (gCurrOption < 0)
        {
            gCurrOption = (numOptions - 1);
        }
    }

    if (GetMenuKey(M_DOWN))
    {
        PlayNote(SFX_MENU_MOVE);
        gCurrOption++;
        if (gCurrOption > (numOptions - 1))
            gCurrOption = 0;
    }

    if (menu->displayObject != 0)
    {
        renderSetMode(RENDER_MODE_3D);

        if (gCurrMenu >= PILOT_MENU_1 && gCurrMenu <= PILOT_MENU_4)
            (menu->displayObject)(gMenuSystem + gCurrMenu, (gameData->team * 2) + gCurrOption);
        else
            (menu->displayObject)(gMenuSystem + gCurrMenu, gCurrOption);

        renderSetMode(RENDER_MODE_2D);
    }

    DrawButtons();
    CentreText(menu->title, text_data(0, 21, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < numOptions; i++)
    {
        CentreText(menu->options[i], text_data(0, 127 + 12 * i, 8), (i == gCurrOption) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    }
    
    if (menu->displayMenu != 0)
    {
        (menu->displayMenu)(selectData, gameData);
    }

    if (GetMenuKey(M_SELECT))
    {
        selectData->select = 1;
    }

    if (GetMenuKey(M_BACK) && (gMenuHistoryIndex > 0))
    {
        if (gCurrMenu == CONTROL_EDIT_MENU)
        {
            ResetKeys();
        }

        GoBack();

        KeyToChange = NONE;
    }
}

int LoadModel(Object** modelShapes, char* fileName)
{
    Object* obj;
    int count = 0;
    char modelFile[64];

    strcpy(modelFile, "wipeout/common/");
    strcat(modelFile, fileName);
    strcat(modelFile, ".prm");

    obj = LoadPrm(modelFile, TextureTableCount);

    while (obj)
    {
        modelShapes[count] = obj;
        count++;
        obj = obj->next;
    }

    return count;
}

int LoadTexModel(Object** modelShapes, char* fileName)
{
    int modelTextures = 0;
    Object* obj;
    int count = 0;
    char modelFile[64];

    /******************************************************/
    /** Load compressed texture files, first to allow    **/
    /** temporary memory to be allocated                 **/
    /******************************************************/

    strcpy(modelFile, "wipeout/common/");
    strcat(modelFile, fileName);
    strcat(modelFile, ".cmp");
    modelTextures = TextureTableCount;
    LoadCompressedTextureSequence(modelFile, 0);

    strcpy(modelFile, "wipeout/common/");
    strcat(modelFile, fileName);
    strcat(modelFile, ".prm");

    obj = LoadPrm(modelFile, modelTextures);

    while (obj)
    {
        modelShapes[count] = obj;
        count++;
        obj = obj->next;
    }

    return count;
}

void DrawMainMenu(const Menu* menu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    int heading = gFrameIndex * 5;

    if (shapeNo == 0)
        DrawTeamShips(0, 0, MODELSIZE, heading, menu->menuModel[shipShapeOrder[0]]);

    // if (shapeNo == 1)
    // {
    //    DrawTeamShips (-450, 0, 1100, heading, menu->menuModel[shipShapeOrder[3]]) ;
    //    DrawTeamShips (450, 0, 1100, heading, menu->menuModel[shipShapeOrder[1]]) ;
    // }

    if (shapeNo == 1)
    {
        DrawObject(menu->menu2Model[3], 0, 0, MODELSIZE, hdg, 0, 0);
    }
}

void DrawTeams(const Menu* menu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    int heading = gFrameIndex * 10;

    DrawObject(menu->menuModel[teamShapeOrder[shapeNo]], 0, 0, 9000, hdg, 0, 0);

    DrawTeamShips(-900, 800, 1500, heading, menu->menu2Model[shipShapeOrder[shapeNo * 2]]);
    DrawTeamShips(900, 800, 1500, heading, menu->menu2Model[shipShapeOrder[shapeNo * 2 + 1]]);
}

void DrawTeamShips(int x, int y, int size, int heading, Object* shipModel)
{
    ShipData shipData;
    shipData.pos.x = x;
    shipData.pos.y = y;
    shipData.pos.z = size;
    shipData.hdg = -heading;
    shipData.pitch = 0;
    shipData.roll = 0;
    shipData.color = 0xFFFFFFFF;
    DrawShip(shipModel, &shipData);
}

void DrawPilot(const Menu* menu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    int shdg = gFrameIndex * 5;

    DrawObject(menu->menuModel[pilotShapeOrder[shapeNo]], 0, -400, 11000, hdg, 0, 0);
    DrawTeamShips(0, 700, 1300, shdg, menu->menu2Model[shipShapeOrder[shapeNo]]);
}

void DrawPlayPad(const Menu* menu, int shapeNo)
{
    InitMenuDisplay(120);
    DrawObject(menu->menuModel[0], 0, -1140, 6220 + zoomInOut, modelHdg, 0, 0);
    DrawObject(menu->menuModel[1], 0, 2260 + downUp, 6670, 250, -3440, 10);
}

void DrawClass(const Menu* menu, int shipNo)
{
    int hdg = gFrameIndex * 20;

    if (shipNo == 0)
    {
        DrawObject(menu->menuModel[0], 0, 0, 500, hdg, 0, 0);
    }

    if (shipNo == 1)
    {
        DrawObject(menu->menuModel[1], 0, -25, 500, hdg, 0, 0);
    }
}

void InitSelectData(SelectionData* selectData)
{
    selectData->optionNo = 0;
    selectData->select = 0;
    selectData->hiscoreTrack = 0;
}

void MusicVSfx(SelectionData* selectData, ConfigData* gameData)
{
    if (xpad & IN_UP)
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->optionNo--;
        if (selectData->optionNo < 0)
            selectData->optionNo = 1;
    }

    if (xpad & IN_DOWN)
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->optionNo++;
        if (selectData->optionNo > 1)
            selectData->optionNo = 0;
    }

    renderImage(sliderTex, 39 + (gMasterBalance * 218 >> 8), 61);
    renderImage(slideTex, 48, 85);

    AddText("SFX", text_data(33, 51, 8), (selectData->optionNo == 0) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("MUSIC", text_data(250, 139, 8), (selectData->optionNo == 0) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("TRACK", text_data(33, 175, 8), (selectData->optionNo == 1) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText(GetMusicTrackName(gMusicTrack), text_data(130, 175, 8), TEXT_COLOR_HIGHLIGHT);

    if (selectData->optionNo == 0)
    {
        if (pad & IN_LEFT)
        {
            if (WhatChannel(SFX_SHIELDS) == -1)
            {
                PlayNote(SFX_SHIELDS);
                SetPitch(SFX_SHIELDS, SND_DEF_PITCH >> 1);
                SetVolume(SFX_SHIELDS, SND_MAX_VOLUME, SND_MAX_VOLUME);
            }
            SetMasterBalance(gMasterBalance - (gFrameIndex - gLastFrameIndex));
        }
        else if (pad & IN_RIGHT)
        {
            if (WhatChannel(SFX_SHIELDS) == -1)
            {
                PlayNote(SFX_SHIELDS);
                SetPitch(SFX_SHIELDS, SND_DEF_PITCH >> 1);
                SetVolume(SFX_SHIELDS, SND_MAX_VOLUME, SND_MAX_VOLUME);
            }
            SetMasterBalance(gMasterBalance + (gFrameIndex - gLastFrameIndex));
        }
        else
        {
            KillNote(SFX_SHIELDS);
        }
    }

    if (selectData->optionNo == 1)
    {
        KillNote(SFX_SHIELDS);

        if (xpad & IN_LEFT)
        {
            GoPrevMusic();
        }
        else if (xpad & IN_RIGHT)
        {
            GoNextMusic();
        }
    }
}

void ShowHiScores(SelectionData* selectData, ConfigData* gameData)
{
    short actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->hiscoreTrack++;
        if (gameData->RapierClass == 0)
        {
            if (selectData->hiscoreTrack == NumTracks)
                selectData->hiscoreTrack = (NumTracks - 1);
        }
        else
        {
            if (gameData->showBonusTrack == 0)
            {
                if (selectData->hiscoreTrack == NumTracks)
                    selectData->hiscoreTrack = (NumTracks + 1);
            }

            if (selectData->hiscoreTrack == totalTracks)
                selectData->hiscoreTrack = (totalTracks - 1);
        }
    }

    if (GetMenuKey(M_LEFT))
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->hiscoreTrack--;
        if (gameData->RapierClass == 0)
        {
            if (selectData->hiscoreTrack < 0)
                selectData->hiscoreTrack = 0;
        }
        else
        {
            if (gameData->showBonusTrack == 0)
            {
                if (selectData->hiscoreTrack == NumTracks)
                    selectData->hiscoreTrack = (NumTracks - 1);
            }

            if (selectData->hiscoreTrack < 0)
                selectData->hiscoreTrack = 0;
        }
    }

    actualTrack = selectData->hiscoreTrack;

    if (selectData->hiscoreTrack >= NumTracks)
    {
        if (gameData->showBonusTrack == 0)
            actualTrack -= (NumTracks + 1);
        else
            actualTrack -= NumTracks;
        CentreText("RAPIER", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }
    else
    {
        CentreText("VENOM", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }

    if (((gameData->RapierClass == 0) && (selectData->hiscoreTrack == (NumTracks - 1))) || ((gameData->RapierClass == 1) && (selectData->hiscoreTrack == (totalTracks - 1))))
    {
    //    DisplayButtons(290, 116, ARROWR);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(290, 116, ARROWR);
    }

    if (selectData->hiscoreTrack == 0)
    {
    //    DisplayButtons(20, 116, ARROWL);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(20, 116, ARROWL);
    }

    CentreText(gMenuSystem[TRACK_MENU].options[actualTrack], text_data(0, 127 - 70, 12), TEXT_COLOR_HIGHLIGHT);

    if (BLINK_SLOW)
    {
        AddText("LAP RECORD", text_data(40, 195, 8), TEXT_COLOR_WHITE);
        AddTime(scoreTable[selectData->hiscoreTrack].lapRecord, text_data(165, 188, 16), TEXT_COLOR_HIGHLIGHT);
    }

    DisplayHiScore(selectData->hiscoreTrack);
}

void ShowTimeTrialScores(SelectionData* selectData, ConfigData* gameData)
{
    short actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->hiscoreTrack++;
        if (gameData->RapierClass == 0)
        {
            if (selectData->hiscoreTrack == (NumTracks + HiScoreTracks))
                selectData->hiscoreTrack = ((NumTracks - 1) + HiScoreTracks);
        }
        else
        {
            if (gameData->showBonusTrack == 0)
            {
                if (selectData->hiscoreTrack == (NumTracks + HiScoreTracks))
                    selectData->hiscoreTrack = ((NumTracks + 1) + HiScoreTracks);
            }

            if (selectData->hiscoreTrack == (totalTracks + HiScoreTracks))
                selectData->hiscoreTrack = ((totalTracks - 1) + HiScoreTracks);
        }
    }

    if (GetMenuKey(M_LEFT))
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->hiscoreTrack--;
        if (gameData->RapierClass == 0)
        {
            if (selectData->hiscoreTrack < 14)
                selectData->hiscoreTrack = 14;
        }
        else
        {
            if (gameData->showBonusTrack == 0)
            {
                if (selectData->hiscoreTrack == (NumTracks + HiScoreTracks))
                    selectData->hiscoreTrack = ((NumTracks - 1) + HiScoreTracks);
            }

            if (selectData->hiscoreTrack < 14)
                selectData->hiscoreTrack = 14;
        }
    }

    actualTrack = (selectData->hiscoreTrack - HiScoreTracks);

    if ((selectData->hiscoreTrack - HiScoreTracks) >= (NumTracks))
    {
        if (gameData->showBonusTrack == 0)
            actualTrack -= ((NumTracks + 1));
        else
            actualTrack -= (NumTracks);
        CentreText("RAPIER", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }
    else
    {
        CentreText("VENOM", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }

    if (((gameData->RapierClass == 0) && ((selectData->hiscoreTrack - HiScoreTracks) == (NumTracks - 1))) || ((gameData->RapierClass == 1) && ((selectData->hiscoreTrack - HiScoreTracks) == (totalTracks - 1))))
    {
    //    DisplayButtons(290, 116, ARROWR);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(290, 116, ARROWR);
    }

    if ((selectData->hiscoreTrack - HiScoreTracks) == 0)
    {
    //    DisplayButtons(20, 116, ARROWL);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(20, 116, ARROWL);
    }

    CentreText(gMenuSystem[TRACK_MENU].options[actualTrack], text_data(0, 127 - 70, 12), TEXT_COLOR_HIGHLIGHT);

    if (BLINK_SLOW)
    {
        AddText("LAP RECORD", text_data(40, 195, 8), TEXT_COLOR_WHITE);
        AddTime(scoreTable[selectData->hiscoreTrack].lapRecord, text_data(165, 188, 16), TEXT_COLOR_HIGHLIGHT);
    }

    DisplayHiScore(selectData->hiscoreTrack);
}

void InitGameData(ConfigData* gameData)
{
    gameData->shipNum = randy() % 8;
    gameData->trackNum = gameData->actTrackNum = gameData->ship2Num = 0;
    gameData->oppMenu = gameData->Class = 0;
    gameData->team = gameData->team2 = gameData->gameType = 0;
    gameData->Lives = 3;
    gameData->EndOfGame = 0;
    gameData->NumTracks = 1;
    gameData->Position = NO_OF_SHIPS;
    gameData->RapierClass = 0;
    gameData->gameComplete = 0;
    gameData->showPilotCount = 0;
    gameData->gameOverCount = 0;
    gameData->venomComplete = 0;
    gameData->newLapRecord = 0;
    gameData->gotoMenu = 0;
    gameData->normpadPreset = 0;
    gameData->cardPresent = 0;
    gameData->currSliderPos = 150;
    gameData->showBonusTrack = 0;
    gameData->trackInc = 1;
    gameData->lostGame = 0;
    gameData->showGameOver = 0;
}

/*TODO lconfirm? xpad?*/
void ContinueGameConfirm(char* confirm, short xpad, char* lConfirm)
{
    CentreText("CONTINUE QUALIFYING OR QUIT", text_data(77, 95, 8), TEXT_COLOR_WHITE);

    AddText("QUIT", text_data(215, 120, 16), (*lConfirm) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("QUALIFY", text_data(45, 120, 16), (*lConfirm) ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void RestartGameConfirm(char* confirm, short xpad, char* lConfirm)
{
    CentreText("RESTART RACE", text_data(77, 95, 16), TEXT_COLOR_WHITE);

    AddText("NO", text_data(215, 120, 16), (*lConfirm) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("YES", text_data(68, 120, 16), (*lConfirm) ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void SetNormPadPresets(ConfigData* gameData)
{
    switch (gameData->normpadPreset)
    {
        case 0:
            pCon.thrust = IN_A;
            pCon.view = IN_Y;
            pCon.fire = IN_B;
            pCon.lBrake = IN_RT;
            pCon.rBrake = IN_LT;
            break;

        case 1:
            pCon.thrust = IN_A;
            pCon.view = IN_Y;
            pCon.fire = IN_X;
            pCon.lBrake = IN_RT;
            pCon.rBrake = IN_LT;
            break;

        case 2:
            pCon.thrust = IN_A;
            pCon.view = IN_LB;
            pCon.fire = IN_RB;
            pCon.lBrake = IN_RT;
            pCon.rBrake = IN_LT;
            break;

        case 3:
            pCon.thrust = IN_X;
            pCon.view = IN_B;
            pCon.fire = IN_A;
            pCon.lBrake = IN_RT;
            pCon.rBrake = IN_LT;
            break;

        case 4:
            pCon.thrust = IN_A;
            pCon.view = IN_Y;
            pCon.fire = IN_B;
            pCon.lBrake = IN_RB;
            pCon.rBrake = IN_LB;
            break;

        case 5:
            pCon.thrust = IN_A;
            pCon.view = IN_Y;
            pCon.fire = IN_X;
            pCon.lBrake = IN_RB;
            pCon.rBrake = IN_LB;
            break;

        case 6:
            pCon.thrust = IN_Y;
            pCon.view = IN_X;
            pCon.fire = IN_B;
            pCon.lBrake = IN_RT;
            pCon.rBrake = IN_LT;
            break;

        case 7:
            pCon.thrust = IN_B;
            pCon.view = IN_X;
            pCon.fire = IN_Y;
            pCon.lBrake = IN_RT;
            pCon.rBrake = IN_LT;
            break;
    }
}

void DrawGames(const Menu* menu, int shipNo)
{
    int hdg = gFrameIndex * 20;

    InitMenuDisplay(90);

    if (shipNo == 0)
    {
        DrawObject(menu->menuModel[0], 0, -50, 600, hdg, 0, 0);
    }

    if (shipNo == 1)
    {
        DrawObject(menu->menuModel[2], 0, -50, 600, hdg, 0, 0);
    }

    if (shipNo == 2)
    {
        DrawObject(menu->menu2Model[2], 0, -50, 600, hdg, 0, 0);
    }
}

void DrawOptionMenus(const Menu* menu, int shipNo)
{
    int hdg = gFrameIndex * 20;
    int heading = gFrameIndex * 5;

    InitMenuDisplay(90);

    switch (shipNo)
    {
        case CONTROLS:
            DisplayPadMenu1(menu, shipNo);
            break;
        case DETAILS:
            DrawTeamShips(0, 0, MODELSIZE, heading, gMenuSystem[DETAILS_MENU].menuModel[shipShapeOrder[0]]);
            break;
        case AUDIO_CONFIG:
            DrawObject(menu->menuModel[3], 0, 0, 600, hdg, 0, 0);
            break;
        case HIGH_SCORES:
            DrawObject(menu->menuModel[2], 0, 0, 600, hdg, 0, 0);
            break;
    }
}

void DrawHiScoreMenu(const Menu* menu, int shipNo)
{
    int hdg = gFrameIndex * 20;

    InitMenuDisplay(90);
    DrawObject(menu->menuModel[2], 0, 0, 600, hdg, 0, 0);
}

void DisplayPadMenu(const Menu* menu, int shapeNo)
{
    int hdg = gFrameIndex * 20;

    DrawObject(menu->menu2Model[0], 0, 0, 6500, hdg, 0, 0);
}

void DisplayPadMenu1(const Menu* menu, int shapeNo)
{
    int hdg = gFrameIndex * 20;

    DrawObject(menu->menu2Model[1], 0, -5, 4500, hdg, 0, 0);
}

void DisplayPadMenu2(const Menu* menu, int shapeNo)
{
    int hdg = gFrameIndex * 20;

    DrawObject(menu->menu2Model[2], 0, 0, 4500, hdg, 0, 0);
}

void InitChampionship(void)
{
    short i;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        winTable[i].points = 0;
        winTable[i].racePoints = 0;
        winTable[i].pilot[0] = '\0';
        winTable[i].xPos = 0;
        winTable[i].position = 0;
        winTable[i].racePosition = 0;
    }
}

void InitButtons(char* fileName)
{
    int tablePos;
    int i;

    tablePos = TextureTableCount;

    LoadCompressedTextureSequence(fileName, 0);

    for (i = 0; i < 18; i++)
    {
        buttonTex[i] = TextureTable[tablePos];
        tablePos++;
    }
}

void QuitYesNo(SelectionData* selectData, ConfigData* gameData)
{
    CentreText("ARE YOU SURE YOU", text_data(77, 95, 8), TEXT_COLOR_HIGHLIGHT);
    CentreText("WANT TO QUIT", text_data(105, 105, 8), TEXT_COLOR_HIGHLIGHT);

    if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
    {
        selectData->confirm ^= 1;
    }

    AddText("YES", text_data(80, 120, 16), selectData->confirm ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("NO", text_data(210, 120, 16), selectData->confirm ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void GraphicOptions(SelectionData* selectData, ConfigData* gameData)
{
    // TODO
}

void ControllerOptions(SelectionData* selectData, ConfigData* gameData)
{
    // TODO
}

void ControllerEdit(SelectionData* selectData, ConfigData* gameData)
{
    KeyConfig();
}

void DrawControllerMenu(const Menu* menu, int shapeNo)
{
    InitMenuDisplay(110);
    // TODO
}

void KeyConfig(void)
{
    // TODO
}

void DrawControllerEdit(const Menu* menu, int shapeNo)
{
    InitMenuDisplay(80);
}

void DrawGraphicDetail(const Menu* menu, int shapeNo)
{
    int heading = gFrameIndex * 5;

    DrawTeamShips(0, 0, MODELSIZE, heading, menu->menuModel[shipShapeOrder[0]]);
}

char CheckKey(unsigned short code)
{
    /*TODO input remap
    if(NewOriginalKeyCodes[15] == code)
       return(0);
    else if(NewOriginalKeyCodes[13] == code)
       return(0);
    else if(NewOriginalKeyCodes[12] == code)
       return(0);
    else if(NewOriginalKeyCodes[14] == code)
       return(0);
    else if(NewOriginalKeyCodes[0] == code)
       return(0);
    else if(NewOriginalKeyCodes[1] == code)
       return(0);
    else if(NewOriginalKeyCodes[6] == code)
       return(0);
    else if(NewOriginalKeyCodes[5] == code)
       return(0);
    else if(NewOriginalKeyCodes[4] == code)
       return(0);
    else if(NewOriginalKeyCodes[11] == code)
       return(0);

    if(code == 0x1c)
       return(0);
       */
    return (1);
}

void ResetKeys(void)
{
    /*TODO input default keys
       OriginalKeyCodes[15] = Keycons[0];
       OriginalKeyCodes[13] = Keycons[1];
       OriginalKeyCodes[12] = Keycons[2];
       OriginalKeyCodes[14] = Keycons[3];
       OriginalKeyCodes[0] = Keycons[4];
       OriginalKeyCodes[1] = Keycons[5];
       OriginalKeyCodes[6] = Keycons[6];
       OriginalKeyCodes[5] = Keycons[7];
       OriginalKeyCodes[4] = Keycons[8];
       OriginalKeyCodes[11] = Keycons[9];

       SetKey(15, OriginalKeyCodes[15]);
       SetKey(13, OriginalKeyCodes[13]);
       SetKey(12, OriginalKeyCodes[12]);
       SetKey(14, OriginalKeyCodes[14]);
       SetKey(0,  OriginalKeyCodes[0]);
       SetKey(1,  OriginalKeyCodes[1]);
       SetKey(6,  OriginalKeyCodes[6]);
       SetKey(5,  OriginalKeyCodes[5]);
       SetKey(4,  OriginalKeyCodes[4]);
       SetKey(11, OriginalKeyCodes[11]);
    */
}
