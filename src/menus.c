#include "game.h"

extern short trackNum; // SJR
extern long ctrlGameLoop;

char andybodge = 1;
extern short endtrack, starttrack;
short globalvol;

Texture* menuScreen;

extern TexChar CharSet16[];
extern HiScores scoreTable[];

extern const short shipShapeOrder[];
const short pilotShapeOrder[8] = { 0, 4, 6, 7, 2, 5, 1, 3 };
const short teamShapeOrder[4] = { 2, 3, 1, 0 }; // jed
ChampionShip winTable[NO_OF_SHIPS];
int modelHdg = 0;
int zoomInOut = 0;
int downUp = 0;

Texture* buttonTex[18];

extern const char* GetCdTrackName(int no);
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

#define MAXSTRING   10


//#define SAVEDIR       "wipeout\\"
#define FILE_EXTENSION ".wsf"
/*TODO saves
extern char *SaveDirBase;
*/

char DelayCounter = 0;
char KeyToChange = NONE;
char davebodge = 1;
char davebodge2 = 0;

short TextEntry = 0;
short TextPos = 0;
char LoadFilename[MAXSTRING];
char SaveFilename[MAXSTRING];

int gCurrMenu;
int gCurrOption;

extern long ctrlfirsttime;

Menus menuSystem[MAX_MENUS];
Object* shipShapes[NO_OF_SHIPS];
Object* audioShapes[3];
Object* pilotShapes[8];
Object* teamShapes[4];

Object* clsShapes[3];
Object* gameShapes[4];
Object* optShapes[7];
Object* padShapes[3];
Object* negShapes[3];
Object** classShapes = clsShapes;
Object** gameTypeShapes = gameShapes;
Object** optionShapes = optShapes;
Object** joypadShapes = padShapes;
Object** negconShapes = negShapes;
SelectionData selectData;

Texture* trackTex[7];
SlideTextures musicTex;
MusicPrims musicPrims;
char loadGames[MAX_GAMES][9];

const char trackList[15] = { 2, 4, 1, 12, 8, 9, 10, 3, 5, 6, 7, 11, 13, 14, 15 };

void DrawControllerMenu(Menus* menuSystem, int* currMenu, int shapeNo);
void DrawControllerEdit(Menus* menuSystem, int* currMenu, int shapeNo);
void EnterLoadGame(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData);
void KeyConfig0(void);
void KeyConfig(void);
void DrawGraphicDetail(Menus* menuSystem, int* currMenu, int shapeNo);
void DisplayPadMenu1(Menus* menuSystem, int* currMenu, int shapeNo);
void DisplayPadMenu2(Menus* menuSystem, int* currMenu, int shapeNo);
void ResetKeys(void);

#define MenuModelSkeletonsMax 36

Skeleton MenuModelSkeletons[MenuModelSkeletonsMax];

void menuInit(void)
{
    Skeleton* skeletons = MenuModelSkeletons;
    skeletons += LoadTexModel(shipShapes, skeletons, "allsh");
    skeletons += LoadTexModel(pilotShapes, skeletons, "pilot");
    skeletons += LoadTexModel(teamShapes, skeletons, "team"); // jed
    skeletons += LoadTexModel(audioShapes, skeletons, "cdobj"); /*audio = headphones*/
    skeletons += LoadTexModel(classShapes, skeletons, "leeg"); // jed
    skeletons += LoadTexModel(joypadShapes, skeletons, "cntrl"); // jed
    skeletons += LoadTexModel(gameTypeShapes, skeletons, "msdos"); // jed
    skeletons += LoadTexModel(optionShapes, skeletons, "misc"); // jed

    InitTrackTex(trackTex);

    trackNum = 15; // pretend track for restoring - SJR

    // following inits were in the #if lightsourced section, this is now all we have (PCwipeout)
    menuSystem[MAIN_MENU].menuModel = shipShapes;
    menuSystem[MAIN_MENU].menu2Model = gameTypeShapes;
    menuSystem[TRACK_MENU].menuModel = shipShapes;
    // menuSystem[ AUDIO_MENU ].menuModel = optionShapes ;
    menuSystem[OPTION_MENU].menuModel = optionShapes;
    menuSystem[CLASS_MENU].menuModel = classShapes;
    menuSystem[TEAM_MENU].menuModel = teamShapes;
    menuSystem[TEAM_MENU].menu2Model = shipShapes;
    menuSystem[PILOT_MENU].menuModel = pilotShapes;
    menuSystem[PILOT_MENU].menu2Model = shipShapes;
    menuSystem[RACE_TYPE_MENU].menuModel = gameTypeShapes;
    menuSystem[RACE_TYPE_MENU].menu2Model = optionShapes;
    menuSystem[SERIAL_RACE_TYPE_MENU].menuModel = shipShapes;
    menuSystem[NEGCON_MENU].menuModel = negconShapes;
    menuSystem[NEGCON_MENU].menu2Model = negconShapes;
    menuSystem[TWIST_MENU].menuModel = negconShapes;
    menuSystem[TWIST_MENU].menu2Model = negconShapes;
    menuSystem[BEST_TIMES_MENU].menuModel = optionShapes;
    menuSystem[CONTROLLER_MENU].menuModel = joypadShapes;
    menuSystem[CONTROLLER_MENU].menu2Model = joypadShapes;
    menuSystem[CONTROL_EDIT_MENU].menuModel = joypadShapes;
    menuSystem[CONTROL_EDIT_MENU].menu2Model = joypadShapes;
    menuSystem[DETAILS_MENU].menuModel = shipShapes;

    InitMenuDisplay(90);
    InitLoadSave(&(loadGames[0]));
    InitMusicVSfx(&musicTex);
    InitButtons("buttons");

    if (ctrlfirsttime)
        InitSelectData(&selectData);

    selectData.fadeHeight = 0;
    selectData.select = 0;
    selectData.combatInit = 0;
    selectData.combatComplete = 0;
    menuSystem[1].numOptions = NumTracks;

    InitMenuSystem(&(menuSystem[0]));

    selectData.menuSystem = &(menuSystem[0]);

    UpdateNotes(NOT_PAUSED);

    menuScreen = LoadTexture("wipeout\\textures\\wipeout1.tim");

    gCurrMenu = 0;
    gCurrOption = 0;
}

void menuLoop(ConfigData* gameData)
{
    int i;

    UpdatePad();

    UpdateNotes(NOT_PAUSED);

    /*TODO title screen backgound*/
    renderClear();

    {
        if ((gCurrMenu == NEGCON_MENU) || (gCurrMenu == TWIST_MENU))
            gCurrMenu = PADCONFIG_MENU;

        menuSystem[OPTION_MENU].menu2Model = joypadShapes;
        menuSystem[PADCONFIG_MENU].menuModel = joypadShapes;
        menuSystem[PADCONFIG_MENU].displayMenu = MenuPlayPad;
        menuSystem[PADCONFIG_MENU].displayObject = DrawPlayPad;
        menuSystem[PADCONFIG_MENU].prevMenu = OPTION_MENU;
    }

    if (gCurrMenu != -2)
    {
        MenuErrors(&gCurrMenu, &(menuSystem[0]));
        if (menuSystem[gCurrMenu].errorFlag != 0)
        {
            selectData.fadeHeight = 0;
            selectData.select = 0;
        }
    }

    if (menuSystem[gCurrMenu].updateCount != 0 && menuSystem[gCurrMenu].errorFlag == 0)
        menuSystem[gCurrMenu].updateCount--;

    DisplayPicture(menuScreen, 0, 0, 320, 240);

    if (menuSystem[gCurrMenu].errorFlag == 0)
    {
        if (gCurrMenu >= 0)
            CallMenuSelector(&(menuSystem[0]), &gCurrMenu, &gCurrOption, shipShapes, gameData, &musicPrims, &musicTex, &selectData, &(loadGames[0]));

        if (gCurrMenu == TRACK_MENU)
            DisplayPicture(trackTex[gCurrOption], 96, 50, 128, 74);

        if (((gCurrMenu == SAVE_MENU) || (gCurrMenu == LOAD_MENU)) && (selectData.formatCard == 1))
        {
            gCurrMenu = FORMAT_CONFIRM;
            gCurrOption = 0;
        }

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

            if (menuSystem[gCurrMenu].updateCount == 0)
                CallMenuActivator(gameData, &gCurrMenu, &gCurrOption, &(menuSystem[0]), &selectData, &(loadGames[0]));
        }

        if (davebodge == 2 || davebodge == 3 || davebodge == 4)
            CallMenuActivator(gameData, &gCurrMenu, &gCurrOption, &(menuSystem[0]), &selectData, &(loadGames[0]));
    }

    if (gCurrMenu == TRACK_MENU)
    {
        if (gameData->Class == 0)
            CentreText("VENOM CLASS", text_data(20, 40, 8), RedText);
        else
            CentreText("RAPIER CLASS", text_data(20, 40, 8), RedText);
    }

    if (gCurrMenu == CLASS_MENU && gCurrOption == 1 && (!gameData->RapierClass))
        CentreText("NOT AVAILABLE", text_data(20, 190, 12), RedText);

    /*** Serial link up within menu structure ***/

    if (gameType == SINGLE)
    {
        if (selectData.combatComplete == 32)
        {
            gCurrMenu = MAIN_MENU;
            gCurrOption = 0;
            selectData.combatInit = 0;
            selectData.combatComplete = 0;
        }
    }

    if (gameType == MASTER || gameType == SLAVE)
    {
        if (selectData.combatComplete == 32)
        {
            gCurrMenu = TRACK_MENU;
            gCurrOption = 0;
            selectData.combatInit = 0;
            selectData.combatComplete = 0;

            if (gameType == SINGLE)
                gCurrMenu = MAIN_MENU;
            else if (gameType == MASTER || gameType == SLAVE)
            {
                if (gameType == MASTER)
                    gCurrMenu = CLASS_MENU;

                if (gameType == SLAVE)
                {
                    gCurrMenu = TEAM_MENU;
                    menuSystem[TEAM_MENU].prevMenu = TEAM_MENU;
                }
            }
        }
    }

    /********************************************/

    selectData.trackNum = gameData->trackNum;

    if (gCurrMenu == -1)
    {
        ctrlGameLoop = GAME_LOOP_RACE_INIT;

        for (i = 0; i < 10; i++)
        {
            /*TODO audio*/
            // c_attr.cd.volume.left  -= c_attr.cd.volume.left >> 3 ;   /* Master volume (left) */
            // c_attr.cd.volume.right -= c_attr.cd.volume.right >> 3;   /* Master volume (right) */
            UpdateNotes(NOT_PAUSED);
        }
        // CdControlB(CdlStop, 0, 0);

        return;
    }

    if (menuSystem[gCurrMenu].updateCount == 0 && menuSystem[gCurrMenu].errorFlag == 0 && selectData.select == 1)
    {
        selectData.select = 0;
        menuSystem[gCurrMenu].updateCount = 0;
    }
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void CallMenuActivator(ConfigData* gameData, int* currMenu, int* currOption, Menus* menuSystem, SelectionData* selectData, char loadGames[][9])
{
    short i;
    long cardStatus, slotStatus;

    /******************/
    /***** Menu 1 *****/
    /******************/

    if (*currMenu == MAIN_MENU)
    {
        menuSystem[TEAM_MENU].prevMenu = RACE_TYPE_MENU;
        menuSystem[CLASS_MENU].prevMenu = MAIN_MENU;

        if (*currOption == SINGLE_GAME)
        {
            //       CommMultiPlaySelect=_SINGLE_PLAY;
            gameType = SINGLE;
            *currMenu = CLASS_MENU;
            *currOption = 0;
            menuSystem[TEAM_MENU].prevMenu = RACE_TYPE_MENU;
            menuSystem[CLASS_MENU].prevMenu = MAIN_MENU;
        }

        //    if (*currOption == MULTI_GAME)
        //    {
        //       *currMenu = MULTI_MENU;
        //       *currOption = 0;
        //    }                       // jed

        if (*currOption == GAME_OPTIONS)
        {
            *currMenu = OPTION_MENU;
            *currOption = 0;
        }

        if (*currOption == QUIT_TO_DOS)
        {
            *currMenu = QUIT_MENU;
            *currOption = 0;
        }
        return;
    }

    /*******************/
    /***** Menu 28 *****/
    /*******************/

    if (*currMenu == MULTI_MENU)
    {
        if (*currOption == DUAL_GAME)
        {
            //       *currMenu = INIT_TWO_PLAYER ;
            andybodge = 1;
            *currMenu = SERIAL_MENU;
            *currOption = 0;

            //       menuSystem[CLASS_MENU].prevMenu = CLASS_MENU ;
            //       if (gameType == SLAVE)
            //          menuSystem[TEAM_MENU].prevMenu = TEAM_MENU ;
            //
            //       gameType = SINGLE ;
            //       gameData->shipNum = 0 ;
            //       gameData->ship2Num = 1 ;
            //       selectData->combatInit = 0 ;
            //       selectData->combatComplete = 0 ;
            //       InitializeCombatLink () ;
        }

        if (*currOption == NETWORK_GAME)
        {
            andybodge = 1;
            *currMenu = NETWORK_MENU;
            *currOption = 0;
        }
        return;
    }

    /*******************/
    /***** Menu 29 *****/
    /*******************/

    if (*currMenu == NETWORK_MENU)
    {
        if (davebodge == 2)
        {
            davebodge = 1;
            *currMenu = INIT_TWO_PLAYER;
        }
        return;
    }

    /*******************/
    /***** Menu 30 *****/
    /*******************/

    if (*currMenu == SERIAL_MENU)
    {
        if (davebodge == 2)
        {
            davebodge = 1;
            *currMenu = INIT_TWO_PLAYER;
        }
        return;
    }

    /******************/
    /***** Menu 2 *****/
    /******************/

    if (*currMenu == TRACK_MENU)
    {
        if (gameData->Class)
            gameData->trackNum = (*currOption + 1) + 7;
        else
            gameData->trackNum = (*currOption + 1);
        gameData->actTrackNum = trackList[(gameData->trackNum - 1)];
        *currMenu = -1;
        *currOption = 0;
        gameData->newLapRecord = 0;

        if (gameType == SINGLE)
        {
            ownShip = gameData->shipNum;
            serialShip = -1;
        }
        else
        {
            if (gameType == MASTER)
            {
                if (gameData->oppMenu == START_TWO_PLAYER_MENU)
                    *currMenu = -1;
                else
                    *currMenu = START_TWO_PLAYER_MENU;

                ownShip = gameData->shipNum;
                serialShip = gameData->ship2Num;
            }
        }
        return;
    }

    /******************/
    /***** Menu 3 *****/
    /******************/

    if (*currMenu == DUAL_GAME_MENU)
    {
        if (*currOption == MASTER_PSX)
        {
            gameType = MASTER;
            *currMenu = TRACK_MENU;
            *currOption = 0;
            menuSystem[1].prevMenu = DUAL_GAME_MENU;
        }

        if (*currOption == SLAVE_PSX)
        {
            gameType = SLAVE;
            *currMenu = TRACK_MENU;
            *currOption = 0;
            menuSystem[1].prevMenu = DUAL_GAME_MENU;
        }
        return;
    }

    /******************/
    /***** Menu 5 *****/
    /******************/

    if (*currMenu == OPTION_MENU)
    {
        if (*currOption == LOAD_GAME)
        {
            selectData->formatConfirm = 0;
            selectData->formatCard = 0;
            selectData->slotRow = 0;
            selectData->slotColumn = 0;
            selectData->slotsFilled = 0;

            for (i = 0; i < 2; i++)
            {
                slotStatus = GetCardInfo(i);

                switch (slotStatus)
                {
                    case 0:
                        break; /* CARD EXISTS */
                    case 1:
                        break; /* ERROR */
                    case 2:
                        break; /* NO CARD */
                    case 3:
                        //                      if (i == 0)
                        //                         _card_clear (0) ; // PCwipeout
                        //                      else
                        //                         _card_clear (16) ; // PCwipeout
                        cardStatus = CheckHWCardEvents();
                        break; /* NEW CARD */
                }

                LoadCard(i);
                cardStatus = CardContentStatus();

                switch (cardStatus)
                {
                    case 0:
                        if (i == 0)
                            selectData->card1 = FORMATTED;
                        else
                            selectData->card2 = FORMATTED;
                        break;
                    case 3:
                        if (i == 0)
                            selectData->card1 = UNFORMATTED;
                        else
                            selectData->card2 = UNFORMATTED;
                        break; /* UNFORMATTED */
                }
            }
            LoadCardFiles(selectData, &(loadGames[0]), selectData->cardNum);
            *currMenu = LOAD_MENU;
            *currOption = 0;
        }

        if (*currOption == SAVE_GAME)
        {
            selectData->formatCard = 0;
            selectData->formatConfirm = 0;
            selectData->slotRow = 0;
            selectData->slotColumn = 0;
            selectData->slotsFilled = 0;

            for (i = 0; i < 2; i++)
            {
                slotStatus = GetCardInfo(i);

                switch (slotStatus)
                {
                    case 0:
                        break; /* CARD EXISTS */
                    case 1:
                        break; /* ERROR */
                    case 2:
                        break;
                    case 3:
                        //                      if (i == 0)
                        //                         _card_clear (0) ; // PCwipeout
                        //                      else
                        //                         _card_clear (16) ; // PCwipeout
                        cardStatus = CheckHWCardEvents();
                        break; /* NEW CARD */
                }

                LoadCard(i);
                cardStatus = CardContentStatus();

                switch (cardStatus)
                {
                    case 0:
                        if (i == 0)
                            selectData->card1 = FORMATTED;
                        else
                            selectData->card2 = FORMATTED;
                        break; /* FORMATTED */
                    case 1:
                        break; /* ERROR */
                    case 2:
                    case 3:
                        if (i == 0)
                            selectData->card1 = UNFORMATTED;
                        else
                            selectData->card2 = UNFORMATTED;
                }
            }

            LoadCardFiles(selectData, &(loadGames[0]), selectData->cardNum);
            *currMenu = ENTER_GAME_NAME;
            *currOption = 0;
        }

        if (*currOption == CONTROLS)
        {
            *currMenu = CONTROLLER_MENU;
            *currOption = 0;
        }

        if (*currOption == DETAILS)
        {
            *currMenu = DETAILS_MENU;
            *currOption = 0;
        }

        /*if (*currOption == AUDIO_CONFIG)
        {
        //       PlayNote(SHIELDS, 0, -1);
        *currMenu = AUDIO_MENU ;
        *currOption = 0 ;
        selectData->optionNo = 0 ;

          InitMenuDisplay ( 180 ) ;
          gameData->oldSliderPos = gameData->currSliderPos ;
          gameData->oldCdTrack = gameData->cdTrack ;
        }*/
        if (*currOption == HIGH_SCORES)
        {
            selectData->hiscoreTrack = 0;
            *currMenu = BEST_TIMES_MENU;
            *currOption = 0;
        }
        return;
    }

    /******************/
    /***** Menu 6 *****/
    /******************/

    if (*currMenu == CLASS_MENU)
    {
        gameData->Class = *currOption;
        if (gameData->RapierClass == 0 && *currOption == 1)
            return;
        *currMenu = RACE_TYPE_MENU;

        if (gameType == SINGLE || gameType == SLAVE)
        {
            *currMenu = RACE_TYPE_MENU;
            menuSystem[CLASS_MENU].prevMenu = MAIN_MENU;
        }
        else
        {
            *currMenu = SERIAL_RACE_TYPE_MENU;
        }

        if (*currOption == 0)
        {
            gameData->trackNum = 1; /* 1 */
        }

        if (*currOption == 1)
        {
            if (gameData->showBonusTrack == 0)
                gameData->trackNum = (NumTracks + 2); /* (NumTracks+2)*/
            else
                gameData->trackNum = (NumTracks + 1); /* (NumTracks+1)*/
        }

        *currOption = 0;
        return;
    }

    /******************/
    /***** Menu 7 *****/
    /******************/

    if (*currMenu == TEAM_MENU)
    {
        gameData->team = *currOption;
        SetUpPilots(gameData->team, menuSystem);
        *currMenu = PILOT_MENU;
        *currOption = 0;

        if (gameType == SINGLE)
        {
            menuSystem[TEAM_MENU].prevMenu = RACE_TYPE_MENU;
        }
        else if (gameType == SLAVE)
        {
            menuSystem[TEAM_MENU].prevMenu = TEAM_MENU;
        }
        else
        {
            menuSystem[TEAM_MENU].prevMenu = SERIAL_RACE_TYPE_MENU;
        }

        return;
    }

    /******************/
    /***** Menu 8 *****/
    /******************/

    if (*currMenu == PILOT_MENU)
    {
        gameData->shipNum = (gameData->team * 2) + *currOption;
        *currOption = 0;
        gameData->newLapRecord = 0;

        if (gameType == SINGLE)
        {
            *currMenu = TRACK_MENU;
            if (gameData->gameType == CHAMPIONSHIP)
            {
                *currMenu = -1;
                *currOption = 0;
                ownShip = gameData->shipNum;
                serialShip = -1;
            }
        }
        else if (gameType == MASTER || gameType == SLAVE)
        {
            if (gameType == MASTER)
                *currMenu = TRACK_MENU;

            if (gameType == SLAVE)
            {
                if (gameData->oppMenu == START_TWO_PLAYER_MENU)
                {
                    *currMenu = -1;
                    ownShip = gameData->shipNum;
                    serialShip = gameData->ship2Num;
                }
                else
                    *currMenu = START_TWO_PLAYER_MENU;
            }
        }
        return;
    }

    /******************/
    /***** Menu 9 *****/
    /******************/

    if (*currMenu == RACE_TYPE_MENU)
    {
        *currMenu = TEAM_MENU;
        menuSystem[1].prevMenu = PILOT_MENU;
        menuSystem[TEAM_MENU].prevMenu = RACE_TYPE_MENU;

        raceType = 0;

        if (((*currOption) + 1) == CHAMPIONSHIP)
        {
            *currOption = 0;

            /***** Initialize Championship Data *****/

            for (i = 0; i < NO_OF_SHIPS; i++)
            {
                winTable[i].points = 0;
                winTable[i].position = 1;
            }

            InitChampTable();

            /***** Set track to play *****/

            menuSystem[1].numOptions = NumTracks;
            gameData->actTrackNum = trackList[(gameData->trackNum - 1)];
            gameData->gameType = CHAMPIONSHIP;
        }

        if (((*currOption) + 1) == TIME_TRIAL)
        {
            *currOption = 0;
            menuSystem[1].numOptions = NumTracks;
            gameData->gameType = TIME_TRIAL;
            raceType = TIME_TRIAL;
        }

        if ((*currOption + 1) == SINGLE_RACE)
        {
            *currOption = 0;
            menuSystem[1].numOptions = NumTracks;
            gameData->gameType = SINGLE_RACE;
        }
        return;
    }

    /*******************/
    /***** Menu 10 *****/
    /*******************/

    if (*currMenu == LOAD_MENU)
    {
        if (TextPos < 1)
            return;

        LoadFilename[TextPos] = '\0';
        TextEntry = 0;
        TextPos = 0;

        if (!LoadCardData(LoadFilename, selectData->cardNum, gameData, selectData))
        {
            menuSystem[(*currMenu)].updateCount = 120;
            menuSystem[(*currMenu)].errorFlag = LOAD_ERROR;
            return;
        }

        strcpy(LoadFilename, "         ");
        LoadFilename[0] = 101;

        *currMenu = MAIN_MENU;
        *currOption = 0;

        menuSystem[1].numOptions = NumTracks;
        return;
    }

    /*******************/
    /***** Menu 11 *****/
    /*******************/

    if (*currMenu == SAVE_MENU)
    {
        if ((selectData->slotsFilled == 0) && (selectData->numCardFiles == 15))
            return;
        *currMenu = ENTER_GAME_NAME;
        *currOption = 0;
        selectData->firstTime = 1;
        return;
    }

    /*******************/
    /***** Menu 12 *****/
    /*******************/
    if (*currMenu == ENTER_GAME_NAME)
    {
        if (TextPos < 1)
            return;

        /*TODO saves
              SaveFilename[TextPos] = '\0';
              TextEntry = 0;
              TextPos = 0;

              CheckSaveType (selectData, &(loadGames[0]), gameData) ;

              char DaveSave[250];
              strcpy(DaveSave, SaveDirBase);
              strcat(DaveSave,"\\");
              strcat(DaveSave, SaveFilename);
              strcat(DaveSave, FILE_EXTENSION);

              fd = fopen(DaveSave, "r");

              if(fd == NULL)
              {
                 // Save
                 if (!SaveCardFile (SaveFilename, selectData->cardNum, gameData, gameData->currSliderPos))
                 {
                    menuSystem[(*currMenu)].updateCount = 120 ;
                    menuSystem[(*currMenu)].errorFlag = SAVE_ERROR ;
                    return ;
                 }

                 strcpy (loadGames[(selectData->slotRow*2)+selectData->slotColumn], selectData->nameEntered) ;
                 strcpy(SaveFilename, "         ");
                 SaveFilename[0] = 101;
                 *currMenu = OPTION_MENU;
              }
              else
              {
                 // Confirm overwrite
                 fclose(fd);
                 *currMenu = CONFIRM_SAVE ;
              }
        */
        *currOption = 0;
        return;
    }

    /*******************/
    /***** Menu 14 *****/
    /*******************/

    if (*currMenu == CONFIRM_SAVE)
    {
        if ((!selectData->confirm) && (SaveFilename != 0))
        {
            // This bit actually does the saving!

            if (!SaveCardFile(SaveFilename, selectData->cardNum, gameData, gameData->currSliderPos))
            {
                menuSystem[(*currMenu)].updateCount = 120;
                menuSystem[(*currMenu)].errorFlag = SAVE_ERROR;
                return;
            }

            strcpy(loadGames[(selectData->slotRow * 2) + selectData->slotColumn], selectData->nameEntered);
        }

        strcpy(SaveFilename, "         ");
        SaveFilename[0] = 101;
        *currMenu = OPTION_MENU;
        *currOption = 0;

        return;
    }

    /*******************/
    /***** Menu 15 *****/
    /*******************/

    if (*currMenu == INIT_TWO_PLAYER)
    {
        if (davebodge == 3)
        {
            davebodge = 4;
            //       menuSystem[(*currMenu)].errorFlag = global_mark_error;
            menuSystem[(*currMenu)].updateCount = 400;
            gameType = SINGLE;
            return;
        }

        //    *currMenu = TRACK_MENU ;
        *currOption = 0;
        selectData->combatInit = 0;
        selectData->combatComplete = 0;

        if (gameType == SINGLE)
        {
            *currMenu = MAIN_MENU;
            davebodge = 1;
        }
        else if (gameType == MASTER || gameType == SLAVE)
        {
            if (gameType == MASTER)
            {
                *currMenu = CLASS_MENU;
                menuSystem[CLASS_MENU].prevMenu = CLASS_MENU;
            }
            if (gameType == SLAVE)
            {
                *currMenu = TEAM_MENU;
                menuSystem[TEAM_MENU].prevMenu = TEAM_MENU;
            }
        }

        return;
    }

    /*******************/
    /***** Menu 16 *****/
    /*******************/

    if (*currMenu == SERIAL_RACE_TYPE_MENU)
    {
        *currMenu = TEAM_MENU;
        menuSystem[1].prevMenu = PILOT_MENU;
        menuSystem[TEAM_MENU].prevMenu = SERIAL_RACE_TYPE_MENU;

        if (*currOption == 0)
        {
            gameData->gameType = TWO_PLAYER_RACE;
            raceType = TWO_PLAYER_RACE;
        }

        if (*currOption == 1)
        {
            raceType = HEAD_TO_HEAD;
            gameData->gameType = HEAD_TO_HEAD;
        }
        *currOption = 0;
        return;
    }

    /*******************/
    /***** Menu 18 *****/
    /*******************/

    if (*currMenu == PADCONFIG_MENU)
    {
        SetNormPadPresets(gameData);
        gameData->normpadPreset = gameData->oldPadPreset;
        *currMenu = OPTION_MENU;

        *currOption = 0;
        return;
    }

    /*******************/
    /***** Menu 19 *****/
    /*******************/
    /*
    if (*currMenu == FORMAT_CONFIRM)
    {
    if (selectData->formatConfirm == 1)
    {
    if (selectData->cardNum == 0)
    {
             ret = format ("bu00:") ;

               if (ret == 0)
               {
               menuSystem[(*currMenu)].updateCount = 120 ;
               menuSystem[(*currMenu)].errorFlag = FORMAT_ERROR ;
               *currMenu = selectData->calledMenu ;
               selectData->formatCard = 0 ;
               selectData->formatConfirm = 0 ;
               *currOption = 0 ;
               return ;
               }
               else
               {
               *currMenu = selectData->calledMenu ;
               selectData->formatCard = 0 ;
               *currOption = 0 ;
               selectData->card1 = FORMATTED ;
               InitLoadSave (&(loadGames[0])) ;
               selectData->slotRow = 0 ;
               selectData->slotColumn = 0 ;
               selectData->slotsFilled = 0 ;
               }
               }
               else
               {
               ret = format ("bu10:") ;
               if (ret == 0)
               {
               menuSystem[(*currMenu)].updateCount = 120 ;
               menuSystem[(*currMenu)].errorFlag = FORMAT_ERROR ;
               *currMenu = selectData->calledMenu ;
               selectData->formatCard = 0 ;
               selectData->formatConfirm = 0 ;
               *currOption = 0 ;
               return ;
               }
               else
               {
               *currMenu = selectData->calledMenu ;
               selectData->formatCard = 0 ;
               *currOption = 0 ;
               selectData->card2 = FORMATTED ;
               InitLoadSave (&(loadGames[0])) ;
               selectData->slotRow = 0 ;
               selectData->slotColumn = 0 ;
               selectData->slotsFilled = 0 ;
               }
               }
               }

                selectData->formatCard = 0 ;

                  *currMenu = selectData->calledMenu ;
                  *currOption = 0 ;

                   return ;
                   }
    */
    /*******************/
    /***** Menu 20 *****/
    /*******************/

    if (*currMenu == NEGCON_MENU)
    {
        if (*currOption == 0)
        {
            *currOption = 0;
            *currMenu = PADCONFIG_MENU;
        }

        if (*currOption == 1)
        {
            *currOption = 0;
            *currMenu = TWIST_MENU;
        }
        return;
    }

    /*******************/
    /***** Menu 21 *****/
    /*******************/

    if (*currMenu == TWIST_MENU)
    {
        *currOption = 0;
        *currMenu = NEGCON_MENU;
        return;
    }

    /*******************/
    /***** Menu 22 *****/
    /*******************/

    if (*currMenu == BEST_TIMES_MENU)
    {
        if (*currOption == 0)
        {
            selectData->hiscoreTrack = 0;
            *currMenu = HIGH_SCORES_MENU;
            *currOption = 0;
        }

        if (*currOption == 1)
        {
            selectData->hiscoreTrack = 14;
            *currMenu = TIME_TRIAL_MENU;
            *currOption = 0;
        }
    }

    /*******************/
    /***** Menu 23 *****/
    /*******************/

    if (*currMenu == QUIT_MENU)
    {
        if (selectData->confirm)
        {
            *currMenu = MAIN_MENU;
            *currOption = 0;
        }
        else
        {
            osQuit();
        }
    }

    /*******************/
    /***** Menu 24 *****/
    /*******************/

    if (*currMenu == DETAILS_MENU)
    {
        //    *currMenu = MAIN_MENU ;
        *currMenu = OPTION_MENU;
        *currOption = 0;
    }

    /*******************/
    /***** Menu 26 *****/
    /*******************/

    if (*currMenu == CONTROL_EDIT_MENU)
    {
        *currMenu = CONTROLLER_MENU;
        *currOption = 0;

        if (davebodge == 2)
            davebodge = 1;

        return;
    }

    /*******************/
    /***** Menu 25 *****/
    /*******************/

    if (*currMenu == CONTROLLER_MENU)
    {
        // store new variables
        *currOption = 0;
        /*TODO input
              controller = orig_controller;
              if (changecontroller)
              {
                 // change the controller to new settings
                 etc_InputDeviceSelect=controller;
                 if(etc_InputDeviceSelect != _KEYBOARD)
                    StartPAD();

                 if((etc_InputDeviceSelect == _JOYSTICK1) && (JoyMinX == 0))
                 {
                    if (JJSJoyConfig())
                    {
                       etc_Joystick1Switch = _DEVICE_ON;
                       etc_InputDeviceSelect = _JOYSTICK1;
                    } else {
                       etc_Joystick1Switch = _DEVICE_OFF;
                       etc_InputDeviceSelect = _KEYBOARD;
                       orig_controller = _KEYBOARD;
                       StartPAD();
                    }

                    *currMenu = OPTION_MENU;
                 } else
                    *currMenu = OPTION_MENU;
              }
              else
              {
                 *currMenu = CONTROL_EDIT_MENU ;
              }
        */
    }
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

void CallMenuSelector(Menus* menuSystem, int* currMenu, int* currOption, Object** shipShapes, ConfigData* gameData, MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9])
{
    int offSet = 12, i;
    int startOption = 0, menuOffset = 0, dispOption = 0;

    if (GetMenuKey(M_UP))
    {
        PlayNote(MENU_MOVE, 1, 20);
        (*currOption)--;
        if ((*currOption) < 0)
        {
            (*currOption) = (menuSystem[(*currMenu)].numOptions - 1);
        }
    }

    if (GetMenuKey(M_DOWN))
    {
        PlayNote(MENU_MOVE, 1, 20);
        (*currOption)++;
        if ((*currOption) > (menuSystem[(*currMenu)].numOptions - 1))
            (*currOption) = 0;
    }

    if (menuSystem[(*currMenu)].displayObject != 0)
    {
        if (*currMenu == PILOT_MENU)
            (menuSystem[(*currMenu)].displayObject)(menuSystem, currMenu, (gameData->team * 2) + *currOption);
        else if (*currMenu == TEAM_MENU)
            (menuSystem[(*currMenu)].displayObject)(menuSystem, currMenu, *currOption);
        else
            (menuSystem[(*currMenu)].displayObject)(menuSystem, currMenu, *currOption);
    }

    if (menuSystem[(*currMenu)].numOptions != 0)
    {

        if (*currMenu == PILOT_MENU && (gameType == MASTER) && *currMenu == gameData->oppMenu)
        {
            gameData->shipNum = *currOption;

            if ((gameData->team == gameData->team2) && (gameData->shipNum == gameData->ship2Num))
                *currOption = (1 - gameData->ship2Num);
        }
        else if (*currMenu == PILOT_MENU && (gameType == SLAVE))
            gameData->shipNum = *currOption;

        if ((gameType == MASTER || gameType == SLAVE))
        {
            if (gameType == SLAVE && (gameData->oppMenu == START_TWO_PLAYER_MENU || gameData->oppMenu == TRACK_MENU) && *currMenu == PILOT_MENU)
            {
                if (gameData->team2 == gameData->team)
                {
                    *currOption = (1 - (gameData->ship2Num % 2));
                    gameData->shipNum = *currOption;
                }
            }

            if (gameType == MASTER && gameData->oppMenu == START_TWO_PLAYER_MENU && *currMenu == PILOT_MENU)
            {
                if (gameData->team2 == gameData->team)
                {
                    *currOption = (1 - (gameData->ship2Num % 2));
                    gameData->shipNum = *currOption;
                }
            }
        }

        {
            if (((gameData->gameType == CHAMPIONSHIP) && (*currMenu == PILOT_MENU)) || (*currMenu == TRACK_MENU) || (gameType == SLAVE && *currMenu == PILOT_MENU))
            {
                DisplayButtons(30, 210, PSELECT);
                DisplayButtons(240, 210, PBACK);
            }
            else
            {
                DisplayButtons(30, 210, PSELECT);
                if ((*currMenu != MAIN_MENU) && (!(((gameType == SLAVE) && (*currMenu == TEAM_MENU)) || ((gameType == MASTER) && (*currMenu == CLASS_MENU)))))
                    DisplayButtons(240, 210, PBACK);
            }
        }

        startOption = (*currOption / NumTracks) * NumTracks;

        dispOption = menuSystem[(*currMenu)].numOptions - startOption;

        if (dispOption >= NumTracks)
            dispOption = NumTracks;

        CentreText(menuSystem[(*currMenu)].menuTitle, text_data(0, 21, 8), RedText);

        for (i = startOption; i < (startOption + dispOption); i++)
        {
            CentreText(menuSystem[(*currMenu)].menuOptions[i].optionText, text_data(0, 127 + (offSet * menuOffset), 8), ((i == (*currOption)) && ((gFrameIndex & 3) < 2)) ? RedText : WhiteText);
            menuOffset++;
        }


        /*TODO options
        textX = 0;
        textY = 130;

        menuPointer = (Texture *)FontTextureTable [DRTEXT_8+1] ;

        SetPolyFT4(&(musicPrims->pointer));
        musicPrims->pointer.r0=230;   // PCwipeout
        musicPrims->pointer.g0=0;
        musicPrims->pointer.b0=BLACK_NOT_DRAWN;

        setXY4(&(musicPrims->pointer), textX-29, textY+((*currOption%5)*offSet),
           textX-5, textY+((*currOption%5)*offSet),
           textX-29, textY+16+((*currOption%5)*offSet),
           textX-5, textY+16+((*currOption%5)*offSet));

        setUV4(&(musicPrims->pointer), menuPointer->u0, menuPointer->v0,
           menuPointer->u1, menuPointer->v1,
           menuPointer->u2, menuPointer->v2+1,
           menuPointer->u3, menuPointer->v3+1) ;

        setTPage(&(musicPrims->pointer),0,0,menuPointer->textureX,menuPointer->textureY);
        musicPrims->pointer.clut = menuPointer->cba;
        */
    }
    else
    {
        if (menuSystem[(*currMenu)].displayMenu != 0)
            (menuSystem[(*currMenu)].displayMenu)(musicPrims, musicTex, selectData, &(loadGames[0]), gameData);
    }

    if (GetMenuKey(M_SELECT))
    {
        if (menuSystem[(*currMenu)].updateCount == 0 && menuSystem[(*currMenu)].errorFlag == 0)
        {
            if (!(*currMenu == ENTER_GAME_NAME && (selectData->nameEntered[strlen(selectData->nameEntered) - 1] >= 'A' && selectData->nameEntered[strlen(selectData->nameEntered) - 1] <= ('Z' + 1))))
            {
                selectData->select = 1;
                menuSystem[(*currMenu)].updateCount = 0;
            }
            PlayNote(MENU_SELECT, 1, 20);
            InitMenuDisplay(90);
        }
    }

    if (GetMenuKey(M_BACK)) // ESCAPE
    {
        if (menuSystem[(*currMenu)].prevMenu != -1)
        {
            //       if(*currMenu == AUDIO_MENU)
            //          KillNote(SHIELDS);

            if (*currMenu == CONTROL_EDIT_MENU) // && controller == _KEYBOARD)
            {
                // JJS new bit 0606 for exiting menus
                {
                    ResetKeys();
                }
            }

            *currMenu = menuSystem[(*currMenu)].prevMenu;
            *currOption = 0;

            SetUpPilots(gameData->team, menuSystem);

            strcpy(LoadFilename, "         ");
            LoadFilename[0] = 101;
            strcpy(SaveFilename, "         ");
            SaveFilename[0] = 101;
            TextEntry = 0;
            TextPos = 0;

            KeyToChange = NONE;

            PlayNote(MENU_SELECT, 1, 20);
            InitMenuDisplay(90);
        }
    }
}

void InitMenuSystem(Menus* menuSystem)
{
    char i;

    for (i = 0; i < MAX_MENUS; i++)
    {
        menuSystem[i].updateCount = 0;
        menuSystem[i].errorFlag = 0;
        menuSystem[i].menuTitle = "";
    }

    /******************/
    /***** Menu 1 *****/
    /******************/

    /* Menu title details */

    menuSystem[0].menuTitle = "WIPEOUT MAIN MENU";

    /* Option details */

    menuSystem[0].menuOptions[0].optionText = "START GAME";

    // menuSystem[0].menuOptions[1].optionText = "MULTI PLAYER";

    menuSystem[0].menuOptions[1].optionText = "OPTIONS";

    menuSystem[0].menuOptions[2].optionText = "QUIT";

    /* Menu details */

    menuSystem[0].numOptions = 3;
    menuSystem[0].displayObject = DrawMainMenu;
    menuSystem[0].displayMenu = 0;
    menuSystem[0].prevMenu = -1;

    /******************/
    /***** Menu 2 *****/
    /******************/

    /* Menu title details */

    menuSystem[1].menuTitle = "SELECT RACING CIRCUIT";

    /* Option details */

    menuSystem[1].menuOptions[0].optionText = "ALTIMA VII";

    menuSystem[1].menuOptions[1].optionText = "KARBONIS V";

    menuSystem[1].menuOptions[2].optionText = "TERRAMAX";

    menuSystem[1].menuOptions[3].optionText = "KORODERA";

    menuSystem[1].menuOptions[4].optionText = "ARRIDOS IV";

    menuSystem[1].menuOptions[5].optionText = "SILVERSTREAM";

    menuSystem[1].menuOptions[6].optionText = "FIRESTAR";

    /* Menu details */

    menuSystem[1].displayObject = 0;
    menuSystem[1].displayMenu = 0;
    menuSystem[1].prevMenu = PILOT_MENU;

    /******************/
    /***** Menu 3 *****/
    /******************/

    /* Menu title details */

    menuSystem[2].menuTitle = "SELECT TWO PLAYER GAME";

    /* Option details */

    menuSystem[2].menuOptions[0].optionText = "MASTER";

    menuSystem[2].menuOptions[1].optionText = "SLAVE";

    /* Menu details */

    menuSystem[2].numOptions = 2;
    menuSystem[2].displayObject = 0;
    menuSystem[2].displayMenu = 0;
    menuSystem[2].prevMenu = MAIN_MENU;

    /******************/
    /***** Menu 4 *****/
    /******************/

    /******************/
    /***** Menu 5 *****/
    /******************/

    /* Menu title details */

    menuSystem[4].menuTitle = "GAME CONFIGURATION";

    /* Option details */

    menuSystem[4].menuOptions[0].optionText = "LOAD DATA";

    menuSystem[4].menuOptions[1].optionText = "SAVE DATA";

    menuSystem[4].menuOptions[2].optionText = "CONTROLLER CONFIG";

    menuSystem[4].menuOptions[3].optionText = "GRAPHICS DETAIL";

    // menuSystem[4].menuOptions[4].optionText = "AUDIO CONFIG";

    menuSystem[4].menuOptions[4].optionText = "VIEW BEST TIMES";

    /* Menu details */

    menuSystem[4].numOptions = 5;
    menuSystem[4].displayObject = DrawOptionMenus;
    menuSystem[4].displayMenu = 0;
    menuSystem[4].prevMenu = MAIN_MENU;

    /******************/
    /***** Menu 6 *****/
    /******************/

    /* Menu title details */

    menuSystem[5].menuTitle = "SELECT RACING CLASS";

    /* Option details */

    menuSystem[5].menuOptions[0].optionText = "VENOM CLASS";

    menuSystem[5].menuOptions[1].optionText = "RAPIER CLASS";

    /* Menu details */

    menuSystem[5].numOptions = 2;
    menuSystem[5].displayObject = DrawClass;
    menuSystem[5].displayMenu = 0;
    menuSystem[5].prevMenu = MAIN_MENU;

    /******************/
    /***** Menu 7 *****/
    /******************/

    /* Menu title details */

    menuSystem[6].menuTitle = "SELECT YOUR TEAM";

    /* Option details */

    menuSystem[6].menuOptions[0].optionText = "AG SYSTEMS";

    menuSystem[6].menuOptions[1].optionText = "AURICOM";

    menuSystem[6].menuOptions[2].optionText = "QIREX";

    menuSystem[6].menuOptions[3].optionText = "FEISAR";

    /* Menu details */

    menuSystem[6].numOptions = 4;
    menuSystem[6].displayObject = DrawTeams;
    menuSystem[6].displayMenu = 0;
    menuSystem[6].prevMenu = RACE_TYPE_MENU;

    /******************/
    /***** Menu 8 *****/
    /******************/

    /* Menu title details */

    menuSystem[7].menuTitle = "CHOOSE YOUR PILOT";

    /* Option details */

    /* Menu details */

    menuSystem[7].numOptions = 2;
    menuSystem[7].displayObject = DrawPilot;
    menuSystem[7].displayMenu = 0;
    menuSystem[7].prevMenu = TEAM_MENU;

    /******************/
    /***** Menu 9 *****/
    /******************/

    /* Menu title details */

    menuSystem[8].menuTitle = "SELECT RACE TYPE";

    /* Option details */

    menuSystem[8].menuOptions[0].optionText = "CHAMPIONSHIP RACE";

    menuSystem[8].menuOptions[1].optionText = "SINGLE RACE";

    menuSystem[8].menuOptions[2].optionText = "TIME TRIAL";

    /* Menu details */

    menuSystem[8].numOptions = 3;
    menuSystem[8].displayObject = DrawGames;
    menuSystem[8].displayMenu = 0;
    menuSystem[8].prevMenu = CLASS_MENU;

    /*******************/
    /***** Menu 10 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[9].numOptions = 0;
    menuSystem[9].displayObject = 0;
    menuSystem[9].displayMenu = EnterLoadGame;
    menuSystem[9].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 11 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[10].numOptions = 0;
    menuSystem[10].displayObject = 0;
    menuSystem[10].displayMenu = EnterSaveGame; // DLSSave
    menuSystem[10].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 12 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[11].numOptions = 0;
    menuSystem[11].displayObject = 0;
    menuSystem[11].displayMenu = EnterSaveGame;
    menuSystem[11].prevMenu = OPTION_MENU; // DLSSave

    /*******************/
    /***** Menu 13 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[12].numOptions = 0;
    menuSystem[12].displayObject = 0;
    menuSystem[12].displayMenu = ShowHiScores;
    menuSystem[12].prevMenu = BEST_TIMES_MENU;

    /*******************/
    /***** Menu 14 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[13].numOptions = 0;
    menuSystem[13].displayObject = 0;
    menuSystem[13].displayMenu = ConfirmSaveGame;
    menuSystem[13].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 15 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[14].numOptions = 0;
    menuSystem[14].displayObject = 0;
    // JJS menuSystem[14].displayMenu = SerialNetworkHandling ;
    menuSystem[14].prevMenu = MAIN_MENU;

    /******************/
    /***** Menu 16 *****/
    /******************/

    /* Menu title details */

    menuSystem[15].menuTitle = "SELECT RACE TYPE";

    /* Option details */

    menuSystem[15].menuOptions[0].optionText = "MULTI PLAYER RACE";

    menuSystem[15].menuOptions[1].optionText = "HEAD TO HEAD";

    /* Menu details */

    menuSystem[15].numOptions = 2;
    menuSystem[15].displayObject = DrawShip;
    menuSystem[15].displayMenu = 0;
    menuSystem[15].prevMenu = CLASS_MENU;

    /*******************/
    /***** Menu 17 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[16].numOptions = 0;
    menuSystem[16].displayObject = 0;
    // menuSystem[16].displayMenu = StartTwoPlayer ;
    menuSystem[16].prevMenu = START_TWO_PLAYER_MENU;

    /*******************/
    /***** Menu 18 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[17].numOptions = 0;
    menuSystem[17].displayObject = 0;
    menuSystem[17].displayMenu = 0;
    menuSystem[17].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 19 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[18].numOptions = 0;
    menuSystem[18].displayObject = 0;
    menuSystem[18].displayMenu = ConfirmFormat;
    menuSystem[18].prevMenu = SAVE_MENU;

    /******************/
    /***** Menu 20 *****/
    /******************/

    /* Menu title details */

    menuSystem[19].menuTitle = "CONFIGURE NEGCON";

    /* Option details */

    menuSystem[19].menuOptions[0].optionText = "DEFINE BUTTON PRESETS";

    menuSystem[19].menuOptions[1].optionText = "DEFINE NEGCON TWIST";

    /* Menu details */

    menuSystem[19].numOptions = 2;
    menuSystem[19].displayObject = DisplayNegConMenu;
    menuSystem[19].displayMenu = 0;
    menuSystem[19].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 21 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[20].numOptions = 0;
    menuSystem[20].displayObject = DrawNegCon;
    menuSystem[20].displayMenu = 0;
    menuSystem[20].prevMenu = NEGCON_MENU;

    /******************/
    /***** Menu 22 *****/
    /******************/

    /* Menu title details */

    menuSystem[21].menuTitle = "VIEW BEST TIMES";

    /* Option details */

    menuSystem[21].menuOptions[0].optionText = "VIEW BEST RACE TIMES";

    menuSystem[21].menuOptions[1].optionText = "VIEW BEST TIME TRIAL TIMES";

    /* Menu details */

    menuSystem[21].numOptions = 2;
    menuSystem[21].displayObject = DrawHiScoreMenu;
    menuSystem[21].displayMenu = 0;
    menuSystem[21].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 23 *****/
    /*******************/

    /* Option details */

    /* Menu details */

    menuSystem[22].numOptions = 0;
    menuSystem[22].displayObject = 0;
    menuSystem[22].displayMenu = ShowTimeTrialScores;
    menuSystem[22].prevMenu = BEST_TIMES_MENU;

    // set what we want to call when option selected

    /*******************/
    /***** Menu 24 *****/
    /*******************/

    menuSystem[23].numOptions = 0;
    menuSystem[23].displayObject = 0;
    menuSystem[23].displayMenu = QuitYesNo;
    menuSystem[23].prevMenu = MAIN_MENU;

    /*******************/
    /***** Menu 25 *****/
    /*******************/

    menuSystem[24].numOptions = 0;
    menuSystem[24].displayObject = DrawGraphicDetail;
    menuSystem[24].displayMenu = GraphicOptions;
    menuSystem[24].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 26 *****/
    /*******************/

    menuSystem[25].numOptions = 0;
    menuSystem[25].displayObject = DrawControllerMenu;
    menuSystem[25].displayMenu = ControllerOptions;
    menuSystem[25].prevMenu = OPTION_MENU;

    /*******************/
    /***** Menu 27 *****/
    /*******************/

    menuSystem[26].numOptions = 0;
    menuSystem[26].displayObject = DrawControllerEdit;
    menuSystem[26].displayMenu = ControllerEdit;
    menuSystem[26].prevMenu = CONTROLLER_MENU;

    /*******************/
    /***** Menu 28 *****/
    /*******************/

    menuSystem[27].menuTitle = "MULTI PLAYER OPTIONS";

    /* Option details */

    menuSystem[27].menuOptions[0].optionText = "TWO PLAYER SERIAL LINK";

    menuSystem[27].menuOptions[1].optionText = "NETWORK";

    menuSystem[27].numOptions = 2;
    menuSystem[27].displayObject = 0;
    menuSystem[27].displayMenu = 0;
    menuSystem[27].prevMenu = MAIN_MENU;

    /*******************/
    /***** Menu 29 *****/
    /*******************/

    menuSystem[28].numOptions = 0;
    menuSystem[28].displayObject = 0;
    // menuSystem[28].displayMenu = NetworkOptions ;
    menuSystem[28].prevMenu = MULTI_MENU;

    /*******************/
    /***** Menu 30 *****/
    /*******************/

    menuSystem[29].numOptions = 0;
    menuSystem[29].displayObject = 0;
    // menuSystem[29].displayMenu = SerialOptions;
    menuSystem[29].prevMenu = MULTI_MENU;
}

int LoadTexModel(Object** modelShapes, Skeleton* skeletons, char* fileName)
{
    int modelTextures = 0;
    Object* obj;
    int count = 0;
    char modelFile[64];

    /******************************************************/
    /** Load compressed texture files, first to allow    **/
    /** temporary memory to be allocated                 **/
    /******************************************************/

    strcpy(modelFile, "wipeout\\common\\");
    strcat(modelFile, fileName);
    strcat(modelFile, ".cmp");
    modelTextures = TextureTableCount;
    LoadCompressedTextureSequence(modelFile);

    strcpy(modelFile, "wipeout\\common\\");
    strcat(modelFile, fileName);
    strcat(modelFile, ".prm");

    modelShapes[0] = (Object*)LoadPrm(modelFile, modelTextures);

    obj = modelShapes[0];

    while (obj)
    {
        obj->skeleton = skeletons + count;
        InitSkeleton(obj->skeleton);
        modelShapes[count] = obj;
        count++;
        obj = obj->next;
    }

    return count;
}

void DrawMainMenu(Menus* menuSystem, int* currMenu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    int heading = gFrameIndex * 5;

    andybodge = 1;

    if (shapeNo == 0)
        DrawTeamShips(0, 0, MODELSIZE, heading, menuSystem[(*currMenu)].menuModel[shipShapeOrder[0]]);

    // if (shapeNo == 1)
    // {
    //    DrawTeamShips (-450, 0, 1100, heading, menuSystem[(*currMenu)].menuModel[shipShapeOrder[3]]) ;
    //    DrawTeamShips (450, 0, 1100, heading, menuSystem[(*currMenu)].menuModel[shipShapeOrder[1]]) ;
    // }

    if (shapeNo == 1)
    {
        SetSkeletonPosition(menuSystem[(*currMenu)].menu2Model[3]->skeleton, 0, 0, MODELSIZE); /*1800*/
        SetSkeletonDirectionHPR(menuSystem[(*currMenu)].menu2Model[3]->skeleton, hdg, 0, 0);
        TransformStaticMenuObject(menuSystem[(*currMenu)].menu2Model[3]);
    }

    // if (shapeNo ==2)
    //{
    // SetSkeletonPosition(menuSystem[(*currMenu)].menu2Model[1]->skeleton,  0, 0, MODELSIZE) ;    /*1800*/
    // SetSkeletonDirectionHPR(menuSystem[(*currMenu)].menu2Model[1]->skeleton,hdg,0,0);
    // TransformStaticMenuObject(menuSystem[(*currMenu)].menu2Model[1]);
    // hdg += 40;
    //}//SJR-msdos
}

void DrawTeams(Menus* menuSystem, int* currMenu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    int heading = gFrameIndex * 10;

    SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[teamShapeOrder[shapeNo]]->skeleton), 0, 0, 9000);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[teamShapeOrder[shapeNo]]->skeleton), hdg, 0, 0);
    TransformStaticObject(menuSystem[(*currMenu)].menuModel[teamShapeOrder[shapeNo]]);

    DrawTeamShips(-900, 800, 1500, heading, menuSystem[(*currMenu)].menu2Model[shipShapeOrder[shapeNo * 2]]);
    DrawTeamShips(900, 800, 1500, heading, menuSystem[(*currMenu)].menu2Model[shipShapeOrder[shapeNo * 2 + 1]]);
}

void DrawTeamShips(int x, int y, int size, int heading, Object* shipModel)
{
    SetSkeletonPosition(shipModel->skeleton, x, y, size);
    SetSkeletonDirectionHPR(shipModel->skeleton, -heading, 0, 0);

    TransformStaticMenuObject(shipModel);
}

void DrawPilot(Menus* menuSystem, int* currMenu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    int shdg = gFrameIndex * 5;

    SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[pilotShapeOrder[shapeNo]]->skeleton), 0, -400, 11000);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[pilotShapeOrder[shapeNo]]->skeleton), hdg, 0, 0);
    TransformStaticObject(menuSystem[(*currMenu)].menuModel[pilotShapeOrder[shapeNo]]);

    DrawTeamShips(0, 700, 1300, shdg, menuSystem[(*currMenu)].menu2Model[shipShapeOrder[shapeNo]]);
}

void DrawPlayPad(Menus* menuSystem, int* currMenu, int shapeNo)
{
    InitMenuDisplay(120);

    SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[0]->skeleton), 0, -1140, 6220 + zoomInOut);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[0]->skeleton), (modelHdg), 0, 0);
    TransformStaticObject(menuSystem[(*currMenu)].menuModel[0]);
    SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[1]->skeleton), 0, 2260 + downUp, 6670);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[1]->skeleton), 250, -3440, 10);
    TransformStaticObject(menuSystem[(*currMenu)].menuModel[1]);
}

void DrawNegCon(Menus* menuSystem, int* currMenu, int shapeNo) { }

void NegConButtonDisplay(Menus* menuSystem, int* currMenu, int shapeNo) { }

void DrawShip(Menus* menuSystem, int* currMenu, int shipNo)
{
    int hdg = gFrameIndex * 5;

    SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[shipShapeOrder[shipNo]]->skeleton), 0, 0, MODELSIZE);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[shipShapeOrder[shipNo]]->skeleton), hdg, 0, 0);
    TransformStaticObject(menuSystem[(*currMenu)].menuModel[shipShapeOrder[shipNo]]);
}

void DrawClass(Menus* menuSystem, int* currMenu, int shipNo)
{
    int hdg = gFrameIndex * 20;

    if (shipNo == 0)
    {
        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[0]->skeleton), 0, 0, 500);
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[0]->skeleton), hdg, 0, 0);
        TransformStaticObject(menuSystem[(*currMenu)].menuModel[0]);
    }

    if (shipNo == 1)
    {
        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[1]->skeleton), 0, -25, 500);
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[1]->skeleton), hdg, 0, 0);
        TransformStaticObject(menuSystem[(*currMenu)].menuModel[1]);
    }
}

void InitSelectData(SelectionData* selectData)
{
    selectData->slotRow = 0;
    selectData->optionNo = 0;
    selectData->slotColumn = 0;
    selectData->slotsFilled = 0;
    selectData->select = 0;
    selectData->firstTime = 1;
    selectData->numCardFiles = 0;
    selectData->hiscoreTrack = 0;
    selectData->speechMode = 0;
    selectData->toggle = 0;
    selectData->cardNum = 0;
    selectData->card1 = FORMATTED;
    selectData->card2 = FORMATTED;
    selectData->formatCard = 0;
    selectData->formatConfirm = 0;
}

void InitMusicVSfx(SlideTextures* musicTex)
{
    musicTex->slideTex = LoadTexture("wipeout\\textures\\slider1.tim"); /*slide*/
    musicTex->sliderTex = LoadTexture("wipeout\\textures\\slide1.tim");
}

void MusicVSfx(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
/*TODO audio settings
    int slidePosx = 48;
    int slidePosy = 85;
    int slideWidth = 224;
    int slideHeight = 28;
    int sliderLeftOffset = 0;
    int sliderRightOffset = 24;

    extern short CurrTrkNo;
*/
    int sliderRange = 218;

    static int ShieldOn = 0;

    if (xpad & IN_UP)
    {
        PlayNote(MENU_MOVE, 1, 2);
        selectData->optionNo--;
        if (selectData->optionNo < 0)
            selectData->optionNo = 1;
    }

    if (xpad & IN_DOWN)
    {
        PlayNote(MENU_MOVE, 1, 2);
        selectData->optionNo++;
        if (selectData->optionNo > 1)
            selectData->optionNo = 0;
    }

    AddText("AUDIO CONFIGURATION", text_data(60, 20, 8), RedText);

    {
        DisplayButtons(30, 210, PSELECT);
        DisplayButtons(240, 210, PBACK);
    }
    /*TODO audio settings*/
#if 0
   /* Slider overlay knob */
   SetPolyFT4(&(musicPrims->sliderPrim));
   musicPrims->sliderPrim.r0=230; // PCwipeout
   musicPrims->sliderPrim.g0=0;
   musicPrims->sliderPrim.b0=BLACK_NOT_DRAWN;

   setXY4(&(musicPrims->sliderPrim), (slidePosx-9) + sliderLeftOffset + gameData->oldSliderPos, slidePosy - 24,
      (slidePosx-9) + sliderRightOffset + gameData->oldSliderPos, slidePosy - 24,
      (slidePosx-9) + sliderLeftOffset + gameData->oldSliderPos, slidePosy + 52,
      (slidePosx-9) + sliderRightOffset + gameData->oldSliderPos, slidePosy + 52) ;

   setUV4(&(musicPrims->sliderPrim), musicTex->sliderTex->u0, musicTex->sliderTex->v0,
      musicTex->sliderTex->u1+1, musicTex->sliderTex->v1,
      musicTex->sliderTex->u2, musicTex->sliderTex->v2+1,
      musicTex->sliderTex->u3+1, musicTex->sliderTex->v3+1) ;

   setTPage(&(musicPrims->sliderPrim),0,0,musicTex->sliderTex->textureX,musicTex->sliderTex->textureY);

   musicPrims->sliderPrim.clut = musicTex->sliderTex->cba;

   AddPrim( OT[ CurrentScreen ] , ( ulong* ) &(musicPrims->sliderPrim) );

   /* Slider background */
   SetPolyFT4(&(musicPrims->slidePrim));
   musicPrims->slidePrim.r0=230; // PCwipeout
   musicPrims->slidePrim.g0=0;
   musicPrims->slidePrim.b0=BLACK_NOT_DRAWN;

   setXY4(&(musicPrims->slidePrim), slidePosx, slidePosy,
      slidePosx + slideWidth, slidePosy,
      slidePosx, slidePosy + slideHeight,
      slidePosx + slideWidth, slidePosy + slideHeight) ;

   setUV4(&(musicPrims->slidePrim), musicTex->slideTex->u0, musicTex->slideTex->v0,
      musicTex->slideTex->u1, musicTex->slideTex->v1,
      musicTex->slideTex->u2, musicTex->slideTex->v2+1,
      musicTex->slideTex->u3, musicTex->slideTex->v3+1) ;

   setTPage(&(musicPrims->slidePrim),0,0,musicTex->slideTex->textureX,musicTex->slideTex->textureY);

   musicPrims->slidePrim.clut = musicTex->slideTex->cba;

   AddPrim( OT[ CurrentScreen ] , ( ulong* ) &(musicPrims->slidePrim) );
#endif
    AddText(GetCdTrackName(gameData->oldCdTrack - 1), text_data(130, 175, 8), RedText);

    if (selectData->optionNo == 0)
    {
        AddText("SFX", text_data(33, 51, 8), RedText);
        AddText("MUSIC", text_data(250, 139, 8), RedText);

        if (pad & IN_LEFT)
        {
            if (!ShieldOn)
            {
                PlayNote(SHIELDS, 0, -1);
                ShieldOn = 1;
            }
            /*TODO audio
            chanTable[vagTable[SHIELDS].channelID].s_attr.volume.left = 0x3fff - (75 * gameData->oldSliderPos);
            chanTable[vagTable[SHIELDS].channelID].s_attr.volume.right = 0x3fff - (75 * gameData->oldSliderPos);
            */
            gameData->oldSliderPos--;
            if (gameData->oldSliderPos < 0)
                gameData->oldSliderPos = 0;
            SfxSetVolume((char)((0x3fff - (75 * gameData->oldSliderPos)) >> 6));
            globalvol = 218 - gameData->oldSliderPos;
            CdSetVol(gameData->oldSliderPos);
            SfxSetVolume((char)((0x3fff - (75 * gameData->oldSliderPos)) >> 6));
        }
        else if (pad & IN_RIGHT)
        {
            if (!ShieldOn)
            {
                PlayNote(SHIELDS, 0, -1);
                ShieldOn = 1;
            }
            /*TODO audio*/
            // chanTable[vagTable[SHIELDS].channelID].s_attr.volume.left = 0x3fff - (75 * gameData->oldSliderPos);
            // chanTable[vagTable[SHIELDS].channelID].s_attr.volume.right = 0x3fff - (75 * gameData->oldSliderPos);
            gameData->oldSliderPos++;
            if (gameData->oldSliderPos > sliderRange)
                gameData->oldSliderPos = sliderRange;
            globalvol = 218 - gameData->oldSliderPos;
            CdSetVol(gameData->oldSliderPos);
            SfxSetVolume((char)((0x3fff - (75 * gameData->oldSliderPos)) >> 6));
#ifdef PORT_SOUND
            SfxSetVolume((char)((0x3fff - (75 * gameData->oldSliderPos)) >> 6));
#endif
        }
        else
        {
            if (ShieldOn)
            {
                KillNote(SHIELDS);
                ShieldOn = 0;
            }
            /*TODO audio
            chanTable[vagTable[SHIELDS].channelID].s_attr.volume.left = 0;
            chanTable[vagTable[SHIELDS].channelID].s_attr.volume.right = 0;
            */
        }

        globalvol = 218 - gameData->oldSliderPos;
        CdSetVol(gameData->oldSliderPos);
        /*TODO audio
        c_attr.cd.volume.left  = 150 * gameData->oldSliderPos;   // Master volume (left)
        c_attr.cd.volume.right  = 150 * gameData->oldSliderPos;  // Master volume (right)
        */
    }
    else
    {
        AddText("SFX", text_data(33, 51, 8), WhiteText);
        AddText("MUSIC", text_data(250, 139, 8), WhiteText);
    }

    if (selectData->optionNo == 1)
    {
        AddText("SELECT CD", text_data(15, 175, 8), RedText);
        if (ShieldOn)
        {
            KillNote(SHIELDS);
            ShieldOn = 0;
        }
        if (xpad & IN_LEFT)
        {
            PlayNote(MENU_MOVE, 1, 2);
            /*TODO audio
            if (gameData->oldCdTrack == starttrack)
                gameData->oldCdTrack = 1;
            else
                gameData->oldCdTrack--;

            if (gameData->oldCdTrack < 1)
                gameData->oldCdTrack = endtrack;

            CdControlB(CdlStop, 0, 0);

            if(gameData->oldCdTrack != 1)
            {
               //          CdControlB(CdlSetloc, (u_char *)&(loc[gameData->oldCdTrack]), 0);
               CurrTrkNo = gameData->oldCdTrack;
               CdControlB(CdlPlay, 0, 0);
            }
            else
            {
               CurrTrkNo = (randy() % (endtrack - 1)) + 2;
               CdControlB(CdlPlay, 0, 0);
            }
            */
        }

        if (xpad & IN_RIGHT)
        {
            PlayNote(MENU_MOVE, 1, 2);
            /*TODO audio
            if (gameData->oldCdTrack == 1)
                gameData->oldCdTrack = starttrack;
            else
                gameData->oldCdTrack++;

            if (gameData->oldCdTrack > endtrack)
                gameData->oldCdTrack = 1;


            CdControlB(CdlStop, 0, 0);

            if(gameData->oldCdTrack != 1)
            {
               //          CdControlB(CdlSetloc, (u_char *)&(loc[gameData->oldCdTrack]), 0);
               CurrTrkNo = gameData->oldCdTrack;
               CdControlB(CdlPlay, 0, 0);
            }
            else
            {
               CurrTrkNo = (randy() % (endtrack - 1)) + 2;
               CdControlB(CdlPlay, 0, 0);
            }
            */
        }
    }
    else
    {
        AddText("SELECT CD", text_data(15, 175, 8), WhiteText);
    }
}

void InitLoadSave(char loadGames[][9])
{
    short i;

    for (i = 0; i < MAX_GAMES; i++)
        loadGames[i][0] = 0;
}

void EnterLoadGame(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    //unsigned short KeyCode = 0;
    // extern signed char      etc_KeyPressed;
    // extern unsigned char etc_Key;
    // extern unsigned long etc_KeyboardMap[256];

    CentreText("LOAD GAME FROM DISK", text_data(70, 20, 8), RedText);
    AddText("ENTER NAME OF FILE", text_data(65, 60, 8), WhiteText);
    /*TODO wtf text input?
       if(TextEntry)
       {
          KeyCode = GetKeyNoBlock();

          if((KeyCode > 0x40 && KeyCode < 0x5b))//A - Z
             // ||       // Q - P

             //  (KeyCode > 29 && KeyCode < 39) ||     // A - L

             //(KeyCode > 43 && KeyCode < 51))         // Z - M
          {
             LoadFilename[TextPos] = ScanToAscii[KeyCode];

             if(TextPos < (MAXSTRING - 2))
                TextPos++;

             LoadFilename[TextPos] = 101;     // Cursor
          }

          if((KeyCode == 0x08) && (TextPos > 0))    // Backspace
          {
             LoadFilename[TextPos] = ' ';
             LoadFilename[--TextPos] = 101;
          }
       }
       else
       {
          if(davebodge2 == 0)
          {
             strcpy(LoadFilename, "         ");
             LoadFilename[0] = 101;
             TextEntry = 1;
             TextPos = 0;
          }
       }
    */
    AddText(LoadFilename, text_data(40, 80, 16), WhiteText);
}

void EnterSaveGame(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    //unsigned short KeyCode = 0;
    // extern signed char      etc_KeyPressed;
    // extern unsigned char etc_Key;
    // extern unsigned long etc_KeyboardMap[256];

    CentreText("SAVE GAME TO DISK", text_data(70, 20, 8), RedText);
    AddText("ENTER NAME OF FILE", text_data(65, 60, 8), WhiteText);
    /*TODO wtf text input
       if(TextEntry)
       {
          KeyCode = GetKeyNoBlock();

          if((KeyCode > 0x40 && KeyCode < 0x5b))//A - Z
             //if((KeyCode > 15 && KeyCode < 26) ||       // Q - P
             //(KeyCode > 29 && KeyCode < 39) ||       // A - L
             //(KeyCode > 43 && KeyCode < 51))         // Z - M
          {
             SaveFilename[TextPos] = ScanToAscii[KeyCode];

             if(TextPos < (MAXSTRING - 2))
                TextPos++;

             SaveFilename[TextPos] = 101;     // Cursor
          }

          if((KeyCode == 0x08) && (TextPos > 0))    // Backspace
          {
             SaveFilename[TextPos] = ' ';
             SaveFilename[--TextPos] = 101;
          }
       }
       else
       {
          if(davebodge2 == 0)
          {
             strcpy(SaveFilename, "         ");
             SaveFilename[0] = 101;
             TextEntry = 1;
             TextPos = 0;
          }
       }

       AddText (SaveFilename, text_data (40, 80, 16), WhiteText) ;
    */
}

void MenuErrors(int* currMenu, Menus* menuSystem)
{
    int i;

    if (menuSystem[*currMenu].updateCount > 0 && menuSystem[*currMenu].errorFlag != 0)
    {
        AddText("ERROR", text_data(105, 20, 16), RedText);

        switch (menuSystem[*currMenu].errorFlag)
        {
            case CARD_FULL:
                AddText("CARD IS FULL", text_data(90, 112, 12), RedText);
                break;

            case DUP_NAME:
                AddText("DUPLICATE NAME", text_data(50, 112, 12), RedText);
                AddText("PLEASE ENTER NEW NAME", text_data(40, 142, 8), RedText);
                break;

            case LOAD_ERROR:
                AddText("ERROR DURING LOAD", text_data(25, 112, 12), RedText);
                break;

            case SAVE_ERROR:
                AddText("ERROR DURING SAVE", text_data(30, 112, 12), RedText);
                break;

            case RENAME_ERROR:
                AddText("ERROR DURING RENAME", text_data(10, 112, 12), RedText);
                break;

            case NO_CARD:
                CentreText("NO CARD PRESENT", text_data(10, 112, 12), RedText);
                break;

            case NO_FILES:
                CentreText("NO FILES PRESENT", text_data(10, 112, 12), RedText);
                break;

            case NO_FORMAT:
                CentreText("CARD NOT FORMATTED", text_data(10, 112, 12), RedText);
                break;

            case FORMAT_ERROR:
                CentreText("ERROR DURING FORMAT", text_data(10, 112, 12), RedText);
                break;

                // unknown errors
            default:
                CentreText("ERROR CONNECTING", text_data(10, 112, 8), RedText);
                break;
        }

        menuSystem[*currMenu].updateCount--;
    }
    else
    {
        for (i = 0; i < MAX_MENUS; i++)
        {
            menuSystem[i].errorFlag = 0;
        }
    }
}

void ShowHiScores(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    short actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(MENU_MOVE, 1, 20);
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
        PlayNote(MENU_MOVE, 1, 20);
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

    CentreText("BEST TIMES", text_data(1, 21, 12), RedText);

    actualTrack = selectData->hiscoreTrack;

    if (selectData->hiscoreTrack >= NumTracks)
    {
        if (gameData->showBonusTrack == 0)
            actualTrack -= (NumTracks + 1);
        else
            actualTrack -= NumTracks;
        CentreText("RAPIER", text_data(1, 40, 8), RedText);
    }
    else
    {
        CentreText("VENOM", text_data(1, 40, 8), RedText);
    }

    DisplayButtons(240, 210, PBACK);

    if (((gameData->RapierClass == 0) && (selectData->hiscoreTrack == (NumTracks - 1))) || ((gameData->RapierClass == 1) && (selectData->hiscoreTrack == (totalTracks - 1))))
        DisplayButtons(290, 116, ARROWR);
    else
    {
        if (BLINK_SLOW)
            DisplayButtons(290, 116, ARROWR);
    }

    if (selectData->hiscoreTrack == 0)
        DisplayButtons(20, 116, ARROWL);
    else
    {
        if (BLINK_SLOW)
            DisplayButtons(20, 116, ARROWL);
    }

    CentreText(selectData->menuSystem[TRACK_MENU].menuOptions[actualTrack].optionText, text_data(0, 127 - 70, 12), RedText);

    if (BLINK_SLOW)
    {
        AddText("LAP RECORD", text_data(40, 195, 8), WhiteText);
        AddTime(scoreTable[selectData->hiscoreTrack].lapRecord, text_data(165, 188, 16), RedText);
    }

    DisplayHiScore(selectData->hiscoreTrack);
}

void ShowTimeTrialScores(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    short actualTrack = 0;

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(MENU_MOVE, 1, 20);
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
        PlayNote(MENU_MOVE, 1, 20);
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

    CentreText("BEST TIMES", text_data(1, 21, 12), RedText);

    actualTrack = (selectData->hiscoreTrack - HiScoreTracks);

    if ((selectData->hiscoreTrack - HiScoreTracks) >= (NumTracks))
    {
        if (gameData->showBonusTrack == 0)
            actualTrack -= ((NumTracks + 1));
        else
            actualTrack -= (NumTracks);
        CentreText("RAPIER", text_data(1, 40, 8), RedText);
    }
    else
    {
        CentreText("VENOM", text_data(1, 40, 8), RedText);
    }

    DisplayButtons(240, 210, PBACK);

    if (((gameData->RapierClass == 0) && ((selectData->hiscoreTrack - HiScoreTracks) == (NumTracks - 1))) || ((gameData->RapierClass == 1) && ((selectData->hiscoreTrack - HiScoreTracks) == (totalTracks - 1))))
        DisplayButtons(290, 116, ARROWR);
    else
    {
        if (BLINK_SLOW)
            DisplayButtons(290, 116, ARROWR);
    }

    if ((selectData->hiscoreTrack - HiScoreTracks) == 0)
        DisplayButtons(20, 116, ARROWL);
    else
    {
        if (BLINK_SLOW)
            DisplayButtons(20, 116, ARROWL);
    }

    CentreText(selectData->menuSystem[TRACK_MENU].menuOptions[actualTrack].optionText, text_data(0, 127 - 70, 12), RedText);

    if (BLINK_SLOW)
    {
        AddText("LAP RECORD", text_data(40, 195, 8), WhiteText);
        AddTime(scoreTable[selectData->hiscoreTrack].lapRecord, text_data(165, 188, 16), RedText);
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
    gameData->Position = 8;
    gameData->RapierClass = 0;
    gameData->cdTrack = 1;
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

void ConfirmSaveGame(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    DisplayButtons(30, 210, PSELECT);
    DisplayButtons(240, 210, PBACK);

    CentreText("OVERWRITE EXISTING FILE", text_data(70, 50, 8), RedText);
    if (selectData->saveType == NORM)
        CentreText("PERFORM SAVE", text_data(55, 60, 8), WhiteText);

    if ((selectData->saveType == OVERWRITE) || (selectData->saveType == RENAME))
        CentreText("OVERWRITE EXISTING FILE", text_data(55, 60, 8), WhiteText);

    AddText(selectData->nameEntered, text_data(140, 74, 8), RedText);

    if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
    {
        selectData->confirm ^= 1;
        PlayNote(MENU_MOVE, 1, 20);
    }

    AddText("YES", text_data(80, 120, 16), selectData->confirm ? WhiteText : BLINK_TEXT);
    AddText("NO", text_data(210, 120, 16), selectData->confirm ? BLINK_TEXT : WhiteText);
}

/*TODO lconfirm? xpad?*/
void ContinueGameConfirm(char* confirm, short xpad, char* lConfirm)
{
    CentreText("CONTINUE QUALIFYING OR QUIT", text_data(77, 95, 8), WhiteText);

    AddText("QUIT", text_data(215, 120, 16), (*lConfirm) ? BLINK_TEXT : WhiteText);
    AddText("QUALIFY", text_data(45, 120, 16), (*lConfirm) ? WhiteText : BLINK_TEXT);
}

void RestartGameConfirm(char* confirm, short xpad, char* lConfirm)
{
    CentreText("RESTART RACE", text_data(77, 95, 16), WhiteText);

    AddText("NO", text_data(215, 120, 16), (*lConfirm) ? BLINK_TEXT : WhiteText);
    AddText("YES", text_data(68, 120, 16), (*lConfirm) ? WhiteText : BLINK_TEXT);
}

void SetUpPilots(int teamNum, Menus* menuSystem)
{

    if (teamNum == AG_SYSTEMS)
    {
        menuSystem[7].menuOptions[0].optionText = "JOHN DEKKA";
        menuSystem[7].menuOptions[1].optionText = "DANIEL CHANG";
    }

    if (teamNum == AURICOM)
    {
        menuSystem[7].menuOptions[0].optionText = "ARIAL TETSUO";
        menuSystem[7].menuOptions[1].optionText = "ANASTASIA CHEROVOSKI";
    }

    if (teamNum == QIREX)
    {
        menuSystem[7].menuOptions[0].optionText = "KEL SOLAAR";
        menuSystem[7].menuOptions[1].optionText = "ARIAN TETSUO";
    }

    if (teamNum == FEISAR)
    {
        menuSystem[7].menuOptions[0].optionText = "SOFIA DE LA RENTE";
        menuSystem[7].menuOptions[1].optionText = "PAUL JACKSON";
    }
}

void MenuPlayPad(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    static int delay = 0;
    static char key = 0;
    char temp = 0;

    DisplayButtons(30, 210, PSELECT);
    DisplayButtons(240, 210, PBACK);

    if (BLINK_SLOW)
    {
        DisplayButtons(290, 116, ARROWR);
        DisplayButtons(20, 116, ARROWL);
    }

    CentreText("JOYPAD CONFIGURATION", text_data(1, 21, 8), RedText);

    temp = 0;

    if (delay == 0)
    {
        if ((GetMenuKey(M_RIGHT)))
        {
            key = 1;
            temp = 1;
            delay = 32;
            PlayNote(MENU_MOVE, 1, 20);
            gameData->oldPadPreset++;
            if (gameData->oldPadPreset == NORMPAD_PRESETS)
                gameData->oldPadPreset = 0;
        }

        if ((GetMenuKey(M_LEFT)))
        {
            key = 2;
            temp = 1;
            delay = 32;
            PlayNote(MENU_MOVE, 1, 20);
            gameData->oldPadPreset--;
            if (gameData->oldPadPreset < 0)
                gameData->oldPadPreset = (NORMPAD_PRESETS - 1);
        }

        if (temp == 0)
        {
            DisplayButtons(17, 153, LBRAKE); /* Left */
            DisplayButtons(237, 153, RBRAKE); /* Right */
            /*TODO show controls*/
#if 0
         switch (gameData->oldPadPreset)
         {
         case 0:  CentreText ("PRESET A", text_data (1,210, 8), RedText) ;

            DrawLine (204, 63, 245, 39, 0) ; /* Triangle */
            DrawLine (216, 76, 251, 68, 1) ; /* Circle */
            DrawLine (204, 87, 230, 97, 3) ; /* Cross */
            DrawLine (210, 153, 252, 153, 6) ; /* Right Top */
            DrawLine (130, 167, 81, 163, 4) ; /* Left Top */

            DisplayButtons ( 248, 39, VIEW) ; /* Triangle */
            DisplayButtons ( 254, 68, PADFIRE) ;  /* Circle */
            DisplayButtons ( 234, 97, THRUST) ; /* Cross */

            break ;

         case 1:  CentreText ("PRESET B", text_data (1,210, 8), RedText) ;

            DrawLine (204, 63, 245, 39, 0) ; /* Triangle */
            DrawLine (204, 87, 230, 97, 3) ; /* Cross */
            DrawLine (192, 75, 132, 113, 2) ; /* Square */
            DrawLine (210, 153, 252, 153, 6) ; /* Right Top */
            DrawLine (130, 167, 81, 163, 4) ; /* Left Top */

            DisplayButtons ( 132, 117, PADFIRE) ; /* Square */
            DisplayButtons ( 234, 97, THRUST) ; /* Cross */
            DisplayButtons ( 248, 39, VIEW) ; /* Triangle */

            break ;

         case 2:  CentreText ("PRESET C", text_data (1,210, 8), RedText) ;

            DrawLine (204, 87, 230, 97, 3) ; /* Cross */
            DrawLine (210, 153, 252, 153, 6) ; /* Right Top */
            DrawLine (130, 167, 81, 163, 4) ; /* Left Top */
            DrawLine (130, 183, 81, 185, 5) ; /* Left Bottom */
            DrawLine (214, 168, 252, 185, 7) ; /* Right Bottom */

            DisplayButtons ( 234, 97, THRUST) ; /* Cross */
            DisplayButtons ( 22, 185, VIEW) ;  /* Circle */
            DisplayButtons ( 252, 185, PADFIRE) ; /* Cross */
            break ;

         case 3:  CentreText ("PRESET D", text_data (1,210, 8), RedText) ;

            DrawLine (216, 76, 251, 68, 1) ; /* Circle */
            DrawLine (192, 75, 132, 113, 2) ; /* Square */
            DrawLine (204, 87, 230, 97, 3) ; /* Cross */
            DrawLine (210, 153, 252, 153, 6) ; /* Right Top */
            DrawLine (130, 167, 81, 163, 4) ; /* Left Top */

            DisplayButtons ( 254, 68, VIEW) ;  /* Circle */
            DisplayButtons ( 132, 117, THRUST) ; /* Square */
            DisplayButtons ( 234, 97, PADFIRE) ; /* Cross */

            break ;

         case 4:  CentreText ("PRESET E", text_data (1,210, 8), RedText) ;

            DrawLine (216, 76, 251, 68, 1) ; /* Circle */
            DrawLine (204, 63, 245, 39, 0) ; /* Triangle */
            DrawLine (204, 87, 230, 97, 3) ; /* Cross */
            DrawLine (126, 183, 81, 163, 5) ; /* Left Bottom */
            DrawLine (214, 168, 252, 153, 7) ; /* Right Bottom */

            DisplayButtons ( 248, 39, VIEW) ; /* Triangle */
            DisplayButtons ( 254, 68, PADFIRE) ;  /* Circle */
            DisplayButtons ( 234, 97, THRUST) ; /* Cross */

            break ;

         case 5:  CentreText ("PRESET F", text_data (1,210, 8), RedText) ;

            DrawLine (204, 63, 245, 39, musicPrims->lines, 0) ; /* Triangle */
            DrawLine (204, 87, 230, 97, musicPrims->lines, 3) ; /* Cross */
            DrawLine (192, 75, 132, 113, musicPrims->lines, 2) ; /* Square */
            DrawLine (126, 183, 81, 163, musicPrims->lines, 5) ; /* Left Bottom */
            DrawLine (214, 168, 252, 153, musicPrims->lines, 7) ; /* Right Bottom */

            DisplayButtons ( 132, 117, PADFIRE) ; /* Square */
            DisplayButtons ( 234, 97, THRUST) ; /* Cross */
            DisplayButtons ( 248, 39, VIEW) ; /* Triangle */

            break ;

         case 6:  CentreText ("PRESET G", text_data (1,210, 8), RedText) ;

            DrawLine (204, 63, 245, 39, musicPrims->lines, 0) ; /* Triangle */
            DrawLine (216, 76, 251, 68, musicPrims->lines, 1) ; /* Circle */
            DrawLine (192, 75, 132, 113, musicPrims->lines, 2) ; /* Square */
            DrawLine (130, 167, 81, 163, musicPrims->lines, 4) ; /* Left Top */
            DrawLine (210, 153, 252, 153, musicPrims->lines, 6) ; /* Right Top */
            DisplayButtons ( 248, 39, THRUST) ; /* Triangle */
            DisplayButtons ( 254, 68, PADFIRE) ;  /* Circle */
            DisplayButtons ( 132, 117, VIEW) ; /* Square */

            break ;

         case 7:  CentreText ("PRESET H", text_data (1,210, 8), RedText) ;

            DrawLine (204, 63, 245, 39, musicPrims->lines, 0) ; /* Triangle */
            DrawLine (216, 76, 251, 68, musicPrims->lines, 1) ; /* Circle */
            DrawLine (192, 75, 132, 113, musicPrims->lines, 2) ; /* Square */
            DrawLine (130, 167, 81, 163, musicPrims->lines, 4) ; /* Left Top */
            DrawLine (210, 153, 252, 153, musicPrims->lines, 6) ; /* Right Top */
            DisplayButtons ( 248, 39, PADFIRE) ; /* Triangle */
            DisplayButtons ( 254, 68, THRUST) ;  /* Circle */
            DisplayButtons ( 132, 117, VIEW) ; /* Square */

            break ;
         }
#endif
        }
    }
    else
    {
        delay--;
        if (key == 1)
            modelHdg += 256;

        if (key == 2)
            modelHdg -= 256;

        if (delay > 15)
        {
            zoomInOut += 500;
            downUp += 500;
        }

        if (delay <= 15)
        {
            zoomInOut -= 500;
            downUp -= 500;
        }
    }
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

void DrawGames(Menus* menuSystem, int* currMenu, int shipNo)
{
    int hdg = gFrameIndex * 20;

    InitMenuDisplay(90);

    // jed

    if (shipNo == 0)
    {
        // jed

        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[0]->skeleton), 0, -50, 600); /*1800*/
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[0]->skeleton), hdg, 0, 0);
        TransformStaticObject(menuSystem[(*currMenu)].menuModel[0]);
    }

    if (shipNo == 1)
    {
        // jed

        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[2]->skeleton), 0, -50, 600); /*1800*/
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[2]->skeleton), hdg, 0, 0);
        TransformStaticObject(menuSystem[(*currMenu)].menuModel[2]);
    }

    if (shipNo == 2)
    {
        // jed

        SetSkeletonPosition((menuSystem[(*currMenu)].menu2Model[2]->skeleton), 0, -50, 600); /*1800*/
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menu2Model[2]->skeleton), hdg, 0, 0);
        TransformStaticMenuObject(menuSystem[(*currMenu)].menu2Model[2]);
    }
}

void DrawOptionMenus(Menus* menuSystem, int* currMenu, int shipNo)
{
    int hdg = gFrameIndex * 20;
    int heading = gFrameIndex * 5;

    andybodge = 1; // pc wipeout - horrid but i don't care !!! (used in detail & controller configs)

    InitMenuDisplay(90);

    if (shipNo == 0)
    {
        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[1]->skeleton), 0, 0, 400); /*1800*/
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[1]->skeleton), hdg, 0, 0);
        TransformStaticObject(menuSystem[(*currMenu)].menuModel[1]);
    }

    if (shipNo == 1)
    {
        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[0]->skeleton), 0, 0, 400); /*1800*/
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[0]->skeleton), hdg, 0, 0);
        TransformStaticObject(menuSystem[(*currMenu)].menuModel[0]);
    }

    if (shipNo == 2)
    {
        /*TODO input menu
        switch(controller)
        {
        case _KEYBOARD:
           DisplayPadMenu1(menuSystem,currMenu,shipNo);
           break;
        case _MOUSE1:
           DisplayPadMenu(menuSystem,currMenu,shipNo);
           break;
        case _JOYSTICK1:
           DisplayPadMenu2(menuSystem,currMenu,shipNo);
           break;
        }
        */
    }

    if (shipNo == 3) // Graphics Detail
    {
        DrawTeamShips(0, 0, MODELSIZE, heading, menuSystem[DETAILS_MENU].menuModel[shipShapeOrder[0]]);
        //    SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[2]->skeleton),0, 0, 600) ;    /*1800*/
        //    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[2]->skeleton), hdg,0, 0);
        //    TransformStaticObject(menuSystem[(*currMenu)].menuModel[2]);
    }

    // silly me - I thought shipNo meant it was drawing ships...
    // it was drawing headphones all the time. SJR

    /*if (shipNo == 4)
    {
    // jed
    Reallyinmenu = 0;
    inmenu = 1;
    SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[3]->skeleton),0, 0, 600) ;    //1800
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[3]->skeleton),  hdg,0,0);
       TransformStaticMenuObject(menuSystem[(*currMenu)].menuModel[3]);
 }*/

    // now i'tll not draw the headphones and draw the stopwatch instead.
    if (shipNo == 4)
    {
        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[2]->skeleton), 0, 0, 600); /*1800*/
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[2]->skeleton), hdg, 0, 0);
        TransformStaticMenuObject(menuSystem[(*currMenu)].menuModel[2]);
    }
}

void DrawHiScoreMenu(Menus* menuSystem, int* currMenu, int shipNo)
{
    int hdg = gFrameIndex * 20;

    InitMenuDisplay(90);

    if ((shipNo == 0) || (shipNo == 1))
    {
        SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[2]->skeleton), 0, 0, 600); /*1800*/
        SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[2]->skeleton), hdg, 0, 0);
        TransformStaticObject(menuSystem[(*currMenu)].menuModel[2]);
    }
}

void DisplayPadMenu(Menus* menuSystem, int* currMenu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    // jed
    SetSkeletonPosition((menuSystem[(*currMenu)].menu2Model[0]->skeleton), 0, 0, 6500);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menu2Model[0]->skeleton), hdg, 0, 0);
    TransformStaticObject(menuSystem[(*currMenu)].menu2Model[0]);
}

void DisplayPadMenu1(Menus* menuSystem, int* currMenu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    // jed
    SetSkeletonPosition((menuSystem[(*currMenu)].menu2Model[1]->skeleton), 0, -5, 4500);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menu2Model[1]->skeleton), hdg, 0, 0);
    TransformStaticMenuObject(menuSystem[(*currMenu)].menu2Model[1]);
}

void DisplayPadMenu2(Menus* menuSystem, int* currMenu, int shapeNo)
{
    int hdg = gFrameIndex * 20;
    // jed
    SetSkeletonPosition((menuSystem[(*currMenu)].menu2Model[2]->skeleton), 0, 0, 4500);
    SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menu2Model[2]->skeleton), hdg, 0, 0);
    TransformStaticObject(menuSystem[(*currMenu)].menu2Model[2]);
}

void DisplayNegConMenu(Menus* menuSystem, int* currMenu, int shapeNo) { }

/*TODO remove memcard shit*/
void ConfirmFormat(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    DisplayButtons(30, 210, PSELECT);
    DisplayButtons(240, 210, PBACK);

    CentreText("FORMAT DISK", text_data(70, 20, 8), RedText);
    CentreText("DO YOU WISH TO FORMAT CARD", text_data(55, 60, 8), WhiteText);
    if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
    {
        selectData->formatConfirm ^= 1;
        PlayNote(MENU_MOVE, 1, 20);
    }

    AddText("YES", text_data(80, 120, 16), selectData->formatConfirm ? BLINK_TEXT : WhiteText);
    AddText("NO", text_data(210, 120, 16), selectData->formatConfirm ? WhiteText : BLINK_TEXT);
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

void CheckSaveType(SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    LoadCardFiles(selectData, &(loadGames[0]), selectData->cardNum);

    if (((selectData->slotRow * 2) + (selectData->slotColumn + 1)) == (selectData->slotsFilled + 1))
    {
        if (selectData->nameEntered[0] != 0)
        {
            selectData->saveType = NORM;
        }
    }
    else
    {
        if (selectData->nameEntered[0] != 0)
        {
            if (strcmp(selectData->nameEntered, loadGames[(selectData->slotRow * 2) + selectData->slotColumn]) == 0)
            {
                selectData->saveType = OVERWRITE;
            }
            else
            {

                selectData->saveType = RENAME;
            }
        }
    }
}

void InitButtons(char* fileName)
{
    int tablePos;
    char modelFile[64];
    int i;

    strcpy(modelFile, "wipeout\\textures\\");
    strcat(modelFile, fileName);
    strcat(modelFile, ".cmp");

    tablePos = TextureTableCount;

    LoadCompressedTextureSequence(modelFile);

    for (i = 0; i < 18; i++)
    {
        buttonTex[i] = TextureTable[tablePos];
        tablePos++;
    }
}

void DisplayButtons(int x, int y, int buttType)
{
    const Texture* tex = buttonTex[buttType];
    AddGlyph(x, y, tex->width, tex->height, 0, 0, tex);
}

void DrawLine(int x, int y, int x1, int y1, int lineNum)
{
    /*TODO draw line
    SetPolyF4(&(line[lineNum]));
    line[lineNum].r0=230 ;
    line[lineNum].g0=0 ;
    line[lineNum].b0=BLACK_NOT_DRAWN ;

    setXY4(&(line[lineNum]),x, y, x+2, y,
       x1, y1, x1+2,y1+2 ) ;

    AddPrim( OT[ CurrentScreen ] , ( ulong* ) &(line[lineNum]) );
    */
}

void QuitYesNo(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    if (andybodge)
    {
        selectData->confirm = 1;
        andybodge = 0;
    }

    DisplayButtons(30, 210, PSELECT);
    DisplayButtons(240, 210, PBACK);

    CentreText("ARE YOU SURE YOU", text_data(77, 95, 8), RedText);
    CentreText("WANT TO QUIT", text_data(105, 105, 8), RedText);

    if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
    {
        selectData->confirm ^= 1;
    }

    AddText("YES", text_data(80, 120, 16), selectData->confirm ? WhiteText : BLINK_TEXT);
    AddText("NO", text_data(210, 120, 16), selectData->confirm ? BLINK_TEXT : WhiteText);
}

void GraphicOptions(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    static char listpos = 0;
    char x = 50, y = 140, l = 12;
    char DrawStr[8];
    char ResStr[8];
    char SkyStr[8];
    char TexStr[8];

    if (andybodge)
    {
        andybodge = 0;
    }

    CentreText("GRAPHIC DETAIL OPTIONS", text_data(1, 21, 8), RedText);

    DisplayButtons(30, 210, PSELECT);
    DisplayButtons(240, 210, PBACK);

    if (GetMenuKey(M_DOWN))
    {
        if (listpos == 3)
            listpos = 0;
        else
            listpos++;
    }

    if (GetMenuKey(M_UP))
    {
        if (listpos == 0)
            listpos = 3;
        else
            listpos--;
    }

    if (GetMenuKey(M_LEFT))
    {
        PlayNote(MENU_SELECT, 1, 20);
    }

    if (GetMenuKey(M_RIGHT))
    {
        PlayNote(MENU_SELECT, 1, 20);
    }

    switch (listpos)
    {
        case 0:
            AddText("DRAW DISTANCE", text_data(x, y, 8), RedText);
            AddText("RESOLUTION", text_data(x, y + l, 8), WhiteText);
            // AddText ("SCREEN SIZE", text_data (x, y+(l*2), 8), WhiteText) ;
            // AddText ("SKY", text_data (x, y+(l*2), 8), WhiteText) ;
            AddText("TEXTURE FILTER", text_data(x, y + (l * 2), 8), WhiteText);
            // AddText ("MIP MAPPING", text_data (x, y+(l*2), 8), WhiteText) ;
            AddText("SKY", text_data(x, y + (l * 3), 8), WhiteText);
            break;
        case 1:
            AddText("RESOLUTION", text_data(x, y + l, 8), RedText);
            AddText("DRAW DISTANCE", text_data(x, y, 8), WhiteText);
            // AddText ("SCREEN SIZE", text_data (x, y+(l*2), 8), WhiteText) ;
            //          AddText ("SKY", text_data (x, y+(l*2), 8), WhiteText) ;
            AddText("TEXTURE FILTER", text_data(x, y + (l * 2), 8), WhiteText);
            //          AddText ("MIP MAPPING", text_data (x, y+(l*2), 8), WhiteText) ;
            AddText("SKY", text_data(x, y + (l * 3), 8), WhiteText);
            break;
#if 0
   case 3:  //AddText ("MIP MAPPING", text_data (x, y+(l*2), 8), RedText) ;
      AddText ("DRAW DISTANCE", text_data (x, y, 8), WhiteText);
      AddText ("RESOLUTION", text_data (x, y+l, 8), WhiteText) ;
      AddText ("TEXTURE FILTER", text_data (x, y+(l*3), 8), WhiteText) ;
      //AddText ("PERS CORR", text_data (x, y+(l*4), 8), WhiteText) ;
      break;
#endif
        case 2:
            AddText("TEXTURE FILTER", text_data(x, y + (l * 2), 8), RedText);
            AddText("DRAW DISTANCE", text_data(x, y, 8), WhiteText);
            AddText("RESOLUTION", text_data(x, y + l, 8), WhiteText);
            // AddText ("SCREEN SIZE", text_data (x, y+(l*2), 8), WhiteText) ;
            // AddText ("SKY", text_data (x, y+(l*2), 8), WhiteText) ;
            // AddText ("MIP MAPPING", text_data (x, y+(l*2), 8), WhiteText) ;
            AddText("SKY", text_data(x, y + (l * 3), 8), WhiteText);
            break;
        case 3:
            AddText("SKY", text_data(x, y + (l * 3), 8), RedText);
            //       AddText ("MIP MAPPING", text_data (x, y+(l*2), 8), WhiteText) ;
            AddText("DRAW DISTANCE", text_data(x, y, 8), WhiteText);
            AddText("RESOLUTION", text_data(x, y + l, 8), WhiteText);
            // AddText ("SCREEN SIZE", text_data (x, y+(l*2), 8), WhiteText) ;
            AddText("TEXTURE FILTER", text_data(x, y + (l * 2), 8), WhiteText);
            break;
    }

    strcpy(ResStr, "320X240");

    AddText(DrawStr, text_data(x + 180, y, 8), RedText);
    AddText(ResStr, text_data(x + 180, y + l, 8), RedText);
    // AddText (ScreenStr, text_data (x+190, y+(l*2), 8), RedText) ;
    AddText(SkyStr, text_data(x + 180, y + (l * 3), 8), RedText);
    AddText(TexStr, text_data(x + 180, y + (l * 2), 8), RedText);
    //    AddText (PCStr, text_data (x+180, y+(l*3), 8), RedText) ;
}

void ControllerOptions(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    /*
    char controllerstring[20];
    char configstring[20];
    char colour1;
    char colour2;
    */
    davebodge = 1;

    if (andybodge)
    {
        andybodge = 0;
    }

    CentreText("CONTROLLER OPTIONS", text_data(1, 21, 8), RedText);

    DisplayButtons(30, 210, PSELECT);
    DisplayButtons(240, 210, PBACK);
    /*TODO input menu
       if (GetMenuKey(M_UP)||GetMenuKey(M_DOWN))
       {
          if (orig_controller != _JOYSTICK1)
          {
             if (changecontroller)
                changecontroller=0;
             else
                changecontroller=1;
          }
       }

       if (changecontroller)
       {
          colour1=RedText;
          colour2=WhiteText;
          if (GetMenuKey(M_LEFT))
          {
             PlayNote(MENU_MOVE, 1, 20);
             if (orig_controller==_KEYBOARD) orig_controller=_JOYSTICK1;
             else
                if (orig_controller==_MOUSE1) orig_controller=_KEYBOARD;
                else
                   if (orig_controller==_JOYSTICK1) orig_controller=_MOUSE1;
          }

          if (GetMenuKey(M_RIGHT))
          {
             PlayNote(MENU_MOVE, 1, 20);
             if (orig_controller==_KEYBOARD) orig_controller=_MOUSE1;
             else
                if (orig_controller==_MOUSE1) orig_controller=_JOYSTICK1;
                else
                   if (orig_controller==_JOYSTICK1) orig_controller=_KEYBOARD;
          }

       }
       else
       {
          if (orig_controller != _JOYSTICK1)
          {
             colour1=WhiteText;
             colour2=RedText;
          }
       }

       switch (orig_controller)
       {
       case _KEYBOARD: strcpy(configstring, "CONFIGURE KEYBOARD");
                      strcpy(controllerstring, "KEYBOARD");
                      CentreText (configstring, text_data (0, 170, 8), colour2) ;
                      break;
       case _MOUSE1: strcpy(configstring, "CONFIGURE MOUSE");
          strcpy(controllerstring, "MOUSE");
                      CentreText (configstring, text_data (0, 170, 8), colour2) ;
                      break;
       case _JOYSTICK1:
          if (!JJSJoyConfig())
          {
             colour1=WhiteText;
             colour2=RedText;
             strcpy(configstring, "NO JOYSTICK DETECTED");
             CentreText (configstring, text_data (0, 170, 8), colour2) ;
          }
          strcpy(controllerstring, "JOYSTICK");

          break;
       }
       CentreText (controllerstring, text_data (0, 50, 8), colour1) ;
    */
}

void ControllerEdit(MusicPrims* musicPrims, SlideTextures* musicTex, SelectionData* selectData, char loadGames[][9], ConfigData* gameData)
{
    DisplayButtons(30, 210, PSELECT);
    DisplayButtons(240, 210, PBACK);

    CentreText("CONFIGURE KEYBOARD", text_data(1, 21, 8), RedText);
    if (davebodge == 1)
    {
        KeyConfig0();
        davebodge = 0;
    }
    else
    {
        KeyConfig();
    }
}

void DrawControllerMenu(Menus* menuSystem, int* currMenu, int shapeNo)
{
    InitMenuDisplay(110);
    /*TODO input draw menu
       switch(orig_controller)
       {
          // jed
       case _KEYBOARD:
          etc_Joystick1Switch = _DEVICE_OFF; //jjs quick and dirty
          SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[1]->skeleton),0, 0, 3500) ;
          SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[1]->skeleton), hdg,0, 0);
          TransformStaticObject(menuSystem[(*currMenu)].menuModel[1]);
          hdg += 20;
          break;

          // jed
       case _MOUSE1:
          SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[0]->skeleton),0, 0, 4500) ;
          SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[0]->skeleton),  hdg,75,25);
          TransformStaticObject(menuSystem[(*currMenu)].menuModel[0]);
          hdg += 40;
          break;

          // jed
       case _JOYSTICK1:
          SetSkeletonPosition((menuSystem[(*currMenu)].menuModel[2]->skeleton),0, 0, 3500) ;
          SetSkeletonDirectionHPR((menuSystem[(*currMenu)].menuModel[2]->skeleton),  hdg,-50,0);
          TransformStaticObject(menuSystem[(*currMenu)].menuModel[2]);
          hdg += 40;
          break;
       }
    */
}

void KeyConfig0(void)
{
    /*TODO input
    char  keyc;


    // Keep a copy of the keys
    Keycons[0] = OriginalKeyCodes[15];
    Keycons[1] = OriginalKeyCodes[13];
    Keycons[2] = OriginalKeyCodes[12];
    Keycons[3] = OriginalKeyCodes[14];
    Keycons[4] = OriginalKeyCodes[0];
    Keycons[5] = OriginalKeyCodes[1];
    Keycons[6] = OriginalKeyCodes[6];
    Keycons[7] = OriginalKeyCodes[5];
    Keycons[8] = OriginalKeyCodes[4];
    Keycons[9] = OriginalKeyCodes[11];

    // Clear the main keys
    //
    // Left
    etc_KeyboardMap[etc_KeyboardConfig[15]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[15] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[15]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[15] + 128] = 0xFFFFFFFF;

    // Right
    etc_KeyboardMap[etc_KeyboardConfig[13]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[13] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[13]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[13] + 128] = 0xFFFFFFFF;

    // Up
    etc_KeyboardMap[etc_KeyboardConfig[12]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[12] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[12]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[12] + 128] = 0xFFFFFFFF;

    // Down
    etc_KeyboardMap[etc_KeyboardConfig[14]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[14] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[14]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[14] + 128] = 0xFFFFFFFF;

    // Brake Left
    etc_KeyboardMap[etc_KeyboardConfig[0]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[0] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[0]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[0] + 128] = 0xFFFFFFFF;

    // Brake Right
    etc_KeyboardMap[etc_KeyboardConfig[1]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[1] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[1]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[1] + 128] = 0xFFFFFFFF;

    // Accel
    etc_KeyboardMap[etc_KeyboardConfig[6]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[6] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[6]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[6] + 128] = 0xFFFFFFFF;

    // Fire
    etc_KeyboardMap[etc_KeyboardConfig[5]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[5] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[5]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[5] + 128] = 0xFFFFFFFF;

    // View [triangle]
    etc_KeyboardMap[etc_KeyboardConfig[4]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[4] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[4]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[4] + 128] = 0xFFFFFFFF;

    // Pause [Start]
    etc_KeyboardMap[etc_KeyboardConfig[11]] = 0;
    //etc_KeyboardMap[etc_KeyboardConfig[11] + 128] = 0xFFFFFFFF;
    etc_ExtendedKeyboardMap[etc_KeyboardConfig[11]] = 0;
    //etc_ExtendedKeyboardMap[etc_KeyboardConfig[11] + 128] = 0xFFFFFFFF;

    etc_KeyboardConfig[15] = 0;      // Left
    etc_KeyboardConfig[13] = 0;      // Right
    etc_KeyboardConfig[12] = 0;      // Up
    etc_KeyboardConfig[14] = 0;      // Down
    etc_KeyboardConfig[0] = 0;    // Brake Left
    etc_KeyboardConfig[1] = 0;    // Brake Right
    etc_KeyboardConfig[6] = 0;    // Accel
    etc_KeyboardConfig[5] = 0;    // Fire
    etc_KeyboardConfig[4] = 0;    // Change View
    etc_KeyboardConfig[11] = 0;      // Pause (start)

    for(keyc = 0; keyc < 16; keyc++)
       NewOriginalKeyCodes[keyc] = 0;

    etc_KeyPressed = 0;
    etc_Key = 0;
    */
}

void KeyConfig(void)
{
    /*TODO input
    char exists;
    unsigned short KeyCode = 0;

       CentreText ("PRESS KEY FOR", text_data (1, 6*12, 8), RedText) ;

       switch(KeyToChange)
       {
       case NONE:
          if(DelayCounter >= 15)
          {
             etc_KeyPressed = 0;
             etc_Key = 0;
             KeyToChange = LEFT;
             DelayCounter = 0;
          }
          else
             DelayCounter++;

          break;

       case LEFT:
          CentreText ("LEFT", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[15] = KeyCode;
                   NewOriginalKeyCodes[15] = KeyCode;
                   SetKey(15, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case RIGHT:
          CentreText ("RIGHT", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[13] = KeyCode;
                   NewOriginalKeyCodes[13] = KeyCode;
                   SetKey(13, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case UP:
          CentreText ("UP", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[12] = KeyCode;
                   NewOriginalKeyCodes[12] = KeyCode;
                   SetKey(12, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case DOWN:
          CentreText ("DOWN", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[14] = KeyCode;
                   NewOriginalKeyCodes[14] = KeyCode;
                   SetKey(14, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case BRAKEL:
          CentreText ("LEFT AIRBRAKE", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[0] = KeyCode;
                   NewOriginalKeyCodes[0] = KeyCode;
                   SetKey(0, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case BRAKER:
          CentreText ("RIGHT AIRBRAKE", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[1] = KeyCode;
                   NewOriginalKeyCodes[1] = KeyCode;
                   SetKey(1, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case ACCEL:
          CentreText ("ACCELERATE", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[6] = KeyCode;
                   NewOriginalKeyCodes[6] = KeyCode;
                   SetKey(6, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case FIRE:
          CentreText ("FIRE", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[5] = KeyCode;
                   NewOriginalKeyCodes[5] = KeyCode;
                   SetKey(5, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case VIEW_KEY:
          CentreText ("CHANGE VIEW", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[4] = KeyCode;
                   NewOriginalKeyCodes[4] = KeyCode;
                   SetKey(4, KeyCode);
                   KeyToChange++;
                }
             }
          }

          break;

       case PAUSE_KEY:
          CentreText ("PAUSE", text_data (1, 7*12, 8), WhiteText) ;

          if(etc_KeyPressed)
          {
             KeyCode = GetKeyNoBlock();

             if(KeyCode > 0)
             {
                exists = CheckKey(KeyCode);

                if(exists)
                {
                   PlayNote(MENU_SELECT, 1, 20);
                   OriginalKeyCodes[11] = KeyCode;
                   NewOriginalKeyCodes[11] = KeyCode;
                   SetKey(11, KeyCode);
                   KeyToChange = NONE;
                   davebodge = 2;
                }
             }
          }

          break;
       }
    */
}

void DrawControllerEdit(Menus* menuSystem, int* currMenu, int shapeNo) { InitMenuDisplay(80); }

void DrawGraphicDetail(Menus* menuSystem, int* currMenu, int shapeNo)
{
    int heading = gFrameIndex * 5;

    DrawTeamShips(0, 0, MODELSIZE, heading, menuSystem[(*currMenu)].menuModel[shipShapeOrder[0]]);
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
#if 0
void NetworkOptions(MusicPrims *musicPrims, SlideTextures *musicTex, SelectionData *selectData, char loadGames[][9], ConfigData *gameData)
{
   static int toggle = 0;
   static option = 1;
   char colour1;
   char colour2;
   char players_colour[10] =  { 1,1,1,1,1,1,1,1,1,1 };
   char socket_colour[10] =   { 1,1,1,1,1,1,1,1,1,1 };
   static char player_number;
   static char socket_number;

   CentreText ("CONFIGURE NETWORK", text_data (1,21, 8), RedText) ;

   DisplayButtons ( 30, 210, PSELECT);
   DisplayButtons ( 240, 210, PBACK);

   if (andybodge)
   {
      andybodge=0;
      player_number=CommNetPlayers;
      socket_number=CommNetSocket;
   }
   players_colour[player_number]=RedText;
   socket_colour[socket_number]=RedText;

   if (GetMenuKey(M_UP)||GetMenuKey(M_DOWN))
   {
      option ^=1;
   }



   if (option) // number of players
   {
      colour1=toggle%2;
      colour2=WhiteText;

      if (GetMenuKey(M_RIGHT))
      {
         players_colour[player_number]=WhiteText;
         player_number++;
         if (player_number>8) player_number=2;
         players_colour[player_number]=RedText;
      }
      if (GetMenuKey(M_LEFT))
      {
         players_colour[player_number]=WhiteText;
         player_number--;
         if (player_number<2) player_number=8;
         players_colour[player_number]=RedText;
      }
   }
   else // socket number
   {
      colour1=WhiteText;
      colour2=toggle%2;

      if (GetMenuKey(M_RIGHT))
      {
         socket_colour[socket_number]=WhiteText;
         socket_number++;
         if (socket_number>9) socket_number=0;
         socket_colour[socket_number]=RedText;
      }
      if (GetMenuKey(M_LEFT))
      {
         socket_colour[socket_number]=WhiteText;
         if (socket_number==0) socket_number=10;
         socket_number--;
         socket_colour[socket_number]=RedText;
      }
   }

   CentreText ("NUMBER OF PLAYERS", text_data (0, 80, 8), colour1) ;
   CentreText ("SELECT SOCKET", text_data (0, 140, 8), colour2) ;

   AddNumber(2, text_data(65,96,8), players_colour[2]);
   AddNumber(3, text_data(95,96,8), players_colour[3]);
   AddNumber(4, text_data(125,96,8), players_colour[4]);
   AddNumber(5, text_data(155,96,8), players_colour[5]);
   AddNumber(6, text_data(185,96,8), players_colour[6]);
   AddNumber(7, text_data(215,96,8), players_colour[7]);
   AddNumber(8, text_data(245,96,8), players_colour[8]);

   AddNumber(0, text_data(20,156,8),  socket_colour[0]);
   AddNumber(1, text_data(50,156,8),  socket_colour[1]);
   AddNumber(2, text_data(80,156,8),  socket_colour[2]);
   AddNumber(3, text_data(110,156,8),  socket_colour[3]);
   AddNumber(4, text_data(140,156,8), socket_colour[4]);
   AddNumber(5, text_data(170,156,8), socket_colour[5]);
   AddNumber(6, text_data(200,156,8), socket_colour[6]);
   AddNumber(7, text_data(230,156,8), socket_colour[7]);
   AddNumber(8, text_data(260,156,8), socket_colour[8]);
   AddNumber(9, text_data(290,156,8), socket_colour[9]);

   if (GetMenuKey(M_SELECT))
   {
      CommNetPlayers=player_number;
      CommNetSocket=socket_number;
      CommMultiPlaySelect=_TCP_NETWORK;
      davebodge=2;
   }

   toggle++;
}



void SerialOptions(MusicPrims *musicPrims, SlideTextures *musicTex, SelectionData *selectData, char loadGames[][9], ConfigData *gameData)
{
   static int toggle = 0;
   static option = 1;
   char colour1;
   char colour2;
   char comm_colour[5] =   {1,1,1,1,1};
   char baud_colour[3] =   {1,1,1};
   static char comm_port;
   static char baud_rate;

   CentreText ("CONFIGURE SERIAL", text_data (1,21, 8), RedText) ;

   DisplayButtons ( 30, 210, PSELECT);
   DisplayButtons ( 240, 210, PBACK);

   if (andybodge)
   {
      andybodge=0;
      comm_port = CommSerialDevice;
      baud_rate=CommSerialBaudRate;
   }

   comm_colour[comm_port]=RedText;
   baud_colour[baud_rate]=RedText;

   if (GetMenuKey(M_UP)||GetMenuKey(M_DOWN))
   {
      option ^=1;
   }



   if (option) // comm port
   {
      colour1=toggle%2;
      colour2=WhiteText;

      if (GetMenuKey(M_RIGHT))
      {
         comm_colour[comm_port]=WhiteText;
         comm_port++;
         if (comm_port > 4) comm_port = 1;
         comm_colour[comm_port]=RedText;
      }
      if (GetMenuKey(M_LEFT))
      {
         comm_colour[comm_port]=WhiteText;
         comm_port--;
         if (comm_port == 0) comm_port = 4;
         comm_colour[comm_port]=RedText;
      }
   }
   else // socket number
   {
      colour1=WhiteText;
      colour2=toggle%2;

      if (GetMenuKey(M_RIGHT))
      {
         baud_colour[baud_rate]=WhiteText;
         baud_rate++;
         if (baud_rate > 2) baud_rate=1;
         baud_colour[baud_rate]=RedText;
      }
      if (GetMenuKey(M_LEFT))
      {
         baud_colour[baud_rate]=WhiteText;
         if (baud_rate==0) baud_rate=2;
         baud_rate--;
         baud_colour[baud_rate]=RedText;
      }
   }

   CentreText ("COMM PORT", text_data (0, 80, 8), colour1) ;
   CentreText ("BAUD RATE", text_data (0, 140, 8), colour2) ;

   AddText("COM", text_data(46, 96, 8), comm_colour[1]);
   AddNumber(1, text_data(86,96,8), comm_colour[1]);

   AddText("COM", text_data(106, 96, 8), comm_colour[2]);
   AddNumber(2, text_data(146,96,8), comm_colour[2]);

   AddText("COM", text_data(166, 96, 8), comm_colour[3]);
   AddNumber(3, text_data(206,96,8), comm_colour[3]);

   AddText("COM", text_data(226, 96, 8), comm_colour[4]);
   AddNumber(4, text_data(266,96,8), comm_colour[4]);


   AddNumber(57600, text_data(80,156,8),  baud_colour[1]);
   AddNumber(115200, text_data(180,156,8),  baud_colour[2]);

   if (GetMenuKey(M_SELECT))
   {
      CommSerialDevice=comm_port;
      CommSerialBaudRate = baud_rate;
      CommMultiPlaySelect=_HEAD_2_HEAD;
      davebodge=2;
   }

   toggle++;
}



void  SerialNetworkHandling(MusicPrims *musicPrims, SlideTextures *musicTex, SelectionData *selectData, char loadGames[][9], ConfigData *gameData)
{
   global_mark_error = 0;
   gameType = SINGLE;

   if(davebodge != 4)
      MasterSlaveSIO(musicPrims,musicTex,selectData,loadGames,gameData);

   if(global_mark_error)
      davebodge = 3;
}

#endif
