#include "game.h"

Effect gEffects[NO_OF_EFFECTS];
Texture* gEffectTextures[NO_OF_EFFECT_TEXTURES];

extern int gFrameIndex;
extern const TrackCamera ctrlcamera;

const char tranny_table[] = {
    YELLOW_WHITE_TRANSL,    // MISSILE/ROCKET EXPLODE HIT NOTHING
    WHITE2_TRANSL,          // MISSILE/ROCKET TRAIL
    YELLOW_TRANSL,          // MISSILE/ROCKEY EXPLODE HIT SHIP
    WHITE8_TRANSL,          // FX_SHADOW NOT USED
    BLUE_TRANSL,            // FX_EBOLT TRAIL
    WHITE8_TRANSL,          // FX_HALO NOT USED
    PURPLE_TRANSL };        // FX_GREENY (NOT USED?) WAS REV_CON SO USE FOR FX_EBOLT EXPLODE

void InitEffects(Skeleton* camPos)
{
    int tablePos;

    tablePos = TextureTableCount;

    LoadCompressedTextureSequence("wipeout\\common\\effects.cmp");
    gEffectTextures[FX_FIRE] = TextureTable[tablePos++];
    gEffectTextures[FX_FIRE_WHITE] = TextureTable[tablePos++];
    gEffectTextures[FX_SMOKE] = TextureTable[tablePos++];
    gEffectTextures[FX_EBOLT] = TextureTable[tablePos++];
    gEffectTextures[FX_HALO] = TextureTable[tablePos++];
    gEffectTextures[FX_GREENY] = TextureTable[tablePos++];

    ResetEffects();
}

void ResetEffects(void)
{
    memset(gEffects, 0, sizeof(gEffects));
}

void UpdateEffects()
{
    int i;

    for (i = 0; i < NO_OF_EFFECTS; i++)
    {
        Effect* fx = gEffects + i;

        if (fx->count > 0)
        {
            fx->x += fx->vx;
            fx->y += fx->vy;
            fx->z += fx->vz;

            fx->count--;
        }
        else if (fx->count < -6)
        {
            fx->count++;
        }
        else if (fx->count == -6)
        {
            fx->count = (uint8)randy() >> 3;
        }
    }
}

void DrawEffects()
{
    int i;
    Billboard b;
    int cx, cy, cz;

    cx = ctrlcamera.camPos->absolute.t[0];
    cy = ctrlcamera.camPos->absolute.t[1];
    cz = ctrlcamera.camPos->absolute.t[2];

    renderSetSkeleton(ctrlcamera.camPos->super, cx, cy, cz);

    renderDepthWrite(0);
    renderSetBlend(BLEND_ADD);

    for (i = 0; i < NO_OF_EFFECTS; i++)
    {
        Effect* fx = gEffects + i;

        if (fx->count > 0)
        {
            b.x = fx->x;
            b.y = fx->y;
            b.z = fx->z;
            b.size = fx->size;
            b.texture = gEffectTextures[fx->type];
            renderBillboard(&b);
        }
    }

    renderDepthWrite(1);
    renderSetBlend(BLEND_NONE);
}

void AddEffects(WeaponData* weaponIndex, ShipData* shipIndex)
{
    int i, j;
    int exploType;
    short noToRelease = 0;

    for (i = 0; i < NO_ACTIVE_WEAPONS; i++)
    {
        WeaponData* weapon = weaponIndex + i;

        /***********************************************************************/
        /****          Add Fire + Smoke Trails for Missiles & Rockets       ****/
        /***********************************************************************/
        if ((weapon->type == ROCKET) ||
            (weapon->type == MISSILE) ||
            (weapon->type == ELEC_BOLT) ||
            (weapon->type == REV_CON))
        {
            if (weapon->type == ELEC_BOLT)
                exploType = FX_EBOLT;
            else if (weapon->type == REV_CON)
                exploType = FX_GREENY;
            else
                exploType = FX_SMOKE;

            if (weapon->count > 0)
            {
                for (j = 0; j < NO_OF_EFFECTS; j++)
                {
                    Effect* fx = gEffects + j;

                    if (fx->count == 0)
                    {
                        if (noToRelease == 0)
                        {
                            fx->x = weapon->pos.vx;
                            fx->y = weapon->pos.vy;
                            fx->z = weapon->pos.vz;
                            fx->vx = (int8)randy() >> 4;
                            fx->vy = (int8)randy() >> 4;
                            fx->vz = (int8)randy() >> 4;
                            fx->count = 20 + (randy() % 10);
                            fx->size = 7;
                            fx->type = exploType;
                        }
                        else if (noToRelease == 1)
                        {
                            fx->x = weapon->pos.vx - (weapon->vel.vx >> 1);
                            fx->y = weapon->pos.vy - (weapon->vel.vy >> 1);
                            fx->z = weapon->pos.vz - (weapon->vel.vz >> 1);
                            fx->vx = 0;
                            fx->vy = 0;
                            fx->vz = 0;
                            fx->count = 5;
                            fx->size = 7;
                            fx->type = (exploType == FX_SMOKE) ? FX_FIRE : exploType;
                        }
                        else if (noToRelease == 2)
                        {
                            fx->x = weapon->pos.vx - (weapon->vel.vx >> 2);
                            fx->y = weapon->pos.vy - (weapon->vel.vy >> 2);
                            fx->z = weapon->pos.vz - (weapon->vel.vz >> 2);
                            fx->vx = (int8)randy() >> 4;
                            fx->vy = (int8)randy() >> 4;
                            fx->vz = (int8)randy() >> 4;
                            fx->count = 20 + (randy() % 10);
                            fx->size = 7;
                            fx->type = exploType;
                        }
                        noToRelease++;
                        if (noToRelease == 3)
                            break;
                    }
                }
            }
        }

        noToRelease = 0;

        if ((weapon->count == SHIP_EXPLODE) ||
            (weapon->count == TRACK_EXPLODE))
        {
            if (weapon->count == SHIP_EXPLODE)
            {
                if (weapon->type == ELEC_BOLT)
                    exploType = FX_GREENY;
                else if (weapon->type == REV_CON)
                    exploType = FX_GREENY;
                else
                    exploType = FX_FIRE_WHITE;
            }
            else
            {
                if (weapon->type == ELEC_BOLT)
                    exploType = FX_EBOLT;
                else if (weapon->type == REV_CON)
                    exploType = FX_GREENY;
                else
                    exploType = FX_FIRE;
            }

            for (j = 0; j < NO_OF_EFFECTS; j++)
            {
                Effect* fx = gEffects + j;

                if (fx->count == 0)
                {
                    fx->x = weapon->pos.vx;
                    fx->y = weapon->pos.vy;
                    fx->z = weapon->pos.vz;

                    fx->vx = (int8)randy() >> 2;
                    fx->vy = (int8)randy() >> 2;
                    fx->vz = (int8)randy() >> 2;

                    if (weapon->count == SHIP_EXPLODE)
                    {
                        fx->vx += (int16)(shipIndex[weapon->targetShip].vel.vx >> 6);
                        fx->vy += (int16)(shipIndex[weapon->targetShip].vel.vy >> 6);
                        fx->vz += (int16)(shipIndex[weapon->targetShip].vel.vz >> 6);
                    }

                    fx->count = -((((uint8)randy()) >> 4) + 8);
                    fx->size = 24;
                    fx->type = exploType;

                    noToRelease++;
                    if (noToRelease == 30)
                        return;
                }
            }
            weapon->count = NON_ACTIVE;
        }
    }
}

const uint8 gStartBoomColors[4][3] = {
    { 0x1C, 0x22, 0x2F }, // grey
    { 0xFF, 0x00, 0x00 }, // red
    { 0xFF, 0x80, 0x00 }, // orange
    { 0x00, 0xFF, 0x00 }  // green
};

void ColourStartBoom(int noToLight)
{
    int i, j;
    Prm prm;

    for (i = 0; i < startBoomCount; i++)
    {
        prm.primitive = startBoom[i]->primitives;

        for (j = 1; j < noToLight; j++)
        {
            prm.gt4 += 1;
        }

        prm.gt4->colour[0].r = prm.gt4->colour[1].r = prm.gt4->colour[2].r = prm.gt4->colour[3].r = gStartBoomColors[noToLight][0];
        prm.gt4->colour[0].g = prm.gt4->colour[1].g = prm.gt4->colour[2].g = prm.gt4->colour[3].g = gStartBoomColors[noToLight][1];
        prm.gt4->colour[0].b = prm.gt4->colour[1].b = prm.gt4->colour[2].b = prm.gt4->colour[3].b = gStartBoomColors[noToLight][2];
    }
}

void ResetStartBoom(void)
{
    int i, j;
    Prm prm;

    for (i = 0; i < startBoomCount; i++)
    {
        prm.primitive = startBoom[i]->primitives;

        for (j = 0; j < 3; j++)
        {
            prm.gt4->colour[0].r = prm.gt4->colour[1].r = prm.gt4->colour[2].r = prm.gt4->colour[3].r = gStartBoomColors[0][0];
            prm.gt4->colour[0].g = prm.gt4->colour[1].g = prm.gt4->colour[2].g = prm.gt4->colour[3].g = gStartBoomColors[0][1];
            prm.gt4->colour[0].b = prm.gt4->colour[1].b = prm.gt4->colour[2].b = prm.gt4->colour[3].b = gStartBoomColors[0][2];

            prm.gt4++;
        }
    }
}

void InitStartBoom(Object** prm, short prmCount)
{
    int i;
    Object* obj;

    startBoomCount = 0;

    for (i = 0; i < prmCount; i++)
    {
        obj = prm[i];
        while (obj)
        {
            if (!strncmp(obj->name, "start", 5))
            {
                startBoom[startBoomCount] = obj;
                startBoomCount++;
            }

            if (startBoomCount > 4)
            {
                ASSERT(0);
            }

            obj = obj->next;
        }
    }
}

void InitRedLight(Object** prm, short prmCount, Object** redLight)
{
    int i;
    Object* obj;

    for (i = 0; i < prmCount; i++)
    {
        obj = prm[i];
        while (obj)
        {
            if (!strncmp(obj->name, "red", 3))
            {
                *redLight = obj;
            }
            obj = obj->next;
        }
    }
}

void ColourRedLight(Object* redLight)
{
    short r;
    short count;

    Prm prm;

    count = gFrameIndex * 140;
    r = (rsin(count) >> 5) + 128;
    if (r > 255)
        r = 255;

    prm.primitive = redLight->primitives;

    prm.gt4->colour[0].r = prm.gt4->colour[1].r = prm.gt4->colour[2].r = prm.gt4->colour[3].r = (uint8)r;
    prm.gt4->colour[0].g = prm.gt4->colour[1].g = prm.gt4->colour[2].g = prm.gt4->colour[3].g = 0x00;
    prm.gt4->colour[0].b = prm.gt4->colour[1].b = prm.gt4->colour[2].b = prm.gt4->colour[3].b = 0x00;
}

void ColourDroidLights(Object* droid)
{
    short r, g, b;
    short rf, gf, bf;
    int i;
    Prm prm;
    short primitiveCount;
    short count = gFrameIndex * 70;

    rf = (rsin(count) >> 5) + 128;
    if (rf > 255)
        rf = 255;

    gf = (rsin(count + 75) >> 5) + 128;
    if (gf > 255)
        gf = 255;

    bf = (rsin((count >> 1) + 32) >> 5) + 128;
    if (bf > 255)
        bf = 255;

    prm.primitive = droid->primitives;

    for (i = 0; i < 11; i++)
    {
        if (i < 2)
        {
            r = 40;
            g = gf;
            b = 40;
        }
        else if (i < 6)
        {
            r = bf >> 1;
            b = bf;
            g = bf >> 1;
        }
        else
        {
            r = rf;
            b = 40;
            g = 40;
        }

        switch (prm.primitive->type)
        {
            case TypeGT3:
                prm.gt3->colour[0].r = prm.gt3->colour[1].r = prm.gt3->colour[2].r = r;
                prm.gt3->colour[0].g = prm.gt3->colour[1].g = prm.gt3->colour[2].g = g;
                prm.gt3->colour[0].b = prm.gt3->colour[1].b = prm.gt3->colour[2].b = b;
                prm.gt3++;
                break;

            case TypeGT4:
                prm.gt4->colour[0].r = prm.gt4->colour[1].r = prm.gt4->colour[2].r = prm.gt4->colour[3].r = r;
                prm.gt4->colour[0].g = prm.gt4->colour[1].g = prm.gt4->colour[2].g = prm.gt4->colour[3].g = g;
                prm.gt4->colour[0].b = prm.gt4->colour[1].b = prm.gt4->colour[2].b = prm.gt4->colour[3].b = b;
                prm.gt4++;
                break;
        }
    }
}

void InitDonkey(Object** prm, short prmCount, Object** donkey)
{
    int i;
    Object* obj;
    int count = 0;

    for (i = 0; i < prmCount; i++)
    {
        obj = prm[i];
        while (obj)
        {
            if (!strncmp(obj->name, "donkey", 6))
            {
                donkey[count] = obj;
                count++;
            }
            obj = obj->next;
        }
    }
}

void MoveDonkey(Object** donkey)
{
    int i;
    Object* obj;
    short count;

    count = gFrameIndex * 20;

    for (i = 0; i < 2; i++)
    {
        obj = donkey[i];

        SetSkeletonDirection(obj->skeleton, rsin(count) >> 3, 0, 0);
    }
}

void InitShadow(Skeleton* camPos)
{
    HACK_LOAD_SHADOW = 1;

    if (ownShip < 2)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad1.tim");
    else if (ownShip < 4)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad2.tim");
    else if (ownShip < 6)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad3.tim");
    else if (ownShip < 8)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad4.tim");

    HACK_LOAD_SHADOW = 0;
}

void DrawShadows(ShipData* shipIndex)
{
    const Face* facePtr;
    Shadow s;
    int i, j, px, py, pz, cx, cy, cz, nx, nz, wx, wz;
    int alpha, intensity;
    const VECTOR* facePoint;
    ShipData* playerShip;
    VECTOR v[4];

    cx = ctrlcamera.camPos->absolute.t[0];
    cy = ctrlcamera.camPos->absolute.t[1];
    cz = ctrlcamera.camPos->absolute.t[2];

    renderSetBlend(BLEND_SUB);
    renderSetSkeleton(ctrlcamera.camPos->super, cx, cy, cz);

    #define SHADOW_SHIFT_N  3
    #define SHADOW_SHIFT_W  4
    #define SHADOW_BIAS     32

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        playerShip = shipIndex + i;

        if (!((playerShip->attr & VISIBLE) && ((playerShip->attr & FLYING) == 0)))
            continue;

        facePtr = track->faces + playerShip->nearTrkSect->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        facePoint = track->vertices + facePtr->vertex[0];

        px = playerShip->pos.vx;
        py = playerShip->pos.vy;
        pz = playerShip->pos.vz;

        nx = playerShip->unitVecNose.vx >> SHADOW_SHIFT_N;
        nz = playerShip->unitVecNose.vz >> SHADOW_SHIFT_N;

        wx = playerShip->unitVecWing.vx >> SHADOW_SHIFT_W;
        wz = playerShip->unitVecWing.vz >> SHADOW_SHIFT_W;

        v[0].vx = px + nx - wx;
        v[0].vy = py;
        v[0].vz = pz + nz - wz;

        v[1].vx = px + nx + wx;
        v[1].vy = py;
        v[1].vz = pz + nz + wz;

        v[2].vx = px - nx + wx;
        v[2].vy = py;
        v[2].vz = pz - nz + wz;

        v[3].vx = px - nx - wx;
        v[3].vy = py;
        v[3].vz = pz - nz - wz;

        intensity = 0;

        /*TODO optimize, single raycast + normal*/
        for (j = 0; j < 4; j++)
        {
            alpha = PlaneLineCollision(facePoint, v + j, &facePtr->normal);
            intensity += alpha;

            alpha -= SHADOW_BIAS;

            v[j].vx -= (alpha * facePtr->normal.vx) >> 12;
            v[j].vy -= (alpha * facePtr->normal.vy) >> 12;
            v[j].vz -= (alpha * facePtr->normal.vz) >> 12;
        }

        s.x0 = v[0].vx;
        s.y0 = v[0].vy;
        s.z0 = v[0].vz;

        s.x1 = v[1].vx;
        s.y1 = v[1].vy;
        s.z1 = v[1].vz;

        s.x2 = v[2].vx;
        s.y2 = v[2].vy;
        s.z2 = v[2].vz;

        s.x3 = v[3].vx;
        s.y3 = v[3].vy;
        s.z3 = v[3].vz;

        s.texture = gEffectTextures[FX_SHADOW];

        intensity = 255 - (intensity >> 3);
        if (intensity < 0)
            intensity = 0;
        if (intensity > 255)
            intensity = 255;

        s.intensity = intensity;

        renderShadow(&s);
    }

    renderSetBlend(BLEND_NONE);
}

int shakeCount = 0;

void SetShake(int duration)
{
    shakeCount = duration;
}

void ShakeScreen(void)
{
    if (shakeCount == 0)
        return;

    if (shakeCount > 1)
    {
        renderOffset(-(randy() % shakeCount) + (shakeCount >> 1), -(randy() % shakeCount) + (shakeCount >> 1));
        shakeCount--;
    }
    else if (shakeCount == 1)
    {
        renderOffset(0, 0);
        shakeCount--;
    }

    osJoyVibrate(0, shakeCount, shakeCount);
}

GT4* abPtr[80];
short* abCoords[80];
signed short greyCoords[80];

#if 1 // TODO WTF is Aurora?
void InitAuroraBorialis(Object* object)
{
    int i;
    Prm poly;
    short primitiveCount;
    short* coords;
    int count = 0;
    short y;
    SVECTOR* vertex;

    poly.primitive = object->primitives;
    primitiveCount = object->primitiveCount;
    vertex = object->vertices;

    for (i = 0; i < 80; i++)
    {
        greyCoords[i] = 0;
    }

    for (i = 0; i < primitiveCount; i++)
    {
        switch (poly.primitive->type)
        {
            case TypeGT3:
                poly.gt3 += 1;
                break;

            case TypeGT4:

                coords = poly.gt4->coords;
                y = vertex[coords[0]].vy;
                if (y < -6000) /* -8000 */
                {
                    abPtr[count] = poly.gt4;
                    if (y > -6800)
                    {
                        abCoords[count] = poly.gt4->coords;
                        greyCoords[count] = -1;
                    }
                    else if (y < -11000)
                    {
                        abCoords[count] = poly.gt4->coords;
                        greyCoords[count] = -2;
                    }
                    else
                        abCoords[count] = poly.gt4->coords;
                    count++;
                }
                poly.gt4 += 1;
                break;
        }
    }
}

void AuroraBorialis(Object* object)
{
    int i;

    Prm poly;
    short* coords;
    static int count = 0;

    count++;

    poly.primitive = object->primitives;

    for (i = 0; i < 80; i++)
    {
        coords = abCoords[i];

        if (greyCoords[i] != -2)
        {
            abPtr[i]->colour[0].r = (rsin(coords[0] * count) >> 6) + 190;
            abPtr[i]->colour[0].g = (rsin(coords[0] * (count + 54)) >> 6) + 190;
            abPtr[i]->colour[0].b = (rsin(coords[0] * (count + 39)) >> 6) + 190;

            abPtr[i]->colour[1].r = (rsin(coords[1] * count) >> 6) + 190;
            abPtr[i]->colour[1].g = (rsin(coords[1] * (count + 54)) >> 6) + 190;
            abPtr[i]->colour[1].b = (rsin(coords[1] * (count + 39)) >> 6) + 190;
        }

        if (greyCoords[i] != -1)
        {
            abPtr[i]->colour[2].r = (rsin(coords[2] * count) >> 6) + 190;
            abPtr[i]->colour[2].g = (rsin(coords[2] * (count + 54)) >> 6) + 190;
            abPtr[i]->colour[2].b = (rsin(coords[2] * (count + 39)) >> 6) + 190;

            abPtr[i]->colour[3].r = (rsin(coords[3] * count) >> 6) + 190;
            abPtr[i]->colour[3].g = (rsin(coords[3] * (count + 54)) >> 6) + 190;
            abPtr[i]->colour[3].b = (rsin(coords[3] * (count + 39)) >> 6) + 190;
        }
    }
}
#endif
