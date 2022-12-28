/* (C) Psygnosis 1994. By Jason Carl Denton & Rob Smith
*/

#include "game.h"

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

#endif
