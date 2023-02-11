#include "game.h"

char TrackDirection(const TrackCamera* camera)
{
    int32 dx, dz;
    int32 angle;
    const TrackSection* nextSection;

    nextSection = camera->section->nextSection;

    dx = nextSection->centre.x - camera->section->centre.x;
    dz = nextSection->centre.z - camera->section->centre.z;

    angle = -(ratan2(dx, dz) + camera->hdg);
    angle = ang(angle);

    if ((angle >= -512) && (angle < 512))
    {
        return VIEW_DIR_NORTH;
    }
    else if ((angle >= 512) && (angle < 1536))
    {
        return VIEW_DIR_WEST;
    }
    else if ((angle >= 1536) || (angle < -1536))
    {
        return VIEW_DIR_SOUTH;
    }
    else
    {
        return VIEW_DIR_EAST;
    }
}

void DrawSky(void)
{
    matrixSetCameraSpace();
    renderSky(gTrack.sky);
}

void DrawLandscape(TrackSection* section)
{
    int32 i;
    int32 x, y, z, px, py;
    int32 extent;
    Object* obj;

    for (i = 0; i < section->objectsCount; i++)
    {
        obj = gTrack.objects + section->objects[i];

        // TODO check z first then x and y

        matrixTranslate(obj->pos.x, obj->pos.y, obj->pos.z);

        extent = obj->extent;

        // get object view-space coords
        x = gMatrixPtr->e03;
        y = gMatrixPtr->e13;
        z = gMatrixPtr->e23;

        px = z + (extent << 1);
        py = extent + ((z + extent) * 3 >> 2);

        if ((x >= -px) && (x <= px) && (y >= -py) && (y <= py))
        {
            renderObject(obj);
        }
    }
}

void DrawTrack(int32 playerIndex)
{
    matrixSetCameraSpace();
    renderTrack(playerIndex);
}

void DrawObject(Object* obj, int x, int y, int z, short hdg, short pitch, short roll)
{
    matrixPush();
    matrixTranslate(x, y, z);
    matrixRotateYXZ(pitch, hdg, roll);
    renderObject(obj);
    matrixPop();
}

void DrawView(int32 playerIndex)
{
    PlayerData* player = gPlayers + playerIndex;

    TrackCamera* cam = &player->camera;

    int32 px = cam->pos.x;
    int32 py = cam->pos.y;
    int32 pz = cam->pos.z;
    int32 rx = cam->pitch;
    int32 ry = cam->hdg;
    int32 rz = cam->roll;

    FRAME_LERP(cam);

    gCameraFOV = cam->fov;
    gCameraRotY = ry;

    if (ctrlmode != ATTRACT)
    {
        ShakeOffset(player->ship->shakeCount);
    }
    else
    {
        ShakeOffset(0);
    }

    renderSetMode(RENDER_MODE_3D);

    if (cam->update == UpdateCameraRaceInternal)
    {
        matrixSetViewInternal(px, py, pz, rx, ry, rz);
    }
    else
    {
        matrixSetViewExternal(px, py, pz, rx, ry);
    }

    DrawSky();
    DrawLandscape(player->camera.section);
    DrawTrack(playerIndex);
    DrawShadows(ctrlshipIndex);
    DrawShips(ctrlshipIndex, ctrlshipShapes);
    DrawDroid(ctrlshipShapes[RESCUE_DROID], &player->droid);
    DrawWeapons();
    DrawEffects();

    renderSetMode(RENDER_MODE_2D);

    if ((ctrlmode != ATTRACT) && player->ship->targetShip)
    {
        DrawTargetIcon(player->ship->targetShip);
    }

    if (player->ship->attr & RACING)
    {
        DrawHud(player->ship);
    }
}
