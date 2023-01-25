#include "game.h"

extern int32 ctrlmode;
extern int32 ctrlGameLoop;

Texture* menuScreen;

extern HiScores scoreTable[];

extern const short shipShapeOrder[];
const short pilotShapeOrder[8] = { 0, 4, 6, 7, 2, 5, 1, 3 };
const short teamShapeOrder[4] = { 2, 3, 1, 0 }; // jed
ChampionShip winTable[NO_OF_SHIPS];

int modelHdg = 0;
int zoomInOut = 0;
int downUp = 0;

Texture* gButtonTex;
Texture* slideTex;
Texture* sliderTex;

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

extern int32 ctrlfirsttime;

Object* audioShapes;
Object* pilotShapes;
Object* teamShapes;

Object* clsShapes;
Object* gameShapes;
Object* optShapes;
Object* padShapes;
SelectionData selectData;

Texture* gTrackIcon;

void DrawControllerMenu(const Menu* menu, int shapeNo);
void DrawControllerEdit(const Menu* menu, int shapeNo);
void DrawGraphicDetail(const Menu* menu, int shapeNo);
void DisplayPadMenu(const Menu* menu, int shapeNo);
void DrawPilot(const Menu* menu, int shapeNo);
void DrawTeams(const Menu* menu, int shapeNo);
void DrawClass(const Menu* menu, int shapeNo);
void DrawMainMenu(const Menu* menu, int shapeNo);
void DrawOptionMenus(const Menu* menu, int shapeNo);
void DrawGames(const Menu* menu, int shapeNo);
void DrawHiScoreMenu(const Menu* menu, int shapeNo);

void CallMenuSelector(Object**, SelectionData*);
void CallMenuActivator(SelectionData*);
Object* LoadModel(const char* fileName);
Object* LoadTexModel(const char* fileName);
void MusicVSfx(SelectionData*);
void ShowHiScores(SelectionData*);
void InitSelectData(SelectionData*);

void DrawTeamShips(int, int, int, int, Object*);
void ShowTimeTrialScores(SelectionData*);
void QuitYesNo(SelectionData*);
void GraphicOptions(SelectionData*);
void ControllerOptions(SelectionData*);
void ControllerEdit(SelectionData*);

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
    { "WIPEOUT MAIN MENU", DrawMainMenu, 0, 
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
    { "SELECT RACING CIRCUIT", 0, 0, 7,
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
    { "AUDIO", 0, MusicVSfx, 0 },
// OPTION_MENU
    { "OPTIONS", DrawOptionMenus, 0, 4,
        { 
            "CONTROLS",
            "GRAPHICS",
            "AUDIO",
            "BEST TIMES"
        }
    },
// CLASS_MENU
    { "SELECT RACING CLASS", DrawClass, 0, 2,
        { 
            "VENOM CLASS",
            "RAPIER CLASS"
        }
    },
// TEAM_MENU
    { "SELECT YOUR TEAM", DrawTeams, 0, 4,
        { 
            "AG SYSTEMS",
            "AURICOM",
            "QIREX",
            "FEISAR"
        }
    },
// PILOT_MENU_1 - AG_SYSTEMS
    { "CHOOSE YOUR PILOT", DrawPilot, 0, 2,
        { 
            "JOHN DEKKA",
            "DANIEL CHANG"
        }
    },
// PILOT_MENU_2 - AURICOM
    { "CHOOSE YOUR PILOT", DrawPilot, 0, 2,
        { 
            "ARIAL TETSUO",
            "ANASTASIA CHEROVOSKI"
        }
    },
// PILOT_MENU_3 - QIREX
    { "CHOOSE YOUR PILOT", DrawPilot, 0, 2,
        { 
            "KEL SOLAAR",
            "ARIAN TETSUO"
        }
    },
// PILOT_MENU_4 - FEISAR
    { "CHOOSE YOUR PILOT", DrawPilot, 0, 2,
        { 
            "SOFIA DE LA RENTE",
            "PAUL JACKSON"
        }
    },
// RACE_TYPE_MENU
    { "SELECT RACE TYPE", DrawGames, 0, 3,
        { 
            "CHAMPIONSHIP RACE",
            "SINGLE RACE",
            "TIME TRIAL"
        }
    },
// HIGH_SCORES_MENU
    { "BEST RACE TIMES", 0, ShowHiScores, 0 },
// BEST_TIMES_MENU
    { "BEST TIMES", DrawHiScoreMenu, 0, 2,
        { 
            "BEST RACE TIMES",
            "BEST TIME TRIAL TIMES"
        }
    },
// TIME_TRIAL_MENU
    { "BEST TIME TRIAL TIMES", 0, ShowTimeTrialScores, 0 },
// QUIT_MENU
    { "", 0, QuitYesNo, 0 },
// DETAILS_MENU
    { "GRAPHICS", DrawGraphicDetail, GraphicOptions, 0 },
// CONTROLLER_MENU
    { "CONTROLS", DrawControllerMenu, ControllerOptions, 0 },
// CONTROL_EDIT_MENU
    { "CONFIGURE KEYBOARD", DrawControllerEdit, ControllerEdit, 0 }
};


//#define VER_PSX

void menuInit(void)
{
    LoadMainMenu();

    gMenuHistoryIndex = 0;

    renderOffset(0, -30);

    if (ctrlfirsttime)
        InitSelectData(&selectData);

    selectData.select = 0;

    gCurrMenu = 0;
    gCurrOption = 0;

    gCameraFOV = gTargetFOV = CAMERA_FOV;
}

void menuLoop(void)
{
    UpdatePad();

    renderFrameBegin();
    renderClear();
    renderSetMode(RENDER_MODE_2D);

    renderBackground(menuScreen);

    if (gCurrMenu >= 0)
        CallMenuSelector(ctrlshipShapes, &selectData);

    if (selectData.select)
    {
        if (gCurrMenu == MAIN_MENU && gCurrOption == 0)
        {
            // CHEAT ACTIVATION BITS - need to be changed for PCwipeout
            /*TODO cheats*/

            if ((pad & IN_LEFT) && (pad & IN_A))
            {
                gConfig.rapierClass = 1;
            }

            if ((pad & IN_RIGHT) && (pad & IN_Y))
            {
                totalTracks = 14;
                NumTracks = 7;
                gConfig.showBonusTrack = 1;
            }
        }

        CallMenuActivator(&selectData);
    }

    if (gCurrMenu == CLASS_MENU && gCurrOption == 1 && (!gConfig.rapierClass))
        CentreText("NOT AVAILABLE", text_data(20, 190, 12), TEXT_COLOR_HIGHLIGHT);

    renderFrameEnd();

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
    renderOffset(0, -30);
}

void GoBack(void)
{
    MenuHistory* h = gMenuHistory + (--gMenuHistoryIndex);

    ASSERT(gMenuHistoryIndex >= 0);

    gCurrMenu = h->menu;
    gCurrOption = h->option;

    KillNote(SFX_SHIELD);

    PlayNote(SFX_MENU_2_MENU);
    renderOffset(0, -30);
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
    renderImage(gButtonTex + buttType, x, y);
}

void DrawButtons(void)
{
    if (gMenuSystem[gCurrMenu].numOptions > 0)
        DisplayButtons(30, 210, PSELECT);

    if (gCurrMenu != MAIN_MENU)
        DisplayButtons(240, 210, PBACK);
}

void CallMenuActivator(SelectionData* selectData)
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
            gConfig.trackNum = (gCurrOption + 1) + gConfig.raceClass * 7;
            gConfig.actTrackNum = gTrackList[(gConfig.trackNum - 1)];
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
                    renderOffset(0, 60);
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
            gConfig.raceClass = gCurrOption;
            if (gConfig.rapierClass == 0 && gCurrOption == RAPIER_CLASS)
                return;

            if (gCurrOption == CHAMPIONSHIP)
            {
                gConfig.trackNum = 1;
            }

            if (gCurrOption == SINGLE_RACE)
            {
                if (gConfig.showBonusTrack == 0)
                    gConfig.trackNum = NumTracks + 2;
                else
                    gConfig.trackNum = NumTracks + 1;
            }

            GoMenu(RACE_TYPE_MENU);
            break;
        }

        case TEAM_MENU:
        {
            gConfig.team = gCurrOption;
            GoMenu(PILOT_MENU_1 + gCurrOption);
            break;
        }

        case PILOT_MENU_1:
        case PILOT_MENU_2:
        case PILOT_MENU_3:
        case PILOT_MENU_4:
        {
            gConfig.newLapRecord = 0;
            gConfig.shipNum = (gConfig.team * 2) + gCurrOption;
            ownShip = gConfig.shipNum;

            if (gConfig.raceType == CHAMPIONSHIP)
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
            switch (gCurrOption)
            {
                case CHAMPIONSHIP:
                    gConfig.raceType = CHAMPIONSHIP;
                    gConfig.actTrackNum = gTrackList[gConfig.trackNum - 1];
                    InitChampTable();
                    break;
                case SINGLE_RACE:
                    gConfig.raceType = SINGLE_RACE;
                    break;
                case TIME_TRIAL:
                    gConfig.raceType = TIME_TRIAL;
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

void CallMenuSelector(Object** shipShapes, SelectionData* selectData)
{
    int32 i;
    const char* title;
    const Menu* menu = gMenuSystem + gCurrMenu;
    int32 numOptions = menu->numOptions;

    if (gCurrMenu == TRACK_MENU)
    {
        renderImage(gTrackIcon + gCurrOption, 96, 50);

        if (gConfig.raceClass == 0)
            title = "VENOM CLASS";
        else
            title = "RAPIER CLASS";

        CentreText(title, text_data(20, 40, 8), TEXT_COLOR_HIGHLIGHT);
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
            (menu->displayObject)(gMenuSystem + gCurrMenu, (gConfig.team * 2) + gCurrOption);
        else
            (menu->displayObject)(gMenuSystem + gCurrMenu, gCurrOption);

        renderSetMode(RENDER_MODE_2D);
    }

    //DrawButtons();
    CentreText(menu->title, text_data(0, guiT + 21, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < numOptions; i++)
    {
        CentreText(menu->options[i], text_data(0, 127 + 12 * i, 8), (i == gCurrOption) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    }
    
    if (menu->displayMenu != 0)
    {
        (menu->displayMenu)(selectData);
    }

    if (GetMenuKey(M_SELECT))
    {
        selectData->select = 1;
    }

    if (GetMenuKey(M_BACK) && (gMenuHistoryIndex > 0))
    {
        /* TODO input mapping
        if (gCurrMenu == CONTROL_EDIT_MENU)
        {
            ResetKeys();
        }
        */
        GoBack();

        KeyToChange = NONE;
    }
}

void DrawMainMenu(const Menu* menu, int shapeNo)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(10);

    if (shapeNo == 0)
        DrawTeamShips(0, 0, MODELSIZE, heading, ctrlshipShapes[shipShapeOrder[0]]);

    // if (shapeNo == 1)
    // {
    //    DrawTeamShips (-450, 0, 1100, heading, menu->menuModel[shipShapeOrder[3]]) ;
    //    DrawTeamShips (450, 0, 1100, heading, menu->menuModel[shipShapeOrder[1]]) ;
    // }

    if (shapeNo == 1)
    {
        DrawObject(gameShapes + 3, 0, 0, MODELSIZE, hdg, 0, 0);
    }
}

void DrawTeams(const Menu* menu, int shapeNo)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(20);

    DrawObject(teamShapes + teamShapeOrder[shapeNo], 0, 0, 9000, hdg, 0, 0);

    DrawTeamShips(-900, 800, 1500, heading, ctrlshipShapes[shipShapeOrder[shapeNo * 2]]);
    DrawTeamShips(900, 800, 1500, heading, ctrlshipShapes[shipShapeOrder[shapeNo * 2 + 1]]);
}

void DrawTeamShips(int x, int y, int size, int heading, Object* shipModel)
{
    matrixPush();
    matrixTranslate(x, y, size);
    matrixRotateYXZ(0, -heading, 0);
    renderShip(shipModel, 0xFF808080);
    matrixPop();
}

void DrawPilot(const Menu* menu, int shapeNo)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(10);

    DrawObject(pilotShapes + pilotShapeOrder[shapeNo], 0, -400, 11000, hdg, 0, 0);
    DrawTeamShips(0, 700, 1300, heading, ctrlshipShapes[shipShapeOrder[shapeNo]]);
}

void DrawClass(const Menu* menu, int shipNo)
{
    int32 hdg = FRAME_LERP_STEP(40);

    if (shipNo == 0)
    {
        DrawObject(clsShapes + 0, 0, 0, 500, hdg, 0, 0);
    }

    if (shipNo == 1)
    {
        DrawObject(clsShapes + 1, 0, -25, 500, hdg, 0, 0);
    }
}

void InitSelectData(SelectionData* selectData)
{
    selectData->optionNo = 0;
    selectData->select = 0;
    selectData->hiscoreTrack = 0;
}

void MusicVSfx(SelectionData* selectData)
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
            if (WhatChannel(SFX_SHIELD) == -1)
            {
                PlayNote(SFX_SHIELD);
                SetPitch(SFX_SHIELD, SND_DEF_PITCH >> 1);
                SetVolume(SFX_SHIELD, SND_MAX_VOLUME, SND_MAX_VOLUME);
            }
            SetMasterBalance(gMasterBalance - (gFrameIndex - gLastFrameIndex) * 2);
        }
        else if (pad & IN_RIGHT)
        {
            if (WhatChannel(SFX_SHIELD) == -1)
            {
                PlayNote(SFX_SHIELD);
                SetPitch(SFX_SHIELD, SND_DEF_PITCH >> 1);
                SetVolume(SFX_SHIELD, SND_MAX_VOLUME, SND_MAX_VOLUME);
            }
            SetMasterBalance(gMasterBalance + (gFrameIndex - gLastFrameIndex) * 2);
        }
        else
        {
            KillNote(SFX_SHIELD);
        }
    }

    if (selectData->optionNo == 1)
    {
        KillNote(SFX_SHIELD);

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

void ShowHiScores(SelectionData* selectData)
{
    int32 actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->hiscoreTrack++;
        if (gConfig.rapierClass == 0)
        {
            if (selectData->hiscoreTrack == NumTracks)
                selectData->hiscoreTrack = (NumTracks - 1);
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
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
        if (gConfig.rapierClass == 0)
        {
            if (selectData->hiscoreTrack < 0)
                selectData->hiscoreTrack = 0;
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
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
        if (gConfig.showBonusTrack == 0)
            actualTrack -= (NumTracks + 1);
        else
            actualTrack -= NumTracks;
        CentreText("RAPIER", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }
    else
    {
        CentreText("VENOM", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }

    if (((gConfig.rapierClass == 0) && (selectData->hiscoreTrack == (NumTracks - 1))) || ((gConfig.rapierClass == 1) && (selectData->hiscoreTrack == (totalTracks - 1))))
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

void ShowTimeTrialScores(SelectionData* selectData)
{
    int32 actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(SFX_MENU_MOVE);
        selectData->hiscoreTrack++;
        if (gConfig.rapierClass == 0)
        {
            if (selectData->hiscoreTrack == (NumTracks + HiScoreTracks))
                selectData->hiscoreTrack = ((NumTracks - 1) + HiScoreTracks);
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
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
        if (gConfig.rapierClass == 0)
        {
            if (selectData->hiscoreTrack < 14)
                selectData->hiscoreTrack = 14;
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
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
        if (gConfig.showBonusTrack == 0)
            actualTrack -= ((NumTracks + 1));
        else
            actualTrack -= (NumTracks);
        CentreText("RAPIER", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }
    else
    {
        CentreText("VENOM", text_data(1, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }

    if (((gConfig.rapierClass == 0) && ((selectData->hiscoreTrack - HiScoreTracks) == (NumTracks - 1))) || ((gConfig.rapierClass == 1) && ((selectData->hiscoreTrack - HiScoreTracks) == (totalTracks - 1))))
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

void InitGameData(void)
{
    gConfig.shipNum = randy() % 8;
    gConfig.trackNum = 0;
    gConfig.actTrackNum = 0;
    gConfig.raceClass = 0;
    gConfig.team = 0;
    gConfig.raceType = 0;
    gConfig.lives = 3;
    gConfig.EndOfGame = 0;
    gConfig.Position = NO_OF_SHIPS;
    gConfig.rapierClass = 0;
    gConfig.gameComplete = 0;
    gConfig.showPilotCount = 0;
    gConfig.gameOverCount = 0;
    gConfig.venomComplete = 0;
    gConfig.newLapRecord = 0;
    gConfig.gotoMenu = 0;
    gConfig.normpadPreset = 0;
    gConfig.showBonusTrack = 0;
    gConfig.trackInc = 1;
    gConfig.lostGame = 0;
    gConfig.showGameOver = 0;
}

void ContinueGameConfirm(int32 lConfirm)
{
    CentreText("CONTINUE QUALIFYING OR QUIT", text_data(77, 95, 8), TEXT_COLOR_WHITE);

    AddText("QUIT", text_data(215, 120, 16), lConfirm ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("QUALIFY", text_data(45, 120, 16), lConfirm ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void RestartGameConfirm(int32 lConfirm)
{
    CentreText("RESTART RACE", text_data(77, 95, 16), TEXT_COLOR_WHITE);

    AddText("NO", text_data(215, 120, 16), lConfirm ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("YES", text_data(68, 120, 16), lConfirm ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void SetNormPadPresets(void)
{
    switch (gConfig.normpadPreset)
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
    int32 hdg = FRAME_LERP_STEP(40);

    renderOffset(0, -30);

    if (shipNo == 0)
    {
        DrawObject(gameShapes + 0, 0, -50, 600, hdg, 0, 0);
    }

    if (shipNo == 1)
    {
        DrawObject(gameShapes + 2, 0, -50, 600, hdg, 0, 0);
    }

    if (shipNo == 2)
    {
        DrawObject(optShapes + 2, 0, -50, 600, hdg, 0, 0);
    }
}

void DrawOptionMenus(const Menu* menu, int shipNo)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(10);

    renderOffset(0, -30);

    switch (shipNo)
    {
        case CONTROLS:
            DisplayPadMenu(menu, shipNo);
            break;
        case DETAILS:
            DrawTeamShips(0, 0, MODELSIZE, heading, ctrlshipShapes[shipShapeOrder[0]]);
            break;
        case AUDIO_CONFIG:
            DrawObject(optShapes + 3, 0, 0, 600, hdg, 0, 0);
            break;
        case HIGH_SCORES:
            DrawObject(optShapes + 2, 0, 0, 600, hdg, 0, 0);
            break;
    }
}

void DrawHiScoreMenu(const Menu* menu, int shipNo)
{
    int32 hdg = FRAME_LERP_STEP(40);

    renderOffset(0, -30);
    DrawObject(optShapes + 2, 0, 0, 600, hdg, 0, 0);
}

void DisplayPadMenu(const Menu* menu, int shapeNo)
{
    int32 hdg = FRAME_LERP_STEP(40);

    DrawObject(padShapes + 0, 0, 0, 6500, hdg, 0, 0);
}

void InitChampionship(void)
{
    memset(winTable, 0, sizeof(winTable));
}

void QuitYesNo(SelectionData* selectData)
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

void GraphicOptions(SelectionData* selectData)
{
    // TODO menu graphics
}

void ControllerOptions(SelectionData* selectData)
{
    // TODO menu controls
}

void ControllerEdit(SelectionData* selectData)
{
    // TODO menu controls
}

void DrawControllerMenu(const Menu* menu, int shapeNo)
{
    // TODO menu controls
}

void DrawControllerEdit(const Menu* menu, int shapeNo)
{
    // TODO menu controls
}

void DrawGraphicDetail(const Menu* menu, int shapeNo)
{
    int32 heading = FRAME_LERP_STEP(10);

    DrawTeamShips(0, 0, MODELSIZE, heading, ctrlshipShapes[shipShapeOrder[0]]);
}
