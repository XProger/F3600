#include "game.h"

char TrackDirection(const TrackCamera* camera)
{
    vec4s reference;
    long angle;
    const TrackSection* nextSection;

    nextSection = camera->section->nextSection;

    reference.x = (short)(nextSection->centre.x - camera->section->centre.x);
    reference.y = (short)(nextSection->centre.y - camera->section->centre.y);
    reference.z = (short)(nextSection->centre.z - camera->section->centre.z);

    angle = -ratan2(reference.x, reference.z) + camera->hdg;
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

void DrawLandscape(TrackCamera* camera)
{
#ifndef __3DO__ // TODO
    int32 i;
    int32 x, y, z, px, py;
    TrackSection* section;
    int32 extent;
    Object* obj;
    MATRIX* matrix;

    section = camera->section;

    for (i = 0; i < section->objectCount; i++)
    {
        obj = objectTable[section->objects[i]];

        obj->skeleton->update = 1;
        matrix = GetSkeletonMatrix(obj->skeleton);
        extent = obj->extent;

        // get object view-space coords
        x = matrix->t[0];
        y = matrix->t[1];
        z = matrix->t[2];

        px = z + (extent << 1);
        py = extent + ((z + extent) * 3 >> 2);

        if ((x >= -px) && (x <= px) && (y >= -py) && (y <= py))
        {
            renderObject(obj);
        }
    }
#endif
}

void DrawObject(Object* obj, int x, int y, int z, short hdg, short pitch, short roll)
{
    SetSkeletonPosition(obj->skeleton, x, y, z);
    SetSkeletonDirectionHPR(obj->skeleton, hdg, pitch, roll);
    renderObject(obj);
}

void DrawShip(Object* obj, const ShipData* shipData)
{
    SetSkeletonPosition(obj->skeleton, shipData->pos.x, shipData->pos.y, shipData->pos.z);
    SetSkeletonDirectionHPR(obj->skeleton, shipData->hdg, shipData->pitch, shipData->roll);
    renderShip(obj, shipData->color);
}
