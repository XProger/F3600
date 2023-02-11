#include "game.h"

short target = 0;

void ResetFOV(TrackCamera* camera)
{
    camera->fov = camera->fovTarget = CAMERA_FOV;
}

void InitCamera(TrackCamera* camera, TrackSection* section)
{
    camera->section = section;
    camera->pos.x = section->centre.x;
    camera->pos.y = section->centre.y;
    camera->pos.z = section->centre.z;
    camera->hdg = 0;
    camera->pitch = 0;
    camera->roll = 0;

    camera->vel.x = 0;
    camera->vel.y = 0;
    camera->vel.z = 0;

    camera->update = UpdateCameraRaceStart;

    ResetFOV(camera);
}

void UpdateFOV(TrackCamera* camera)
{
    if (camera->fovTarget > CAMERA_FOV)
    {
        camera->fovTarget -= (1 << 12);
    }
    camera->fov += (camera->fovTarget - camera->fov) >> 3;
}

void UpdateCameraRaceExternal(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    vec3i bestPath;
    vec3i distance;
    vec3i acc;
    int mag;
    static int vel = 0;

    TrackSection* nextSection;

    camera->pos.x = camShip->pos.x - (camShip->unitVecNose.x >> 2);
    camera->pos.y = camShip->pos.y - (camShip->unitVecNose.y >> 2) - 200;
    camera->pos.z = camShip->pos.z - (camShip->unitVecNose.z >> 2);

    camera->section = FindNearestSection(camera->section, &camera->pos);

    nextSection = camera->section->nextSection;

    GetTargetPos(&bestPath, &camera->pos, &nextSection->centre, &camera->section->centre);

    distance.x = camera->pos.x - bestPath.x;
    distance.y = camera->pos.y - bestPath.y;
    distance.z = camera->pos.z - bestPath.z;

    acc.x = distance.x;
    acc.y = distance.y;
    acc.z = distance.z;

    mag = GetMagnitude(&distance);

    vel += mag >> 7;
    vel -= vel >> 4;
    camera->pos.y -= vel;

    camera->vel.x -= acc.x >> 6;
    camera->vel.y -= acc.y >> 6;
    camera->vel.z -= acc.z >> 6;

    camera->vel.x -= camera->vel.x >> 3;
    camera->vel.y -= camera->vel.y >> 3;
    camera->vel.z -= camera->vel.z >> 3;

    camera->pos.x += camera->vel.x;
    camera->pos.y += camera->vel.y;
    camera->pos.z += camera->vel.z;

    camera->pitch = camShip->pitch;
    camera->hdg = camShip->hdg;

    UpdateFOV(camera);
}

void UpdateCameraRaceInternal(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    int32 sx, sy, sz;
    int32 cx, cy, cz;
    vec3i unitFinVector;

    camera->section = camShip->nearTrkSect;
    camera->pitch = camShip->pitch;
    camera->hdg = camShip->hdg;
    camera->roll = camShip->roll;

    rsincos(camera->pitch, sx, cx);
    rsincos(camera->hdg, sy, cy);
    rsincos(camera->roll, sz, cz);

    unitFinVector.x = ((cy * sz) - (((sy * sx) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;
    unitFinVector.y = (cx * cz) >> FIXED_SHIFT;
    unitFinVector.z = ((sy * sz) + (((cy * sx) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;

    camera->pos.x = camShip->pos.x - (unitFinVector.x >> 5);
    camera->pos.y = camShip->pos.y - (unitFinVector.y >> 5);
    camera->pos.z = camShip->pos.z - (unitFinVector.z >> 5);

    UpdateFOV(camera);
}

void UpdateCameraRaceStart(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    vec4i targetVector;
    int32 s;

    s = rsin((camShip->updateCount - SWITCH_TO_RACE_VIEW) * 3);

    camera->pos.x = camShip->pos.x - (camShip->unitVecNose.x >> 2) + s;
    camera->pos.y = camShip->pos.y - (camShip->unitVecNose.y >> 2) - ((2 * (camShip->updateCount - SWITCH_TO_RACE_VIEW)) + 200);
    camera->pos.z = camShip->pos.z - (camShip->unitVecNose.z >> 2) + s;

    if (camShip->updateCount == START_TIME)
    {
        camera->section = camShip->nearTrkSect;
    }
    else
    {
        camera->section = FindNearestSection(camera->section, &camera->pos);
    }

    targetVector.x = camShip->pos.x - camera->pos.x;
    targetVector.y = camShip->pos.y - camera->pos.y;
    targetVector.z = camShip->pos.z - camera->pos.z;

    camera->pitch = camShip->pitch >> 1;
    camera->hdg = -ratan2(targetVector.x, targetVector.z);

    if (camShip->updateCount == SWITCH_TO_RACE_VIEW)
    {
        /* TODO camera save default view */
        // camShip->attr |= VIEW_IN;
        // CameraUpdate = UpdateCameraRaceInternal;
        camShip->attr &= ~VIEW_IN;
        camera->update = UpdateCameraRaceExternal;
    }

    ResetFOV(camera);
}

void UpdateCameraDroidView(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    int temp;
    vec4i targetVector;

    camera->pos.x = camera->section->centre.x + 300;
    camera->pos.y = camera->section->centre.y - 1500;
    camera->pos.z = camera->section->centre.z + 300;

    targetVector.x = rescueDroid->pos.x - camera->pos.x;
    targetVector.y = rescueDroid->pos.y - camera->pos.y;
    targetVector.z = rescueDroid->pos.z - camera->pos.z;

    temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    camera->pitch = -ratan2(targetVector.y, temp);
    camera->hdg = -ratan2(targetVector.x, targetVector.z);

    ResetFOV(camera);
}

void UpdateCameraAttractCircle(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    int temp;
    vec4i targetVector;
    int32 sx, sy, sz, st;
    int32 cx, cy, cz, ct;
    vec4i unitFinVector;

    camera->section = camShip->nearTrkSect;

    if (target == 0)
    {
        target = 150;
    }
    target--;
    if (target == 0)
    {
        camera->update = UpdateCameraAttractMode;
    }

    rsincos(camShip->pitch, sx, cx);
    rsincos(camShip->hdg, sy, cy);
    rsincos(camShip->roll, sz, cz);
    rsincos(target << 3, st, ct);

    unitFinVector.x = ((cy * sz) - (((sy * sx) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;
    unitFinVector.y = (cx * cz) >> FIXED_SHIFT;
    unitFinVector.z = ((sy * sz) + (((cy * sx) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;

    camera->pos.x = camShip->pos.x + (sy / 6);
    camera->pos.y = camShip->pos.y + ((camShip->pitch >> 2) - 200);
    camera->pos.z = camShip->pos.z - (cy / 6);

    camera->pos.x += st >> 3;
    camera->pos.y -= 200;
    camera->pos.z += ct >> 3;

    camera->pos.x -= unitFinVector.x >> 4;
    camera->pos.y -= unitFinVector.y >> 4;
    camera->pos.z -= unitFinVector.z >> 4;

    targetVector.x = camShip->pos.x - camera->pos.x;
    targetVector.y = camShip->pos.y - camera->pos.y;
    targetVector.z = camShip->pos.z - camera->pos.z;

    temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    camera->pitch = -ratan2(targetVector.y, temp);
    camera->hdg = -ratan2(targetVector.x, targetVector.z);

    ResetFOV(camera);
}

void UpdateCameraStaticFollow(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    int temp;
    vec4i targetVector;
    TrackSection* section;
    int i;

    if (target == 0)
    {
        section = camShip->nearTrkSect->nextSection;
        for (i = 0; i < 10; i++)
        {
            section = section->nextSection;
        }

        camera->section = section;

        camera->pos.x = section->centre.x;
        camera->pos.y = section->centre.y - 500;
        camera->pos.z = section->centre.z;
        target = 150;
    }
    target--;
    if (target == 0)
    {
        camera->update = UpdateCameraAttractMode;
    }

    targetVector.x = camShip->pos.x - camera->pos.x;
    targetVector.y = camShip->pos.y - camera->pos.y;
    targetVector.z = camShip->pos.z - camera->pos.z;

    temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    camera->pitch = -ratan2(targetVector.y, temp);
    camera->hdg = -ratan2(targetVector.x, targetVector.z);

    ResetFOV(camera);
}

void UpdateCameraAttractMode(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    switch (rand() % 2)
    {
        case 0:
            camera->update = UpdateCameraAttractCircle;
            break;
        case 1:
            camera->update = UpdateCameraStaticFollow;
            break;
    }
    target = 0;
    camShip->attr &= ~VIEW_IN;
    (camera->update)(camera, camShip, rescueDroid);

    ResetFOV(camera);
}
