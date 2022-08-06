#ifndef MENUS_H
#define MENUS_H

#define NORMAL_COMBAT_LINK 1
#define MAX_OPTIONS 7
#define MAX_MENUS 30
#define VRAM 0
#define SCREEN 1
#define MODELSIZE 800 /*1800*/
#define MAX_CD_TRACKS 11
#define MAX_GAMES 15

#define CARD_FULL       101
#define DUP_NAME        102
#define LOAD_ERROR      103
#define SAVE_ERROR      104
#define RENAME_ERROR    105
#define NO_CARD         106
#define NO_FILES        107
#define NO_FORMAT       108
#define FORMAT_ERROR    109

#define FORMATTED       0
#define UNFORMATTED     1
#define NORMPAD_PRESETS 8
#define NEGCON_PRESETS  5
#define NORM            1
#define OVERWRITE       2
#define RENAME          3

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
#define DOTT        17

enum MenuScreen
{
    MAIN_MENU,
    TRACK_MENU,
    AUDIO_MENU,
    OPTION_MENU,
    CLASS_MENU,
    TEAM_MENU,
    PILOT_MENU_1,
    PILOT_MENU_2,
    PILOT_MENU_3,
    PILOT_MENU_4,
    RACE_TYPE_MENU,
    HIGH_SCORES_MENU,
    BEST_TIMES_MENU,
    TIME_TRIAL_MENU,
    QUIT_MENU,
    DETAILS_MENU,
    CONTROLLER_MENU,
    CONTROL_EDIT_MENU
};


// MAIN_MENU
#define SINGLE_GAME     0
#define MULTI_GAME      1
#define GAME_OPTIONS    1
#define QUIT_GAME       2

// CLASS_MENU
#define VENOM_CLASS     0
#define RAPIER_CLASS    1

// OPTION_MENU
#define CONTROLS        0
#define DETAILS         1
#define AUDIO_CONFIG    2
#define HIGH_SCORES     3

// TEAM_MENU
#define AG_SYSTEMS      0
#define AURICOM         1
#define QIREX           2
#define FEISAR          3

// RACE_TYPE_MENU
#define CHAMPIONSHIP    0
#define SINGLE_RACE     1
#define TIME_TRIAL      2
#define NO_GAME_SET     3

struct SelectionData;
struct ConfigData;

typedef struct Menu
{
    const char* title;
    Object** menuModel;
    Object** menu2Model;
    void (*displayObject)(const struct Menu*, int);
    void (*displayMenu)(struct SelectionData*, struct ConfigData*);
    char numOptions;
    const char* options[MAX_OPTIONS];
} Menu;

typedef struct ConfigData
{
    short gameOverCount;
    short showPilotCount;

    short showPointCount;
    short showChampCount;

    short currSliderPos;
    short oldSliderPos;

    char venomComplete;
    signed char normpadPreset;
    signed char oldPadPreset;
    char unused_1;

    char unused_2;
    char trackNum;
    char actTrackNum;
    char shipNum;

    char ship2Num;
    char Class;
    char team;
    char team2;

    char oppMenu;
    char gameType;
    char NumTracks;
    char Lives;

    char Position;
    char EndOfGame;
    char gameComplete;
    char RapierClass;

    char cardPresent;
    char pad5;
    char newLapRecord;
    char gotoMenu;

    char showGameOver;
    char showBonusTrack;
    char trackInc;
    char lostGame;
} ConfigData;

typedef struct SelectionData
{
    // short       currSliderPos;
    signed char slotRow;
    signed char slotColumn;
    signed char optionNo;
    char slotsFilled;
    char select;
    char firstTime;
    char nameEntered[9];
    char numCardFiles;
    char trackNum;
    char fadeHeight;
    char confirm;
    signed char speechMode;
    signed char hiscoreTrack;
    uint32 toggle;
    char combatInit;
    char combatComplete;
    int cardNum;
    char card1;
    char card2;
    char formatCard;
    char formatConfirm;
    char calledMenu;
    char saveType;
} SelectionData;

typedef struct ChampionShip
{
    short points;
    short racePoints;
    char pilot[25];
    short xPos;
    char position;
    char racePosition;
} ChampionShip;

typedef struct SortTable
{
    char shipNum;
    char points;
} SortTable;

void menuInit(void);
void menuLoop(ConfigData*);

void InitGameData(ConfigData*);
void InitChampionship(void);
void ContinueGameConfirm(char*, short, char*);
void RestartGameConfirm(char* confirm, short xpad, char* lConfirm);
void SetNormPadPresets(ConfigData*);

void GoPrevMusic(void);
void GoNextMusic(void);

#endif
