#include "game.h"

//#define OLD_SHIELD

WeaponData gWeapons[MAX_WEAPONS];
WeaponData* gWeaponsFree;
WeaponData* gWeaponsActive;

extern long ctrlNeedTargetIcon;
extern int ctrlnearShip;
extern TrackCamera ctrlcamera;

Texture* targetTexture;

void UpdateWeapons(ShipData* shipIndex, WeaponShape* weaponShapes)
{
    WeaponData* weapon = gWeaponsActive;

    while (weapon)
    {
        WeaponData* next = weapon->next;
        (weapon->updateWeapon)(shipIndex, weaponShapes, weapon);
        weapon = next;
    }
}

void DrawWeapons(ShipData* shipIndex, WeaponShape* weaponShapes)
{
    Object* obj;
    WeaponData* weapon = gWeaponsActive;

    while (weapon)
    {
        obj = weaponShapes[weapon->currentWeapon].shapePtr;

        if (weapon->type == WPN_SHIELD)
        {
            SetSkeletonPosition(obj->skeleton, weapon->pos.x, weapon->pos.y, weapon->pos.z);
            SetSkeletonDirectionHPR(obj->skeleton, weapon->hdg, weapon->pitch, shipIndex[weapon->fireShip].roll);
            renderShield(obj, weapon->count);
        }
        else
        {
            DrawObject(weaponShapes[weapon->currentWeapon].shapePtr, weapon->pos.x, weapon->pos.y, weapon->pos.z, weapon->hdg, weapon->pitch, 0);
        }

        weapon = weapon->next;
    }
}

void MissileLock(ShipData* playerShip, ShipData* shipIndex, Object** shipShapes)
{
    int i;
    int distance, shortestDistance;
    int nearShip = 0;
    TrackSection* section;

    shortestDistance = 999;

    for (i = (NO_OF_SHIPS - 1); i >= 0; i--)
    {
        if (i == ownShip)
            continue;

        /*** Is Player ship upon a junction branch **/
        if (playerShip->nearTrkSect->flags & JunctionSection)
        {
            /*** If remote ship on same branch the attempt to target ***/
            if (shipIndex[i].nearTrkSect->flags & JunctionSection)
            {
                distance = shipIndex[i].nearTrkSect->secNo - playerShip->nearTrkSect->secNo;

                if ((distance < shortestDistance) && (distance > 0))
                {
                    shortestDistance = distance;
                    nearShip = i;
                }
            }
            else
            {
                /*** Remote ship is not same branch ***/
                section = playerShip->nearTrkSect;

                for (distance = 0; distance < 10; distance++)
                {
                    section = section->nextSection;
                    if (shipIndex[i].nearTrkSect == section)
                    {
                        if ((distance < shortestDistance) && (distance > 0))
                        {
                            shortestDistance = distance;
                            nearShip = i;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            if (shipIndex[i].nearTrkSect->flags & JunctionSection)
            {
                section = playerShip->nearTrkSect;
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
                    if (shipIndex[i].nearTrkSect == section)
                    {
                        if ((distance < shortestDistance) && (distance > 0))
                        {
                            shortestDistance = distance;
                            nearShip = i;
                            break;
                        }
                    }
                }
            }
            else
            {
                distance = shipIndex[i].nearTrkSect->secNo - playerShip->nearTrkSect->secNo;

                if ((distance < shortestDistance) && (distance > 0))
                {
                    shortestDistance = distance;
                    nearShip = i;
                }
            }
        }
    }

    if (shortestDistance < 10)
    {
        ctrlNeedTargetIcon = 1;
        ctrlnearShip = nearShip;

        playerShip->targetShip = nearShip;
    }
    else
        playerShip->targetShip = 0xFF;
}

void DrawTargetIcon(ShipData* shipIndex, Object** shipShapes)
{
    vec3s* vertex;
    vec2s sxy;
    long flag;
    const MATRIX* matrix;

    shipShapes[shipIndex[ctrlnearShip].highResTableIndex]->skeleton->update = 1;
    vertex = (vec3s*)shipShapes[shipIndex[ctrlnearShip].highResTableIndex]->vertices;
    matrix = GetSkeletonMatrix(shipShapes[shipIndex[ctrlnearShip].highResTableIndex]->skeleton);

    RotTransPers(matrix, vertex, &sxy, &flag);

    if ((flag & (CrtClipped | ZNegative | ZLarge | ZSmall)))
        return;

    renderSetBlend(BLEND_ADD);
    renderImage(targetTexture, sxy.x - (targetTexture->width >> 1), sxy.y - (targetTexture->height >> 1));
    renderSetBlend(BLEND_ALPHA);
}

void InitWeaponObj(WeaponShape* weaponShapes, int textures, const char* name, int shapeIndex)
{
    Prm prm;
    Object* object;

    prm.object = LoadPrm(name, textures);
    weaponShapes[shapeIndex].shapePtr = prm.object;
    object = prm.object++;
    object->skeleton = prm.skeleton++;
    object->vertices = prm.svector;
    prm.svector += object->vertexCount;
    object->unused_normals = prm.svector;
    prm.svector += object->unused_normalCount;
    object->primitives = prm.primitive;
}

void InitWeapons(WeaponShape* weaponShapes)
{
    int weaponTextures = gTexturePtr - gTextures;
    LoadCompressedTextureSequence("wipeout/common/mine.cmp", 1);

    InitWeaponObj(weaponShapes, weaponTextures, "wipeout/common/rock.prm", WEAPON_SHAPE_ROCKET);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout/common/mine.prm", WEAPON_SHAPE_MINE);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout/common/miss.prm", WEAPON_SHAPE_MISSILE);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout/common/shld.prm", WEAPON_SHAPE_SHIELD);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout/common/ebolt.prm", WEAPON_SHAPE_EBOLT);

    targetTexture = LoadTexture("wipeout/textures/target2.tim");
}

void ResetWeapons(WeaponShape* weaponShapes, Skeleton* camPos)
{
    int32 i;
    Object* obj;

    for (i = 0; i < WEAPON_SHAPE_NAX; i++)
    {
        obj = weaponShapes[i].shapePtr;
        while (obj)
        {
            obj->skeleton->super = camPos;
            obj = obj->next;
        }
    }

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

    facePoint = (vec3i*)(track->vertices + facePtr->vertex[0]);

    target.x = shipPos.x + (target.x >> 6);
    target.y = shipPos.y + (target.y >> 6);
    target.z = shipPos.z + (target.z >> 6);

    alpha = PlaneLineCollision(facePoint, &target, &facePtr->normal);
    origAlpha = PlaneLineCollision(facePoint, &shipPos, &facePtr->normal);

    alpha = (alpha * 31) >> 5;

    alpha -= origAlpha;

    result->x = target.x - ((alpha * facePtr->normal.x) >> 12);
    result->y = target.y - ((alpha * facePtr->normal.y) >> 12);
    result->z = target.z - ((alpha * facePtr->normal.z) >> 12);

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
            if (GetMagnitudeApprox((weapon->pos.x - mine->pos.x) >> 2, (weapon->pos.z - mine->pos.z) >> 2) < 30)
            {
                PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);
                RemoveWeapon(weapon);
                RemoveWeapon(mine);
                break;
            }
        }
        mine = mine->next;
    }
}


void UpdateMine(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    int ossilation;
    int i, mag;
    Prm poly;
    short r;

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

    poly.primitive = weaponShapes[weapon->currentWeapon].shapePtr->primitives;

    r = (rsin(weapon->count * 140) >> 6) + 230;
    if (r > 255)
        r = 255;

    for (i = 0; i < 8; i++)
    {
        switch (poly.primitive->type)
        {
            case TypeGT3:
                poly.gt3->colour[0].r = poly.gt3->colour[1].r = poly.gt3->colour[2].r = (uint8)r;
                poly.gt3->colour[0].g = poly.gt3->colour[1].g = poly.gt3->colour[2].g = 0x40;
                poly.gt3->colour[0].b = poly.gt3->colour[1].b = poly.gt3->colour[2].b = 0;
                poly.gt3 += 1;
                break;
        }
    }

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i == weapon->fireShip)
            continue;

        distance.x = (shipIndex[i].pos.x - weapon->pos.x) >> 2;
        distance.y = (shipIndex[i].pos.y - weapon->pos.y) >> 2;
        distance.z = (shipIndex[i].pos.z - weapon->pos.z) >> 2;

        mag = GetMagnitude(&distance);

        if (mag < 100)
        {
            if ((shipIndex[i].attr & SHIELDED) == 0)
            {
                if (i != ownShip)
                {
                    shipIndex[i].speed = shipIndex[i].speed >> 3;
                }
                else
                {
                    shipIndex[i].vel.x -= shipIndex[i].vel.x >> 3;
                    shipIndex[i].vel.y -= shipIndex[i].vel.y >> 3;
                    shipIndex[i].vel.z -= shipIndex[i].vel.z >> 3;
                    SetShake(20);
                }
            }

            PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }
}

void InitMine(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    if (weapon->count == 1)
    {
        weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;
        weapon->currentWeapon = WEAPON_SHAPE_MINE;
        weapon->pos.x = shipIndex[weapon->fireShip].pos.x;
        weapon->pos.y = shipIndex[weapon->fireShip].pos.y;
        weapon->pos.z = shipIndex[weapon->fireShip].pos.z;
        weapon->hdg = 0;
        weapon->updateWeapon = UpdateMine;
        weapon->count = MINE_DURATION;

        if (weapon->fireShip == ownShip)
        {
            PlayNote(SFX_MINE_DROP);
        }
    }
    else
    {
        weapon->count--;
    }
}


void UpdateMissile(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    vec3i targetVector;
    Face* facePtr;
    int i, mag, temp, vhdg, vpitch;
    const vec3i* facePoint;
    int alpha;
    int32 sx, sy;
    int32 cx, cy;

    if (weapon->targetShip > -1)
    {
        targetVector.x = (shipIndex[weapon->targetShip].pos.x - weapon->pos.x) >> 3;
        targetVector.y = (shipIndex[weapon->targetShip].pos.y - weapon->pos.y) >> 3;
        targetVector.z = (shipIndex[weapon->targetShip].pos.z - weapon->pos.z) >> 3;
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

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = track->vertices + facePtr->vertex[0];

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
        if (i == weapon->fireShip)
            continue;

        distance.x = (shipIndex[i].pos.x - weapon->pos.x) >> 3;
        distance.y = (shipIndex[i].pos.y - weapon->pos.y) >> 3;
        distance.z = (shipIndex[i].pos.z - weapon->pos.z) >> 3;

        mag = GetMagnitude(&distance);

        if (mag < 100)
        {
            if ((shipIndex[i].attr & SHIELDED) == 0)
            {
                if (i != ownShip)
                {
                    shipIndex[i].speed = shipIndex[i].speed >> 5;
                    shipIndex[i].vroll += 5000;
                    shipIndex[i].ahdg = (randy() - 16384) >> 1;
                }
                else
                {
                    shipIndex[i].vel.x -= (shipIndex[i].vel.x * 3) >> 2;
                    shipIndex[i].vel.y -= (shipIndex[i].vel.y * 3) >> 2;
                    shipIndex[i].vel.z -= (shipIndex[i].vel.z * 3) >> 2;
                    shipIndex[i].vroll += randy() % 250;
                    shipIndex[i].ahdg = (randy() - 16384) >> 1;
                    SetShake(20);
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

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
    {
        facePoint = track->vertices + facePtr->vertex[0];
        mag = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
        if (mag <= 0)
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

void InitMissile(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    Face* facePtr;

    if (weapon->fireShip != ownShip)
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
    else
    {
        weapon->count = MISSILE_DURATION;
    }
    weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;

    weapon->currentWeapon = WEAPON_SHAPE_MISSILE;
    weapon->targetShip = shipIndex[weapon->fireShip].targetShip;

    facePtr = track->faces + shipIndex[weapon->fireShip].nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    GetFireVector(&weapon->acc, shipIndex[weapon->fireShip].unitVecNose, facePtr, shipIndex[weapon->fireShip].pos);

    weapon->vel.x = (shipIndex[weapon->fireShip].vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (shipIndex[weapon->fireShip].vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (shipIndex[weapon->fireShip].vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = shipIndex[weapon->fireShip].pos.x;
    weapon->pos.y = shipIndex[weapon->fireShip].pos.y;
    weapon->pos.z = shipIndex[weapon->fireShip].pos.z;

    weapon->hdg = shipIndex[weapon->fireShip].hdg;
    weapon->pitch = shipIndex[weapon->fireShip].pitch;

    weapon->updateWeapon = UpdateMissile;
}

void UpdateRocket(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    Face* facePtr;
    int i, mag;
    const vec3i* facePoint;
    int alpha;

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, &weapon->pos);

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

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

    facePoint = track->vertices + facePtr->vertex[0];

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
        if (i == weapon->fireShip)
            continue;

        distance.x = (shipIndex[i].pos.x - weapon->pos.x) >> 2;
        distance.y = (shipIndex[i].pos.y - weapon->pos.y) >> 2;
        distance.z = (shipIndex[i].pos.z - weapon->pos.z) >> 2;

        mag = GetMagnitude(&distance);

        if (mag < 100)
        {
            if ((shipIndex[i].attr & SHIELDED) == 0)
            {

                if (i != ownShip)
                {
                    shipIndex[i].speed = shipIndex[i].speed >> 5;
                    shipIndex[i].vroll += randy() % 250;
                    shipIndex[i].ahdg = (randy() - 16384) >> 1;
                }
                else
                {
                    shipIndex[i].vel.x -= (shipIndex[i].vel.x * 3) >> 2;
                    shipIndex[i].vel.y -= (shipIndex[i].vel.y * 3) >> 2;
                    shipIndex[i].vel.z -= (shipIndex[i].vel.z * 3) >> 2;
                    shipIndex[i].vroll += randy() % 250;
                    shipIndex[i].ahdg = (randy() - 16384) >> 1;
                    SetShake(20);
                }
            }

            if (i != ownShip)
            {
                PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);
            }
            else
            {
                PlayNote(SFX_EXPLOSION_1);
                SetVolume(SFX_EXPLOSION_1, SND_MAX_VOLUME, SND_MAX_VOLUME);
            }

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }
    /***********************************************************/
    /****   Perform collision checks against track           ***/
    /***********************************************************/

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    if ((weapon->nearTrkSect->flags & Jump) == 0)
    {
        for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
        {
            facePoint = track->vertices + facePtr->vertex[0];

            mag = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
            if (mag <= 0)
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

void InitRocket(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    int i;
    Face* facePtr;

    if (weapon->fireShip != ownShip)
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
    else
    {
        weapon->count = ROCKET_DURATION;
    }

    weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_ROCKET;

    facePtr = track->faces + shipIndex[weapon->fireShip].nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    GetFireVector(&weapon->acc, shipIndex[weapon->fireShip].unitVecNose, facePtr, shipIndex[weapon->fireShip].pos);

    weapon->vel.x = (shipIndex[weapon->fireShip].vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (shipIndex[weapon->fireShip].vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (shipIndex[weapon->fireShip].vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = shipIndex[weapon->fireShip].pos.x;
    weapon->pos.y = shipIndex[weapon->fireShip].pos.y;
    weapon->pos.z = shipIndex[weapon->fireShip].pos.z;

    weapon->hdg = shipIndex[weapon->fireShip].hdg;
    weapon->pitch = shipIndex[weapon->fireShip].pitch;

    weapon->updateWeapon = UpdateRocket;
}


void InitSpecial(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    int32 i;
    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i != weapon->fireShip)
        {
            shipIndex[i].combatAttr |= SPECIALED;
            shipIndex[i].specialCount = SPECIAL_DURATION;
        }
    }

    RemoveWeapon(weapon);
}


void UpdateElecBolt(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    int i;
    vec3i distance;
    vec3i targetVector;
    Face* facePtr;
    int mag, temp, vhdg, vpitch;
    const vec3i* facePoint;
    int alpha;
    int32 sx, sy;
    int32 cx, cy;

    if (weapon->targetShip > -1)
    {
        targetVector.x = (shipIndex[weapon->targetShip].pos.x - weapon->pos.x) >> 3;
        targetVector.y = (shipIndex[weapon->targetShip].pos.y - weapon->pos.y) >> 3;
        targetVector.z = (shipIndex[weapon->targetShip].pos.z - weapon->pos.z) >> 3;
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

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = track->vertices + facePtr->vertex[0];

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
        if (i == weapon->fireShip)
            continue;

        distance.x = (shipIndex[i].pos.x - weapon->pos.x) >> 3;
        distance.y = (shipIndex[i].pos.y - weapon->pos.y) >> 3;
        distance.z = (shipIndex[i].pos.z - weapon->pos.z) >> 3;

        mag = GetMagnitude(&distance);

        if (mag < 100)
        {
            if ((shipIndex[i].attr & SHIELDED) == 0)
            {
                shipIndex[i].attr |= ELECTROED;
                shipIndex[i].electroCount = ELEC_DURATION;
            }

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }

    /***********************************************************/
    /****   Perform collision checks against track           ***/
    /***********************************************************/

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
    {
        facePoint = track->vertices + facePtr->vertex[0];

        mag = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
        if (mag <= 0)
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

void InitElecBolt(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    if (weapon->fireShip != ownShip)
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
    else
    {
        weapon->count = ELEC_DURATION;
    }

    weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_EBOLT;
    weapon->targetShip = shipIndex[weapon->fireShip].targetShip;

    weapon->acc.x = shipIndex[weapon->fireShip].unitVecNose.x >> 8;
    weapon->acc.y = shipIndex[weapon->fireShip].unitVecNose.y >> 8;
    weapon->acc.z = shipIndex[weapon->fireShip].unitVecNose.z >> 8;

    weapon->vel.x = (shipIndex[weapon->fireShip].vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (shipIndex[weapon->fireShip].vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (shipIndex[weapon->fireShip].vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = shipIndex[weapon->fireShip].pos.x;
    weapon->pos.y = shipIndex[weapon->fireShip].pos.y;
    weapon->pos.z = shipIndex[weapon->fireShip].pos.z;

    weapon->hdg = shipIndex[weapon->fireShip].hdg;
    weapon->pitch = shipIndex[weapon->fireShip].pitch;
    weapon->updateWeapon = UpdateElecBolt;
}


void UpdateRevCon(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    vec3i distance;
    vec3i targetVector;
    const vec3i* facePoint;
    Face* facePtr;
    int i, mag, temp, vhdg, vpitch;
    int alpha;
    int32 sx, sy;
    int32 cx, cy;

    if (weapon->targetShip > -1)
    {
        targetVector.x = (shipIndex[weapon->targetShip].pos.x - weapon->pos.x) >> 3;
        targetVector.y = (shipIndex[weapon->targetShip].pos.y - weapon->pos.y) >> 3;
        targetVector.z = (shipIndex[weapon->targetShip].pos.z - weapon->pos.z) >> 3;
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

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = track->vertices + facePtr->vertex[0];

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
        if (i == weapon->fireShip)
            continue;

        distance.x = (shipIndex[i].pos.x - weapon->pos.x) >> 3;
        distance.y = (shipIndex[i].pos.y - weapon->pos.y) >> 3;
        distance.z = (shipIndex[i].pos.z - weapon->pos.z) >> 3;

        mag = GetMagnitude(&distance);

        if (mag < 100)
        {
            if ((shipIndex[i].attr & SHIELDED) == 0)
            {
                if (i != ownShip)
                {
                    shipIndex[i].combatAttr |= REVCONNED;
                    shipIndex[i].revConCount = REV_CON_DURATION;
                }
                else
                {
                    shipIndex[i].combatAttr |= REVCONNED;
                    shipIndex[i].revConCount = REV_CON_DURATION;
                }
            }

            AddExplosion(weapon, i);
            RemoveWeapon(weapon);
            return;
        }
    }

    /***********************************************************/
    /****   Perform collision checks against track           ***/
    /***********************************************************/

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    for (i = 0; i < weapon->nearTrkSect->faceCount; i++)
    {
        facePoint = track->vertices + facePtr->vertex[0];

        mag = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);
        if (mag <= 0)
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

void InitRevCon(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    if (weapon->fireShip != ownShip)
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
    else
    {
        weapon->count = ELEC_DURATION;
    }

    weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_EBOLT;
    weapon->targetShip = shipIndex[weapon->fireShip].targetShip;

    weapon->acc.x = shipIndex[weapon->fireShip].unitVecNose.x >> 8;
    weapon->acc.y = shipIndex[weapon->fireShip].unitVecNose.y >> 8;
    weapon->acc.z = shipIndex[weapon->fireShip].unitVecNose.z >> 8;

    weapon->vel.x = (shipIndex[weapon->fireShip].vel.x >> VELOCITY_SHIFT);
    weapon->vel.y = (shipIndex[weapon->fireShip].vel.y >> VELOCITY_SHIFT);
    weapon->vel.z = (shipIndex[weapon->fireShip].vel.z >> VELOCITY_SHIFT);

    weapon->pos.x = shipIndex[weapon->fireShip].pos.x;
    weapon->pos.y = shipIndex[weapon->fireShip].pos.y;
    weapon->pos.z = shipIndex[weapon->fireShip].pos.z;

    weapon->hdg = shipIndex[weapon->fireShip].hdg;
    weapon->pitch = shipIndex[weapon->fireShip].pitch;
    weapon->updateWeapon = UpdateRevCon;
}


#ifdef USE_OLD_SHIELD
void UpdateShieldColors(Object* object, int32 counter)
{
    int i;
    Prm poly;
    short primitiveCount;
    short* coords;
    short col0, col1, col2, col3;

    poly.primitive = object->primitives;
    primitiveCount = object->primitiveCount;

    for (i = 0; i < primitiveCount; i++)
    {
        switch (poly.primitive->type)
        {
            case TypeG3:
                coords = poly.g3->coords;

                col0 = (rsin(counter * coords[0]) >> 5) + 128;
                col1 = (rsin(counter * coords[1]) >> 5) + 128;
                col2 = (rsin(counter * coords[2]) >> 5) + 128;

                if (col0 > 255)
                    col0 = 255;
                if (col1 > 255)
                    col1 = 255;
                if (col2 > 255)
                    col2 = 255;

                poly.g3->colour[0].r = (uint8)(col0 >> 1);
                poly.g3->colour[1].r = (uint8)(col1 >> 1);
                poly.g3->colour[2].r = (uint8)(col2 >> 1);
                poly.g3->colour[0].g = 64;
                poly.g3->colour[1].g = 64;
                poly.g3->colour[2].g = 64;
                poly.g3->colour[0].b = 128;
                poly.g3->colour[1].b = 128;
                poly.g3->colour[2].b = 128;

                poly.g3 += 1;
                break;

            case TypeG4:
                coords = poly.g4->coords;

                col0 = (rsin(counter * coords[0]) >> 5) + 128;
                col1 = (rsin(counter * coords[1]) >> 5) + 128;
                col2 = (rsin(counter * coords[2]) >> 5) + 128;
                col3 = (rsin(counter * coords[3]) >> 5) + 128;

                if (col0 > 255)
                    col0 = 255;
                if (col1 > 255)
                    col1 = 255;
                if (col2 > 255)
                    col2 = 255;
                if (col3 > 255)
                    col2 = 255;

                poly.g4->colour[0].r = (uint8)(col0 >> 1);
                poly.g4->colour[1].r = (uint8)(col1 >> 1);
                poly.g4->colour[2].r = (uint8)(col2 >> 1);
                poly.g4->colour[3].r = (uint8)(col3 >> 1);
                poly.g4->colour[0].g = 64;
                poly.g4->colour[1].g = 64;
                poly.g4->colour[2].g = 64;
                poly.g4->colour[3].g = 64;
                poly.g4->colour[0].b = 128;
                poly.g4->colour[1].b = 128;
                poly.g4->colour[2].b = 128;
                poly.g4->colour[3].b = 128;

                poly.g4 += 1;
                break;
        }
    }
}
#endif

void UpdateShield(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    if (shipIndex[weapon->fireShip].attr & VIEW_IN)
    {
        weapon->pos.x = shipIndex[weapon->fireShip].pos.x + (shipIndex[weapon->fireShip].unitVecNose.x >> 6);
        weapon->pos.y = shipIndex[weapon->fireShip].pos.y + (shipIndex[weapon->fireShip].unitVecNose.y >> 6);
        weapon->pos.z = shipIndex[weapon->fireShip].pos.z + (shipIndex[weapon->fireShip].unitVecNose.z >> 6);
    }
    else
    {
        weapon->pos.x = shipIndex[weapon->fireShip].pos.x;
        weapon->pos.y = shipIndex[weapon->fireShip].pos.y;
        weapon->pos.z = shipIndex[weapon->fireShip].pos.z;
    }

    weapon->count--;

    if (weapon->count == 0)
    {
        if (weapon->fireShip == ownShip)
        {
            KillNote(SFX_SHIELDS);
        }

        shipIndex[weapon->fireShip].attr &= ~SHIELDED;

        RemoveWeapon(weapon);
        return;
    }

    if ((shipIndex[weapon->fireShip].attr & VIEW_IN) == 0)
    {
        weapon->hdg = shipIndex[weapon->fireShip].hdg;
        weapon->pitch = shipIndex[weapon->fireShip].pitch;

#ifdef USE_OLD_SHIELD
        UpdateShieldColors(weaponShapes[weapon->currentWeapon].shapePtr, weapon->count);
#endif
    }
}

void InitShield(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon)
{
    weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;
    weapon->currentWeapon = WEAPON_SHAPE_SHIELD;

    weapon->pos.x = shipIndex[weapon->fireShip].pos.x;
    weapon->pos.y = shipIndex[weapon->fireShip].pos.y;
    weapon->pos.z = shipIndex[weapon->fireShip].pos.z;

    weapon->hdg = shipIndex[weapon->fireShip].hdg;
    weapon->pitch = shipIndex[weapon->fireShip].pitch;

    if (weapon->fireShip == ownShip)
    {
        PlayNote(SFX_SHIELDS);
        SetPitch(SFX_SHIELDS, SND_DEF_PITCH >> 1);
    }
    weapon->updateWeapon = UpdateShield;
}


void FireWeapon(ShipData* playerShip, int shipNo, char decision)
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
            weapon->fireShip = shipNo;
            weapon->type = WPN_ROCKET;
            playerShip->weaponType = WPN_NONE;
            playerShip->haveFired = WPN_ROCKET;
            if (shipNo == ownShip)
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
            weapon->fireShip = shipNo;
            weapon->type = WPN_MISSILE;
            playerShip->weaponType = WPN_NONE;
            playerShip->haveFired = WPN_MISSILE;
            if (shipNo == ownShip)
            {
                PlayNote(SFX_MISS_LAUNCH);
                SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH);
            }
            break;
        }

        case WPN_MINE:
        {
            int32 i;
            int32 count = (weapon->fireShip != ownShip) ? WEAPON_DELAY : 0;

            for (i = 0; i < MINES_COUNT; i++)
            {
                count += MINE_RELEASE_RATE;
                weapon->updateWeapon = InitMine;
                weapon->fireShip = shipNo;
                weapon->count = count;
                weapon->type = WPN_MINE;

                if (i < MINES_COUNT - 1)
                {
                    weapon = CreateWeapon();
                    if (!weapon)
                        break;
                }
            }

            playerShip->weaponType = WPN_NONE;
            playerShip->haveFired = WPN_MINE;
            break;
        }

        case WPN_SPECIAL:
        {
            weapon->updateWeapon = InitSpecial;
            weapon->count = 1;
            weapon->fireShip = shipNo;
            playerShip->weaponType = WPN_NONE;
            playerShip->haveFired = WPN_SPECIAL;
            break;
        }

        case WPN_ELEC_BOLT:
        {
            weapon->updateWeapon = InitElecBolt;
            weapon->count = WEAPON_DELAY;
            weapon->fireShip = shipNo;
            weapon->type = WPN_ELEC_BOLT;
            playerShip->haveFired = WPN_ELEC_BOLT;
            playerShip->weaponType = WPN_NONE;
            if (shipNo == ownShip)
            {
                PlayNote(SFX_ELECTROBOLT);
            }
            break;
        }

        case WPN_REV_CON:
        {
            weapon->updateWeapon = InitRevCon;
            weapon->count = WEAPON_DELAY;
            weapon->fireShip = shipNo;
            weapon->type = WPN_REV_CON;
            playerShip->haveFired = WPN_REV_CON;
            playerShip->weaponType = WPN_NONE;
            if (shipNo == ownShip)
            {
                PlayNote(SFX_ELECTROBOLT);
            }
            break;
        }

        case WPN_SHIELD:
        {
            weapon->updateWeapon = InitShield;
            weapon->count = SHIELD_DURATION;
            weapon->fireShip = shipNo;
            weapon->type = WPN_SHIELD;
            playerShip->attr |= SHIELDED;
            playerShip->haveFired = WPN_SHIELD;
            playerShip->weaponType = WPN_NONE;
            break;
        }

        case WPN_TURBO:
        {
            playerShip->weaponType = WPN_NONE;

            playerShip->vel.x += playerShip->unitVecNose.x * 48 / 5;
            playerShip->vel.y += playerShip->unitVecNose.y * 48 / 5;
            playerShip->vel.z += playerShip->unitVecNose.z * 48 / 5;

            if (shipNo == ownShip)
            {
                PlayNote(SFX_MISS_LAUNCH);
                SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH >> 2);
                gTargetFOV = CAMERA_FOV_EXTRA;
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

#ifdef __3DO__ // TODO optimize
    return;
#endif

    for (i = 0; i < noWeaponGrids; i++)
    {
        facePtr = track->faces + weaponGrid[i].section->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        for (j = 0; j < NO_OF_SHIPS; j++)
        {
            if ((shipIndex[j].combatAttr & SPECIALED) == 0)
            {
                if (((facePtr->flags & WEAPON_GRID_LEFT) && (shipIndex[j].attr & LSIDE)))
                {
                    if ((weaponGrid[i].section == shipIndex[j].nearTrkSect) && (shipIndex[j].weaponType == WPN_NONE) && (weaponGrid[i].count >= 0))
                    {
                        weaponGrid[i].count = -30;
                        if (j == ownShip)
                        {
                            PlayNote(SFX_POWER_UP);
                            shipIndex[j].weaponType = GetNewWeapon();
                        }
                        else
                            shipIndex[j].weaponType = 1;
                    }
                }
            }
            facePtr++;
            if ((shipIndex[j].combatAttr & SPECIALED) == 0)
            {
                if ((facePtr->flags & WEAPON_GRID_RIGHT) && ((shipIndex[j].attr & LSIDE) == 0))
                {
                    if ((weaponGrid[i].section == shipIndex[j].nearTrkSect) && (shipIndex[j].weaponType == WPN_NONE) && (weaponGrid[i].count >= 0))
                    {
                        weaponGrid[i].count = -30;
                        if (j == ownShip)
                        {
                            PlayNote(SFX_POWER_UP);
                            shipIndex[j].weaponType = GetNewWeapon();
                        }
                        else
                            shipIndex[j].weaponType = 1;
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

            weaponGrid[i].colour.r = r;
            weaponGrid[i].colour.g = g;
            weaponGrid[i].colour.b = b;
        }
        else
        {
            weaponGrid[i].colour.r = 128;
            weaponGrid[i].colour.g = 128;
            weaponGrid[i].colour.b = 128;
        }

        if (weaponGrid[i].count < 0)
        {
            weaponGrid[i].count++;
        }
        weaponGrid[i].cycleCount += 32;

        if (weaponGrid[i].attr & WEAPON_GRID_LEFT)
        {
        #ifndef __3DO__ // TODO track attribs
            facePtr->red = weaponGrid[i].colour.r;
            facePtr->green = weaponGrid[i].colour.g;
            facePtr->blue = weaponGrid[i].colour.b;
        #endif
        }
        if (weaponGrid[i].attr & WEAPON_GRID_RIGHT)
        {
            facePtr++;
        #ifndef __3DO__ // TODO track attribs
            facePtr->red = weaponGrid[i].colour.r;
            facePtr->green = weaponGrid[i].colour.g;
            facePtr->blue = weaponGrid[i].colour.b;
        #endif
        }
    }
}
