#include "game.h"

void UpdatePad(void)
{
    int32 i, pad, opad, ctrl;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;

        if (player->Lx != 0)
        {
            int32 right = (player->Lx > 0) ? 1 : 0;
            player->stickX = (abs(player->Lx) + right) >> 7;
        }
        else
        {
            player->stickX = 256;
        }

        if (player->Ly != 0)
        {
            int32 up = (player->Ly > 0) ? 1 : 0;
            player->stickY = (abs(player->Ly) + up) >> 6;
        }
        else
        {
            player->stickY = 256;
        }

        if (player->stickX < 1)
            player->stickX = 1;
        if (player->stickY < 1)
            player->stickY = 1;

        if (player->stickX > 256)
            player->stickX = 256;
        if (player->stickY > 256)
            player->stickY = 256;

        pad = player->buttons;
        opad = player->pad;
        player->pad = pad;
        player->xpad = (player->pad ^ opad) & player->pad;

        ctrl = 0;
        if (pad & IN_LEFT)
            ctrl |= CTRL_LEFT;
        if (pad & IN_DOWN)
            ctrl |= CTRL_DOWN;
        if (pad & IN_RIGHT)
            ctrl |= CTRL_RIGHT;
        if (pad & IN_UP)
            ctrl |= CTRL_UP;
        if (pad & player->binding.thrust)
            ctrl |= CTRL_THRUST;
        if (pad & player->binding.fire)
            ctrl |= CTRL_FIRE;
        if (pad & player->binding.lBrake)
            ctrl |= CTRL_LBRAKE;
        if (pad & player->binding.rBrake)
            ctrl |= CTRL_RBRAKE;
        player->ctrl = ctrl;
    }
}

void SetBrake(ShipData* ship, short padLeft, short padRight)
{
    PlayerData* player = (PlayerData*)ship->player;
    ASSERT(player);

    // right
    if (player->pad & padRight)
        ship->brakeRotR += FR32U;
    else if (ship->brakeRotR > 0)
        ship->brakeRotR -= FR32U;

    if (ship->brakeRotR < 0)
        ship->brakeRotR = 0;

    if (ship->brakeRotR > 256)
        ship->brakeRotR = 256;

    // left
    if (player->pad & padLeft)
        ship->brakeRotL += FR32U;
    else if (ship->brakeRotL > 0)
        ship->brakeRotL -= FR32U;

    if (ship->brakeRotL < 0)
        ship->brakeRotL = 0;

    if (ship->brakeRotL > 256)
        ship->brakeRotL = 256;
}

void SetView(ShipData* ship, short padView)
{
    PlayerData* player = (PlayerData*)ship->player;
    ASSERT(player);

    if (player->xpad & padView)
    {
        if ((ship->attr & VIEW_IN) == 0)
        {
            player->camera.update = UpdateCameraRaceInternal;
            ship->attr |= VIEW_IN;
        }
        else
        {
            player->camera.update = UpdateCameraRaceExternal;
            ship->attr &= ~VIEW_IN;
        }
    }
}

void SetFire(ShipData* ship, short padFire, int shipNo)
{
    PlayerData* player = (PlayerData*)ship->player;
    ASSERT(player);

    if ((player->xpad & padFire) && (ship->weaponType != WPN_NONE))
    {
        if ((ship->attr & SHIELDED) == 0)
            FireWeapon(ship, shipNo, ship->weaponType);
        else
            PlayNote(SFX_MENU_MOVE);
    }
}

int32 GetMenuKeyPlayer(int32 key, int32 playerIndex)
{
    int32 xpad = gPlayers[playerIndex].xpad;

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
    return 0;
}

int32 GetMenuKey(int32 key)
{
    return GetMenuKeyPlayer(key, 0);
}

PlayerData* getPlayerByJoyIndex(int32 joyIndex)
{
    int32 i;

    if (joyIndex == -1)
        return gPlayers + 0;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;
        if (player->joyIndex == joyIndex)
            return player;
    }

    return NULL;
}

void padReset(int32 joyIndex)
{
    PlayerData* player = getPlayerByJoyIndex(joyIndex);
    if (!player)
        return;

    player->buttons = 0;
    player->Lx = player->Ly = 0;
    player->Rx = player->Ry = 0;
}

void padSetKey(int32 joyIndex, int32 inputKey, int32 down)
{
    PlayerData* player = getPlayerByJoyIndex(joyIndex);
    if (!player)
        return;

    if (down)
    {
        player->buttons |= inputKey;
    }
    else
    {
        player->buttons &= ~inputKey;
    }
}

void padSetStick(int32 joyIndex, int32 inputKey, int32 x, int32 y)
{
    PlayerData* player = getPlayerByJoyIndex(joyIndex);
    if (!player)
        return;

    if (inputKey == IN_L)
    {
        if (player->Lx != x)
        {
            if (x < 0)
            {
                player->buttons |= IN_LEFT;
                player->buttons &= ~IN_RIGHT;
            }
            else if (x > 0)
            {
                player->buttons &= ~IN_LEFT;
                player->buttons |= IN_RIGHT;
            }
            else
            {
                player->buttons &= ~(IN_LEFT | IN_RIGHT);
            }
        }

        if (player->Ly != y)
        {
            if (y < 0)
            {
                player->buttons |= IN_DOWN;
                player->buttons &= ~IN_UP;
            }
            else if (y > 0)
            {
                player->buttons &= ~IN_DOWN;
                player->buttons |= IN_UP;
            }
            else
            {
                player->buttons &= ~(IN_DOWN | IN_UP);
            }
        }

        player->Lx = x;
        player->Ly = y;
    }
    else if (inputKey == IN_R)
    {
        if (player->Rx != x)
        {
            if (x < -0x4000)
            {
                player->buttons |= IN_LT;
                player->buttons &= ~IN_RT;
            }
            else if (x > 0x4000)
            {
                player->buttons &= ~IN_LT;
                player->buttons |= IN_RT;
            }
            else
            {
                player->buttons &= ~(IN_LT | IN_RT);
            }
        }

        if (player->Ry != y)
        {
            if (y > 0)
            {
                player->buttons |= IN_A;
            }
            else
            {
                player->buttons &= ~IN_A;
            }
        }

        player->Rx = x;
        player->Ry = y;
    }
}