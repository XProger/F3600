/************************************************************/
/******     ships.c - Ship initialisation routines      *****/
/************************************************************/

#include "game.h"

Cone ctrlcone;

const short startLinePos[16] = {
    27,     /* NULL             */
    27,     /* V - TerrorMax    */
    27,     /* V - Altima VII   */
    27,     /* R - Altima VII   */
    16,     /* V - Karbonis     */
    16,     /* R - Karbonis     */
    27,     /* R - TerrorMax    */
    16,     /* R - Korrodera    */
    16,     /* V - Arridos IV   */
    16,     /* V - SilverStream */
    27,     /* V - FireStar     */
    16,     /* R - Arridos IV   */
    16,     /* V - Korrodera    */
    16,     /* R - SilverStream */
    27,     /* R - FireStar     */
    27 };   /* NULL             */

const short behindSpeed[16] = {
    100,    /* NULL             */
    350,    /* V - TerrorMax    */
    300,    /* V - Altima VII   */
    500,    /* R - Altima VII   */
    200,    /* V - Karbonis     */
    500,    /* R - Karbonis     */
    500,    /* R - TerrorMax    */
    500,    /* R - Korrodera    */
    350,    /* V - Arridos IV   */
    150,    /* V - SilverStream */
    200,    /* V - FireStar     */
    450,    /* R - Arridos IV   */
    450,    /* V - Korrodera    */
    10,     /* R - SilverStream */
    500,    /* R - FireStar     */
    200 };  /* NULL             */

const short shipShapeOrder[8] = { 7, 3, 5, 6, 1, 4, 0, 2 };

short championOrder[8];
short raceOrder[8];
short shipOrder[8];

void SetSpread(ShipData* ship, ConfigData* gameData, int position)
{
    int32 speed;

    position--;

    switch (gameData->actTrackNum)
    {
        case 1:
            speed = 60 + position * 11;
            break;
        case 2:
            speed = 80 + position * 20;
            break;
        case 3:
            speed = 80 + position * 11;
            break;
        case 4:
            speed = 10 + position * 8;
            break;
        case 5:
            speed = 10 + position * 8;
            break;
        case 6:
            speed = 60 + position * 8;
            break;
        case 7:
            speed = 30 + position * 11;
            break;
        case 8:
            speed = 80 + position * 15;
            break;
        case 9:
            speed = 10 + position * 8;
            break;
        case 10:
            speed = 40 + position * 11;
            break;
        case 11:
            speed = 30 + position * 11;
            break;
        case 12:
            speed = 40 + position * 11;
            break;
        case 13:
            speed = 10 + position * 8;
            break;
        case 14:
            speed = 40 + position * 11;
            break;
        default:
            speed = 0;
            ASSERT(0);
    }

    ship->numtospeed = speed * position;
}

void DrawShips(ShipData* shipIndex, Object** shipShapes, int camShip)
{
    int32 i;
    int32 dist, dx, dy, dz, vz;
    Object* obj;
    ShipData* ship;
    int32 attr;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (!((i == ownShip) || (raceType != TIME_TRIAL)))
            continue;

        ship = shipIndex + i;

        dx = ship->pos.x - gCamera.pos.x;
        dz = ship->pos.z - gCamera.pos.z;

        dist = GetMagnitudeApprox(dx, dz);

        attr = ship->attr & ~VISIBLE;

        if (dist < SHIP_VIS_DIST)
        {
            dy = ship->pos.y - gCamera.pos.y;
            vz = ((dx * gMatrixPtr->e20) + (dy * gMatrixPtr->e21) + (dz * gMatrixPtr->e22)) >> FIXED_SHIFT;

            if (vz > 0)
            {
            #ifdef USE_SHIP_LODS
                if (dist < SHIP_LOD_DIST)
                    obj = shipShapes[ship->highResTableIndex];
                else
                    obj = shipShapes[ship->lowResTableIndex];
            #else
                obj = shipShapes[ship->highResTableIndex];
            #endif

                if (!(attr & VIEW_IN) || (attr & REMOTE_VIEW))
                {
                    attr |= VISIBLE;

                    matrixPush();
                    matrixTranslate(ship->pos.x, ship->pos.y, ship->pos.z);
                    matrixRotateYXZ(ship->pitch, ship->hdg, ship->roll);

                    renderShip(obj, ship->color);

                    matrixPop();
                }
            }
        }

        ship->attr = attr;
    }
}

void loadShipData(ShipData* shipIndex, Object** shipShapes)
{
    int shipTextures, droidTextures, colTextures;
    Object* obj;
    int32 i, objCount;
    int32 count = 0;

    /******************************************************/
    /** Load compressed texture files, first to allow    **/
    /** temporary memory to be allocated                 **/
    /******************************************************/

    shipTextures = gTexturePtr - gTextures;
    LoadCompressedTextureSequence("wipeout/common/allsh.cmp", 1);

    colTextures = gTexturePtr - gTextures;
    LoadCompressedTextureSequence("wipeout/common/alcol.cmp", 1);

    droidTextures = gTexturePtr - gTextures;
    LoadCompressedTextureSequence("wipeout/common/rescu.cmp", 1);

    obj = LoadPrm("wipeout/common/allsh.prm", shipTextures, &objCount);
    ASSERT(objCount == NO_OF_SHIPS);

    for (i = 0; i < objCount; i++)
    {
        shipIndex[count].highResTableIndex = shipShapeOrder[count];
        shipShapes[count] = obj + i;

        count += 1;
    }

    obj = LoadPrm("wipeout/common/alcol.prm", colTextures, &objCount);
    ASSERT(objCount == NO_OF_SHIPS);

    for (i = 0; i < objCount; i++)
    {
        shipIndex[count - NO_OF_SHIPS].lowResTableIndex = count;
        shipShapes[count] = obj + i;

        count += 1;
    }

    shipShapes[RESCUE_DROID] = LoadPrm("wipeout/common/rescu.prm", droidTextures, NULL);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ctrlcone.object[i] = shipShapes[shipIndex[i].highResTableIndex];
        ctrlcone.ship[i] = &shipIndex[i];
    }

    InitCone(&ctrlcone);
}

void initShipData(TrackSection* section, ShipData* shipIndex, ConfigData* gameData)
{
    int i, j;
    Face* face;
    vec4i targetVector;
    TrackSection* nextSection;
    vec4i facePoint;
    TrackSection* startSection[8];
    short position = 1;
    short used[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    int index;
    ShipData* ship;

    /***********************************************************************************/
    /**                     Ship ATTRIBUTES                                        *****/
    /***********************************************************************************/
    /******   TEAM 1 -   Acc            ***                                        *****/
    /******              Top Speed      ****                                       *****/
    /******              Armour         *****                                      *****/
    /******              Turn Rate      **                                         *****/
    /***********************************************************************************/
    /******   TEAM 2 -   Acc            *****                                      *****/
    /******              Top Speed      **                                         *****/
    /******              Armour         ***                                        *****/
    /******              Turn Rate      ****                                       *****/
    /***********************************************************************************/
    /******   TEAM 3 -   Acc            **                                         *****/
    /******              Top Speed      *****                                      *****/
    /******              Armour         ****                                       *****/
    /******              Turn Rate      ***                                        *****/
    /***********************************************************************************/
    /******   TEAM 4 -   Acc            ****                                       *****/
    /******              Top Speed      ***                                        *****/
    /******              Armour         **                                         *****/
    /******              Turn Rate      *****                                      *****/
    /***********************************************************************************/

    /****                                  Team Specific                                     ****/
    /**** Ship No               0       1       2       3       4       5       6       7    ****/
    short maxThrustBeg[8]   = { 790,    790,    850,    850,    850,    850,    790,    790     };
    short resistanceBeg[8]  = { 140,    140,    134,    134,    140,    140,    134,    134     };
    short headingIncBeg[8]  = { 160,    160,    140,    140,    120,    120,    180,    180     };
    short maxHeadingBeg[8]  = { 2560,   2560,   1920,   1920,   1920,   1920,   2560,   2560    };
    short skidBeg[8]        = { 12,     12,     20,     20,     24,     24,     12,     12      };

    short maxThrustPro[8]   = { 1200,   1200,   1400,   1400,   1400,   1400,   1200,   1200    };
    short resistancePro[8]  = { 140,    140,    140,    140,    130,    130,    130,    130     };
    short headingIncPro[8]  = { 160,    160,    120,    120,    140,    140,    180,    180     };
    short maxHeadingPro[8]  = { 2560,   2560,   1920,   1920,   1920,   1920,   2560,   2560    };
    short skidPro[8]        = { 10,     10,     14,     14,     16,     16,     8,      8       };

    /****                      Remote Control Specific                               ****/
    /**** Ship No               0       1       2       3       4       5       6    ****/
    short RMaxThrustBeg[7]  = { 2550,   2600,   2630,   2660,   2700,   2720,   2750    };
    short RThrustMagBeg[7]  = { 44,     45,     45,     46,     47,     48,     49      };
    short fightBackBeg[7]   = { 1,      1,      1,      1,      1,      1,      1       };

    short RMaxThrustPro[7]  = { 3750,   3780,   3800,   3850,   3900,   3950,   4000    };
    short RThrustMagPro[7]  = { 50,     53,     55,     57,     60,     62,     65      };
    short fightBackPro[7]   = { 1,      1,      1,      1,      1,      1,      1       };

    crossedLine = 0;

    if (((gameData->gameType == CHAMPIONSHIP) && (gameData->trackNum == 1)) || ((gameData->gameType == CHAMPIONSHIP) && (gameData->trackNum == 8)) || (gameData->gameType != CHAMPIONSHIP))
    {
        for (i = 0; i < NO_OF_SHIPS; i++)
        {
            if (i != ownShip)
            {
                while (1)
                {
                    index = randy() % 8;
                    if (index != 0)
                    {
                        if (used[index] == 0)
                        {
                            championOrder[index] = i;
                            raceOrder[index] = i;
                            shipOrder[i] = index;
                            used[index] = 1;
                            break;
                        }
                    }
                }
            }
        }

        championOrder[0] = ownShip;
        raceOrder[0] = ownShip;
        shipOrder[ownShip] = 0;
    }
    else if (gameData->gameType == CHAMPIONSHIP)
    {
        raceOrder[7] = championOrder[7];
        shipOrder[raceOrder[7]] = 7;

        if ((randy() % 2) == 0)
        {
            raceOrder[5] = championOrder[5];
            raceOrder[6] = championOrder[6];
        }
        else
        {
            raceOrder[6] = championOrder[5];
            raceOrder[5] = championOrder[6];
        }

        shipOrder[raceOrder[5]] = 5;
        shipOrder[raceOrder[6]] = 6;

        for (i = 1; i < 5; i++)
        {
            while (1)
            {
                index = (randy() % 4) + 1;
                {
                    if (used[index] == 0)
                    {
                        used[index] = 1;
                        raceOrder[i] = championOrder[index];
                        break;
                    }
                }
            }
        }
        shipOrder[raceOrder[4]] = 4;
        shipOrder[raceOrder[3]] = 3;
        shipOrder[raceOrder[2]] = 2;
        shipOrder[raceOrder[1]] = 1;

        shipOrder[ownShip] = 0;
        raceOrder[0] = ownShip;
    }

    for (i = 0; i < startLinePos[gameData->actTrackNum] - 15; i++)
    {
        section = section->nextSection;
    }

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        startSection[i] = section;
        section = section->nextSection;
        if ((i % 2) == 0)
            section = section->nextSection;
    }

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ship = shipIndex + i;

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
        ship->thrust_mag = 0;
        ship->r_brake_rot = 0;
        ship->l_brake_rot = 0;
        ship->attr = 0;
        ship->attr |= RACING;
        ship->attr |= VISIBLE;
        ship->attr |= DIRECTION;
        ship->combatAttr = 0;
        ship->weaponType = WPN_NONE;
        ship->lapNo = 0;
        ship->speed = 0;
        ship->electroCount = 0;
        ship->revConCount = 0;

        if (gameData->Class == 0)
        {
            ship->max_thrust = maxThrustBeg[i];
            ship->skid = skidBeg[i];
        }
        else
        {
            ship->max_thrust = maxThrustPro[i];
            ship->skid = skidPro[i];
        }

        if (gameData->Class == 0)
        {
            ship->headingInc = headingIncBeg[i];
            ship->maxHeading = maxHeadingBeg[i];
        }
        else
        {
            ship->headingInc = headingIncPro[i];
            ship->maxHeading = maxHeadingPro[i];
        }

        if (gameData->Class == 0)
        {
            ship->resistance = resistanceBeg[i];
            ship->fightBack = fightBackBeg[i];
        }
        else
        {
            ship->resistance = resistancePro[i];
            ship->fightBack = fightBackPro[i];
        }

        for (j = 0; j < 5; j++)
            ship->lapTimes[j] = 0;

        ship->updateCount = START_TIME;
        ship->targetShip = 0xFF;

        if (i == ownShip)
        {
            ship->update = UpdatePlayerShipNormStart;
        }
        else
            ship->update = UpdateRemoteShipStart;

        if (i == ownShip)
        {
            position = 0;
            ship->position = position;
            ship->remoteMaxThrust = 2900;
            ship->remoteThrustMag = 46;
            ship->fightBack = 0;
        }
        else
        {
            position = shipOrder[i];
            ship->position = position;

            if (gameData->Class == 0)
            {
                ship->remoteMaxThrust = RMaxThrustBeg[position - 1];
                ship->remoteThrustMag = RThrustMagBeg[position - 1];
                ship->fightBack = fightBackBeg[position - 1];
            }
            else
            {
                ship->remoteMaxThrust = RMaxThrustPro[position - 1];
                ship->remoteThrustMag = RThrustMagPro[position - 1];
                ship->fightBack = fightBackPro[position - 1];
            }
        }

        ship->nearTrkSect = startSection[position];
        ship->prevShipSect = startSection[position];
        SetSpread(ship, gameData, position);

        face = gTrack.faces + startSection[position]->faceStart;
        face++;
        if ((position % 2) != 0)
            face++;

        facePoint.x = (gTrack.vertices[(face->vertex[0])].x + gTrack.vertices[(face->vertex[2])].x) >> 1;
        facePoint.y = (gTrack.vertices[(face->vertex[0])].y + gTrack.vertices[(face->vertex[2])].y) >> 1;
        facePoint.z = (gTrack.vertices[(face->vertex[0])].z + gTrack.vertices[(face->vertex[2])].z) >> 1;
        ship->pos.x = facePoint.x + ((face->normal.x * 200) >> FIXED_SHIFT);
        ship->pos.y = facePoint.y + ((face->normal.y * 200) >> FIXED_SHIFT);
        ship->pos.z = facePoint.z + ((face->normal.z * 200) >> FIXED_SHIFT);

        ship->nearSecNo = startSection[position]->secNo;
        ship->prevSecNo = startSection[position]->secNo;
        ship->DPASectDiff = startSection[position]->secNo;

        nextSection = startSection[position]->nextSection;
        targetVector.x = nextSection->centre.x - startSection[position]->centre.x;
        targetVector.y = nextSection->centre.y - startSection[position]->centre.y;
        targetVector.z = nextSection->centre.z - startSection[position]->centre.z;
        ship->hdg = -ratan2(targetVector.x, targetVector.z);
    }
}
