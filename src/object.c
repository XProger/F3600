
/*
 * (C) Psygnosis 1994, 1995. By Jason Carl Denton & Rob Smith
 */

#include "game.h"

// PCwipeout
#define SHIELD_BODGE 50

extern char ScreenSize;
extern char Resolution;

/*
 * !!! NOTES ON 2D PRIMITIVES !!!
 *
 * The tag field of each 2d primitive has the highest order byte set to the same
 * value as the olen field in the TMD data!
 *
 * Null 2D primitives ( as in empty entries in an ordering table ) have their
 * highest order byte set to zero!
 *
 * The last link in the chain of 2D primitives has its pointer set to -1,
 * overall value being 0x00FFFFFF.
 *
 * The tag pointers are absolute values. To get the true address, clear the high
 * order byte and set the highest order bit, to generate 0x80 in the high
 * byte.
 *
 */

void InitMenuDisplay(int geomOffset)
{
    SetGeomOffset(160, geomOffset);
}

void InitGameDisplay(void)
{
    SetGeomOffset(160, 120);
}
