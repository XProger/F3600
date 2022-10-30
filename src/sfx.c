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
    gSfxParams[SFX_SHIELDS].loop = 1;

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

short AdjustDirectionalNote(enum SfxType id, const vec3i* pos)
{
    vec3i distance;
    int mag;
    int voll, volr;
    short angle;

    distance.x = (pos->x + cameraPtr->camPos->relative.t[0]) >> 3;
    distance.y = (pos->y + cameraPtr->camPos->relative.t[1]) >> 3;
    distance.z = (pos->z + cameraPtr->camPos->relative.t[2]) >> 3;

    mag = GetMagnitude(&distance);
    mag = mag << 2;

    voll = 0x2fff - mag;
    if (voll < 0)
        voll = 0;

    volr = voll;

    angle = -ratan2(distance.x, distance.z) + cameraPtr->hdg;
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
        SetVolume(id, voll, volr);
    }
    else
    {
        SetVolume(id, 0, 0);
    }

    return 1;
}

short AdjustRemoteEngineSound(ShipData* shipIndex, Object** shipShapes, TrackCamera* camera)
{
    uint16 i;
    vec3i distance;
    int mag, smallestMag;
    short ident;
    int voll, volr;
    short angle;
    vec3i dopler;
    int doplerMag;

#ifdef __3DO__ // TODO optimize
    return;
#endif

    smallestMag = 14000;
    ident = -1;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i != ownShip)
        {
            distance.x = (shipIndex[i].pos.x + camera->camPos->relative.t[0]) >> 3;
            distance.y = (shipIndex[i].pos.y + camera->camPos->relative.t[1]) >> 3;
            distance.z = (shipIndex[i].pos.z + camera->camPos->relative.t[2]) >> 3;

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

        angle = -ratan2(distance.x, distance.z) + camera->hdg;
        angle = ang(angle);

        if (angle > 1024)
            angle = 1024 - (angle - 1024);
        if (angle < -1024)
            angle = -1024 - (angle + 1024);

        voll += (voll * angle) >> 11;
        volr -= (volr * angle) >> 11;

        //voll >>= 2;
        //volr >>= 2;

        dopler.x = (shipIndex[ident].vel.x >> VELOCITY_SHIFT) - camera->vel.x;
        dopler.y = (shipIndex[ident].vel.y >> VELOCITY_SHIFT) - camera->vel.y;
        dopler.z = (shipIndex[ident].vel.z >> VELOCITY_SHIFT) - camera->vel.z;

        dopler.x += distance.x;
        dopler.y += distance.y;
        dopler.z += distance.z;

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

#ifdef __3DO__ // TODO optimize
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
    int channel = PlayNote(id);
    AdjustDirectionalNote(id, pos);
    return channel;
}
