#include "game.h"

/*
void convertTim3DO(FILE* f, Tim* tim)
{
    int32 i, j, w, h, iw;
    uint32 info;
    uint8 zero = 0;

    if (TimType(tim) == TIM_4)
    {
        Tim4* t = &tim->tim4;
        uint8* src = (uint8*)(t + 1);

        w = t->textureW * 4;
        h = t->textureH;

        iw = (w + 7) & ~7;

        info = INIT_TEX_INFO(iw, h, TEX_TYPE_4);

        IntelLong(&info);
        fwrite(&info, sizeof(info), 1, f);

        for (i = 0; i < 16; i++)
        {
            uint16 p = t->clut[i];
            uint8 r = p & 31;
            uint8 g = (p >> 5) & 31;
            uint8 b = (p >> 10) & 31;
            p = b | (g << 5) | (r << 10);
            IntelShort(&p);
            fwrite(&p, sizeof(p), 1, f);
        }

        for (i = 0; i < h; i++)
        {
            uint8 index;

            for (j = 0; j < (w >> 1); j++)
            {
                index = *src++;
                index = ((index & 15) << 4) | (index >> 4);
                fwrite(&index, sizeof(index), 1, f);
            }

            index = 0;
            for (j = (w >> 1); j < (iw >> 1); j++)
            {
                fwrite(&index, sizeof(index), 1, f);
            }
        }
    }
    else if (TimType(tim) == TIM_8) // TODO 8-bpp textures for 3DO
    {
        Tim8* t = &tim->tim8;
        uint8* src = (uint8*)(t + 1);

        w = t->textureW * 2;
        h = t->textureH;

        iw = (w + 1) & ~1;

        info = INIT_TEX_INFO(iw, h, TEX_TYPE_16);
        IntelLong(&info);
        fwrite(&info, sizeof(info), 1, f);

        for (i = 0; i < h; i++)
        {
            uint16 p;

            for (j = 0; j < w; j++)
            {
                p = t->clut[*src++];
                uint8 r = p & 31;
                uint8 g = (p >> 5) & 31;
                uint8 b = (p >> 10) & 31;
                p = b | (g << 5) | (r << 10);
                IntelShort(&p);
                fwrite(&p, sizeof(p), 1, f);
            }

            p = 0;
            for (j = w; j < iw; j++)
            {
                fwrite(&p, sizeof(p), 1, f);
            }
        }
    }
    else if (TimType(tim) == TIM_16)
    {
        Tim16* t = &tim->tim16;
        uint16* src = (uint16*)(t + 1);

        w = t->textureW;
        h = t->textureH;

        iw = (w + 1) & ~1;

        info = INIT_TEX_INFO(iw, h, TEX_TYPE_16);
        IntelLong(&info);
        fwrite(&info, sizeof(info), 1, f);

        for (i = 0; i < h; i++)
        {
            uint16 p;

            for (j = 0; j < w; j++)
            {
                p = *src++;
                uint8 r = p & 31;
                uint8 g = (p >> 5) & 31;
                uint8 b = (p >> 10) & 31;
                p = b | (g << 5) | (r << 10);
                IntelShort(&p);
                fwrite(&p, sizeof(p), 1, f);
            }

            p = 0;
            for (j = w; j < iw; j++)
            {
                fwrite(&p, sizeof(p), 1, f);
            }
        }
    }
    else
    {
        ASSERT(0);
    }
}

void convertCmp3DO(const char* fileName, int32 numTex, uint8 compBuffer)
{
    int32 i;
    FILE* f;
    char path[256];
    int32 numTexBE = numTex;

    strcpy(path, "../src/platform/3do/CD/");
    strcat(path, fileName);

    f = fopen(path, "wb");

    IntelLong(&numTexBE);
    fwrite(&numTexBE, 1, 4, f);

    timTotalSize = 0;
    for (i = 0; i < numTex; i++)
    {
        convertTim3DO(f, (Tim*)(pVRAM + timTotalSize));

        timSize = *(int*)(compBuffer + 4 + i * 4);
        SwapLE32(timSize);
        timTotalSize += timSize;
    }

    fclose(f);
}
*/

#ifdef _DEBUG
typedef struct TrackHeader3DO
{
    int32 vertexCount;
    int32 faceCount;
    int32 sectionCount;
    int32 plutCount;
} TrackHeader3DO;

typedef struct TrackSection3DO
{
    uint32 junction;
    uint32 prevSection;
    uint32 nextSection;

    vec3i centre;

    uint16 faceStart;
    uint16 faceCount;

    uint32 objects;
    uint32 objectsCount;

    uint16 globalRadius;
    uint16 localRadius;

    uint16 flags;
    uint16 secNo; // 0

    TrackRange visRange[2][2];
} TrackSection3DO;

#define MAX_TRACK_CLUTS 8192

int32 clutsCount;
uint16 cluts[MAX_TRACK_CLUTS][16];
uint16 faceCLUT[4096][5];

vec3i trackVertex[4096];
int32 trackVertexCount;

Face trackFaces[4096];
int32 trackFaceCount;

int32 clutAdd(int32 faceTexIndex, float r, float g, float b)
{
#if 0
    int32 i, pr, pg, pb;
    uint16 clut[16];
    Tim* tim = gTextures[faceTexIndex + ctrllibraryTextures].tim;

    ASSERT(TimType(tim) == TIM_4);

    for (i = 0; i < 16; i++)
    {
        uint16 p = tim->tim4.clut[i];

        pr = 31 & (p);
        pg = 31 & (p >> 5);
        pb = 31 & (p >> 10);

        pr = (int32)(r * pr + 0.5f);
        pg = (int32)(g * pg + 0.5f);
        pb = (int32)(b * pb + 0.5f);

        if (pr > 31) pr = 31;
        if (pg > 31) pg = 31;
        if (pb > 31) pb = 31;

        p = pb | (pg << 5) | (pr << 10);
        IntelShort((short*)&p);

        clut[i] = p;
    }

    for (i = 0; i < clutsCount; i++)
    {
        if (memcmp(cluts[i], clut, sizeof(clut)) == 0)
        {
            return i;
        }
    }

    ASSERT(clutsCount < MAX_TRACK_CLUTS);
    memcpy(cluts[clutsCount++], clut, sizeof(clut));
    return clutsCount - 1;
#endif
    return 0;
}

int32 vertexAdd(const vec3i* v)
{
    int32 i, start;

    start = trackVertexCount - 256;
    if (start < 0)
    {
        start = 0;
    }

    for (i = start; i < trackVertexCount; i++)
    {
        if (trackVertex[i].x == v->x &&
            trackVertex[i].y == v->y &&
            trackVertex[i].z == v->z)
        {
            return i;
        }
    }

    trackVertex[trackVertexCount] = *v;
    return trackVertexCount++;
}

void collectTrackSectionGeometry(const TrackSection* section)
{
    int32 i;
    Face* face = trackFaces + trackFaceCount;

    for (i = section->faceStart; i < section->faceStart + section->faceCount; i++)
    {
        *face = gTrack.faces[i];

        face->vertex[0] = vertexAdd(gTrack.vertices + face->vertex[0]);
        face->vertex[1] = vertexAdd(gTrack.vertices + face->vertex[1]);
        face->vertex[2] = vertexAdd(gTrack.vertices + face->vertex[2]);
        face->vertex[3] = vertexAdd(gTrack.vertices + face->vertex[3]);

        ASSERT(face->vertex[0] != face->vertex[1] && face->vertex[0] != face->vertex[2] && face->vertex[0] != face->vertex[3]);
        ASSERT(face->vertex[1] != face->vertex[2] && face->vertex[1] != face->vertex[3]);
        ASSERT(face->vertex[2] != face->vertex[3]);

        face++;
    }

    trackFaceCount += section->faceCount;
}

void collectTrackGeometry(void)
{
    int32 i;
    const TrackSection* section = gTrack.sections;

    trackVertexCount = 0;
    trackFaceCount = 0;

    do
    {
        if (section->junction)
        {
            const TrackSection* jsection = section->junction;
            do
            {
                collectTrackSectionGeometry(jsection);
                jsection = jsection->nextSection;
            } while (!jsection->junction);
        }

        collectTrackSectionGeometry(section);
        section = section->nextSection;
    } while (section != gTrack.sections);

    ASSERT(trackVertexCount <= gTrack.vertexCount);
    ASSERT(trackFaceCount == gTrack.faceCount);
}

void collectTrackCLUT(void)
{
    /*
    int32 i;
    float r, g, b;
    Face* face = trackFaces;

    clutsCount = 0;

    for (i = 0; i < gTrack.faceCount; i++, face++)
    {
        r = 2.0f * face->red / 255.0f;
        g = 2.0f * face->green / 255.0f;
        b = 2.0f * face->blue / 255.0f;

        faceCLUT[i][0] = clutAdd(ctrltexTemplate[face->texture].tex1[0], r, g, b);

        //for (j = 0; j < 4; j++)
        //{
        //    faceCLUT[i][j + 1] = clutAdd(ctrltexTemplate[face->texture].tex4[j], r, g, b);
        //}
    }
    */
}

void getFaceRanges(const TrackSection* section, TrackRange* ranges, int32 from, int32 to)
{
    int32 i;

    for (i = 0; i < from; i++)
    {
        section = section->nextSection;
    }

    ranges[0].vStart = 0;
    ranges[0].vCount = 0;
    ranges[0].fStart = section->faceStart;
    ranges[0].fCount = 0;

    ranges[1].vStart = 0;
    ranges[1].vCount = 0;
    ranges[1].fStart = 0;
    ranges[1].fCount = 0;

    for (i = from; i < to; i++)
    {
        ranges->fCount += section->faceCount;
        section = section->nextSection;

        if (section == gTrack.sections)
        {
            ranges++;
            ranges->fStart = section->faceStart;
            ranges->fCount = 0;
        }
    }
}

#define X_MIN(a, b) ((a) < (b)) ? (a) : (b)
#define X_MAX(a, b) ((a) > (b)) ? (a) : (b)

void getVertexRange(TrackRange* range)
{
    int32 i, minIndex, maxIndex;
    const Face* face = trackFaces + range->fStart;

    if (!range->fCount)
        return;

    minIndex = maxIndex = face->vertex[0];

    for (i = 0; i < range->fCount; i++, face++)
    {
        minIndex = X_MIN(minIndex, face->vertex[0]);
        minIndex = X_MIN(minIndex, face->vertex[1]);
        minIndex = X_MIN(minIndex, face->vertex[2]);
        minIndex = X_MIN(minIndex, face->vertex[3]);

        maxIndex = X_MAX(maxIndex, face->vertex[0]);
        maxIndex = X_MAX(maxIndex, face->vertex[1]);
        maxIndex = X_MAX(maxIndex, face->vertex[2]);
        maxIndex = X_MAX(maxIndex, face->vertex[3]);
    }

    range->vStart = minIndex;
    range->vCount = maxIndex - minIndex + 1;
}

void getVisibilityRange(const TrackSection* section, TrackRange* ranges, int32 from, int32 to)
{
    getFaceRanges(section, ranges, from, to);
    getVertexRange(ranges + 0);
    getVertexRange(ranges + 1);
}

void writeShort(FILE* f, int16 value)
{
#if 0
    IntelShort(&value);
    fwrite(&value, sizeof(value), 1, f);
#endif
}

void writeInt(FILE* f, int32 value)
{
#if 0
    IntelLong(&value);
    fwrite(&value, sizeof(value), 1, f);
#endif
}

void convertTrack3DO(int32 trackNum)
{
#if 0
    TrackHeader3DO header;
    TrackSection3DO section;
    int32 i, j, index, objOffset;

    FILE* f;

    Face* face;
    char path[256];

    collectTrackGeometry();
    collectTrackCLUT();

    // VRAM
    strcpy(path, "../src/platform/3do/CD/");
    getPath(path + strlen(path), trackNum, "/track.pal");
    f = fopen(path, "wb");
    fwrite(cluts, sizeof(cluts[0]), clutsCount, f);
    fclose(f);

    // DRAM
    strcpy(path, "../src/platform/3do/CD/");
    getPath(path + strlen(path), trackNum, "/track.trk");
    f = fopen(path, "wb");

    header.vertexCount = trackVertexCount;
    header.faceCount = trackFaceCount;
    header.sectionCount = gTrack.sectionCount;
    header.plutCount = clutsCount;

    IntelLong(&header.vertexCount);
    IntelLong(&header.faceCount);
    IntelLong(&header.sectionCount);
    IntelLong(&header.plutCount);

    fwrite(&header, sizeof(header), 1, f);

    // vertices
    for (i = 0; i < trackVertexCount; i++)
    {
        int32 x = trackVertex[i].x;
        int32 y = trackVertex[i].y;
        int32 z = trackVertex[i].z;

        IntelLong(&x);
        IntelLong(&y);
        IntelLong(&z);

        fwrite(&x, sizeof(x), 1, f);
        fwrite(&y, sizeof(y), 1, f);
        fwrite(&z, sizeof(z), 1, f);
    }

    // faces
    face = trackFaces;
    for (i = 0; i < trackFaceCount; i++, face++)
    {
        uint16 i0 = face->vertex[0];
        uint16 i1 = face->vertex[1];
        uint16 i2 = face->vertex[2];
        uint16 i3 = face->vertex[3];
        int16 nx = face->normal.x;
        int16 ny = face->normal.y;
        int16 nz = face->normal.z;
        uint8 flags = face->flags;
        uint8 texture = face->texture;
        uint32 plut = faceCLUT[i][0] * sizeof(cluts[0]);

        // back to big endian
        IntelShort(&i0);
        IntelShort(&i1);
        IntelShort(&i2);
        IntelShort(&i3);

        IntelShort(&nx);
        IntelShort(&ny);
        IntelShort(&nz);

        IntelLong(&plut);

        fwrite(&i0, sizeof(i0), 1, f);
        fwrite(&i1, sizeof(i1), 1, f);
        fwrite(&i2, sizeof(i2), 1, f);
        fwrite(&i3, sizeof(i3), 1, f);
        fwrite(&nx, sizeof(nx), 1, f);
        fwrite(&ny, sizeof(ny), 1, f);
        fwrite(&nz, sizeof(nz), 1, f);
        fwrite(&flags, sizeof(flags), 1, f);
        fwrite(&texture, sizeof(texture), 1, f);
        fwrite(&plut, sizeof(plut), 1, f);
    }

    // sections
    ASSERT(gTrack.sections->junction == NULL); // track sections can't start from junction

    objOffset = gTrack.sectionCount * sizeof(TrackSection3DO);

    for (i = 0; i < gTrack.sectionCount; i++)
    {
        TrackSection* sec = gTrack.sections + i;

        if (sec->junction)
        {
            section.junction = (sec->junction - gTrack.sections) * sizeof(TrackSection3DO);
        }
        else
        {
            section.junction = 0;
        }

        section.prevSection = (sec->prevSection - gTrack.sections) * sizeof(TrackSection3DO);
        section.nextSection = (sec->nextSection - gTrack.sections) * sizeof(TrackSection3DO);

        section.centre = sec->centre;

        section.faceStart = sec->faceStart;
        section.faceCount = sec->faceCount;

        section.objects = objOffset;
        section.objectsCount = sec->objectsCount;

        objOffset += section.objectsCount * sizeof(uint16);

        section.globalRadius = sec->globalRadius;
        section.localRadius = sec->localRadius;

        section.flags = sec->flags;
        section.secNo = 0;

        IntelLong(&section.junction);
        IntelLong(&section.prevSection);
        IntelLong(&section.nextSection);

        IntelLong(&section.centre.x);
        IntelLong(&section.centre.y);
        IntelLong(&section.centre.z);

        IntelShort(&section.faceStart);
        IntelShort(&section.faceCount);

        IntelLong(&section.objects);
        IntelLong(&section.objectsCount);

        IntelShort(&section.globalRadius);
        IntelShort(&section.localRadius);

        IntelShort(&section.flags);

        getVisibilityRange(sec, section.visRange[0], 0, 4);
        getVisibilityRange(sec, section.visRange[1], 4, 24);

        for (j = 0; j < 2; j++)
        {
            IntelShort(&section.visRange[j][0].vStart);
            IntelShort(&section.visRange[j][0].vCount);
            IntelShort(&section.visRange[j][0].fStart);
            IntelShort(&section.visRange[j][0].fCount);

            IntelShort(&section.visRange[j][1].vStart);
            IntelShort(&section.visRange[j][1].vCount);
            IntelShort(&section.visRange[j][1].fStart);
            IntelShort(&section.visRange[j][1].fCount);
        }

        fwrite(&section, sizeof(section), 1, f);
    }

    for (i = 0; i < gTrack.sectionCount; i++)
    {
        TrackSection* sec = gTrack.sections + i;

        for (j = 0; j < sec->objectsCount; j++)
        {
            writeShort(f, sec->objects[j]);
        }
    }

    fclose(f);

    printf("track cluts: %d\n", clutsCount);
#endif
}

void convertObjects3DO(const char* name, const Object* objects, int32 count)
{
#if 0
    int32 i, j;
    int32 offset;
    const Object* obj;
    FILE* f;
    char path[256];

    strcpy(path, "../src/platform/3do/CD/");
    strcat(path, name);

    f = fopen(path, "wb");

    writeInt(f, count);

    offset = sizeof(Object) * count;

    obj = objects;
    for (i = 0; i < count; i++, obj++)
    {
        writeInt(f, obj->type); // TODO remove (use scene header)
        writeInt(f, obj->vCount);
        writeInt(f, obj->fCount);
        writeInt(f, obj->sCount);
        writeInt(f, obj->pos.x);
        writeInt(f, obj->pos.y);
        writeInt(f, obj->pos.z);
        writeInt(f, obj->extent);

        writeInt(f, offset);
        offset += obj->vCount * sizeof(vec3i);
        writeInt(f, offset);
        offset += obj->fCount * 8; // 8 = sizeof(ObjFace3DO)
        writeInt(f, offset);
        offset += obj->sCount * 8; // 8 = sizeof(ObjSprite3DO)
    }

    obj = objects;
    for (i = 0; i < count; i++, obj++)
    {
        for (j = 0; j < obj->vCount; j++)
        {
            // pre-shift for 3DO math co-processor
            writeInt(f, obj->vertices[j].x << 4);
            writeInt(f, obj->vertices[j].y << 4);
            writeInt(f, obj->vertices[j].z << 4);
        }

        for (j = 0; j < obj->fCount; j++)
        {
            uint32 indices =
                (obj->faces[j].index[0]) |
                (obj->faces[j].index[1] << 8) |
                (obj->faces[j].index[2] << 16) |
                (obj->faces[j].index[3] << 24);

            writeInt(f, obj->faces[j].flags & 0xFFFF);
            writeInt(f, indices);
        }

        for (j = 0; j < obj->sCount; j++)
        {
            uint16 size = 0; // TODO
            writeInt(f, obj->sprites[j].flags);
            writeShort(f, obj->sprites[j].index);
            writeShort(f, size);
        }
    }

    fclose(f);
#endif
}
#endif


#ifdef __3DO__
Object* LoadPrm(const char* name, Texture* texturePtr, int32* count)
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
    #if 0
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
    #endif
    return NULL;
}
#endif
