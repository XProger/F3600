#include "game.h"

uint8 vagData[29 * 16 * 1024]; // should be enough
uint16 vagOffset[SFX_MAX];
uint16 vagSize[SFX_MAX];

int32 SfxLoadBankVAB(const char* vh, const char* vb)
{
    typedef struct
    {
        uint32 form;
        uint32 ver;
        uint32 id;
        uint32 fsize;
        uint16 reserved0;
        uint16 ps;
        uint16 ts;
        uint16 vs;
        uint8 mvol;
        uint8 pan;
        uint8 attr1;
        uint8 attr2;
        uint32 reserved1;
    } VabHdr;

    FILE* f;
    int32 i, offset;
    VabHdr header;

    f = fopen(vh, "rb");
    if (!f) return 0;
    fread(&header, sizeof(header), 1, f);
    ASSERT(SFX_MAX == header.vs);
    fseek(f, 128 * 16 + (32 * 16 * header.ps) + 2, SEEK_CUR); // skip programs and tones
    fread(vagSize, 2, header.vs, f);
    fclose(f);

    offset = 0;
    for (i = 0; i < header.vs; i++)
    {
        vagOffset[i] = offset;
        offset += vagSize[i];
        ASSERT((vagSize[i] * 8) % 16 == 0);
    }

    ASSERT(offset * 8 < sizeof(vagData));

    f = fopen(vb, "rb");
    if (!f) return 0;
    fread(vagData, 1, offset * 8, f);
    fclose(f);

    //dump_vags();

    return 1;
}

int32 SfxLoadBankFx(const char* fileName)
{
    //char text[512 * 3];

    //if (!LoadFile(fileName, text))
    //    return 0;

    return 0;
}

void SfxInit(void)
{
    if (SfxLoadBankVAB("wipeout\\sound\\wipeout.vh", "wipeout\\sound\\wipeout.vb"))
        return;

    if (SfxLoadBankFx("wipeout\\wipeout.fx"))
        return;

    ASSERT(0); // no banks found
}

void SfxFree(void)
{

}

short AdjustDirectionalNote(enum SfxType vagID, const VECTOR* pos)
{
    VECTOR distance;
    int mag;
    int voll, volr;
    short angle;

    distance.vx = (pos->vx + cameraPtr->camPos->relative.t[0]) >> 3;
    distance.vy = (pos->vy + cameraPtr->camPos->relative.t[1]) >> 3;
    distance.vz = (pos->vz + cameraPtr->camPos->relative.t[2]) >> 3;

    mag = GetMagnitude(&distance);
    mag = mag << 2;

    voll = 0x2fff - mag;
    if (voll < 0)
        voll = 0;

    volr = voll;

    angle = -ratan2(distance.vx, distance.vz) + cameraPtr->hdg;
    angle = ang(angle);

    if (angle > 1024)
        angle = 1024 - (angle - 1024);
    if (angle < -1024)
        angle = -1024 - (angle + 1024);

    voll += voll * angle >> 11;
    volr -= volr * angle >> 11;

    //voll >>= 2;
    //volr >>= 2;

    if ((voll > 0) && (volr > 0))
    {
        SetVolume(vagID, voll, volr);
    }
    else
    {
        SetVolume(vagID, 0, 0);
    }

    return 1;
}

short AdjustRemoteEngineSound(ShipData* shipIndex, Object** shipShapes, TrackCamera* camera)
{
    uint16 i;
    VECTOR distance;
    int mag, smallestMag;
    short ident;
    int voll, volr;
    short angle;
    VECTOR dopler;
    int doplerMag;

    smallestMag = 14000;
    ident = -1;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i != ownShip)
        {
            distance.vx = (shipIndex[i].pos.vx + camera->camPos->relative.t[0]) >> 3;
            distance.vy = (shipIndex[i].pos.vy + camera->camPos->relative.t[1]) >> 3;
            distance.vz = (shipIndex[i].pos.vz + camera->camPos->relative.t[2]) >> 3;

            mag = GetMagnitude(&distance);

            if (mag < smallestMag)
            {
                smallestMag = mag;
                ident = i;
            }
        }
    }

    if (ident > -1)
    {
        mag = smallestMag;
        mag <<= 2;

        if (mag > 1000)
            voll = 0x2fff - ((mag - 1000) << 2);
        else
            voll = 0x2fff;
        if (voll < 0)
            voll = 0;

        if (mag > 1000)
            volr = 0x2fff - ((mag - 1000) << 2);
        else
            volr = 0x2fff;
        if (volr < 0)
            volr = 0;

        angle = -ratan2(distance.vx, distance.vz) + camera->hdg;
        angle = ang(angle);

        if (angle > 1024)
            angle = 1024 - (angle - 1024);
        if (angle < -1024)
            angle = -1024 - (angle + 1024);

        voll += (voll * angle) >> 11;
        volr -= (volr * angle) >> 11;

        //voll >>= 2;
        //volr >>= 2;

        dopler.vx = (shipIndex[ident].vel.vx >> 6) - camera->vel.vx;
        dopler.vy = (shipIndex[ident].vel.vy >> 6) - camera->vel.vy;
        dopler.vz = (shipIndex[ident].vel.vz >> 6) - camera->vel.vz;

        dopler.vx += distance.vx;
        dopler.vy += distance.vy;
        dopler.vz += distance.vz;

        doplerMag = mag - GetMagnitude(&dopler);

        SetPitch(SFX_REMOTE_ENG, (SND_DEF_PITCH >> 1) + doplerMag);
        SetVolume(SFX_REMOTE_ENG, voll, volr);
    }
    else
    {
        SetVolume(SFX_REMOTE_ENG, 0, 0);
    }

    return 1;
}

/*********************************************************************/
/**** WiPe OuT specific functions to adjust in game sound effects ****/
/*********************************************************************/

void AdjustEngineSound(ShipData* playerShip, TrackCamera* camera)
{
    short vol;
    short pitch;

    /******************************************/
    /****             ENGINE 2             ****/
    /******************************************/

    vol = (ENG_AIR_INTAKE_VOL * playerShip->speed) >> 15;
    if (vol > ENG_AIR_INTAKE_VOL)
        vol = ENG_AIR_INTAKE_VOL;

    /***        Pitch Increment 0x1000           ***/
    /***  Pitch was started at 42 + 4 octaves    ***/

    pitch = playerShip->speed >> 2;

    SetPitch(SFX_ENG_AIR_INTAKE, (SND_DEF_PITCH >> 1) + pitch);
    SetVolume(SFX_ENG_AIR_INTAKE, vol, vol);

    vol = 375 + ((ENG_THRUST_VOL * playerShip->thrust_mag) / playerShip->max_thrust);

    pitch = (0x7ff * playerShip->thrust_mag) / playerShip->max_thrust;
    pitch += playerShip->speed >> 5;

    /***        Pitch Increment 2048             ***/
    /***  Pitch was started at 55 + 2 octaves    ***/

    SetPitch(SFX_ENG_THRUST, SND_DEF_PITCH + pitch);
    SetVolume(SFX_ENG_THRUST, vol, vol);

    SetVolume(SFX_ENG_RUMBLE, ENG_RUMBLE_VOL, ENG_RUMBLE_VOL);

    /******************************************/
    /****      TURBULANCE & AIR BRAKES     ****/
    /******************************************/

    /******* Needs re-balancing ********/

    vol = playerShip->speed >> 1;
    SetVolume(SFX_TURBULENCE, 
        (vol >> 2) + ((vol * playerShip->r_brake_rot) >> 9),
        (vol >> 2) + ((vol * playerShip->l_brake_rot) >> 9));
}

int PlayDirectionalNote(enum SfxType id, const VECTOR* pos)
{
    int channel = PlayNote(id);
    AdjustDirectionalNote(id, pos);
    return channel;
}
