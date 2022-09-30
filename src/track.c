/* (C) Psygnosis 1994. By Jason Carl Denton & Rob Smith
*/

#include "game.h"

#define TrackVersion 8

void IntelVertex(void)
{
    VECTOR* vert;
    short i;

    /* intel the vertex data */

    vert = track->vertices;
    for (i = 0; i < track->vertexCount; i++)
    {
        IntelLong(&vert->vx);
        IntelLong(&vert->vy);
        IntelLong(&vert->vz);

        vert += 1;
    }
}

void IntelFace(void)
{
    Face* face;
    short i;

    /* intel the face data */

    face = track->faces;
    for (i = 0; i < track->faceCount; i++)
    {
        IntelShort(&face->vertex[0]);
        IntelShort(&face->vertex[1]);
        IntelShort(&face->vertex[2]);
        IntelShort(&face->vertex[3]);

        IntelShort(&face->normal.vx);
        IntelShort(&face->normal.vy);
        IntelShort(&face->normal.vz);

        face += 1;
    }
}

void IntelSection(void)
{
    TrackSection* section;
    short i, j;

    /* intel the section data */

    section = track->sections;
    for (i = 0; i < track->sectionCount; i++)
    {
        IntelLong(&section->junction.index);
        IntelLong(&section->prevSection.index);
        IntelLong(&section->nextSection.index);

        IntelLong(&section->centre.vx);
        IntelLong(&section->centre.vy);
        IntelLong(&section->centre.vz);

        IntelShort(&section->version);
        IntelShort(&section->pad);

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

void CheckVersion(void)
{
    TrackSection* section;
    short i;

    section = track->sections;
    for (i = 0; i < track->sectionCount; i++)
    {
        if (section->version != TrackVersion)
        {
            ASSERT(0);
        }
        section += 1;
    }
}

void IndexToPointers(void)
{
    TrackSection* section;
    short i;

    /* change the section indices to pointers */

    section = track->sections;
    for (i = 0; i < track->sectionCount; i++)
    {
        section->prevSection.ptr = track->sections + section->prevSection.index;
        section->nextSection.ptr = track->sections + section->nextSection.index;

        if (section->junction.index != -1)
        {
            section->junction.ptr = track->sections + section->junction.index;
        }

        section += 1;
    }
}

void PointersToIndex(void)
{
    TrackSection* section;
    short i;

    /* change the section pointers back to indices */

    section = track->sections;
    for (i = 0; i < track->sectionCount; i++)
    {
        section->prevSection.index = section->prevSection.ptr - track->sections;
        section->nextSection.index = section->nextSection.ptr - track->sections;

        if (section->junction.index != -1)
        {
            section->junction.index = section->junction.ptr - track->sections;
        }

        section += 1;
    }
}

void InitVertices(char* file)
{
    long size = osLoadFile(file, (char*)track->vertices);
    track->vertexCount = size / sizeof(VECTOR);
    ASSERT(track->vertexCount <= MAX_VERTICES);
    IntelVertex();
}

void InitFaces(char* file)
{
    long size = osLoadFile(file, (char*)track->faces);
    track->faceCount = size / sizeof(Face);
    ASSERT(track->faceCount <= MAX_FACES);
    IntelFace();
}

void InitSections(char* file)
{
    long size = osLoadFile(file, (char*)track->sections);
    track->sectionCount = size / sizeof(TrackSection);
    ASSERT(track->sectionCount <= MAX_SECTIONS);
    IntelSection();
    IndexToPointers();
#if 1
    CheckVersion();
#endif
}

void InitViewList(char* file)
{
    TrackSection* section;
    long total;
    short i, j;
    int viewListCount;

    long size = osLoadFile(file, (char*)track->viewList);
    viewListCount = size / 2;

    ASSERT(viewListCount <= MAX_VIEWLIST);

    for (i = 0; i < viewListCount; i++)
    {
        IntelShort(&track->viewList[i]);
    }

    section = track->sections;
    total = 0;
    for (i = 0; i < track->sectionCount; i++)
    {
        for (j = 0; j < 3; j++)
        {
            section->northViewSection[j] = track->viewList + total;
            total += section->northViewCount[j];

            section->southViewSection[j] = track->viewList + total;
            total += section->southViewCount[j];

            section->eastViewSection[j] = track->viewList + total;
            total += section->eastViewCount[j];

            section->westViewSection[j] = track->viewList + total;
            total += section->westViewCount[j];

            section->allViewSection[j] = track->viewList + total;
            total += section->allViewCount[j];
        }

        section++;
    }
}

TrackSection* NextIndexSection(TrackSection* section)
{
    section += 1;

    if (section > track->sections + track->sectionCount - 1)
    {
        section = track->sections;
    }

    return (section);
}

TrackSection* PrevIndexSection(TrackSection* section)
{
    section -= 1;

    if (section < track->sections)
    {
        section = track->sections + track->sectionCount - 1;
    }

    return (section);
}
