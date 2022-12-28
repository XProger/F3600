#define NUM_BARS        12

typedef struct
{
    int start_pos_x;
    int start_pos_y;
    int size;
} TEXT_DATA;

typedef struct
{
    char pilot[25];
    short x, y;
} PilotData;

void fill_speedo(int, int);
void AddNumber(int, TEXT_DATA*, uint32);
void AddTime(int, TEXT_DATA*, uint32);
void AddText(const char*, TEXT_DATA*, uint32);
TEXT_DATA* text_data(int, int, int);
void UpdateRaceHud(ShipData*, int, ConfigData*);
void SetCharTextures(int32 size);
void AddDelEnd(char dispChar, int32 x, int32 y, uint32 colour);
void CentreText(const char*, TEXT_DATA*, uint32);
void DisplayLives(ConfigData*);
int CalcPosition(ShipData* shipIndex);
void InitChampTable(void);
void DisplayChampTable(void);
void DisplayRacePoints(void);
void DisplayVenomComplete(void);
void DisplayAllVenomComplete(void);
void DisplayWinLose(int32 index);
void DisplayRaceInfo(ConfigData* gameData, HiScoreData* hiScoreData);
void DrawHud(ShipData* shipIndex, int choice, ConfigData* gameData);
void DrawWeaponSprtIcons(ShipData* playerShip);
