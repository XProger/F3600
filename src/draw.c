#include "game.h"

const int32 gSkyPos[15] = { -820, -2520, -1930, -5000, -5000, 0, -2260, -40, -2700, 0, -240, -2120, -2700, 0, 0 };

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

void DrawSky(Object* sky)
{
    TrackCamera* cam = &gCamera;
    int32 px = FRAME_LERP_POS(cam->_pos.x, cam->pos.x);
    int32 py = FRAME_LERP_POS(cam->_pos.y, cam->pos.y);
    int32 pz = FRAME_LERP_POS(cam->_pos.z, cam->pos.z);

    matrixTranslate(px, py + gSkyPos[trackNo - 1], pz);
    renderSky(sky);
}

void DrawLandscape(TrackCamera* camera)
{
    int32 i;
    int32 x, y, z, px, py;
    TrackSection* section;
    int32 extent;
    Object* obj;

    section = camera->section;

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

void DrawTrack(void)
{
    matrixTranslate(0, 0, 0);
    renderTrack();
}

void DrawObject(Object* obj, int x, int y, int z, short hdg, short pitch, short roll)
{
    matrixPush();
    matrixTranslate(x, y, z);
    matrixRotateYXZ(pitch, hdg, roll);
    renderObject(obj);
    matrixPop();
}
