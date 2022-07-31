#include "game.h"

extern short starttrack, endtrack;

const char* const fulltrack[] = { "RANDOM", "CAIRODROME", "CARDINAL DANCER", "COLD COMFORT", "DOH T", "MESSIJ", "OPERATIQUE", "TENTATIVE", "TRANCEVAAL", "AFRO RIDE", "CHEMICAL BEATS", "WIPEOUT" };
const char* const sometrack[] = { "RANDOM", "COLD COMFORT", "DOH T", "MESSIJ", "TENTATIVE", "WIPEOUT" };
const char* const ingameMenu[] = { "CONTINUE", "RESTART", "QUIT", "CD TRACK" };

const char* GetCdTrackName(int no)
{
    if (no < 0)
        return ("NO CD");
    else if (endtrack - starttrack == 7)
        return (fulltrack[no]);
    else
        return (sometrack[no]);
}

void InGameReset(int choice, ConfigData* gameData, int* cdPause)
{
    int i;

    if (BLINK_SLOW)
    {
        CentreText("PAUSED", text_data(107, 90, 12), RedText);
    }

    for (i = 0; i < 4; i++)
    {
        CentreText(ingameMenu[i], text_data(0, 122 + (16 * i), 8), (i == choice) ? BLINK_TEXT : WhiteText);
    }

    if (choice == 3)
    {
        CentreText(GetCdTrackName(gameData->cdTrack - 1), text_data(0, 122 + (16 * 4), 8), RedText);
    }
}

void InGameConfirm(char* confirm, int choice, short xpad, char* lConfirm)
{
    CentreText("ARE YOU SURE YOU", text_data(0, 95, 8), WhiteText);

    if (choice == 1)
    {
        CentreText("WANT TO RESTART", text_data(0, 105, 8), WhiteText);
    }
    else
    {
        CentreText("WANT TO QUIT", text_data(0, 105, 8), WhiteText);
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
