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
    VECTOR distance;
    int collide;
    VECTOR vc;
    int ownMass, remMass;
    VECTOR ship2ship;
    VECTOR ship1React, ship2React;
    VECTOR soundPos;

    distance.vx = (shipIndex[i].pos.vx - shipIndex[j].pos.vx) >> 3;
    distance.vy = (shipIndex[i].pos.vy - shipIndex[j].pos.vy) >> 3;
    distance.vz = (shipIndex[i].pos.vz - shipIndex[j].pos.vz) >> 3;

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

            ownMass = shipIndex[j].mass;
            remMass = shipIndex[i].mass;

            vc.vx = ((shipIndex[j].vel.vx * ownMass) + (shipIndex[i].vel.vx * remMass)) / (ownMass + remMass);
            vc.vy = ((shipIndex[j].vel.vy * ownMass) + (shipIndex[i].vel.vy * remMass)) / (ownMass + remMass);
            vc.vz = ((shipIndex[j].vel.vz * ownMass) + (shipIndex[i].vel.vz * remMass)) / (ownMass + remMass);

            ship1React.vx = (vc.vx - shipIndex[j].vel.vx) >> 1;
            ship1React.vy = (vc.vy - shipIndex[j].vel.vy) >> 1;
            ship1React.vz = (vc.vz - shipIndex[j].vel.vz) >> 1;

            ship2React.vx = (vc.vx - shipIndex[i].vel.vx) >> 1;
            ship2React.vy = (vc.vy - shipIndex[i].vel.vy) >> 1;
            ship2React.vz = (vc.vz - shipIndex[i].vel.vz) >> 1;

            shipIndex[j].pos.vx -= sar(shipIndex[j].vel.vx, 6);
            shipIndex[j].pos.vy -= sar(shipIndex[j].vel.vy, 6);
            shipIndex[j].pos.vz -= sar(shipIndex[j].vel.vz, 6);

            shipIndex[i].pos.vx -= sar(shipIndex[i].vel.vx, 6);
            shipIndex[i].pos.vy -= sar(shipIndex[i].vel.vy, 6);
            shipIndex[i].pos.vz -= sar(shipIndex[i].vel.vz, 6);

            /**************** YOUR CRAFT REACTION *********************/
            shipIndex[j].vel.vx = vc.vx + ship1React.vx;
            shipIndex[j].vel.vy = vc.vy + ship1React.vy;
            shipIndex[j].vel.vz = vc.vz + ship1React.vz;

            /*************** REMOTE CRAFT REACTION ********************/
            shipIndex[i].vel.vx = vc.vx + ship2React.vx;
            shipIndex[i].vel.vy = vc.vy + ship2React.vy;
            shipIndex[i].vel.vz = vc.vz + ship2React.vz;

            ship2ship.vx = shipIndex[i].pos.vx - shipIndex[j].pos.vx;
            ship2ship.vy = shipIndex[i].pos.vy - shipIndex[j].pos.vy;
            ship2ship.vz = shipIndex[i].pos.vz - shipIndex[j].pos.vz;

            shipIndex[i].vel.vx += ship2ship.vx << 2;
            shipIndex[i].vel.vy += ship2ship.vy << 2;
            shipIndex[i].vel.vz += ship2ship.vz << 2;

            shipIndex[j].vel.vx -= ship2ship.vx << 2;
            shipIndex[j].vel.vy -= ship2ship.vy << 2;
            shipIndex[j].vel.vz -= ship2ship.vz << 2;

            shipIndex[j].vel.vx = (shipIndex[j].vel.vx * 60) / FR60;
            shipIndex[j].vel.vy = (shipIndex[j].vel.vy * 60) / FR60;
            shipIndex[j].vel.vz = (shipIndex[j].vel.vz * 60) / FR60;

            shipIndex[i].vel.vx = (shipIndex[i].vel.vx * 60) / FR60;
            shipIndex[i].vel.vy = (shipIndex[i].vel.vy * 60) / FR60;
            shipIndex[i].vel.vz = (shipIndex[i].vel.vz * 60) / FR60;

            shipIndex[j].pos.vx += sar(shipIndex[j].vel.vx, 6);
            shipIndex[j].pos.vy += sar(shipIndex[j].vel.vy, 6);
            shipIndex[j].pos.vz += sar(shipIndex[j].vel.vz, 6);

            shipIndex[i].pos.vx += sar(shipIndex[i].vel.vx, 6);
            shipIndex[i].pos.vy += sar(shipIndex[i].vel.vy, 6);
            shipIndex[i].pos.vz += sar(shipIndex[i].vel.vz, 6);

            if ((shipIndex[i].attr & SHIP_COLL) == 0)
            {
                if ((shipIndex[j].attr & SHIP_COLL) == 0)
                {
                    soundPos.vx = (shipIndex[i].pos.vx + shipIndex[j].pos.vx) >> 1;
                    soundPos.vy = (shipIndex[i].pos.vy + shipIndex[j].pos.vy) >> 1;
                    soundPos.vz = (shipIndex[i].pos.vz + shipIndex[j].pos.vz) >> 1;
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

    VECTOR p1, p2, p3; /* the three plane points of other ship */

    VECTOR a, b, c, d; /* four vector points of the hero ship */

    int factor2; /* intermediate calculation terms */
    VECTOR factor1;
    VECTOR term1;

    VECTOR res_vec; /* resultant intersection vector */
    int alpha; /* the above alpha value */

    VECTOR DC, BC, AC; /* position vectors of hero ship */
    VECTOR BD, AB;
    VECTOR AD;

    VECTOR* start_vec; /* is assigned a, b, c or d from above */
    VECTOR* pos_vec; /* is assigned DC, BC, AC, BD, AB or AD */

    VECTOR plane1; /* normal of plane of other ship */

    VECTOR P1P2; /* 2 position vectors of other ship */
    VECTOR P1P3; /* (all that is needed to get normal) */

    VECTOR vec0, vec1, vec2; /* vectors between plane coords and intersection point */
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

                term1.vx = ((alpha * pos_vec->vx) >> 12);
                term1.vy = ((alpha * pos_vec->vy) >> 12);
                term1.vz = ((alpha * pos_vec->vz) >> 12);

                res_vec.vx = start_vec->vx + term1.vx;
                res_vec.vy = start_vec->vy + term1.vy;
                res_vec.vz = start_vec->vz + term1.vz;

                if ((alpha >= 0) && (alpha <= 4096))
                {
                    vec0.vx = res_vec.vx - p1.vx;
                    vec0.vy = res_vec.vy - p1.vy;
                    vec0.vz = res_vec.vz - p1.vz;

                    vec1.vx = res_vec.vx - p2.vx;
                    vec1.vy = res_vec.vy - p2.vy;
                    vec1.vz = res_vec.vz - p2.vz;

                    vec2.vx = res_vec.vx - p3.vx;
                    vec2.vy = res_vec.vy - p3.vy;
                    vec2.vz = res_vec.vz - p3.vz;

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

int PlaneLineCollision(const VECTOR* plane_point, const VECTOR* ship_pos, const SVector* normal)
{
    int dot_product, norm_dot_product;

    dot_product = (ship_pos->vx - plane_point->vx) * normal->vx +
                  (ship_pos->vy - plane_point->vy) * normal->vy +
                  (ship_pos->vz - plane_point->vz) * normal->vz;

    norm_dot_product = ((normal->vx * normal->vx) + (normal->vy * normal->vy) + (normal->vz * normal->vz)) >> 12;

    return dot_product / norm_dot_product;
}

void Ship2Track(ShipData* playerShip, Face* facePtr)
{
    VECTOR nosePos, wingPos;
    int alpha;
    VECTOR direction;
    int downTrack;
    TrackSection* trackPtr;
    int toFace;
    VECTOR toFaceVector;
    const VECTOR* facePoint;
    int collide;
    Face* facePtr2;

    nosePos.vx = playerShip->pos.vx + (playerShip->unitVecNose.vx >> 3);
    nosePos.vy = playerShip->pos.vy + (playerShip->unitVecNose.vy >> 3);
    nosePos.vz = playerShip->pos.vz + (playerShip->unitVecNose.vz >> 3);

    wingPos.vx = playerShip->pos.vx - (playerShip->unitVecWing.vx >> 4) - (playerShip->unitVecNose.vx >> 4);
    wingPos.vy = playerShip->pos.vy - (playerShip->unitVecWing.vy >> 4) - (playerShip->unitVecNose.vy >> 4);
    wingPos.vz = playerShip->pos.vz - (playerShip->unitVecWing.vz >> 4) - (playerShip->unitVecNose.vz >> 4);

    trackPtr = playerShip->nearTrkSect->nextSection;

    direction.vx = trackPtr->centre.vx - playerShip->nearTrkSect->centre.vx;
    direction.vy = trackPtr->centre.vy - playerShip->nearTrkSect->centre.vy;
    direction.vz = trackPtr->centre.vz - playerShip->nearTrkSect->centre.vz;

    downTrack = ((direction.vx * playerShip->unitVecNose.vx) + (direction.vy * playerShip->unitVecNose.vy) + (direction.vz * playerShip->unitVecNose.vz));

    if (downTrack < 0)
        playerShip->attr &= ~DIRECTION;
    else
        playerShip->attr |= DIRECTION;

    toFaceVector.vx = track->vertices[(facePtr->vertex[0])].vx - track->vertices[(facePtr->vertex[1])].vx;
    toFaceVector.vy = track->vertices[(facePtr->vertex[0])].vy - track->vertices[(facePtr->vertex[1])].vy;
    toFaceVector.vz = track->vertices[(facePtr->vertex[0])].vz - track->vertices[(facePtr->vertex[1])].vz;

    direction.vx = playerShip->nearTrkSect->centre.vx - playerShip->pos.vx;
    direction.vy = playerShip->nearTrkSect->centre.vy - playerShip->pos.vy;
    direction.vz = playerShip->nearTrkSect->centre.vz - playerShip->pos.vz;

    toFace = ((direction.vx * toFaceVector.vx) + (direction.vy * toFaceVector.vy) + (direction.vz * toFaceVector.vz));

    facePtr--;

    /******** Check against left hand side of track ********/

    if (toFace > 0)
    {
        playerShip->attr |= LSIDE;
        if (1) //(facePtr == (track->faces + playerShip->nearTrkSect->faceStart)) || playerShip->nearTrkSect->junction) // TODO og bug, possible optimization?
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

            wingPos.vx = playerShip->pos.vx + (playerShip->unitVecWing.vx >> 4) - (playerShip->unitVecNose.vx >> 4);
            wingPos.vy = playerShip->pos.vy + (playerShip->unitVecWing.vy >> 4) - (playerShip->unitVecNose.vy >> 4);
            wingPos.vz = playerShip->pos.vz + (playerShip->unitVecWing.vz >> 4) - (playerShip->unitVecNose.vz >> 4);

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

        if (1) //(playerShip->nearTrkSect->faceCount > 3) || playerShip->nearTrkSect->junction) // TODO og bug, possible optimization?
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

            wingPos.vx = playerShip->pos.vx + (playerShip->unitVecWing.vx >> 4) - (playerShip->unitVecNose.vx >> 4);
            wingPos.vy = playerShip->pos.vy + (playerShip->unitVecWing.vy >> 4) - (playerShip->unitVecNose.vy >> 4);
            wingPos.vz = playerShip->pos.vz + (playerShip->unitVecWing.vz >> 4) - (playerShip->unitVecNose.vz >> 4);
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

int CheckPolygonCollision(const Face* facePtr, const VECTOR* pos, int alpha)
{
    VECTOR planePoint;
    VECTOR vec0, vec1, vec2, vec3; /* vectors between plane coords and intersection point */
    int angle;

    planePoint.vx = pos->vx - ((facePtr->normal.vx * alpha) >> 12);
    planePoint.vy = pos->vy - ((facePtr->normal.vy * alpha) >> 12);
    planePoint.vz = pos->vz - ((facePtr->normal.vz * alpha) >> 12);

    vec0.vx = planePoint.vx - track->vertices[(facePtr->vertex[1])].vx;
    vec0.vy = planePoint.vy - track->vertices[(facePtr->vertex[1])].vy;
    vec0.vz = planePoint.vz - track->vertices[(facePtr->vertex[1])].vz;

    vec1.vx = planePoint.vx - track->vertices[(facePtr->vertex[2])].vx;
    vec1.vy = planePoint.vy - track->vertices[(facePtr->vertex[2])].vy;
    vec1.vz = planePoint.vz - track->vertices[(facePtr->vertex[2])].vz;

    vec2.vx = planePoint.vx - track->vertices[(facePtr->vertex[0])].vx;
    vec2.vy = planePoint.vy - track->vertices[(facePtr->vertex[0])].vy;
    vec2.vz = planePoint.vz - track->vertices[(facePtr->vertex[0])].vz;

    vec3.vx = planePoint.vx - track->vertices[(facePtr->vertex[3])].vx;
    vec3.vy = planePoint.vy - track->vertices[(facePtr->vertex[3])].vy;
    vec3.vz = planePoint.vz - track->vertices[(facePtr->vertex[3])].vz;

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
    SVECTOR collisionVector;
    int angle;
    VECTOR wingPos;
    int magnitude;

    collisionVector.vx = playerShip->nearTrkSect->centre.vx - track->vertices[(facePtr->vertex[2])].vx;
    collisionVector.vy = playerShip->nearTrkSect->centre.vy - track->vertices[(facePtr->vertex[2])].vy;
    collisionVector.vz = playerShip->nearTrkSect->centre.vz - track->vertices[(facePtr->vertex[2])].vz;

    angle = AngleOfCollision(&collisionVector, &playerShip->unitVecNose);

    Reflection(&(playerShip->vel), &facePtr->normal);

    playerShip->pos.vx -= playerShip->vel.vx >> 6;
    playerShip->pos.vy -= playerShip->vel.vy >> 6;
    playerShip->pos.vz -= playerShip->vel.vz >> 6;

    /*TODO no slowdown by side collision*/
    playerShip->vel.vx -= playerShip->vel.vx >> 1;
    playerShip->vel.vy -= playerShip->vel.vy >> 1;
    playerShip->vel.vz -= playerShip->vel.vz >> 1;

    playerShip->vel.vx += (facePtr->normal.vx);
    playerShip->vel.vy += (facePtr->normal.vy);
    playerShip->vel.vz += (facePtr->normal.vz);

    magnitude = (abs(angle) * playerShip->speed) >> 16;

    if (direction > 0)
        playerShip->vroll += magnitude;
    else
        playerShip->vroll -= magnitude;

    if (direction > 0)
    {
        wingPos.vx = playerShip->pos.vx + (playerShip->unitVecWing.vx >> 4) - (playerShip->unitVecNose.vx >> 4);
        wingPos.vy = playerShip->pos.vy + (playerShip->unitVecWing.vy >> 4) - (playerShip->unitVecNose.vy >> 4);
        wingPos.vz = playerShip->pos.vz + (playerShip->unitVecWing.vz >> 4) - (playerShip->unitVecNose.vz >> 4);
    }
    else
    {
        wingPos.vx = playerShip->pos.vx - (playerShip->unitVecWing.vx >> 4) - (playerShip->unitVecNose.vx >> 4);
        wingPos.vy = playerShip->pos.vy - (playerShip->unitVecWing.vy >> 4) - (playerShip->unitVecNose.vy >> 4);
        wingPos.vz = playerShip->pos.vz - (playerShip->unitVecWing.vz >> 4) - (playerShip->unitVecNose.vz >> 4);
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
    SVECTOR collisionVector;
    int angle;
    VECTOR nosePos;
    int magnitude;

    collisionVector.vx = playerShip->nearTrkSect->centre.vx - track->vertices[(facePtr->vertex[2])].vx;
    collisionVector.vy = playerShip->nearTrkSect->centre.vy - track->vertices[(facePtr->vertex[2])].vy;
    collisionVector.vz = playerShip->nearTrkSect->centre.vz - track->vertices[(facePtr->vertex[2])].vz;

    angle = abs(AngleOfCollision(&collisionVector, &playerShip->unitVecNose));

    Reflection(&(playerShip->vel), &facePtr->normal);

    magnitude = (playerShip->speed >> 4) + 400;
    if (direction > 0)
        playerShip->vhdg += magnitude;
    else
        playerShip->vhdg -= magnitude;

    playerShip->pos.vx -= playerShip->vel.vx >> 6;
    playerShip->pos.vy -= playerShip->vel.vy >> 6;
    playerShip->pos.vz -= playerShip->vel.vz >> 6;

    playerShip->vel.vx -= playerShip->vel.vx >> 1;
    playerShip->vel.vy -= playerShip->vel.vy >> 1;
    playerShip->vel.vz -= playerShip->vel.vz >> 1;

    playerShip->vel.vx += facePtr->normal.vx;
    playerShip->vel.vy += facePtr->normal.vy;
    playerShip->vel.vz += facePtr->normal.vz;

    nosePos.vx = playerShip->pos.vx + (playerShip->unitVecNose.vx >> 3);
    nosePos.vy = playerShip->pos.vy + (playerShip->unitVecNose.vy >> 3);
    nosePos.vz = playerShip->pos.vz + (playerShip->unitVecNose.vz >> 3);

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

void Reflection(VECTOR* incidence, const SVector* normal)
{
    int temp;

    temp = ((((int)normal->vx * -incidence->vx) >> 12) + ((int)(normal->vy * -incidence->vy) >> 12) + ((int)(normal->vz * -incidence->vz) >> 12));

    incidence->vx = (((2 * (int)normal->vx) * temp) >> 12) + incidence->vx;
    incidence->vy = (((2 * (int)normal->vy) * temp) >> 12) + incidence->vy;
    incidence->vz = (((2 * (int)normal->vz) * temp) >> 12) + incidence->vz;
}

int AngleOfCollision(const SVECTOR* normal, const VECTOR* ship_point)
{
    int dot_product;
    int norm_dist, shpoint_dist;
    int dist_product;

    int tmp1, tmp2;
    long sx, sy, sz;

    sx = ship_point->vx >> 9;
    sy = ship_point->vy >> 9;
    sz = ship_point->vz >> 9;

    dot_product = ((sx * normal->vx) + (sy * normal->vy) + (sz * normal->vz));

    norm_dist = SquareRoot0((normal->vx * normal->vx) + (normal->vy * normal->vy) + (normal->vz * normal->vz));

    shpoint_dist = SquareRoot0((sx * sx) + (sy * sy) + (sz * sz));

    dist_product = (shpoint_dist * norm_dist);

    tmp1 = dot_product << 12;
    if (dist_product != 0)
        tmp2 = (tmp1 / dist_product);
    else
        tmp2 = 0;

    return tmp2;
}
