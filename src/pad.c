#include "game.h"

void InitPadSettings(void)
{
    pCon.thrust = IN_A;
    pCon.view   = IN_Y;
    pCon.fire   = IN_B;
    pCon.lBrake = IN_RT;
    pCon.rBrake = IN_LT;
    pCon.up     = IN_UP;
    pCon.down   = IN_DOWN;
    pCon.left   = IN_LEFT;
    pCon.right  = IN_RIGHT;
}

void UpdatePad()
{
    opad = pad;
    pad = gPad;
    xpad = (pad ^ opad) & pad;
}

void SetBrake(ShipData* playerShip, short padLeft, short padRight)
{
    // right
    if (pad & padRight)
        playerShip->r_brake_rot += FR32U;
    else if (playerShip->r_brake_rot > 0)
        playerShip->r_brake_rot -= FR32U;

    if (playerShip->r_brake_rot < 0)
        playerShip->r_brake_rot = 0;

    if (playerShip->r_brake_rot > 256)
        playerShip->r_brake_rot = 256;

    // left
    if (pad & padLeft)
        playerShip->l_brake_rot += FR32U;
    else if (playerShip->l_brake_rot > 0)
        playerShip->l_brake_rot -= FR32U;

    if (playerShip->l_brake_rot < 0)
        playerShip->l_brake_rot = 0;

    if (playerShip->l_brake_rot > 256)
        playerShip->l_brake_rot = 256;
}

void SetView(ShipData* playerShip, short padView)
{
    if (xpad & padView)
    {
        if ((playerShip->attr & VIEW_IN) == 0)
        {
            CameraUpdate = UpdateCameraRaceInternal;
            playerShip->attr |= VIEW_IN;
        }
        else
        {
            CameraUpdate = UpdateCameraRaceExternal;
            playerShip->attr &= ~VIEW_IN;
        }
    }
}

void SetFire(ShipData* playerShip, short padFire, WeaponData* weaponIndex, int shipNo)
{
    if ((xpad & padFire) && (playerShip->weaponType != NO_WEAPON))
    {
        if ((playerShip->attr & SHIELDED) == 0)
            FireWeapon(playerShip, weaponIndex, shipNo, playerShip->weaponType);
        else
            PlayNote(MENU_MOVE, 1, 20);
    }
}

short GetMenuKey(short key)
{
    switch (key)
    {
        case M_UP:
            return (xpad & IN_UP);
        case M_DOWN:
            return (xpad & IN_DOWN);
        case M_LEFT:
            return (xpad & IN_LEFT);
        case M_RIGHT:
            return (xpad & IN_RIGHT);
        case M_BACK:
            return (xpad & (IN_B | IN_START));
        case M_SELECT:
            return (xpad & IN_A);
        case M_START:
            return (xpad & IN_START);
        default:
            break;
    }
    return (1); // PCwipeout
}
