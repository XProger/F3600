#define SOUND  1

#define NOT_PAUSED 1
#define PAUSED 0
#define PAUSE_LEAVE 8


#define SND_CHANNELS    2
#define SND_SAMPLES     (2352 * 2)
#define SND_OUTPUT_FREQ 44100

#define SND_MAX_VOLUME  0x8000
#define SND_DEF_PITCH   0x1000

/****** PROGRAME IDENTIFIERS ******/

enum SfxType
{
    SFX_SHIP_2_SHIP,
    SFX_ELECTROBOLT,
    SFX_ENG_AIR_INTAKE,
    SFX_ENG_RUMBLE,
    SFX_ENG_THRUST,
    SFX_EXPLOSION_1,
    SFX_EXPLOSION_2,
    SFX_SHIP_2_TRACK,
    SFX_MENU_MOVE,
    SFX_MENU_SELECT,
    SFX_MENU_2_MENU,
    SFX_MINE_DROP,
    SFX_MISS_LAUNCH,
    SFX_REMOTE_ENG,
    SFX_POWER_UP,
    SFX_SHIELDS,
    SFX_START_SIREN, // unused
    SFX_TRACTOR_BEAM,
    SFX_TURBULENCE,
    SFX_CROWD,
    SFX_SP_MINES,
    SFX_SP_MISSILE,
    SFX_SP_ROCKET,
    SFX_SP_REVCON,
    SFX_SP_SHOCKWAVE,
    SFX_SP_SPECIAL,
    SFX_SP_3,
    SFX_SP_2,
    SFX_SP_1,
    SFX_SP_GO,
    SFX_MAX
};

/****** Max volume settings ******/
#define ENG_AIR_INTAKE_VOL 4096
#define ENG_RUMBLE_VOL 8192
#define ENG_THRUST_VOL 500
#define REMOTE_ENG_VOL 125

typedef struct SampRec
{
    uint8 SampVol;
    uint8 SampPan;
    uint16 SampPitch;
    uint8 Loop;
    uint8 Playing;
    int16 PlayID;
} SampRec;

int WhatChannel(enum SfxType id);
int IsPlaying(int channel);
void InitNotes(void);
void KillNote(enum SfxType id);
void KillNotes(void);
int PlayNote(enum SfxType id);
void SetPitch(enum SfxType id, int32 pitch);
void SetVolume(enum SfxType id, int32 volL, int32 volR);
void FillNotes(int16* samples, int32 count);

int MusicIsPlaying();
void MusicPlay(int track);
void MusicStop();
