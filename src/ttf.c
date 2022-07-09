/* (C) Psygnosis 1994. By Jason Carl Denton & Rob Smith
*/

#include "game.h"

#define TTF_SIZE_MAX    1024

char gTTF[TTF_SIZE_MAX];

TexTemplate* LoadTtfFile(char* file, short* count, short libraryTextures)
{
    long length;
    short* tex;
    short i;

    length = FileLength(file);
    if (length <= 0)
    {
        ASSERT(0);
    }

    ASSERT(length <= TTF_SIZE_MAX);

    if (LoadFile(file, gTTF) != length)
    {
        /* File Error! */
        ASSERT(0);
    }

    tex = (short*)gTTF;
    while (tex < ((short*)(gTTF + length)))
    {
        for (i = 0; i < 21; i++)
        {
            IntelShort(tex);
            tex += 1;
        }
        (*count)++; /* count of textures */
    }

    return ((TexTemplate*)gTTF);
}
