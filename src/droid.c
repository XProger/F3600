#include "game.h"

/**************************************************************************/
/****             Rescue droid control update routine                  ****/
/**************************************************************************/
void UpdateRescueDroid(ShipData* shipIndex, int shipNo)
{
    ShipData* ship;
    TrackSection* section;
    TrackSection* nextSection;
    static vec3i target;
    vec3i targetVector;
    vec3i distance;
    int i;

    ship = shipIndex + shipNo;
    ship->attr |= FLYING;

    if (ship->updateCount == 500)
    {
        section = ship->nearTrkSect->prevSection;
        section = section->prevSection;
        section = section->prevSection;
        section = section->prevSection;
        for (i = 0; i < 10; i++)
        {
            section = section->nextSection;
            if (section->flags & Jump)
            {
                break;
            }
        }
        ship->nearTrkSect = section;
        nextSection = section->nextSection;

        target.x = (section->centre.x + nextSection->centre.x) >> 1;
        target.y = ((section->centre.y + nextSection->centre.y) >> 1) - 2000;
        target.z = (section->centre.z + nextSection->centre.z) >> 1;

        ship->vel.x = 0;
        ship->vel.y = 0;
        ship->vel.z = 0;

        ship->updateCount--;
    }

    nextSection = ship->nearTrkSect->nextSection;

    if (ship->attr & IN_TOW)
    {
        target.x += (nextSection->centre.x - target.x) >> 7;
        target.y += (nextSection->centre.y - target.y) >> 7;
        target.z += (nextSection->centre.z - target.z) >> 7;

        ship->vel.x = target.x - ship->pos.x;
        ship->vel.y = target.y - ship->pos.y;
        ship->vel.z = target.z - ship->pos.z;

        targetVector.x = nextSection->centre.x - ship->nearTrkSect->centre.x;
        targetVector.y = nextSection->centre.y - ship->nearTrkSect->centre.y;
        targetVector.z = nextSection->centre.z - ship->nearTrkSect->centre.z;

        ship->vhdg = -ratan2(targetVector.x, targetVector.z) - ship->hdg;
        if (ship->vhdg > 2048)
            ship->vhdg = ship->vhdg - 4096;
        else if (ship->vhdg < -2048)
            ship->vhdg = ship->vhdg + 4096;
        ship->hdg += ship->vhdg >> 6;
        ship->hdg = ang(ship->hdg);
    }

    ship->vel.x -= ship->vel.x >> 4;
    ship->vel.y -= ship->vel.y >> 4;
    ship->vel.z -= ship->vel.z >> 4;

    ship->pos.x += ship->vel.x >> 5;
    ship->pos.y += ship->vel.y >> 5;
    ship->pos.z += ship->vel.z >> 5;

    distance.x = ship->pos.x - ship->nearTrkSect->centre.x;
    distance.y = ship->pos.y - ship->nearTrkSect->centre.y;
    distance.z = ship->pos.z - ship->nearTrkSect->centre.z;

    ship->pitch -= ship->pitch >> 3;
    ship->roll -= ship->roll >> 5;

    /**** Needs PAL ****/
    if ((ship->attr & IN_TOW) && (GetMagnitudeSqr(&distance) < (800 * 800)))
    {
        ship->update = UpdatePlayerShipNorm;
        ship->updateCount = 0;
        ship->attr &= ~RESCUE;
    }
}

/**************************************************************************/
/****             Rescue droid control update routine                  ****/
/**************************************************************************/
void UpdateDroidStart(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes)
{
    int32 cx, cy, sy;

    rescueDroid->updateCount--;

    if (rescueDroid->updateCount < 400)
    {
        cx = rcos(rescueDroid->pitch);
        rsincos(rescueDroid->hdg, sy, cy);

        rescueDroid->acc.x = (-sy * cx) >> 14;
        rescueDroid->acc.y = 0;
        rescueDroid->acc.z = (sy * cx) >> 14;

        rescueDroid->vhdg = 0;
    }
    else if (rescueDroid->updateCount < 710)
    {
        cx = rcos(rescueDroid->pitch);
        rsincos(rescueDroid->hdg, sy, cy);

        rescueDroid->acc.x = (-sy * cx) >> 15;
        rescueDroid->acc.y = -140;
        rescueDroid->acc.z = (cy * cx) >> 15;

        rescueDroid->vhdg = -8;
    }
    else if (rescueDroid->updateCount == 770)
    {
        rescueDroid->acc.y -= 90;
        rescueDroid->vhdg = 8;
    }

    SetDroidVectors(rescueDroid, shipShapes);

    if (rescueDroid->updateCount == 0)
    {
        rescueDroid->updateCount = 800;
        rescueDroid->droidUpdate = UpdateDroidCircle;
        rescueDroid->pos.x = rescueDroid->section->centre.x;
        rescueDroid->pos.y = -3000;
        rescueDroid->pos.z = rescueDroid->section->centre.z;
    }
}

void UpdateDroidCircle(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes)
{
    TrackSection* nextSection;
    vec4i targetVector;
    vec4i target;
    short targetHdg;
    short quickestTurn, turn;
    int32 cx, cy, sy;

    nextSection = rescueDroid->section->nextSection;

    target.x = (rescueDroid->section->centre.x + nextSection->centre.x) >> 1;
    target.y = rescueDroid->section->centre.y - 3000;
    target.z = (rescueDroid->section->centre.z + nextSection->centre.z) >> 1;

    targetVector.x = target.x - rescueDroid->pos.x;
    targetVector.y = target.y - rescueDroid->pos.y;
    targetVector.z = target.z - rescueDroid->pos.z;

    targetHdg = -ratan2(targetVector.x, targetVector.z);

    quickestTurn = targetHdg - rescueDroid->hdg;

    if (rescueDroid->hdg < 0)
        turn = targetHdg - (rescueDroid->hdg + 4096);
    else
        turn = targetHdg - (rescueDroid->hdg - 4096);

    if (abs(turn) < abs(quickestTurn))
    {
        rescueDroid->vhdg = turn >> 6;
    }
    else
    {
        rescueDroid->vhdg = quickestTurn >> 6;
    }

    cx = rcos(rescueDroid->pitch);
    rsincos(rescueDroid->hdg, sy, cy);

    rescueDroid->acc.x = (-sy * cx) >> 15;
    rescueDroid->acc.y = targetVector.y >> 6;
    rescueDroid->acc.z = (cy * cx) >> 15;

    if (shipIndex->attr & RESCUE)
    {
        PlayerData* player = (PlayerData*)shipIndex->player;
        ASSERT(player);

        PlayNote(SFX_TRACTOR_BEAM);
        rescueDroid->droidUpdate = UpdateDroidRescue;
        rescueDroid->updateCount = 800;

        shipIndex->attr |= REMOTE_VIEW;
        player->camera.update = UpdateCameraDroidView;
        if (shipIndex->nearTrkSect->flags & Jump)
            player->camera.section = shipIndex->nearTrkSect->nextSection;
        else
            player->camera.section = shipIndex->nearTrkSect;
    }

    SetDroidVectors(rescueDroid, shipShapes);
}

void UpdateDroidRescue(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes)
{
    vec3i target;
    vec3i distance;

    TrackSection* nextSection;

    if (rescueDroid->updateCount == 800)
    {

        if ((rescueDroid->section == shipIndex->nearTrkSect) || (rescueDroid->section == shipIndex->nearTrkSect->prevSection))
        {
            shipIndex->attr &= ~IN_TOW;
        }
        else
        {
            rescueDroid->section = shipIndex->nearTrkSect;
            nextSection = rescueDroid->section->nextSection;

            rescueDroid->pos.x = (rescueDroid->section->centre.x + nextSection->centre.x) >> 1;
            rescueDroid->pos.y = rescueDroid->section->centre.y - 3000;
            rescueDroid->pos.z = (rescueDroid->section->centre.z + nextSection->centre.z) >> 1;
            shipIndex->attr &= ~IN_TOW;
        }

        rescueDroid->vel.x = 0;
        rescueDroid->vel.y = 0;
        rescueDroid->vel.z = 0;

        rescueDroid->acc.x = 0;
        rescueDroid->acc.y = 0;
        rescueDroid->acc.z = 0;

        rescueDroid->updateCount = 799;
    }

    target.x = shipIndex->pos.x;
    target.y = shipIndex->pos.y - 350;
    target.z = shipIndex->pos.z;

    rescueDroid->vhdg = 0;
    rescueDroid->hdg = shipIndex->hdg;

    distance.x = (target.x - rescueDroid->pos.x);
    distance.y = (target.y - rescueDroid->pos.y);
    distance.z = (target.z - rescueDroid->pos.z);

    rescueDroid->vel.x = (distance.x) << 4;
    rescueDroid->vel.y = (distance.y) << 4;
    rescueDroid->vel.z = (distance.z) << 4;

    if ((rescueDroid->vel.x < 129) && (rescueDroid->vel.x > 0))
        rescueDroid->vel.x = 129;
    if ((rescueDroid->vel.y < 129) && (rescueDroid->vel.y > 0))
        rescueDroid->vel.y = 129;
    if ((rescueDroid->vel.z < 129) && (rescueDroid->vel.z > 0))
        rescueDroid->vel.z = 129;

    if ((rescueDroid->vel.x > -129) && (rescueDroid->vel.x < 0))
        rescueDroid->vel.x = -129;
    if ((rescueDroid->vel.y > -129) && (rescueDroid->vel.y < 0))
        rescueDroid->vel.y = -129;
    if ((rescueDroid->vel.z > -129) && (rescueDroid->vel.z < 0))
        rescueDroid->vel.z = -129;

    if (shipIndex->attr & IN_TOW)
    {

        rescueDroid->acc.x = 0;
        rescueDroid->acc.y = 0;
        rescueDroid->acc.z = 0;

        rescueDroid->vel.x = 0;
        rescueDroid->vel.y = 0;
        rescueDroid->vel.z = 0;

        rescueDroid->pos.x = target.x;
        rescueDroid->pos.y = target.y;
        rescueDroid->pos.z = target.z;
    }

    if ((GetMagnitudeSqr(&distance) < 1) && ((shipIndex->attr & IN_TOW) == 0))
    {
        shipIndex->attr |= IN_TOW;

        rescueDroid->acc.x = 0;
        rescueDroid->acc.y = 0;
        rescueDroid->acc.z = 0;

        rescueDroid->vel.x = 0;
        rescueDroid->vel.y = 0;
        rescueDroid->vel.z = 0;

        rescueDroid->pos.x = target.x;
        rescueDroid->pos.y = target.y;
        rescueDroid->pos.z = target.z;
    }

    if ((shipIndex->attr & RESCUE) == 0)
    {
        PlayerData* player = (PlayerData*)shipIndex->player;
        ASSERT(player);

        KillNote(SFX_TRACTOR_BEAM);
        rescueDroid->droidUpdate = UpdateDroidCircle;
        rescueDroid->updateCount = 800;
        shipIndex->attr &= ~REMOTE_VIEW;

        if (shipIndex->attr & VIEW_IN)
            player->camera.update = UpdateCameraRaceInternal;
        else
            player->camera.update = UpdateCameraRaceExternal;

        while ((rescueDroid->section->flags & Jump) == 0)
        {
            rescueDroid->section = rescueDroid->section->prevSection;
        }
    }

    SetDroidVectors(rescueDroid, shipShapes);
}

void SetDroidVectors(RescueData* rescueDroid, Object** shipShapes)
{
    rescueDroid->vel.x += rescueDroid->acc.x;
    rescueDroid->vel.y += rescueDroid->acc.y;
    rescueDroid->vel.z += rescueDroid->acc.z;

    rescueDroid->vel.x -= rescueDroid->vel.x >> 3;
    rescueDroid->vel.y -= rescueDroid->vel.y >> 3;
    rescueDroid->vel.z -= rescueDroid->vel.z >> 3;

    rescueDroid->pos.x += rescueDroid->vel.x >> VELOCITY_SHIFT;
    rescueDroid->pos.y += rescueDroid->vel.y >> VELOCITY_SHIFT;
    rescueDroid->pos.z += rescueDroid->vel.z >> VELOCITY_SHIFT;

    rescueDroid->hdg += rescueDroid->vhdg;
    rescueDroid->hdg = ang(rescueDroid->hdg);
}

void InitRescueDroid(RescueData* rescueDroid, Object** shipShapes, ShipData* shipIndex)
{
    rescueDroid->section = gTrack.sections;

    while (1)
    {
        rescueDroid->section = rescueDroid->section->nextSection;
        if (rescueDroid->section->flags & Jump)
            break;
    }

    rescueDroid->pos.x = shipIndex->pos.x;
    rescueDroid->pos.y = shipIndex->pos.y - 200;
    rescueDroid->pos.z = shipIndex->pos.z;

    rescueDroid->vel.x = 0;
    rescueDroid->vel.y = 0;
    rescueDroid->vel.z = 0;

    rescueDroid->acc.x = 0;
    rescueDroid->acc.y = 0;
    rescueDroid->acc.z = 0;

    rescueDroid->vhdg = 0;
    rescueDroid->vpitch = 0;
    rescueDroid->vroll = 0;

    rescueDroid->updateCount = 800;

    rescueDroid->modelID = RESCUE_DROID;

    rescueDroid->hdg = 0;
    rescueDroid->pitch = 0;
    rescueDroid->roll = 0;
    FRAME_LERP_INIT(rescueDroid);

    rescueDroid->droidUpdate = UpdateDroidStart;
}

void DrawDroid(Object* droid, const RescueData* rescueData)
{
#ifndef __3DO__ // TODO 3DO visibility check
    int32 px = rescueData->pos.x;
    int32 py = rescueData->pos.y;
    int32 pz = rescueData->pos.z;
    int32 rx = rescueData->pitch;
    int32 ry = rescueData->hdg;
    int32 rz = rescueData->roll;

    FRAME_LERP(rescueData);

    DrawObject(droid, px, py, pz, ry, rx, rz);
#endif
}
