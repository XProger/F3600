enum InputKey
{
    IN_NONE     = 0,
    IN_LEFT     = (1 << 0),
    IN_DOWN     = (1 << 1),
    IN_RIGHT    = (1 << 2),
    IN_UP       = (1 << 3),
    IN_X        = (1 << 4),
    IN_A        = (1 << 5),
    IN_B        = (1 << 6),
    IN_Y        = (1 << 7),
    IN_L        = (1 << 8),
    IN_R        = (1 << 9),
    IN_LB       = (1 << 10),
    IN_LT       = (1 << 11),
    IN_RB       = (1 << 12),
    IN_RT       = (1 << 13),
    IN_SELECT   = (1 << 14),
    IN_START    = (1 << 15)
};

enum MenuKey
{
    M_UP,
    M_DOWN,
    M_LEFT,
    M_RIGHT,
    M_BACK,
    M_SELECT,
    M_START
};

enum CtrlKey
{
    CTRL_LEFT   = (1 << 0),
    CTRL_DOWN   = (1 << 1),
    CTRL_RIGHT  = (1 << 2),
    CTRL_UP     = (1 << 3),
    CTRL_THRUST = (1 << 4),
    CTRL_FIRE   = (1 << 5),
    CTRL_LBRAKE = (1 << 6),
    CTRL_RBRAKE = (1 << 7)
};

void UpdatePad(void);
void SetBrake(ShipData* ship, short padLeft, short padRight);
void SetView(ShipData* ship, short padView);
void SetFire(ShipData* ship, short padFire, int shipNo);
int32 GetMenuKeyPlayer(int32 key, int32 playerIndex);
int32 GetMenuKey(int32 key);

void padReset(int32 joyIndex);
void padSetKey(int32 joyIndex, int32 inputKey, int32 down);
void padSetStick(int32 joyIndex, int32 inputKey, int32 x, int32 y);

