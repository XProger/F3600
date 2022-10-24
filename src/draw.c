#include "game.h"

char TrackDirection(const TrackCamera* camera)
{
    SVECTOR reference;
    long angle;
    const TrackSection* nextSection;

    nextSection = camera->section->nextSection;

    reference.vx = (short)(nextSection->centre.vx - camera->section->centre.vx);
    reference.vy = (short)(nextSection->centre.vy - camera->section->centre.vy);
    reference.vz = (short)(nextSection->centre.vz - camera->section->centre.vz);

    angle = -ratan2(reference.vx, reference.vz) + camera->hdg;
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
    VECTOR result;
    long flag;
    TrackSection* section;
    long extent;
    SVECTOR centre;
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

            centre.vx = 0;
            centre.vy = 0;
            centre.vz = 0;

            RotTrans(matrix, &centre, &result, &flag);

            result.vz += extent;

            if (result.vz == 0)
                result.vz = 1;

            result.vx = (result.vx * 160) / result.vz;
            result.vy = (result.vy * 160) / result.vz;

            extent = (extent * 160) / result.vz;
            /*TODO visibility*/
            if ((result.vx >= -(extent + 160)) && (result.vx <= (extent + 160)) && (result.vy >= -(extent + 120)) && (result.vy <= (extent + 120)))
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
    SetSkeletonPosition(obj->skeleton, shipData->pos.vx, shipData->pos.vy, shipData->pos.vz);
    SetSkeletonDirectionHPR(obj->skeleton, shipData->hdg, shipData->pitch, shipData->roll);
    renderShip(obj, shipData->color);
}
