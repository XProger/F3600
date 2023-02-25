/************************************************************/
/******     ships.c - Ship initialisation routines      *****/
/************************************************************/

#include "game.h"

ShipData* championShipIndex[NO_OF_SHIPS];

const short startLinePos[14] = {
// Venom
    27,     /* Altima VII   */
    16,     /* Karbonis     */
    27,     /* TerrorMax    */
    16,     /* Korrodera    */
    16,     /* Arridos IV   */
    16,     /* SilverStream */
    27,     /* FireStar     */
// Rapier
    27,     /* Altima VII   */
    16,     /* Karbonis     */
    27,     /* TerrorMax    */
    16,     /* Korrodera    */
    16,     /* Arridos IV   */
    16,     /* SilverStream */
    27      /* FireStar     */
};

typedef struct
{
    int16 maxThrust;
    int16 headingInc;
    int16 maxHeading;
    uint8 resistance;
    uint8 skid;
} ShipSpecs;

const ShipSpecs SHIP_SPECS[2][NO_OF_SHIPS / 2] =
{
    { // Venom
        { 790, 160, 2560, 140, 12 },
        { 850, 140, 1920, 134, 20 },
        { 850, 120, 1920, 140, 24 },
        { 790, 180, 2560, 134, 12 }
    },
    { // Rapier
        { 1200, 160, 2560, 140, 10 },
        { 1400, 120, 1920, 140, 14 },
        { 1400, 140, 1920, 130, 16 },
        { 1200, 180, 2560, 130, 8  }
    }
};

typedef struct
{
    int16 maxThrust;
    int16 thrustMag;
} RemoteShipSpecs;

const RemoteShipSpecs REMOTE_SHIP_SPECS[2][NO_OF_SHIPS] =
{
    { // Venom
        { 2520, 43 },
        { 2550, 44 },
        { 2600, 45 },
        { 2630, 45 },
        { 2660, 46 },
        { 2700, 47 },
        { 2720, 48 },
        { 2750, 49 }
    },
    { // Rapier
        { 3720, 47 },
        { 3750, 50 },
        { 3780, 53 },
        { 3800, 55 },
        { 3850, 57 },
        { 3900, 60 },
        { 3950, 62 },
        { 4000, 65 }
    }
};

int32 SetSpread(int32 order)
{
    int32 speed;

    order--;

    switch (gConfig.trackNum)
    {
        case 0:
            speed = 80 + order * 20;
            break;
        case 1:
            speed = 10 + order * 8;
            break;
        case 2:
            speed = 60 + order * 11;
            break;
        case 3:
            speed = 40 + order * 11;
            break;
        case 4:
            speed = 80 + order * 15;
            break;
        case 5:
            speed = 10 + order * 8;
            break;
        case 6:
            speed = 40 + order * 11;
            break;
        case 7:
            speed = 80 + order * 11;
            break;
        case 8:
            speed = 10 + order * 8;
            break;
        case 9:
            speed = 60 + order * 8;
            break;
        case 10:
            speed = 30 + order * 11;
            break;
        case 11:
            speed = 30 + order * 11;
            break;
        case 12:
            speed = 10 + order * 8;
            break;
        case 13:
            speed = 40 + order * 11;
            break;
        default:
            speed = 0;
            ASSERT(0);
    }

    return speed * order;
}

void DrawShips(ShipData* shipIndex)
{
    int32 i;
    int32 dist, dx, dy, dz, vz;
    Object* obj;
    ShipData* ship = shipIndex;
    int32 attr;

    for (i = 0; i < NO_OF_SHIPS; i++, ship++)
    {
        ship = shipIndex + i;

        if (!(ship->player || (gConfig.raceType != TIME_TRIAL)))
            continue;

        dx = ship->pos.x - gCameraViewPos.x;
        dz = ship->pos.z - gCameraViewPos.z;

        dist = GetMagnitudeApprox(dx, dz);

        attr = ship->attr & ~VISIBLE;

        if (dist < SHIP_VIS_DIST)
        {
            dy = ship->pos.y - gCameraViewPos.y;
            vz = ((dx * gMatrixPtr->e20) + (dy * gMatrixPtr->e21) + (dz * gMatrixPtr->e22)) >> FIXED_SHIFT;

            if (vz > 0)
            {
            #ifdef USE_SHIP_LODS
                if (dist < SHIP_LOD_DIST)
                    obj = ship->shape;
                else
                    obj = ship->shape + NO_OF_SHIPS;
            #else
                obj = ship->shape;
            #endif

                if (!(attr & VIEW_IN) || (attr & REMOTE_VIEW))
                {
                    int32 px = ship->pos.x;
                    int32 py = ship->pos.y;
                    int32 pz = ship->pos.z;
                    int32 rx = ship->pitch;
                    int32 ry = ship->hdg;
                    int32 rz = ship->roll;

                    FRAME_LERP(ship);

                    matrixPush();
                    matrixTranslate(px, py, pz);
                    matrixRotateYXZ(rx, ry, rz);

                #ifdef USE_SHIP_AMBIENT
                    ShipCalcAmbient(ship);
                #endif

                    renderShip(obj, ship->color);

                    matrixPop();

                    attr |= VISIBLE;
                }
            }
        }

        ship->attr = attr;
    }
}

void InitShipData(TrackSection* section, ShipData* shipIndex)
{
    int32 i, j, t, order;
    Face* face;
    vec3i facePoint;
    vec3i targetVector;
    ShipData* ship;
    TrackSection* startSection[NO_OF_SHIPS];
    int32 startSecNo = startLinePos[gConfig.trackNum];
    const ShipSpecs* specs;
    const RemoteShipSpecs* remoteSpecs;

    int8 usedShape[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    order = 0;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;
        if (player->shapeIndex != -1)
        {
            usedShape[player->shapeIndex] = 1;
            player->ship->shape = ctrlshipShapes + player->shapeIndex;
            player->ship->order = order++;
        }
    }

    // shuffle players
    for (i = 1; i < gNumPlayers; i++)
    {
        j = randy() % gNumPlayers;
        t = shipIndex[i].order;
        shipIndex[i].order = shipIndex[j].order;
        shipIndex[j].order = t;
    }

    if ((gConfig.raceType != CHAMPIONSHIP) || (gConfig.trackNum == 0) || (gConfig.trackNum == 7))
    {
        for (i = gNumPlayers; i < NO_OF_SHIPS; i++)
        {
            for (j = 0; j < NO_OF_SHIPS; j++)
            {
                if (usedShape[j] == 0)
                {
                    usedShape[j] = 1;
                    shipIndex[i].shape = ctrlshipShapes + j;
                    shipIndex[i].order = order++;
                    break;
                }
            }
        }

        ASSERT(order == NO_OF_SHIPS);

        // shuffle enemies
        for (i = gNumPlayers; i < NO_OF_SHIPS; i++)
        {
            j = gNumPlayers + (randy() % (NO_OF_SHIPS - gNumPlayers));
            t = shipIndex[i].order;
            shipIndex[i].order = shipIndex[j].order;
            shipIndex[j].order = t;
        }

        for (i = 0; i < NO_OF_SHIPS; i++)
        {
            championShipIndex[shipIndex[i].order] = shipIndex + i;
        }
    }
    else
    {
        if (randy() & 1)
        {
            t = championShipIndex[5]->order;
            championShipIndex[5]->order = championShipIndex[6]->order;
            championShipIndex[6]->order = t;
        }

        for (i = gNumPlayers; i < 5; i++)
        {
            j = gNumPlayers + (randy() % (5 - gNumPlayers));
            t = championShipIndex[i]->order;
            championShipIndex[i]->order = championShipIndex[j]->order;
            championShipIndex[j]->order = t;
        }
    }

    for (i = 0; i < startSecNo - 15; i++)
    {
        section = section->nextSection;
    }

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        startSection[i] = section;
        section = section->nextSection;
        if ((i & 1) == 0)
            section = section->nextSection;
    }

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ship = shipIndex + i;

        ASSERT(ship->shape);

        ship->vel.x = 0;
        ship->vel.y = 0;
        ship->vel.z = 0;
        ship->hdg = 0;
        ship->pitch = 0;
        ship->roll = 0;
        ship->vhdg = 0;
        ship->vpitch = 0;
        ship->vroll = 0;
        ship->ahdg = 0;
        ship->thrustMag = 0;
        ship->brakeRotR = 0;
        ship->brakeRotL = 0;
        ship->attr = RACING | VISIBLE | DIRECTION;
        ship->weaponType = WPN_NONE;
        ship->lapNo = 0;
        ship->speed = 0;
        ship->electroCount = 0;
        ship->revConCount = 0;
        ship->crossedLine = 0;

        t = (ship->shape - ctrlshipShapes) >> 1; // team
        specs = SHIP_SPECS[gConfig.raceClass] + t;

        ship->maxThrust = specs->maxThrust;
        ship->headingInc = specs->headingInc;
        ship->maxHeading = specs->maxHeading;
        ship->resistance = specs->resistance;
        ship->skid = specs->skid;

        for (j = 0; j < 5; j++)
            ship->lapTimes[j] = 0;

        ship->updateCount = START_TIME;
        ship->targetShip = NULL;

        order = ship->order;
        section = startSection[order];

        if (ship->player)
        {
            ship->update = UpdatePlayerShipNormStart;

            ship->remoteMaxThrust = 2900;
            ship->remoteThrustMag = 46;
        }
        else
        {
            ship->update = UpdateRemoteShipStart;

            remoteSpecs = REMOTE_SHIP_SPECS[gConfig.raceClass] + order;
            ship->remoteMaxThrust = remoteSpecs->maxThrust;
            ship->remoteThrustMag = remoteSpecs->thrustMag;

            ship->numtospeed = SetSpread(order);
        }

        ship->nearTrkSect = ship->prevShipSect = section;

        face = gTrack.faces + section->faceStart; // left border face
        face++; // left side
        if (order & 1)
        {
            face++; // right side
        }

        facePoint.x = (gTrack.vertices[(face->vertex[0])].x + gTrack.vertices[(face->vertex[2])].x) >> 1;
        facePoint.y = (gTrack.vertices[(face->vertex[0])].y + gTrack.vertices[(face->vertex[2])].y) >> 1;
        facePoint.z = (gTrack.vertices[(face->vertex[0])].z + gTrack.vertices[(face->vertex[2])].z) >> 1;
        ship->pos.x = facePoint.x + ((face->normal.x * 200) >> FIXED_SHIFT);
        ship->pos.y = facePoint.y + ((face->normal.y * 200) >> FIXED_SHIFT);
        ship->pos.z = facePoint.z + ((face->normal.z * 200) >> FIXED_SHIFT);

        ship->nearSecNo = ship->prevSecNo = ship->DPASectDiff = section->secNo;

        targetVector.x = section->nextSection->centre.x - section->centre.x;
        targetVector.y = section->nextSection->centre.y - section->centre.y;
        targetVector.z = section->nextSection->centre.z - section->centre.z;
        ship->hdg = -ratan2(targetVector.x, targetVector.z);
        FRAME_LERP_INIT(ship);
    }
}
