#include "game.h"

extern int32 ctrlmode;

void ForceFeedback(int32 joyIndex, int32 mag)
{
    int32 rumble;

    if (ctrlmode == ATTRACT)
        return;

    rumble = mag >> 3;
    if (rumble > 255)
    {
        rumble = 255;
    }

    osJoyVibrate(joyIndex, rumble, rumble);
}

#define SHIP_RADIUS 50

void Ship2ShipCollision(ShipData* shipIndex, Object** shipShapes, int i, int j)
{
    vec3i distance;
    vec3i vc;
    vec3i ship2ship;
    vec3i ship1React, ship2React;
    vec3i soundPos;
    ShipData* ship1 = shipIndex + j;
    ShipData* ship2 = shipIndex + i;

    distance.x = (ship2->pos.x - ship1->pos.x) >> 3;
    distance.y = (ship2->pos.y - ship1->pos.y) >> 3;
    distance.z = (ship2->pos.z - ship1->pos.z) >> 3;

    if (GetMagnitudeSqr(&distance) < SHIP_RADIUS * SHIP_RADIUS) // TODO add nose sphere
    {
        /**********************************************************/
        /***        Calculate velocity of zero-momentum frame   ***/
        /***                                                    ***/
        /***                    vc = m1v1 + m2v2                ***/
        /***                         -----------                ***/
        /***                           m1 + m2                  ***/
        /**********************************************************/

        // optimization
        // all ships in wipeout have the same mass == 150
        // so vc = (v1 + v2) / 2

        vc.x = (ship1->vel.x + ship2->vel.x) >> 1;
        vc.y = (ship1->vel.y + ship2->vel.y) >> 1;
        vc.z = (ship1->vel.z + ship2->vel.z) >> 1;

        ship1React.x = (vc.x - ship1->vel.x) >> 1;
        ship1React.y = (vc.y - ship1->vel.y) >> 1;
        ship1React.z = (vc.z - ship1->vel.z) >> 1;

        ship2React.x = (vc.x - ship2->vel.x) >> 1;
        ship2React.y = (vc.y - ship2->vel.y) >> 1;
        ship2React.z = (vc.z - ship2->vel.z) >> 1;

        ship1->pos.x -= ship1->vel.x >> VELOCITY_SHIFT;
        ship1->pos.y -= ship1->vel.y >> VELOCITY_SHIFT;
        ship1->pos.z -= ship1->vel.z >> VELOCITY_SHIFT;

        ship2->pos.x -= ship2->vel.x >> VELOCITY_SHIFT;
        ship2->pos.y -= ship2->vel.y >> VELOCITY_SHIFT;
        ship2->pos.z -= ship2->vel.z >> VELOCITY_SHIFT;

        ship1->vel.x = vc.x + ship1React.x;
        ship1->vel.y = vc.y + ship1React.y;
        ship1->vel.z = vc.z + ship1React.z;

        ship2->vel.x = vc.x + ship2React.x;
        ship2->vel.y = vc.y + ship2React.y;
        ship2->vel.z = vc.z + ship2React.z;

        ship2ship.x = ship2->pos.x - ship1->pos.x;
        ship2ship.y = ship2->pos.y - ship1->pos.y;
        ship2ship.z = ship2->pos.z - ship1->pos.z;

        ship2->vel.x += ship2ship.x << 2;
        ship2->vel.y += ship2ship.y << 2;
        ship2->vel.z += ship2ship.z << 2;

        ship1->vel.x -= ship2ship.x << 2;
        ship1->vel.y -= ship2ship.y << 2;
        ship1->vel.z -= ship2ship.z << 2;

        ship1->pos.x += ship1->vel.x >> VELOCITY_SHIFT;
        ship1->pos.y += ship1->vel.y >> VELOCITY_SHIFT;
        ship1->pos.z += ship1->vel.z >> VELOCITY_SHIFT;

        ship2->pos.x += ship2->vel.x >> VELOCITY_SHIFT;
        ship2->pos.y += ship2->vel.y >> VELOCITY_SHIFT;
        ship2->pos.z += ship2->vel.z >> VELOCITY_SHIFT;

        if (((ship1->attr | ship2->attr) & SHIP_COLL) == 0)
        {
            soundPos.x = (ship2->pos.x + ship1->pos.x) >> 1;
            soundPos.y = (ship2->pos.y + ship1->pos.y) >> 1;
            soundPos.z = (ship2->pos.z + ship1->pos.z) >> 1;
            PlayDirectionalNote(SFX_SHIP_2_SHIP, &soundPos);
        }

        if (ship1->player)
        {
            PlayerData* player = (PlayerData*)ship1->player;
            ForceFeedback(player->joyIndex, 255);
        }

        if (ship2->player)
        {
            PlayerData* player = (PlayerData*)ship2->player;
            ForceFeedback(player->joyIndex, 255);
        }

        ship2->attr |= SHIP_COLL;
        ship1->attr |= SHIP_COLL;
    }
    else
    {
        ship2->attr &= ~SHIP_COLL;
        ship1->attr &= ~SHIP_COLL;
    }
}

void CallShip2ShipCollision(ShipData* shipIndex, Object** shipShapes)
{
    int32 i, j;

    for (j = 0; j < (NO_OF_SHIPS - 1); j++)
    {
        for (i = j + 1; i < NO_OF_SHIPS; i++)
        {
            Ship2ShipCollision(shipIndex, shipShapes, i, j);
        }
    }
}

int PlaneLineCollision(const vec3i* plane_point, const vec3i* ship_pos, const vec3s* normal)
{
    int32 dot_product =
        (ship_pos->x - plane_point->x) * normal->x +
        (ship_pos->y - plane_point->y) * normal->y +
        (ship_pos->z - plane_point->z) * normal->z;

#if 1 // optim normal length == 4096
    return dot_product >> FIXED_SHIFT;
#else
    int32 norm_dot_product = ((normal->x * normal->x) + (normal->y * normal->y) + (normal->z * normal->z)) >> FIXED_SHIFT;
    return dot_product / norm_dot_product;
#endif
}

void Ship2Track(ShipData* ship, Face* facePtr)
{
    vec3i nosePos, wingPos;
    int alpha;
    vec3i direction;
    int downTrack;
    TrackSection* trackPtr;
    int toFace;
    vec3i toFaceVector;
    const vec3i* facePoint;
    int collide;
    Face* facePtr2;

    nosePos.x = ship->pos.x + (ship->unitVecNose.x >> 3);
    nosePos.y = ship->pos.y + (ship->unitVecNose.y >> 3);
    nosePos.z = ship->pos.z + (ship->unitVecNose.z >> 3);

    wingPos.x = ship->pos.x - (ship->unitVecWing.x >> 4) - (ship->unitVecNose.x >> 4);
    wingPos.y = ship->pos.y - (ship->unitVecWing.y >> 4) - (ship->unitVecNose.y >> 4);
    wingPos.z = ship->pos.z - (ship->unitVecWing.z >> 4) - (ship->unitVecNose.z >> 4);

    trackPtr = ship->nearTrkSect->nextSection;

    direction.x = trackPtr->centre.x - ship->nearTrkSect->centre.x;
    direction.y = trackPtr->centre.y - ship->nearTrkSect->centre.y;
    direction.z = trackPtr->centre.z - ship->nearTrkSect->centre.z;

    downTrack = ((direction.x * ship->unitVecNose.x) + (direction.y * ship->unitVecNose.y) + (direction.z * ship->unitVecNose.z));

    if (downTrack < 0)
        ship->attr &= ~DIRECTION;
    else
        ship->attr |= DIRECTION;

    toFaceVector.x = gTrack.vertices[(facePtr->vertex[0])].x - gTrack.vertices[(facePtr->vertex[1])].x;
    toFaceVector.y = gTrack.vertices[(facePtr->vertex[0])].y - gTrack.vertices[(facePtr->vertex[1])].y;
    toFaceVector.z = gTrack.vertices[(facePtr->vertex[0])].z - gTrack.vertices[(facePtr->vertex[1])].z;

    direction.x = ship->nearTrkSect->centre.x - ship->pos.x;
    direction.y = ship->nearTrkSect->centre.y - ship->pos.y;
    direction.z = ship->nearTrkSect->centre.z - ship->pos.z;

    toFace = ((direction.x * toFaceVector.x) + (direction.y * toFaceVector.y) + (direction.z * toFaceVector.z));

    facePtr--;

    /******** Check against left hand side of track ********/

    if (toFace > 0)
    {
        ship->attr |= LSIDE;
        if (1) //(facePtr == (track->faces + ship->nearTrkSect->faceStart)) || ship->nearTrkSect->junction)
        {
            facePoint = gTrack.vertices + facePtr->vertex[0];

            alpha = PlaneLineCollision(facePoint, &nosePos, &facePtr->normal);

            if (alpha <= 0)
            {
                if (ship->nearTrkSect->flags & JunctionStart)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, -downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + ship->nearTrkSect->nextSection->faceStart;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(ship, facePtr, -downTrack, alpha);
                        }
                    }
                }
                else if (ship->nearTrkSect->flags & JunctionEnd)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, -downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + ship->nearTrkSect->prevSection->faceStart;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(ship, facePtr, -downTrack, alpha);
                        }
                    }
                }
                else
                    NoseCollision(ship, facePtr, -downTrack, alpha);
                return;
            }

            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);

            if (alpha <= 0)
            {
                if ((ship->nearTrkSect->flags & JunctionStart) || (ship->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, -downTrack, alpha);
                    }
                }
                else
                    WingCollision(ship, facePtr, -downTrack, alpha);
                return;
            }

            wingPos.x = ship->pos.x + (ship->unitVecWing.x >> 4) - (ship->unitVecNose.x >> 4);
            wingPos.y = ship->pos.y + (ship->unitVecWing.y >> 4) - (ship->unitVecNose.y >> 4);
            wingPos.z = ship->pos.z + (ship->unitVecWing.z >> 4) - (ship->unitVecNose.z >> 4);

            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);
            if (alpha <= 0)
            {
                if ((ship->nearTrkSect->flags & JunctionStart) || (ship->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, -downTrack, alpha);
                    }
                }
                else
                    WingCollision(ship, facePtr, -downTrack, alpha);
                return;
            }
        }
    }
    /****************************************************************************/
    /*****                  Collision check against 2nd wall                *****/
    /****************************************************************************/
    else
    {
        ship->attr &= ~LSIDE;

        facePtr++;
        while (facePtr->flags & TRACK_BASE)
        {
            facePtr++;
        }

        if ((ship->nearTrkSect->faceCount > 3) || ship->nearTrkSect->junction)
        {

            facePoint = gTrack.vertices + facePtr->vertex[0];

            alpha = PlaneLineCollision(facePoint, &nosePos, &facePtr->normal);
            if (alpha <= 0)
            {
                if (ship->nearTrkSect->flags & JunctionStart)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + ship->nearTrkSect->nextSection->faceStart;
                        facePtr2 += 3;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(ship, facePtr, -downTrack, alpha);
                        }
                    }
                }
                else if (ship->nearTrkSect->flags & JunctionEnd)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, -downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + ship->nearTrkSect->prevSection->faceStart;
                        facePtr2 += 3;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(ship, facePtr2, -downTrack, alpha);
                        }
                    }
                }
                else
                    NoseCollision(ship, facePtr, downTrack, alpha);
                return;
            }
            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);
            if (alpha <= 0)
            {
                if ((ship->nearTrkSect->flags & JunctionStart) || (ship->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, downTrack, alpha);
                    }
                }
                else
                    WingCollision(ship, facePtr, downTrack, alpha);
                return;
            }

            wingPos.x = ship->pos.x + (ship->unitVecWing.x >> 4) - (ship->unitVecNose.x >> 4);
            wingPos.y = ship->pos.y + (ship->unitVecWing.y >> 4) - (ship->unitVecNose.y >> 4);
            wingPos.z = ship->pos.z + (ship->unitVecWing.z >> 4) - (ship->unitVecNose.z >> 4);
            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);
            if (alpha <= 0)
            {
                if ((ship->nearTrkSect->flags & JunctionStart) || (ship->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(ship, facePtr, downTrack, alpha);
                    }
                }
                else
                    WingCollision(ship, facePtr, downTrack, alpha);
                return;
            }
        }
    }
}

int CheckPolygonCollision(const Face* facePtr, const vec3i* pos, int alpha)
{
    vec3i planePoint;
    vec3i vec0, vec1, vec2, vec3; /* vectors between plane coords and intersection point */
    int angle;

    planePoint.x = pos->x - ((facePtr->normal.x * alpha) >> FIXED_SHIFT);
    planePoint.y = pos->y - ((facePtr->normal.y * alpha) >> FIXED_SHIFT);
    planePoint.z = pos->z - ((facePtr->normal.z * alpha) >> FIXED_SHIFT);

    vec0.x = planePoint.x - gTrack.vertices[(facePtr->vertex[1])].x;
    vec0.y = planePoint.y - gTrack.vertices[(facePtr->vertex[1])].y;
    vec0.z = planePoint.z - gTrack.vertices[(facePtr->vertex[1])].z;

    vec1.x = planePoint.x - gTrack.vertices[(facePtr->vertex[2])].x;
    vec1.y = planePoint.y - gTrack.vertices[(facePtr->vertex[2])].y;
    vec1.z = planePoint.z - gTrack.vertices[(facePtr->vertex[2])].z;

    vec2.x = planePoint.x - gTrack.vertices[(facePtr->vertex[0])].x;
    vec2.y = planePoint.y - gTrack.vertices[(facePtr->vertex[0])].y;
    vec2.z = planePoint.z - gTrack.vertices[(facePtr->vertex[0])].z;

    vec3.x = planePoint.x - gTrack.vertices[(facePtr->vertex[3])].x;
    vec3.y = planePoint.y - gTrack.vertices[(facePtr->vertex[3])].y;
    vec3.z = planePoint.z - gTrack.vertices[(facePtr->vertex[3])].z;

    /*TODO optimize this school math shit*/
    angle = GetAng(&vec0, &vec2);
    angle += GetAng(&vec2, &vec3);
    angle += GetAng(&vec3, &vec1);
    angle += GetAng(&vec1, &vec0);

    if ((angle <= 30000))
    {
        return 0;
    }

    return 1;
}

void WingCollision(ShipData* ship, Face* facePtr, int direction, int alpha)
{
    vec3s collisionVector;
    int angle;
    vec3i wingPos;
    int magnitude;

    collisionVector.x = ship->nearTrkSect->centre.x - gTrack.vertices[(facePtr->vertex[2])].x;
    collisionVector.y = ship->nearTrkSect->centre.y - gTrack.vertices[(facePtr->vertex[2])].y;
    collisionVector.z = ship->nearTrkSect->centre.z - gTrack.vertices[(facePtr->vertex[2])].z;

    angle = AngleOfCollision(&collisionVector, &ship->unitVecNose);

    Reflection(&(ship->vel), &facePtr->normal);

    ship->pos.x -= ship->vel.x >> VELOCITY_SHIFT;
    ship->pos.y -= ship->vel.y >> VELOCITY_SHIFT;
    ship->pos.z -= ship->vel.z >> VELOCITY_SHIFT;

    /*TODO no slowdown by side collision*/
    ship->vel.x -= ship->vel.x >> 1;
    ship->vel.y -= ship->vel.y >> 1;
    ship->vel.z -= ship->vel.z >> 1;

    ship->vel.x += (facePtr->normal.x);
    ship->vel.y += (facePtr->normal.y);
    ship->vel.z += (facePtr->normal.z);

    magnitude = (abs(angle) * ship->speed) >> 16;

    if (direction > 0)
        ship->vroll += magnitude;
    else
        ship->vroll -= magnitude;

    if (direction > 0)
    {
        wingPos.x = ship->pos.x + (ship->unitVecWing.x >> 4) - (ship->unitVecNose.x >> 4);
        wingPos.y = ship->pos.y + (ship->unitVecWing.y >> 4) - (ship->unitVecNose.y >> 4);
        wingPos.z = ship->pos.z + (ship->unitVecWing.z >> 4) - (ship->unitVecNose.z >> 4);
    }
    else
    {
        wingPos.x = ship->pos.x - (ship->unitVecWing.x >> 4) - (ship->unitVecNose.x >> 4);
        wingPos.y = ship->pos.y - (ship->unitVecWing.y >> 4) - (ship->unitVecNose.y >> 4);
        wingPos.z = ship->pos.z - (ship->unitVecWing.z >> 4) - (ship->unitVecNose.z >> 4);
    }

    PlayDirectionalNote(SFX_SHIP_2_TRACK, &wingPos);

    /***** Magnitude returns approx between 0 & 1000    *****/
    /***** scale this and shift to a figure around zero *****/
    if (ship->player)
    {
        PlayerData* player = (PlayerData*)ship->player;
        ForceFeedback(player->joyIndex, magnitude);
    }

/*TODO audio coll volume
    magnitude = magnitude - 500;
    magnitude = magnitude << 2;

    SetVolume(SFX_SHIP_2_TRACK,
       chanTable[vagTable[SFX_SHIP_2_TRACK].channelID].s_attr.volume.left + magnitude,
       chanTable[vagTable[SFX_SHIP_2_TRACK].channelID].s_attr.volume.right + magnitude);
*/
}

void NoseCollision(ShipData* ship, Face* facePtr, int direction, int alpha)
{
    vec3s collisionVector;
    int angle;
    vec3i nosePos;
    int magnitude;

    collisionVector.x = ship->nearTrkSect->centre.x - gTrack.vertices[(facePtr->vertex[2])].x;
    collisionVector.y = ship->nearTrkSect->centre.y - gTrack.vertices[(facePtr->vertex[2])].y;
    collisionVector.z = ship->nearTrkSect->centre.z - gTrack.vertices[(facePtr->vertex[2])].z;

    angle = abs(AngleOfCollision(&collisionVector, &ship->unitVecNose));

    Reflection(&(ship->vel), &facePtr->normal);

    magnitude = (ship->speed >> 4) + 400;
    if (direction > 0)
        ship->vhdg += magnitude;
    else
        ship->vhdg -= magnitude;

    ship->pos.x -= ship->vel.x >> VELOCITY_SHIFT;
    ship->pos.y -= ship->vel.y >> VELOCITY_SHIFT;
    ship->pos.z -= ship->vel.z >> VELOCITY_SHIFT;

    ship->vel.x -= ship->vel.x >> 1;
    ship->vel.y -= ship->vel.y >> 1;
    ship->vel.z -= ship->vel.z >> 1;

    ship->vel.x += facePtr->normal.x;
    ship->vel.y += facePtr->normal.y;
    ship->vel.z += facePtr->normal.z;

    nosePos.x = ship->pos.x + (ship->unitVecNose.x >> 3);
    nosePos.y = ship->pos.y + (ship->unitVecNose.y >> 3);
    nosePos.z = ship->pos.z + (ship->unitVecNose.z >> 3);

    PlayDirectionalNote(SFX_SHIP_2_TRACK, &nosePos);

    /***** Magnitude returns approx between 0 & 1000    *****/
    /***** scale this and shift to a figure around zero *****/
    if (ship->player)
    {
        PlayerData* player = (PlayerData*)ship->player;
        ForceFeedback(player->joyIndex, magnitude);
    }

    magnitude = magnitude - 500;
    magnitude = magnitude << 2;
    /*TODO audio
    SetVolume(SFX_SHIP_2_TRACK,
       chanTable[vagTable[SFX_SHIP_2_TRACK].channelID].s_attr.volume.left + magnitude,
       chanTable[vagTable[SFX_SHIP_2_TRACK].channelID].s_attr.volume.right + magnitude);
    */
}

void Reflection(vec3i* incidence, const vec3s* normal)
{
    int32 temp = -((normal->x * incidence->x + normal->y * incidence->y + normal->z * incidence->z) >> FIXED_SHIFT);

    incidence->x = (normal->x * temp >> (FIXED_SHIFT - 1)) + incidence->x;
    incidence->y = (normal->y * temp >> (FIXED_SHIFT - 1)) + incidence->y;
    incidence->z = (normal->z * temp >> (FIXED_SHIFT - 1)) + incidence->z;
}

int AngleOfCollision(const vec3s* normal, const vec3i* ship_point)
{
    int32 dot_product;
    int32 norm_dist, shpoint_dist;
    int32 dist_product;

    int32 tmp1, tmp2;
    int32 sx, sy, sz;

    sx = ship_point->x >> 9;
    sy = ship_point->y >> 9;
    sz = ship_point->z >> 9;

    dot_product = ((sx * normal->x) + (sy * normal->y) + (sz * normal->z));

    norm_dist = rsqrt((normal->x * normal->x) + (normal->y * normal->y) + (normal->z * normal->z));

    shpoint_dist = rsqrt((sx * sx) + (sy * sy) + (sz * sz));

    dist_product = (shpoint_dist * norm_dist);

    tmp1 = dot_product << 12;
    if (dist_product != 0)
        tmp2 = (tmp1 / dist_product);
    else
        tmp2 = 0;

    return tmp2;
}
