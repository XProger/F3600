#include "game.h"

Texture* menuScreen;

extern HiScores scoreTable[];

extern const short shipShapeOrder[];

// TODO reorder this shit
const short pilotShapeOrder[8] = { 0, 4, 6, 7, 2, 5, 1, 3 };
const short teamShapeOrder[4] = { 2, 3, 1, 0 }; // jed

ChampionShip winTable[NO_OF_SHIPS];

int modelHdg = 0;
int zoomInOut = 0;
int downUp = 0;

Texture* gButtonTex;
Texture* slideTex;
Texture* sliderTex;

int gCurrMenu;
int gCurrOption;

Object* audioShapes;
Object* pilotShapes;
Object* teamShapes;

Object* clsShapes;
Object* gameShapes;
Object* optShapes;
Object* padShapes;
SelectionData gSelectData;

Texture* gTrackIcon;

typedef struct Menu
{
    uint8 title; // StringID
    uint8 options[MAX_OPTIONS]; // StringID
    void (*drawProc)(int32 index, int32 id);
} Menu;

void DrawMenuMain(int32 index, int32 id);
void DrawMenuTrack(int32 index, int32 id);
void DrawMenuOptions(int32 index, int32 id);
void DrawMenuClass(int32 index, int32 id);
void DrawMenuRace(int32 index, int32 id);
void DrawMenuTeam(int32 index, int32 id);
void DrawMenuPilot(int32 index, int32 id);
void DrawMenuQuit(int32 index, int32 id);
void DrawMenuControls(int32 index, int32 id);
void DrawMenuGraphics(int32 index, int32 id);
void DrawMenuAudio(int32 index, int32 id);
void DrawMenuHighScores(int32 index, int32 id);
void DrawMenuBestRace(int32 index, int32 id);
void DrawMenuBestTrial(int32 index, int32 id);
void DrawMenuMultiplayer(int32 index, int32 id);

void CallMenuSelector(void);
void CallMenuActivator(void);

#define MAX_MENU_HISTORY 8

typedef struct
{
    uint8 menu;
    uint8 option;
} MenuHistory;

int gMenuHistoryIndex;
MenuHistory gMenuHistory[MAX_MENU_HISTORY];

const Menu gMenuSystem[] =
{
// MENU_MAIN
    { STR_TITLE_MAIN,
        {
            STR_MAIN_START_GAME,
        #ifdef MENU_HAS_MULTIPLAYER
            STR_MAIN_MULTIPLAYER,
        #endif
            STR_MAIN_OPTIONS,
        #ifdef MENU_HAS_QUIT
            STR_MAIN_QUIT,
        #endif
            STR_MAX
        }, DrawMenuMain
    },
// MENU_TRACK
    { STR_TITLE_TRACK,
        {
            STR_TRACK_ALTIMA,
            STR_TRACK_KARBONIS,
            STR_TRACK_TERRAMAX,
            STR_TRACK_KORODERA,
            STR_TRACK_ARRIDOS,
            STR_TRACK_SILVERSTREAM,
            STR_TRACK_FIRESTAR,
            STR_MAX
        }, DrawMenuTrack
    },
// MENU_OPTIONS
    { STR_TITLE_OPTIONS,
        {
            STR_OPTION_CONTROLS,
            STR_OPTION_GRAPHICS,
            STR_OPTION_AUDIO,
            STR_OPTION_BEST_TIMES,
            STR_MAX
        }, DrawMenuOptions
    },
// MENU_CLASS
    { STR_TITLE_CLASS,
        {
            STR_CLASS_VENOM,
            STR_CLASS_RAPIER,
            STR_MAX
        }, DrawMenuClass
    },
// MENU_RACE
    { STR_TITLE_RACE_TYPE,
        {
            STR_RACE_CHAMPIONSHIP,
            STR_RACE_SINGLE,
            STR_RACE_TIME_TRIAL,
            STR_MAX
        }, DrawMenuRace
    },
// MENU_TEAM
    { STR_TITLE_TEAM,
        {
            STR_TEAM_AG_SYSTEMS,
            STR_TEAM_AURICOM,
            STR_TEAM_QIREX,
            STR_TEAM_FEISAR,
            STR_MAX
        }, DrawMenuTeam
    },
// MENU_PILOT_1 - AG_SYSTEMS
    { STR_TITLE_PILOT,
        {
            STR_PILOT_JOHN,
            STR_PILOT_DANIEL,
            STR_MAX
        }, DrawMenuPilot
    },
// MENU_PILOT_2 - AURICOM
    { STR_TITLE_PILOT,
        {
            STR_PILOT_ARIAL,
            STR_PILOT_ANASTASIA,
            STR_MAX
        }, DrawMenuPilot
    },
// MENU_PILOT_3 - QIREX
    { STR_TITLE_PILOT,
        {
            STR_PILOT_KEL,
            STR_PILOT_ARIAN,
            STR_MAX
        }, DrawMenuPilot
    },
// MENU_PILOT_4 - FEISAR
    { STR_TITLE_PILOT,
        {
            STR_PILOT_SOFIA,
            STR_PILOT_PAUL,
            STR_MAX
        }, DrawMenuPilot
    },
// MENU_QUIT
    { STR_TITLE_QUIT,
        {
            STR_MAX
        }, DrawMenuQuit
    },
// MENU_CONTROLS
    { STR_TITLE_CONTROLS,
        {
            STR_MAX
        }, DrawMenuControls
    },
// MENU_GRAPHICS
    { STR_TITLE_GRAPHICS,
        {
            STR_MAX
        }, DrawMenuGraphics
    },
// MENU_AUDIO
    { STR_TITLE_AUDIO,
        {
            STR_MAX
        }, DrawMenuAudio
    },
// MENU_HIGH_SCORES
    { STR_TITLE_HIGH_SCORES,
        {
            STR_BEST_RACE,
            STR_BEST_TRIAL,
            STR_MAX
        }, DrawMenuHighScores
    },
// MENU_BEST_RACE
    { STR_BEST_RACE,
        {
            STR_MAX
        }, DrawMenuBestRace
    },
// MENU_BEST_TRIAL
    { STR_BEST_TRIAL,
        {
            STR_MAX
        }, DrawMenuBestTrial
    },
    { STR_TITLE_MULTIPLAYER,
        {
            STR_MAX
        }, DrawMenuMultiplayer
    },
};

void InitGameData(void)
{
    int32 i;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;

        strcpy(player->name, STR[STR_DEF_NAME_1 + i]);
        player->joyIndex = i;

        player->binding.thrust = IN_A;
        player->binding.view   = IN_Y | IN_LB | IN_L;
        player->binding.fire   = IN_B | IN_RB | IN_R;
        player->binding.lBrake = IN_RT;
        player->binding.rBrake = IN_LT;
    }

    gConfig.trackNum = 0;
    gConfig.raceClass = 0;
    gConfig.team = 0;
    gConfig.raceType = CHAMPIONSHIP;
    gConfig.lives = 3;
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

void DrawContinueGameConfirm(int32 lConfirm)
{
    CentreText(STR_ACK_CONTINUE, text_data(0, 95, 8), TEXT_COLOR_WHITE);

    AddText(STR_QUIT, text_data(215, 120, 16), lConfirm ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText(STR_QUALIFY, text_data(45, 120, 16), lConfirm ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void DrawRestartGameConfirm(int32 lConfirm)
{
    CentreText(STR_ACK_RESTART, text_data(0, 95, 16), TEXT_COLOR_WHITE);

    AddText(STR_NO, text_data(215, 120, 16), lConfirm ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText(STR_YES, text_data(68, 120, 16), lConfirm ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void InitChampionship(void)
{
    memset(winTable, 0, sizeof(winTable));
}

void CallMenuSelector(void)
{
    int32 i;
    const Menu* menu = gMenuSystem + gCurrMenu;
    int32 numOptions = 0;

    for (i = 0; i < MAX_OPTIONS; i++)
    {
        if (menu->options[i] == STR_MAX)
            break;
        numOptions++;
    }

    if (gCurrMenu == MENU_TRACK)
    {
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

    if (menu->drawProc != 0)
    {
        (menu->drawProc)(gCurrOption, menu->options[gCurrOption]);
    }

    //DrawButtons();
    CentreText(menu->title, text_data(0, guiT + 21, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < MAX_OPTIONS; i++)
    {
        if (menu->options[i] == STR_MAX)
            break;

        CentreText(menu->options[i], text_data(0, 127 + 12 * i, 8), (i == gCurrOption) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    }

    if (GetMenuKey(M_SELECT))
    {
        gSelectData.select = 1;
    }

    if (GetMenuKey(M_BACK) && (gMenuHistoryIndex > 0))
    {
        GoBack();
    }
}

void CallMenuActivator(void)
{
    int32 i;
    int32 id = gMenuSystem[gCurrMenu].options[gCurrOption];

    switch (gCurrMenu)
    {
        case MENU_MAIN:
        {
            switch (id)
            {
                case STR_MAIN_START_GAME:
                    gNumPlayers = 1;
                    GoMenu(MENU_CLASS);
                    break;
                case STR_MAIN_MULTIPLAYER:
                    gSelectData.optionNo = 0;
                    gConfig.trackNum = 0;
                    gConfig.gameMode = 0; // local
                    gConfig.raceType = SINGLE_RACE;
                    GoMenu(MENU_MULTIPLAYER);
                    break;
                case STR_MAIN_OPTIONS:
                    GoMenu(MENU_OPTIONS);
                    break;
                case STR_MAIN_QUIT:
                    gSelectData.confirm = 0;
                    GoMenu(MENU_QUIT);
                    break;
            }
            break;
        }

        case MENU_TRACK:
        {
            gConfig.trackNum = gCurrOption + gConfig.raceClass * 7;
            GoMenu(-1);
            break;
        }

        case MENU_OPTIONS:
        {
            switch (id)
            {
                case STR_OPTION_CONTROLS:
                    GoMenu(MENU_CONTROLS);
                    break;
                case STR_OPTION_GRAPHICS:
                    GoMenu(MENU_GRAPHICS);
                    break;
                case STR_OPTION_AUDIO:
                    GoMenu(MENU_AUDIO);
                    renderOffset(0, 60);
                    break;
                case STR_OPTION_BEST_TIMES:
                    GoMenu(MENU_HIGH_SCORES);
                    gSelectData.hiscoreTrack = 0;
                    break;
            }
            break;
        }

        case MENU_CLASS:
        {
            gConfig.raceClass = gCurrOption;
            if (gConfig.rapierClass == 0 && id == STR_CLASS_RAPIER)
                return;
            GoMenu(MENU_RACE);
            break;
        }

        case MENU_RACE:
        {
            switch (id)
            {
                case STR_RACE_CHAMPIONSHIP:
                    gConfig.raceType = CHAMPIONSHIP;
                    gConfig.trackNum = gConfig.raceClass * 7;
                    InitChampTable();
                    break;
                case STR_RACE_SINGLE:
                    gConfig.raceType = SINGLE_RACE;
                case STR_RACE_TIME_TRIAL:
                    gConfig.raceType = TIME_TRIAL;
                    break;
            }

            GoMenu(MENU_TEAM);
            break;
        }

        case MENU_TEAM:
        {
            gConfig.team = gCurrOption;
            GoMenu(MENU_PILOT_1 + gCurrOption);
            break;
        }

        case MENU_PILOT_1:
        case MENU_PILOT_2:
        case MENU_PILOT_3:
        case MENU_PILOT_4:
        {
            gConfig.newLapRecord = 0;
            gPlayers[0].ready = 1;
            gPlayers[0].shipNum = gCurrOption + gConfig.team * 2;
            ownShip = gPlayers[0].shipNum;

            if (gConfig.raceType == CHAMPIONSHIP)
            {
                GoMenu(-1);
            }
            else
            {
                GoMenu(MENU_TRACK);
            }
            break;
        }

        case MENU_HIGH_SCORES:
        {
            switch (id)
            {
                case STR_BEST_RACE:
                    gSelectData.hiscoreTrack = 0;
                    GoMenu(MENU_BEST_RACE);
                    break;
                case STR_BEST_TRIAL:
                    gSelectData.hiscoreTrack = 14;
                    GoMenu(MENU_BEST_TRIAL);
                    break;
            }
            break;
        }

        case MENU_QUIT:
        {
            if (gSelectData.confirm)
            {
                osQuit();
            }
            else
            {
                GoBack();
            }
            break;
        }
    }
}

void menuInit(void)
{
    int32 i;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        gPlayers[i].shipNum = -1;
        gPlayers[i].ready = 0;
    }

    LoadMainMenu();

    gSelectData.optionNo = 0;
    gSelectData.select = 0;
    gSelectData.hiscoreTrack = 0;

    gMenuHistoryIndex = 0;
    gCurrMenu = 0;
    gCurrOption = 0;

    gCameraFOV = CAMERA_FOV;
    gCameraRotY = 0;
}

void menuLoop(void)
{
    UpdatePad();

    renderFrameBegin();
    renderClear();
    renderSetMode(RENDER_MODE_2D);

    renderBackground(menuScreen);

    if (gCurrMenu >= 0)
        CallMenuSelector();

    if (gSelectData.select)
    {
        if (gCurrMenu == MENU_MAIN && gCurrOption == 0)
        {
            int32 pad = gPlayers[0].pad;
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

        CallMenuActivator();
    }

    if (gCurrMenu == MENU_CLASS && gCurrOption == 1 && (!gConfig.rapierClass))
        CentreText(STR_NOT_AVAILABLE, text_data(0, 190, 12), TEXT_COLOR_HIGHLIGHT);

    renderFrameEnd();

    if (gCurrMenu == -1)
    {
        ctrlmode = INTERACTIVE;
        ctrlGameLoop = GAME_LOOP_RACE_INIT;
        return;
    }

    if (gSelectData.select == 1)
    {
        gSelectData.select = 0;
    }
}

void DisplayButtons(int x, int y, int buttType)
{
    renderImage(gButtonTex + buttType, x, y);
}

void DrawButtons(void)
{
    if (gMenuSystem[gCurrMenu].options[0] != STR_MAX)
        DisplayButtons(30, 210, PSELECT);

    if (gCurrMenu != MENU_MAIN)
        DisplayButtons(240, 210, PBACK);
}

void DrawTeamShips(int x, int y, int size, int heading, Object* shipModel)
{
    matrixPush();
    matrixTranslate(x, y, size);
    matrixRotateYXZ(0, -heading, 0);
    renderShip(shipModel, 0xFF808080);
    matrixPop();
}

void DrawMenuMain(int32 index, int32 id)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(10);

    renderOffset(0, -30);
    renderSetMode(RENDER_MODE_3D);

    if (id == STR_MAIN_START_GAME)
    {
        DrawTeamShips(0, 0, MODELSIZE, heading, ctrlshipShapes + 0);
    }

    if (id == STR_MAIN_MULTIPLAYER)
    {
        DrawTeamShips(-450, 0, 1100, heading, ctrlshipShapes + 3);
        DrawTeamShips(450, 0, 1100, -heading, ctrlshipShapes + 1);
    }

    if (id == STR_MAIN_OPTIONS)
    {
        DrawObject(gameShapes + 3, 0, 0, MODELSIZE, hdg, 0, 0);
    }

    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuTrack(int32 index, int32 id)
{
    renderImage(gTrackIcon + gCurrOption, 96, 50);
    CentreText((gConfig.raceClass == 0) ? STR_CLASS_VENOM : STR_CLASS_RAPIER, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
}

void DrawMenuOptions(int32 index, int32 id)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(10);

    renderOffset(0, -30);

    renderSetMode(RENDER_MODE_3D);

    switch (id)
    {
        case STR_OPTION_CONTROLS:
            DrawObject(padShapes + 0, 0, 0, 6500, hdg, 0, 0);
            break;
        case STR_OPTION_GRAPHICS:
            DrawTeamShips(0, 0, MODELSIZE, heading, ctrlshipShapes + 0);
            break;
        case STR_OPTION_AUDIO:
            DrawObject(optShapes + 3, 0, 0, 600, hdg, 0, 0);
            break;
        case STR_OPTION_BEST_TIMES:
            DrawObject(optShapes + 2, 0, 0, 600, hdg, 0, 0);
            break;
    }

    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuClass(int32 index, int32 id)
{
    int32 hdg = FRAME_LERP_STEP(40);

    renderSetMode(RENDER_MODE_3D);

    if (id == STR_CLASS_VENOM)
    {
        DrawObject(clsShapes + 0, 0, 0, 500, hdg, 0, 0);
    }

    if (id == STR_CLASS_RAPIER)
    {
        DrawObject(clsShapes + 1, 0, -25, 500, hdg, 0, 0);
    }

    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuRace(int32 index, int32 id)
{
    int32 hdg = FRAME_LERP_STEP(40);

    renderOffset(0, -30);

    renderSetMode(RENDER_MODE_3D);

    if (id == STR_RACE_CHAMPIONSHIP)
    {
        DrawObject(gameShapes + 0, 0, -50, 600, hdg, 0, 0);
    }

    if (id == STR_RACE_SINGLE)
    {
        DrawObject(gameShapes + 2, 0, -50, 600, hdg, 0, 0);
    }

    if (id == STR_RACE_TIME_TRIAL)
    {
        DrawObject(optShapes + 2, 0, -50, 600, hdg, 0, 0);
    }

    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuTeam(int32 index, int32 id)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(20);

    renderSetMode(RENDER_MODE_3D);

    DrawObject(teamShapes + teamShapeOrder[index], 0, 0, 9000, hdg, 0, 0);

    DrawTeamShips(-900, 800, 1500, heading, ctrlshipShapes + (index * 2));
    DrawTeamShips(900, 800, 1500, heading, ctrlshipShapes + (index * 2 + 1));

    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuPilot(int32 index, int32 id)
{
    int32 hdg = FRAME_LERP_STEP(40);
    int32 heading = FRAME_LERP_STEP(10);

    int32 pilotIndex = id - STR_PILOT_JOHN;

    renderSetMode(RENDER_MODE_3D);

    DrawObject(pilotShapes + pilotShapeOrder[pilotIndex], 0, -400, 11000, hdg, 0, 0);
    DrawTeamShips(0, 700, 1300, heading, ctrlshipShapes + pilotIndex);

    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuQuit(int32 index, int32 id)
{
    CentreText(STR_ACK_SURE, text_data(0, 95, 8), TEXT_COLOR_HIGHLIGHT);
    CentreText(STR_ACK_WANT_QUIT, text_data(0, 105, 8), TEXT_COLOR_HIGHLIGHT);

    if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
    {
        gSelectData.confirm ^= 1;
    }

    AddText(STR_YES, text_data(80, 120, 16), gSelectData.confirm ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText(STR_NO, text_data(210, 120, 16), gSelectData.confirm ? TEXT_COLOR_WHITE : BLINK_TEXT);
}

void DrawMenuControls(int32 index, int32 id)
{
    // TODO menu controls
}

void DrawMenuGraphics(int32 index, int32 id)
{
    int32 heading = FRAME_LERP_STEP(10);

    renderSetMode(RENDER_MODE_3D);
    DrawTeamShips(0, 0, MODELSIZE, heading, ctrlshipShapes + 0);
    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuAudio(int32 index, int32 id)
{
    int32 opt = gSelectData.optionNo;

    if (GetMenuKey(M_UP))
    {
        opt--;
        if (opt < 0)
            opt = 1;
    }

    if (GetMenuKey(M_DOWN))
    {
        opt++;
        if (opt > 1)
            opt = 0;
    }

    gSelectData.optionNo = opt;

    renderImage(sliderTex, 39 + (gMasterBalance * 218 >> 8), 61);
    renderImage(slideTex, 48, 85);

    AddText(STR_AUDIO_SFX, text_data(33, 51, 8), (opt == 0) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText(STR_AUDIO_MUSIC, text_data(250, 139, 8), (opt == 0) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText(STR_AUDIO_TRACK, text_data(33, 175, 8), (opt == 1) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddTextStr(GetMusicTrackName(gMusicTrack), text_data(130, 175, 8), TEXT_COLOR_HIGHLIGHT);

    if (opt == 0)
    {
        int32 pad = gPlayers[0].pad;

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

    if (opt == 1)
    {
        KillNote(SFX_SHIELD);

        if (GetMenuKey(M_LEFT))
        {
            GoPrevMusic();
        }
        else if (GetMenuKey(M_RIGHT))
        {
            GoNextMusic();
        }
    }
}

void DrawMenuHighScores(int32 index, int32 id)
{
    int32 hdg = FRAME_LERP_STEP(40);

    renderOffset(0, -30);

    renderSetMode(RENDER_MODE_3D);
    DrawObject(optShapes + 2, 0, 0, 600, hdg, 0, 0);
    renderSetMode(RENDER_MODE_2D);
}

void DrawMenuBestRace(int32 index, int32 id)
{
    int32 actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(SFX_MENU_MOVE);
        gSelectData.hiscoreTrack++;
        if (gConfig.rapierClass == 0)
        {
            if (gSelectData.hiscoreTrack == NumTracks)
                gSelectData.hiscoreTrack = (NumTracks - 1);
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
            {
                if (gSelectData.hiscoreTrack == NumTracks)
                    gSelectData.hiscoreTrack = (NumTracks + 1);
            }

            if (gSelectData.hiscoreTrack == totalTracks)
                gSelectData.hiscoreTrack = (totalTracks - 1);
        }
    }

    if (GetMenuKey(M_LEFT))
    {
        PlayNote(SFX_MENU_MOVE);
        gSelectData.hiscoreTrack--;
        if (gConfig.rapierClass == 0)
        {
            if (gSelectData.hiscoreTrack < 0)
                gSelectData.hiscoreTrack = 0;
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
            {
                if (gSelectData.hiscoreTrack == NumTracks)
                    gSelectData.hiscoreTrack = (NumTracks - 1);
            }

            if (gSelectData.hiscoreTrack < 0)
                gSelectData.hiscoreTrack = 0;
        }
    }

    actualTrack = gSelectData.hiscoreTrack;

    if (gSelectData.hiscoreTrack >= NumTracks)
    {
        if (gConfig.showBonusTrack == 0)
            actualTrack -= (NumTracks + 1);
        else
            actualTrack -= NumTracks;
        CentreText(STR_RAPIER, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }
    else
    {
        CentreText(STR_VENOM, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }

    if (((gConfig.rapierClass == 0) && (gSelectData.hiscoreTrack == (NumTracks - 1))) || ((gConfig.rapierClass == 1) && (gSelectData.hiscoreTrack == (totalTracks - 1))))
    {
    //    DisplayButtons(290, 116, ARROWR);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(290, 116, ARROWR);
    }

    if (gSelectData.hiscoreTrack == 0)
    {
    //    DisplayButtons(20, 116, ARROWL);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(20, 116, ARROWL);
    }

    CentreText(gMenuSystem[MENU_TRACK].options[actualTrack], text_data(0, 127 - 70, 12), TEXT_COLOR_HIGHLIGHT);

    if (BLINK_SLOW)
    {
        AddText(STR_LAP_RECORD, text_data(40, 195, 8), TEXT_COLOR_WHITE);
        AddTime(scoreTable[gSelectData.hiscoreTrack].lapRecord, text_data(165, 188, 16), TEXT_COLOR_HIGHLIGHT);
    }

    DisplayHiScore(gSelectData.hiscoreTrack);
}

void DrawMenuBestTrial(int32 index, int32 id)
{
    int32 actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(SFX_MENU_MOVE);
        gSelectData.hiscoreTrack++;
        if (gConfig.rapierClass == 0)
        {
            if (gSelectData.hiscoreTrack == (NumTracks + HiScoreTracks))
                gSelectData.hiscoreTrack = ((NumTracks - 1) + HiScoreTracks);
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
            {
                if (gSelectData.hiscoreTrack == (NumTracks + HiScoreTracks))
                    gSelectData.hiscoreTrack = ((NumTracks + 1) + HiScoreTracks);
            }

            if (gSelectData.hiscoreTrack == (totalTracks + HiScoreTracks))
                gSelectData.hiscoreTrack = ((totalTracks - 1) + HiScoreTracks);
        }
    }

    if (GetMenuKey(M_LEFT))
    {
        PlayNote(SFX_MENU_MOVE);
        gSelectData.hiscoreTrack--;
        if (gConfig.rapierClass == 0)
        {
            if (gSelectData.hiscoreTrack < 14)
                gSelectData.hiscoreTrack = 14;
        }
        else
        {
            if (gConfig.showBonusTrack == 0)
            {
                if (gSelectData.hiscoreTrack == (NumTracks + HiScoreTracks))
                    gSelectData.hiscoreTrack = ((NumTracks - 1) + HiScoreTracks);
            }

            if (gSelectData.hiscoreTrack < 14)
                gSelectData.hiscoreTrack = 14;
        }
    }

    actualTrack = (gSelectData.hiscoreTrack - HiScoreTracks);

    if ((gSelectData.hiscoreTrack - HiScoreTracks) >= (NumTracks))
    {
        if (gConfig.showBonusTrack == 0)
            actualTrack -= ((NumTracks + 1));
        else
            actualTrack -= (NumTracks);
        CentreText(STR_RAPIER, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }
    else
    {
        CentreText(STR_VENOM, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
    }

    if (((gConfig.rapierClass == 0) && ((gSelectData.hiscoreTrack - HiScoreTracks) == (NumTracks - 1))) || ((gConfig.rapierClass == 1) && ((gSelectData.hiscoreTrack - HiScoreTracks) == (totalTracks - 1))))
    {
    //    DisplayButtons(290, 116, ARROWR);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(290, 116, ARROWR);
    }

    if ((gSelectData.hiscoreTrack - HiScoreTracks) == 0)
    {
    //    DisplayButtons(20, 116, ARROWL);
    }
    else if (BLINK_SLOW)
    {
        DisplayButtons(20, 116, ARROWL);
    }

    CentreText(gMenuSystem[MENU_TRACK].options[actualTrack], text_data(0, 127 - 70, 12), TEXT_COLOR_HIGHLIGHT);

    if (BLINK_SLOW)
    {
        AddText(STR_LAP_RECORD, text_data(40, 195, 8), TEXT_COLOR_WHITE);
        AddTime(scoreTable[gSelectData.hiscoreTrack].lapRecord, text_data(165, 188, 16), TEXT_COLOR_HIGHLIGHT);
    }

    DisplayHiScore(gSelectData.hiscoreTrack);
}

void DrawMenuMultiplayer(int32 index, int32 id)
{
    int32 i;
    int32 x;
    int32 y = 40;
    int32 hdg = FRAME_LERP_STEP(40);
    int32 opt = gSelectData.optionNo;
    int32 gameMode = gConfig.gameMode;
    int32 trackNum = gConfig.trackNum;
    int32 raceClass = gConfig.raceClass;
    int32 raceType = gConfig.raceType;
    int32 done;

    if (GetMenuKey(M_UP))
    {
        opt--;
        if (opt < 0)
            opt = 4;
    }

    if (GetMenuKey(M_DOWN))
    {
        opt++;
        if (opt > 4)
            opt = 0;
    }

    if (GetMenuKey(M_LEFT))
    {
        switch (opt)
        {
            case 0: // mode
                //gameMode ^= 1;
                break;
            case 1: // track
                trackNum--;
                if (trackNum < 0)
                    trackNum = 6;
                break;
            case 2: // class
                raceClass ^= 1;
                break;
            case 3: // type
                raceType--;
                if (raceType < SINGLE_RACE)
                    raceType = TIME_TRIAL;
                break;
        }
        PlayNote(SFX_MENU_MOVE);
    }

    if (GetMenuKey(M_RIGHT))
    {
        switch (opt)
        {
            case 0: // mode
                //gameMode ^= 1;
                break;
            case 1: // track
                trackNum++;
                if (trackNum > 6)
                    trackNum = 0;
                break;
            case 2: // class
                raceClass ^= 1;
                break;
            case 3: // type
                raceType++;
                if (raceType > TIME_TRIAL)
                    raceType = SINGLE_RACE;
                break;
        }
        PlayNote(SFX_MENU_MOVE);
    }

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        int32 xpad = gPlayers[i].xpad;

        if (gPlayers[i].shipNum == -1)
        {
            if (xpad)
            {
                gPlayers[i].shipNum = 0;
            }
        }
        else
        {
            if ((i == 0 && opt == 4) || (i != 0))
            {
                if (GetMenuKeyPlayer(M_LEFT, i))
                {
                    gPlayers[i].shipNum--;
                    if (gPlayers[i].shipNum < 0)
                        gPlayers[i].shipNum = 7;
                }

                if (GetMenuKeyPlayer(M_RIGHT, i))
                {
                    gPlayers[i].shipNum++;
                    if (gPlayers[i].shipNum > 7)
                        gPlayers[i].shipNum = 0;
                }

                if (GetMenuKeyPlayer(M_SELECT, i))
                {
                    gPlayers[i].ready ^= 1;
                }

                if (GetMenuKeyPlayer(M_BACK, i))
                {
                    gPlayers[i].ready = 0;
                }
            }

            if (i > 0)
            {
                if (GetMenuKeyPlayer(M_START, i))
                {
                    gPlayers[i].shipNum = -1;
                }
            }
        }
    }

    gSelectData.optionNo = opt;
    gConfig.gameMode = gameMode;
    gConfig.trackNum = trackNum;
    gConfig.raceClass = raceClass;
    gConfig.raceType = raceType;

    x = -120;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        if (gPlayers[i].shipNum != -1)
        {
            renderOffset(x, -8 + 24);
            renderSetMode(RENDER_MODE_3D);
            DrawTeamShips(0, 800, 2200, hdg, ctrlshipShapes + gPlayers[i].shipNum); // gPlayers[i].shipNum
        }
        hdg += 1024;
        x += 80;
    }

    renderSetMode(RENDER_MODE_2D);

    // track thumbnail
    renderImage(gTrackIcon + trackNum, 320 - 128 - 20, y);

    y += 4;

    // mode
    AddText(STR_MULTI_MODE, text_data(20, y, 8), TEXT_COLOR_HIGHLIGHT);
    y += 12;
    AddText(STR_MODE_LOCAL + gameMode, text_data(28, y, 8), (opt == 0) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    y += 12;

    // circuit
    AddText(STR_MULTI_CIRCUIT, text_data(20, y, 8), TEXT_COLOR_HIGHLIGHT);
    y += 12;
    AddText(STR_TRACK_ALTIMA + trackNum, text_data(28, y, 8), (opt == 1) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    y += 12;

    // race class
    AddText(STR_MULTI_CLASS, text_data(20, y, 8), TEXT_COLOR_HIGHLIGHT);
    y += 12;
    AddText((raceClass == 0) ? STR_VENOM : STR_RAPIER, text_data(28, y, 8), (opt == 2) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    y += 12;

    // race type
    AddText(STR_MULTI_TYPE, text_data(20, y, 8), TEXT_COLOR_HIGHLIGHT);
    y += 12;
    AddText(STR_RACE_CHAMPIONSHIP + raceType, text_data(28, y, 8), (opt == 3) ? BLINK_TEXT : TEXT_COLOR_WHITE);

    y += 32;

    x = -120;

    done = gPlayers[0].ready;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        if (gPlayers[i].ready)
        {
            CentreTextStr(gPlayers[i].name, text_data(x, y, 4), TEXT_COLOR_WHITE);
            CentreText(STR_MULTI_READY, text_data(x, y + 24, 8), TEXT_COLOR_HIGHLIGHT);
        }
        else
        {
            if (gPlayers[i].shipNum != -1)
            {
                CentreTextStr(gPlayers[i].name, text_data(x, y, 4), (opt == 4) || (i > 0) ? BLINK_TEXT : TEXT_COLOR_WHITE);
                CentreText(STR_TEAM_AG_SYSTEMS + gPlayers[i].shipNum / 2, text_data(x, y + 8, 4), TEXT_COLOR_HIGHLIGHT);
            }
            else
            {
                CentreTextStr(gPlayers[i].name, text_data(x, y, 4), TEXT_COLOR_WHITE);
                if (BLINK_SLOW)
                {
                    CentreText(STR_PRESS_ANY_KEY, text_data(x, y + 24, 4), TEXT_COLOR_HIGHLIGHT);
                }
            }
        }

        x += 80;

        if (!gPlayers[i].ready && (gPlayers[i].shipNum != -1))
        {
            done = 0;
        }
    }

    if (done)
    {
        gNumPlayers = 0;
        for (i = 0; i < MAX_PLAYERS; i++)
        {
            if (gPlayers[i].ready)
            {
                gNumPlayers++;
            }
        }

        ownShip = gPlayers[0].shipNum;
        gConfig.trackNum = gConfig.trackNum + gConfig.raceClass * 7;
        GoMenu(-1);
    }
}
