#include "game.h"

extern long ctrlmode;

void ForceFeedback(int mag)
{
    int rumble;

    if (ctrlmode == ATTRACT)
        return;

    rumble = mag >> 3;
    if (rumble > 255)
    {
        rumble = 255;
    }

    osJoyVibrate(0, rumble, rumble);
}

void CallShip2ShipCollision(ShipData* shipIndex, Object** shipShapes)
{
    int i, j;

    for (j = 0; j < (NO_OF_SHIPS - 1); j++)
    {
        for (i = j + 1; i < NO_OF_SHIPS; i++)
        {
            Ship2ShipCollision(shipIndex, shipShapes, i, j);
        }
    }
}

void Ship2ShipCollision(ShipData* shipIndex, Object** shipShapes, int i, int j)
{
    vec3i distance;
    int collide;
    vec3i vc;
    vec3i ship2ship;
    vec3i ship1React, ship2React;
    vec3i soundPos;

    distance.x = (shipIndex[i].pos.x - shipIndex[j].pos.x) >> 3;
    distance.y = (shipIndex[i].pos.y - shipIndex[j].pos.y) >> 3;
    distance.z = (shipIndex[i].pos.z - shipIndex[j].pos.z) >> 3;

    if (GetMagnitudeSqr(&distance) < (40 * 40)) 
    {
        collide = NOSE_COLLISION;

        if (collide == NOSE_COLLISION)
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

            vc.x = (shipIndex[j].vel.x + shipIndex[i].vel.x) >> 1;
            vc.y = (shipIndex[j].vel.y + shipIndex[i].vel.y) >> 1;
            vc.z = (shipIndex[j].vel.z + shipIndex[i].vel.z) >> 1;

            ship1React.x = (vc.x - shipIndex[j].vel.x) >> 1;
            ship1React.y = (vc.y - shipIndex[j].vel.y) >> 1;
            ship1React.z = (vc.z - shipIndex[j].vel.z) >> 1;

            ship2React.x = (vc.x - shipIndex[i].vel.x) >> 1;
            ship2React.y = (vc.y - shipIndex[i].vel.y) >> 1;
            ship2React.z = (vc.z - shipIndex[i].vel.z) >> 1;

            shipIndex[j].pos.x -= shipIndex[j].vel.x >> VELOCITY_SHIFT;
            shipIndex[j].pos.y -= shipIndex[j].vel.y >> VELOCITY_SHIFT;
            shipIndex[j].pos.z -= shipIndex[j].vel.z >> VELOCITY_SHIFT;

            shipIndex[i].pos.x -= shipIndex[i].vel.x >> VELOCITY_SHIFT;
            shipIndex[i].pos.y -= shipIndex[i].vel.y >> VELOCITY_SHIFT;
            shipIndex[i].pos.z -= shipIndex[i].vel.z >> VELOCITY_SHIFT;

            /**************** YOUR CRAFT REACTION *********************/
            shipIndex[j].vel.x = vc.x + ship1React.x;
            shipIndex[j].vel.y = vc.y + ship1React.y;
            shipIndex[j].vel.z = vc.z + ship1React.z;

            /*************** REMOTE CRAFT REACTION ********************/
            shipIndex[i].vel.x = vc.x + ship2React.x;
            shipIndex[i].vel.y = vc.y + ship2React.y;
            shipIndex[i].vel.z = vc.z + ship2React.z;

            ship2ship.x = shipIndex[i].pos.x - shipIndex[j].pos.x;
            ship2ship.y = shipIndex[i].pos.y - shipIndex[j].pos.y;
            ship2ship.z = shipIndex[i].pos.z - shipIndex[j].pos.z;

            shipIndex[i].vel.x += ship2ship.x << 2;
            shipIndex[i].vel.y += ship2ship.y << 2;
            shipIndex[i].vel.z += ship2ship.z << 2;

            shipIndex[j].vel.x -= ship2ship.x << 2;
            shipIndex[j].vel.y -= ship2ship.y << 2;
            shipIndex[j].vel.z -= ship2ship.z << 2;

            shipIndex[j].pos.x += shipIndex[j].vel.x >> VELOCITY_SHIFT;
            shipIndex[j].pos.y += shipIndex[j].vel.y >> VELOCITY_SHIFT;
            shipIndex[j].pos.z += shipIndex[j].vel.z >> VELOCITY_SHIFT;

            shipIndex[i].pos.x += shipIndex[i].vel.x >> VELOCITY_SHIFT;
            shipIndex[i].pos.y += shipIndex[i].vel.y >> VELOCITY_SHIFT;
            shipIndex[i].pos.z += shipIndex[i].vel.z >> VELOCITY_SHIFT;

            if ((shipIndex[i].attr & SHIP_COLL) == 0)
            {
                if ((shipIndex[j].attr & SHIP_COLL) == 0)
                {
                    soundPos.x = (shipIndex[i].pos.x + shipIndex[j].pos.x) >> 1;
                    soundPos.y = (shipIndex[i].pos.y + shipIndex[j].pos.y) >> 1;
                    soundPos.z = (shipIndex[i].pos.z + shipIndex[j].pos.z) >> 1;
                    PlayDirectionalNote(SFX_SHIP_2_SHIP, &soundPos);

                    if (j == ownShip)
                    {
                        ForceFeedback(255);
                    }
                }
            }
            shipIndex[i].attr |= SHIP_COLL;
            shipIndex[j].attr |= SHIP_COLL;
        }
    }
    else
    {
        shipIndex[i].attr &= ~SHIP_COLL;
        shipIndex[j].attr &= ~SHIP_COLL;
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

void Ship2Track(ShipData* playerShip, Face* facePtr)
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

    nosePos.x = playerShip->pos.x + (playerShip->unitVecNose.x >> 3);
    nosePos.y = playerShip->pos.y + (playerShip->unitVecNose.y >> 3);
    nosePos.z = playerShip->pos.z + (playerShip->unitVecNose.z >> 3);

    wingPos.x = playerShip->pos.x - (playerShip->unitVecWing.x >> 4) - (playerShip->unitVecNose.x >> 4);
    wingPos.y = playerShip->pos.y - (playerShip->unitVecWing.y >> 4) - (playerShip->unitVecNose.y >> 4);
    wingPos.z = playerShip->pos.z - (playerShip->unitVecWing.z >> 4) - (playerShip->unitVecNose.z >> 4);

    trackPtr = playerShip->nearTrkSect->nextSection;

    direction.x = trackPtr->centre.x - playerShip->nearTrkSect->centre.x;
    direction.y = trackPtr->centre.y - playerShip->nearTrkSect->centre.y;
    direction.z = trackPtr->centre.z - playerShip->nearTrkSect->centre.z;

    downTrack = ((direction.x * playerShip->unitVecNose.x) + (direction.y * playerShip->unitVecNose.y) + (direction.z * playerShip->unitVecNose.z));

    if (downTrack < 0)
        playerShip->attr &= ~DIRECTION;
    else
        playerShip->attr |= DIRECTION;

    toFaceVector.x = gTrack.vertices[(facePtr->vertex[0])].x - gTrack.vertices[(facePtr->vertex[1])].x;
    toFaceVector.y = gTrack.vertices[(facePtr->vertex[0])].y - gTrack.vertices[(facePtr->vertex[1])].y;
    toFaceVector.z = gTrack.vertices[(facePtr->vertex[0])].z - gTrack.vertices[(facePtr->vertex[1])].z;

    direction.x = playerShip->nearTrkSect->centre.x - playerShip->pos.x;
    direction.y = playerShip->nearTrkSect->centre.y - playerShip->pos.y;
    direction.z = playerShip->nearTrkSect->centre.z - playerShip->pos.z;

    toFace = ((direction.x * toFaceVector.x) + (direction.y * toFaceVector.y) + (direction.z * toFaceVector.z));

    facePtr--;

    /******** Check against left hand side of track ********/

    if (toFace > 0)
    {
        playerShip->attr |= LSIDE;
        if (1) //(facePtr == (track->faces + playerShip->nearTrkSect->faceStart)) || playerShip->nearTrkSect->junction)
        {
            facePoint = gTrack.vertices + facePtr->vertex[0];

            alpha = PlaneLineCollision(facePoint, &nosePos, &facePtr->normal);

            if (alpha <= 0)
            {
                if (playerShip->nearTrkSect->flags & JunctionStart)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, -downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + playerShip->nearTrkSect->nextSection->faceStart;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(playerShip, facePtr, -downTrack, alpha);
                        }
                    }
                }
                else if (playerShip->nearTrkSect->flags & JunctionEnd)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, -downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + playerShip->nearTrkSect->prevSection->faceStart;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(playerShip, facePtr, -downTrack, alpha);
                        }
                    }
                }
                else
                    NoseCollision(playerShip, facePtr, -downTrack, alpha);
                return;
            }

            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);

            if (alpha <= 0)
            {
                if ((playerShip->nearTrkSect->flags & JunctionStart) || (playerShip->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, -downTrack, alpha);
                    }
                }
                else
                    WingCollision(playerShip, facePtr, -downTrack, alpha);
                return;
            }

            wingPos.x = playerShip->pos.x + (playerShip->unitVecWing.x >> 4) - (playerShip->unitVecNose.x >> 4);
            wingPos.y = playerShip->pos.y + (playerShip->unitVecWing.y >> 4) - (playerShip->unitVecNose.y >> 4);
            wingPos.z = playerShip->pos.z + (playerShip->unitVecWing.z >> 4) - (playerShip->unitVecNose.z >> 4);

            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);
            if (alpha <= 0)
            {
                if ((playerShip->nearTrkSect->flags & JunctionStart) || (playerShip->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, -downTrack, alpha);
                    }
                }
                else
                    WingCollision(playerShip, facePtr, -downTrack, alpha);
                return;
            }
        }
    }
    /****************************************************************************/
    /*****                  Collision check against 2nd wall                *****/
    /****************************************************************************/
    else
    {
        playerShip->attr &= ~LSIDE;

        facePtr++;
        while (facePtr->flags & TRACK_BASE)
        {
            facePtr++;
        }

        if ((playerShip->nearTrkSect->faceCount > 3) || playerShip->nearTrkSect->junction)
        {

            facePoint = gTrack.vertices + facePtr->vertex[0];

            alpha = PlaneLineCollision(facePoint, &nosePos, &facePtr->normal);
            if (alpha <= 0)
            {
                if (playerShip->nearTrkSect->flags & JunctionStart)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + playerShip->nearTrkSect->nextSection->faceStart;
                        facePtr2 += 3;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(playerShip, facePtr, -downTrack, alpha);
                        }
                    }
                }
                else if (playerShip->nearTrkSect->flags & JunctionEnd)
                {
                    collide = CheckPolygonCollision(facePtr, &nosePos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, -downTrack, alpha);
                    }
                    else
                    {
                        facePtr2 = gTrack.faces + playerShip->nearTrkSect->prevSection->faceStart;
                        facePtr2 += 3;
                        collide = CheckPolygonCollision(facePtr2, &nosePos, alpha);
                        if (collide == 1)
                        {
                            NoseCollision(playerShip, facePtr2, -downTrack, alpha);
                        }
                    }
                }
                else
                    NoseCollision(playerShip, facePtr, downTrack, alpha);
                return;
            }
            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);
            if (alpha <= 0)
            {
                if ((playerShip->nearTrkSect->flags & JunctionStart) || (playerShip->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, downTrack, alpha);
                    }
                }
                else
                    WingCollision(playerShip, facePtr, downTrack, alpha);
                return;
            }

            wingPos.x = playerShip->pos.x + (playerShip->unitVecWing.x >> 4) - (playerShip->unitVecNose.x >> 4);
            wingPos.y = playerShip->pos.y + (playerShip->unitVecWing.y >> 4) - (playerShip->unitVecNose.y >> 4);
            wingPos.z = playerShip->pos.z + (playerShip->unitVecWing.z >> 4) - (playerShip->unitVecNose.z >> 4);
            alpha = PlaneLineCollision(facePoint, &wingPos, &facePtr->normal);
            if (alpha <= 0)
            {
                if ((playerShip->nearTrkSect->flags & JunctionStart) || (playerShip->nearTrkSect->flags & JunctionEnd))
                {
                    collide = CheckPolygonCollision(facePtr, &wingPos, alpha);
                    if (collide == 1)
                    {
                        NoseCollision(playerShip, facePtr, downTrack, alpha);
                    }
                }
                else
                    WingCollision(playerShip, facePtr, downTrack, alpha);
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

void WingCollision(ShipData* playerShip, Face* facePtr, int direction, int alpha)
{
    vec3s collisionVector;
    int angle;
    vec3i wingPos;
    int magnitude;

    collisionVector.x = playerShip->nearTrkSect->centre.x - gTrack.vertices[(facePtr->vertex[2])].x;
    collisionVector.y = playerShip->nearTrkSect->centre.y - gTrack.vertices[(facePtr->vertex[2])].y;
    collisionVector.z = playerShip->nearTrkSect->centre.z - gTrack.vertices[(facePtr->vertex[2])].z;

    angle = AngleOfCollision(&collisionVector, &playerShip->unitVecNose);

    Reflection(&(playerShip->vel), &facePtr->normal);

    playerShip->pos.x -= playerShip->vel.x >> VELOCITY_SHIFT;
    playerShip->pos.y -= playerShip->vel.y >> VELOCITY_SHIFT;
    playerShip->pos.z -= playerShip->vel.z >> VELOCITY_SHIFT;

    /*TODO no slowdown by side collision*/
    playerShip->vel.x -= playerShip->vel.x >> 1;
    playerShip->vel.y -= playerShip->vel.y >> 1;
    playerShip->vel.z -= playerShip->vel.z >> 1;

    playerShip->vel.x += (facePtr->normal.x);
    playerShip->vel.y += (facePtr->normal.y);
    playerShip->vel.z += (facePtr->normal.z);

    magnitude = (abs(angle) * playerShip->speed) >> 16;

    if (direction > 0)
        playerShip->vroll += magnitude;
    else
        playerShip->vroll -= magnitude;

    if (direction > 0)
    {
        wingPos.x = playerShip->pos.x + (playerShip->unitVecWing.x >> 4) - (playerShip->unitVecNose.x >> 4);
        wingPos.y = playerShip->pos.y + (playerShip->unitVecWing.y >> 4) - (playerShip->unitVecNose.y >> 4);
        wingPos.z = playerShip->pos.z + (playerShip->unitVecWing.z >> 4) - (playerShip->unitVecNose.z >> 4);
    }
    else
    {
        wingPos.x = playerShip->pos.x - (playerShip->unitVecWing.x >> 4) - (playerShip->unitVecNose.x >> 4);
        wingPos.y = playerShip->pos.y - (playerShip->unitVecWing.y >> 4) - (playerShip->unitVecNose.y >> 4);
        wingPos.z = playerShip->pos.z - (playerShip->unitVecWing.z >> 4) - (playerShip->unitVecNose.z >> 4);
    }

    PlayDirectionalNote(SFX_SHIP_2_TRACK, &wingPos);

    /***** Magnitude returns approx between 0 & 1000    *****/
    /***** scale this and shift to a figure around zero *****/
    ForceFeedback(magnitude);

/*TODO audio coll volume
    magnitude = magnitude - 500;
    magnitude = magnitude << 2;

    SetVolume(SFX_SHIP_2_TRACK,
       chanTable[vagTable[SFX_SHIP_2_TRACK].channelID].s_attr.volume.left + magnitude,
       chanTable[vagTable[SFX_SHIP_2_TRACK].channelID].s_attr.volume.right + magnitude);
*/
}

void NoseCollision(ShipData* playerShip, Face* facePtr, int direction, int alpha)
{
    vec3s collisionVector;
    int angle;
    vec3i nosePos;
    int magnitude;

    collisionVector.x = playerShip->nearTrkSect->centre.x - gTrack.vertices[(facePtr->vertex[2])].x;
    collisionVector.y = playerShip->nearTrkSect->centre.y - gTrack.vertices[(facePtr->vertex[2])].y;
    collisionVector.z = playerShip->nearTrkSect->centre.z - gTrack.vertices[(facePtr->vertex[2])].z;

    angle = abs(AngleOfCollision(&collisionVector, &playerShip->unitVecNose));

    Reflection(&(playerShip->vel), &facePtr->normal);

    magnitude = (playerShip->speed >> 4) + 400;
    if (direction > 0)
        playerShip->vhdg += magnitude;
    else
        playerShip->vhdg -= magnitude;

    playerShip->pos.x -= playerShip->vel.x >> VELOCITY_SHIFT;
    playerShip->pos.y -= playerShip->vel.y >> VELOCITY_SHIFT;
    playerShip->pos.z -= playerShip->vel.z >> VELOCITY_SHIFT;

    playerShip->vel.x -= playerShip->vel.x >> 1;
    playerShip->vel.y -= playerShip->vel.y >> 1;
    playerShip->vel.z -= playerShip->vel.z >> 1;

    playerShip->vel.x += facePtr->normal.x;
    playerShip->vel.y += facePtr->normal.y;
    playerShip->vel.z += facePtr->normal.z;

    nosePos.x = playerShip->pos.x + (playerShip->unitVecNose.x >> 3);
    nosePos.y = playerShip->pos.y + (playerShip->unitVecNose.y >> 3);
    nosePos.z = playerShip->pos.z + (playerShip->unitVecNose.z >> 3);

    PlayDirectionalNote(SFX_SHIP_2_TRACK, &nosePos);

    /***** Magnitude returns approx between 0 & 1000    *****/
    /***** scale this and shift to a figure around zero *****/

    ForceFeedback(magnitude); // TODO ownShip check

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
    int dot_product;
    int norm_dist, shpoint_dist;
    int dist_product;

    int tmp1, tmp2;
    long sx, sy, sz;

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
