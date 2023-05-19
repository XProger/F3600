#define NUM_BARS        12

typedef struct
{
    int start_pos_x;
    int start_pos_y;
    int size;
} TEXT_DATA;

void DisplaySpeedo(int32 speed, int32 thrust);
void AddNumber(int, TEXT_DATA*, uint32);
void AddTime(int, TEXT_DATA*, uint32);
void AddTextStr(const char* text_ptr, TEXT_DATA* pos_data, uint32 colour);
void AddText(int32 strId, TEXT_DATA* pos_data, uint32 colour);
TEXT_DATA* text_data(int, int, int);
void SetCharTextures(int32 size);
void AddDelEnd(char dispChar, int32 x, int32 y, uint32 colour);
void CentreTextStr(const char* the_text_ptr, TEXT_DATA* pos_data, uint32 colour);
void CentreText(int32 strId, TEXT_DATA* pos_data, uint32 colour);
void DisplayLives(void);
void InitChampTable(void);
void DisplayChampTable(void);
void DisplayRacePoints(void);
void DisplayVenomComplete(void);
void DisplayAllVenomComplete(void);
void DisplayWinLose(int32 index);
void DisplayRaceInfo(HiScoreData* hiScoreData);
void DrawHud(PlayerData* player);
void DrawWeaponIcon(ShipData* ship);
void UpdateGameDisplay(int32 width, int32 height);