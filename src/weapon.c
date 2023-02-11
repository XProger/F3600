#include "game.h"

WeaponData gWeapons[MAX_WEAPONS];
WeaponData* gWeaponsFree;
WeaponData* gWeaponsActive;

extern Texture* gTargetTex;

void UpdateWeapons(ShipData* shipIndex, Object** weaponShapes)
{
    WeaponData* weapon = gWeaponsActive;

    while (weapon)
    {
        WeaponData* next = weapon->next;

        FRAME_LERP_INIT(weapon);

        (weapon->updateWeapon)(shipIndex, weaponShapes, weapon);
        weapon = next;
    }
}

void DrawWeapons(void)
{
    Object* obj;
    WeaponData* weapon = gWeaponsActive;

    while (weapon)
    {
        int32 px = weapon->pos.x;
        int32 py = weapon->pos.y;
        int32 pz = weapon->pos.z;
        int32 rx = weapon->pitch;
        int32 ry = weapon->hdg;
        int32 rz = weapon->roll;

        obj = gWeaponShapes[weapon->currentWeapon];

        matrixPush();

        FRAME_LERP(weapon);

        matrixTranslate(px, py, pz);
        matrixRotateYXZ(rx, ry, rz);

        if (weapon->type == WPN_SHIELD)
        {
            renderShield(obj, weapon->count);
        }
        else
        {
            renderObject(obj);
        }

        matrixPop();

        weapon = weapon->next;
    }
}

void MissileLock(ShipData* ship, ShipData* shipIndex, Object** shipShapes)
{
    int32 i;
    int32 distance, shortestDistance;
    ShipData* nearShip = NULL;
    TrackSection* section;

    shortestDistance = 999;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ShipData* targetShip = shipIndex + i;

        if (ship == targetShip)
            continue;

        /*** Is Player ship upon a junction branch **/
        if (ship->nearTrkSect->flags & JunctionSection)
        {
            /*** If remote ship on same branch the attempt to target ***/
            if (targetShip->nearTrkSect->flags & JunctionSection)
            {
                distance = targetShip->nearTrkSect->secNo - ship->nearTrkSect->secNo;

                if ((distance < shortestDistance) && (distance > 0))
                {
                    shortestDistance = distance;
                    nearShip = targetShip;
                }
            }
            else
            {
                /*** Remote ship is not same branch ***/
                section = ship->nearTrkSect;

                for (distance = 0; distance < 10; distance++)
                {
                    section = section->nextSection;
                    if (targetShip->nearTrkSect == section)
                    {
                        if ((distance < shortestDistance) && (distance > 0))
                        {
                            shortestDistance = distance;
                            nearShip = targetShip;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            if (targetShip->nearTrkSect->flags & JunctionSection)
            {
                section = ship->nearTrkSect;
                for (distance = 0; distance < 10; distance++)
                {
                    if (section->junction)
                    {
                        section = section->junction;
                    }
                    else
                    {
                        section = section->nextSection;
                    }
                    if (targetShip->nearTrkSect == section)
                    {
                        if ((distance < shortestDistance) && (distance > 0))
                        {
                            shortestDistance = distance;
                            nearShip = targetShip;
                            break;
                        }
                    }
                }
            }
            else
            {
                distance = targetShip->nearTrkSect->secNo - ship->nearTrkSect->secNo;

                if ((distance < shortestDistance) && (distance > 0))
                {
                    shortestDistance = distance;
                    nearShip = targetShip;
                }
            }
        }
    }

    if (shortestDistance < 10)
    {
        ship->targetShip = nearShip;
    }
    else
    {
        ship->targetShip = NULL;
    }
}

void DrawTargetIcon(ShipData* targetShip)
{
    int32 px = FRAME_LERP_POS(targetShip->_pos.x, targetShip->pos.x);
    int32 py = FRAME_LERP_POS(targetShip->_pos.y, targetShip->pos.y);
    int32 pz = FRAME_LERP_POS(targetShip->_pos.z, targetShip->pos.z);

    matrixTranslate(px, py, pz);
    renderTargetIcon(gTargetTex);
}

void ResetWeapons(void)
{
    int32 i;

    memset(gWeapons, 0, sizeof(gWeapons));

    for (i = 0; i < MAX_WEAPONS - 1; i++)
    {
        gWeapons[i].next = gWeapons + i + 1;
    }
    gWeapons[MAX_WEAPONS - 1].next = NULL;

    gWeaponsFree = gWeapons;
    gWeaponsActive = NULL;
}

WeaponData* CreateWeapon()
{
    WeaponData* weapon = gWeaponsFree;
    if (weapon)
    {
        gWeaponsFree = weapon->next;

        weapon->next = gWeaponsActive;
        weapon->prev = NULL;
        if (gWeaponsActive)
        {
            gWeaponsActive->prev = weapon;
        }
        gWeaponsActive = weapon;
    }
    return weapon;
}

void RemoveWeapon(WeaponData* weapon)
{
    if (weapon->next)
    {
        weapon->next->prev = weapon->prev;
    }

    if (weapon->prev)
    {
        weapon->prev->next = weapon->next;
    }
    else
    {
        gWeaponsActive = weapon->next;
    }

    weapon->next = gWeaponsFree;
    gWeaponsFree = weapon;
}


void GetFireVector(vec3i* result, vec3i target, Face* facePtr, vec3i shipPos)
{
    int alpha, origAlpha;
    const vec3i* facePoint;

    facePoint = gTrack.vertices + facePtr->vertex[0];

    target.x = shipPos.x + (target.x >> 6);
    target.y = shipPos.y + (target.y >> 6);
    target.z = shipPos.z + (target.z >> 6);

    alpha = PlaneLineCollision(facePoint, &target, &facePtr->normal);
    origAlpha = PlaneLineCollision(facePoint, &shipPos, &facePtr->normal);

    alpha = (alpha * 31) >> 5;

    alpha -= origAlpha;

    result->x = target.x - ((facePtr->normal.x * alpha) >> FIXED_SHIFT);
    result->y = target.y - ((facePtr->normal.y * alpha) >> FIXED_SHIFT);
    result->z = target.z - ((facePtr->normal.z * alpha) >> FIXED_SHIFT);

    result->x -= shipPos.x;
    result->y -= shipPos.y;
    result->z -= shipPos.z;
}


void Weapon2Mine(WeaponData* weapon)
{
    WeaponData* mine = gWeaponsActive;

    while (mine)
    {
        if (mine->type == WPN_MINE)
        {
            if (GetMagnitudeApprox((mine->pos.x - weapon->pos.x) >> 2, (mine->pos.z - weapon->pos.z) >> 2) < 60)
            {
                mine->count = 1;

                PlayDirectionalNote(SFX_EXPLOSION_2, &mine->pos);
                AddExplosion(weapon, -1);
                RemoveWeapon(weapon);
                break;
            }
        }
        mine = mine->next;
    }
}


void UpdateMine(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    int32 ossilation;
    int32 i, magSq;
    int32 r;
    ObjFace* face;

    weapon->count--;
    if (weapon->count == 0)
    {
        RemoveWeapon(weapon);
        return;
    }

    ossilation = rsin((weapon->currentWeapon * 400) + (weapon->count * 40)) >> 9;

    weapon->pos.x += ossilation;
    weapon->hdg += 128;
    weapon->pitch = 0;

#ifdef USE_VERTEX_COLOR
    face = weaponShapes[weapon->currentWeapon]->faces;

    r = (rsin(weapon->count * 140) >> 6) + 230;
    if (r > 255)
        r = 255;

    for (i = 0; i < 8; i++, face++) // the original hack, first 8 triangles (top & bottom) are lit
    {
        ASSERT(!FACE_QUAD(face));
        face->color[0] =
        face->color[1] =
        face->color[2] =
        face->color[3] = r | 0xFF004000;
    }
#endif

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ShipData* ship = shipIndex + i;

        if (ship == weapon->fireShip)
            continue;

        distance.x = (ship->pos.x - weapon->pos.x) >> 4;
        distance.y = (ship->pos.y - weapon->pos.y) >> 4;
        distance.z = (ship->pos.z - weapon->pos.z) >> 4;

        magSq = GetMagnitudeSqr(&distance);

        if (magSq < (25 * 25))
        {
            if ((ship->attr & SHIELDED) == 0)
            {
                if (ship->player)
                {
                    ship->vel.x -= ship->vel.x >> 3;
                    ship->vel.y -= ship->vel.y >> 3;
                    ship->vel.z -= ship->vel.z >> 3;
                    ship->shakeCount = 20;
                }
                else
                {
                    ship->speed = ship->speed >> 3;
                }
            }

            PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }
}

void InitMine(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    if (weapon->count == 1)
    {
        ShipData* ship = weapon->fireShip;
        weapon->nearTrkSect = ship->nearTrkSect;
        weapon->currentWeapon = WEAPON_SHAPE_MINE;

        weapon->pos.x = ship->pos.x;
        weapon->pos.y = ship->pos.y;
        weapon->pos.z = ship->pos.z;
        weapon->pitch = 0;
        weapon->hdg = 0;
        weapon->roll = 0;
        FRAME_LERP_INIT(weapon);

        weapon->updateWeapon = UpdateMine;
        weapon->count = MINE_DURATION;

        if (ship->player)
        {
            PlayNote(SFX_MINE_DROP);
        }
    }
    else
    {
        weapon->count--;
    }
}


void UpdateMissile(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    vec3i targetVector;
    Face* facePtr;
    int i, magSq, temp, vhdg, vpitch;
    const vec3i* facePoint;
    int alpha;
    int32 sx, sy;
    int32 cx, cy;

    if (weapon->targetShip)
    {
        ShipData* target = weapon->targetShip;
        targetVector.x = (target->pos.x - weapon->pos.x) >> 3;
        targetVector.y = (target->pos.y - weapon->pos.y) >> 3;
        targetVector.z = (target->pos.z - weapon->pos.z) >> 3;
        temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

        vhdg = ((-ratan2(targetVector.x, targetVector.z)) - weapon->hdg);
        vpitch = ((-ratan2(targetVector.y, temp)) - weapon->pitch);
    }
    else
    {
        vhdg = 0;
        vpitch = 0;
    }

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, &weapon->pos);

    if (vhdg > 2048)
        vhdg = vhdg - 4096;
    else if (vhdg < -2048)
        vhdg = vhdg + 4096;
    if (vpitch > 2048)
        vpitch = vpitch - 4096;
    else if (vpitch < -2048)
        vpitch = vpitch + 4096;

    weapon->hdg += vhdg >> 2;
    weapon->pitch += vpitch >> 2;

    weapon->hdg = ang(weapon->hdg);
    weapon->pitch = ang(weapon->pitch);

    rsincos(weapon->pitch, sx, cx);
    rsincos(weapon->hdg, sy, cy);

    weapon->acc.x = (-sy * cx) >> 16;
    weapon->acc.y = -sx >> 4;
    weapon->acc.z = (cy * cx) >> 16;

    weapon->vel.x += weapon->acc.x;
    weapon->vel.y += weapon->acc.y;
    weapon->vel.z += weapon->acc.z;

    weapon->vel.x -= weapon->vel.x >> 2;
    weapon->vel.y -= weapon->vel.y >> 2;
    weapon->vel.z -= weapon->vel.z >> 2;

    weapon->pos.x += weapon->vel.x;
    weapon->pos.y += weapon->vel.y;
    weapon->pos.z += weapon->vel.z;

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = gTrack.vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.x += (((200 - alpha) * (int)facePtr->normal.x) >> 13);
        weapon->pos.y += (((200 - alpha) * (int)facePtr->normal.y) >> 13);
        weapon->pos.z += (((200 - alpha) * (int)facePtr->normal.z) >> 13);
    }

    weapon->count--;
    if (weapon->count == 0)
    {
        RemoveWeapon(weapon);
        return;
    }

    AddTrail(weapon);

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ShipData* ship = shipIndex + i;

        if (ship == weapon->fireShip)
            continue;

        distance.x = (ship->pos.x - weapon->pos.x) >> 5;
        distance.y = (ship->pos.y - weapon->pos.y) >> 5;
        distance.z = (ship->pos.z - weapon->pos.z) >> 5;

        magSq = GetMagnitudeSqr(&distance);

        if (magSq < (25 * 25))
        {
            if ((ship->attr & SHIELDED) == 0)
            {
                if (ship->player)
                {
                    ship->vel.x -= (ship->vel.x * 3) >> 2;
                    ship->vel.y -= (ship->vel.y * 3) >> 2;
                    ship->vel.z -= (ship->vel.z * 3) >> 2;
                    ship->vroll += randy() % 250;
                    ship->ahdg = (randy() - 16384) >> 1;
                    ship->shakeCount = 20;
                }
                else
                {
                    ship->speed = ship->speed >> 5;
                    ship->vroll += 5000;
                    ship->ahdg = (randy() - 16384) >> 1;
                }
            }

            PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }
    /***********************************************************/
    /****   Perform collision checks against track           ***/
    /***********************************************************/

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
    {
        facePoint = gTrack.vertices + facePtr->vertex[0];
        temp = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
        if (temp <= 0)
        {
            PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);

            AddExplosion(weapon, -1);
            RemoveWeapon(weapon);
            return;
        }
        facePtr++;
    }

    Weapon2Mine(weapon);
}

void InitMissile(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    Face* facePtr;
    ShipData* ship = weapon->fireShip;

    if (ship->player)
    {
        weapon->count = MISSILE_DURATION;
    }
    else
    {
        if (weapon->count > 1)
        {
            weapon->count--;
            return;
        }
        else
        {
            weapon->count = MISSILE_DURATION;
        }
    }

    weapon->nearTrkSect = ship->nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_MISSILE;
    weapon->targetShip = ship->targetShip;
    ship->targetShip = NULL;

    facePtr = gTrack.faces + ship->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    GetFireVector(&weapon->acc, ship->unitVecNose, facePtr, ship->pos);

    weapon->vel.x = (ship->vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (ship->vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (ship->vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = ship->pos.x;
    weapon->pos.y = ship->pos.y;
    weapon->pos.z = ship->pos.z;
    weapon->pitch = ship->pitch;
    weapon->hdg = ship->hdg;
    weapon->roll = 0;
    FRAME_LERP_INIT(weapon);

    weapon->updateWeapon = UpdateMissile;
}

void UpdateRocket(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    Face* facePtr;
    int i, magSq;
    const vec3i* facePoint;
    int alpha;

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, &weapon->pos);

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    weapon->vel.x += weapon->acc.x;
    weapon->vel.y += weapon->acc.y;
    weapon->vel.z += weapon->acc.z;

    weapon->vel.x -= weapon->vel.x >> 5;
    weapon->vel.y -= weapon->vel.y >> 5;
    weapon->vel.z -= weapon->vel.z >> 5;

    weapon->pos.x += weapon->vel.x;
    weapon->pos.y += weapon->vel.y;
    weapon->pos.z += weapon->vel.z;

    facePoint = gTrack.vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.x += (((200 - alpha) * (int)facePtr->normal.x) >> 13);
        weapon->pos.y += (((200 - alpha) * (int)facePtr->normal.y) >> 13);
        weapon->pos.z += (((200 - alpha) * (int)facePtr->normal.z) >> 13);
    }

    weapon->count--;
    if (weapon->count == 0)
    {
        RemoveWeapon(weapon);
        return;
    }

    AddTrail(weapon);

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ShipData* ship = shipIndex + i;

        if (ship == weapon->fireShip)
            continue;

        distance.x = (ship->pos.x - weapon->pos.x) >> 4;
        distance.y = (ship->pos.y - weapon->pos.y) >> 4;
        distance.z = (ship->pos.z - weapon->pos.z) >> 4;

        magSq = GetMagnitudeSqr(&distance);

        if (magSq < (25 * 25))
        {
            if ((ship->attr & SHIELDED) == 0)
            {
                if (ship->player)
                {
                    ship->vel.x -= (ship->vel.x * 3) >> 2;
                    ship->vel.y -= (ship->vel.y * 3) >> 2;
                    ship->vel.z -= (ship->vel.z * 3) >> 2;
                    ship->vroll += randy() % 250;
                    ship->ahdg = (randy() - 16384) >> 1;
                    ship->shakeCount = 20;
                }
                else
                {
                    ship->speed = ship->speed >> 5;
                    ship->vroll += randy() % 250;
                    ship->ahdg = (randy() - 16384) >> 1;
                }
            }

            if (ship->player)
            {
                PlayNote(SFX_EXPLOSION_1);
                SetVolume(SFX_EXPLOSION_1, SND_MAX_VOLUME, SND_MAX_VOLUME);
            }
            else
            {
                PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);
            }

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }
    /***********************************************************/
    /****   Perform collision checks against track           ***/
    /***********************************************************/

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    if ((weapon->nearTrkSect->flags & Jump) == 0)
    {
        for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
        {
            facePoint = gTrack.vertices + facePtr->vertex[0];

            alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
            if (alpha <= 0)
            {
                PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);

                AddExplosion(weapon, -1);
                RemoveWeapon(weapon);
                return;
            }
            facePtr++;
        }
    }

    Weapon2Mine(weapon);
}

void InitRocket(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    Face* facePtr;
    ShipData* ship = weapon->fireShip;

    if (ship->player)
    {
        weapon->count = ROCKET_DURATION;
    }
    else
    {
        if (weapon->count > 1)
        {
            weapon->count--;
            return;
        }
        else
        {
            weapon->count = ROCKET_DURATION;
        }
    }

    weapon->nearTrkSect = ship->nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_ROCKET;

    facePtr = gTrack.faces + ship->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    GetFireVector(&weapon->acc, ship->unitVecNose, facePtr, ship->pos);

    weapon->vel.x = (ship->vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (ship->vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (ship->vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = ship->pos.x;
    weapon->pos.y = ship->pos.y;
    weapon->pos.z = ship->pos.z;
    weapon->pitch = ship->pitch;
    weapon->hdg = ship->hdg;
    weapon->roll = 0;
    FRAME_LERP_INIT(weapon);

    weapon->updateWeapon = UpdateRocket;
}


void InitSpecial(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    int32 i;
    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ShipData* ship = shipIndex + i;

        if (ship != weapon->fireShip)
        {
            ship->combatAttr |= SPECIALED;
            ship->specialCount = SPECIAL_DURATION;
        }
    }

    RemoveWeapon(weapon);
}


void UpdateElecBolt(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    int i;
    vec3i distance;
    vec3i targetVector;
    Face* facePtr;
    int magSq, temp, vhdg, vpitch;
    const vec3i* facePoint;
    int alpha;
    int32 sx, sy;
    int32 cx, cy;

    if (weapon->targetShip)
    {
        ShipData* target = weapon->targetShip;
        targetVector.x = (target->pos.x - weapon->pos.x) >> 3;
        targetVector.y = (target->pos.y - weapon->pos.y) >> 3;
        targetVector.z = (target->pos.z - weapon->pos.z) >> 3;
        temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

        vhdg = ((-ratan2(targetVector.x, targetVector.z)) - weapon->hdg);
        vpitch = ((-ratan2(targetVector.y, temp)) - weapon->pitch);
    }
    else
    {
        vhdg = 0;
        vpitch = 0;
    }
    if (vhdg > 2048)
        vhdg = vhdg - 4096;
    else if (vhdg < -2048)
        vhdg = vhdg + 4096;
    if (vpitch > 2048)
        vpitch = vpitch - 4096;
    else if (vpitch < -2048)
        vpitch = vpitch + 4096;

    weapon->hdg += vhdg >> 2;
    weapon->pitch += vpitch >> 2;

    weapon->hdg = ang(weapon->hdg);
    weapon->pitch = ang(weapon->pitch);

    rsincos(weapon->pitch, sx, cx);
    rsincos(weapon->hdg, sy, cy);

    weapon->acc.x = (-sy * cx) >> 16;
    weapon->acc.y = -sx >> 4;
    weapon->acc.z = (cy * cx) >> 16;

    weapon->vel.x += weapon->acc.x;
    weapon->vel.y += weapon->acc.y;
    weapon->vel.z += weapon->acc.z;

    weapon->vel.x -= weapon->vel.x >> 2;
    weapon->vel.y -= weapon->vel.y >> 2;
    weapon->vel.z -= weapon->vel.z >> 2;

    weapon->pos.x += weapon->vel.x;
    weapon->pos.y += weapon->vel.y;
    weapon->pos.z += weapon->vel.z;

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, &weapon->pos);

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = gTrack.vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.x += (((200 - alpha) * (int)facePtr->normal.x) >> 13);
        weapon->pos.y += (((200 - alpha) * (int)facePtr->normal.y) >> 13);
        weapon->pos.z += (((200 - alpha) * (int)facePtr->normal.z) >> 13);
    }

    weapon->count--;
    if (weapon->count == 0)
    {
        RemoveWeapon(weapon);
        return;
    }

    AddTrail(weapon);

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ShipData* ship = shipIndex + i;

        if (ship == weapon->fireShip)
            continue;

        distance.x = (ship->pos.x - weapon->pos.x) >> 5;
        distance.y = (ship->pos.y - weapon->pos.y) >> 5;
        distance.z = (ship->pos.z - weapon->pos.z) >> 5;

        magSq = GetMagnitudeSqr(&distance);

        if (magSq < (25 * 25))
        {
            if ((ship->attr & SHIELDED) == 0)
            {
                ship->attr |= ELECTROED;
                ship->electroCount = ELEC_DURATION;
            }

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }

    /***********************************************************/
    /****   Perform collision checks against track           ***/
    /***********************************************************/

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
    {
        facePoint = gTrack.vertices + facePtr->vertex[0];

        temp = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
        if (temp <= 0)
        {
            PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);

            AddExplosion(weapon, -1);
            RemoveWeapon(weapon);
            return;
        }
        facePtr++;
    }

    Weapon2Mine(weapon);
}

void InitElecBolt(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    ShipData* ship = weapon->fireShip;

    if (ship->player)
    {
        weapon->count = ELEC_DURATION;
    }
    else
    {
        if (weapon->count > 1)
        {
            weapon->count--;
            return;
        }
        else
        {
            weapon->count = ELEC_DURATION;
        }
    }

    weapon->nearTrkSect = ship->nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_EBOLT;
    weapon->targetShip = ship->targetShip;
    ship->targetShip = NULL;

    weapon->acc.x = ship->unitVecNose.x >> 8;
    weapon->acc.y = ship->unitVecNose.y >> 8;
    weapon->acc.z = ship->unitVecNose.z >> 8;

    weapon->vel.x = (ship->vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (ship->vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (ship->vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = ship->pos.x;
    weapon->pos.y = ship->pos.y;
    weapon->pos.z = ship->pos.z;
    weapon->pitch = ship->pitch;
    weapon->hdg = ship->hdg;
    weapon->roll = 0;
    FRAME_LERP_INIT(weapon);

    weapon->updateWeapon = UpdateElecBolt;
}


void UpdateRevCon(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    vec3i targetVector;
    const vec3i* facePoint;
    Face* facePtr;
    int i, magSq, temp, vhdg, vpitch;
    int alpha;
    int32 sx, sy;
    int32 cx, cy;

    if (weapon->targetShip)
    {
        ShipData* target = weapon->targetShip;
        targetVector.x = (target->pos.x - weapon->pos.x) >> 3;
        targetVector.y = (target->pos.y - weapon->pos.y) >> 3;
        targetVector.z = (target->pos.z - weapon->pos.z) >> 3;
        temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

        vhdg = ((-ratan2(targetVector.x, targetVector.z)) - weapon->hdg);
        vpitch = ((-ratan2(targetVector.y, temp)) - weapon->pitch);
    }
    else
    {
        vhdg = 0;
        vpitch = 0;
    }
    if (vhdg > 2048)
        vhdg = vhdg - 4096;
    else if (vhdg < -2048)
        vhdg = vhdg + 4096;
    if (vpitch > 2048)
        vpitch = vpitch - 4096;
    else if (vpitch < -2048)
        vpitch = vpitch + 4096;

    weapon->hdg += vhdg >> 2;
    weapon->pitch += vpitch >> 2;

    weapon->hdg = ang(weapon->hdg);
    weapon->pitch = ang(weapon->pitch);

    rsincos(weapon->pitch, sx, cx);
    rsincos(weapon->hdg, sy, cy);

    weapon->acc.x = (-sy * cx) >> 16;
    weapon->acc.y = -sx >> 4;
    weapon->acc.z = (cy * cx) >> 16;

    weapon->vel.x += weapon->acc.x;
    weapon->vel.y += weapon->acc.y;
    weapon->vel.z += weapon->acc.z;

    weapon->vel.x -= weapon->vel.x >> 2;
    weapon->vel.y -= weapon->vel.y >> 2;
    weapon->vel.z -= weapon->vel.z >> 2;

    weapon->pos.x += weapon->vel.x;
    weapon->pos.y += weapon->vel.y;
    weapon->pos.z += weapon->vel.z;

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, &weapon->pos);

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = gTrack.vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.x += (((200 - alpha) * (int)facePtr->normal.x) >> 13);
        weapon->pos.y += (((200 - alpha) * (int)facePtr->normal.y) >> 13);
        weapon->pos.z += (((200 - alpha) * (int)facePtr->normal.z) >> 13);
    }

    weapon->count--;
    if (weapon->count == 0)
    {
        RemoveWeapon(weapon);
        return;
    }

    AddTrail(weapon);

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ShipData* ship = shipIndex + i;

        if (ship == weapon->fireShip)
            continue;

        distance.x = (ship->pos.x - weapon->pos.x) >> 5;
        distance.y = (ship->pos.y - weapon->pos.y) >> 5;
        distance.z = (ship->pos.z - weapon->pos.z) >> 5;

        magSq = GetMagnitudeSqr(&distance);

        if (magSq < (25 * 25))
        {
            if ((ship->attr & SHIELDED) == 0)
            {
                ship->combatAttr |= REVCONNED;
                ship->revConCount = REV_CON_DURATION;
            }

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }

    /***********************************************************/
    /****   Perform collision checks against track           ***/
    /***********************************************************/

    facePtr = gTrack.faces + weapon->nearTrkSect->faceStart;

    for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
    {
        facePoint = gTrack.vertices + facePtr->vertex[0];

        temp = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
        if (temp <= 0)
        {
            PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);

            AddExplosion(weapon, -1);
            RemoveWeapon(weapon);
            return;
        }
        facePtr++;
    }

    Weapon2Mine(weapon);
}

void InitRevCon(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    ShipData* ship = weapon->fireShip;

    if (ship->player)
    {
        weapon->count = ELEC_DURATION;
    }
    else
    {
        if (weapon->count > 1)
        {
            weapon->count--;
            return;
        }
        else
        {
            weapon->count = ELEC_DURATION;
        }
    }

    weapon->nearTrkSect = ship->nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_EBOLT;
    weapon->targetShip = ship->targetShip;
    ship->targetShip = NULL;

    weapon->acc.x = ship->unitVecNose.x >> 8;
    weapon->acc.y = ship->unitVecNose.y >> 8;
    weapon->acc.z = ship->unitVecNose.z >> 8;

    weapon->vel.x = (ship->vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (ship->vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (ship->vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = ship->pos.x;
    weapon->pos.y = ship->pos.y;
    weapon->pos.z = ship->pos.z;
    weapon->pitch = ship->pitch;
    weapon->hdg = ship->hdg;
    weapon->roll = 0;
    FRAME_LERP_INIT(weapon);

    weapon->updateWeapon = UpdateRevCon;
}


#ifdef USE_OLD_SHIELD
void UpdateShieldColors(Object* object, int32 counter)
{
    ObjFace* f = object->faces;
    ObjFace* last = f + object->fCount;
    int32 col0, col1, col2, col3;

    while (f < last)
    {
        if (FACE_QUAD(f))
        {
            col0 = (rsin(counter * f->index[0]) >> 5) + 128;
            col1 = (rsin(counter * f->index[1]) >> 5) + 128;
            col2 = (rsin(counter * f->index[2]) >> 5) + 128;
            col3 = (rsin(counter * f->index[3]) >> 5) + 128;

            if (col0 > 255)
                col0 = 255;
            if (col1 > 255)
                col1 = 255;
            if (col2 > 255)
                col2 = 255;
            if (col3 > 255)
                col2 = 255;

            f->color[0] = (uint8)(col0 >> 1) | (64 << 8) | (128 << 16);
            f->color[1] = (uint8)(col1 >> 1) | (64 << 8) | (128 << 16);
            f->color[2] = (uint8)(col2 >> 1) | (64 << 8) | (128 << 16);
            f->color[3] = (uint8)(col3 >> 1) | (64 << 8) | (128 << 16);
        }
        else
        {
            col0 = (rsin(counter * f->index[0]) >> 5) + 128;
            col1 = (rsin(counter * f->index[1]) >> 5) + 128;
            col2 = (rsin(counter * f->index[2]) >> 5) + 128;

            if (col0 > 255)
                col0 = 255;
            if (col1 > 255)
                col1 = 255;
            if (col2 > 255)
                col2 = 255;

            f->color[0] = (uint8)(col0 >> 1) | (64 << 8) | (128 << 16);
            f->color[1] = (uint8)(col1 >> 1) | (64 << 8) | (128 << 16);
            f->color[2] = (uint8)(col2 >> 1) | (64 << 8) | (128 << 16);
        }
        f++;
    }
}
#endif

void UpdateShield(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    ShipData* ship = weapon->fireShip;
    int32 px = ship->pos.x;
    int32 py = ship->pos.y;
    int32 pz = ship->pos.z;

    if (ship->attr & VIEW_IN)
    {
        px += (ship->unitVecNose.x >> 6);
        py += (ship->unitVecNose.y >> 6);
        pz += (ship->unitVecNose.z >> 6);
    }

    weapon->pos.x = px;
    weapon->pos.y = py;
    weapon->pos.z = pz;
    weapon->pitch = ship->pitch;
    weapon->hdg = ship->hdg;
    weapon->roll = ship->roll;

    weapon->count--;
    if (weapon->count == 0)
    {
        if (ship->player)
        {
            KillNote(SFX_SHIELD);
        }

        ship->attr &= ~SHIELDED;

        RemoveWeapon(weapon);
        return;
    }

#ifdef USE_OLD_SHIELD
    UpdateShieldColors(gWeaponShapes[weapon->currentWeapon], weapon->count);
#endif
}

void InitShield(ShipData* shipIndex, Object** weaponShapes, WeaponData* weapon)
{
    ShipData* ship = weapon->fireShip;

    weapon->nearTrkSect = ship->nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_SHIELD;

    weapon->pos.x = ship->pos.x;
    weapon->pos.y = ship->pos.y;
    weapon->pos.z = ship->pos.z;
    weapon->pitch = ship->pitch;
    weapon->hdg = ship->hdg;
    weapon->roll = ship->roll;
    FRAME_LERP_INIT(weapon);

    if (ship->player)
    {
        PlayNote(SFX_SHIELD);
        SetPitch(SFX_SHIELD, SND_DEF_PITCH >> 1);
    }
    weapon->updateWeapon = UpdateShield;
}


void FireWeapon(ShipData* ship, int shipNo, char decision)
{
    WeaponData* weapon = CreateWeapon();

    if (!weapon)
        return;

    switch (decision)
    {
        case WPN_ROCKET:
        {
            weapon->updateWeapon = InitRocket;
            weapon->count = WEAPON_DELAY;
            weapon->fireShip = ship;
            weapon->type = WPN_ROCKET;
            ship->weaponType = WPN_NONE;
            if (ship->player)
            {
                PlayNote(SFX_MISS_LAUNCH);
                SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH);
            }
            break;
        }

        case WPN_MISSILE:
        {
            weapon->updateWeapon = InitMissile;
            weapon->count = WEAPON_DELAY;
            weapon->fireShip = ship;
            weapon->type = WPN_MISSILE;
            ship->weaponType = WPN_NONE;
            if (ship->player)
            {
                PlayNote(SFX_MISS_LAUNCH);
                SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH);
            }
            break;
        }

        case WPN_MINE:
        {
            int32 i;
            int32 count = ship->player ? 0 : WEAPON_DELAY;

            for (i = 0; i < MINES_COUNT; i++)
            {
                count += MINE_RELEASE_RATE;
                weapon->updateWeapon = InitMine;
                weapon->fireShip = ship;
                weapon->count = count;
                weapon->type = WPN_MINE;

                if (i < MINES_COUNT - 1)
                {
                    weapon = CreateWeapon();
                    if (!weapon)
                        break;
                }
            }

            ship->weaponType = WPN_NONE;
            break;
        }

        case WPN_SPECIAL:
        {
            weapon->updateWeapon = InitSpecial;
            weapon->count = 1;
            weapon->fireShip = ship;
            ship->weaponType = WPN_NONE;
            break;
        }

        case WPN_ELEC_BOLT:
        {
            weapon->updateWeapon = InitElecBolt;
            weapon->count = WEAPON_DELAY;
            weapon->fireShip = ship;
            weapon->type = WPN_ELEC_BOLT;
            ship->weaponType = WPN_NONE;
            if (ship->player)
            {
                PlayNote(SFX_ELECTROBOLT);
            }
            break;
        }

        case WPN_REV_CON:
        {
            weapon->updateWeapon = InitRevCon;
            weapon->count = WEAPON_DELAY;
            weapon->fireShip = ship;
            weapon->type = WPN_REV_CON;
            ship->weaponType = WPN_NONE;
            if (ship->player)
            {
                PlayNote(SFX_ELECTROBOLT);
            }
            break;
        }

        case WPN_SHIELD:
        {
            weapon->updateWeapon = InitShield;
            weapon->count = SHIELD_DURATION;
            weapon->fireShip = ship;
            weapon->type = WPN_SHIELD;
            ship->attr |= SHIELDED;
            ship->weaponType = WPN_NONE;
            break;
        }

        case WPN_TURBO:
        {
            ship->weaponType = WPN_NONE;

            ship->vel.x += ship->unitVecNose.x * 48 / 5;
            ship->vel.y += ship->unitVecNose.y * 48 / 5;
            ship->vel.z += ship->unitVecNose.z * 48 / 5;

            if (ship->player)
            {
                PlayNote(SFX_MISS_LAUNCH);
                SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH >> 2);

                if (ship->player)
                {
                    PlayerData* player = (PlayerData*)ship->player;
                    player->camera.fovTarget = CAMERA_FOV_EXTRA;
                }
            }

            RemoveWeapon(weapon);
            break;
        }
    }
}

int GetNewWeapon(void)
{
    int index = randy() % 65;

    if (index < 17)
        return WPN_ROCKET;
    else if (index < 35)
        return WPN_MINE;
    else if (index < 45)
        return WPN_SHIELD;
    else if (index < 53)
        return WPN_MISSILE;
    else if (index < 59)
        return WPN_TURBO;
    else
        return WPN_ELEC_BOLT;
}

void CycleWeaponGrids(ShipData* shipIndex, WeaponGrid* weaponGrid, int noWeaponGrids)
{
    int i, j;
    Face* facePtr;

#ifdef __3DO__ // TODO 3DO optimize
    return;
#endif

    for (i = 0; i < noWeaponGrids; i++)
    {
        facePtr = gTrack.faces + weaponGrid[i].section->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        for (j = 0; j < NO_OF_SHIPS; j++)
        {
            ShipData* ship = shipIndex + j;
 
            if ((ship->combatAttr & SPECIALED) == 0)
            {
                if (((facePtr->flags & WEAPON_GRID_LEFT) && (ship->attr & LSIDE)))
                {
                    if ((weaponGrid[i].section == ship->nearTrkSect) && (ship->weaponType == WPN_NONE) && (weaponGrid[i].count >= 0))
                    {
                        weaponGrid[i].count = -30;
                        if (ship->player)
                        {
                            PlayNote(SFX_POWER_UP);
                            ship->weaponType = GetNewWeapon();
                        }
                        else
                        {
                            ship->weaponType = 1;
                        }
                    }
                }
            }
            facePtr++;
            if ((ship->combatAttr & SPECIALED) == 0)
            {
                if ((facePtr->flags & WEAPON_GRID_RIGHT) && ((ship->attr & LSIDE) == 0))
                {
                    if ((weaponGrid[i].section == ship->nearTrkSect) && (ship->weaponType == WPN_NONE) && (weaponGrid[i].count >= 0))
                    {
                        weaponGrid[i].count = -30;
                        if (ship->player)
                        {
                            PlayNote(SFX_POWER_UP);
                            ship->weaponType = GetNewWeapon();
                        }
                        else
                        {
                            ship->weaponType = 1;
                        }
                    }
                }
            }
            facePtr--;
        }

        if ((weaponGrid[i].count >= 0) && ((shipIndex[ownShip].combatAttr & SPECIALED) == 0))
        {
            int32 r, g, b;
            rsincos(weaponGrid[i].cycleCount, r, g);
            b = rsin(-weaponGrid[i].cycleCount);
            r = (r >> 5) + 128;
            g = (g >> 5) + 128;
            b = (b >> 5) + 128;

            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;

            weaponGrid[i].color = r | (g << 8) | (b << 16) | 0xFF000000;
        }
        else
        {
            weaponGrid[i].color = 0xFF808080;
        }

        if (weaponGrid[i].count < 0)
        {
            weaponGrid[i].count++;
        }
        weaponGrid[i].cycleCount += 32;

        if (weaponGrid[i].attr & WEAPON_GRID_LEFT)
        {
        #ifndef __3DO__ // TODO track attribs
            facePtr->color = weaponGrid[i].color;
        #endif
        }
        if (weaponGrid[i].attr & WEAPON_GRID_RIGHT)
        {
            facePtr++;
        #ifndef __3DO__ // TODO track attribs
            facePtr->color = weaponGrid[i].color;
        #endif
        }
    }
}
