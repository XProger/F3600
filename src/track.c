/* (C) Psygnosis 1994. By Jason Carl Denton & Rob Smith
*/

#include "standard.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "ships.h"
#include "droid.h"
#include "camera.h"
#include "weapon.h"
#include "control.h"
#include "sound.h"
#include "draw.h"
#include "global.h"
#include "ttf.h"
#include "effects.h"
#include "scene.h"
#include "dynam.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "cone.h"
#include "race.h"
#include "error.h"

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

        for (j = 0; j < 4; j++)
        {
            IntelShort(&section->unused_high[j]);
            IntelShort(&section->unused_med[j]);
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
            Error("Convert track with track10");
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
    long length;

    length = FileLength(file);
    if (length <= 0)
    {
        Error("Track.c:InitVertices: file %s not found\n", file);
    }

    track->vertices = (VECTOR*)malloc(length + 32);
    if (!track->vertices)
    {
        Error("Track.c:InitVertices: Failed to allocate memory for track vertices");
    }

    if (LoadFile(file, (Ptr)track->vertices) != length)
    {
        Error("Track.c:InitVertices(): Failed to load file %s\n", file);
    }

    track->vertexCount = length / sizeof(VECTOR);

    IntelVertex();
}

void InitFaces(char* file)
{
    long length;
    short i;
    CVECTOR cols;

    length = FileLength(file);
    if (length <= 0)
    {
        Error("Track.c:InitFaces: file %s not found\n", file);
    }

    track->faces = (Face*)malloc(length + 32);
    if (!track->faces)
    {
        Error("Track.c:InitFaces: Failed to allocate memory for track faces");
    }

    if (LoadFile(file, (Ptr)track->faces) != length)
    {
        Error("Track.c:InitFaces(): Failed to load file %s\n", file);
    }
    track->faceCount = length / sizeof(Face);

    IntelFace();
}

void InitSections(char* file)
{
    long length;
    TrackSection* junctionSection;
    TrackSection* section;
    short i;

    length = FileLength(file);
    if (length <= 0)
    {
        Error("Track.c:InitSections: file %s not found\n", file);
    }

    track->sections = (TrackSection*)malloc(length + 32);
    if (!track->sections)
    {
        Error("Track.c:InitSections: Failed to allocate memory for track sections");
    }

    if (LoadFile(file, (Ptr)track->sections) != length)
    {
        Error("Track.c:InitSections(): Failed to load file %s\n", file);
    }
    track->sectionCount = length / sizeof(TrackSection);

    IntelSection();

    IndexToPointers();

#if 1
    CheckVersion();
#endif
}

void InitViewList(char* file)
{

    long length;
    TrackSection* section;
    long total;
    short i, j;

    length = FileLength(file);
    if (length <= 0)
    {
        Error("Track.c:InitViewList: file %s not found\n", file);
    }

    track->viewList = (short*)malloc(length + 32);
    if (!track->viewList)
    {
        Error("Track.c:InitViewList: Failed to allocate memory for view list");
    }

    if (LoadFile(file, (Ptr)track->viewList) != length)
    {
        Error("Track.c:InitViewList(): Failed to load file %s\n", file);
    }

    for (i = 0; i < length / 2; i++)
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

    if ((total * 2) != length)
    {
        printf("\n");
        printf("total  %d\n", total);
        printf("length %d\n", length);
        Error("track.c::InitViewList():problem with view list");
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
