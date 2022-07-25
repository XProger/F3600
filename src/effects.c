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

    renderSetMatrix(ctrlcamera.camPos->super, cx, cy, cz);

    renderDepthWrite(0);
    renderTransparent(1);

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
    renderTransparent(0);
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
                        fx->vx += (int16)(shipIndex[weapon->targetShip].vpivot.vx >> 6);
                        fx->vy += (int16)(shipIndex[weapon->targetShip].vpivot.vy >> 6);
                        fx->vz += (int16)(shipIndex[weapon->targetShip].vpivot.vz >> 6);
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
    if (ownShip < 2)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad1.tim");
    else if (ownShip < 4)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad2.tim");
    else if (ownShip < 6)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad3.tim");
    else if (ownShip < 8)
        gEffectTextures[FX_SHADOW] = LoadTexture("wipeout\\textures\\shad4.tim");

    /*TODO shadow
    Shadow* shadow = shipShadow;

    char u[6], v[6];

    u[0] = (effectTextures[FX_SHADOW]->u0 + effectTextures[FX_SHADOW]->u1) >> 1;
    v[0] = (effectTextures[FX_SHADOW]->v0 + effectTextures[FX_SHADOW]->v1) >> 1;
    u[1] = effectTextures[FX_SHADOW]->u2;
    v[1] = effectTextures[FX_SHADOW]->v2;
    u[2] = effectTextures[FX_SHADOW]->u3;
    v[2] = effectTextures[FX_SHADOW]->v3;
    u[3] = (u[0] + u[1]) >> 1;
    v[3] = (v[0] + v[1]) >> 1;
    u[4] = (u[1] + u[2]) >> 1;
    v[4] = (v[1] + v[2]) >> 1;
    u[5] = (u[2] + u[0]) >> 1;
    v[5] = (v[2] + v[0]) >> 1;

    for(i = 0; i < NO_OF_SHIPS; i++)
    {
       for(j = 0; j < 2; j++)
       {
          for(k = 0; k < 4; k++)
          {
             SetPolyFT3(&(shadow[i].poly[k][j]));
             setTPage(&(shadow[i].poly[k][j]),0, 2,
                effectTextures[FX_SHADOW]->textureX,
                effectTextures[FX_SHADOW]->textureY);
             shadow[i].poly[k][j].clut = effectTextures[FX_SHADOW]->cba;

             SetShadeTex((P_TAG *)&(shadow[i].poly[k][j]), 1);

             SetSemiTrans((P_TAG *)&(shadow[i].poly[k][j]), 1);

             shadow[i].poly[k][j].r0 = 230;   // PCwipeout
             shadow[i].poly[k][j].g0 = 0;
             shadow[i].poly[k][j].b0 = BLACK5_TRANSL;
          }
          setUV3(&(shadow[i].poly[0][j]),
             u[0], v[0],
             u[3], v[3],
             u[5], v[5]);
          setUV3(&(shadow[i].poly[1][j]),
             u[3], v[3],
             u[1], v[1],
             u[4], v[4]);
          setUV3(&(shadow[i].poly[2][j]),
             u[5], v[5],
             u[3], v[3],
             u[4], v[4]);
          setUV3(&(shadow[i].poly[3][j]),
             u[5], v[5],
             u[4], v[4],
             u[2], v[2]);
       }
    }
    */
}

void DrawShadow(Object** shipShapes, ShipData* shipIndex)
{
    Face* facePtr;
    VECTOR p;
    int i;
    int alpha;
    VECTOR facePoint;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (!((shipIndex[i].attr & VISIBLE) && ((shipIndex[i].attr & FLYING) == 0)))
            continue;

        facePtr = track->faces + shipIndex[i].nearTrkSect->faceStart;

        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        facePoint.vx = track->vertices[(facePtr->vertex[0])].vx;
        facePoint.vy = track->vertices[(facePtr->vertex[0])].vy;
        facePoint.vz = track->vertices[(facePtr->vertex[0])].vz;

        alpha = PlaneLineCollision(&facePoint, &p, &facePtr->normal);
        p.vx -= (alpha * facePtr->normal.vx) >> 12;
        p.vy -= (alpha * facePtr->normal.vy) >> 12;
        p.vz -= (alpha * facePtr->normal.vz) >> 12;

#if 0
        a.vx = (shipIndex[i].ppivot.vx + ((shipIndex[i].unitVecNose.vx * 3) >> 5));
        a.vy = (shipIndex[i].ppivot.vy + ((shipIndex[i].unitVecNose.vy * 3) >> 5));
        a.vz = (shipIndex[i].ppivot.vz + ((shipIndex[i].unitVecNose.vz * 3) >> 5));

        b.vx = (shipIndex[i].ppivot.vx - ((shipIndex[i].unitVecNose.vx * 3) >> 5)) + (shipIndex[i].unitVecWing.vx >> 4);
        b.vy = (shipIndex[i].ppivot.vy - ((shipIndex[i].unitVecNose.vy * 3) >> 5)) + (shipIndex[i].unitVecWing.vy >> 4);
        b.vz = (shipIndex[i].ppivot.vz - ((shipIndex[i].unitVecNose.vz * 3) >> 5)) + (shipIndex[i].unitVecWing.vz >> 4);

        c.vx = (shipIndex[i].ppivot.vx - ((shipIndex[i].unitVecNose.vx * 3) >> 5)) - (shipIndex[i].unitVecWing.vx >> 4);
        c.vy = (shipIndex[i].ppivot.vy - ((shipIndex[i].unitVecNose.vy * 3) >> 5)) - (shipIndex[i].unitVecWing.vy >> 4);
        c.vz = (shipIndex[i].ppivot.vz - ((shipIndex[i].unitVecNose.vz * 3) >> 5)) - (shipIndex[i].unitVecWing.vz >> 4);


        alpha = PlaneLineCollision(facePoint, b, facePtr->normal);
        b.vx -= (alpha * facePtr->normal.vx) >> 12;
        b.vy -= (alpha * facePtr->normal.vy) >> 12;
        b.vz -= (alpha * facePtr->normal.vz) >> 12;

        alpha = PlaneLineCollision(facePoint, c, facePtr->normal);
        c.vx -= (alpha * facePtr->normal.vx) >> 12;
        c.vy -= (alpha * facePtr->normal.vy) >> 12;
        c.vz -= (alpha * facePtr->normal.vz) >> 12;

        vertex[0].vx = a.vx + cameraPtr->camPos->relative.t[0];
        vertex[0].vy = a.vy + cameraPtr->camPos->relative.t[1];
        vertex[0].vz = a.vz + cameraPtr->camPos->relative.t[2];

        vertex[1].vx = b.vx + cameraPtr->camPos->relative.t[0];
        vertex[1].vy = b.vy + cameraPtr->camPos->relative.t[1];
        vertex[1].vz = b.vz + cameraPtr->camPos->relative.t[2];

        vertex[2].vx = c.vx + cameraPtr->camPos->relative.t[0];
        vertex[2].vy = c.vy + cameraPtr->camPos->relative.t[1];
        vertex[2].vz = c.vz + cameraPtr->camPos->relative.t[2];

        // Depth returned is 1/4 the Z component sz of the screen coords
        // corresponding to v2
        //           Faster but inaccurate sorting !!!!
#if 0
             depth = RotTransPers3(  &(vertex[0]),
                &(vertex[1]),
                &(vertex[2]),
                &(sxy[0]),
                &(sxy[1]),
                &(sxy[2]),
                &p,
                &flag);

#else
        // Depth returned for each vertex
        // Slower but slightly better sorting  !!!!

        depth[0] = RotTransPers(&(vertex[0]), &(sxy[0]), &p, &flag);

        depth[1] = RotTransPers(&(vertex[1]), &(sxy[1]), &p, &flag);

        depth[2] = RotTransPers(&(vertex[2]), &(sxy[2]), &p, &flag);
#endif

        x[0] = sxy[0].vx;
        y[0] = sxy[0].vy;
        x[1] = sxy[1].vx;
        y[1] = sxy[1].vy;
        x[2] = sxy[2].vx;
        y[2] = sxy[2].vy;

        x[3] = (x[0] + x[1]) >> 1;
        y[3] = (y[0] + y[1]) >> 1;
        x[4] = (x[1] + x[2]) >> 1;
        y[4] = (y[1] + y[2]) >> 1;
        x[5] = (x[2] + x[0]) >> 1;
        y[5] = (y[2] + y[0]) >> 1;

        if (!(flag & (CrtClipped | ZNegative | ZLarge | ZSmall)))
        {

            shadow[i].poly[0][CurrentScreen].x0 = x[0];
            shadow[i].poly[0][CurrentScreen].y0 = y[0];
            shadow[i].poly[0][CurrentScreen].x1 = x[3];
            shadow[i].poly[0][CurrentScreen].y1 = y[3];
            shadow[i].poly[0][CurrentScreen].x2 = x[5];
            shadow[i].poly[0][CurrentScreen].y2 = y[5];

            shadow[i].poly[0][CurrentScreen].z0 = vertex[0].vz;
            shadow[i].poly[0][CurrentScreen].z1 = vertex[0].vz;
            shadow[i].poly[0][CurrentScreen].z2 = vertex[0].vz;
            shadow[i].poly[0][CurrentScreen].pad4 = 0;
            shadow[i].poly[0][CurrentScreen].pad5 = 0;
            shadow[i].poly[0][CurrentScreen].pad6 = 0;

            shadow[i].poly[1][CurrentScreen].x0 = x[3];
            shadow[i].poly[1][CurrentScreen].y0 = y[3];
            shadow[i].poly[1][CurrentScreen].x1 = x[1];
            shadow[i].poly[1][CurrentScreen].y1 = y[1];
            shadow[i].poly[1][CurrentScreen].x2 = x[4];
            shadow[i].poly[1][CurrentScreen].y2 = y[4];

            shadow[i].poly[1][CurrentScreen].z0 = vertex[1].vz;
            shadow[i].poly[1][CurrentScreen].z1 = vertex[1].vz;
            shadow[i].poly[1][CurrentScreen].z2 = vertex[1].vz;
            shadow[i].poly[1][CurrentScreen].pad4 = 0;
            shadow[i].poly[1][CurrentScreen].pad5 = 0;
            shadow[i].poly[1][CurrentScreen].pad6 = 0;

            shadow[i].poly[2][CurrentScreen].x0 = x[5];
            shadow[i].poly[2][CurrentScreen].y0 = y[5];
            shadow[i].poly[2][CurrentScreen].x1 = x[3];
            shadow[i].poly[2][CurrentScreen].y1 = y[3];
            shadow[i].poly[2][CurrentScreen].x2 = x[4];
            shadow[i].poly[2][CurrentScreen].y2 = y[4];

            shadow[i].poly[2][CurrentScreen].z0 = vertex[0].vz;
            shadow[i].poly[2][CurrentScreen].z1 = vertex[0].vz;
            shadow[i].poly[2][CurrentScreen].z2 = vertex[0].vz;
            shadow[i].poly[2][CurrentScreen].pad4 = 0;
            shadow[i].poly[2][CurrentScreen].pad5 = 0;
            shadow[i].poly[2][CurrentScreen].pad6 = 0;

            shadow[i].poly[3][CurrentScreen].x0 = x[5];
            shadow[i].poly[3][CurrentScreen].y0 = y[5];
            shadow[i].poly[3][CurrentScreen].x1 = x[4];
            shadow[i].poly[3][CurrentScreen].y1 = y[4];
            shadow[i].poly[3][CurrentScreen].x2 = x[2];
            shadow[i].poly[3][CurrentScreen].y2 = y[2];

            shadow[i].poly[3][CurrentScreen].z0 = vertex[2].vz;
            shadow[i].poly[3][CurrentScreen].z1 = vertex[2].vz;
            shadow[i].poly[3][CurrentScreen].z2 = vertex[2].vz;
            shadow[i].poly[3][CurrentScreen].pad4 = 0;
            shadow[i].poly[3][CurrentScreen].pad5 = 0;
            shadow[i].poly[3][CurrentScreen].pad6 = 0;

            depth[0] -= 180;
            if (depth[0] < 0)
                depth[0] = 0;
            depth[1] -= 60;
            if (depth[1] < 0)
                depth[1] = 0;
            depth[2] -= 60;
            if (depth[2] < 0)
                depth[2] = 0;

            AddPrim(OT[CurrentScreen] + (depth[0]), (ulong*)&(shadow[i].poly[0][CurrentScreen]));
            AddPrim(OT[CurrentScreen] + (depth[2]), (ulong*)&(shadow[i].poly[1][CurrentScreen]));
            AddPrim(OT[CurrentScreen] + (depth[2]), (ulong*)&(shadow[i].poly[2][CurrentScreen]));
            AddPrim(OT[CurrentScreen] + (depth[2]), (ulong*)&(shadow[i].poly[3][CurrentScreen]));
        }
#endif
    }
}

void InitWeaponSprtIcons(Texture** weaponIconTextures)
{
    int tablePos = TextureTableCount;

    LoadCompressedTextureSequence("wipeout\\common\\wicons.cmp");

    weaponIconTextures[MINE - 1] = TextureTable[tablePos++];
    weaponIconTextures[MISSILE - 1] = TextureTable[tablePos++];
    weaponIconTextures[ROCKET - 1] = TextureTable[tablePos++];
    weaponIconTextures[SPECIAL - 1] = TextureTable[tablePos++];
    weaponIconTextures[ELEC_BOLT - 1] = TextureTable[tablePos++];
    weaponIconTextures[FLARE - 1] = TextureTable[tablePos++];
    weaponIconTextures[REV_CON - 1] = TextureTable[tablePos++];
    weaponIconTextures[SHIELD - 1] = TextureTable[tablePos++];
    weaponIconTextures[TURBO - 1] = TextureTable[tablePos++];
}

void DrawWeaponSprtIcons(Texture** weaponIconTextures, ShipData* playerShip)
{
    int icon = playerShip->weaponType;

    if (icon != NO_WEAPON)
    {
        icon -= 1;
        AddGlyph(144, 20, 32, 32, 0, 0, weaponIconTextures[icon]);
    }
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
        }
        if (greyCoords[i] != -2)
        {
            abPtr[i]->colour[1].r = (rsin(coords[1] * count) >> 6) + 190;
            abPtr[i]->colour[1].g = (rsin(coords[1] * (count + 54)) >> 6) + 190;
            abPtr[i]->colour[1].b = (rsin(coords[1] * (count + 39)) >> 6) + 190;
        }
        if (greyCoords[i] != -1)
        {
            abPtr[i]->colour[2].r = (rsin(coords[2] * count) >> 6) + 190;
            abPtr[i]->colour[2].g = (rsin(coords[2] * (count + 54)) >> 6) + 190;
            abPtr[i]->colour[2].b = (rsin(coords[2] * (count + 39)) >> 6) + 190;
        }

        if (greyCoords[i] != -1)
        {
            abPtr[i]->colour[3].r = (rsin(coords[3] * count) >> 6) + 190;
            abPtr[i]->colour[3].g = (rsin(coords[3] * (count + 54)) >> 6) + 190;
            abPtr[i]->colour[3].b = (rsin(coords[3] * (count + 39)) >> 6) + 190;
        }
    }
}
#endif
