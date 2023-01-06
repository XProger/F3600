#include "game.h"

SfxParam gSfxParams[SFX_MAX];

#ifdef __3DO__
uint8* sdxData;

int32 SfxLoadBankSDX(const char* fileName)
{
    int32 size = osLoadFile(fileName, pDRAM);
    sdxData = (uint8*)allocDRAM(size);
    return 1;
}
#else
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

    int32 i, offset;
    VabHdr* header;
    uint8* ptr = pDRAM;

    osLoadFile(vh, ptr);
    header = (VabHdr*)ptr;
    ASSERT(SFX_MAX == header->vs);
    ptr += sizeof(VabHdr);
    ptr += 128 * 16 + (32 * 16 * header->ps) + 2;
    memcpy(vagSize, ptr, header->vs * sizeof(uint16));

    offset = 0;
    for (i = 0; i < header->vs; i++)
    {
        vagOffset[i] = offset;
        offset += vagSize[i];
        ASSERT((vagSize[i] * 8) % 16 == 0);
    }

    ASSERT(offset * 8 < sizeof(vagData));

    osLoadFile(vb, vagData);

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
#endif

void SfxInit(void)
{
    int32 i;

    for (i = 0; i < SFX_MAX; i++)
    {
        gSfxParams[i].volL = SND_MAX_VOLUME;
        gSfxParams[i].volR = SND_MAX_VOLUME;
        gSfxParams[i].pitch = SND_DEF_PITCH;
        gSfxParams[i].loop = 0;
    }

    gSfxParams[SFX_POWER_UP].pitch = SND_DEF_PITCH >> 1;

    gSfxParams[SFX_REMOTE_ENG].loop =
    gSfxParams[SFX_ENG_RUMBLE].loop =
    gSfxParams[SFX_ENG_AIR_INTAKE].loop =
    gSfxParams[SFX_ENG_THRUST].loop =
    gSfxParams[SFX_TURBULENCE].loop =
    gSfxParams[SFX_CROWD].loop =
    gSfxParams[SFX_TRACTOR_BEAM].loop =
    gSfxParams[SFX_SHIELD].loop = 1;

#ifdef __3DO__
    if (SfxLoadBankSDX("wipeout/sound/wipeout.sdx"))
        return;
#else
    if (SfxLoadBankVAB("wipeout/sound/wipeout.vh", "wipeout/sound/wipeout.vb"))
        return;

    if (SfxLoadBankFx("wipeout/wipeout.fx"))
        return;
#endif

    ASSERT(0); // no banks found
}

void SfxFree(void)
{

}

void AdjustDirectionalNote(enum SfxType id, int32 x, int32 z)
{
    int32 dx, dz;
    int32 mag;
    int32 voll, volr;
    int32 angle;

    dx = (x - gCamera.pos.x) >> 3;
    dz = (z - gCamera.pos.z) >> 3;

    mag = GetMagnitudeApprox(dx, dz);
    mag = mag << 2;

    voll = 0x2fff - mag;
    if (voll < 0)
        voll = 0;

    volr = voll;

    angle = -(ratan2(dx, dz) + gCamera.hdg);
    angle = ang(angle);

    if (angle > 1024)
        angle = 1024 - (angle - 1024);
    if (angle < -1024)
        angle = -1024 - (angle + 1024);

    voll += voll * angle >> 11;
    volr -= volr * angle >> 11;

    if ((voll > 0) && (volr > 0))
    {
        SetVolume(id, voll, volr);
    }
    else
    {
        SetVolume(id, 0, 0);
    }
}

void AdjustRemoteEngineSound(ShipData* shipIndex, Object** shipShapes)
{
    uint16 i;
    int32 dx, dz;
    int32 mag, smallestMag;
    int32 ident;
    int32 voll, volr;
    int32 angle;
    int32 doplerMag;

#ifdef __3DO__ // TODO 3DO optimize
    return;
#endif

    smallestMag = 14000;
    ident = -1;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i != ownShip)
        {
            dx = (shipIndex[i].pos.x - gCamera.pos.x) >> 3;
            dz = (shipIndex[i].pos.z - gCamera.pos.z) >> 3;

            mag = GetMagnitudeApprox(dx, dz);

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

        angle = -(ratan2(dx, dz) + gCamera.hdg);
        angle = ang(angle);

        if (angle > 1024)
            angle = 1024 - (angle - 1024);
        if (angle < -1024)
            angle = -1024 - (angle + 1024);

        voll += (voll * angle) >> 11;
        volr -= (volr * angle) >> 11;

        dx += ((shipIndex[ident].vel.x >> VELOCITY_SHIFT) - gCamera.vel.x);
        dz += ((shipIndex[ident].vel.z >> VELOCITY_SHIFT) - gCamera.vel.z);

        doplerMag = mag - GetMagnitudeApprox(dx, dz);

        SetPitch(SFX_REMOTE_ENG, (SND_DEF_PITCH >> 1) + doplerMag);
        SetVolume(SFX_REMOTE_ENG, voll, volr);
    }
    else
    {
        SetVolume(SFX_REMOTE_ENG, 0, 0);
    }
}

/*********************************************************************/
/**** WiPe OuT specific functions to adjust in game sound effects ****/
/*********************************************************************/

void AdjustEngineSound(ShipData* playerShip, TrackCamera* camera)
{
    int32 vol;
    int32 pitch;

#ifdef __3DO__ // TODO 3DO optimize
    return;
#endif

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

int PlayDirectionalNote(enum SfxType id, const vec3i* pos)
{
    int32 channel = PlayNote(id);
    AdjustDirectionalNote(id, pos->x, pos->z);
    return channel;
}
