#include "game.h"

char TrackDirection(TrackCamera* camera)
{
    SVECTOR reference;
    long angle;
    TrackSection* nextSection;

    nextSection = camera->section->nextSection.ptr;

    reference.vx = (short)(nextSection->centre.vx - camera->section->centre.vx);
    reference.vy = (short)(nextSection->centre.vy - camera->section->centre.vy);
    reference.vz = (short)(nextSection->centre.vz - camera->section->centre.vz);

    angle = -ratan2(reference.vx, reference.vz) + camera->hdg;
    angle = ang(angle);

    if ((angle >= -512) && (angle < 512))
    {
        return (ShipForwards);
    }
    else if ((angle >= 512) && (angle < 1536))
    {
        return (ShipRight);
    }
    else if ((angle >= 1536) || (angle < -1536))
    {
        return (ShipBackwards);
    }
    else
    {
        return (ShipLeft);
    }
}

void DrawLandscape(ShipData* playerShip, TrackCamera* camera)
{
    short i;
    VECTOR result;
    long flag;
    TrackSection* section;
    long extent;
    SVECTOR centre;
    Object* obj; // = objectTable[0];

    section = camera->section;

    if (section->objectCount > 0)
    {
        for (i = 0; i < section->objectCount; i++)
        //        while (obj)
        {
            obj = objectTable[section->objects[i]];

            obj->skeleton->update = 1;
            SetSkeletonTransform(obj->skeleton);
            extent = obj->extent;

            centre.vx = 0;
            centre.vy = 0;
            centre.vz = 0;

            RotTrans(&centre, &result, &flag);

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

            // obj = obj->next;
        }
    }
}
