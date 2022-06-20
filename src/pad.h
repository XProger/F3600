#include "input.h"

#define M_UP      0
#define M_DOWN    1
#define M_LEFT    2
#define M_RIGHT   3
#define M_BACK    4
#define M_SELECT  5
#define M_START   6

typedef struct PadInfo
{
   u_short     thrust;
   u_short     view;
   u_short     fire;
   u_short     lBrake;
   u_short     rBrake;
   u_short     up;
   u_short     down;
   u_short     left;
   u_short     right;
} PadInfo;

void  InitPadSettings   ();
void  SetBrake          (ShipData *playerShip, short padLeft, short padRight);
void  SetView           (ShipData *playerShip, short padView);
void  SetFire           (ShipData *playerShip, short padFire, WeaponData *weaponIndex, int shipNo);
short GetMenuKey        (short key);
