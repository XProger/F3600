#include "game.h"

Effect gEffects[MAX_EFFECTS];
Effect* gEffectsFree;
Effect* gEffectsActive;

Texture* gShadowTex;

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
    matrixTranslate(gCameraViewPos.x, gCameraViewPos.y, gCameraViewPos.z);
    renderEffects(gEffectsActive);
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

    if ((fx = CreateEffect()) != NULL)
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

    if ((fx = CreateEffect()) != NULL)
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

    if ((fx = CreateEffect()) != NULL)
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
        if ((fx = CreateEffect()) != NULL)
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
                fx->vel.x += ship->vel.x >> VELOCITY_SHIFT;
                fx->vel.y += ship->vel.y >> VELOCITY_SHIFT;
                fx->vel.z += ship->vel.z >> VELOCITY_SHIFT;
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

const uint32 gStartBoomColors[4] = {
    0xFF2F221C, // grey
    0xFF0000FF, // red
    0xFF0080FF, // orange
    0xFF00FF00  // green
};

void ColourStartBoom(int noToLight)
{
#ifdef USE_VERTEX_COLOR
    int32 i;
    ObjFace* face;

    for (i = 0; i < gTrack.startCount; i++)
    {
        face = gTrack.start[i].faces + noToLight - 1;

        face->color[0] =
        face->color[1] =
        face->color[2] =
        face->color[3] = gStartBoomColors[noToLight];
    }
#endif
}

void ResetStartBoom(void)
{
#ifdef USE_VERTEX_COLOR
    int32 i, j;
    ObjFace* face;

    for (i = 0; i < gTrack.startCount; i++)
    {
        face = gTrack.start[i].faces;

        for (j = 0; j < 3; j++, face++)
        {
            face->color[0] =
            face->color[1] =
            face->color[2] =
            face->color[3] = gStartBoomColors[0];
        }
    }
#endif
}

void ColourRedLight(void)
{
#ifdef USE_VERTEX_COLOR
    int32 i;
    uint32 r;
    ObjFace* face;

    int32 count = FRAME_LERP_STEP(280);

    r = (rsin(count) >> 5) + 128;
    if (r > 255)
        r = 255;
    r |= 0xFF000000;

    for (i = 0; i < gTrack.redCount; i++)
    {
        face = gTrack.red[i].faces;

        face->color[0] =
        face->color[1] =
        face->color[2] =
        face->color[3] = r;
    }
#endif
}

void ColourDroidLights(Object* droid)
{
#ifdef USE_VERTEX_COLOR
    int32 i;
    int32 r, g, b;
    int32 rf, gf, bf;
    ObjFace* face;

    int32 count = FRAME_LERP_STEP(140);

    rf = (rsin(count) >> 5) + 128;
    if (rf > 255)
        rf = 255;

    gf = (rsin(count + 75) >> 5) + 128;
    if (gf > 255)
        gf = 255;

    bf = (rsin((count >> 1) + 32) >> 5) + 128;
    if (bf > 255)
        bf = 255;

    face = droid->faces;
    for (i = 0; i < 11; i++, face++)
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

        face->color[0] =
        face->color[1] =
        face->color[2] =
        face->color[3] = r | (g << 8) | (b << 16) | 0xFF000000;
    }
#endif
}

void MoveDonkey(void)
{
#if 0 // TODO donkey animation
    int32 i;
    Object* obj;

    int32 count = FRAME_LERP_STEP(40);

    for (i = 0; i < 2; i++)
    {
        obj = donkey[i];

        SetSkeletonDirection(obj->skeleton, rsin(count) >> 3, 0, 0);
    }
#endif
}

#define SHADOW_BIAS     48

void DrawShadows(ShipData* shipIndex)
{
    const Face* facePtr;
    int i, px, py, pz, nx, ny, nz, dx, dy, dz, rx, ry, rz;
    int alpha, intensity;
    const vec3i* facePoint;
    ShipData* playerShip;

    renderSetBlend(BLEND_SUB);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        playerShip = shipIndex + i;

        if (!((playerShip->attr & VISIBLE) && ((playerShip->attr & FLYING) == 0)))
            continue;

        facePtr = gTrack.faces + playerShip->nearTrkSect->faceStart;

        while (!(facePtr->flags & TRACK_BASE))
        {
            facePtr++;
        }

        facePoint = gTrack.vertices + facePtr->vertex[0];

        dx = playerShip->unitVecNose.x;
        dy = playerShip->unitVecNose.y;
        dz = playerShip->unitVecNose.z;

        nx = facePtr->normal.x;
        ny = facePtr->normal.y;
        nz = facePtr->normal.z;

        // get fixed right vector
        rx = (dy * nz - dz * ny) >> FIXED_SHIFT;
        ry = (dz * nx - dx * nz) >> FIXED_SHIFT;
        rz = (dx * ny - dy * nx) >> FIXED_SHIFT;

        alpha = rsqrt(rx * rx + ry * ry + rz * rz);
        if (alpha == 0)
            continue;

        alpha = (1 << 16) / alpha;
        rx = rx * alpha >> (16 - FIXED_SHIFT);
        ry = ry * alpha >> (16 - FIXED_SHIFT);
        rz = rz * alpha >> (16 - FIXED_SHIFT);

        // get fixed direction vector
        dx = (ny * rz - nz * ry) >> FIXED_SHIFT;
        dy = (nz * rx - nx * rz) >> FIXED_SHIFT;
        dz = (nx * ry - ny * rx) >> FIXED_SHIFT;

        // get the shadow pivot point
        px = FRAME_LERP_POS(playerShip->_pos.x, playerShip->pos.x);
        py = FRAME_LERP_POS(playerShip->_pos.y, playerShip->pos.y);
        pz = FRAME_LERP_POS(playerShip->_pos.z, playerShip->pos.z);

        alpha = (nx * (px - facePoint->x) + ny * (py - facePoint->y) + nz * (pz - facePoint->z)) >> FIXED_SHIFT;

        intensity = 255 - (alpha >> 3);
        if (intensity < 0)
            intensity = 0;
        if (intensity > 255)
            intensity = 255;

        alpha -= SHADOW_BIAS;

        px -= (nx * alpha) >> FIXED_SHIFT;
        py -= (ny * alpha) >> FIXED_SHIFT;
        pz -= (nz * alpha) >> FIXED_SHIFT;

        renderShadow(0, intensity, px, py, pz, dx, dy, dz, rx, ry, rz);
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

#ifdef USE_VERTEX_COLOR

/*TODO optimize this shit!*/
#define MAX_AURORA_PRIMS 80

ObjFace* abPtr[MAX_AURORA_PRIMS];
uint16* abCoords[MAX_AURORA_PRIMS];
int16 greyCoords[MAX_AURORA_PRIMS];
int32 abCount;
int32 abPhase;

void InitAuroraBorialis(Object* object)
{
    int32 i;
    ObjFace* face;
    uint16* coords;
    int32 y;
    vec3s* vertex;

    abCount = 0;

    face = object->faces;
    vertex = object->vertices;

    for (i = 0; i < object->fCount; i++, face++)
    {
        coords = face->index;
        y = vertex[coords[0]].y;

        if (FACE_QUAD(face) && (y < -6000)) /* -8000 */
        {
            ASSERT(abCount < MAX_AURORA_PRIMS);
            abPtr[abCount] = face;

            if (y > -6800)
            {
                abCoords[abCount] = coords;
                greyCoords[abCount] = -1;
            }
            else if (y < -11000)
            {
                abCoords[abCount] = coords;
                greyCoords[abCount] = -2;
            }
            else
            {
                abCoords[abCount] = coords;
                greyCoords[abCount] = 0;
            }

            abCount++;
        }
    }
}

void AuroraBorialis(Object* object)
{
    int32 i, r, g, b;
    uint16* coords;

    abPhase++;

    for (i = 0; i < abCount; i++)
    {
        coords = abCoords[i];

        if (greyCoords[i] != -2)
        {
            r = (rsin(coords[0] * abPhase) >> 6) + 190;
            g = (rsin(coords[0] * (abPhase + 54)) >> 6) + 190;
            b = (rsin(coords[0] * (abPhase + 39)) >> 6) + 190;
            abPtr[i]->color[0] = r | (g << 8) | (b << 16) | 0xFF000000;

            r = (rsin(coords[1] * abPhase) >> 6) + 190;
            g = (rsin(coords[1] * (abPhase + 54)) >> 6) + 190;
            b = (rsin(coords[1] * (abPhase + 39)) >> 6) + 190;
            abPtr[i]->color[1] = r | (g << 8) | (b << 16) | 0xFF000000;
        }

        if (greyCoords[i] != -1)
        {
            r = (rsin(coords[2] * abPhase) >> 6) + 190;
            g = (rsin(coords[2] * (abPhase + 54)) >> 6) + 190;
            b = (rsin(coords[2] * (abPhase + 39)) >> 6) + 190;
            abPtr[i]->color[2] = r | (g << 8) | (b << 16) | 0xFF000000;

            r = (rsin(coords[3] * abPhase) >> 6) + 190;
            g = (rsin(coords[3] * (abPhase + 54)) >> 6) + 190;
            b = (rsin(coords[3] * (abPhase + 39)) >> 6) + 190;
            abPtr[i]->color[3] = r | (g << 8) | (b << 16) | 0xFF000000;
        }
    }
}

#else // #ifdef USE_VERTEX_COLOR

void InitAuroraBorialis(Object* object)
{
}

void AuroraBorialis(Object* object)
{
}

#endif
