/* (C) Psygnosis 1994. By Jason Carl Denton & Rob Smith
*/

#include "game.h"

typedef struct TrackHeader3DO
{
    int32 vertexCount;
    int32 faceCount;
    int32 sectionCount;
    int32 plutCount;
} TrackHeader3DO;

TrackSection* NextIndexSection(TrackSection* section)
{
    section += 1;

    if (section > gTrack.sections + gTrack.sectionCount - 1)
    {
        section = gTrack.sections;
    }

    return (section);
}

#ifdef __3DO__
void InitPLUT(char* file)
{
    int32 i;
    long size = osLoadFile(file, (char*)pVRAM);
    uint32 plutsAddr = (uint32)allocVRAM(size);

    for (i = 0; i < gTrack.faceCount; i++)
    {
        gTrack.faces[i].plut += plutsAddr;
    }
}

void InitTrack(char* file)
{
    int32 i;
    TrackSection* section;
    int32 size = osLoadFile(file, (char*)pDRAM);
    uint8* ptr = (uint8*)allocDRAM(size);

    TrackHeader3DO* header = (TrackHeader3DO*)ptr;
    ptr += sizeof(TrackHeader3DO);

    gTrack.vertexCount = header->vertexCount;
    gTrack.faceCount = header->faceCount;
    gTrack.sectionCount = header->sectionCount;

    gTrack.vertices = (vec3i*)ptr;
    ptr += header->vertexCount * sizeof(vec3i);

    gTrack.faces = (Face*)ptr;
    ptr += header->faceCount * sizeof(Face);

    gTrack.sections = (TrackSection*)ptr;

    // init section pointers
    section = gTrack.sections;

    for (i = 0; i < gTrack.sectionCount; i++)
    {
        section->prevSection = (TrackSection*)(ptr + (int32)section->prevSection);
        section->nextSection = (TrackSection*)(ptr + (int32)section->nextSection);

        if (section->junction)
        {
            section->junction = (TrackSection*)(ptr + (int32)section->junction);
        }

        section->objects = (int16*)(ptr + (int32)section->objects);

        section++;
    }
}

void LoadTrack(int32 trackNum)
{
    char trackPath[64];

    getPath(trackPath, trackNum, "/library.ttf");
    ctrltexTemplate = LoadTtfFile(trackPath);

    ctrllibraryTextures = gTexturePtr - gTextures;
    getPath(trackPath, trackNum, "/library.cmp");
    LoadCompressedTextureSequence(trackPath, 0);

    getPath(trackPath, trackNum, "/track.trk");
    InitTrack(trackPath);

    getPath(trackPath, trackNum, "/track.pal");
    InitPLUT(trackPath);
}
#else
void IntelVertex(void)
{
    vec4i* vert;
    vec3i* v;
    short i;

    /* intel the vertex data */

    // convert from vec4i to vec3i
    vert = (vec4i*)gTrack.vertices;
    v = gTrack.vertices;
    for (i = 0; i < gTrack.vertexCount; i++, vert++, v++)
    {
        v->x = vert->x;
        v->y = vert->y;
        v->z = vert->z;

        IntelLong(&v->x);
        IntelLong(&v->y);
        IntelLong(&v->z);
    }
}

void IntelFace(void)
{
    Face* face;
    short i;

    /* intel the face data */

    face = gTrack.faces;
    for (i = 0; i < gTrack.faceCount; i++)
    {
        IntelShort(&face->vertex[0]);
        IntelShort(&face->vertex[1]);
        IntelShort(&face->vertex[2]);
        IntelShort(&face->vertex[3]);

        IntelShort(&face->normal.x);
        IntelShort(&face->normal.y);
        IntelShort(&face->normal.z);

        face += 1;
    }
}

void IntelSection(void)
{
    TrackSection* section;
    short i, j;

    /* intel the section data */

    section = gTrack.sections;
    for (i = 0; i < gTrack.sectionCount; i++)
    {
        IntelLong(&section->junction);
        IntelLong(&section->prevSection);
        IntelLong(&section->nextSection);

        IntelLong(&section->centre.x);
        IntelLong(&section->centre.y);
        IntelLong(&section->centre.z);

        //IntelShort(&section->version);
        //IntelShort(&section->pad);

        for (j = 0; j < 3; j++)
        {
            IntelShort(&section->northViewCount[j]);
            IntelShort(&section->southViewCount[j]);
            IntelShort(&section->eastViewCount[j]);
            IntelShort(&section->westViewCount[j]);
            IntelShort(&section->allViewCount[j]);
        }

        IntelShort(&section->faceStart);
        IntelShort(&section->faceCount);

        IntelShort(&section->globalRadius);
        IntelShort(&section->localRadius);

        IntelShort(&section->flags);
        IntelShort(&section->secNo);

        section += 1;
    }
}

void IndexToPointers(void)
{
    TrackSection* section;
    short i;

    /* change the section indices to pointers */

    section = gTrack.sections;
    for (i = 0; i < gTrack.sectionCount; i++)
    {
        section->prevSection = gTrack.sections + (int32)section->prevSection;
        section->nextSection = gTrack.sections + (int32)section->nextSection;

        if ((int32)section->junction != -1)
        {
            section->junction = gTrack.sections + (int32)section->junction;
        }
        else
        {
            section->junction = NULL;
        }

        section += 1;
    }
}

void InitVertices(char* file)
{
    long size = osLoadFile(file, pDRAM);
    gTrack.vertexCount = size / sizeof(vec4i);
    gTrack.vertices = (vec3i*)allocDRAM(gTrack.vertexCount * sizeof(vec3i));
    IntelVertex();
}

void InitFaces(char* file)
{
    long size = osLoadFile(file, pDRAM);
    gTrack.faces = (Face*)allocDRAM(size);
    gTrack.faceCount = size / sizeof(Face);
    IntelFace();
}

void InitSections(char* file)
{
    long size = osLoadFile(file, pDRAM);
    gTrack.sections = (TrackSection*)allocDRAM(size);
    gTrack.sectionCount = size / sizeof(TrackSection);
    IntelSection();
    IndexToPointers();
}

void InitViewList(char* file)
{
    TrackSection* section;
    long total;
    short i, j;
    int viewListCount;

    long size = osLoadFile(file, pDRAM);
    gTrack.viewList = (int16*)allocDRAM(size);
    viewListCount = size / 2;

    for (i = 0; i < viewListCount; i++)
    {
        IntelShort(&gTrack.viewList[i]);
    }

    section = gTrack.sections;
    total = 0;
    for (i = 0; i < gTrack.sectionCount; i++)
    {
        for (j = 0; j < 3; j++)
        {
            section->northViewSection[j] = gTrack.viewList + total;
            total += section->northViewCount[j];

            section->southViewSection[j] = gTrack.viewList + total;
            total += section->southViewCount[j];

            section->eastViewSection[j] = gTrack.viewList + total;
            total += section->eastViewCount[j];

            section->westViewSection[j] = gTrack.viewList + total;
            total += section->westViewCount[j];

            section->allViewSection[j] = gTrack.viewList + total;
            total += section->allViewCount[j];
        }

        section++;
    }
}

void LoadTrack(int32 trackNum)
{
    char trackPath[64];

    getPath(trackPath, trackNum, "/library.ttf");
    ctrltexTemplate = LoadTtfFile(trackPath);

    HACK_LOAD_TRACK = 1;
    ctrllibraryTextures = gTexturePtr - gTextures;
    getPath(trackPath, trackNum, "/library.cmp");
    LoadCompressedTextureSequence(trackPath, 0);
    HACK_LOAD_TRACK = 0;

    getPath(trackPath, trackNum, "/track.trv");
    InitVertices(trackPath);
    getPath(trackPath, trackNum, "/track.trf");
    InitFaces(trackPath);

    getPath(trackPath, trackNum, "/track.trs");
    InitSections(trackPath);
    getPath(trackPath, trackNum, "/track.vew");
    InitViewList(trackPath);
}
#endif

#ifdef _DEBUG
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
    IntelShort(&value);
    fwrite(&value, sizeof(value), 1, f);
}

void writeInt(FILE* f, int32 value)
{
    IntelLong(&value);
    fwrite(&value, sizeof(value), 1, f);
}

void convertTrack3DO(int32 trackNum)
{
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
}

void convertObjects3DO(const char* name, const Object* objects, int32 count)
{
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
}
#endif