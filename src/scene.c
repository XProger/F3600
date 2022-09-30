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

        case TypeLSF3:
            IntelShort(&prm.lsf3->coords[0]);
            IntelShort(&prm.lsf3->coords[1]);
            IntelShort(&prm.lsf3->coords[2]);
            IntelShort(&prm.lsf3->normal);
            break;

        case TypeLSFT3:
            IntelShort(&prm.lsft3->coords[0]);
            IntelShort(&prm.lsft3->coords[1]);
            IntelShort(&prm.lsft3->coords[2]);
            IntelShort(&prm.lsft3->normal);
            IntelShort(&prm.lsft3->texture);
            break;

        case TypeLSF4:
            IntelShort(&prm.lsf4->coords[0]);
            IntelShort(&prm.lsf4->coords[1]);
            IntelShort(&prm.lsf4->coords[2]);
            IntelShort(&prm.lsf4->coords[3]);
            IntelShort(&prm.lsf4->normal);
            break;

        case TypeLSFT4:
            IntelShort(&prm.lsft4->coords[0]);
            IntelShort(&prm.lsft4->coords[1]);
            IntelShort(&prm.lsft4->coords[2]);
            IntelShort(&prm.lsft4->coords[3]);
            IntelShort(&prm.lsft4->normal);
            IntelShort(&prm.lsft4->texture);
            break;

        case TypeLSG3:
            IntelShort(&prm.lsg3->coords[0]);
            IntelShort(&prm.lsg3->coords[1]);
            IntelShort(&prm.lsg3->coords[2]);
            IntelShort(&prm.lsg3->normals[0]);
            IntelShort(&prm.lsg3->normals[1]);
            IntelShort(&prm.lsg3->normals[2]);
            break;

        case TypeLSGT3:
            IntelShort(&prm.lsgt3->coords[0]);
            IntelShort(&prm.lsgt3->coords[1]);
            IntelShort(&prm.lsgt3->coords[2]);
            IntelShort(&prm.lsgt3->normals[0]);
            IntelShort(&prm.lsgt3->normals[1]);
            IntelShort(&prm.lsgt3->normals[2]);
            IntelShort(&prm.lsgt3->texture);
            break;

        case TypeLSG4:
            IntelShort(&prm.lsg4->coords[0]);
            IntelShort(&prm.lsg4->coords[1]);
            IntelShort(&prm.lsg4->coords[2]);
            IntelShort(&prm.lsg4->coords[3]);
            IntelShort(&prm.lsg4->normals[0]);
            IntelShort(&prm.lsg4->normals[1]);
            IntelShort(&prm.lsg4->normals[2]);
            IntelShort(&prm.lsg4->normals[3]);
            break;

        case TypeLSGT4:
            IntelShort(&prm.lsgt4->coords[0]);
            IntelShort(&prm.lsgt4->coords[1]);
            IntelShort(&prm.lsgt4->coords[2]);
            IntelShort(&prm.lsgt4->coords[3]);
            IntelShort(&prm.lsgt4->normals[0]);
            IntelShort(&prm.lsgt4->normals[1]);
            IntelShort(&prm.lsgt4->normals[2]);
            IntelShort(&prm.lsgt4->normals[3]);
            IntelShort(&prm.lsgt4->texture);
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

        case TypeInfiniteLight:
            // skip
            break;

        case TypePointLight:
            // skip
            break;

        case TypeSpotLight:
            // skip
            break;

        default:
            ASSERT(0);
            break;
    }
}

void IntelLong(long* lvalue)
{
#ifndef CPU_BIG_ENDIAN
    char* cvalue;
    char temp;

    ASSERT(((int32)lvalue & 3) == 0);
    cvalue = (char*)lvalue;

    temp = cvalue[0];
    cvalue[0] = cvalue[3];
    cvalue[3] = temp;

    temp = cvalue[1];
    cvalue[1] = cvalue[2];
    cvalue[2] = temp;
#endif
}

void IntelShort(short* svalue)
{
#ifndef CPU_BIG_ENDIAN
    char* cvalue;
    char temp;

    ASSERT(((int16)svalue & 1) == 0);
    cvalue = (char*)svalue;

    temp = cvalue[0];
    cvalue[0] = cvalue[1];
    cvalue[1] = temp;
#endif
}

Object* LoadPrm(const char* name, short textureStart)
{
    Object* object;
    Object* lastObject;
    Object* firstObject;

    Prm prm;

    long length;

    short i, j;

    char* start;
    char* finish;

    length = osLoadFile(name, NULL);

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

    if (osLoadFile(name, start) != length)
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
                    prm.f3++;
                    break;

                case TypeF4:
                    prm.f4++;
                    break;

                case TypeFT3:
                    prm.ft3->texture += textureStart;
                    prm.ft3++;
                    break;

                case TypeFT4:
                    prm.ft4->texture += textureStart;
                    prm.ft4++;
                    break;

                case TypeG3:
                    prm.g3++;
                    break;

                case TypeG4:
                    prm.g4++;
                    break;

                case TypeGT3:
                    prm.gt3->texture += textureStart;
                    prm.gt3++;
                    break;

                case TypeGT4:
                    prm.gt4->texture += textureStart;
                    prm.gt4++;
                    break;

                case TypeTSPR:
                case TypeBSPR:
                    prm.spr->texture += textureStart;
                    prm.spr++;
                    break;

                case TypeLSF3:
                    prm.lsf3++;
                    break;

                case TypeLSFT3:
                    prm.lsft3->texture += textureStart;
                    prm.lsft3++;
                    break;

                case TypeLSF4:
                    prm.lsf4++;
                    break;

                case TypeLSFT4:
                    prm.lsft4->texture += textureStart;
                    prm.lsft4++;
                    break;

                case TypeLSG3:
                    prm.lsg3++;
                    break;

                case TypeLSGT3:
                    prm.lsgt3->texture += textureStart;
                    prm.lsgt3++;
                    break;

                case TypeLSG4:
                    prm.lsg4++;
                    break;

                case TypeLSGT4:
                    prm.lsgt4->texture += textureStart;
                    prm.lsgt4++;
                    break;

                case TypeSpline:
                    prm.spline++;
                    break;

                case TypeInfiniteLight:
                    prm.infiniteLight++;
                    break;

                case TypePointLight:
                    prm.pointLight++;
                    break;

                case TypeSpotLight:
                    prm.spotLight++;
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
