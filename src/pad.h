#include "input.h"

#define M_UP        0
#define M_DOWN      1
#define M_LEFT      2
#define M_RIGHT     3
#define M_BACK      4
#define M_SELECT    5
#define M_START     6

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
void SetBrake(ShipData* playerShip, short padLeft, short padRight);
void SetView(ShipData* playerShip, short padView);
void SetFire(ShipData* playerShip, short padFire, WeaponData* weaponIndex, int shipNo);
short GetMenuKey(short key);
