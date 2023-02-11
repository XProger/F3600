#include "game.h"

const char* const gMusicTrackNames[] = { "", "CAIRODROME", "CARDINAL DANCER", "COLD COMFORT", "DOH T", "MESSIJ", "OPERATIQUE", "TENTATIVE", "TRANCEVAAL", "AFRO RIDE", "CHEMICAL BEATS", "WIPEOUT" };

const char* GetMusicTrackName(int musicTrack)
{
    if (musicTrack == 0)
        return STR[STR_RANDOM];
    return gMusicTrackNames[musicTrack];
}

void InGameReset(int choice)
{
    int i;

    if (BLINK_SLOW)
    {
        CentreText(STR_INGAME_PAUSED, text_data(0, 90, 12), TEXT_COLOR_HIGHLIGHT);
    }

    for (i = 0; i < 4; i++)
    {
        CentreText(STR_INGAME_CONTINUE + i, text_data(0, 122 + (16 * i), 8), (i == choice) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    }

    if (choice == 3)
    {
        CentreTextStr(GetMusicTrackName(gMusicTrack), text_data(0, 122 + (16 * 4), 8), TEXT_COLOR_HIGHLIGHT);

        if (gMusicTrack == 0) // show the actual track title for random
        {
            i = MusicGetTrack();
            CentreTextStr(GetMusicTrackName(i), text_data(0, 122 + (16 * 5), 8), TEXT_COLOR_HIGHLIGHT);
        }
    }
}

void InGameConfirm(int choice, char* lConfirm)
{
    CentreText(STR_ACK_SURE, text_data(0, 95, 8), TEXT_COLOR_WHITE);

    if (choice == 1)
    {
        CentreText(STR_ACK_WANT_RESTART, text_data(0, 105, 8), TEXT_COLOR_WHITE);
    }
    else
    {
        CentreText(STR_ACK_WANT_QUIT, text_data(0, 105, 8), TEXT_COLOR_WHITE);
    }

    AddText(STR_YES, text_data(75, 120, 16), (*lConfirm) ? BLINK_TEXT : TEXT_COLOR_WHITE);
    AddText(STR_NO, text_data(205, 120, 16), (*lConfirm) ? TEXT_COLOR_WHITE : BLINK_TEXT);
}
