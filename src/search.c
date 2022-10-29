#include "game.h"

#define LOOK_BACK 3
#define LOOK_AHEAD 6

int SectionSearch(ShipData* playerShip)
{
    TrackSection *trk, *nearest_section;
    int smallest_mag;
    vec4i cent2ship;
    int mag;
    int i;
    short juncChecked = 0;
    TrackSection* junc = NULL;

    trk = playerShip->nearTrkSect;

    /****** Start search several sections before current section ******/

    for (i = 0; i < LOOK_BACK; i++)
    {
        trk = trk->prevSection;
    }

    /****** Find vector from ship centre to track section under  ******/
    /******                     consideration                    ******/

    cent2ship.x = playerShip->pos.x - trk->centre.x;
    cent2ship.y = (playerShip->pos.y - trk->centre.y) >> 2;
    cent2ship.z = playerShip->pos.z - trk->centre.z;

    smallest_mag = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

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

        cent2ship.x = playerShip->pos.x - trk->centre.x;
        cent2ship.y = (playerShip->pos.y - trk->centre.y) >> 2;
        cent2ship.z = playerShip->pos.z - trk->centre.z;
        mag = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

        if (mag < smallest_mag)
        {
            smallest_mag = mag;
            nearest_section = trk;
        }
    }

    if (juncChecked == 1)
    {
        trk = junc;

        for (i = 0; i < LOOK_AHEAD; i++)
        {
            cent2ship.x = playerShip->pos.x - trk->centre.x;
            cent2ship.y = (playerShip->pos.y - trk->centre.y) >> 2;
            cent2ship.z = playerShip->pos.z - trk->centre.z;
            mag = ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

            if (mag < smallest_mag)
            {
                smallest_mag = mag;
                nearest_section = trk;
            }
            if (junc->flags & JunctionStart)
                trk = trk->nextSection;
            else
                trk = trk->prevSection;
        }
    }

    smallest_mag = rsqrt(smallest_mag);

    if (smallest_mag > 3700)
    {
        playerShip->attr |= FLYING;
    }
    else
    {
        playerShip->attr &= ~FLYING;
    }

    playerShip->nearTrkSect = nearest_section;
    return (smallest_mag);
}

TrackSection* FindNearestSection(TrackSection* trk, vec4i pos)
{
    TrackSection* nearest_section;
    int smallest_mag;
    vec4i cent2ship;
    int mag;
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

    cent2ship.x = pos.x - trk->centre.x;
    cent2ship.y = pos.y - trk->centre.y;
    cent2ship.z = pos.z - trk->centre.z;

    smallest_mag = /*rsqrt*/ ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

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

        cent2ship.x = pos.x - trk->centre.x;
        cent2ship.y = pos.y - trk->centre.y;
        cent2ship.z = pos.z - trk->centre.z;
        mag = /*rsqrt*/ ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

        if (mag < smallest_mag)
        {
            smallest_mag = mag;
            nearest_section = trk;
        }
    }

    if (juncChecked == 1)
    {
        trk = junc;

        for (i = 0; i < LOOK_AHEAD; i++)
        {
            cent2ship.x = pos.x - trk->centre.x;
            cent2ship.y = pos.y - trk->centre.y;
            cent2ship.z = pos.z - trk->centre.z;
            mag = /*rsqrt*/ ((cent2ship.x * cent2ship.x) + (cent2ship.y * cent2ship.y) + (cent2ship.z * cent2ship.z));

            if (mag < smallest_mag)
            {
                smallest_mag = mag;
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
