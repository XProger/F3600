/************************************************************/
/******     dynam.c - Ship 2 track reaction             *****/
/************************************************************/
#include "game.h"

#ifdef USE_SHIP_AMBIENT
void ShipCalcAmbient(ShipData* ship)
{
    Face* facePtr;

    facePtr = gTrack.faces + ship->nearTrkSect->faceStart;

    while (facePtr->flags != TRACK_BASE) // base track only (no speedups, weapons etc)
    {
        facePtr++;
    }

    ship->color = facePtr->color;
}
#endif

void ApplySpeedUp(ShipData* ship)
{
    ship->vel.x += ship->nearTrkSect->nextSection->centre.x - ship->nearTrkSect->centre.x;
    ship->vel.y += ship->nearTrkSect->nextSection->centre.y - ship->nearTrkSect->centre.y;
    ship->vel.z += ship->nearTrkSect->nextSection->centre.z - ship->nearTrkSect->centre.z;

    if (ship->player)
    {
        PlayerData* player = (PlayerData*)ship->player;
        player->camera.fovTarget = CAMERA_FOV_EXTRA;
    }
}

void ShipTrkReaction(ShipData* ship)
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
    ship->thrust.x = (ship->thrust_mag * ship->unitVecNose.x) >> 6;
    ship->thrust.y = (ship->thrust_mag * ship->unitVecNose.y) >> 6;
    ship->thrust.z = (ship->thrust_mag * ship->unitVecNose.z) >> 6;

    velMag = GetMagnitude(&ship->vel);

    ship->speed = velMag;

    velDOF.x = sar(velMag * ship->unitVecNose.x, 12);
    velDOF.y = sar(velMag * ship->unitVecNose.y, 12);
    velDOF.z = sar(velMag * ship->unitVecNose.z, 12);

    if (ship->nearTrkSect->flags & Jump)
    {
        facePtr = gTrack.faces + ship->nearTrkSect->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        facePoint = gTrack.vertices + facePtr->vertex[0];

        alpha = PlaneLineCollision(facePoint, &ship->pos, &facePtr->normal);

        planePoint.x = ship->pos.x - ((facePtr->normal.x * alpha) >> FIXED_SHIFT);
        planePoint.y = ship->pos.y - ((facePtr->normal.y * alpha) >> FIXED_SHIFT);
        planePoint.z = ship->pos.z - ((facePtr->normal.z * alpha) >> FIXED_SHIFT);

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
            ship->attr |= FLYING;
        }
    }

    /***************************************************************************/
    /***                    Ship is held by the track                        ***/
    /***************************************************************************/

    if ((ship->attr & FLYING) == 0)
    {
        facePtr = gTrack.faces + ship->nearTrkSect->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        Ship2Track(ship, facePtr);

        if ((ship->attr & SPECIALED) == 0)
        {
            if (((facePtr->flags & SPEED_UP) && (ship->attr & LSIDE)))
            {
                ApplySpeedUp(ship);
            }
            facePtr++;

            if ((facePtr->flags & SPEED_UP) && ((ship->attr & LSIDE) == 0))
            {
                ApplySpeedUp(ship);
            }
            facePtr--;
        }

        if ((ship->attr & LSIDE) == 0)
        {
            facePtr++;
        }

        facePoint = gTrack.vertices + facePtr->vertex[0];

        alpha = PlaneLineCollision(facePoint, &ship->pos, &facePtr->normal);
        noseAlpha = alpha;

        /******************************************************************/
        /***                    Collision with floor                    ***/
        /******************************************************************/
        if (alpha <= 0)
        {
            PlayDirectionalNote(SFX_SHIP_2_TRACK, &ship->pos);

            Reflection(&(ship->vel), &facePtr->normal);

            ship->vel.x -= (ship->vel.x) >> 3;
            ship->vel.y -= (ship->vel.y) >> 3;
            ship->vel.z -= (ship->vel.z) >> 3;

            ship->vel.x -= facePtr->normal.x;
            ship->vel.y -= facePtr->normal.y;
            ship->vel.z -= facePtr->normal.z;
        }
        else if (alpha <= 30)
        {
            ship->vel.x += facePtr->normal.x;
            ship->vel.y += facePtr->normal.y;
            ship->vel.z += facePtr->normal.z;
        }

        if (alpha < 50)
            alpha = 50;

        // ApplyMagnet
        alpha = (TRACK_MAGNET << 8) * (TARGETHEIGHT - alpha) / alpha;
        force.x = (facePtr->normal.x * alpha) >> 8;
        force.y = (facePtr->normal.y * alpha) >> 8;
        force.z = (facePtr->normal.z * alpha) >> 8;

        force.y += 30000;

        brake = (ship->r_brake_rot + ship->l_brake_rot);

        tmp = FixedInvU(ship->skid + (brake >> 2));
        accX = (velDOF.x - ship->vel.x) * tmp;
        accY = (velDOF.y - ship->vel.y) * tmp;
        accZ = (velDOF.z - ship->vel.z) * tmp;

        accX += (force.x + ship->thrust.x) * SHIP_MASS_INV;
        accY += (force.y + ship->thrust.y) * SHIP_MASS_INV;
        accZ += (force.z + ship->thrust.z) * SHIP_MASS_INV;

        ship->vel.x += accX >> 16;
        ship->vel.y += accY >> 16;
        ship->vel.z += accZ >> 16;

        /*********************************************/
        /****          Resistance vectors         ****/
        /*********************************************/
        /*** Aveage resistance value based around 128 ***/
        resistance = (ship->resistance * (74 - (brake >> 3))) >> 7;

        tmp = FixedInvU(resistance);
        ship->vel.x -= ship->vel.x * tmp >> 16;
        ship->vel.y -= ship->vel.y * tmp >> 16;
        ship->vel.z -= ship->vel.z * tmp >> 16;

        ship->pos.x += sar(ship->vel.x, VELOCITY_SHIFT);
        ship->pos.y += sar(ship->vel.y, VELOCITY_SHIFT);
        ship->pos.z += sar(ship->vel.z, VELOCITY_SHIFT);

        nosePos.x = (ship->unitVecNose.x >> 5) + ship->pos.x;
        nosePos.y = (ship->unitVecNose.y >> 5) + ship->pos.y;
        nosePos.z = (ship->unitVecNose.z >> 5) + ship->pos.z;

        alpha = PlaneLineCollision(facePoint, &nosePos, &facePtr->normal);

        noseAlpha -= alpha;

        if (alpha < 600)
        {
            ship->vpitch += (noseAlpha + 5);
        }
        else
        {
            ship->vpitch -= 50;
        }

        ship->vpitch -= ship->vpitch >> 2;
        brake = sar(ship->r_brake_rot - ship->l_brake_rot, 3);

        ship->hdg += sar(brake * ship->speed, 15);

        ship->vroll += sar(ship->vhdg, 5);
        ship->vroll -= sar(ship->vroll, 1);
    }

    /***************************************************************************/
    /***                          Ship is flying                             ***/
    /***************************************************************************/
    else
    {
        /************************************************************************/
        /***              Detect the need for a rescue droid                  ***/
        /************************************************************************/

        nextSection = ship->nearTrkSect->nextSection;

        GetTargetPos(&bestPath, &ship->pos, &nextSection->centre, &ship->nearTrkSect->centre);

        distance.x = (bestPath.x - ship->pos.x);
        distance.y = (bestPath.y - ship->pos.y);
        distance.z = (bestPath.z - ship->pos.z);

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
            ship->update = UpdateRescueDroid;
            ship->updateCount = 500;
            ship->attr |= RESCUE;
        }

        brake = (ship->r_brake_rot + ship->l_brake_rot);

        tmp = FixedInvU(MIN_RESISTANCE + (brake << 2));
        accX = (velDOF.x - ship->vel.x) * tmp;
        accY = (velDOF.y - ship->vel.y) * tmp;
        accZ = (velDOF.z - ship->vel.z) * tmp;

        accX += (ship->thrust.x) * SHIP_MASS_INV;
        accY += (GRAVITY + ship->thrust.y) * SHIP_MASS_INV;
        accZ += (ship->thrust.z) * SHIP_MASS_INV;

        ship->vel.x += accX >> 16;
        ship->vel.y += accY >> 16;
        ship->vel.z += accZ >> 16;

        /*********************************************/
        /****          Resistance vectors         ****/
        /*********************************************/

        resistance = (ship->resistance * (74 - (brake >> 3))) >> 7;

        tmp = FixedInvU(resistance);
        ship->vel.x -= ship->vel.x * tmp >> 16;
        ship->vel.y -= ship->vel.y * tmp >> 16;
        ship->vel.z -= ship->vel.z * tmp >> 16;

        ship->pos.x += sar(ship->vel.x, VELOCITY_SHIFT);
        ship->pos.y += sar(ship->vel.y, VELOCITY_SHIFT);
        ship->pos.z += sar(ship->vel.z, VELOCITY_SHIFT);

        ship->vpitch -= FR50;
        ship->vpitch -= ship->vpitch >> 2;

        brake = sar(ship->r_brake_rot - ship->l_brake_rot, 3);
        ship->hdg += sar(brake * ship->speed, 15);

        ship->vroll += sar(ship->vhdg, 5);
        ship->vroll -= sar(ship->vroll, 1);
    }
    ship->hdg += sar(ship->vhdg, 6);

    ship->pitch += ship->vpitch >> 4;

    ship->roll += ship->vroll;
    ship->roll -= ship->roll >> 3;

    ship->hdg = ang(ship->hdg);
    ship->pitch = ang(ship->pitch);
    ship->roll = ang(ship->roll);

    nextSection = ship->nearTrkSect->prevSection;

    if ((ship->attr & DIRECTION) == 0)
    {
        if (nextSection->flags & Jump)
        {
            repulseVector.x = ship->nearTrkSect->nextSection->centre.x - ship->nearTrkSect->centre.x;
            repulseVector.y = ship->nearTrkSect->nextSection->centre.y - ship->nearTrkSect->centre.y;
            repulseVector.z = ship->nearTrkSect->nextSection->centre.z - ship->nearTrkSect->centre.z;

            ship->vel.x += repulseVector.x << 1;
            ship->vel.y += repulseVector.y << 1;
            ship->vel.z += repulseVector.z << 1;
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
