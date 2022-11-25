/* (C) Psygnosis 1994, 1995. By Jason Carl Denton & Rob Smith
*/

#include "game.h"

#ifdef __3DO__
Object* LoadPrm(const char* name, int32 textureStart, int32* count)
{
    uint8* ptr;
    int32 objCount;
    int32 i, size;
    Object* obj;

    size = osLoadFile(name, pDRAM);
    ptr = (uint8*)allocDRAM(size);

    objCount = *(uint32*)ptr;
    ptr += 4;

    obj = (Object*)ptr;
    for (i = 0; i < objCount; i++, obj++)
    {
        obj->vertices = (vec3i*)(ptr + (uint32)obj->vertices);
        obj->faces = (ObjFace*)(ptr + (uint32)obj->faces);
        obj->sprites = (ObjSprite*)(ptr + (uint32)obj->sprites);
        obj->type = OBJ_TYPE_MAX; // TODO remove (use scene header)
    }

    if (count)
    {
        *count = objCount;
    }

    return obj - objCount;
}

/* TODO Scene WAD
void InitSceneVRAM(uint8* ptr)
{
    //
}

void InitSceneDRAM(uint8* ptr)
{
    //
}

void InitScene(int32 trackNum)
{
    char trackPath[64];
    int32 size;
    uint8* ptr;

    getPath(trackPath, trackNum, "/track.d");
    size = osLoadFile(trackPath, pDRAM);
    ptr = (uint8*)allocDRAM(size);
    InitSceneDRAM(ptr);

    getPath(trackPath, trackNum, "/track.v");
    size = osLoadFile(trackPath, pVRAM);
    ptr = (uint8*)allocVRAM(size);
    InitSceneVRAM(ptr);
}
*/
#else

#define CONV_COLOR(c) (c.r | (c.g << 8) | (c.b << 16) | 0xFF000000)

#define SET_FACE_IDX_T(f, i)        f->index[0] = i->coords[0]; f->index[1] = i->coords[1]; f->index[2] = i->coords[2]; f->index[3] = i->coords[2];
#define SET_FACE_IDX_Q(f, i)        f->index[0] = i->coords[0]; f->index[1] = i->coords[1]; f->index[2] = i->coords[3]; f->index[3] = i->coords[2];
#define SET_FACE_TEX_Z(f)           f->flags = NO_TEXTURE; memset(f->uv, 0, sizeof(f->uv))
#define SET_FACE_TEX_T(f, p)        f->flags = p->texture; f->uv[0][0] = p->u0; f->uv[0][1] = p->v0; f->uv[1][0] = p->u1; f->uv[1][1] = p->v1; f->uv[2][0] = p->u2; f->uv[2][1] = p->v2; f->uv[3][0] = f->uv[3][1] = 0
#define SET_FACE_TEX_Q(f, p)        f->flags = p->texture; f->uv[0][0] = p->u0; f->uv[0][1] = p->v0; f->uv[1][0] = p->u1; f->uv[1][1] = p->v1; f->uv[2][0] = p->u3; f->uv[2][1] = p->v3; f->uv[3][0] = p->u2; f->uv[3][1] = p->v2
#define SET_FACE_COLOR_FLAT(f, p)   f->color[0] = f->color[1] = f->color[2] = f->color[3] = CONV_COLOR(p->colour)
#define SET_FACE_COLOR_T(f, p)      f->color[0] = CONV_COLOR(p->colour[0]); f->color[1] = CONV_COLOR(p->colour[1]); f->color[2] = CONV_COLOR(p->colour[2]); f->color[3] = 0
#define SET_FACE_COLOR_Q(f, p)      f->color[0] = CONV_COLOR(p->colour[0]); f->color[1] = CONV_COLOR(p->colour[1]); f->color[2] = CONV_COLOR(p->colour[3]); f->color[3] = CONV_COLOR(p->colour[2])

#define SET_SPRITE_IDX(s, p)        s->index = p->coord
#define SET_SPRITE_TEX(s, p)        s->flags = p->texture
#define SET_SPRITE_SIZE(s, p);      s->width = p->width; s->height = p->height
#define SET_SPRITE_COLOR(s, p)      s->color = CONV_COLOR(p->colour)

Object* convertObjects(ObjectPSX* firstObject, int32* objCount)
{
    int32 i;
    int32 count = 0;
    ObjectPSX* obj;
    Object* retObj;
    int32 vCount = 0;
    int32 fCount = 0;
    int32 sCount = 0;
    PrmPSX prm;
    uint8* ptr;
    vec3i* v;
    ObjFace* f;
    ObjSprite* s;

    obj = firstObject;
    while (obj)
    {
        vCount += obj->vertexCount;

        prm.primitive = obj->primitives;
        for (i = 0; i < obj->primitiveCount; i++)
        {
            switch (prm.primitive->type)
            {
                case TypeF3:
                    prm.f3++;
                    fCount++;
                    break;

                case TypeF4:
                    prm.f4++;
                    fCount++;
                    break;

                case TypeFT3:
                    prm.ft3++;
                    fCount++;
                    break;

                case TypeFT4:
                    prm.ft4++;
                    fCount++;
                    break;

                case TypeG3:
                    prm.g3++;
                    fCount++;
                    break;

                case TypeG4:
                    prm.g4++;
                    fCount++;
                    break;

                case TypeGT3:
                    prm.gt3++;
                    fCount++;
                    break;

                case TypeGT4:
                    prm.gt4++;
                    fCount++;
                    break;

                case TypeTSPR:
                case TypeBSPR:
                    prm.spr++;
                    sCount++;
                    break;

                case TypeLSF3:
                    prm.lsf3++;
                    break;

                case TypeLSFT3:
                    prm.lsft3++;
                    break;

                case TypeLSF4:
                    prm.lsf4++;
                    break;

                case TypeLSFT4:
                    prm.lsft4++;
                    break;

                case TypeLSG3:
                    prm.lsg3++;
                    break;

                case TypeLSGT3:
                    prm.lsgt3++;
                    break;

                case TypeLSG4:
                    prm.lsg4++;
                    break;

                case TypeLSGT4:
                    prm.lsgt4++;
                    break;

                case TypeSpline:
                    prm.spline++;
                    break;

                case TypeInfiniteLight:
                case TypePointLight:
                case TypeSpotLight:
                    break;

                default:
                    ASSERT(0);
            }
        }

        count++;
        obj = obj->next;
    }

    *objCount = count;

    retObj = (Object*)allocDRAM(count * sizeof(Object) + vCount * sizeof(vec3i) + fCount * sizeof(ObjFace) + sCount * sizeof(ObjSprite));
    
    ptr = ((uint8*)retObj) + count * sizeof(Object);

    obj = firstObject;
    while (obj)
    {
        v = (vec3i*)ptr;
        retObj->vertices = v;

        //ASSERT(obj->vertexCount <= 255);
        for (i = 0; i < obj->vertexCount; i++)
        {
            v->x = obj->vertices[i].x;
            v->y = obj->vertices[i].y;
            v->z = obj->vertices[i].z;
            v++;
        }
        ptr = (uint8*)v;

        f = (ObjFace*)ptr;
        retObj->faces = f;

        prm.primitive = obj->primitives;
        for (i = 0; i < obj->primitiveCount; i++)
        {
            switch (prm.primitive->type)
            {
                case TypeF3:
                    SET_FACE_IDX_T(f, prm.f3);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_FLAT(f, prm.f3);
                    prm.f3++;
                    f++;
                    break;

                case TypeF4:
                    SET_FACE_IDX_Q(f, prm.f4);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_FLAT(f, prm.f4);
                    prm.f4++;
                    f++;
                    break;

                case TypeFT3:
                    SET_FACE_IDX_T(f, prm.ft3);
                    SET_FACE_TEX_T(f, prm.ft3);
                    SET_FACE_COLOR_FLAT(f, prm.ft3);

                    if (prm.ft3->flag & ShipEngine)
                    {
                        f->flags |= FACE_FLAG_ENGINE;
                    }

                    prm.ft3++;
                    f++;
                    break;

                case TypeFT4:
                    SET_FACE_IDX_Q(f, prm.ft4);
                    SET_FACE_TEX_Q(f, prm.ft4);
                    SET_FACE_COLOR_FLAT(f, prm.ft4);
                    prm.ft4++;
                    f++;
                    break;

                case TypeG3:
                    SET_FACE_IDX_T(f, prm.g3);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_T(f, prm.g3);
                    prm.g3++;
                    f++;
                    break;

                case TypeG4:
                    SET_FACE_IDX_Q(f, prm.g4);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_Q(f, prm.g4);
                    prm.g4++;
                    f++;
                    break;

                case TypeGT3:
                    SET_FACE_IDX_T(f, prm.gt3);
                    SET_FACE_TEX_T(f, prm.gt3);
                    SET_FACE_COLOR_T(f, prm.gt3);

                    if (prm.gt3->flag & ShipEngine)
                    {
                        f->flags |= FACE_FLAG_ENGINE;
                    }

                    prm.gt3++;
                    f++;
                    break;

                case TypeGT4:
                    SET_FACE_IDX_Q(f, prm.gt4);
                    SET_FACE_TEX_Q(f, prm.gt4);
                    SET_FACE_COLOR_Q(f, prm.gt4);
                    prm.gt4++;
                    f++;
                    break;

                case TypeTSPR:
                case TypeBSPR:
                    prm.spr++;
                    break;

                case TypeLSF3:
                    prm.lsf3++;
                    break;

                case TypeLSFT3:
                    prm.lsft3++;
                    break;

                case TypeLSF4:
                    prm.lsf4++;
                    break;

                case TypeLSFT4:
                    prm.lsft4++;
                    break;

                case TypeLSG3:
                    prm.lsg3++;
                    break;

                case TypeLSGT3:
                    prm.lsgt3++;
                    break;

                case TypeLSG4:
                    prm.lsg4++;
                    break;

                case TypeLSGT4:
                    prm.lsgt4++;
                    break;

                case TypeSpline:
                    prm.spline++;
                    break;

                case TypeInfiniteLight:
                case TypePointLight:
                case TypeSpotLight:
                    break;

                default:
                    ASSERT(0);
            }
        }
        ptr = (uint8*)f;

        s = (ObjSprite*)ptr;
        retObj->sprites = s;

        prm.primitive = obj->primitives;
        for (i = 0; i < obj->primitiveCount; i++)
        {
            switch (prm.primitive->type)
            {
                case TypeF3:
                    prm.f3++;
                    break;

                case TypeF4:
                    prm.f4++;
                    break;

                case TypeFT3:
                    prm.ft3++;
                    break;

                case TypeFT4:
                    prm.ft4++;
                    break;

                case TypeG3:
                    prm.g3++;
                    break;

                case TypeG4:
                    prm.g4++;
                    break;

                case TypeGT3:
                    prm.gt3++;
                    break;

                case TypeGT4:
                    prm.gt4++;
                    break;

                case TypeTSPR:
                case TypeBSPR:
                    SET_SPRITE_IDX(s, prm.spr);
                    SET_SPRITE_TEX(s, prm.spr);
                    SET_SPRITE_SIZE(s, prm.spr);
                    SET_SPRITE_COLOR(s, prm.spr);

                    if (prm.spr->type == TypeBSPR) // flipped
                    {
                        s->flags |= (1 << 31);
                    }

                    prm.spr++;
                    s++;
                    break;

                case TypeLSF3:
                    prm.lsf3++;
                    break;

                case TypeLSFT3:
                    prm.lsft3++;
                    break;

                case TypeLSF4:
                    prm.lsf4++;
                    break;

                case TypeLSFT4:
                    prm.lsft4++;
                    break;

                case TypeLSG3:
                    prm.lsg3++;
                    break;

                case TypeLSGT3:
                    prm.lsgt3++;
                    break;

                case TypeLSG4:
                    prm.lsg4++;
                    break;

                case TypeLSGT4:
                    prm.lsgt4++;
                    break;

                case TypeSpline:
                    prm.spline++;
                    break;

                case TypeInfiniteLight:
                case TypePointLight:
                case TypeSpotLight:
                    break;

                default:
                    ASSERT(0);
            }
        }
        ptr = (uint8*)s;

        if (!strncmp(obj->name, "start", 5))
            retObj->type = OBJ_TYPE_START;
        else if (!strncmp(obj->name, "lost", 4))
            retObj->type = OBJ_TYPE_LOST;
        else if (!strncmp(obj->name, "redl", 4))
            retObj->type = OBJ_TYPE_RED;
        else if (!strncmp(obj->name, "donkey", 6))
            retObj->type = OBJ_TYPE_DONKEY;
        else
            retObj->type = OBJ_TYPE_MAX;

        retObj->vCount = v - retObj->vertices;
        retObj->fCount = f - retObj->faces;
        retObj->sCount = s - retObj->sprites;

        retObj->pos.x = obj->skeleton->relative.t[0];
        retObj->pos.y = obj->skeleton->relative.t[1];
        retObj->pos.z = obj->skeleton->relative.t[2];
        retObj->extent = obj->extent;

        obj = obj->next;
        retObj++;
    }

    return retObj - count;
}

void IntelPrim(PrmPSX prm)
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
            IntelLong(&prm.spline->control1.x);
            IntelLong(&prm.spline->control1.y);
            IntelLong(&prm.spline->control1.z);
            IntelLong(&prm.spline->position.x);
            IntelLong(&prm.spline->position.y);
            IntelLong(&prm.spline->position.z);
            IntelLong(&prm.spline->control2.x);
            IntelLong(&prm.spline->control2.y);
            IntelLong(&prm.spline->control2.z);
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

Object* LoadPrm(const char* name, int32 textureStart, int32* count)
{
    Object* obj;
    ObjectPSX* object;
    ObjectPSX* lastObject;
    ObjectPSX* firstObject;
    int32 objCount;

    PrmPSX prm;

    int32 length;

    int32 i, j;

    uint8* start;
    uint8* finish;

    length = osLoadFile(name, pDRAM);

    start = (char*)malloc(length);
    finish = start + length;
    memcpy(start, pDRAM, length);

    prm.ptr = start;
    firstObject = prm.object;
    lastObject = NULL;

    while (prm.ptr < (start + length))
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
                ASSERT(object->skeleton->relative.m[i][j] == 0 || object->skeleton->relative.m[i][j] == 4096);
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
            IntelShort(&object->vertices[i].x);
            IntelShort(&object->vertices[i].y);
            IntelShort(&object->vertices[i].z);
        }

        /* Normals */

        object->normals = prm.svector;
        prm.svector += object->normalCount;

        for (i = 0; i < object->normalCount; i++)
        {
            IntelShort(&object->normals[i].x);
            IntelShort(&object->normals[i].y);
            IntelShort(&object->normals[i].z);
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

        if (lastObject)
        {
            lastObject->next = object;
        }

        lastObject = object;
    }

    obj = convertObjects(firstObject, &objCount);

    if (count)
    {
        *count = objCount;
    }

#ifdef _DEBUG
    convertObjects3DO(name, obj, objCount);
#endif

    free(start);

    return obj;
}
#endif
void SortObjects(void)
{
    int i, swapped;

    do
    {
        swapped = 0;
        for (i = 1; i < gTrack.objCount; i++)
        {
            if (gTrack.objects[i - 1].type > gTrack.objects[i].type)
            {
                Object t = gTrack.objects[i - 1];
                gTrack.objects[i - 1] = gTrack.objects[i];
                gTrack.objects[i] = t;
                swapped = 1;
            }
        }
    } while (swapped);
}

void InitObjects()
{
    int32 i;

    gTrack.startCount = 0;
    gTrack.lostCount = 0;
    gTrack.redCount = 0;
    gTrack.donkeyCount = 0;

    for (i = 0; i < gTrack.objCount; i++)
    {
        int32 type = gTrack.objects[i].type;

        if (type == OBJ_TYPE_START)
            gTrack.startCount++;
        else if (type == OBJ_TYPE_LOST)
            gTrack.lostCount++;
        else if (type == OBJ_TYPE_RED)
            gTrack.redCount++;
        else if (type == OBJ_TYPE_DONKEY)
            gTrack.donkeyCount++;
        else
            break;
    }

    gTrack.start = gTrack.objects;
    gTrack.lost = gTrack.start + gTrack.startCount;
    gTrack.red = gTrack.lost + gTrack.lostCount;
    gTrack.donkey = gTrack.red + gTrack.redCount;
}

WeaponGrid gWeaponGrid[64]; // TODO remove

void InitScene(int32 trackNum)
{
    int sceneTextures;
    int skyTextures;

    char trackPath[64];

    LoadTrack(trackNum);
    gTrack.weapons = gWeaponGrid;

    InitWeaponGrids();

    sceneTextures = gTexturePtr - gTextures;
    getPath(trackPath, trackNum, "/scene.cmp");
    LoadCompressedTextureSequence(trackPath, 1);

    skyTextures = gTexturePtr - gTextures;
    getPath(trackPath, trackNum, "/sky.cmp");
    LoadCompressedTextureSequence(trackPath, 1);

    getPath(trackPath, trackNum, "/scene.prm");

    gTrack.objects = LoadPrm(trackPath, sceneTextures, &gTrack.objCount);

    SortObjects();
    InitObjects();

    if (gTrack.redCount)
    {
        ASSERT((trackNo == 8) || (trackNo == 11) || (trackNo == 1));
    }

    getPath(trackPath, trackNum, "/sky.prm");
    gTrack.sky = LoadPrm(trackPath, skyTextures, NULL);

    SceneRadiusCheck(gTrack.objects, gTrack.objCount);

    if (trackNo == 13)
    {
        InitAuroraBorialis(gTrack.sky);
    }

    /* silverstream */
    if (trackNo == 13)
    {
        camSpline[4] = LoadPrm("wipeout/common/cam5s.prm", 0, NULL);
        shipSpline[4] = LoadPrm("wipeout/common/shp5s.prm", 0, NULL);

        camSpline[5] = LoadPrm("wipeout/common/cam6s.prm", 0, NULL);
        shipSpline[5] = LoadPrm("wipeout/common/shp6s.prm", 0, NULL);

        camSpline[6] = LoadPrm("wipeout/common/cam7s.prm", 0, NULL);
        shipSpline[6] = LoadPrm("wipeout/common/shp7s.prm", 0, NULL);
    }

    /* firestar */
    if (trackNo == 14)
    {
        camSpline[0] = LoadPrm("wipeout/common/cam1s.prm", 0, NULL);
        shipSpline[0] = LoadPrm("wipeout/common/shp1s.prm", 0, NULL);

        camSpline[1] = LoadPrm("wipeout/common/cam2s.prm", 0, NULL);
        shipSpline[1] = LoadPrm("wipeout/common/shp2s.prm", 0, NULL);

        camSpline[2] = LoadPrm("wipeout/common/cam3s.prm", 0, NULL);
        shipSpline[2] = LoadPrm("wipeout/common/shp3s.prm", 0, NULL);

        camSpline[3] = LoadPrm("wipeout/common/cam4s.prm", 0, NULL);
        shipSpline[3] = LoadPrm("wipeout/common/shp4s.prm", 0, NULL);
    }
}
