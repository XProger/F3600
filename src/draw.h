typedef struct
{
    ShipData* ship;
    TrackCamera camera;
    RescueData droid;
    char name[16];
    int16 shipNum;
    int16 ready;
    int32 pad, xpad;
    int32 joyIndex;
    int32 buttons, obuttons, Lx, Ly, Rx, Ry; // input
    int32 stickX, stickY;
} PlayerData;

char TrackDirection(const TrackCamera* camera);
void DrawSky(void);
void DrawLandscape(TrackSection* section);
void DrawTrack(int32 playerIndex);
void DrawObject(Object* obj, int x, int y, int z, short hdg, short pitch, short roll);
void DrawView(int32 playerIndex);
