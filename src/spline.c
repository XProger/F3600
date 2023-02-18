#include "game.h"

/* the spline position we are currently on */
short camerasplineCount = 0;
short shipsplineCount = 0;

/* the spline interpolated notch we are currently on */
short camerastepCount = 0;
short shipstepCount = 0;

/* changeable camera coords on high up camera views */
short camVx = -9900;
short camVy = 9100;
short camVz = 6900;

/* the timeout period high camera views */
short timeOut;

/* which camera - ship spline path to use */
short splineNumber;

short startIceText;
short startFireText;
short textY;

#define Red 0
#define White 1

typedef struct
{
    uint16 str;
    uint8 size;
    uint8 colour;
} Congrats;

const Congrats* con;

const Congrats gRepierCongrats[] = 
{
    { STR_REPIER_COMP_1  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_2  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_3  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_4  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_5  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_6  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_7  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_8  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_9  , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_10 , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_11 , 8, White },
    { STR_EMPTY          , 8, White },
    { STR_REPIER_COMP_12 , 8, White },
    { 0, 0, 0 }
};

const Congrats gGameCongrats[] = 
{
    { STR_GAME_COMP_1  ,  8, Red   },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_2  ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_3  ,  8, Red   },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_4  ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_5  ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_6  , 16, Red   },
    { STR_EMPTY        ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_7  ,  8, Red   },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_8  ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_9  ,  8, White },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_10 ,  8, Red   },
    { STR_EMPTY        ,  8, White },
    { STR_GAME_COMP_11 ,  8, White },
    { 0, 0, 0 }
};

#define GlobalDistance 32767

TrackSection* FindAllNearestSection(const vec3i* pos)
{
    TrackSection* nearSection = NULL;
    TrackSection* trk;
    vec3i distance;
    int32 smallestMag = 0x7fffffff; // pretty big // PCwipeout
    int32 mag;
    int32 i;

    trk = gTrack.sections;

    for (i = 0; i < gTrack.sectionCount; i++)
    {
        distance.x = pos->x - trk->centre.x;
        distance.y = pos->y - trk->centre.y;
        distance.z = pos->z - trk->centre.z;

        if ((abs(distance.x) < GlobalDistance) && (abs(distance.y) < GlobalDistance) && (abs(distance.z) < GlobalDistance))
        {
            mag = (rsqrt((distance.x * distance.x) + (distance.y * distance.y) + (distance.z * distance.z)));

            if ((mag < GlobalDistance) && (mag < smallestMag))
            {
                smallestMag = mag;
                nearSection = trk;
            }
        }

        trk = NextIndexSection(trk);
    }

    return (nearSection);
}

void UpdateShipRemote(ShipData* shipIndex, int shipNo)
{
    ShipData *playerShip, *remoteShip;
    Face* facePtr;
    TrackSection *nextSection, *section;
    int i;
    vec3i targetVector, bestPath;
    const vec3i* facePoint;
    int temp, alpha;
    vec3i offSetVector;
    vec3i path1, path2;
    vec3i force;
    int gap_mag, decide;
    short DPASectDiff;

    offSetVector.x = 0;
    offSetVector.y = 0;
    offSetVector.z = 0;

    playerShip = shipIndex + ownShip;
    remoteShip = shipIndex + shipNo;

    facePtr = remoteShip->face;

    if ((remoteShip->attr & FLYING) == 0)
    {
        /*** Find First track base section ***/
        DPASectDiff = remoteShip->DPASectDiff - playerShip->DPASectDiff;

        remoteShip->updateControl = HoldCentre;

        (remoteShip->updateControl)(&offSetVector, shipIndex, facePtr, shipNo);

        if ((remoteShip->remoteMaxThrust > remoteShip->speed))
        {
            remoteShip->speed += remoteShip->remoteThrustMag;
        }

        /***************************************************************************/
        /***************************************************************************/
        /***************************************************************************/
        /***************************************************************************/

        /************************************************************************/
        /**** Make decision as to which path the craft will take at a        ****/
        /**** junction                                                       ****/
        /************************************************************************/

        section = remoteShip->nearTrkSect->prevSection;

        for (i = 0; i < 4; i++)
        {
            section = section->nextSection;
        }

        if (section->junction)
        {
            if (section->junction->flags & TRACK_JUNCTION_START)
            {
                decide = randy() % 2;
                if (decide == 0)
                    remoteShip->attr |= JUNC_LEFT;
                else
                    remoteShip->attr &= ~JUNC_LEFT;

                /* rob */
                remoteShip->attr &= ~JUNC_LEFT;
            }
        }

        section = remoteShip->nearTrkSect->prevSection;

        for (i = 0; i < 4; i++)
        {
            if (section->junction)
            {
                if (section->junction->flags & TRACK_JUNCTION_START)
                {
                    if (remoteShip->attr & JUNC_LEFT)
                    {
                        section = section->junction;
                    }
                    else
                        section = section->nextSection;
                }
                else
                    section = section->nextSection;
            }
            else
                section = section->nextSection;
        }
        nextSection = section->nextSection;
        section = remoteShip->nearTrkSect;

        /***************************************************************************/
        /****                General routines - Non decision based              ****/
        /***************************************************************************/

        /***** Bleed off speed as orientation changes *****/
#if 0
      remoteShip->speed -= (abs(remoteShip->speed * remoteShip->vhdg))>>14 ;    /* 14 */
      remoteShip->speed -= (abs(remoteShip->speed * remoteShip->vpitch))>>14 ;
#endif
        if (remoteShip->nearTrkSect->flags & TRACK_JUMP) /**** Cure for ships getting stuck on hump lip ****/
        {
            targetVector.x = remoteShip->nearTrkSect->centre.x - remoteShip->nearTrkSect->prevSection->centre.x;
            targetVector.y = remoteShip->nearTrkSect->centre.y - remoteShip->nearTrkSect->prevSection->centre.y;
            targetVector.z = remoteShip->nearTrkSect->centre.z - remoteShip->nearTrkSect->prevSection->centre.z;
        }
        else
        {
            targetVector.x = nextSection->centre.x - section->centre.x;
            targetVector.y = nextSection->centre.y - section->centre.y;
            targetVector.z = nextSection->centre.z - section->centre.z;
        }

        gap_mag = GetMagnitude(&targetVector);

        targetVector.x = (targetVector.x * remoteShip->speed) / gap_mag;
        targetVector.y = (targetVector.y * remoteShip->speed) / gap_mag;
        targetVector.z = (targetVector.z * remoteShip->speed) / gap_mag;

        path1.x = section->centre.x + offSetVector.x;
        path1.y = section->centre.y + offSetVector.y;
        path1.z = section->centre.z + offSetVector.z;

        path2.x = nextSection->centre.x + offSetVector.x;
        path2.y = nextSection->centre.y + offSetVector.y;
        path2.z = nextSection->centre.z + offSetVector.z;

        GetTargetPos(&bestPath, &remoteShip->pos, &path2, &path1);

        remoteShip->vel.x += targetVector.x + ((bestPath.x - remoteShip->pos.x) >> 1);
        remoteShip->vel.y += targetVector.y + ((bestPath.y - remoteShip->pos.y) >> 1);
        remoteShip->vel.z += targetVector.z + ((bestPath.z - remoteShip->pos.z) >> 1);
        /***** If remote has gone over speed-up *****/

        /****** Speed Test, should not make much difference ******/
        /****** to overall lap times of opposition          ******/
        facePoint = gTrack.vertices + facePtr->vertex[0];

        alpha = PlaneLineCollision(facePoint, &remoteShip->pos, &facePtr->normal);

        ApplyMagnet(&force, &facePtr->normal, alpha);

        remoteShip->vel.x += force.x;
        remoteShip->vel.y += force.y;
        remoteShip->vel.z += force.z;

        temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

        remoteShip->vhdg = -ratan2(targetVector.x, targetVector.z) - remoteShip->hdg;
        remoteShip->vhdg += remoteShip->ahdg; /*** Result of weapon collision ***/
        remoteShip->ahdg -= remoteShip->ahdg >> 3;

        if (remoteShip->vhdg > 2048)
            remoteShip->vhdg = remoteShip->vhdg - 4096;
        else if (remoteShip->vhdg < -2048)
            remoteShip->vhdg = remoteShip->vhdg + 4096;

        remoteShip->hdg += remoteShip->vhdg >> 4;

        remoteShip->vpitch = -ratan2(targetVector.y, temp) - remoteShip->pitch;

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
        section = section->nextSection;
        section = section->nextSection;
        nextSection = section->nextSection;

        remoteShip->updateControl = HoldCentre;
        (remoteShip->updateControl)(&offSetVector, playerShip, remoteShip->face, shipNo);

        if (remoteShip->remoteMaxThrust > remoteShip->speed)
        {
            remoteShip->speed += remoteShip->remoteThrustMag;
        }

        remoteShip->speed -= (abs(remoteShip->speed * remoteShip->vhdg)) >> 14;
        /*    remoteShip->speed -= (abs(remoteShip->speed * remoteShip->vpitch))>>14 ;
         */

        targetVector.x = nextSection->centre.x - section->centre.x;
        targetVector.y = nextSection->centre.y - section->centre.y;
        targetVector.z = nextSection->centre.z - section->centre.z;

        gap_mag = GetMagnitude(&targetVector);

        targetVector.x = (targetVector.x * remoteShip->speed) / gap_mag;
        targetVector.z = (targetVector.z * remoteShip->speed) / gap_mag;

        GetTargetPos(&bestPath, &remoteShip->pos, &nextSection->centre, &remoteShip->nearTrkSect->centre);

        // accelerate
        remoteShip->vel.x += targetVector.x + ((bestPath.x - remoteShip->pos.x) >> 1);
        remoteShip->vel.y += 500; // const
        remoteShip->vel.z += targetVector.z + ((bestPath.z - remoteShip->pos.z) >> 1);

        remoteShip->vpitch = -200 - remoteShip->pitch;

        remoteShip->pitch += remoteShip->vpitch >> 4;

        remoteShip->vhdg = -ratan2(targetVector.x, targetVector.z) - remoteShip->hdg;
        if (remoteShip->vhdg > 2048)
            remoteShip->vhdg = remoteShip->vhdg - 4096;
        else if (remoteShip->vhdg < -2048)
            remoteShip->vhdg = remoteShip->vhdg + 4096;
        remoteShip->hdg += remoteShip->vhdg >> 4;
    }

    remoteShip->vel.x -= remoteShip->vel.x >> 3;
    remoteShip->vel.y -= remoteShip->vel.y >> 3;
    remoteShip->vel.z -= remoteShip->vel.z >> 3;

    remoteShip->pos.x += remoteShip->vel.x >> VELOCITY_SHIFT;
    remoteShip->pos.y += remoteShip->vel.y >> VELOCITY_SHIFT;
    remoteShip->pos.z += remoteShip->vel.z >> VELOCITY_SHIFT;

    remoteShip->hdg = ang(remoteShip->hdg);
    remoteShip->pitch = ang(remoteShip->pitch);
    remoteShip->roll = ang(remoteShip->roll);
}

void UpdateShipLinearInterpolateSpline(ShipData* shipIndex, int shipNo)
{
#if 0 // TODO splines
    vec4i pos;
    vec4i pos1;
    vec4i step;
    PrmPSX prm;
    vec4i targetVector;
    int32 temp;

    if (shipstepCount == 8)
    {
        shipsplineCount += 1;
        shipstepCount = 0;

        if (shipsplineCount >= shipSpline[splineNumber]->primitiveCount - 2)
        {
            CameraUpdate = UpdateCameraExtroAttract;
            return;
        }
    }

    shipstepCount++;

    prm.primitive = shipSpline[splineNumber]->primitives;
    prm.spline += shipsplineCount;

    pos.x = prm.spline->position.x;
    pos.y = prm.spline->position.y;
    pos.z = prm.spline->position.z;

    pos1.x = (prm.spline + 1)->position.x;
    pos1.y = (prm.spline + 1)->position.y;
    pos1.z = (prm.spline + 1)->position.z;

    step.x = (pos1.x - pos.x) >> 3;
    step.y = (pos1.y - pos.y) >> 3;
    step.z = (pos1.z - pos.z) >> 3;

    step.x = (pos.x + (step.x << 3) - shipIndex[ownShip].pos.x);
    step.y = (pos.y + (step.y << 3) - shipIndex[ownShip].pos.y);
    step.z = (pos.z + (step.z << 3) - shipIndex[ownShip].pos.z);

    shipIndex[ownShip].vel.x += step.x;
    shipIndex[ownShip].vel.y += step.y;
    shipIndex[ownShip].vel.z += step.z;

    shipIndex[ownShip].vel.x -= shipIndex[ownShip].vel.x >> 3;
    shipIndex[ownShip].vel.y -= shipIndex[ownShip].vel.y >> 3;
    shipIndex[ownShip].vel.z -= shipIndex[ownShip].vel.z >> 3;

    shipIndex[ownShip].pos.x += (shipIndex[ownShip].vel.x >> 7);
    shipIndex[ownShip].pos.y += (shipIndex[ownShip].vel.y >> 7);
    shipIndex[ownShip].pos.z += (shipIndex[ownShip].vel.z >> 7);

    shipIndex[ownShip].nearTrkSect = FindAllNearestSection(&shipIndex[ownShip].pos);

    targetVector.x = pos1.x - shipIndex[ownShip].pos.x;
    targetVector.y = pos1.y - shipIndex[ownShip].pos.y;
    targetVector.z = pos1.z - shipIndex[ownShip].pos.z;

    temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    shipIndex[ownShip].vhdg = -ratan2(targetVector.x, targetVector.z) - shipIndex[ownShip].hdg;

    if (shipIndex[ownShip].vhdg > 2048)
    {
        shipIndex[ownShip].vhdg = shipIndex[ownShip].vhdg - 4096;
    }
    else if (shipIndex[ownShip].vhdg < -2048)
    {
        shipIndex[ownShip].vhdg = shipIndex[ownShip].vhdg + 4096;
    }

    shipIndex[ownShip].vhdg -= shipIndex[ownShip].vhdg >> 3;
    shipIndex[ownShip].hdg += shipIndex[ownShip].vhdg >> 4;

    shipIndex[ownShip].vpitch = -ratan2(targetVector.y, temp) - shipIndex[ownShip].pitch;
    shipIndex[ownShip].vpitch -= shipIndex[ownShip].vpitch >> 3;
    shipIndex[ownShip].pitch += shipIndex[ownShip].vpitch >> 4;

    shipIndex[ownShip].hdg = ang(shipIndex[ownShip].hdg);
    shipIndex[ownShip].pitch = ang(shipIndex[ownShip].pitch);
#endif
}

void UpdateCameraLinearInterpolateSpline(TrackCamera* camera, ShipData* ship, RescueData* rescueDroid)
{
#if 0 // TODO splines
    vec4i pos1;
    vec4i step;
    vec4i targetVector;
    vec3i pos;
    int32 temp;
    PrmPSX prm;
    int32 vhdg, vpitch;

    if (camerastepCount == 8)
    {
        camerasplineCount += 1;
        camerastepCount = 0;

        if (camerasplineCount >= camSpline[splineNumber]->primitiveCount - 1)
        {
            CameraUpdate = UpdateCameraExtroAttract;
            return;
        }
    }

    camerastepCount++;

    prm.primitive = camSpline[splineNumber]->primitives;
    prm.spline += camerasplineCount;

    pos.x = prm.spline->position.x;
    pos.y = prm.spline->position.y;
    pos.z = prm.spline->position.z;

    pos1.x = (prm.spline + 1)->position.x;
    pos1.y = (prm.spline + 1)->position.y;
    pos1.z = (prm.spline + 1)->position.z;

    step.x = (pos1.x - pos.x) >> 3;
    step.y = (pos1.y - pos.y) >> 3;
    step.z = (pos1.z - pos.z) >> 3;

    step.x = (pos.x + (step.x << 3) - gCamera.pos.x) >> 7;
    step.y = (pos.y + (step.y << 3) - gCamera.pos.y) >> 7;
    step.z = (pos.z + (step.z << 3) - gCamera.pos.z) >> 7;

    camera->vel.x += step.x;
    camera->vel.y += step.y;
    camera->vel.z += step.z;

    camera->vel.x -= camera->vel.x >> 3;
    camera->vel.y -= camera->vel.y >> 3;
    camera->vel.z -= camera->vel.z >> 3;

    pos.x = camera->vel.x - gCamera.pos.x;
    pos.y = camera->vel.y - gCamera.pos.y;
    pos.z = camera->vel.z - gCamera.pos.z;

    camera->section = FindNearestSection(camera->section, &pos);

    targetVector.x = ship[ownShip].pos.x - pos.x;
    targetVector.y = ship[ownShip].pos.y - pos.y;
    targetVector.z = ship[ownShip].pos.z - pos.z;

    temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    vhdg = ratan2(targetVector.x, targetVector.z) - camera->hdg;

    if (vhdg > 2048)
    {
        vhdg = vhdg - 4096;
    }
    else if (vhdg < -2048)
    {
        vhdg = vhdg + 4096;
    }

    vhdg -= vhdg >> 3;

    camera->hdg += vhdg >> 4;

    vpitch = ratan2(targetVector.y, temp) - camera->pitch;
    vpitch -= vpitch >> 3;
    camera->pitch += vpitch >> 4;

    camera->hdg = ang(camera->hdg);
    camera->pitch = ang(camera->pitch);

    matrixSetViewExternal(&pos, -camera->pitch, -camera->hdg); /* TODO check splines camera */
#endif
}

/*

This is the camera high up looking down

*/
void UpdateCameraHighUp(TrackCamera* camera, ShipData* ship, RescueData* rescueDroid)
{
    int32 temp;
    vec4i targetVector;

    ship += ownShip;

    camera->section = ship->nearTrkSect;

    timeOut--;
    if (timeOut <= 0)
    {
        PlayerData* player = (PlayerData*)ship->player;
        ASSERT(player);
        player->camera.update = UpdateCameraExtroAttract;
    }

    camera->pos.x = ship->pos.x - camVx;
    camera->pos.y = ship->pos.y - camVy;
    camera->pos.z = ship->pos.z - camVz;

    targetVector.x = ship->pos.x - camera->pos.x;
    targetVector.y = ship->pos.y - camera->pos.y;
    targetVector.z = ship->pos.z - camera->pos.z;

    temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    camera->hdg = -ratan2(targetVector.x, targetVector.z);
    camera->pitch = -ratan2(targetVector.y, temp);
}

/*

This is the camera high up looking down

*/
void UpdateCameraStationary(TrackCamera* camera, ShipData* ship, RescueData* rescueDroid)
{
    int32 temp;
    vec4i targetVector;
    
    ship += ownShip;

    timeOut--;
    if (timeOut <= 0)
    {
        PlayerData* player = (PlayerData*)ship->player;
        ASSERT(player);
        player->camera.update = UpdateCameraExtroAttract;
    }

    camera->pos.x = camera->section->centre.x - camVx;
    camera->pos.y = camera->section->centre.y - camVy;
    camera->pos.z = camera->section->centre.z - camVz;

    targetVector.x = ship->pos.x - camera->pos.x;
    targetVector.y = ship->pos.y - camera->pos.y;
    targetVector.z = ship->pos.z - camera->pos.z;

    temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

    camera->hdg = -ratan2(targetVector.x, targetVector.z);
    camera->pitch = -ratan2(targetVector.y, temp);
}

void UpdateCameraExtroAttract(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid)
{
    short i;
    static short count = 0;
    TrackSection* startSection;
    TrackSection* nextSection;
    vec4i targetVector;
#if 0 // TODO splines
    PrmPSX prm;
#endif
    int32 temp;

    /* firestar */
    if (gConfig.trackNum == 13)
    {
        switch (count)
        {
            case 0:
                /* set the remote ships to somewhere around the track */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = gTrack.sections + i + 50;
                    camShip[i].pos.x = (gTrack.sections + i + 50)->centre.x;
                    camShip[i].pos.y = (gTrack.sections + i + 50)->centre.y;
                    camShip[i].pos.z = (gTrack.sections + i + 50)->centre.z;
                }

                /* the spline we want to play */
                splineNumber = 0;

                /* set the initial ship position */
                camShip[ownShip].update = UpdateShipLinearInterpolateSpline;
#if 0 // TODO splines
                prm.primitive = shipSpline[splineNumber]->primitives;

                camShip[ownShip].pos.x = prm.spline->position.x;
                camShip[ownShip].pos.y = prm.spline->position.y;
                camShip[ownShip].pos.z = prm.spline->position.z;

                camShip[ownShip].nearTrkSect = FindAllNearestSection(&camShip[ownShip].pos);

                /* reset ship direction */
                camShip[ownShip].roll = 0;
                camShip[ownShip].vroll = 0;

                /* set the initial camera position */
                prm.primitive = camSpline[splineNumber]->primitives;

                camera->pos.x = prm.spline->position.x;
                camera->pos.y = prm.spline->position.y;
                camera->pos.z = prm.spline->position.z;

                camera->section = FindAllNearestSection(&camera->pos);

                /* set the camera direction */
                targetVector.x = camShip[ownShip].pos.x - camera->pos.x;
                targetVector.y = camShip[ownShip].pos.y - camera->pos.y;
                targetVector.z = camShip[ownShip].pos.z - camera->pos.z;

                temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

                camera->hdg = -ratan2(targetVector.x, targetVector.z);
                camera->pitch = -ratan2(targetVector.y, temp);

                /* initialize the camera velocities */
                camera->vel.x = 0;
                camera->vel.y = 0;
                camera->vel.z = 0;

                /* reset the spline counts */
                shipsplineCount = 0;
                shipstepCount = 0;

                camerasplineCount = 0;
                camerastepCount = 0;

                /* set the camera update */
                CameraUpdate = UpdateCameraLinearInterpolateSpline;
#endif
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 1:
                /* high camera */
                /* put the remotes out of the way */

                startFireText = 1;
                textY = 300;

                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    startSection = gTrack.sections + 0 + i + 1;

                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = startSection;
                    camShip[i].pos.x = startSection->centre.x;
                    camShip[i].pos.y = startSection->centre.y;
                    camShip[i].pos.z = startSection->centre.z;

                    nextSection = startSection->nextSection;
                    targetVector.x = nextSection->centre.x - startSection->centre.x;
                    targetVector.y = nextSection->centre.y - startSection->centre.y;
                    targetVector.z = nextSection->centre.z - startSection->centre.z;
                    camShip[i].hdg = -ratan2(targetVector.x, targetVector.z);
                }

                startSection = gTrack.sections + 50;

                camShip[ownShip].update = UpdateShipRemote;
                camShip[ownShip].nearTrkSect = startSection;
                camShip[ownShip].pos.x = startSection->centre.x;
                camShip[ownShip].pos.y = startSection->centre.y;
                camShip[ownShip].pos.z = startSection->centre.z;

                camVx = 4350;
                camVy = 9850;
                camVz = 4400;

                timeOut = FR30 * 13;

                camera->update = UpdateCameraHighUp;
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 2:
                /* spline */

                /* set the remote ships to somewhere around the track */

                startFireText = 0;

                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = gTrack.sections + i;
                    camShip[i].pos.x = (gTrack.sections + i)->centre.x;
                    camShip[i].pos.y = (gTrack.sections + i)->centre.y;
                    camShip[i].pos.z = (gTrack.sections + i)->centre.z;
                }

                /* the spline we want to play */
                splineNumber = 1;

                /* set the initial ship position */
                camShip[ownShip].update = UpdateShipLinearInterpolateSpline;
#if 0 // TODO splines
                prm.primitive = shipSpline[splineNumber]->primitives;

                camShip[ownShip].pos.x = prm.spline->position.x;
                camShip[ownShip].pos.y = prm.spline->position.y;
                camShip[ownShip].pos.z = prm.spline->position.z;

                camShip[ownShip].nearTrkSect = FindAllNearestSection(&camShip[ownShip].pos);

                /* reset ship direction */
                camShip[ownShip].roll = 0;
                camShip[ownShip].vroll = 0;

                /* set the initial camera position */
                prm.primitive = camSpline[splineNumber]->primitives;

                camera->pos.x = prm.spline->position.x;
                camera->pos.y = prm.spline->position.y;
                camera->pos.z = prm.spline->position.z;

                camera->section = FindAllNearestSection(&camera->pos);

                /* set the camera direction */
                targetVector.x = camShip[ownShip].pos.x - camera->pos.x;
                targetVector.y = camShip[ownShip].pos.y - camera->pos.y;
                targetVector.z = camShip[ownShip].pos.z - camera->pos.z;

                temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

                camera->hdg = -ratan2(targetVector.x, targetVector.z);
                camera->pitch = -ratan2(targetVector.y, temp);

                /* initialize the camera velocities */
                camera->vel.x = 0;
                camera->vel.y = 0;
                camera->vel.z = 0;

                /* reset the spline counts */
                shipsplineCount = 0;
                shipstepCount = 0;

                camerasplineCount = 0;
                camerastepCount = 0;

                /* set the camera update */
                CameraUpdate = UpdateCameraLinearInterpolateSpline;
#endif
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 3:
                /* spline */

                /* set the remote ships to somewhere around the track */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = gTrack.sections + i;
                    camShip[i].pos.x = (gTrack.sections + i)->centre.x;
                    camShip[i].pos.y = (gTrack.sections + i)->centre.y;
                    camShip[i].pos.z = (gTrack.sections + i)->centre.z;
                }

                /* the spline we want to play */
                splineNumber = 2;

                /* set the initial ship position */
                camShip[ownShip].update = UpdateShipLinearInterpolateSpline;
#if 0 // TODO splines
                prm.primitive = shipSpline[splineNumber]->primitives;

                camShip[ownShip].pos.x = prm.spline->position.x;
                camShip[ownShip].pos.y = prm.spline->position.y;
                camShip[ownShip].pos.z = prm.spline->position.z;

                camShip[ownShip].nearTrkSect = FindAllNearestSection(&camShip[ownShip].pos);

                /* reset ship direction */
                camShip[ownShip].roll = 0;
                camShip[ownShip].vroll = 0;

                /* set the initial camera position */
                prm.primitive = camSpline[splineNumber]->primitives;

                camera->pos.x = prm.spline->position.x;
                camera->pos.y = prm.spline->position.y;
                camera->pos.z = prm.spline->position.z;

                camera->section = FindAllNearestSection(&camera->pos);

                /* set the camera direction */
                targetVector.x = camShip[ownShip].pos.x - camera->pos.x;
                targetVector.y = camShip[ownShip].pos.y - camera->pos.y;
                targetVector.z = camShip[ownShip].pos.z - camera->pos.z;

                temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

                camera->hdg = -ratan2(targetVector.x, targetVector.z);
                camera->pitch = -ratan2(targetVector.y, temp);

                /* initialize the camera velocities */
                camera->vel.x = 0;
                camera->vel.y = 0;
                camera->vel.z = 0;

                /* reset the spline counts */
                shipsplineCount = 0;
                shipstepCount = 0;

                camerasplineCount = 0;
                camerastepCount = 0;

                /* set the camera update */
                CameraUpdate = UpdateCameraLinearInterpolateSpline;
#endif
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 4:
                /* spline */

                /* set the remote ships to somewhere around the track */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = gTrack.sections + i;
                    camShip[i].pos.x = (gTrack.sections + i)->centre.x;
                    camShip[i].pos.y = (gTrack.sections + i)->centre.y;
                    camShip[i].pos.z = (gTrack.sections + i)->centre.z;
                }

                /* the spline we want to play */
                splineNumber = 3;

                /* set the initial ship position */
                camShip[ownShip].update = UpdateShipLinearInterpolateSpline;
#if 0 // TODO splines
                prm.primitive = shipSpline[splineNumber]->primitives;

                camShip[ownShip].pos.x = prm.spline->position.x;
                camShip[ownShip].pos.y = prm.spline->position.y;
                camShip[ownShip].pos.z = prm.spline->position.z;

                camShip[ownShip].nearTrkSect = FindAllNearestSection(&camShip[ownShip].pos);

                /* reset ship direction */
                camShip[ownShip].roll = 0;
                camShip[ownShip].vroll = 0;

                /* set the initial camera position */
                prm.primitive = camSpline[splineNumber]->primitives;

                camera->pos.x = prm.spline->position.x;
                camera->pos.y = prm.spline->position.y;
                camera->pos.z = prm.spline->position.z;

                camera->section = FindAllNearestSection(&camera->pos);

                /* set the camera direction */
                targetVector.x = camShip[ownShip].pos.x - camera->pos.x;
                targetVector.y = camShip[ownShip].pos.y - camera->pos.y;
                targetVector.z = camShip[ownShip].pos.z - camera->pos.z;

                temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

                camera->hdg = -ratan2(targetVector.x, targetVector.z);
                camera->pitch = -ratan2(targetVector.y, temp);

                /* initialize the camera velocities */
                camera->vel.x = 0;
                camera->vel.y = 0;
                camera->vel.z = 0;

                /* reset the spline counts */
                shipsplineCount = 0;
                shipstepCount = 0;

                camerasplineCount = 0;
                camerastepCount = 0;

                /* set the camera update */
                CameraUpdate = UpdateCameraLinearInterpolateSpline;
#endif
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 5:
                finishedExtro = 1;
                break;

            default:
                ASSERT(0);
                break;
        }

        count++;
    }

    /* silverstream */
    if (gConfig.trackNum == 12)
    {
        switch (count)
        {
            case 0:
                /* spline */

                /* set the remote ships to somewhere around the track */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = gTrack.sections + i + 100;
                    camShip[i].pos.x = (gTrack.sections + i + 100)->centre.x;
                    camShip[i].pos.y = (gTrack.sections + i + 100)->centre.y;
                    camShip[i].pos.z = (gTrack.sections + i + 100)->centre.z;
                }

                /* the spline we want to play */
                splineNumber = 4;

                /* set the initial ship position */
                camShip[ownShip].update = UpdateShipLinearInterpolateSpline;
#if 0 // TODO splines
                prm.primitive = shipSpline[splineNumber]->primitives;

                camShip[ownShip].pos.x = prm.spline->position.x;
                camShip[ownShip].pos.y = prm.spline->position.y;
                camShip[ownShip].pos.z = prm.spline->position.z;

                camShip[ownShip].nearTrkSect = FindAllNearestSection(&camShip[ownShip].pos);

                /* reset ship direction */
                camShip[ownShip].roll = 0;
                camShip[ownShip].vroll = 0;

                /* set the initial camera position */
                prm.primitive = camSpline[splineNumber]->primitives;

                camera->pos.x = prm.spline->position.x;
                camera->pos.y = prm.spline->position.y;
                camera->pos.z = prm.spline->position.z;

                camera->section = FindAllNearestSection(&camera->pos);

                /* set the camera direction */
                targetVector.x = camShip[ownShip].pos.x - camera->pos.x;
                targetVector.y = camShip[ownShip].pos.y - camera->pos.y;
                targetVector.z = camShip[ownShip].pos.z - camera->pos.z;

                temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

                camera->hdg = -ratan2(targetVector.x, targetVector.z);
                camera->pitch = -ratan2(targetVector.y, temp);

                /* initialize the camera velocities */
                camera->vel.x = 0;
                camera->vel.y = 0;
                camera->vel.z = 0;

                /* reset the spline counts */
                shipsplineCount = 0;
                shipstepCount = 0;

                camerasplineCount = 0;
                camerastepCount = 0;

                /* set the camera update */
                CameraUpdate = UpdateCameraLinearInterpolateSpline;
#endif
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 1:
                /* stationary camera */
                /* put the remotes out of the way */

                startIceText = 1;
                textY = 300;

                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    startSection = gTrack.sections + 90 + i + 1;

                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = startSection;
                    camShip[i].pos.x = startSection->centre.x;
                    camShip[i].pos.y = startSection->centre.y;
                    camShip[i].pos.z = startSection->centre.z;

                    nextSection = startSection->nextSection;
                    targetVector.x = nextSection->centre.x - startSection->centre.x;
                    targetVector.y = nextSection->centre.y - startSection->centre.y;
                    targetVector.z = nextSection->centre.z - startSection->centre.z;
                    camShip[i].hdg = -ratan2(targetVector.x, targetVector.z);
                }

                startSection = gTrack.sections + 75;

                camShip[ownShip].update = UpdateShipRemote;
                camShip[ownShip].nearTrkSect = startSection;
                camShip[ownShip].pos.x = startSection->centre.x;
                camShip[ownShip].pos.y = startSection->centre.y;
                camShip[ownShip].pos.z = startSection->centre.z;

                camVx = 2000;
                camVy = 2750;
                camVz = 0;

                timeOut = FR30 * 3;

                camera->section = startSection + 12;
                camera->update = UpdateCameraStationary;
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 2:
                /* stationary camera */
                /* put the remotes out of the way */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    startSection = gTrack.sections + 90 + i + 1;

                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = startSection;
                    camShip[i].pos.x = startSection->centre.x;
                    camShip[i].pos.y = startSection->centre.y;
                    camShip[i].pos.z = startSection->centre.z;

                    nextSection = startSection->nextSection;
                    targetVector.x = nextSection->centre.x - startSection->centre.x;
                    targetVector.y = nextSection->centre.y - startSection->centre.y;
                    targetVector.z = nextSection->centre.z - startSection->centre.z;
                    camShip[i].hdg = -ratan2(targetVector.x, targetVector.z);
                }

                startSection = gTrack.sections + 120;

                camShip[ownShip].update = UpdateShipRemote;
                camShip[ownShip].nearTrkSect = startSection;
                camShip[ownShip].pos.x = startSection->centre.x;
                camShip[ownShip].pos.y = startSection->centre.y;
                camShip[ownShip].pos.z = startSection->centre.z;

                camVx = 2250;
                camVy = 1500;
                camVz = -1500;

                timeOut = FR30 * 6;

                camera->section = startSection + 12;
                camera->update = UpdateCameraStationary;
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 3:
                /* high camera */
                /* put the remotes out of the way */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    startSection = gTrack.sections + 50 + i + 1;

                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = startSection;
                    camShip[i].pos.x = startSection->centre.x;
                    camShip[i].pos.y = startSection->centre.y;
                    camShip[i].pos.z = startSection->centre.z;

                    nextSection = startSection->nextSection;
                    targetVector.x = nextSection->centre.x - startSection->centre.x;
                    targetVector.y = nextSection->centre.y - startSection->centre.y;
                    targetVector.z = nextSection->centre.z - startSection->centre.z;
                    camShip[i].hdg = -ratan2(targetVector.x, targetVector.z);
                }

                startSection = gTrack.sections + 193;

                camShip[ownShip].update = UpdateShipRemote;
                camShip[ownShip].nearTrkSect = startSection;
                camShip[ownShip].pos.x = startSection->centre.x;
                camShip[ownShip].pos.y = startSection->centre.y;
                camShip[ownShip].pos.z = startSection->centre.z;

                nextSection = startSection->nextSection;
                targetVector.x = nextSection->centre.x - startSection->centre.x;
                targetVector.y = nextSection->centre.y - startSection->centre.y;
                targetVector.z = nextSection->centre.z - startSection->centre.z;
                camShip[ownShip].hdg = -ratan2(targetVector.x, targetVector.z);

                camVx = 350;
                camVy = 11100;
                camVz = -2850;

                timeOut = FR30 * 6;

                camera->update = UpdateCameraHighUp;
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 4:
                /* spline */

                /* set the remote ships to somewhere around the track */

                startIceText = 0;

                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = gTrack.sections + i + 100;
                    camShip[i].pos.x = (gTrack.sections + i + 100)->centre.x;
                    camShip[i].pos.y = (gTrack.sections + i + 100)->centre.y;
                    camShip[i].pos.z = (gTrack.sections + i + 100)->centre.z;
                }

                /* the spline we want to play */
                splineNumber = 5;

                /* set the initial ship position */
                camShip[ownShip].update = UpdateShipLinearInterpolateSpline;
#if 0 // TODO splines
                prm.primitive = shipSpline[splineNumber]->primitives;

                camShip[ownShip].pos.x = prm.spline->position.x;
                camShip[ownShip].pos.y = prm.spline->position.y;
                camShip[ownShip].pos.z = prm.spline->position.z;

                camShip[ownShip].nearTrkSect = FindAllNearestSection(&camShip[ownShip].pos);

                /* reset ship direction */
                camShip[ownShip].roll = 0;
                camShip[ownShip].vroll = 0;

                startSection = camShip[ownShip].nearTrkSect;
                nextSection = startSection->nextSection;
                targetVector.x = nextSection->centre.x - startSection->centre.x;
                targetVector.y = nextSection->centre.y - startSection->centre.y;
                targetVector.z = nextSection->centre.z - startSection->centre.z;
                camShip[ownShip].hdg = -ratan2(targetVector.x, targetVector.z);

                /* set the initial camera position */
                prm.primitive = camSpline[splineNumber]->primitives;

                camera->pos.x = prm.spline->position.x;
                camera->pos.y = prm.spline->position.y;
                camera->pos.z = prm.spline->position.z;

                camera->section = FindAllNearestSection(&camera->pos);

                /* set the camera direction */
                targetVector.x = camShip[ownShip].pos.x - camera->pos.x;
                targetVector.y = camShip[ownShip].pos.y - camera->pos.y;
                targetVector.z = camShip[ownShip].pos.z - camera->pos.z;

                temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

                camera->hdg = -ratan2(targetVector.x, targetVector.z);
                camera->pitch = -ratan2(targetVector.y, temp);

                /* initialize the camera velocities */
                camera->vel.x = 0;
                camera->vel.y = 0;
                camera->vel.z = 0;

                /* reset the spline counts */
                shipsplineCount = 0;
                shipstepCount = 0;

                camerasplineCount = 0;
                camerastepCount = 0;

                /* set the camera update */
                CameraUpdate = UpdateCameraLinearInterpolateSpline;
#endif
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 5:
                /* spline */

                /* set the remote ships to somewhere around the track */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = gTrack.sections + i + 100;
                    camShip[i].pos.x = (gTrack.sections + i + 100)->centre.x;
                    camShip[i].pos.y = (gTrack.sections + i + 100)->centre.y;
                    camShip[i].pos.z = (gTrack.sections + i + 100)->centre.z;
                }

                /* the spline we want to play */
                splineNumber = 6;

                /* set the initial ship position */
                camShip[ownShip].update = UpdateShipLinearInterpolateSpline;
#if 0 // TODO splines
                prm.primitive = shipSpline[splineNumber]->primitives;

                camShip[ownShip].pos.x = prm.spline->position.x;
                camShip[ownShip].pos.y = prm.spline->position.y;
                camShip[ownShip].pos.z = prm.spline->position.z;

                camShip[ownShip].nearTrkSect = FindAllNearestSection(&camShip[ownShip].pos);

                /* reset ship direction */
                camShip[ownShip].roll = 0;
                camShip[ownShip].vroll = 0;

                /* set the initial camera position */
                prm.primitive = camSpline[splineNumber]->primitives;

                camera->pos.x = (prm.spline)->position.x;
                camera->pos.y = (prm.spline)->position.y;
                camera->pos.z = (prm.spline)->position.z;

                camera->section = FindAllNearestSection(&camera->pos);

                /* set the camera direction */
                targetVector.x = camShip[ownShip].pos.x - camera->pos.x;
                targetVector.y = camShip[ownShip].pos.y - camera->pos.y;
                targetVector.z = camShip[ownShip].pos.z - camera->pos.z;

                temp = rsqrt(targetVector.x * targetVector.x + targetVector.z * targetVector.z);

                camera->hdg = -ratan2(targetVector.x, targetVector.z);
                camera->pitch = -ratan2(targetVector.y, temp);

                /* initialize the camera velocities */
                camera->vel.x = 0;
                camera->vel.y = 0;
                camera->vel.z = 0;

                /* reset the spline counts */
                shipsplineCount = 0;
                shipstepCount = 0;

                camerasplineCount = 0;
                camerastepCount = 0;

                /* set the camera update */
                CameraUpdate = UpdateCameraLinearInterpolateSpline;
#endif
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 6:
                /* high camera */
                /* put the remotes out of the way */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    startSection = gTrack.sections + 90 + i + 1;

                    camShip[i].update = UpdateShipRemote;
                    camShip[i].nearTrkSect = startSection;
                    camShip[i].pos.x = startSection->centre.x;
                    camShip[i].pos.y = startSection->centre.y;
                    camShip[i].pos.z = startSection->centre.z;

                    nextSection = startSection->nextSection;
                    targetVector.x = nextSection->centre.x - startSection->centre.x;
                    targetVector.y = nextSection->centre.y - startSection->centre.y;
                    targetVector.z = nextSection->centre.z - startSection->centre.z;
                    camShip[i].hdg = -ratan2(targetVector.x, targetVector.z);
                }

                startSection = gTrack.sections + 335;

                camShip[ownShip].update = UpdateShipRemote;
                camShip[ownShip].nearTrkSect = startSection;
                camShip[ownShip].pos.x = startSection->centre.x;
                camShip[ownShip].pos.y = startSection->centre.y;
                camShip[ownShip].pos.z = startSection->centre.z;

                nextSection = startSection->nextSection;
                targetVector.x = nextSection->centre.x - startSection->centre.x;
                targetVector.y = nextSection->centre.y - startSection->centre.y;
                targetVector.z = nextSection->centre.z - startSection->centre.z;
                camShip[ownShip].hdg = -ratan2(targetVector.x, targetVector.z);

                camShip[ownShip].vel.x = targetVector.x >> 2;
                camShip[ownShip].vel.y = targetVector.y >> 2;
                camShip[ownShip].vel.z = targetVector.z >> 2;

                camVx = 1100;
                camVy = 7850;
                camVz = 4400;

                timeOut = FR30 * 7;

                camera->update = UpdateCameraHighUp;
                camShip[ownShip].attr &= ~VIEW_IN;
                break;

            case 7:
                finishedExtro = 1;
                break;

            default:
                ASSERT(0);
                break;
        }

        count++;
    }

    if (!finishedExtro)
    {
        (camera->update)(camera, camShip, rescueDroid);
    }
}

void InitFireCongratulations(void)
{
    con = gGameCongrats;
}

void InitIceCongratulations(void)
{
    con = gRepierCongrats;
}

void ShowCongratulations(void)
{
    int32 lineY = textY;
    const Congrats* c = con;

    while (c->size)
    {
        if ((lineY > -10) && (lineY < 270))
        {
            CentreText(c->str, text_data(0, lineY, c->size), (c->colour == White) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT);
        }

        lineY += 12;
        c++;
    }

    if (textY > -350)
    {
        textY -= (gFrameIndex - gLastFrameIndex);
    }
}
