#include "game.h"

const char *creditsText[] = {
  " MANAGING DIRECTORS"
, "IAN HETHERINGTON"
, "JONATHAN ELLIS"
, " DIRECTOR OF DEVELOPMENT"
, "JOHN WHITE"
, " PRODUCERS"
, "DOMINIC MALLINSON"
, "ANDY YELLAND"
, " PRODUCT MANAGER"
, "SUE CAMPBELL"
, " GAME DESIGNER"
, "NICK BURCOMBE"
, ""
, ""
, " ATI 3D RAGE VERSION"
, " PRODUCER"
, "BILL ALLEN"
, " DEVELOPED BY"
, " BROADSWORD INTERACTIVE LTD"
, "STEPHEN ROSE"
, "JOHN JONES STEELE"
, " PC VERSION"
, " PROGRAMMERS"
, "ANDY YELLAND"
, "ANDY SATTERTHWAITE"
, "DAVE SMITH"
, "MARK KELLY"
, "JED ADAMS"
, "STEVE WARD"
, "CHRIS EDEN"
, "SALIM SIWANI"
, " SOUND PROGRAMMING"
, "ANDY CROWLEY"
, " MOVIE PROGRAMMING"
, "MIKE ANTHONY"
, " CONVERSION ARTISTS"
, "JOHN DWYER"
, "GARY BURLEY"
, ""
, ""
, " INTRO SEQUENCE"
, "LEE CARUS WESTCOTT"
, " CONCEPTUAL ARTIST"
, "JIM BOWERS"
, " ADDITIONAL GRAPHIC DESIGN"
, "THE DESIGNERS REPUBLIC"
, " MUSIC"
, "COLD STORAGE"
, "LEFTFIELD" // was hidden
, "CHEMICAL BROTHERS" // was hidden
, "ORBITAL" // was hidden
, " SOUND EFFECTS"
, "TIM WRIGHT"
, " MANUAL WRITTEN BY"
, "DAMON FAIRCLOUGH"
, "NICK BURCOMBE"
, " PACKAGING DESIGN"
, "THE DESIGNERS REPUBLIC"
, "KEITH HOPWOOD"
, " DEVELOPMENT SECRETARY"
, "JENNIFER REES"
, ""
, ""
, " PLAYSTATION VERSION"
, " PROGRAMMERS"
, "DAVE ROSE"
, "ROB SMITH"
, "JASON DENTON"
, "STEWART SOCKETT"
, " ORIGINAL ARTISTS"
, "NICKY CARUS WESTCOTT"
, "LAURA GRIEVE"
, "LOUISE SMITH"
, "DARREN DOUGLAS"
, "POL SIGERSON"
, ""
, ""
, " QUALITY ASSURANCE"
, "STUART ALLEN"
, "CHRIS GRAHAM"
, "THOMAS REES"
, "BRIAN WALSH"
, "CARL BERRY"
, "MARK INMAN"
, "PAUL TWEEDLE"
, "ANTHONY CROSS"
, "EDWARD HAY"
, "ROB WOLFE"
, " SPECIAL THANKS TO"
, "THE HACKERS TEAM MGM"
, "SOFTIMAGE"
, "SGI"
, "GLEN OCONNELL"
, "JOANNE GALVIN"
, "ALL AT PSYGNOSIS"
, "PERFECT PIZZA" // was hidden
};

void ShowCredits(int attractDelay)
{
    int32 i, y;
    int32 isTitle;
    const char* str;

    y = attractDelay - (60 * FR30) - 16; // whats set in race.c -400
    for (i = 0; i < sizeof(creditsText) / sizeof(creditsText[0]); i++)
    {
        str = creditsText[i];
        isTitle = (str[0] == ' ');

        if (isTitle)
        {
            str++;
            y += 12; // was 16
        }

        if ((y > -10) && (y < 264))
        {
            CentreTextStr(str, text_data(0, y, 8), isTitle ? TEXT_COLOR_HIGHLIGHT : TEXT_COLOR_WHITE);
        }

        y += isTitle ? 16 : 12;
    }
}
