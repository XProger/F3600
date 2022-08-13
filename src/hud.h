#define NUM_BARS        12
#define SPEEDO_X        179
#define SPEEDO_Y        195 /*200*/
#define SPEEDO_WIDTH    128
#define SPEEDO_HEIGHT   32
#define BAR_X           178
#define BAR_Y           229
#define MAX_TEXT        256
#define MINUS           10
#define DOT             10
#define COLON           11
#define SLASH           12
#define RACE            0
#define MENU            1

typedef struct
{
    int start_pos_x;
    int start_pos_y;
    int size;
} TEXT_DATA;

typedef struct
{
    short vramX;
    short vramY;
    short width;
    short pad;
} TexChar;

typedef struct
{
    char pilot[25];
    short x, y;
} PilotData;

void InitText(void);
void InitSpeedo(void);
void fill_speedo(int, int);
void AddNumber(int, TEXT_DATA*, uint32);
void AddTime(int, TEXT_DATA*, uint32);
void AddText(const char*, TEXT_DATA*, uint32);
TEXT_DATA* text_data(int, int, int);
void UpdateRaceHud(ShipData*, int, ConfigData*);
void SetCharTextures(char);
void AddDelEnd(char, TEXT_DATA*, int);
void CentreText(const char*, TEXT_DATA*, uint32);
void DisplayLives(ConfigData*); // PCwipeout
int CalcPosition(ShipData* shipIndex);
void ScreenFooter(int x, int y, int colour);
void InitChampTable(void);
void DisplayChampTable(void);
void DisplayRacePoints(void);
void InitScreenTex(Texture** screenTex, const char* fileName);
void DisplayPicture(Texture* pic, short x, short y, short width, short height);
void DisplayVenomComplete(void);
void DisplayAllVenomComplete(void);
void InitTrackTex(Texture** trackTex);
void DisplayRaceInfo(ConfigData* gameData, HiScoreData* hiScoreData);
void DrawHud(ShipData* shipIndex, int choice, ConfigData* gameData);
void AddGlyph(int x, int y, int w, int h, int u, int v, uint32 color, const Texture* tex);
void InitWeaponSprtIcons(void);
void DrawWeaponSprtIcons(ShipData* playerShip);
