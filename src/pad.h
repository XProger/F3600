enum InputKey
{
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

typedef struct PadInfo
{
    uint16 thrust;
    uint16 view;
    uint16 fire;
    uint16 lBrake;
    uint16 rBrake;
    uint16 up;
    uint16 down;
    uint16 left;
    uint16 right;
} PadInfo;

void InitPadSettings(void);
void UpdatePad();
void SetBrake(ShipData* playerShip, short padLeft, short padRight);
void SetView(ShipData* playerShip, short padView);
void SetFire(ShipData* playerShip, short padFire, WeaponData* weaponIndex, int shipNo);
short GetMenuKey(short key);
