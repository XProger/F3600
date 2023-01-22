/************************************************************/
/******     dynam.c - Ship 2 track reaction             *****/
/************************************************************/
#include "game.h"

#ifdef USE_SHIP_AMBIENT
void ShipCalcAmbient(ShipData* playerShip)
{
    Face* facePtr;

    facePtr = gTrack.faces + playerShip->nearTrkSect->faceStart;

    while (facePtr->flags != TRACK_BASE) // base track only (no speedups, weapons etc)
    {
        facePtr++;
    }

    playerShip->color = facePtr->color;
}
#endif

void ApplySpeedUp(ShipData* playerShip)
{
    playerShip->vel.x += playerShip->nearTrkSect->nextSection->centre.x - playerShip->nearTrkSect->centre.x;
    playerShip->vel.y += playerShip->nearTrkSect->nextSection->centre.y - playerShip->nearTrkSect->centre.y;
    playerShip->vel.z += playerShip->nearTrkSect->nextSection->centre.z - playerShip->nearTrkSect->centre.z;

    gTargetFOV = CAMERA_FOV_EXTRA;
}

void ShipTrkReaction(ShipData* playerShip)
{
    int alpha, noseAlpha, tmp;
    int resistance;
    Face* facePtr;
    vec3i force;
    vec3i velDOF;
    int velMag;
    vec3i nosePos;
    int32 brake;
    const vec3i* facePoint;
    TrackSection* nextSection;
    vec3i bestPath, distance;
    vec3i repulseVector;
    vec3i planePoint;
    vec3i vec0, vec1, vec2, vec3; /* vectors between plane coords and intersection point */
    int angle;
    int32 accX, accY, accZ;

    /***** Calculate thrust vector along principle axis of ship *****/
    playerShip->thrust.x = (playerShip->thrust_mag * playerShip->unitVecNose.x) >> 6;
    playerShip->thrust.y = (playerShip->thrust_mag * playerShip->unitVecNose.y) >> 6;
    playerShip->thrust.z = (playerShip->thrust_mag * playerShip->unitVecNose.z) >> 6;

    velMag = GetMagnitude(&playerShip->vel);

    playerShip->speed = velMag;

    velDOF.x = (velMag * playerShip->unitVecNose.x) >> 12;
    velDOF.y = (velMag * playerShip->unitVecNose.y) >> 12;
    velDOF.z = (velMag * playerShip->unitVecNose.z) >> 12;

    if (playerShip->nearTrkSect->flags & Jump)
    {
        facePtr = gTrack.faces + playerShip->nearTrkSect->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        facePoint = gTrack.vertices + facePtr->vertex[0];

        alpha = PlaneLineCollision(facePoint, &playerShip->pos, &facePtr->normal);

        planePoint.x = playerShip->pos.x - ((facePtr->normal.x * alpha) >> FIXED_SHIFT);
        planePoint.y = playerShip->pos.y - ((facePtr->normal.y * alpha) >> FIXED_SHIFT);
        planePoint.z = playerShip->pos.z - ((facePtr->normal.z * alpha) >> FIXED_SHIFT);

        vec0.x = planePoint.x - gTrack.vertices[(facePtr->vertex[1])].x;
        vec0.y = planePoint.y - gTrack.vertices[(facePtr->vertex[1])].y;
        vec0.z = planePoint.z - gTrack.vertices[(facePtr->vertex[1])].z;

        vec1.x = planePoint.x - gTrack.vertices[(facePtr->vertex[2])].x;
        vec1.y = planePoint.y - gTrack.vertices[(facePtr->vertex[2])].y;
        vec1.z = planePoint.z - gTrack.vertices[(facePtr->vertex[2])].z;

        facePtr++;

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
            playerShip->attr |= FLYING;
        }
    }

    /***************************************************************************/
    /***                    Ship is held by the track                        ***/
    /***************************************************************************/

    if ((playerShip->attr & FLYING) == 0)
    {
        facePtr = gTrack.faces + playerShip->nearTrkSect->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        Ship2Track(playerShip, facePtr);

        if ((playerShip->attr & SPECIALED) == 0)
        {
            if (((facePtr->flags & SPEED_UP) && (playerShip->attr & LSIDE)))
            {
                ApplySpeedUp(playerShip);
            }
            facePtr++;

            if ((facePtr->flags & SPEED_UP) && ((playerShip->attr & LSIDE) == 0))
            {
                ApplySpeedUp(playerShip);
            }
            facePtr--;
        }

        if ((playerShip->attr & LSIDE) == 0)
        {
            facePtr++;
        }

        facePoint = gTrack.vertices + facePtr->vertex[0];

        alpha = PlaneLineCollision(facePoint, &playerShip->pos, &facePtr->normal);
        noseAlpha = alpha;

        /******************************************************************/
        /***                    Collision with floor                    ***/
        /******************************************************************/
        if (alpha <= 0)
        {
            PlayDirectionalNote(SFX_SHIP_2_TRACK, &playerShip->pos);

            Reflection(&(playerShip->vel), &facePtr->normal);

            playerShip->vel.x -= (playerShip->vel.x) >> 3;
            playerShip->vel.y -= (playerShip->vel.y) >> 3;
            playerShip->vel.z -= (playerShip->vel.z) >> 3;

            playerShip->vel.x -= facePtr->normal.x;
            playerShip->vel.y -= facePtr->normal.y;
            playerShip->vel.z -= facePtr->normal.z;
        }
        else if (alpha <= 30)
        {
            playerShip->vel.x += facePtr->normal.x;
            playerShip->vel.y += facePtr->normal.y;
            playerShip->vel.z += facePtr->normal.z;
        }

        if (alpha < 50)
            alpha = 50;

        // ApplyMagnet
        alpha = (TRACK_MAGNET << 8) * (TARGETHEIGHT - alpha) / alpha;
        force.x = (facePtr->normal.x * alpha) >> 8;
        force.y = (facePtr->normal.y * alpha) >> 8;
        force.z = (facePtr->normal.z * alpha) >> 8;

        force.y += 30000;

        brake = (playerShip->r_brake_rot + playerShip->l_brake_rot);

        tmp = FixedInvU(playerShip->skid + (brake >> 2));
        accX = (velDOF.x - playerShip->vel.x) * tmp;
        accY = (velDOF.y - playerShip->vel.y) * tmp;
        accZ = (velDOF.z - playerShip->vel.z) * tmp;

        accX += (force.x + playerShip->thrust.x) * SHIP_MASS_INV;
        accY += (force.y + playerShip->thrust.y) * SHIP_MASS_INV;
        accZ += (force.z + playerShip->thrust.z) * SHIP_MASS_INV;

        playerShip->vel.x += accX >> 16;
        playerShip->vel.y += accY >> 16;
        playerShip->vel.z += accZ >> 16;

        /*********************************************/
        /****          Resistance vectors         ****/
        /*********************************************/
        /*** Aveage resistance value based around 128 ***/
        resistance = (playerShip->resistance * (74 - (brake >> 3))) >> 7;

        tmp = FixedInvU(resistance);
        playerShip->vel.x -= playerShip->vel.x * tmp >> 16;
        playerShip->vel.y -= playerShip->vel.y * tmp >> 16;
        playerShip->vel.z -= playerShip->vel.z * tmp >> 16;

        playerShip->pos.x += playerShip->vel.x >> VELOCITY_SHIFT;
        playerShip->pos.y += playerShip->vel.y >> VELOCITY_SHIFT;
        playerShip->pos.z += playerShip->vel.z >> VELOCITY_SHIFT;

        nosePos.x = (playerShip->unitVecNose.x >> 5) + playerShip->pos.x;
        nosePos.y = (playerShip->unitVecNose.y >> 5) + playerShip->pos.y;
        nosePos.z = (playerShip->unitVecNose.z >> 5) + playerShip->pos.z;

        alpha = PlaneLineCollision(facePoint, &nosePos, &facePtr->normal);

        noseAlpha -= alpha;

        if (alpha < 600)
        {
            playerShip->vpitch += (noseAlpha + 5);
        }
        else
        {
            playerShip->vpitch -= 50;
        }

        playerShip->vpitch -= playerShip->vpitch >> 2;
        brake = (playerShip->r_brake_rot - playerShip->l_brake_rot) >> 3;

        playerShip->hdg += (brake * playerShip->speed) >> 15;

        playerShip->vroll += playerShip->vhdg >> 5;
        playerShip->vroll -= playerShip->vroll >> 1;
    }

    /***************************************************************************/
    /***                          Ship is flying                             ***/
    /***************************************************************************/
    else
    {
        /************************************************************************/
        /***              Detect the need for a rescue droid                  ***/
        /************************************************************************/

        nextSection = playerShip->nearTrkSect->nextSection;

        GetTargetPos(&bestPath, &playerShip->pos, &nextSection->centre, &playerShip->nearTrkSect->centre);

        distance.x = (bestPath.x - playerShip->pos.x);
        distance.y = (bestPath.y - playerShip->pos.y);
        distance.z = (bestPath.z - playerShip->pos.z);

        if (distance.y > 0)
            distance.y = distance.y >> 7;
        else
        {
            distance.x = distance.x << 3;
            distance.y = distance.y << 3;
            distance.z = distance.z << 3;
        }

        if (GetMagnitudeSqr(&distance) > (4000 * 4000))
        {
            playerShip->update = UpdateRescueDroid;
            playerShip->updateCount = 500;
            playerShip->attr |= RESCUE;
        }

        brake = (playerShip->r_brake_rot + playerShip->l_brake_rot);

        tmp = FixedInvU(MIN_RESISTANCE + (brake << 2));
        accX = (velDOF.x - playerShip->vel.x) * tmp;
        accY = (velDOF.y - playerShip->vel.y) * tmp;
        accZ = (velDOF.z - playerShip->vel.z) * tmp;

        accX += (playerShip->thrust.x) * SHIP_MASS_INV;
        accY += (GRAVITY + playerShip->thrust.y) * SHIP_MASS_INV;
        accZ += (playerShip->thrust.z) * SHIP_MASS_INV;

        playerShip->vel.x += accX >> 16;
        playerShip->vel.y += accY >> 16;
        playerShip->vel.z += accZ >> 16;

        /*********************************************/
        /****          Resistance vectors         ****/
        /*********************************************/

        resistance = (playerShip->resistance * (74 - (brake >> 3))) >> 7;

        tmp = FixedInvU(resistance);
        playerShip->vel.x -= playerShip->vel.x * tmp >> 16;
        playerShip->vel.y -= playerShip->vel.y * tmp >> 16;
        playerShip->vel.z -= playerShip->vel.z * tmp >> 16;

        playerShip->pos.x += playerShip->vel.x >> VELOCITY_SHIFT;
        playerShip->pos.y += playerShip->vel.y >> VELOCITY_SHIFT;
        playerShip->pos.z += playerShip->vel.z >> VELOCITY_SHIFT;

        playerShip->vpitch -= FR50;
        playerShip->vpitch -= playerShip->vpitch >> 2;

        brake = (playerShip->r_brake_rot - playerShip->l_brake_rot) >> 3;
        playerShip->hdg += (brake * playerShip->speed) >> 15;

        playerShip->vroll += playerShip->vhdg >> 5;
        playerShip->vroll -= playerShip->vroll >> 1;
    }
    playerShip->hdg += playerShip->vhdg >> 6;

    playerShip->pitch += playerShip->vpitch >> 4;

    playerShip->roll += playerShip->vroll;
    playerShip->roll -= playerShip->roll >> 3;

    playerShip->hdg = ang(playerShip->hdg);
    playerShip->pitch = ang(playerShip->pitch);
    playerShip->roll = ang(playerShip->roll);

    nextSection = playerShip->nearTrkSect->prevSection;

    if ((playerShip->attr & DIRECTION) == 0)
    {
        if (nextSection->flags & Jump)
        {
            repulseVector.x = playerShip->nearTrkSect->nextSection->centre.x - playerShip->nearTrkSect->centre.x;
            repulseVector.y = playerShip->nearTrkSect->nextSection->centre.y - playerShip->nearTrkSect->centre.y;
            repulseVector.z = playerShip->nearTrkSect->nextSection->centre.z - playerShip->nearTrkSect->centre.z;

            playerShip->vel.x += repulseVector.x << 1;
            playerShip->vel.y += repulseVector.y << 1;
            playerShip->vel.z += repulseVector.z << 1;
        }
    }
}

/**********************************************************/
/** Returns the position of a point in space relative to **/
/**                        the camera                    **/
/**********************************************************/
void GetTargetPos(vec3i* result, const vec3i* pos, const vec3i* cent1, const vec3i* cent2)
{
    int a_mag, b_mag, c_mag;
    vec3i a, b, c;
    int a_sq, b_sq, c_sq;
    int cos_alpha;
    int angle;

    a.x = pos->x - cent1->x;
    a.y = pos->y - cent1->y;
    a.z = pos->z - cent1->z;

    b.x = cent2->x - cent1->x;
    b.y = cent2->y - cent1->y;
    b.z = cent2->z - cent1->z;

    c.x = pos->x - cent2->x;
    c.y = pos->y - cent2->y;
    c.z = pos->z - cent2->z;

    a_mag = GetMagnitude(&a);
    b_mag = GetMagnitude(&b);

    if ((a_mag != 0) && (b_mag != 0))
    {
        c_mag = GetMagnitude(&c);

        a_sq = (a_mag * a_mag);
        b_sq = (b_mag * b_mag);
        c_sq = (c_mag * c_mag);

        cos_alpha = ((-b_sq - (a_sq + c_sq)) << 12) / ((a_mag * b_mag) << 1);
        angle = arccos(cos_alpha) >> 2; // TODO remove arccos
        c_mag = ((rcos(angle) * a_mag) >> FIXED_SHIFT);
        result->x = cent1->x - ((b.x * c_mag) / b_mag);
        result->y = cent1->y - ((b.y * c_mag) / b_mag);
        result->z = cent1->z - ((b.z * c_mag) / b_mag);
    }
}

void SetUnitVectors(ShipData* ship)
{
    int32 sx, sy, sz;
    int32 cx, cy, cz;
    vec4i direction;
    vec4i toFaceVector;
    int toFace;
    Face* facePtr;

    rsincos(ship->pitch, sx, cx);
    rsincos(ship->hdg, sy, cy);
    rsincos(ship->roll, sz, cz);

    ship->unitVecNose.x = -frac(sy, cx);
    ship->unitVecNose.y = -sx;
    ship->unitVecNose.z = frac(cy, cx);

    ship->unitVecWing.x = ((cy * cz) + (((sy * sz) >> FIXED_SHIFT) * sx)) >> FIXED_SHIFT;
    ship->unitVecWing.y = -(sz * cx) >> FIXED_SHIFT;
    ship->unitVecWing.z = ((sy * cz) - (((cy * sx) >> FIXED_SHIFT) * sz)) >> FIXED_SHIFT;

    ship->prevShipSect = ship->nearTrkSect;
    SectionSearch(ship);

    ship->prevSecNo = ship->prevShipSect->secNo;
    ship->nearSecNo = ship->nearTrkSect->secNo;

    facePtr = gTrack.faces + ship->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    toFaceVector.x = gTrack.vertices[(facePtr->vertex[0])].x - gTrack.vertices[(facePtr->vertex[1])].x;
    toFaceVector.y = gTrack.vertices[(facePtr->vertex[0])].y - gTrack.vertices[(facePtr->vertex[1])].y;
    toFaceVector.z = gTrack.vertices[(facePtr->vertex[0])].z - gTrack.vertices[(facePtr->vertex[1])].z;

    direction.x = ship->nearTrkSect->centre.x - ship->pos.x;
    direction.y = ship->nearTrkSect->centre.y - ship->pos.y;
    direction.z = ship->nearTrkSect->centre.z - ship->pos.z;

    toFace = ((direction.x * toFaceVector.x) + (direction.y * toFaceVector.y) + (direction.z * toFaceVector.z));

    if (toFace > 0)
        ship->attr |= LSIDE;
    else
        ship->attr &= ~LSIDE;
}

int SetTrackAttributes(TrackSection* section, short i, WeaponGrid* weaponGrid, int noWeaponGrids)
{
    Face* facePtr;

    facePtr = gTrack.faces + section->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    if (facePtr->flags & WEAPON_GRID_LEFT)
    {
        if (gConfig.raceType != TIME_TRIAL)
        {
        #ifdef USE_VERTEX_COLOR
            facePtr->color = 0xFF7523FF;
        #endif
        }
        weaponGrid[noWeaponGrids].section = section;
        weaponGrid[noWeaponGrids].count = 0;
        weaponGrid[noWeaponGrids].attr = 0;
        weaponGrid[noWeaponGrids].attr |= WEAPON_GRID_LEFT;
        weaponGrid[noWeaponGrids].cycleCount = randy();
        noWeaponGrids++;
    }
    else if (facePtr->flags & SPEED_UP)
    {
    #ifdef USE_VERTEX_COLOR
        facePtr->color = 0xFFFF0000;
    #endif
    }

    facePtr++;

    if (facePtr->flags & WEAPON_GRID_RIGHT)
    {
        if (gConfig.raceType != TIME_TRIAL)
        {
        #ifdef USE_VERTEX_COLOR
            facePtr->color = 0xFF75EE54;
        #endif
        }
        weaponGrid[noWeaponGrids].section = section;
        weaponGrid[noWeaponGrids].count = 0;
        weaponGrid[noWeaponGrids].attr = 0;
        weaponGrid[noWeaponGrids].attr |= WEAPON_GRID_RIGHT;
        noWeaponGrids++;
    }
    else if (facePtr->flags & SPEED_UP)
    {
    #ifdef USE_VERTEX_COLOR
        facePtr->color = 0xFFFF0000;
    #endif
    }

    return (noWeaponGrids);
}

void ApplyMagnet(vec3i* acc, const vec3s* normal, int32 alpha)
{
    if (alpha < 50)
    {
        alpha = 50;
    }

    alpha = (TRACK_MAGNET << 8) * (TARGETHEIGHT - alpha) / alpha;
    acc->x = (normal->x * alpha) >> 16;
    acc->y = (normal->y * alpha) >> 16;
    acc->z = (normal->z * alpha) >> 16;
}
