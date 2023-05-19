typedef struct
{
    uint16 thrust;
    uint16 view;
    uint16 fire;
    uint16 lBrake;
    uint16 rBrake;
} Binding;

#define MAX_REPLAY_SIZE 0xFFFF

typedef struct
{
    uint32 pos;
    uint32 size;
    uint8 tick;
    uint8 ctrl;
    uint8 stickX;
    uint8 stickY;
    uint8 data[MAX_REPLAY_SIZE];
} Replay;

typedef struct
{
    ShipData* ship;
    TrackCamera camera;
    RescueData droid;
    char name[16];
    int16 shapeIndex;
    int16 ready;
    int32 pad, xpad, ctrl;
    int32 joyIndex;
    int32 buttons, obuttons, Lx, Ly, Rx, Ry; // input
    int32 stickX, stickY;
    Binding binding;

    uint32 lapTimes[8];

#ifdef USE_SHIP_GHOST
    Replay replay;
#endif
} PlayerData;

char TrackDirection(const TrackCamera* camera);
void DrawSky(void);
void DrawLandscape(TrackSection* section);
void DrawTrack(int32 playerIndex, int32 specialed);
void DrawObject(Object* obj, int x, int y, int z, short hdg, short pitch, short roll);
void DrawView(int32 playerIndex);
