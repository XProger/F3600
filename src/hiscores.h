#define NumNames 5
#define nameLetters 3
#define HiScoreTracks 14

typedef struct
{

    char names[NumNames][nameLetters + 1]; /* 20 */
    int lapTimes[NumNames]; /* 20 */
    signed char lapTimesPilot[NumNames]; /* 5  */
    short position[NumNames]; /* 10 */
    int lapRecord; /*  4 */
    signed char lapRecordPilot; /*  1 */
} HiScores; /* 64 */

typedef struct
{
    char nameEntered[9];
    char currChar;
    short charVal;
    short numChars;
    char dispChar[2];
    int toggleDisplay;
    char userEntry;
    char checkVal;
    char enterHs;
    int count;
    int padDelay;
    int bestLap;
    char highScore;
    char displayHiScore;
    short charX;
    char confirm;
    char lConfirm;
} HiScoreData;

void HighScore(void);
void AddEntry2Table(int, char[]);
int32 CheckLapRecord(ShipData* ship);
void DisplayHiScore(char);
void InitHiScoreData(HiScoreData*);
int CheckHiScore(HiScoreData*);
void NewHighScore(HiScoreData*);
char ChampLapRecord(ShipData*);
void NewHighScoreDraw(HiScoreData* hiScoreData);
