/* (C) Psygnosis 1994, 1995. By Jason Carl Denton & Rob Smith
*/

#include "game.h"

void IntelPrim(Prm prm)
{
    IntelShort((short*)&prm.f3->type);
    IntelShort((short*)&prm.f3->flag);

    switch (prm.f3->type)
    {
        case TypeF3:
            IntelShort(&prm.f3->coords[0]);
            IntelShort(&prm.f3->coords[1]);
            IntelShort(&prm.f3->coords[2]);
            break;

        case TypeF4:
            IntelShort(&prm.f4->coords[0]);
            IntelShort(&prm.f4->coords[1]);
            IntelShort(&prm.f4->coords[2]);
            IntelShort(&prm.f4->coords[3]);
            break;

        case TypeFT3:
            IntelShort(&prm.ft3->coords[0]);
            IntelShort(&prm.ft3->coords[1]);
            IntelShort(&prm.ft3->coords[2]);

            IntelShort(&prm.ft3->texture);
            break;

        case TypeFT4:
            IntelShort(&prm.ft4->coords[0]);
            IntelShort(&prm.ft4->coords[1]);
            IntelShort(&prm.ft4->coords[2]);
            IntelShort(&prm.ft4->coords[3]);

            IntelShort(&prm.ft4->texture);
            break;

        case TypeG3:
            IntelShort(&prm.g3->coords[0]);
            IntelShort(&prm.g3->coords[1]);
            IntelShort(&prm.g3->coords[2]);
            break;

        case TypeG4:
            IntelShort(&prm.g4->coords[0]);
            IntelShort(&prm.g4->coords[1]);
            IntelShort(&prm.g4->coords[2]);
            IntelShort(&prm.g4->coords[3]);
            break;

        case TypeGT3:
            IntelShort(&prm.gt3->coords[0]);
            IntelShort(&prm.gt3->coords[1]);
            IntelShort(&prm.gt3->coords[2]);

            IntelShort(&prm.gt3->texture);
            break;

        case TypeGT4:
            IntelShort(&prm.gt4->coords[0]);
            IntelShort(&prm.gt4->coords[1]);
            IntelShort(&prm.gt4->coords[2]);
            IntelShort(&prm.gt4->coords[3]);

            IntelShort(&prm.gt4->texture);
            break;

        case TypeTSPR:
        case TypeBSPR:
            IntelShort(&prm.spr->coord);
            IntelShort(&prm.spr->width);
            IntelShort(&prm.spr->height);
            IntelShort(&prm.spr->texture);
            break;

        case TypeSpline:
            IntelLong(&prm.spline->control1.vx);
            IntelLong(&prm.spline->control1.vy);
            IntelLong(&prm.spline->control1.vz);
            IntelLong(&prm.spline->position.vx);
            IntelLong(&prm.spline->position.vy);
            IntelLong(&prm.spline->position.vz);
            IntelLong(&prm.spline->control2.vx);
            IntelLong(&prm.spline->control2.vy);
            IntelLong(&prm.spline->control2.vz);
            break;

        default:
            ASSERT(0);
            break;
    }
}

void IntelLong(long* lvalue)
{
    char* cvalue;
    char temp;

    cvalue = (char*)lvalue;

    temp = cvalue[0];
    cvalue[0] = cvalue[3];
    cvalue[3] = temp;

    temp = cvalue[1];
    cvalue[1] = cvalue[2];
    cvalue[2] = temp;
}

void IntelShort(short* svalue)
{
    char* cvalue;
    char temp;

    cvalue = (char*)svalue;

    temp = cvalue[0];
    cvalue[0] = cvalue[1];
    cvalue[1] = temp;
}

Object* LoadPrm(char* name, short textureStart)
{
    Object* object;
    Object* lastObject;
    Object* firstObject;

    Prm prm;

    long length;

    short i, j;

    char* start;
    char* finish;

    length = FileLength(name);

    if (length <= 0)
    {
        ASSERT(0);
    }

    start = malloc(length + 32);
    finish = start + length;

    if (!start)
    {
        /* Memory Allocation Failed! */
        ASSERT(0);
    }

    if (LoadFile(name, start) != length)
    {
        /* File Error! */
        ASSERT(0);
    }

    prm.ptr = start;
    firstObject = prm.object;
    lastObject = NULL;

    while (prm.ptr < finish)
    {
        /* Object */

        object = prm.object++;

        IntelShort(&object->vertexCount);
        IntelShort(&object->normalCount);
        IntelShort(&object->primitiveCount);
        IntelLong(&object->extent);

        /* Skeleton */

        object->skeleton = prm.skeleton++;

        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 3; j++)
            {
                IntelShort(&object->skeleton->relative.m[i][j]);
            }

            IntelLong(&object->skeleton->relative.t[i]);
            IntelLong(&object->skeleton->absolute.t[i]);
        }

        IntelShort(&object->skeleton->update);

        /* Next */

        object->next = NULL;

        /* Vertices */

        object->vertices = prm.svector;
        prm.svector += object->vertexCount;

        for (i = 0; i < object->vertexCount; i++)
        {
            IntelShort(&object->vertices[i].vx);
            IntelShort(&object->vertices[i].vy);
            IntelShort(&object->vertices[i].vz);
        }

        /* Normals */

        object->normals = prm.svector;
        prm.svector += object->normalCount;

        for (i = 0; i < object->normalCount; i++)
        {
            IntelShort(&object->normals[i].vx);
            IntelShort(&object->normals[i].vy);
            IntelShort(&object->normals[i].vz);
        }

        /* Primitives */

        object->primitives = prm.primitive;

        for (i = 0; i < object->primitiveCount; i++)
        {
            IntelPrim(prm);

            switch (prm.f3->type)
            {
                case TypeF3:
                    prm.f3 += 1;
                    break;

                case TypeF4:
                    prm.f4 += 1;
                    break;

                case TypeFT3:
                    prm.ft3->texture += textureStart;
                    prm.ft3 += 1;
                    break;

                case TypeFT4:
                    prm.ft4->texture += textureStart;
                    prm.ft4 += 1;
                    break;

                case TypeG3:
                    prm.g3 += 1;
                    break;

                case TypeG4:
                    prm.g4 += 1;
                    break;

                case TypeGT3:
                    prm.gt3->texture += textureStart;
                    prm.gt3 += 1;
                    break;

                case TypeGT4:
                    prm.gt4->texture += textureStart;
                    prm.gt4 += 1;
                    break;

                case TypeTSPR:
                case TypeBSPR:
                    prm.spr->texture += textureStart;
                    prm.spr += 1;
                    break;

                default:
                    ASSERT(0);
                    break;
            }
        }

        /* Bodged check for rubbish at end of file ( prm.ptr <= finish ) */

        if (lastObject)
        {
            lastObject->next = object;
        }

        lastObject = object;
    }

    return (firstObject);
}
