#include "game.h"

const char* const gMusicTrackNames[] = { "RANDOM", "CAIRODROME", "CARDINAL DANCER", "COLD COMFORT", "DOH T", "MESSIJ", "OPERATIQUE", "TENTATIVE", "TRANCEVAAL", "AFRO RIDE", "CHEMICAL BEATS", "WIPEOUT" };
const char* const gInGameMenu[] = { "CONTINUE", "RESTART", "QUIT", "TRACK" };

const char* GetMusicTrackName(int musicTrack)
{
    return gMusicTrackNames[musicTrack];
}

void InGameReset(int choice)
{
    int i;

    if (BLINK_SLOW)
    {
        CentreText("PAUSED", text_data(107, 90, 12), TEXT_COLOR_HIGHLIGHT);
    }

    for (i = 0; i < 4; i++)
    {
        CentreText(gInGameMenu[i], text_data(0, 122 + (16 * i), 8), (i == choice) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    }

    if (choice == 3)
    {
        CentreText(GetMusicTrackName(gMusicTrack), text_data(0, 122 + (16 * 4), 8), TEXT_COLOR_HIGHLIGHT);

        if (gMusicTrack == 0) // show the actual track title for random
        {
            i = MusicGetTrack();
            CentreText(GetMusicTrackName(i), text_data(0, 122 + (16 * 5), 8), TEXT_COLOR_HIGHLIGHT);
        }
    }
}

void InGameConfirm(char* confirm, int choice, short xpad, char* lConfirm)
{
    CentreText("ARE YOU SURE YOU", text_data(0, 95, 8), TEXT_COLOR_WHITE);

    if (choice == 1)
    {
        CentreText("WANT TO RESTART", text_data(0, 105, 8), TEXT_COLOR_WHITE);
    }
    else
    {
        CentreText("WANT TO QUIT", text_data(0, 105, 8), TEXT_COLOR_WHITE);
    }

    AddText("YES", text_data(75, 120, 16), (*lConfirm) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText("NO", text_data(205, 120, 16), (*lConfirm) ? TEXT_COLOR_WHITE : BLINK_TEXT);
}
