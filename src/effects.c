#include "game.h"

Effect gEffects[MAX_EFFECTS];
Effect* gEffectsFree;
Effect* gEffectsActive;

Texture* gEffectTextures;
Texture* gShadowTex;

extern int gFrameIndex;
extern const TrackCamera ctrlcamera;
extern ShipData ctrlshipIndex[NO_OF_SHIPS];

void InitEffects(void)
{
    gEffectTextures = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/effects.cmp", 1);
}

void ResetEffects(void)
{
    int32 i;

    memset(gEffects, 0, sizeof(gEffects));

    for (i = 0; i < MAX_EFFECTS - 1; i++)
    {
        gEffects[i].next = gEffects + i + 1;
    }
    gEffects[MAX_EFFECTS - 1].next = NULL;

    gEffectsFree = gEffects;
    gEffectsActive = NULL;
}

Effect* CreateEffect()
{
    Effect* fx = gEffectsFree;
    if (fx)
    {
        gEffectsFree = fx->next;

        fx->next = gEffectsActive;
        fx->prev = NULL;
        if (gEffectsActive)
        {
            gEffectsActive->prev = fx;
        }
        gEffectsActive = fx;
    }
    return fx;
}

void RemoveEffect(Effect* fx)
{
    if (fx->next)
    {
        fx->next->prev = fx->prev;
    }

    if (fx->prev)
    {
        fx->prev->next = fx->next;
    }
    else
    {
        gEffectsActive = fx->next;
    }

    fx->next = gEffectsFree;
    gEffectsFree = fx;
}

void UpdateEffects(void)
{
    int i;
    Effect* fx = gEffectsActive;

    while (fx)
    {
        Effect* next = fx->next;

        if (fx->count < -6)
        {
            fx->count++;
        }
        else if (fx->count == -6)
        {
            fx->count = (uint8)randy() >> 3;
        }
        else
        {
            fx->pos.x += fx->vel.x;
            fx->pos.y += fx->vel.y;
            fx->pos.z += fx->vel.z;
            fx->count--;
        }

        if (fx->count == 0)
        {
            RemoveEffect(fx);
        }

        fx = next;
    }
}

void DrawEffects(void)
{
#ifndef __3DO__ // TODO
    int i;
    Effect* fx = gEffectsActive;
    Billboard b;
    int cx, cy, cz;

    cx = ctrlcamera.camPos->absolute.t[0];
    cy = ctrlcamera.camPos->absolute.t[1];
    cz = ctrlcamera.camPos->absolute.t[2];

    renderSetSkeleton(ctrlcamera.camPos->super, cx, cy, cz);

    renderDepthWrite(0);
    renderSetBlend(BLEND_ADD);

    while (fx)
    {
        b.x = fx->pos.x;
        b.y = fx->pos.y;
        b.z = fx->pos.z;
        b.size = fx->size;
        b.texture = gEffectTextures + fx->type;
        renderBillboard(&b);

        fx = fx->next;
    }

    renderDepthWrite(1);
    renderSetBlend(BLEND_NONE);
#endif
}

void AddTrail(WeaponData* weapon)
{
    Effect* fx;
    int32 fxType;

    if (weapon->type == WPN_ELEC_BOLT)
        fxType = FX_EBOLT;
    else if (weapon->type == WPN_REV_CON)
        fxType = FX_GREENY;
    else
        fxType = FX_SMOKE;

    if (fx = CreateEffect())
    {
        fx->pos.x = weapon->pos.x;
        fx->pos.y = weapon->pos.y;
        fx->pos.z = weapon->pos.z;
        fx->vel.x = (int8)randy() >> 4;
        fx->vel.y = (int8)randy() >> 4;
        fx->vel.z = (int8)randy() >> 4;
        fx->count = 20 + (randy() & 7);
        fx->size = 7;
        fx->type = fxType;
    }

    if (fx = CreateEffect())
    {
        fx->pos.x = weapon->pos.x - (weapon->vel.x >> 2);
        fx->pos.y = weapon->pos.y - (weapon->vel.y >> 2);
        fx->pos.z = weapon->pos.z - (weapon->vel.z >> 2);
        fx->vel.x = (int8)randy() >> 4;
        fx->vel.y = (int8)randy() >> 4;
        fx->vel.z = (int8)randy() >> 4;
        fx->count = 20 + (randy() & 7);
        fx->size = 7;
        fx->type = fxType;
    }

    if (fx = CreateEffect())
    {
        fx->pos.x = weapon->pos.x - (weapon->vel.x >> 1);
        fx->pos.y = weapon->pos.y - (weapon->vel.y >> 1);
        fx->pos.z = weapon->pos.z - (weapon->vel.z >> 1);
        fx->vel.x = 0;
        fx->vel.y = 0;
        fx->vel.z = 0;
        fx->count = 5;
        fx->size = 7;
        fx->type = (fxType == FX_SMOKE) ? FX_FIRE : fxType;
    }
}

void AddExplosion(WeaponData* weapon, int32 targetShip)
{
    int32 i, fxType;
    Effect* fx;

    if (targetShip >= 0)
    {
        if (weapon->type == WPN_ELEC_BOLT)
            fxType = FX_GREENY;
        else if (weapon->type == WPN_REV_CON)
            fxType = FX_GREENY;
        else
            fxType = FX_FIRE_WHITE;
    }
    else
    {
        if (weapon->type == WPN_ELEC_BOLT)
            fxType = FX_EBOLT;
        else if (weapon->type == WPN_REV_CON)
            fxType = FX_GREENY;
        else
            fxType = FX_FIRE;
    }

    for (i = 0; i < WEAPON_FX_EXPLOSION_SPRITES; i++)
    {
        if (fx = CreateEffect())
        {
            fx->pos.x = weapon->pos.x;
            fx->pos.y = weapon->pos.y;
            fx->pos.z = weapon->pos.z;

            fx->vel.x = (int8)randy() >> 2;
            fx->vel.y = (int8)randy() >> 2;
            fx->vel.z = (int8)randy() >> 2;

            if (targetShip >= 0)
            {
                ShipData* ship = ctrlshipIndex + targetShip;
                fx->vel.x += (int16)(ship->vel.x >> VELOCITY_SHIFT);
                fx->vel.y += (int16)(ship->vel.y >> VELOCITY_SHIFT);
                fx->vel.z += (int16)(ship->vel.z >> VELOCITY_SHIFT);
            }

            fx->count = -((((uint8)randy()) >> 4) + 8);
            fx->size = 24;
            fx->type = fxType;
        }
        else
        {
            break;
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

void InitShadow(void)
{
    const char* name;

    if (ownShip < 2)
        name = "wipeout/textures/shad1.tim";
    else if (ownShip < 4)
        name = "wipeout/textures/shad2.tim";
    else if (ownShip < 6)
        name = "wipeout/textures/shad3.tim";
    else
        name = "wipeout/textures/shad4.tim";

    HACK_LOAD_SHADOW = 1;
    gShadowTex = gTexturePtr;
    LoadTexture(name);
    HACK_LOAD_SHADOW = 0;
}

void DrawShadows(ShipData* shipIndex)
{
#ifndef __3DO__ // TODO
    const Face* facePtr;
    Shadow s;
    int i, j, px, py, pz, cx, cy, cz, nx, nz, wx, wz;
    int alpha, intensity;
    const vec3i* facePoint;
    ShipData* playerShip;
    vec3i v[4];

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

        px = playerShip->pos.x;
        py = playerShip->pos.y;
        pz = playerShip->pos.z;

        nx = playerShip->unitVecNose.x >> SHADOW_SHIFT_N;
        nz = playerShip->unitVecNose.z >> SHADOW_SHIFT_N;

        wx = playerShip->unitVecWing.x >> SHADOW_SHIFT_W;
        wz = playerShip->unitVecWing.z >> SHADOW_SHIFT_W;

        v[0].x = px + nx - wx;
        v[0].y = py;
        v[0].z = pz + nz - wz;

        v[1].x = px + nx + wx;
        v[1].y = py;
        v[1].z = pz + nz + wz;

        v[2].x = px - nx + wx;
        v[2].y = py;
        v[2].z = pz - nz + wz;

        v[3].x = px - nx - wx;
        v[3].y = py;
        v[3].z = pz - nz - wz;

        intensity = 0;

        /*TODO optimize, single raycast + normal*/
        for (j = 0; j < 4; j++)
        {
            alpha = PlaneLineCollision(facePoint, v + j, &facePtr->normal);
            intensity += alpha;

            alpha -= SHADOW_BIAS;

            v[j].x -= (alpha * facePtr->normal.x) >> 12;
            v[j].y -= (alpha * facePtr->normal.y) >> 12;
            v[j].z -= (alpha * facePtr->normal.z) >> 12;
        }

        s.x0 = v[0].x;
        s.y0 = v[0].y;
        s.z0 = v[0].z;

        s.x1 = v[1].x;
        s.y1 = v[1].y;
        s.z1 = v[1].z;

        s.x2 = v[2].x;
        s.y2 = v[2].y;
        s.z2 = v[2].z;

        s.x3 = v[3].x;
        s.y3 = v[3].y;
        s.z3 = v[3].z;

        s.texture = gShadowTex;

        intensity = 255 - (intensity >> 3);
        if (intensity < 0)
            intensity = 0;
        if (intensity > 255)
            intensity = 255;

        s.intensity = intensity;

        renderShadow(&s);
    }

    renderSetBlend(BLEND_NONE);
#endif
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

/*TODO optimize this shit!*/
#define MAX_AURORA_PRIMS 80

GT4* abPtr[MAX_AURORA_PRIMS];
short* abCoords[MAX_AURORA_PRIMS];
signed short greyCoords[MAX_AURORA_PRIMS];
int abPhase;

void InitAuroraBorialis(Object* object)
{
    int i;
    Prm poly;
    short primitiveCount;
    short* coords;
    int count = 0;
    short y;
    vec4s* vertex;

    poly.primitive = object->primitives;
    primitiveCount = object->primitiveCount;
    vertex = object->vertices;

    for (i = 0; i < primitiveCount; i++)
    {
        switch (poly.primitive->type)
        {
            case TypeGT3:
                poly.gt3 += 1;
                break;

            case TypeGT4:
                coords = poly.gt4->coords;
                y = vertex[coords[0]].y;
                if (y < -6000) /* -8000 */
                {
                    ASSERT(count < MAX_AURORA_PRIMS);
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
                    {
                        abCoords[count] = poly.gt4->coords;
                        greyCoords[count] = 0;
                    }
                    count++;
                }
                poly.gt4 += 1;
                break;
        }
    }
}

void AuroraBorialis(Object* object)
{
#ifndef __3DO__ // TODO
    int i;

    Prm poly;
    short* coords;

    abPhase++;

    poly.primitive = object->primitives;

    for (i = 0; i < MAX_AURORA_PRIMS; i++)
    {
        coords = abCoords[i];

        if (greyCoords[i] != -2)
        {
            abPtr[i]->colour[0].r = (rsin(coords[0] * abPhase) >> 6) + 190;
            abPtr[i]->colour[0].g = (rsin(coords[0] * (abPhase + 54)) >> 6) + 190;
            abPtr[i]->colour[0].b = (rsin(coords[0] * (abPhase + 39)) >> 6) + 190;

            abPtr[i]->colour[1].r = (rsin(coords[1] * abPhase) >> 6) + 190;
            abPtr[i]->colour[1].g = (rsin(coords[1] * (abPhase + 54)) >> 6) + 190;
            abPtr[i]->colour[1].b = (rsin(coords[1] * (abPhase + 39)) >> 6) + 190;
        }

        if (greyCoords[i] != -1)
        {
            abPtr[i]->colour[2].r = (rsin(coords[2] * abPhase) >> 6) + 190;
            abPtr[i]->colour[2].g = (rsin(coords[2] * (abPhase + 54)) >> 6) + 190;
            abPtr[i]->colour[2].b = (rsin(coords[2] * (abPhase + 39)) >> 6) + 190;

            abPtr[i]->colour[3].r = (rsin(coords[3] * abPhase) >> 6) + 190;
            abPtr[i]->colour[3].g = (rsin(coords[3] * (abPhase + 54)) >> 6) + 190;
            abPtr[i]->colour[3].b = (rsin(coords[3] * (abPhase + 39)) >> 6) + 190;
        }
    }
#endif
}

