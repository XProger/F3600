#include "game.h"

extern short behindSpeed[];
extern short shipOrder[];

#define FULL   0
#define HIGH   3
#define MEDIUM 2
#define LOW    1
#define SMALL  0

extern int gPadStickX;
extern int gPadStickY;

/**********************************************************************/
/***              Call update routine for each ship                 ***/
/**********************************************************************/
void UpdateShips(ShipData* shipIndex, WeaponData* weaponIndex)
{
    int i;

    SetUnitVectors(shipIndex);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        (shipIndex[i].update)(shipIndex, i, weaponIndex);

    #ifdef USE_SHIP_AMBIENT
        if (shipIndex[i].attr & VISIBLE)
        {
            ShipCalcAmbient(shipIndex + i);
        }
    #endif
    }
}

/**************************************************************************/
/****                                                                  ****/
/****                         START ROUTINES                           ****/
/****                                                                  ****/
/**************************************************************************/

void GeneralStartProcs(ShipData* playerShip)
{
    if (playerShip->updateCount == START_TIME)
    {
        PlayNote(SFX_REMOTE_ENG);
        SetVolume(SFX_REMOTE_ENG, 0, 0);

        PlayNote(SFX_ENG_RUMBLE);
        SetVolume(SFX_ENG_RUMBLE, 0, 0);

        PlayNote(SFX_ENG_AIR_INTAKE);
        SetVolume(SFX_ENG_AIR_INTAKE, 0, 0);

        PlayNote(SFX_ENG_THRUST);
        SetVolume(SFX_ENG_THRUST, 50, 50);

        PlayNote(SFX_TURBULENCE);
        SetVolume(SFX_TURBULENCE, 0, 0);

        PlayNote(SFX_CROWD);
        SetVolume(SFX_CROWD, 0, 0);
        SetPitch(SFX_CROWD, SND_DEF_PITCH >> 1);
    }

    playerShip->updateCount--;

    if (playerShip->updateCount == RACE_START)
    {
        ColourStartBoom(3);
        PlayNote(SFX_SP_GO);
        SetPitch(SFX_SP_GO, SND_DEF_PITCH);
    }
    else if (playerShip->updateCount <= AMBER_LIGHT)
    {
        ColourStartBoom(2);
    }
    else if (playerShip->updateCount <= RED_LIGHT)
    {
        ColourStartBoom(1);
    }

    if (playerShip->updateCount == AMBER_LIGHT)
    {
        PlayNote(SFX_SP_1);
        SetPitch(SFX_SP_1, SND_DEF_PITCH);
    }
    else if (playerShip->updateCount == RED_LIGHT)
    {
        PlayNote(SFX_SP_2);
        SetPitch(SFX_SP_2, SND_DEF_PITCH);
    }
    else if (playerShip->updateCount == THREE)
    {
        PlayNote(SFX_SP_3);
        SetPitch(SFX_SP_3, SND_DEF_PITCH);
    }
}

/**************************************************************************/
/****             Start control routine for normal pad                 ****/
/**************************************************************************/

void UpdatePlayerShipNormStart(ShipData* shipIndex, int shipNo, WeaponData* weaponIndex)
{
    ShipData* playerShip;
    static int startPos;

    playerShip = &(shipIndex[shipNo]);

    GeneralStartProcs(playerShip);

    if (playerShip->updateCount == START_TIME - 1)
        startPos = playerShip->pos.vy;

    playerShip->pos.vy = startPos + (rsin(playerShip->updateCount * 80) >> 7);

    if (playerShip->updateCount == 0)
    {
        if (playerShip->attr & RACING)
        {
            playerShip->updateCount = STALL_DELAY;
            playerShip->update = UpdatePlayerShipNorm;
        }
        else
            playerShip->update = UpdateRemoteShip;
    }

    SetBrake(playerShip, pCon.lBrake, pCon.rBrake);

    if (playerShip->attr & RACING)
        SetView(playerShip, pCon.view);

    if (pad & pCon.thrust)
        playerShip->thrust_mag += (FR15U + 1);
    else if (playerShip->thrust_mag > FR15U)
        playerShip->thrust_mag -= (FR15U + 1);

    if (playerShip->thrust_mag > playerShip->max_thrust)
        playerShip->thrust_mag = playerShip->max_thrust;
}

/**************************************************************************/
/****             Race control update routine for normal pad           ****/
/**************************************************************************/
void UpdatePlayerShipNorm(ShipData* shipIndex, int shipNo, WeaponData* weaponIndex)
{
    ShipData* playerShip;
    int headingInc;
    int vpitchInc;
    static int maxThrust;
    static short revLeft = 0;
    static short revRight = 0;

    playerShip = &(shipIndex[shipNo]);

    if (playerShip->electroCount > 0)
        playerShip->electroCount--;

    if (playerShip->electroCount == 0)
        playerShip->attr &= ~ELECTROED;

    if (playerShip->revConCount > 0)
        playerShip->revConCount--;

    if (playerShip->revConCount == 0)
        playerShip->combatAttr &= ~REVCONNED;

    if (playerShip->specialCount > 0)
        playerShip->specialCount--;

    if (playerShip->specialCount == 0)
        playerShip->combatAttr &= ~SPECIALED;

    if ((playerShip->combatAttr & REVCONNED) && (revLeft == 0))
    {
        pCon.left = IN_RIGHT;
        pCon.right = IN_LEFT;
        revLeft = pCon.lBrake;
        revRight = pCon.rBrake;
        pCon.lBrake = revRight;
        pCon.rBrake = revLeft;
    }
    else if (((playerShip->combatAttr & REVCONNED) == 0) && (revLeft != 0))
    {
        pCon.left = IN_LEFT;
        pCon.right = IN_RIGHT;
        pCon.lBrake = revLeft;
        pCon.rBrake = revRight;
        revLeft = 0;
        revRight = 0;
    }

    headingInc = (playerShip->headingInc * gPadStickX * 30 / FR30) >> 8;
    vpitchInc = (gPadStickY * 30 * 30 / FR30) >> 8;

    if (pad & pCon.left) /** Inc = 128 , max 2048 **/
    {
        if (playerShip->vhdg >= 0)
            playerShip->vhdg += headingInc;
        else if (playerShip->vhdg < 0)
            playerShip->vhdg += (headingInc << 1);
    }
    else if (pad & pCon.right)
    {
        if (playerShip->vhdg <= 0)
            playerShip->vhdg -= headingInc;
        else if (playerShip->vhdg > 0)
            playerShip->vhdg -= (headingInc << 1);
    }
    else
    {
        if (playerShip->vhdg > 0)
            playerShip->vhdg -= headingInc;
        else if (playerShip->vhdg < 0)
            playerShip->vhdg += headingInc;
    }
    if (playerShip->vhdg < -playerShip->maxHeading)
        playerShip->vhdg = -playerShip->maxHeading;
    if (playerShip->vhdg > playerShip->maxHeading)
        playerShip->vhdg = playerShip->maxHeading;

    if (playerShip->attr & ELECTROED)
    {
        SetShake(4);
        playerShip->vhdg += (randy() % 600) - 300;
    }

    if (pad & pCon.down)
        playerShip->vpitch += vpitchInc;
    if (pad & pCon.up)
        playerShip->vpitch -= vpitchInc;

    /**** Check for Stall and set thrust ***/
    if (playerShip->updateCount == STALL_DELAY)
    {
        if ((playerShip->thrust_mag >= 680) && (playerShip->thrust_mag <= 700))
        {
            playerShip->thrust_mag = 1800;
            maxThrust = 1800;
        }
        else if (playerShip->thrust_mag < 680)
            maxThrust = playerShip->max_thrust;

        else
            maxThrust = 200;
    }

    if (playerShip->updateCount > 0)
    {
        playerShip->updateCount--;
    }
    else
        maxThrust = playerShip->max_thrust;

    if (maxThrust < 500)
        maxThrust += (randy() % 11) >> 1;

    if (pad & pCon.thrust)
    {
        playerShip->thrust_mag += (FR15U + 1);
    }
    else if (playerShip->thrust_mag > FR7U)
        playerShip->thrust_mag -= (FR7U + 1);

    if (playerShip->thrust_mag > maxThrust)
        playerShip->thrust_mag = maxThrust;

    if ((playerShip->attr & ELECTROED) && ((randy() % 80) == 0))
        playerShip->thrust_mag -= playerShip->thrust_mag >> 2;

    SetBrake(playerShip, pCon.lBrake, pCon.rBrake);
    SetView(playerShip, pCon.view);
    SetFire(playerShip, pCon.fire, weaponIndex, shipNo);
    ShipTrkReaction(playerShip);
}

/***************************************************************/
/***************************************************************/
/******              REMOTE CRAFT CONTROL                  *****/
/***************************************************************/
/***************************************************************/

void UpdateRemoteShipStart(ShipData* shipIndex, int shipNo, WeaponData* weaponIndex)
{
    static startPos;

    shipIndex = &(shipIndex[shipNo]);

    shipIndex->prevShipSect = shipIndex->nearTrkSect;
    SectionSearch(shipIndex);

    if (shipIndex->updateCount == START_TIME)
    {
        startPos = shipIndex->pos.vy;
    }

    shipIndex->pos.vy = startPos + (rsin(shipIndex->updateCount * (80 + shipNo * 3)) >> 7);

    shipIndex->updateCount--;
    if (shipIndex->updateCount == 0)
    {
        shipIndex->update = UpdateRemoteShip;
    }
}

void HoldLeft(VECTOR* result, ShipData* shipIndex, Face* facePtr, int shipNo)
{
    VECTOR faceVert1, faceVert2;

    faceVert1.vx = track->vertices[(facePtr->vertex[1])].vx;
    faceVert1.vy = track->vertices[(facePtr->vertex[1])].vy;
    faceVert1.vz = track->vertices[(facePtr->vertex[1])].vz;

    faceVert2.vx = track->vertices[(facePtr->vertex[0])].vx;
    faceVert2.vy = track->vertices[(facePtr->vertex[0])].vy;
    faceVert2.vz = track->vertices[(facePtr->vertex[0])].vz;

    result->vx = (faceVert1.vx - faceVert2.vx) >> 1;
    result->vy = (faceVert1.vy - faceVert2.vy) >> 1;
    result->vz = (faceVert1.vz - faceVert2.vz) >> 1;
}

void HoldRight(VECTOR* result, ShipData* shipIndex, Face* facePtr, int shipNo)
{
    VECTOR faceVert1, faceVert2;

    faceVert1.vx = track->vertices[(facePtr->vertex[0])].vx;
    faceVert1.vy = track->vertices[(facePtr->vertex[0])].vy;
    faceVert1.vz = track->vertices[(facePtr->vertex[0])].vz;

    faceVert2.vx = track->vertices[(facePtr->vertex[1])].vx;
    faceVert2.vy = track->vertices[(facePtr->vertex[1])].vy;
    faceVert2.vz = track->vertices[(facePtr->vertex[1])].vz;

    result->vx = (faceVert1.vx - faceVert2.vx) >> 1;
    result->vy = (faceVert1.vy - faceVert2.vy) >> 1;
    result->vz = (faceVert1.vz - faceVert2.vz) >> 1;
}

void HoldCentre(VECTOR* result, ShipData* shipIndex, Face* facePtr, int shipNo)
{
    result->vx = 0;
    result->vy = 0;
    result->vz = 0;
}

void BlockShip(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int shipNo)
{
    if (shipIndex[ownShip].attr & LSIDE)
        HoldLeft(blockVector, shipIndex, facePtr, shipNo);
    else
        HoldRight(blockVector, shipIndex, facePtr, shipNo);
}

void AvoidShip(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int shipNo)
{
    if (shipIndex[ownShip].attr & LSIDE)
        HoldRight(blockVector, shipIndex, facePtr, shipNo);
    else
        HoldLeft(blockVector, shipIndex, facePtr, shipNo);
}

void AvoidOtherShips(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int shipNo)
{
    int i;
    short DPASectDiff; /** Change to run from sect not DPA sect **/
    short minDPASec = 100;
    int shipID = -1;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i != shipNo)
        {
            DPASectDiff = shipIndex[i].DPASectDiff - shipIndex[shipNo].DPASectDiff;
            if (minDPASec < DPASectDiff)
            {
                minDPASec = DPASectDiff;
                shipID = i;
            }
        }
    }

    if (shipID == -1)
        return;

    if ((minDPASec < 10) && (minDPASec > -2))
    {
        if (shipIndex[shipID].attr & LSIDE)
            HoldRight(blockVector, shipIndex, facePtr, shipNo);
        else
            HoldLeft(blockVector, shipIndex, facePtr, shipNo);
    }
}

void ZigZagShip(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int shipNo)
{
    if ((shipIndex->updateCount % 50) == 0)
    {
        if (((shipIndex->updateCount / 50) % 2) == 0)
            HoldLeft(blockVector, shipIndex, facePtr, shipNo);
        else
            HoldRight(blockVector, shipIndex, facePtr, shipNo);
    }
}

void UpdateRemoteShip(ShipData* shipIndex, int shipNo, WeaponData* weaponIndex)
{
    ShipData *myShip, *remoteShip;
    Face* facePtr = NULL;
    TrackSection *nextSection, *section;
    int i;
    VECTOR targetVector, bestPath;
    const VECTOR* facePoint;
    int temp, alpha;
    VECTOR offSetVector;
    Vector path1, path2;
    int gap_mag, decide;
    short DPASectDiff;

#ifdef __3DO__
    return;
#endif

    offSetVector.vx = 0;
    offSetVector.vy = 0;
    offSetVector.vz = 0;

    myShip = &(shipIndex[ownShip]);
    remoteShip = &(shipIndex[shipNo]);

    if (remoteShip->electroCount > 0)
        remoteShip->electroCount--;
    if (remoteShip->electroCount == 0)
        remoteShip->attr &= ~ELECTROED;

    if ((remoteShip->attr & FLYING) == 0)
    {
        /*** Find First track base section ***/
        facePtr = track->faces + remoteShip->nearTrkSect->faceStart;
        while ((facePtr->flags & TRACK_BASE) == 0)
        {
            facePtr++;
        }

        DPASectDiff = remoteShip->DPASectDiff - myShip->DPASectDiff;

        remoteShip->attr &= ~JUST_IN_FRONT;

        if (remoteShip == myShip)
        {

            remoteShip->updateControl = AvoidOtherShips;
            if (((remoteShip->remoteMaxThrust) > remoteShip->speed))
                remoteShip->speed += remoteShip->remoteThrustMag;
        }
        else
        {

            /************************************************************************/
            /**** Make global DPA decisions , these will effect the craft in     ****/
            /**** relation to your race position                                 ****/
            /************************************************************************/

            /************************************************************************/
            /**** Accelerate remote ships away at start, numtospeed set in       ****/
            /**** InitShipData and is an exponential progression                 ****/
            /************************************************************************/

            if (remoteShip->numtospeed > 0)
            {
                remoteShip->numtospeed--;
                remoteShip->updateCount = 0;
                remoteShip->updateControl = AvoidShip;
                if ((remoteShip->remoteMaxThrust + 1200) > remoteShip->speed)
                    remoteShip->speed += (remoteShip->remoteThrustMag + 150);
            }

            /************************************************************************/
            /**** Ship has been left WELL BEHIND therefore set it to avoid       ****/
            /**** other ships and update its speed as normal                     ****/
            /************************************************************************/
            else if (DPASectDiff < -10) /*** Ship behind, AVOID ***/
            {
                remoteShip->updateCount = 0;
                remoteShip->updateControl = AvoidShip;

                /***** If ship has been well passed, increase its speed to allow *****/
                /***** it to make a challenge when the player fouls up           *****/

                if (((remoteShip->remoteMaxThrust + behindSpeed[trackNo]) > remoteShip->speed))
                {
                    remoteShip->speed += remoteShip->remoteThrustMag;
                }
            }

            /************************************************************************/
            /**** Ship is JUST AHEAD therefore                                   ****/
            /****                                                                ****/
            /************************************************************************/

            else if ((DPASectDiff <= 4) && (DPASectDiff > 0)) /***** Ship close by, beware does not account for lapped opponents yet *****/
            {
                remoteShip->attr |= JUST_IN_FRONT;

                if (remoteShip->updateCount == 0) /*** Make New Decision ***/
                {
                    decide = randy() % 64; /* 12 */

                    remoteShip->updateCount = JUST_FRONT_DELAY;
                    if (remoteShip->fightBack) /**** Ship wants to make life difficult ****/
                    {
                        if ((decide < 40) || (remoteShip->weaponType == NO_WEAPON)) /*** Ship will try to block you ***/
                        {
                            remoteShip->updateControl = BlockShip;
                        }
                        else if ((decide >= 40) && (decide < 52)) /*** Ship will attempt to drop mines in your path ***/
                        {
                            remoteShip->updateControl = BlockShip;
                            if (((remoteShip->attr & SHIELDED) == 0) && (remoteShip->attr & RACING))
                            {
                                PlayNote(SFX_SP_MINES);
                                SetPitch(SFX_SP_MINES, SND_DEF_PITCH);

                                remoteShip->weaponType = MINE;
                                FireWeapon(remoteShip, weaponIndex, shipNo, remoteShip->weaponType);
                            }
                        }
                        else if ((decide >= 52) && (decide < 64)) /*** Ship will raise its shield ***/
                        {
                            remoteShip->updateControl = BlockShip;
                            if ((remoteShip->attr & SHIELDED) == 0)
                            {
                                remoteShip->weaponType = SHIELD;
                                FireWeapon(remoteShip, weaponIndex, shipNo, remoteShip->weaponType);
                            }
                        }
                    }
                    else /**** Let the first ships be easy to pass ****/
                    {
                        remoteShip->updateControl = AvoidShip;
                    }
                }

                remoteShip->updateCount--;

                if (remoteShip->attr & OVERTAKE)
                {
                    /*** If ship has just overtaken, slow it down to a reasonable speed ***/
                    if ((remoteShip->remoteMaxThrust + behindSpeed[trackNo]) > remoteShip->speed)
                        remoteShip->speed += (remoteShip->remoteThrustMag);
                }
                else
                {
                    /*** Increase the speed of any craft just in front slightly ***/
                    if (((remoteShip->remoteMaxThrust + (behindSpeed[trackNo] >> 1)) > remoteShip->speed))
                        remoteShip->speed += remoteShip->remoteThrustMag;
                }
            }

            /************************************************************************/
            /**** Ship is JUST BEHIND therefore we must decided if and how many  ****/
            /**** times it 'should have a go back'                               ****/
            /************************************************************************/

            else if ((DPASectDiff >= -10) && (DPASectDiff <= 0)) /*** Ship just behind, MAKE DECISION ***/
            {
                if (remoteShip->updateCount == 0) /*** Make New Decision ***/
                {
                    remoteShip->updateCount = JUST_BEHIND_DELAY;

                    if (remoteShip->fightBack) /**** Ship wants you to say "Outside Now!" ****/
                    {
                        if (remoteShip->weaponType == NO_WEAPON)
                        {
                            remoteShip->updateControl = AvoidShip;
                            remoteShip->attr |= OVERTAKE;
                        }
                        else
                        {
                            decide = randy() % 64;

                            if (decide < 48)
                            {
                                remoteShip->updateControl = BlockShip;
                            }
                            else if ((decide >= 40) && (decide < 54))
                            {
                                remoteShip->updateControl = AvoidShip;
                                remoteShip->attr &= ~OVERTAKE;
                                if (((remoteShip->attr & SHIELDED) == 0) && (remoteShip->attr & RACING))
                                {
                                    PlayNote(SFX_SP_ROCKET);
                                    SetPitch(SFX_SP_ROCKET, SND_DEF_PITCH);

                                    remoteShip->weaponType = ROCKET;
                                    FireWeapon(remoteShip, weaponIndex, shipNo, remoteShip->weaponType);
                                }
                            }
                            else if ((decide >= 54) && (decide < 60))
                            {
                                remoteShip->updateControl = AvoidShip;
                                remoteShip->attr &= ~OVERTAKE;
                                if (((remoteShip->attr & SHIELDED) == 0) && (remoteShip->attr & RACING))
                                {
                                    PlayNote(SFX_SP_MISSILE);
                                    SetPitch(SFX_SP_MISSILE, SND_DEF_PITCH);

                                    remoteShip->weaponType = MISSILE;
                                    remoteShip->targetShip = ownShip;
                                    FireWeapon(remoteShip, weaponIndex, shipNo, remoteShip->weaponType);
                                }
                            }
                            else if ((decide >= 60) && (decide < 64))
                            {
                                remoteShip->updateControl = AvoidShip;
                                remoteShip->attr &= ~OVERTAKE;
                                if (((remoteShip->attr & SHIELDED) == 0) && (remoteShip->attr & RACING))
                                {
                                    PlayNote(SFX_SP_SHOCKWAVE);
                                    SetPitch(SFX_SP_SHOCKWAVE, SND_DEF_PITCH);

                                    remoteShip->weaponType = ELEC_BOLT;
                                    remoteShip->targetShip = ownShip;
                                    FireWeapon(remoteShip, weaponIndex, shipNo, remoteShip->weaponType);
                                }
                            }
                        }
                    }
                    else /**** If ship destined to be tail-ender then slow down ****/
                    {
                        remoteShip->remoteMaxThrust = 2100;
                        remoteShip->remoteThrustMag = 25;
                        remoteShip->speed = 2100;
                        remoteShip->updateControl = AvoidShip;
                        remoteShip->attr &= ~OVERTAKE;
                    }
                }

                for (i = 0; i < NO_OF_SHIPS; i++) /**** If another ship is just in front pass fight on ****/
                {
                    if (shipIndex[i].attr & JUST_IN_FRONT)
                    {
                        remoteShip->updateControl = AvoidShip;
                        remoteShip->attr &= ~OVERTAKE;
                    }
                }

                remoteShip->updateCount--;

                if (remoteShip->attr & OVERTAKE)
                {
                    if ((remoteShip->remoteMaxThrust + 700) > remoteShip->speed)
                        remoteShip->speed += (remoteShip->remoteThrustMag << 1);
                }
                else
                {
                    if (((remoteShip->remoteMaxThrust + behindSpeed[trackNo]) > remoteShip->speed))
                        remoteShip->speed += remoteShip->remoteThrustMag;
                }
            }

            /************************************************************************/
            /**** Ship is WELL AHEAD therefore we must slow the opponent to      ****/
            /**** give the weaker player a chance to catch up                    ****/
            /************************************************************************/

            else if ((DPASectDiff > (shipOrder[shipNo] * 15)) && (DPASectDiff < 150)) /*** Ship well ahead, SLOW DOWN ***/
            {
                remoteShip->speed += (remoteShip->remoteThrustMag >> 1);
                if (remoteShip->speed > (remoteShip->remoteMaxThrust >> 1))
                    remoteShip->speed = (remoteShip->remoteMaxThrust >> 1);

                remoteShip->updateCount = 0;
                remoteShip->updateControl = HoldCentre;
            }

            /************************************************************************/
            /**** Ship is TOO FAR AHEAD therefore                                ****/
            /****                                                                ****/
            /************************************************************************/

            else if (DPASectDiff >= 150) /*** Ship too far ahead, let it continue ***/
            {
                remoteShip->updateCount = 0;
                remoteShip->updateControl = AvoidShip;

                if ((remoteShip->remoteMaxThrust > remoteShip->speed))
                {
                    remoteShip->speed += remoteShip->remoteThrustMag;
                }
            }

            /************************************************************************/
            /**** Ship is IN SIGHT therefore                                  ****/
            /****                                                                ****/
            /************************************************************************/

            else if ((DPASectDiff <= 10) && (DPASectDiff > 4)) /***** Ship close by, beware does not account for lapped opponents yet *****/
            {
                if (remoteShip->updateCount == 0) /*** Make New Decision ***/
                {
                    decide = randy() % 5;

                    remoteShip->updateCount = IN_SIGHT_DELAY;
                    switch (decide)
                    {
                        case 0:
                            remoteShip->updateControl = HoldCentre;
                            break;

                        case 1:
                            remoteShip->updateControl = HoldLeft;
                            break;

                        case 2:
                            remoteShip->updateControl = HoldRight;
                            break;

                        case 3:
                            remoteShip->updateControl = BlockShip;
                            break;

                        case 4:
                            remoteShip->updateControl = ZigZagShip;
                            break;
                    }
                }

                remoteShip->updateCount--;

                if ((remoteShip->remoteMaxThrust > remoteShip->speed))
                {
                    remoteShip->speed += remoteShip->remoteThrustMag;
                }
            } /***** End of DPA control options *****/

            /************************************************************************/
            /**** Ship is JUST OUT OF SIGHT therefore                            ****/
            /****                                                                ****/
            /************************************************************************/

            else
            {
                remoteShip->updateCount = 0;
                remoteShip->updateControl = HoldCentre;
                if ((remoteShip->remoteMaxThrust > remoteShip->speed))
                {
                    remoteShip->speed += remoteShip->remoteThrustMag;
                }
            }
        }

        (remoteShip->updateControl)(&offSetVector, shipIndex, facePtr, shipNo);

        /***************************************************************************/
        /***************************************************************************/
        /***************************************************************************/
        /***************************************************************************/

        /************************************************************************/
        /**** Make decision as to which path the craft will take at a        ****/
        /**** junction                                                       ****/
        /************************************************************************/

        section = remoteShip->nearTrkSect->prevSection.ptr;

        for (i = 0; i < 4; i++)
        {
            section = section->nextSection.ptr;
        }

        if (section->junction.index != -1)
        {
            if (section->junction.ptr->flags & JunctionStart)
            {
                decide = randy() % 2;
                if (decide == 0)
                    remoteShip->attr |= JUNC_LEFT;
                else
                    remoteShip->attr &= ~JUNC_LEFT;
            }
        }

        section = remoteShip->nearTrkSect->prevSection.ptr;

        for (i = 0; i < 4; i++)
        {
            if (section->junction.index != -1)
            {
                if (section->junction.ptr->flags & JunctionStart)
                {
                    if (remoteShip->attr & JUNC_LEFT)
                    {
                        section = section->junction.ptr;
                    }
                    else
                        section = section->nextSection.ptr;
                }
                else
                    section = section->nextSection.ptr;
            }
            else
                section = section->nextSection.ptr;
        }
        nextSection = section->nextSection.ptr;
        section = remoteShip->nearTrkSect;

        /***************************************************************************/
        /****                General routines - Non decision based              ****/
        /***************************************************************************/

        /***** Bleed off speed as orientation changes *****/

        remoteShip->speed -= (abs(remoteShip->speed * ((remoteShip->vhdg * FR60) / 60))) >> 14; /* 14 */
        remoteShip->speed -= (abs(remoteShip->speed * ((remoteShip->vpitch * FR60) / 60))) >> 14;

        if (remoteShip->nearTrkSect->flags & Jump) /**** Cure for ships getting stuck on hump lip ****/
        {
            targetVector.vx = remoteShip->nearTrkSect->centre.vx - remoteShip->nearTrkSect->prevSection.ptr->centre.vx;
            targetVector.vy = remoteShip->nearTrkSect->centre.vy - remoteShip->nearTrkSect->prevSection.ptr->centre.vy;
            targetVector.vz = remoteShip->nearTrkSect->centre.vz - remoteShip->nearTrkSect->prevSection.ptr->centre.vz;
        }
        else
        {
            targetVector.vx = nextSection->centre.vx - section->centre.vx;
            targetVector.vy = nextSection->centre.vy - section->centre.vy;
            targetVector.vz = nextSection->centre.vz - section->centre.vz;
        }

        gap_mag = GetMagnitude(&targetVector);

        targetVector.vx = (targetVector.vx * remoteShip->speed) / gap_mag;
        targetVector.vy = (targetVector.vy * remoteShip->speed) / gap_mag;
        targetVector.vz = (targetVector.vz * remoteShip->speed) / gap_mag;

        path1.vx = section->centre.vx + offSetVector.vx;
        path1.vy = section->centre.vy + offSetVector.vy;
        path1.vz = section->centre.vz + offSetVector.vz;

        path2.vx = nextSection->centre.vx + offSetVector.vx;
        path2.vy = nextSection->centre.vy + offSetVector.vy;
        path2.vz = nextSection->centre.vz + offSetVector.vz;

        GetTargetPos(&bestPath, &remoteShip->pos, &path2, &path1);

        remoteShip->acc.vx = (targetVector.vx + ((bestPath.vx - remoteShip->pos.vx) >> 1));
        remoteShip->acc.vy = (targetVector.vy + ((bestPath.vy - remoteShip->pos.vy) >> 1));
        remoteShip->acc.vz = (targetVector.vz + ((bestPath.vz - remoteShip->pos.vz) >> 1));

        remoteShip->acc.vx = (remoteShip->acc.vx * 60) / FR60;
        remoteShip->acc.vy = (remoteShip->acc.vy * 60) / FR60;
        remoteShip->acc.vz = (remoteShip->acc.vz * 60) / FR60;

        remoteShip->vel.vx += remoteShip->acc.vx;
        remoteShip->vel.vy += remoteShip->acc.vy;
        remoteShip->vel.vz += remoteShip->acc.vz;
        /***** If remote has gone over speed-up *****/

        /***** Can use control routine to determine position now *****/
        /***** should use actual position finally                *****/

        if ((facePtr->flags & SPEED_UP) && ((remoteShip->updateControl == HoldLeft) || (remoteShip->updateControl == HoldCentre)))
        {
            remoteShip->speed += 200;
        }
        facePtr++;

        if ((facePtr->flags & SPEED_UP) && ((remoteShip->updateControl == HoldRight) || (remoteShip->updateControl == HoldCentre)))
        {
            remoteShip->speed += 200;
        }

        /****** Speed Test, should not make much difference ******/
        /****** to overall lap times of opposition          ******/
        facePoint = track->vertices + facePtr->vertex[0];
        alpha = PlaneLineCollision(facePoint, &remoteShip->pos, &facePtr->normal);

        if (alpha < 50)
            alpha = 50;

    #if 0
        alpha = TRACK_MAGNET * (TARGETHEIGHT - alpha) / alpha;
        remoteShip->acc.vx = (facePtr->normal.vx * alpha) >> 8;
        remoteShip->acc.vy = (facePtr->normal.vy * alpha) >> 8;
        remoteShip->acc.vz = (facePtr->normal.vz * alpha) >> 8;
    #else
        remoteShip->acc.vx = (((facePtr->normal.vx * TARGETHEIGHT * TRACK_MAGNET) / alpha) - (facePtr->normal.vx * TRACK_MAGNET)) >> 8;
        remoteShip->acc.vy = (((facePtr->normal.vy * TARGETHEIGHT * TRACK_MAGNET) / alpha) - (facePtr->normal.vy * TRACK_MAGNET)) >> 8;
        remoteShip->acc.vz = (((facePtr->normal.vz * TARGETHEIGHT * TRACK_MAGNET) / alpha) - (facePtr->normal.vz * TRACK_MAGNET)) >> 8;
    #endif

        remoteShip->vel.vx += remoteShip->acc.vx;
        remoteShip->vel.vy += remoteShip->acc.vy;
        remoteShip->vel.vz += remoteShip->acc.vz;

        temp = SquareRoot0(targetVector.vx * targetVector.vx + targetVector.vz * targetVector.vz);

        remoteShip->vhdg = -ratan2(targetVector.vx, targetVector.vz) - remoteShip->hdg;
        remoteShip->vhdg += remoteShip->ahdg; /*** Result of weapon collision ***/
        remoteShip->ahdg -= remoteShip->ahdg >> 3;

        if (remoteShip->vhdg > 2048)
            remoteShip->vhdg = remoteShip->vhdg - 4096;
        else if (remoteShip->vhdg < -2048)
            remoteShip->vhdg = remoteShip->vhdg + 4096;

        remoteShip->vhdg = (remoteShip->vhdg * 60) / FR60;
        remoteShip->hdg += remoteShip->vhdg >> 4;

        remoteShip->vpitch = -ratan2(targetVector.vy, temp) - remoteShip->pitch;
        remoteShip->vpitch = (remoteShip->vpitch * 60) / FR60;

        remoteShip->pitch += remoteShip->vpitch >> 4;
        remoteShip->vroll += sar(remoteShip->vhdg, 2);
        remoteShip->vroll -= remoteShip->vroll >> 1;
        remoteShip->roll += remoteShip->vroll;
        remoteShip->roll -= remoteShip->roll >> 3;
    }

    /************************************************************************/
    /**** Ship is FLYING therefore                                       ****/
    /****                                                                ****/
    /************************************************************************/

    else
    {
        section = remoteShip->nearTrkSect;
        section = section->nextSection.ptr;
        section = section->nextSection.ptr;
        nextSection = section->nextSection.ptr;

        remoteShip->updateControl = HoldCentre;
        (remoteShip->updateControl)(&offSetVector, myShip, facePtr, shipNo);

        if (remoteShip->remoteMaxThrust > remoteShip->speed)
        {
            remoteShip->speed += remoteShip->remoteThrustMag;
        }

        remoteShip->speed -= (abs(remoteShip->speed * remoteShip->vhdg)) >> 14;

        targetVector.vx = nextSection->centre.vx - section->centre.vx;
        targetVector.vy = nextSection->centre.vy - section->centre.vy;
        targetVector.vz = nextSection->centre.vz - section->centre.vz;

        gap_mag = GetMagnitude(&targetVector);

        targetVector.vx = (targetVector.vx * remoteShip->speed) / gap_mag;
        targetVector.vz = (targetVector.vz * remoteShip->speed) / gap_mag;

        targetVector.vy = 500;

        GetTargetPos(&bestPath, &remoteShip->pos, &nextSection->centre, &remoteShip->nearTrkSect->centre);

        remoteShip->acc.vx = (targetVector.vx + ((bestPath.vx - remoteShip->pos.vx) >> 1));
        remoteShip->acc.vy = targetVector.vy;
        remoteShip->acc.vz = (targetVector.vz + ((bestPath.vz - remoteShip->pos.vz) >> 1));

        remoteShip->acc.vx = (remoteShip->acc.vx * 60) / FR60;
        remoteShip->acc.vy = (remoteShip->acc.vy * 60) / FR60;
        remoteShip->acc.vz = (remoteShip->acc.vz * 60) / FR60;

        remoteShip->vel.vx += remoteShip->acc.vx;
        remoteShip->vel.vy += remoteShip->acc.vy;
        remoteShip->vel.vz += remoteShip->acc.vz;
        remoteShip->vpitch = -200 - remoteShip->pitch;
        remoteShip->pitch += remoteShip->vpitch >> 4;
        remoteShip->vhdg = -ratan2(targetVector.vx, targetVector.vz) - remoteShip->hdg;

        if (remoteShip->vhdg > 2048)
            remoteShip->vhdg = remoteShip->vhdg - 4096;
        else if (remoteShip->vhdg < -2048)
            remoteShip->vhdg = remoteShip->vhdg + 4096;

        remoteShip->hdg += remoteShip->vhdg >> 4;
    }

    remoteShip->vel.vx -= remoteShip->vel.vx >> 3;
    remoteShip->vel.vy -= remoteShip->vel.vy >> 3;
    remoteShip->vel.vz -= remoteShip->vel.vz >> 3;

    remoteShip->pos.vx += remoteShip->vel.vx >> 6;
    remoteShip->pos.vy += remoteShip->vel.vy >> 6;
    remoteShip->pos.vz += remoteShip->vel.vz >> 6;

    if (remoteShip->attr & ELECTROED)
    {
        remoteShip->pos.vx += (randy() - 16384) % 20;
        remoteShip->pos.vy += (randy() - 16384) % 20;
        remoteShip->pos.vz += (randy() - 16384) % 20;

        if ((randy() % 50) == 0)
            remoteShip->speed -= remoteShip->speed >> 1;
    }

    remoteShip->hdg = ang(remoteShip->hdg);
    remoteShip->pitch = ang(remoteShip->pitch);
    remoteShip->roll = ang(remoteShip->roll);
}

void SetSkeletonDirectionHPR(Skeleton* skeleton, short hdg, short pitch, short roll)
{
    short sx, sy, sz;
    short cx, cy, cz;

    sy = rsin(hdg);
    sx = rsin(pitch);
    sz = rsin(roll);

    cy = rcos(hdg);
    cx = rcos(pitch);
    cz = rcos(roll);

    skeleton->relative.m[0][0] = ((cy * cz) >> 12) + ((((sy * sz) >> 12) * sx) >> 12);
    skeleton->relative.m[0][1] = ((cy * sz) >> 12) - ((((sy * sx) >> 12) * cz) >> 12);
    skeleton->relative.m[0][2] = (-sy * cx) >> 12;

    skeleton->relative.m[1][0] = -(sz * cx) >> 12;
    skeleton->relative.m[1][1] = (cx * cz) >> 12;
    skeleton->relative.m[1][2] = -sx;

    skeleton->relative.m[2][0] = ((sy * cz) >> 12) - ((((cy * sx) >> 12) * sz) >> 12);
    skeleton->relative.m[2][1] = ((sy * sz) >> 12) + ((((cy * sx) >> 12) * cz) >> 12);
    skeleton->relative.m[2][2] = ((cx * cy) >> 12);

    /* Mark skeleton for update */

    skeleton->update = 1;
}

void SetSkeletonDirectionRPH(Skeleton* skeleton, short roll, short pitch, short hdg)
{
    short sx, sy, sz;
    short cx, cy, cz;

    sy = rsin(hdg);
    sx = rsin(pitch);
    sz = rsin(roll);

    cy = rcos(hdg);
    cx = rcos(pitch);
    cz = rcos(roll);

    skeleton->relative.m[0][0] = ((cy * cz) >> 12) - ((((sz * sx) >> 12) * sy) >> 12);
    skeleton->relative.m[0][1] = (sz * cx) >> 12;
    skeleton->relative.m[0][2] = (-(sy * cz) >> 12) - ((((sz * sx) >> 12) * cy) >> 12);

    skeleton->relative.m[1][0] = (-(sz * cy) >> 12) - ((((sx * cz) >> 12) * sy) >> 12);
    skeleton->relative.m[1][1] = (cx * cz) >> 12;
    skeleton->relative.m[1][2] = ((sz * sy) >> 12) - ((((sx * cz) >> 12) * cy) >> 12);

    skeleton->relative.m[2][0] = (cx * sy) >> 12;
    skeleton->relative.m[2][1] = sx;
    skeleton->relative.m[2][2] = (cx * cy) >> 12;

    /* Mark skeleton for update */

    skeleton->update = 1;
}
