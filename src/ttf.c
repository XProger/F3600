/* (C) Psygnosis 1994. By Jason Carl Denton & Rob Smith
*/

#include "standard.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "ttf.h"
#include "track.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "droid.h"
#include "camera.h"
#include "sound.h"
#include "global.h"
#include "error.h"

TexTemplate* LoadTtfFile(char* file, short* count, short libraryTextures)
{
    long length;
    char* ttfFile;
    short* tex;
    short i;

    length = FileLength(file);
    if (length <= 0)
    {
        Error("Ttf.c:LoadTtfFile: Ttf file %s not found\n", file);
    }

    ttfFile = (char*)malloc(length + 32);

    if (!ttfFile)
    {
        /* Memory Allocation Failure! */
        Error("Ttf.c:LoadTtfFile: Failed to allocate memory for file %s\n", file);
    }

    if (LoadFile(file, ttfFile) != length)
    {
        /* File Error! */
        free(ttfFile);
        Error("Ttf.c:LoadTtfFile: Failed to load file %s\n", file);
    }

    tex = (short*)ttfFile;
    while (tex < ((short*)(ttfFile + length)))
    {
#if LoadMessages
//    printf( "TTF %2d: ", *count );
#endif
        for (i = 0; i < 21; i++)
        {
            *tex = ((*tex & 0x00ff) << 8) + ((*tex & 0xff00) >> 8);
#if 0
         *tex += libraryTextures;
#endif

#if LoadMessages
//       printf( "%3d ", *tex );
#endif
            tex += 1;
        }
        (*count)++; /* count of textures */
#if LoadMessages
//    printf( "\n" );
#endif
    }

    return ((TexTemplate*)ttfFile);
}
