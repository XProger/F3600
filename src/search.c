#include "game.h"

#define LOOK_BACK 3
#define LOOK_AHEAD 6

void SectionSearch(ShipData* ship)
{
    TrackSection *trk, *nearest_section;
    int smallest_magSq;
    vec4i cent2ship;
    int magSq;
    int i;
    short juncChecked = 0;
    TrackSection* junc = NULL;

    trk = ship->nearTrkSect;

    /****** Start search several sections before current section ******/

    for (i = 0; i < LOOK_BACK; i++)
    {
        trk = trk->prevSection;
    }

    /****** Find vector from ship centre to track section under  ******/
    /******                     consideration                    ******/

    cent2ship.x = ship->pos.x - trk->centre.x;
    cent2ship.y = (ship->pos.y - trk->centre.y) >> 2;
    cent2ship.z = ship->pos.z - trk->centre.z;

    smallest_magSq = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

    if (trk->junction)
    {
        junc = trk->junction;
        juncChecked = 1;
    }

    nearest_section = trk;

    for (i = 0; i < LOOK_AHEAD; i++)
    {
        trk = trk->nextSection;

        if (trk->junction)
        {
            junc = trk->junction;
            juncChecked = 1;
        }

        cent2ship.x = ship->pos.x - trk->centre.x;
        cent2ship.y = (ship->pos.y - trk->centre.y) >> 2;
        cent2ship.z = ship->pos.z - trk->centre.z;
        magSq = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

        if (magSq < smallest_magSq)
        {
            smallest_magSq = magSq;
            nearest_section = trk;
        }
    }

    if (juncChecked == 1)
    {
        trk = junc;

        for (i = 0; i < LOOK_AHEAD; i++)
        {
            cent2ship.x = ship->pos.x - trk->centre.x;
            cent2ship.y = (ship->pos.y - trk->centre.y) >> 2;
            cent2ship.z = ship->pos.z - trk->centre.z;
            magSq = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

            if (magSq < smallest_magSq)
            {
                smallest_magSq = magSq;
                nearest_section = trk;
            }
            if (junc->flags & JunctionStart)
                trk = trk->nextSection;
            else
                trk = trk->prevSection;
        }
    }

    if (smallest_magSq > (3700 * 3700))
    {
        ship->attr |= FLYING;
    }
    else
    {
        ship->attr &= ~FLYING;
    }

    ship->nearTrkSect = nearest_section;
}

TrackSection* FindNearestSection(TrackSection* trk, const vec3i* pos)
{
    TrackSection* nearest_section;
    int smallest_magSq;
    vec4i cent2ship;
    int magSq;
    int i;
    short juncChecked = 0;
    TrackSection* junc = NULL;

    /****** Start search several sections before current section ******/

    for (i = 0; i < LOOK_BACK; i++)
    {
        trk = trk->prevSection;
    }

    /****** Find vector from ship centre to track section under  ******/
    /******                     consideration                    ******/

    cent2ship.x = pos->x - trk->centre.x;
    cent2ship.y = pos->y - trk->centre.y;
    cent2ship.z = pos->z - trk->centre.z;

    smallest_magSq = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

    nearest_section = trk;

    if (trk->junction)
    {
        junc = trk->junction;
        juncChecked = 1;
    }

    for (i = 0; i < LOOK_AHEAD; i++)
    {
        trk = trk->nextSection;

        if (trk->junction)
        {
            junc = trk->junction;
            juncChecked = 1;
        }

        cent2ship.x = pos->x - trk->centre.x;
        cent2ship.y = pos->y - trk->centre.y;
        cent2ship.z = pos->z - trk->centre.z;
        magSq = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

        if (magSq < smallest_magSq)
        {
            smallest_magSq = magSq;
            nearest_section = trk;
        }
    }

    if (juncChecked == 1)
    {
        trk = junc;

        for (i = 0; i < LOOK_AHEAD; i++)
        {
            cent2ship.x = pos->x - trk->centre.x;
            cent2ship.y = pos->y - trk->centre.y;
            cent2ship.z = pos->z - trk->centre.z;
            magSq = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

            if (magSq < smallest_magSq)
            {
                smallest_magSq = magSq;
                nearest_section = trk;
            }
            if (junc->flags & JunctionStart)
                trk = trk->nextSection;
            else
                trk = trk->prevSection;
        }
    }

    return (nearest_section);
}
