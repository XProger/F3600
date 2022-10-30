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

    if (raceType != TIME_TRIAL)
    {
        for (j = 0; j < (NO_OF_SHIPS - 1); j++)
        {
            for (i = j + 1; i < NO_OF_SHIPS; i++)
            {
                Ship2ShipCollision(shipIndex, shipShapes, i, j);
            }
        }
    }
}

void Ship2ShipCollision(ShipData* shipIndex, Object** shipShapes, int i, int j)
{
    int mag;
    vec3i distance;
    int collide;
    vec3i vc;
    int ownMass, remMass;
    vec3i ship2ship;
    vec3i ship1React, ship2React;
    vec3i soundPos;

    distance.x = (shipIndex[i].pos.x - shipIndex[j].pos.x) >> 3;
    distance.y = (shipIndex[i].pos.y - shipIndex[j].pos.y) >> 3;
    distance.z = (shipIndex[i].pos.z - shipIndex[j].pos.z) >> 3;

    mag = GetMagnitude(&distance);

    if (mag < 120) /*** Do line/plane collision check ***/
    {
        collide = CollideLineToPlane(j, i, shipShapes, shipIndex);

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

/* ****************************************************************
Line to plane collision routine

  alpha = ((p-s).n)/(d.n)

   X = s + alpha*d

     p = one point of the poly
     s = start of the line
     n = normal
     d = direction of the line
******************************************************************** */

int CollideLineToPlane(int shipNo1, int shipNo2, Object** shipShapes, ShipData* shipIndex)
{
    int i; /* index count (planes) */
    int count; /* index count (lines) */
    Prm poly;
    short* coords;
    short primitiveCount;

    vec3i p1, p2, p3; /* the three plane points of other ship */

    vec3i a, b, c, d; /* four vector points of the hero ship */

    int factor2; /* intermediate calculation terms */
    vec3i factor1;
    vec3i term1;

    vec3i res_vec; /* resultant intersection vector */
    int alpha; /* the above alpha value */

    vec3i DC, BC, AC; /* position vectors of hero ship */
    vec3i BD, AB;
    vec3i AD;

    vec3i* start_vec; /* is assigned a, b, c or d from above */
    vec3i* pos_vec; /* is assigned DC, BC, AC, BD, AB or AD */

    vec3i plane1; /* normal of plane of other ship */

    vec3i P1P2; /* 2 position vectors of other ship */
    vec3i P1P3; /* (all that is needed to get normal) */

    vec3i vec0, vec1, vec2; /* vectors between plane coords and intersection point */
    int angle; /* total angle */

    short high1, high2;
    short low1, low2;

    high1 = shipIndex[shipNo1].highResTableIndex;
    low1 = shipIndex[shipNo1].lowResTableIndex;
    high2 = shipIndex[shipNo2].highResTableIndex;
    low2 = shipIndex[shipNo2].lowResTableIndex;

    /** Get 4 points of collision model relative to the  **/
    /**                   camera                          **/

    GetWorldPoint(&a, shipShapes[high2], shipShapes[low2], 0);
    GetWorldPoint(&b, shipShapes[high2], shipShapes[low2], 1);
    GetWorldPoint(&c, shipShapes[high2], shipShapes[low2], 2);
    GetWorldPoint(&d, shipShapes[high2], shipShapes[low2], 3);

    /**    Calculate the 6 line vectors for the model    **/

    MinusVector(&a, &c, &AC);
    MinusVector(&b, &c, &BC);
    MinusVector(&d, &c, &DC);
    MinusVector(&a, &b, &AB);
    MinusVector(&b, &d, &BD);
    MinusVector(&a, &d, &AD);

    poly.primitive = shipShapes[low2]->primitives;
    primitiveCount = shipShapes[low2]->primitiveCount;

    /*TODO optimize collision check*/
    ASSERT(shipShapes[low2]->vertexCount == 4);
    ASSERT(primitiveCount == 4);

    for (i = 0; i < primitiveCount; i++) /* for all 4 planes of the enemy ship */
    {
        ASSERT(poly.primitive->type == TypeFT3);

        coords = poly.ft3->coords;

        GetWorldPoint(&p1, shipShapes[high1], shipShapes[low1], coords[0]);
        GetWorldPoint(&p2, shipShapes[high1], shipShapes[low1], coords[1]);
        GetWorldPoint(&p3, shipShapes[high1], shipShapes[low1], coords[2]);

        poly.ft3++;

        /* ********************************************** */
        /*        Find polyGon line vectors                */
        /* ********************************************** */
        MinusVector(&p1, &p2, &P1P2);
        MinusVector(&p1, &p3, &P1P3);

        /* ********************************************** */
        /*             Find plane equations                */
        /* ********************************************** */
        CrossProduct(&plane1, &P1P2, &P1P3);

        for (count = 0; count < 6; count++)
        {
            switch (count)
            {
                case 0:
                    start_vec = &b;
                    pos_vec = &BC;
                    break;
                case 1:
                    start_vec = &a;
                    pos_vec = &AC;
                    break;
                case 2:
                    start_vec = &d;
                    pos_vec = &DC;
                    break;
                case 3:
                    start_vec = &a;
                    pos_vec = &AB;
                    break;
                case 4:
                    start_vec = &a;
                    pos_vec = &AD;
                    break;
                default: // 5
                    start_vec = &b;
                    pos_vec = &BD;
                    break;
            }

            MinusVector(start_vec, &p1, &factor1); /* (p-s) */
            alpha = (ScalarProduct(&factor1, &plane1) << 12); /* (p-s).n */
            factor2 = (ScalarProduct(pos_vec, &plane1)); /* d.n */
            if (factor2 != 0)
            {
                alpha = alpha / factor2;

                term1.x = ((alpha * pos_vec->x) >> 12);
                term1.y = ((alpha * pos_vec->y) >> 12);
                term1.z = ((alpha * pos_vec->z) >> 12);

                res_vec.x = start_vec->x + term1.x;
                res_vec.y = start_vec->y + term1.y;
                res_vec.z = start_vec->z + term1.z;

                if ((alpha >= 0) && (alpha <= 4096))
                {
                    vec0.x = res_vec.x - p1.x;
                    vec0.y = res_vec.y - p1.y;
                    vec0.z = res_vec.z - p1.z;

                    vec1.x = res_vec.x - p2.x;
                    vec1.y = res_vec.y - p2.y;
                    vec1.z = res_vec.z - p2.z;

                    vec2.x = res_vec.x - p3.x;
                    vec2.y = res_vec.y - p3.y;
                    vec2.z = res_vec.z - p3.z;

                    angle = GetAng(&vec0, &vec1);
                    angle += GetAng(&vec1, &vec2);
                    angle += GetAng(&vec2, &vec0);

                    if ((angle >= 30000))
                    {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int PlaneLineCollision(const vec3i* plane_point, const vec3i* ship_pos, const vec3s* normal)
{
    int dot_product, norm_dot_product;

    dot_product = (ship_pos->x - plane_point->x) * normal->x +
                  (ship_pos->y - plane_point->y) * normal->y +
                  (ship_pos->z - plane_point->z) * normal->z;

    norm_dot_product = ((normal->x * normal->x) + (normal->y * normal->y) + (normal->z * normal->z)) >> 12;

    return dot_product / norm_dot_product;
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

    toFaceVector.x = track->vertices[(facePtr->vertex[0])].x - track->vertices[(facePtr->vertex[1])].x;
    toFaceVector.y = track->vertices[(facePtr->vertex[0])].y - track->vertices[(facePtr->vertex[1])].y;
    toFaceVector.z = track->vertices[(facePtr->vertex[0])].z - track->vertices[(facePtr->vertex[1])].z;

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
            facePoint = track->vertices + facePtr->vertex[0];

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
                        facePtr2 = track->faces + playerShip->nearTrkSect->nextSection->faceStart;
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
                        facePtr2 = track->faces + playerShip->nearTrkSect->prevSection->faceStart;
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

            facePoint = track->vertices + facePtr->vertex[0];

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
                        facePtr2 = track->faces + playerShip->nearTrkSect->nextSection->faceStart;
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
                        facePtr2 = track->faces + playerShip->nearTrkSect->prevSection->faceStart;
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

    planePoint.x = pos->x - ((facePtr->normal.x * alpha) >> 12);
    planePoint.y = pos->y - ((facePtr->normal.y * alpha) >> 12);
    planePoint.z = pos->z - ((facePtr->normal.z * alpha) >> 12);

    vec0.x = planePoint.x - track->vertices[(facePtr->vertex[1])].x;
    vec0.y = planePoint.y - track->vertices[(facePtr->vertex[1])].y;
    vec0.z = planePoint.z - track->vertices[(facePtr->vertex[1])].z;

    vec1.x = planePoint.x - track->vertices[(facePtr->vertex[2])].x;
    vec1.y = planePoint.y - track->vertices[(facePtr->vertex[2])].y;
    vec1.z = planePoint.z - track->vertices[(facePtr->vertex[2])].z;

    vec2.x = planePoint.x - track->vertices[(facePtr->vertex[0])].x;
    vec2.y = planePoint.y - track->vertices[(facePtr->vertex[0])].y;
    vec2.z = planePoint.z - track->vertices[(facePtr->vertex[0])].z;

    vec3.x = planePoint.x - track->vertices[(facePtr->vertex[3])].x;
    vec3.y = planePoint.y - track->vertices[(facePtr->vertex[3])].y;
    vec3.z = planePoint.z - track->vertices[(facePtr->vertex[3])].z;

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

    collisionVector.x = playerShip->nearTrkSect->centre.x - track->vertices[(facePtr->vertex[2])].x;
    collisionVector.y = playerShip->nearTrkSect->centre.y - track->vertices[(facePtr->vertex[2])].y;
    collisionVector.z = playerShip->nearTrkSect->centre.z - track->vertices[(facePtr->vertex[2])].z;

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
    ForceFeedback(magnitude); // TODO ownShip check

    magnitude = magnitude - 500;
    magnitude = magnitude << 2;
    /*TODO audio
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

    collisionVector.x = playerShip->nearTrkSect->centre.x - track->vertices[(facePtr->vertex[2])].x;
    collisionVector.y = playerShip->nearTrkSect->centre.y - track->vertices[(facePtr->vertex[2])].y;
    collisionVector.z = playerShip->nearTrkSect->centre.z - track->vertices[(facePtr->vertex[2])].z;

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
    int temp;

    temp = ((((int)normal->x * -incidence->x) >> 12) + ((int)(normal->y * -incidence->y) >> 12) + ((int)(normal->z * -incidence->z) >> 12));

    incidence->x = (((2 * (int)normal->x) * temp) >> 12) + incidence->x;
    incidence->y = (((2 * (int)normal->y) * temp) >> 12) + incidence->y;
    incidence->z = (((2 * (int)normal->z) * temp) >> 12) + incidence->z;
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
