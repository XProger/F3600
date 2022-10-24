#include "game.h"

short target = 0;

void InitCamera(TrackCamera* camera, TrackSection* section)
{
    int i;
    vec4i pos;

    camera->section = section;
    for (i = 0; i < 10; i++)
    {
        camera->section = camera->section->nextSection;
    }
    pos.x = camera->section->centre.x;
    pos.y = camera->section->centre.y;
    pos.z = camera->section->centre.z;
    SetSkeletonPosition(camera->camPos, -pos.x, -pos.y, -pos.z);
    camera->hdg = 0;
    camera->pitch = 0;
    camera->roll = 0;

    camera->vel.x = 0;
    camera->vel.y = 0;
    camera->vel.z = 0;
}

void UpdateCameraRaceExternal(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    vec4i pos;
    vec4i bestPath;
    vec4i distance;
    vec4i acc;
    int mag;
    static int vel = 0;

    TrackSection* nextSection;

    pos.x = camShip->pos.x - (camShip->unitVecNose.x >> 2);
    pos.y = camShip->pos.y - (camShip->unitVecNose.y >> 2);
    pos.z = camShip->pos.z - (camShip->unitVecNose.z >> 2);
    pos.y -= 200;

    camera->section = (TrackSection*)FindNearestSection(camera->section, pos);

    nextSection = camera->section->nextSection;

    GetTargetPos(&bestPath, &pos, &nextSection->centre, &camera->section->centre);

    distance.x = pos.x - bestPath.x;
    distance.y = pos.y - bestPath.y;
    distance.z = pos.z - bestPath.z;

    acc.x = distance.x;
    acc.y = distance.y;
    acc.z = distance.z;

    acc.x = (acc.x * 60) / FR60;
    acc.y = (acc.y * 60) / FR60;
    acc.z = (acc.z * 60) / FR60;

    camera->vel.x -= acc.x >> 6;
    camera->vel.y -= acc.y >> 6;
    camera->vel.z -= acc.z >> 6;

    camera->vel.x -= sar(camera->vel.x, 3);
    camera->vel.y -= sar(camera->vel.y, 3);
    camera->vel.z -= sar(camera->vel.z, 3);

    pos.x += camera->vel.x;
    pos.y += camera->vel.y;
    pos.z += camera->vel.z;

    mag = GetMagnitude(&distance);

    vel += (mag >> 7);
    vel -= vel >> 4;
    pos.y -= vel;

    SetSkeletonPosition(camera->camPos, -pos.x, -pos.y, -pos.z);

    camera->pitch = -camShip->pitch;
    camera->roll = 0;

    camera->hdg = -(camShip->hdg);
    SetSkeletonDirection(camera->camPos->super, camera->pitch, camera->hdg, 0);

    camera->camPos->super->update = 1;
}

void UpdateCameraRaceInternal(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    short sx, sy, sz;
    short cx, cy, cz;
    vec4i unitFinVector;

    camera->section = camShip->nearTrkSect;

    sy = rsin(camShip->hdg);
    sx = rsin(camShip->pitch);
    sz = rsin(camShip->roll);
    cy = rcos(camShip->hdg);
    cx = rcos(camShip->pitch);
    cz = rcos(camShip->roll);

    unitFinVector.x = ((cy * sz) >> 12) - ((((sy * sx) >> 12) * cz) >> 12);
    unitFinVector.y = (cx * cz) >> 12;
    unitFinVector.z = ((sy * sz) >> 12) + ((((cy * sx) >> 12) * cz) >> 12);

    SetSkeletonPosition(camera->camPos, -(camShip->pos.x - (unitFinVector.x >> 5)), -(camShip->pos.y - (unitFinVector.y >> 5)), -(camShip->pos.z - (unitFinVector.z >> 5)));

    camera->pitch = -camShip->pitch;
    camera->hdg = -camShip->hdg;
    camera->roll = -camShip->roll;
    SetSkeletonDirectionRPH(camera->camPos->super, -camShip->roll, -camShip->pitch, -camShip->hdg);

    camera->camPos->super->update = 1;
}

void UpdateCameraRaceStart(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    vec4i pos;
    vec4i targetVector;

    /*** Set to final position ***/
    pos.x = camShip->pos.x - (camShip->unitVecNose.x >> 2);
    pos.y = camShip->pos.y - (camShip->unitVecNose.y >> 2);
    pos.z = camShip->pos.z - (camShip->unitVecNose.z >> 2);
    pos.w = 0;

    pos.x += ((rsin((camShip->updateCount - SWITCH_TO_RACE_VIEW) * 3)));
    pos.y -= (2 * (camShip->updateCount - SWITCH_TO_RACE_VIEW)) + 200;
    pos.z += ((rsin((camShip->updateCount - SWITCH_TO_RACE_VIEW) * 3)));

    if (camShip->updateCount == START_TIME)
    {
        camera->section = camShip->nearTrkSect;
    }
    else
    {
        camera->section = (TrackSection*)FindNearestSection(camera->section, pos);
    }

    SetSkeletonPosition(camera->camPos, -pos.x, -pos.y, -pos.z);

    camera->pitch = -sar(camShip->pitch, 1);

    targetVector.x = camShip->pos.x - pos.x;
    targetVector.y = camShip->pos.y - pos.y;
    targetVector.z = camShip->pos.z - pos.z;

    camera->hdg = ratan2(targetVector.x, targetVector.z);

    SetSkeletonDirection(camera->camPos->super, camera->pitch, camera->hdg, 0);

    camera->camPos->super->update = 1;

    if (camShip->updateCount == SWITCH_TO_RACE_VIEW)
    {
        /* TODO camera save default view */
        // camShip->attr |= VIEW_IN;
        // CameraUpdate = UpdateCameraRaceInternal;
        camShip->attr &= ~VIEW_IN;
        CameraUpdate = UpdateCameraRaceExternal;
    }
}

void UpdateCameraAttractCircle(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    vec4i pos;
    int temp;
    vec4i targetVector;
    short sx, sy, sz;
    short cx, cy, cz;
    vec4i unitFinVector;

    camera->section = camShip->nearTrkSect;

    if (target == 0)
    {
        target = 150;
    }
    target--;
    if (target == 0)
    {
        CameraUpdate = UpdateCameraAttractMode;
    }

    sy = rsin(camShip->hdg);
    sx = rsin(camShip->pitch);
    sz = rsin(camShip->roll);
    cy = rcos(camShip->hdg);
    cx = rcos(camShip->pitch);
    cz = rcos(camShip->roll);

    unitFinVector.x = ((cy * sz) >> 12) - ((((sy * sx) >> 12) * cz) >> 12);
    unitFinVector.y = (cx * cz) >> 12;
    unitFinVector.z = ((sy * sz) >> 12) + ((((cy * sx) >> 12) * cz) >> 12);

    pos.x = camShip->pos.x + (rsin(camShip->hdg) / 6);
    pos.y = camShip->pos.y + ((camShip->pitch >> 2) - 200);
    pos.z = camShip->pos.z - (rcos(camShip->hdg) / 6);

    pos.x += rsin(target << 3) >> 3;
    pos.y -= 200;
    pos.z += rcos(target << 3) >> 3;

    pos.x -= unitFinVector.x >> 4;
    pos.y -= unitFinVector.y >> 4;
    pos.z -= unitFinVector.z >> 4;

    targetVector.x = camShip->pos.x - pos.x;
    targetVector.y = camShip->pos.y - pos.y;
    targetVector.z = camShip->pos.z - pos.z;

    temp = SquareRoot0(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    SetSkeletonPosition(camera->camPos, -pos.x, -pos.y, -pos.z);

    camera->hdg = ratan2(targetVector.x, targetVector.z);
    camera->pitch = ratan2(targetVector.y, temp);
    SetSkeletonDirection(camera->camPos->super, camera->pitch, camera->hdg, 0);

    camera->camPos->super->update = 1;
}

void UpdateCameraDroidView(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    vec4i pos;
    int temp;
    vec4i targetVector;
    vec4i av;

    pos.x = camera->section->centre.x + 300;
    pos.y = camera->section->centre.y - 1500;
    pos.z = camera->section->centre.z + 300;

    av.x = rescueDroid->pos.x;
    av.y = rescueDroid->pos.y;
    av.z = rescueDroid->pos.z;

    targetVector.x = av.x - pos.x;
    targetVector.y = av.y - pos.y;
    targetVector.z = av.z - pos.z;

    temp = SquareRoot0(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    SetSkeletonPosition(camera->camPos, -pos.x, -pos.y, -pos.z);

    camera->pitch = ratan2(targetVector.y, temp);
    camera->hdg = ratan2(targetVector.x, targetVector.z);
    SetSkeletonDirection(camera->camPos->super, camera->pitch, camera->hdg, 0);

    camera->camPos->super->update = 1;
}

void UpdateCameraStaticFollow(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    static vec4i pos;
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

        pos.x = section->centre.x;
        pos.y = section->centre.y - 500;
        pos.z = section->centre.z;
        target = 150;
    }
    target--;
    if (target == 0)
    {
        CameraUpdate = UpdateCameraAttractMode;
    }
    targetVector.x = camShip->pos.x - pos.x;
    targetVector.y = camShip->pos.y - pos.y;
    targetVector.z = camShip->pos.z - pos.z;

    temp = SquareRoot0(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    SetSkeletonPosition(camera->camPos, -pos.x, -pos.y, -pos.z);

    camera->hdg = ratan2(targetVector.x, targetVector.z);
    camera->pitch = ratan2(targetVector.y, temp);
    SetSkeletonDirection(camera->camPos->super, camera->pitch, camera->hdg, 0);

    camera->camPos->super->update = 1;
}

void UpdateCameraAttractMode(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    switch (rand() % 2)
    {
        case 0:
            CameraUpdate = UpdateCameraAttractCircle;
            camShip->attr &= ~VIEW_IN;
            target = 0;
            break;

        case 1:
            CameraUpdate = UpdateCameraStaticFollow;
            camShip->attr &= ~VIEW_IN;
            target = 0;
            break;
    }
    (CameraUpdate)(camera, camShip, rescueDroid);
}
