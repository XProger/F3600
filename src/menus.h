#ifndef MENUS_H
#define MENUS_H

#define NORMAL_COMBAT_LINK 1
#define MAX_OPTIONS 11
#define MAX_MENUS 30
#define VRAM 0
#define SCREEN 1
#define MODELSIZE 800 /*1800*/
#define MAX_CD_TRACKS 11
#define MAX_GAMES 15

/* BUTTON TYPE */
#define PSELECT     0
#define PSTART      1
#define PBACK       2
#define PCHANGE     3
#define PFORMAT     4
#define NSELECT     5
#define NSTART      6
#define NBACK       7
#define NCHANGE     8
#define NFORMAT     9
#define THRUST      10
#define PADFIRE     11
#define VIEW        12
#define LBRAKE      13
#define RBRAKE      14
#define ARROWL      15
#define ARROWR      16

enum MenuScreen
{
    MENU_MAIN,
    MENU_TRACK,
    MENU_OPTIONS,
    MENU_CLASS,
    MENU_RACE,
    MENU_TEAM,
    MENU_PILOT_1,
    MENU_PILOT_2,
    MENU_PILOT_3,
    MENU_PILOT_4,
    MENU_QUIT,
    MENU_CONTROLS,
    MENU_GRAPHICS,
    MENU_AUDIO,
    MENU_HIGH_SCORES,
    MENU_BEST_RACE,
    MENU_BEST_TRIAL,
    MENU_MULTIPLAYER
};

#define CHAMPIONSHIP    0
#define SINGLE_RACE     1
#define TIME_TRIAL      2

struct SelectionData;

typedef struct ConfigData
{
    char raceClass;
    char raceType;
    char team;
    char lives;

    char gameOverCount;
    char showPilotCount;
    char showPointCount;
    char showChampCount;

    char gameMode;
    char trackNum;
    char padding_unused_0;
    char padding_unused_1;

    char Position;
    char EndOfGame;
    char gameComplete;
    char rapierClass;

    char venomComplete;
    char normpadPreset;
    char newLapRecord;
    char gotoMenu;

    char showGameOver;
    char showBonusTrack;
    char trackInc;
    char lostGame;
} ConfigData;

typedef struct SelectionData
{
    signed char optionNo;
    char select;
    char confirm;
    signed char hiscoreTrack;
} SelectionData;

typedef struct ChampionShip
{
    uint8 points;
    uint8 racePoints;
    uint8 position;
    uint8 racePosition;
    char pilot[24];
} ChampionShip;

typedef struct SortTable
{
    char shipNum;
    char points;
} SortTable;

extern ChampionShip winTable[];

void menuInit(void);
void menuLoop(void);

void InitGameData(void);
void InitChampionship(void);
void DrawContinueGameConfirm(int32 lConfirm);
void DrawRestartGameConfirm(int32 lConfirm);

void GoPrevMusic(void);
void GoNextMusic(void);

#endif
