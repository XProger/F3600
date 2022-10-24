/* (C) Psygnosis 1994. By Jason Carl Denton & Rob Smith
*/

#include "game.h"

#define TTF_SIZE_MAX    1024

char gTTF[TTF_SIZE_MAX];

TexTemplate* ctrltexTemplate;
short ctrlnumLib;
short ctrllibraryTextures;

TexTemplate* LoadTtfFile(char* file)
{
    short* tex;
    short i;

    long size = osLoadFile(file, gTTF);
    ASSERT(size <= TTF_SIZE_MAX);

    ctrlnumLib = 0;

    tex = (short*)gTTF;
    while (tex < ((short*)(gTTF + size)))
    {
        for (i = 0; i < 21; i++)
        {
            IntelShort(tex);
            tex += 1;
        }
        ctrlnumLib++; /* count of textures */
    }

    return ((TexTemplate*)gTTF);
}
