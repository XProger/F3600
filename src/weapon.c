#include "game.h"

//#define OLD_SHIELD

extern long ctrlNeedTargetIcon;
extern int ctrlnearShip;
extern TrackCamera ctrlcamera;

Texture* targetTexture;

void UpdateWeapons(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weaponIndex)
{
    int i;

    for (i = 0; i < NO_ACTIVE_WEAPONS; i++)
    {
        if (weaponIndex[i].count > 0)
        {
            (weaponIndex[i].updateWeapon)(shipIndex, weaponShapes, &(weaponIndex[i]), weaponIndex);
        }
    }
}

void DrawWeapons(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weaponIndex)
{
    int i;
    int16 roll;
    WeaponData* weapon;
    Object* obj;

    for (i = 0; i < NO_ACTIVE_WEAPONS; i++)
    {
        if ((weaponIndex[i].count > 0) && (weaponIndex[i].availible))
        {
            weapon = weaponIndex + i;
            obj = weaponShapes[weapon->currentWeapon].shapePtr;

            if (weapon->type == SHIELD)
            {
                SetSkeletonPosition(obj->skeleton, weapon->pos.vx, weapon->pos.vy, weapon->pos.vz);
                SetSkeletonDirectionHPR(obj->skeleton, weapon->hdg, weapon->pitch, shipIndex[weapon->fireShip].roll);
                renderShield(obj, weapon->count);
            }
            else
            {
                DrawObject(weaponShapes[weapon->currentWeapon].shapePtr, weapon->pos.vx, weapon->pos.vy, weapon->pos.vz, weapon->hdg, weapon->pitch, 0);
            }
        }
    }
}

void FireWeapon(ShipData* playerShip, WeaponData* weaponIndex, int shipNo, char decision)
{
    int i;
    int count = 0;

    for (i = 0; i < NO_ACTIVE_WEAPONS; i++)
    {
        if (weaponIndex[i].count == 0)
        {
            switch (decision)
            {
                case ROCKET:
                    weaponIndex[i].updateWeapon = InitRocket;
                    weaponIndex[i].count = WEAPON_DELAY;
                    weaponIndex[i].fireShip = shipNo;
                    if (shipNo == ownShip)
                    {
                        PlayNote(SFX_MISS_LAUNCH);
                        SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH);
                    }
                    weaponIndex[i].type = ROCKET;
                    playerShip->weaponType = NO_WEAPON;
                    playerShip->haveFired = ROCKET;
                    return;

                case MISSILE:
                    weaponIndex[i].updateWeapon = InitMissile;
                    weaponIndex[i].count = WEAPON_DELAY;
                    weaponIndex[i].fireShip = shipNo;
                    if (shipNo == ownShip)
                    {
                        PlayNote(SFX_MISS_LAUNCH);
                        SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH);
                    }
                    weaponIndex[i].type = MISSILE;
                    playerShip->weaponType = NO_WEAPON;
                    playerShip->haveFired = MISSILE;
                    return;

                case MINE:
                    weaponIndex[i].updateWeapon = InitMine;
                    count += MINE_RELEASE_RATE;
                    weaponIndex[i].fireShip = shipNo;
                    if (weaponIndex[i].fireShip != ownShip)
                    {
                        weaponIndex[i].count = (count + WEAPON_DELAY);
                    }
                    else
                    {
                        weaponIndex[i].count = count;
                    }
                    weaponIndex[i].type = MINE;
                    if (count >= (5 * MINE_RELEASE_RATE))
                    {
                        playerShip->weaponType = NO_WEAPON;
                        playerShip->haveFired = MINE;
                        return;
                    }
                    break;

                case SPECIAL:
                    weaponIndex[i].updateWeapon = InitSpecial;
                    weaponIndex[i].count = 1;
                    playerShip->weaponType = NO_WEAPON;
                    playerShip->haveFired = SPECIAL;
                    weaponIndex[i].fireShip = shipNo;
                    return;

                case ELEC_BOLT:
                    weaponIndex[i].updateWeapon = InitElecBolt;
                    weaponIndex[i].count = WEAPON_DELAY;
                    weaponIndex[i].fireShip = shipNo;
                    weaponIndex[i].type = ELEC_BOLT;
                    playerShip->haveFired = ELEC_BOLT;
                    playerShip->weaponType = NO_WEAPON;
                    if (shipNo == ownShip)
                    {
                        PlayNote(SFX_ELECTROBOLT);
                    }
                    return;

                case REV_CON:
                    weaponIndex[i].updateWeapon = InitRevCon;
                    weaponIndex[i].count = WEAPON_DELAY;
                    weaponIndex[i].fireShip = shipNo;
                    weaponIndex[i].type = REV_CON;
                    playerShip->haveFired = REV_CON;
                    playerShip->weaponType = NO_WEAPON;
                    if (shipNo == ownShip)
                    {
                        PlayNote(SFX_ELECTROBOLT);
                    }
                    return;

                case SHIELD:
                    weaponIndex[i].updateWeapon = InitShield;
                    weaponIndex[i].count = SHIELD_DURATION;
                    weaponIndex[i].fireShip = shipNo;
                    weaponIndex[i].type = SHIELD;
                    playerShip->attr |= SHIELDED;
                    playerShip->haveFired = SHIELD;
                    playerShip->weaponType = NO_WEAPON;
                    return;

                case TURBO:
                    playerShip->weaponType = NO_WEAPON;

                    playerShip->vel.vx += (((playerShip->unitVecNose.vx) << 3) * FR60) / 50;
                    playerShip->vel.vy += (((playerShip->unitVecNose.vy) << 3) * FR60) / 50;
                    playerShip->vel.vz += (((playerShip->unitVecNose.vz) << 3) * FR60) / 50;

                    if (shipNo == ownShip)
                    {
                        PlayNote(SFX_MISS_LAUNCH);
                        SetPitch(SFX_MISS_LAUNCH, SND_DEF_PITCH >> 2);
                        gTargetFOV = CAMERA_FOV_EXTRA;
                    }
                    return;
            }
        }
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
                    section = section->nextSection.ptr;
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
                    if (section->junction.index != -1)
                    {
                        section = section->junction.ptr;
                    }
                    else
                    {
                        section = section->nextSection.ptr;
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

void DrawTargetIcon(ShipData* playerShip, ShipData* shipIndex, Object** shipShapes)
{
    SVECTOR* vertex;
    DVECTOR sxy;
    long flag;
    const MATRIX* matrix;

    shipShapes[shipIndex[ctrlnearShip].highResTableIndex]->skeleton->update = 1;
    vertex = shipShapes[shipIndex[ctrlnearShip].highResTableIndex]->vertices;
    matrix = GetSkeletonMatrix(shipShapes[shipIndex[ctrlnearShip].highResTableIndex]->skeleton);

    RotTransPers(matrix, vertex, &sxy, &flag);

    if ((flag & (CrtClipped | ZNegative | ZLarge | ZSmall)))
        return;

    renderSetBlend(BLEND_ADD);
    AddGlyph(sxy.vx - (targetTexture->width >> 1), sxy.vy - (targetTexture->height >> 1), targetTexture->width, targetTexture->height, 0, 0, TEXT_COLOR_WHITE, targetTexture);
    renderSetBlend(BLEND_ALPHA);
}

void InitWeaponObj(WeaponShape* weaponShapes, int textures, const char* name, int offset, int count)
{
    long length;
    int i;
    char* start;
    Prm prm;
    Object* object;

    length = FileLength(name);
    weaponShapes[offset].shapePtr = LoadPrm(name, textures);
    for (i = offset + 1; i < offset + count; i++)
    {
        start = (char*)malloc(length + 32);
        prm.ptr = start;
        weaponShapes[i].shapePtr = prm.object;
        memcpy(start, weaponShapes[offset].shapePtr, length);
        object = prm.object++;
        object->skeleton = prm.skeleton++;
        object->vertices = prm.svector;
        prm.svector += object->vertexCount;
        object->normals = prm.svector;
        prm.svector += object->normalCount;
        object->primitives = prm.primitive;
    }
}

void initWeapons(WeaponShape* weaponShapes, Skeleton* camPos, WeaponData* weaponIndex)
{
    int i;
    Object* obj;
    int weaponTextures;

    targetTexture = LoadTexture("wipeout\\textures\\target2.tim");

    weaponTextures = TextureTableCount;
    LoadCompressedTextureSequence("wipeout\\common\\mine.cmp", 1);

    InitWeaponObj(weaponShapes, weaponTextures, "wipeout\\common\\rock.prm", ROCKET_OFFSET, MINE_OFFSET - ROCKET_OFFSET);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout\\common\\mine.prm", MINE_OFFSET, MISSILE_OFFSET - MINE_OFFSET);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout\\common\\miss.prm", MISSILE_OFFSET, SHIELD_OFFSET - MISSILE_OFFSET);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout\\common\\shld.prm", SHIELD_OFFSET, NO_OF_SHIELDS);
    InitWeaponObj(weaponShapes, weaponTextures, "wipeout\\common\\ebolt.prm", EBOLT_OFFSET, NO_OF_EBOLTS);

    for (i = 0; i < NO_OF_WEAPONS; i++)
    {
        weaponShapes[i].used = 0;
        obj = weaponShapes[i].shapePtr;
        while (obj)
        {
            obj->skeleton->super = camPos;
            obj = obj->next;
        }
    }

    for (i = 0; i < NO_ACTIVE_WEAPONS; i++)
    {
        weaponIndex[i].count = 0;
        weaponIndex[i].availible = 0;
        weaponIndex[i].type = NO_WEAPON;
    }
}

void UpdateRocket(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    VECTOR distance;
    Face* facePtr;
    int i, mag;
    const VECTOR* facePoint;
    int alpha;

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, weapon->pos);

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    weapon->vel.vx += weapon->acc.vx;
    weapon->vel.vy += weapon->acc.vy;
    weapon->vel.vz += weapon->acc.vz;

    weapon->vel.vx -= weapon->vel.vx >> 5;
    weapon->vel.vy -= weapon->vel.vy >> 5;
    weapon->vel.vz -= weapon->vel.vz >> 5;

    weapon->pos.vx += weapon->vel.vx;
    weapon->pos.vy += weapon->vel.vy;
    weapon->pos.vz += weapon->vel.vz;

    facePoint = track->vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.vx += (((200 - alpha) * (int)facePtr->normal.vx) >> 13);
        weapon->pos.vy += (((200 - alpha) * (int)facePtr->normal.vy) >> 13);
        weapon->pos.vz += (((200 - alpha) * (int)facePtr->normal.vz) >> 13);
    }

    weapon->count--;

    if (weapon->count == 0)
    {
        weaponShapes[weapon->currentWeapon].used = 0;
        weaponIndex[weapon->currentWeapon].availible = 0;
    }

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i == weapon->fireShip)
            continue;

        distance.vx = (shipIndex[i].pos.vx - weapon->pos.vx) >> 2;
        distance.vy = (shipIndex[i].pos.vy - weapon->pos.vy) >> 2;
        distance.vz = (shipIndex[i].pos.vz - weapon->pos.vz) >> 2;

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
                    shipIndex[i].vel.vx -= (shipIndex[i].vel.vx * 3) >> 2;
                    shipIndex[i].vel.vy -= (shipIndex[i].vel.vy * 3) >> 2;
                    shipIndex[i].vel.vz -= (shipIndex[i].vel.vz * 3) >> 2;
                    shipIndex[i].vroll += randy() % 250;
                    shipIndex[i].ahdg = (randy() - 16384) >> 1;
                    SetShake(20);
                }
            }
            weapon->count = SHIP_EXPLODE;
            weapon->targetShip = i;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;

            if (i != ownShip)
            {
                PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);
            }
            else
            {
                PlayNote(SFX_EXPLOSION_1);
                SetVolume(SFX_EXPLOSION_1, SND_MAX_VOLUME, SND_MAX_VOLUME);
            }
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
                weapon->count = TRACK_EXPLODE;
                weaponShapes[weapon->currentWeapon].used = 0;
                weaponIndex[weapon->currentWeapon].availible = 0;
                PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);
                return;
            }
            facePtr++;
        }
    }

    Weapon2Mine(weapon, weaponIndex, weaponShapes);
}

void UpdateMissile(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    VECTOR distance;
    VECTOR targetVector;
    Face* facePtr;
    int i, mag, temp, vhdg, vpitch;
    const VECTOR* facePoint;
    int alpha;

    if (weapon->targetShip > -1)
    {
        targetVector.vx = (shipIndex[weapon->targetShip].pos.vx - weapon->pos.vx) >> 3;
        targetVector.vy = (shipIndex[weapon->targetShip].pos.vy - weapon->pos.vy) >> 3;
        targetVector.vz = (shipIndex[weapon->targetShip].pos.vz - weapon->pos.vz) >> 3;
        temp = SquareRoot0(targetVector.vx * targetVector.vx + targetVector.vz * targetVector.vz);

        vhdg = ((-ratan2(targetVector.vx, targetVector.vz)) - weapon->hdg);
        vpitch = ((-ratan2(targetVector.vy, temp)) - weapon->pitch);
    }
    else
    {
        vhdg = 0;
        vpitch = 0;
    }

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, weapon->pos);

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

    weapon->acc.vx = (-rsin(weapon->hdg) * rcos(weapon->pitch)) >> 16;
    weapon->acc.vy = -rsin(weapon->pitch) >> 4;
    weapon->acc.vz = (rcos(weapon->hdg) * rcos(weapon->pitch)) >> 16;

    weapon->acc.vx = (weapon->acc.vx * 60) / FR60;
    weapon->acc.vy = (weapon->acc.vy * 60) / FR60;
    weapon->acc.vz = (weapon->acc.vz * 60) / FR60;

    weapon->vel.vx += weapon->acc.vx;
    weapon->vel.vy += weapon->acc.vy;
    weapon->vel.vz += weapon->acc.vz;

    weapon->vel.vx -= weapon->vel.vx >> 2;
    weapon->vel.vy -= weapon->vel.vy >> 2;
    weapon->vel.vz -= weapon->vel.vz >> 2;

    weapon->pos.vx += weapon->vel.vx;
    weapon->pos.vy += weapon->vel.vy;
    weapon->pos.vz += weapon->vel.vz;

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = track->vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.vx += (((200 - alpha) * (int)facePtr->normal.vx) >> 13);
        weapon->pos.vy += (((200 - alpha) * (int)facePtr->normal.vy) >> 13);
        weapon->pos.vz += (((200 - alpha) * (int)facePtr->normal.vz) >> 13);
    }

    weapon->count--;

    if (weapon->count == NON_ACTIVE)
    {
        weaponShapes[weapon->currentWeapon].used = 0;
        weaponIndex[weapon->currentWeapon].availible = 0;
    }

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i == weapon->fireShip)
            continue;

        distance.vx = (shipIndex[i].pos.vx - weapon->pos.vx) >> 3;
        distance.vy = (shipIndex[i].pos.vy - weapon->pos.vy) >> 3;
        distance.vz = (shipIndex[i].pos.vz - weapon->pos.vz) >> 3;

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
                    shipIndex[i].vel.vx -= (shipIndex[i].vel.vx * 3) >> 2;
                    shipIndex[i].vel.vy -= (shipIndex[i].vel.vy * 3) >> 2;
                    shipIndex[i].vel.vz -= (shipIndex[i].vel.vz * 3) >> 2;
                    shipIndex[i].vroll += randy() % 250;
                    shipIndex[i].ahdg = (randy() - 16384) >> 1;
                    SetShake(20);
                }
            }
            weapon->count = SHIP_EXPLODE;
            weapon->targetShip = i;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;

            PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);
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
            weapon->count = TRACK_EXPLODE;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;
            PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);
            return;
        }
        facePtr++;
    }

    Weapon2Mine(weapon, weaponIndex, weaponShapes);
}

void InitMissile(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
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
            weapon->count = MISSILE_DURATION;
        }
    }
    else
    {
        weapon->count = MISSILE_DURATION;
    }
    weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;
    for (i = 0; i < NO_OF_MISSILES; i++)
    {
        if (weaponShapes[MISSILE_OFFSET + i].used == 0)
        {
            weapon->currentWeapon = MISSILE_OFFSET + i;
            weaponShapes[MISSILE_OFFSET + i].used = 1;
            weapon->targetShip = shipIndex[weapon->fireShip].targetShip;

            facePtr = track->faces + shipIndex[weapon->fireShip].nearTrkSect->faceStart;

            while ((facePtr->flags & TRACK_BASE) == 0)
            {
                facePtr++;
            }

            GetFireVector(&weapon->acc, shipIndex[weapon->fireShip].unitVecNose, facePtr, shipIndex[weapon->fireShip].pos);

            weapon->acc.vx = (weapon->acc.vx * 60) / FR60;
            weapon->acc.vy = (weapon->acc.vy * 60) / FR60;
            weapon->acc.vz = (weapon->acc.vz * 60) / FR60;

            weapon->vel.vx = (shipIndex[weapon->fireShip].vel.vx >> 6);
            weapon->vel.vy = (shipIndex[weapon->fireShip].vel.vy >> 6);
            weapon->vel.vz = (shipIndex[weapon->fireShip].vel.vz >> 6);

            weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx;
            weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy;
            weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz;

            weapon->hdg = shipIndex[weapon->fireShip].hdg;
            weapon->pitch = shipIndex[weapon->fireShip].pitch;

            weapon->updateWeapon = UpdateMissile;
            weapon->availible = 1; // AJY
            return;
        }
    }
    weapon->count = NON_ACTIVE;
}

void UpdateMine(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    VECTOR distance;
    int ossilation;
    int i, mag;
    Prm poly;
    short r;

    weapon->count--;

    if (weapon->count == NON_ACTIVE)
    {
        weaponShapes[weapon->currentWeapon].used = 0;
        weaponIndex[weapon->currentWeapon].availible = 0;
        return;
    }

    ossilation = sar(rsin((weapon->currentWeapon * 400) + (weapon->count * 40)), 9);

    weapon->pos.vx += ossilation;
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

        distance.vx = (shipIndex[i].pos.vx - weapon->pos.vx) >> 2;
        distance.vy = (shipIndex[i].pos.vy - weapon->pos.vy) >> 2;
        distance.vz = (shipIndex[i].pos.vz - weapon->pos.vz) >> 2;

        mag = GetMagnitude(&distance);

        if (mag < 100)
        {
            if ((shipIndex[i].attr & SHIELDED) == 0)
            {
                weapon->count = SHIP_EXPLODE;
                weapon->targetShip = i;
                if (i != ownShip)
                {
                    shipIndex[i].speed = shipIndex[i].speed >> 3;
                }
                else
                {
                    shipIndex[i].vel.vx -= shipIndex[i].vel.vx >> 3;
                    shipIndex[i].vel.vy -= shipIndex[i].vel.vy >> 3;
                    shipIndex[i].vel.vz -= shipIndex[i].vel.vz >> 3;
                    SetShake(20);
                }
            }
            weapon->count = SHIP_EXPLODE;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;
            PlayDirectionalNote(SFX_EXPLOSION_1, &weapon->pos);
            return;
        }
    }
}

void InitRocket(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
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

    for (i = 0; i < NO_OF_ROCKETS; i++)
    {
        if (weaponShapes[ROCKET_OFFSET + i].used == 0)
        {
            weapon->currentWeapon = ROCKET_OFFSET + i;
            weaponShapes[ROCKET_OFFSET + i].used = 1;

            facePtr = track->faces + shipIndex[weapon->fireShip].nearTrkSect->faceStart;

            while ((facePtr->flags & TRACK_BASE) == 0)
            {
                facePtr++;
            }

            GetFireVector(&weapon->acc, shipIndex[weapon->fireShip].unitVecNose, facePtr, shipIndex[weapon->fireShip].pos);

            weapon->acc.vx = (weapon->acc.vx * 60) / FR60;
            weapon->acc.vy = (weapon->acc.vy * 60) / FR60;
            weapon->acc.vz = (weapon->acc.vz * 60) / FR60;

            weapon->vel.vx = (shipIndex[weapon->fireShip].vel.vx >> 6);
            weapon->vel.vy = (shipIndex[weapon->fireShip].vel.vy >> 6);
            weapon->vel.vz = (shipIndex[weapon->fireShip].vel.vz >> 6);

            weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx;
            weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy;
            weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz;

            weapon->hdg = shipIndex[weapon->fireShip].hdg;
            weapon->pitch = shipIndex[weapon->fireShip].pitch;

            weapon->updateWeapon = UpdateRocket;
            weapon->availible = 1; // AJY
            return;
        }
    }
    weapon->count = NON_ACTIVE;
}

void UpdateElecBolt(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    VECTOR distance;
    VECTOR targetVector;
    Face* facePtr;
    int i, mag, temp, vhdg, vpitch;
    const VECTOR* facePoint;
    int alpha;

    if (weapon->targetShip > -1)
    {
        targetVector.vx = (shipIndex[weapon->targetShip].pos.vx - weapon->pos.vx) >> 3;
        targetVector.vy = (shipIndex[weapon->targetShip].pos.vy - weapon->pos.vy) >> 3;
        targetVector.vz = (shipIndex[weapon->targetShip].pos.vz - weapon->pos.vz) >> 3;
        temp = SquareRoot0(targetVector.vx * targetVector.vx + targetVector.vz * targetVector.vz);

        vhdg = ((-ratan2(targetVector.vx, targetVector.vz)) - weapon->hdg);
        vpitch = ((-ratan2(targetVector.vy, temp)) - weapon->pitch);
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

    weapon->acc.vx = (-rsin(weapon->hdg) * rcos(weapon->pitch)) >> 16;
    weapon->acc.vy = -rsin(weapon->pitch) >> 4;
    weapon->acc.vz = (rcos(weapon->hdg) * rcos(weapon->pitch)) >> 16;

    weapon->acc.vx = (weapon->acc.vx * 60) / FR60;
    weapon->acc.vy = (weapon->acc.vy * 60) / FR60;
    weapon->acc.vz = (weapon->acc.vz * 60) / FR60;

    weapon->vel.vx += weapon->acc.vx;
    weapon->vel.vy += weapon->acc.vy;
    weapon->vel.vz += weapon->acc.vz;

    weapon->vel.vx -= weapon->vel.vx >> 2;
    weapon->vel.vy -= weapon->vel.vy >> 2;
    weapon->vel.vz -= weapon->vel.vz >> 2;

    weapon->pos.vx += weapon->vel.vx;
    weapon->pos.vy += weapon->vel.vy;
    weapon->pos.vz += weapon->vel.vz;

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, weapon->pos);

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = track->vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.vx += (((200 - alpha) * (int)facePtr->normal.vx) >> 13);
        weapon->pos.vy += (((200 - alpha) * (int)facePtr->normal.vy) >> 13);
        weapon->pos.vz += (((200 - alpha) * (int)facePtr->normal.vz) >> 13);
    }

    weapon->count--;

    if (weapon->count == NON_ACTIVE)
    {
        weaponShapes[weapon->currentWeapon].used = 0;
        weaponIndex[weapon->currentWeapon].availible = 0;
    }

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i == weapon->fireShip)
            continue;

        distance.vx = (shipIndex[i].pos.vx - weapon->pos.vx) >> 3;
        distance.vy = (shipIndex[i].pos.vy - weapon->pos.vy) >> 3;
        distance.vz = (shipIndex[i].pos.vz - weapon->pos.vz) >> 3;

        mag = GetMagnitude(&distance);

        if (mag < 100)
        {
            if ((shipIndex[i].attr & SHIELDED) == 0)
            {
                shipIndex[i].attr |= ELECTROED;
                shipIndex[i].electroCount = ELEC_DURATION;
            }
            weapon->count = SHIP_EXPLODE;
            weapon->targetShip = i;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;
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
            weapon->count = TRACK_EXPLODE;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;
            PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);
            return;
        }
        facePtr++;
    }
    Weapon2Mine(weapon, weaponIndex, weaponShapes);
}

void InitElecBolt(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    int i;

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
    for (i = 0; i < NO_OF_EBOLTS; i++)
    {
        if (weaponShapes[EBOLT_OFFSET + i].used == 0)
        {
            weapon->currentWeapon = EBOLT_OFFSET + i;
            weaponShapes[EBOLT_OFFSET + i].used = 1;
            weapon->targetShip = shipIndex[weapon->fireShip].targetShip;

            weapon->acc.vx = shipIndex[weapon->fireShip].unitVecNose.vx >> 8;
            weapon->acc.vy = shipIndex[weapon->fireShip].unitVecNose.vy >> 8;
            weapon->acc.vz = shipIndex[weapon->fireShip].unitVecNose.vz >> 8;

            weapon->acc.vx = (weapon->acc.vx * 60) / FR60;
            weapon->acc.vy = (weapon->acc.vy * 60) / FR60;
            weapon->acc.vz = (weapon->acc.vz * 60) / FR60;

            weapon->vel.vx = (shipIndex[weapon->fireShip].vel.vx >> 6);
            weapon->vel.vy = (shipIndex[weapon->fireShip].vel.vy >> 6);
            weapon->vel.vz = (shipIndex[weapon->fireShip].vel.vz >> 6);

            weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx;
            weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy;
            weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz;

            weapon->hdg = shipIndex[weapon->fireShip].hdg;
            weapon->pitch = shipIndex[weapon->fireShip].pitch;
            weapon->updateWeapon = UpdateElecBolt;
            weapon->availible = 1; // AJY
            return;
        }
    }
    weapon->count = NON_ACTIVE;
}

void InitSpecial(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    int i;
    weapon->count--;
    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i != weapon->fireShip)
        {
            shipIndex[i].combatAttr |= SPECIALED;
            shipIndex[i].specialCount = SPECIAL_DURATION;
        }
    }
    weapon->availible = 1; // AJY
}

void InitRevCon(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    int i;

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
    for (i = 0; i < NO_OF_EBOLTS; i++)
    {
        if (weaponShapes[EBOLT_OFFSET + i].used == 0)
        {
            weapon->currentWeapon = EBOLT_OFFSET + i;
            weaponShapes[EBOLT_OFFSET + i].used = 1;
            weapon->targetShip = shipIndex[weapon->fireShip].targetShip;

            weapon->acc.vx = shipIndex[weapon->fireShip].unitVecNose.vx >> 8;
            weapon->acc.vy = shipIndex[weapon->fireShip].unitVecNose.vy >> 8;
            weapon->acc.vz = shipIndex[weapon->fireShip].unitVecNose.vz >> 8;

            weapon->acc.vx = (weapon->acc.vx * 60) / FR60;
            weapon->acc.vy = (weapon->acc.vy * 60) / FR60;
            weapon->acc.vz = (weapon->acc.vz * 60) / FR60;

            weapon->vel.vx = (shipIndex[weapon->fireShip].vel.vx >> 6);
            weapon->vel.vy = (shipIndex[weapon->fireShip].vel.vy >> 6);
            weapon->vel.vz = (shipIndex[weapon->fireShip].vel.vz >> 6);

            weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx;
            weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy;
            weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz;

            weapon->hdg = shipIndex[weapon->fireShip].hdg;
            weapon->pitch = shipIndex[weapon->fireShip].pitch;
            weapon->updateWeapon = UpdateRevCon;
            weapon->availible = 1; // AJY
            return;
        }
    }
    weapon->count = NON_ACTIVE;
}

void UpdateRevCon(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    VECTOR distance;
    VECTOR targetVector;
    const VECTOR* facePoint;
    Face* facePtr;
    int i, mag, temp, vhdg, vpitch;
    int alpha;

    if (weapon->targetShip > -1)
    {
        targetVector.vx = (shipIndex[weapon->targetShip].pos.vx - weapon->pos.vx) >> 3;
        targetVector.vy = (shipIndex[weapon->targetShip].pos.vy - weapon->pos.vy) >> 3;
        targetVector.vz = (shipIndex[weapon->targetShip].pos.vz - weapon->pos.vz) >> 3;
        temp = SquareRoot0(targetVector.vx * targetVector.vx + targetVector.vz * targetVector.vz);

        vhdg = ((-ratan2(targetVector.vx, targetVector.vz)) - weapon->hdg);
        vpitch = ((-ratan2(targetVector.vy, temp)) - weapon->pitch);
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

    weapon->acc.vx = (-rsin(weapon->hdg) * rcos(weapon->pitch)) >> 16;
    weapon->acc.vy = -rsin(weapon->pitch) >> 4;
    weapon->acc.vz = (rcos(weapon->hdg) * rcos(weapon->pitch)) >> 16;

    weapon->acc.vx = (weapon->acc.vx * 60) / FR60;
    weapon->acc.vy = (weapon->acc.vy * 60) / FR60;
    weapon->acc.vz = (weapon->acc.vz * 60) / FR60;

    weapon->vel.vx += weapon->acc.vx;
    weapon->vel.vy += weapon->acc.vy;
    weapon->vel.vz += weapon->acc.vz;

    weapon->vel.vx -= weapon->vel.vx >> 2;
    weapon->vel.vy -= weapon->vel.vy >> 2;
    weapon->vel.vz -= weapon->vel.vz >> 2;

    weapon->pos.vx += weapon->vel.vx;
    weapon->pos.vy += weapon->vel.vy;
    weapon->pos.vz += weapon->vel.vz;

    weapon->nearTrkSect = FindNearestSection(weapon->nearTrkSect, weapon->pos);

    facePtr = track->faces + weapon->nearTrkSect->faceStart;

    while ((facePtr->flags & TRACK_BASE) == 0)
    {
        facePtr++;
    }

    facePoint = track->vertices + facePtr->vertex[0];

    alpha = PlaneLineCollision(facePoint, &weapon->pos, &facePtr->normal);

    if (alpha < 2000)
    {
        weapon->pos.vx += (((200 - alpha) * (int)facePtr->normal.vx) >> 13);
        weapon->pos.vy += (((200 - alpha) * (int)facePtr->normal.vy) >> 13);
        weapon->pos.vz += (((200 - alpha) * (int)facePtr->normal.vz) >> 13);
    }

    weapon->count--;

    if (weapon->count == NON_ACTIVE)
    {
        weaponShapes[weapon->currentWeapon].used = 0;
        weaponIndex[weapon->currentWeapon].availible = 0;
    }

    /***********************************************************/
    /****   Perform collision checks against other ships     ***/
    /***********************************************************/

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i == weapon->fireShip)
            continue;

        distance.vx = (shipIndex[i].pos.vx - weapon->pos.vx) >> 3;
        distance.vy = (shipIndex[i].pos.vy - weapon->pos.vy) >> 3;
        distance.vz = (shipIndex[i].pos.vz - weapon->pos.vz) >> 3;

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
            weapon->count = SHIP_EXPLODE;
            weapon->targetShip = i;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;
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
            weapon->count = TRACK_EXPLODE;
            weaponShapes[weapon->currentWeapon].used = 0;
            weaponIndex[weapon->currentWeapon].availible = 0;
            PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);
            return;
        }
        facePtr++;
    }
    Weapon2Mine(weapon, weaponIndex, weaponShapes);
}

void InitMine(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    int i;

    if (weapon->count == 1)
    {
        weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;

        for (i = 0; i < NO_OF_MINES; i++)
        {
            if (weaponShapes[MINE_OFFSET + i].used == 0)
            {
                weapon->currentWeapon = MINE_OFFSET + i;
                weaponShapes[MINE_OFFSET + i].used = 1;
                weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx;
                weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy;
                weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz;
                weapon->hdg = 0;
                if (weapon->fireShip == ownShip)
                {
                    PlayNote(SFX_MINE_DROP);
                }
                weapon->updateWeapon = UpdateMine;
                weapon->count = MINE_DURATION;
                weapon->availible = 1; // AJY

                return;
            }
        }
        weapon->count = NON_ACTIVE;
    }
    else if (weapon->count > 0)
        weapon->count--;
}

void InitShield(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    int i;

    weapon->nearTrkSect = shipIndex[weapon->fireShip].nearTrkSect;
    for (i = 0; i < NO_OF_SHIELDS; i++)
    {
        if (weaponShapes[SHIELD_OFFSET + i].used == 0)
        {
            weapon->currentWeapon = SHIELD_OFFSET + i;
            weaponShapes[SHIELD_OFFSET + i].used = 1;

            weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx;
            weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy;
            weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz;

            weapon->hdg = shipIndex[weapon->fireShip].hdg;
            weapon->pitch = shipIndex[weapon->fireShip].pitch;

            if (weapon->fireShip == ownShip)
            {
                PlayNote(SFX_SHIELDS);
                SetPitch(SFX_SHIELDS, SND_DEF_PITCH >> 1);
            }
            weapon->updateWeapon = UpdateShield;
            weapon->availible = 1; // AJY
            return;
        }
    }
    weapon->count = NON_ACTIVE;
}

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

void UpdateShield(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weapon, WeaponData* weaponIndex)
{
    if (shipIndex[weapon->fireShip].attr & VIEW_IN)
    {
        weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx + (shipIndex[weapon->fireShip].unitVecNose.vx >> 6);
        weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy + (shipIndex[weapon->fireShip].unitVecNose.vy >> 6);
        weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz + (shipIndex[weapon->fireShip].unitVecNose.vz >> 6);
    }
    else
    {
        weapon->pos.vx = shipIndex[weapon->fireShip].pos.vx;
        weapon->pos.vy = shipIndex[weapon->fireShip].pos.vy;
        weapon->pos.vz = shipIndex[weapon->fireShip].pos.vz;
    }

    weapon->count--;

    if (weapon->count == 0)
    {
        weaponShapes[weapon->currentWeapon].used = 0;
        weaponIndex[weapon->currentWeapon].availible = 0;

        if (weapon->fireShip == ownShip)
        {
            KillNote(SFX_SHIELDS);
        }

        shipIndex[weapon->fireShip].attr &= ~SHIELDED;
    }

    if ((shipIndex[weapon->fireShip].attr & VIEW_IN) == 0)
    {
        weapon->hdg = shipIndex[weapon->fireShip].hdg;
        weapon->pitch = shipIndex[weapon->fireShip].pitch;

#ifdef OLD_SHIELD
        UpdateShieldColors(weaponShapes[weapon->currentWeapon].shapePtr, weapon->count);
#endif
    }
}

void CycleWeaponGrids(ShipData* shipIndex, WeaponGrid* weaponGrid, int noWeaponGrids)
{
    int i, j;
    Face* facePtr;

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
                    if ((weaponGrid[i].section == shipIndex[j].nearTrkSect) && (shipIndex[j].weaponType == NO_WEAPON) && (weaponGrid[i].count >= 0))
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
                    if ((weaponGrid[i].section == shipIndex[j].nearTrkSect) && (shipIndex[j].weaponType == NO_WEAPON) && (weaponGrid[i].count >= 0))
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
            int r = (rsin(weaponGrid[i].cycleCount) >> 5) + 128;
            int g = (rcos(weaponGrid[i].cycleCount) >> 5) + 128;
            int b = (rsin(-weaponGrid[i].cycleCount) >> 5) + 128;

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
            facePtr->red = weaponGrid[i].colour.r;
            facePtr->green = weaponGrid[i].colour.g;
            facePtr->blue = weaponGrid[i].colour.b;
        }
        if (weaponGrid[i].attr & WEAPON_GRID_RIGHT)
        {
            facePtr++;
            facePtr->red = weaponGrid[i].colour.r;
            facePtr->green = weaponGrid[i].colour.g;
            facePtr->blue = weaponGrid[i].colour.b;
        }
    }
}

void GetFireVector(VECTOR* result, VECTOR target, Face* facePtr, VECTOR shipPos)
{
    int alpha, origAlpha;
    const VECTOR* facePoint;

    facePoint = track->vertices + facePtr->vertex[0];

    target.vx = shipPos.vx + (target.vx >> 6);
    target.vy = shipPos.vy + (target.vy >> 6);
    target.vz = shipPos.vz + (target.vz >> 6);

    alpha = PlaneLineCollision(facePoint, &target, &facePtr->normal);
    origAlpha = PlaneLineCollision(facePoint, &shipPos, &facePtr->normal);

    alpha = (alpha * 31) >> 5;

    alpha -= origAlpha;

    result->vx = target.vx - ((alpha * facePtr->normal.vx) >> 12);
    result->vy = target.vy - ((alpha * facePtr->normal.vy) >> 12);
    result->vz = target.vz - ((alpha * facePtr->normal.vz) >> 12);

    result->vx -= shipPos.vx;
    result->vy -= shipPos.vy;
    result->vz -= shipPos.vz;
}

void Weapon2Mine(WeaponData* weapon, WeaponData* weaponIndex, WeaponShape* weaponShapes)
{
    int i;
    VECTOR distance;
    int mag;

    for (i = 0; i < NO_ACTIVE_WEAPONS; i++)
    {
        if ((weaponIndex[i].type == MINE) && (weaponIndex[i].count > 0))
        {
            distance.vx = (weaponIndex[i].pos.vx - weapon->pos.vx) >> 2;
            distance.vy = (weaponIndex[i].pos.vy - weapon->pos.vy) >> 2;
            distance.vz = (weaponIndex[i].pos.vz - weapon->pos.vz) >> 2;

            mag = GetMagnitude(&distance);

            if (mag < 30)
            {
                weaponIndex->count = NON_ACTIVE;
                weaponShapes[weaponIndex->currentWeapon].used = 0;
                weapon->count = TRACK_EXPLODE;
                weaponShapes[weapon->currentWeapon].used = 0;
                PlayDirectionalNote(SFX_EXPLOSION_2, &weapon->pos);
                return;
            }
        }
    }
}

int GetNewWeapon(void)
{
    int index = randy() % 65;

    if (index < 17)
        return ROCKET;
    else if (index < 35)
        return MINE;
    else if (index < 45)
        return SHIELD;
    else if (index < 53)
        return MISSILE;
    else if (index < 59)
        return TURBO;
    else
        return ELEC_BOLT;
}

void SetTargetShip(ShipData* shipIndex, int shipNo)
{
    int i;
    int distance, shortestDistance;
    int nearShip = 0;
    TrackSection* section;

    shortestDistance = 999;

    for (i = NO_OF_SHIPS - 1; i >= 0; i--)
    {
        if (i == shipNo)
            continue;

        /*** Is Player ship upon a junction branch **/

        if (shipIndex[shipNo].nearTrkSect->flags & JunctionSection)
        {
            /*** If remote ship on same branch the attempt to target ***/
            if (shipIndex[i].nearTrkSect->flags & JunctionSection)
            {
                distance = shipIndex[i].nearTrkSect->secNo - shipIndex[shipNo].nearTrkSect->secNo;

                if ((distance < shortestDistance) && (distance > 0))
                {
                    shortestDistance = distance;
                    nearShip = i;
                }
            }
            else
            {
                /*** Remote ship is not same branch ***/
                section = shipIndex[shipNo].nearTrkSect;

                for (distance = 0; distance < 10; distance++)
                {
                    section = section->nextSection.ptr;
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
                section = shipIndex[shipNo].nearTrkSect;
                for (distance = 0; distance < 10; distance++)
                {
                    if (section->junction.index != -1)
                    {
                        section = section->junction.ptr;
                    }
                    else
                    {
                        section = section->nextSection.ptr;
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
                distance = shipIndex[i].nearTrkSect->secNo - shipIndex[shipNo].nearTrkSect->secNo;

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
        shipIndex[shipNo].targetShip = nearShip;
    }
    else
        shipIndex[shipNo].targetShip = 0xFF;
}
