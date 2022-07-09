#include "game.h"

extern short starttrack, endtrack;

const char* const fulltrack[] = { "RANDOM", "CAIRODROME", "CARDINAL DANCER", "COLD COMFORT", "DOH T", "MESSIJ", "OPERATIQUE", "TENTATIVE", "TRANCEVAAL", "AFRO RIDE", "CHEMICAL BEATS", "WIPEOUT" };
const char* const sometrack[] = { "RANDOM", "COLD COMFORT", "DOH T", "MESSIJ", "TENTATIVE", "WIPEOUT" };

const char* GetCdTrackName(int no)
{
    if (no < 0)
        return ("NO CD");
    else if (endtrack - starttrack == 7)
        return (fulltrack[no]);
    else
        return (sometrack[no]);
}

void InGameReset(int choice, char machinePaused, ConfigData* gameData, int* cdPause)
{
    int i;
    char ingameMenu[4][9];
    short yPos = 122, xPos[4];
    char gap = 16;

    if (BLINK_SLOW)
    {
        CentreText("PAUSED", text_data(107, 90, 12), RedText);
    }

    if (machinePaused == SINGLE || machinePaused == MASTER)
    {
        strcpy(ingameMenu[0], "CONTINUE");
        strcpy(ingameMenu[1], "RESTART");
        strcpy(ingameMenu[2], "QUIT");
        strcpy(ingameMenu[3], "CD TRACK");
        xPos[0] = 120;
        xPos[1] = 128;
        xPos[2] = 140;
        xPos[3] = 140;

        for (i = 0; i < 4; i++)
        {
            CentreText(&(ingameMenu[i][0]), text_data(xPos[i], yPos + (gap * i), 8), (i == choice) ? BLINK_TEXT : WhiteText);
        }

        if (choice == 3)
        {
            CentreText(GetCdTrackName(gameData->cdTrack - 1), text_data(0, yPos + (gap * 4), 8), RedText);
        }
    }
}

void InGameConfirm(char* confirm, int choice, short xpad, char* lConfirm)
{
    if (choice == 1)
    {
        CentreText("ARE YOU SURE YOU", text_data(77, 95, 8), WhiteText);
        CentreText("WANT TO RESTART", text_data(90, 105, 8), WhiteText);
    }
    else
    {
        CentreText("ARE YOU SURE YOU", text_data(77, 95, 8), WhiteText);
        CentreText("WANT TO QUIT", text_data(105, 105, 8), WhiteText);
    }

#if 0
   if ((GetMenuKey(M_LEFT)) || (GetMenuKey(M_RIGHT)))
   {
      *lConfirm ^= 1 ;
      PlayNote(MENU_MOVE, 1, 20);
   }
#endif

    AddText("YES", text_data(75, 120, 16), (*lConfirm) ? BLINK_TEXT : WhiteText);
    AddText("NO", text_data(205, 120, 16), (*lConfirm) ? WhiteText : BLINK_TEXT);

#if 0
   if ((GetMenuKey(M_SELECT)) || (GetMenuKey(M_START)))
      *confirm = *lConfirm ;
#endif
}

void DeSelectScreen(void)
{
    /*TODO deselect screen?
       SetPolyF4(&(selectBox[CurrentScreen]));
       selectBox[CurrentScreen].r0=230;
       selectBox[CurrentScreen].g0=0;
       selectBox[CurrentScreen].b0=BLACK2_TRANSL;

       setXY4(&(selectBox[CurrentScreen]),0 , 0,
                              320 , 0,
                              0 , 240,
                              320 , 240);

       SetSemiTrans((P_TAG *)&(selectBox[CurrentScreen]), 1);

    // SetDrawMode(&(selectTrans[CurrentScreen]), 1, 1, GetTPage(0, 0, 0, 0), 0);
       SetDrawMode(&(selectTrans[CurrentScreen]), 1, 1, FONT_TPAGE, 0);


       AddPrim( OT[ CurrentScreen ] , ( ulong* ) &(selectBox[CurrentScreen]) );
       AddPrim( OT[ CurrentScreen ] , ( ulong* ) &(selectTrans[CurrentScreen]) );
       */
}
