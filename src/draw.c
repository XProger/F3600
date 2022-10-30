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

void DrawLandscape(ShipData* playerShip, TrackCamera* camera)
{
#ifndef __3DO__ // TODO
    short i;
    vec3i result;
    long flag;
    TrackSection* section;
    long extent;
    vec3s centre;
    Object* obj;
    MATRIX* matrix;

    section = camera->section;

    if (section->objectCount > 0)
    {
        for (i = 0; i < section->objectCount; i++)
        {
            obj = objectTable[section->objects[i]];

            obj->skeleton->update = 1;
            matrix = GetSkeletonMatrix(obj->skeleton);
            extent = obj->extent;

            centre.x = 0;
            centre.y = 0;
            centre.z = 0;

            RotTrans(matrix, &centre, &result, &flag);

            result.z += extent;

            if (result.z == 0)
                result.z = 1;

            result.x = (result.x * 160) / result.z;
            result.y = (result.y * 160) / result.z;

            extent = (extent * 160) / result.z;
            /*TODO visibility*/
            if ((result.x >= -(extent + 160)) && (result.x <= (extent + 160)) && (result.y >= -(extent + 120)) && (result.y <= (extent + 120)))
            {
                renderObject(obj);
            }
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
