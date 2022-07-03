#include "standard.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "tilelib.h"
#include "ttf.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "droid.h"
#include "camera.h"
#include "draw.h"
#include "scene.h"
#include "dynam.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "sound.h"
#include "effects.h"
#include "fileops.h"
#include "options.h"
#include "global.h"
#include "cone.h"
#include "race.h"

const char Mds[]        = "MANAGING DIRECTORS";
const char DirDev[]     = "DIRECTOR OF DEVELOPMENT";
const char Prod[]       = "PRODUCERS";
const char ProdMan[]    = "PRODUCT MANAGER";
const char TeamL[]      = "TEAM LEADER";
const char GameD[]      = "GAME DESIGNER";

const char PSX[]        = "PLAYSTATION VERSION";
const char w95[]        = "ATI 3D RAGE VERSION";
const char XPORT[]      = "XPORT VERSION";
const char bsd[]        = "BROADSWORD INTERACTIVE LTD";
const char pro[]        = "PRODUCER";
const char atidev[]     = "DEVELOPED BY";
const char ati[]        = "ATI LIASON";
const char ati2[]       = "ATI QA";
const char Prog[]       = "PROGRAMMERS";
const char Artists[]    = "ORIGINAL ARTISTS";
const char Intro[]      = "INTRO SEQUENCE";
const char Concept[]    = "CONCEPTUAL ARTIST";
const char GDesign[]    = "ADDITIONAL GRAPHIC DESIGN";
const char Music[]      = "MUSIC";
const char MusicL[]     = "MUSIC LIAISON";
const char Sound[]      = "SOUND EFFECTS";
const char Manual[]     = "MANUAL WRITTEN BY";
const char Pack[]       = "PACKAGING DESIGN";
const char Tech[]       = "TECHNICAL ASSISTANT";
const char Sec[]        = "DEVELOPMENT SECRETARY";
const char QA[]         = "QUALITY ASSURANCE";
const char Thanks[]     = "SPECIAL THANKS TO";
const char LOrbital[]   = "ORBITAL";
const char LChem[]      = "CHEMICAL BROTHERS";
const char LLeft[]      = "LEFTFIELD";

const char blank[]      = "";
const char PCVersion[]  = "PC VERSION";
const char PCArtists[]  = "CONVERSION ARTISTS";
const char PCSndProg[]  = "SOUND PROGRAMMING";
const char PCNetwork[]  = "NETWORK PROGRAMMING";
const char PCMovies[]   = "MOVIE PROGRAMMING";

const char PCAndyY[]    = "ANDY YELLAND";
const char PCAndyS[]    = "ANDY SATTERTHWAITE";
const char PCDaveS[]    = "DAVE SMITH";
const char PCJedA[]     = "JED ADAMS";
const char PCMarkK[]    = "MARK KELLY";
const char PCSteveW[]   = "STEVE WARD";
const char PCChrisE[]   = "CHRIS EDEN";
const char PCSalim[]    = "SALIM SIWANI";
const char PCAndyC[]    = "ANDY CROWLEY";
const char PCSteveD[]   = "STEVE DAVIES";
const char PCMikeA[]    = "MIKE ANTHONY";
const char PCGaryB[]    = "GARY BURLEY";
const char PCJohnD[]    = "JOHN DWYER";
const char ATISteve[]   = "STEPHEN ROSE";
const char ATIJohn[]    = "JOHN JONES STEELE";
const char ATIBill[]    = "BILL ALLEN";

const char Dave[]       = "DAVE ROSE";
const char Stu[]        = "STEWART SOCKETT";
const char Rob[]        = "ROB SMITH";
const char Jas[]        = "JASON DENTON";
const char IanH[]       = "IAN HETHERINGTON";
const char JohnE[]      = "JONATHAN ELLIS";
const char JohnW[]      = "JOHN WHITE";
const char Dom[]        = "DOMINIC MALLINSON";
const char Sue[]        = "SUE CAMPBELL";
const char Nicky[]      = "NICKY CARUS WESTCOTT";
const char Nik[]        = "NICK BURCOMBE";
const char Laura[]      = "LAURA GRIEVE";
const char Lou[]        = "LOUISE SMITH";
const char Darren[]     = "DARREN DOUGLAS";
const char Pol[]        = "POL SIGERSON";
const char Lee[]        = "LEE CARUS WESTCOTT";
const char Jim[]        = "JIM BOWERS";
const char DR[]         = "THE DESIGNERS REPUBLIC";
const char Orbital[]    = "ORBITAL";
const char ChemBro[]    = "THE CHEMICAL BROTHERS";
const char LeftF[]      = "LEFTFIELD";
const char ColdS[]      = "COLD STORAGE";
const char PaulMG[]     = "PAUL MCGARVEY";
const char TimW[]       = "TIM WRIGHT";
const char Damon[]      = "DAMON FAIRCLOUGH";
const char Keith[]      = "KEITH HOPWOOD";
const char Paul[]       = "PAUL CHARSLEY";
const char Jen[]        = "JENNIFER REES";
const char XProger[]    = "TIMUR GAGIEV";

const char Test0[]      = "STUART ALLEN";
const char Test1[]      = "CHRIS GRAHAM";
const char Test2[]      = "THOMAS REES";
const char Test3[]      = "BRIAN WALSH";
const char Test4[]      = "CARL BERRY";
const char Test5[]      = "MARK INMAN";
const char Test6[]      = "PAUL TWEEDLE";
const char Test7[]      = "ANTHONY CROSS";
const char Test8[]      = "EDWARD HAY";
const char Test9[]      = "ROB WOLFE";

const char Hack[]       = "THE HACKERS TEAM MGM";
const char Soft[]       = "SOFTIMAGE";
const char SGI[]        = "SGI";
const char Glen[]       = "GLEN OCONNELL";
const char Jo[]         = "JOANNE GALVIN";
const char All[]        = "ALL AT PSYGNOSIS";
const char Pizza[]      = "PERFECT PIZZA";

typedef struct Title
{
    const char* title;
    int         noOfNames;
    const char* name[10];
} Title;

const Title credits[] = {
    { Mds,         2,    IanH, JohnE },
    { DirDev,      1,    JohnW },
    { Prod,        2,    Dom, PCAndyY },
    { ProdMan,     1,    Sue },
    { GameD,       6,    Nik, blank, blank, blank, blank, blank },

    { w95,         0 },
    { pro,         1,    ATIBill },
    { atidev,      0 },
    { bsd,         2,    ATISteve, ATIJohn },
    { PCVersion,   0 },
    { Prog,        8,    PCAndyY, PCAndyS, PCDaveS, PCMarkK, PCJedA, PCSteveW, PCChrisE, PCSalim },
    { PCSndProg,   1,    PCAndyC },
    { PCMovies,    1,    PCMikeA },
    { PCArtists,   7,    PCJohnD, PCGaryB, blank, blank, blank, blank, blank },

    { Intro,       1,    Lee },
    { Concept,     1,    Jim },
    { GDesign,     1,    DR },
    { Music,       1,    ColdS },
    { Sound,       1,    TimW },
    { Manual,      2,    Damon, Nik },
    { Pack,        2,    DR, Keith },
    { Sec,         6,    Jen, blank, blank, blank, blank, blank },

    { PSX,         0 },
    { Prog,        4,    Dave, Rob, Jas, Stu },
    { Artists,     10,   Nicky, Laura, Lou, Darren, Pol, blank, blank, blank, blank, blank },

    { QA,          10,   Test0, Test1, Test2, Test3, Test4, Test5, Test6, Test7, Test8, Test9 },

    { Thanks,      9,    Hack, Soft, SGI, Glen, Jo, All, blank, blank, blank },

    { XPORT,       0 },
    { Prog,        1,    XProger },
};

void ShowCredits(int attractDelay)
{
    int i, j;
    int place;

    place = attractDelay - (60 * FR30); // whats set in race.c -400
    for (i = 0; i < sizeof(credits) / sizeof(credits[0]); i++)
    {
        if ((place > -10) && (place < 264))
            CentreText(credits[i].title, text_data(230, place, 8), RedText);
        place += 16;
        for (j = 0; j < credits[i].noOfNames; j++)
        {
            if ((place > -10) && (place < 264))
                CentreText(credits[i].name[j], text_data(230, place, 8), WhiteText);
            place += 12;
        }
        place += 16;
    }
}
