#include "game.h"
#include <soundplayer.h>

#define MAX_CHANNELS        7 // 5 mono + 1 stereo = 7
#define MUSIC_CHANNEL       (MAX_CHANNELS - 2) // preserve 2 mixer channels for the stereo music output

#define BLOCK_SIZE_CD       (2 * 1024)
#define SND_BUFFER_SIZE     (8 * BLOCK_SIZE_CD)
#define SND_BUFFERS         4

extern uint8* sdxData; // should be enough

typedef struct
{
    Item gainL;
    Item gainR;
    Item instrument;
    Item attach;
    Item frequency;
    Item amplitude;

    int32 id;
    int32 playing;
} Channel;

typedef struct
{
    Item sample;
    int32 size;
} SampleData;

Channel channels[MAX_CHANNELS]; // [sample, sample, sample, music]
SampleData samples[SFX_MAX];

Item sndMixer;
Item musicThread;
struct SPPlayer* musicPlayer;
struct SPSound* music;
uint32 musicSignal;
int32 musicTrack;
uint8* gMusicDRAM;

void musicProc(void)
{
    int32 i;
    int32 track;
    int32 signalMask;
    void* buffers[SND_BUFFERS];

    OpenAudioFolio();

    musicSignal = AllocSignal(0);

    for (i = 0; i < SND_BUFFERS; i++)
    {
        buffers[i] = gMusicDRAM + i * SND_BUFFER_SIZE;
    }

    spCreatePlayer(&musicPlayer, channels[MUSIC_CHANNEL].instrument, SND_BUFFERS, SND_BUFFER_SIZE, buffers);

    signalMask = spGetPlayerSignalMask(musicPlayer);

    while (1)
    {
        WaitSignal(musicSignal);

        channels[MUSIC_CHANNEL].playing = 1;

        track = musicTrack;
        {
            char path[32];
            strcpy(path, "audio/");
            getTrackStrNo(path + 6, musicTrack + 1);
            strcat(path, ".aifc");

            spAddSoundFile(&music, musicPlayer, path);
        }

        spStartReading(music, SP_MARKER_NAME_BEGIN);
        spStartPlayingVA(musicPlayer, AF_TAG_AMPLITUDE, 0x7FFF, TAG_END);

        while (spGetPlayerStatus(musicPlayer) & SP_STATUS_F_BUFFER_ACTIVE)
        {
            int32 signal = WaitSignal(signalMask);
            if (spService(musicPlayer, signal) < 0)
                break;

            if (track != musicTrack)
            {
                spStop(musicPlayer);
            }
        }

        spRemoveSound(music);
        channels[MUSIC_CHANNEL].playing = 0;
    }
}

void soundInit(void)
{
    int32 i;
    char* LGainName = "LeftGain0";
    char* RGainName = "RightGain0";
    char* InputName = "Input0";

    #if MAX_CHANNELS != 7
        #error change the sound mixer or channels count
    #endif

    gMusicDRAM = (uint8*)allocDRAM(SND_BUFFERS * SND_BUFFER_SIZE);

    sndMixer = LoadInstrument("mixer8x2.dsp", 0, 100);

    // mono samples
    for (i = 0; i < MUSIC_CHANNEL; i++)
    {
        Channel* ch = channels + i;

        LGainName[8] = '0' + i;
        RGainName[9] = '0' + i;
        InputName[5] = '0' + i;

        ch->gainL = GrabKnob(sndMixer, LGainName);
        ch->gainR = GrabKnob(sndMixer, RGainName);

        ch->instrument = LoadInstrument("dcsqxdvarmono.dsp", 0, 100);
        ConnectInstruments(ch->instrument, "Output", sndMixer, InputName);

        ch->frequency = GrabKnob(ch->instrument, "Frequency");
        ch->amplitude = GrabKnob(ch->instrument, "Amplitude");

        TweakKnob(ch->frequency, 0x2000);
        TweakKnob(ch->amplitude, 0x7FFF);

        TweakKnob(ch->gainL, 0x7FFF);
        TweakKnob(ch->gainR, 0x7FFF);

        ch->id = SFX_MAX;
        ch->playing = 0;
    }

    // stereo music
    {
        Item gainTemp;
        Channel* ch = channels + MUSIC_CHANNEL;

        ch->instrument = LoadInstrument("dcsqxdstereo.dsp", 0, 100);

        // left channel
        LGainName[8] = '0' + (MUSIC_CHANNEL + 0);
        RGainName[9] = '0' + (MUSIC_CHANNEL + 0);
        InputName[5] = '0' + (MUSIC_CHANNEL + 0);
        ch->gainL = GrabKnob(sndMixer, LGainName);
        gainTemp = GrabKnob(sndMixer, RGainName);
        TweakKnob(gainTemp, 0);
        ConnectInstruments(ch->instrument, "LeftOutput", sndMixer, InputName);

        // right channel
        LGainName[8] = '0' + (MUSIC_CHANNEL + 1);
        RGainName[9] = '0' + (MUSIC_CHANNEL + 1);
        InputName[5] = '0' + (MUSIC_CHANNEL + 1);
        gainTemp = GrabKnob(sndMixer, LGainName);
        ch->gainR = GrabKnob(sndMixer, RGainName);
        TweakKnob(gainTemp, 0);
        ConnectInstruments(ch->instrument, "RightOutput", sndMixer, InputName);

        TweakKnob(ch->gainL, 0x7FFF);
        TweakKnob(ch->gainR, 0x7FFF);

        ch->frequency = -1;
        ch->amplitude = GrabKnob(ch->instrument, "Amplitude");

        TweakKnob(ch->amplitude, 0x7FFF);

        ch->id = SFX_MAX;
        ch->playing = 0;
    }

    StartInstrument(sndMixer, NULL);

    musicThread = CreateThread("music", 180, musicProc, 2048);
}

int WhatChannel(enum SfxType id) { return 0; }
int IsPlaying(int channel) { return 0; }

void InitNotes(void)
{
    int32 i, frames;
    uint8* data = sdxData;

    for (i = 0; i < SFX_MAX; i++)
    {
        frames = *(int32*)data;

        samples[i].size = frames >> 1;
        samples[i].sample = CreateSampleVA(
            AF_TAG_FRAMES, frames,
            AF_TAG_ADDRESS, (uint8*)data + 4,
            AF_TAG_CHANNELS, 1,
            AF_TAG_WIDTH, 2,
            AF_TAG_COMPRESSIONTYPE, ID_SDX2,
            AF_TAG_COMPRESSIONRATIO, 2,
            TAG_END);

        data += (4 + frames + 3) & ~3;
    }
}

void KillNote(enum SfxType id)
{
    int i;
    Channel* ch;

    for (i = 0; i < MUSIC_CHANNEL; i++)
    {
        ch = channels + i;

        if (ch->playing && ch->id == id)
        {
            StopInstrument(ch->instrument, NULL);
            DetachSample(ch->attach);
            ch->playing = 0;
        }
    }
}

void KillNotes(void)
{
    int i;
    Channel* ch;

    for (i = 0; i < MUSIC_CHANNEL; i++)
    {
        ch = channels + i;

        if (ch->playing)
        {
            StopInstrument(ch->instrument, NULL);
            DetachSample(ch->attach);
            ch->playing = 0;
        }
    }
}

int PlayNote(enum SfxType id)
{
    Channel* ch;
    int i, pos;
    int32 maxPos = -2;
    int32 maxPosIndex = -1;

    if (id == SFX_REMOTE_ENG || id == SFX_ENG_RUMBLE || id == SFX_ENG_AIR_INTAKE || id == SFX_ENG_THRUST || id == SFX_TURBULENCE)
    {
        return -1;
    }

// update playing status
    for (i = 0; i < SND_CHANNELS; i++)
    {
        ch = channels + i;
        if (!ch->playing)
            continue;

        if (gSfxParams[ch->id].loop)
            continue;

        pos = WhereAttachment(ch->attach);
        if (pos == -1 || pos >= samples[ch->id].size)
        {
            ch->playing = 0;
        }

        if (maxPos < pos) {
            maxPos = pos;
            maxPosIndex = i;
        }
    }

    for (i = 0; i < MUSIC_CHANNEL; i++)
    {
        ch = channels + i;

        if (ch->playing)
            continue;

        if (ch->id != SFX_MAX)
        {
            StopInstrument(ch->instrument, NULL);
            DetachSample(ch->attach);
        }

        TweakKnob(ch->gainL, gSfxParams[id].volL);
        TweakKnob(ch->gainR, gSfxParams[id].volR);
        TweakKnob(ch->frequency, gSfxParams[id].pitch << 1);

        ch->attach = AttachSample(ch->instrument, samples[id].sample, NULL);
        ch->id = id;
        ch->playing = 1;

        if (gSfxParams[id].loop)
        {
            LinkAttachments(ch->attach, ch->attach);
        }

        StartInstrument(ch->instrument, NULL);

        return i;
    }

    return -1;
}

void SetMasterBalance(int32 value)
{
    int32 i, volume;

    // TODO convert value to log

    if (value < 0)
    {
        value = 0;
    }
    else if (value > 256)
    {
        value = 256;
    }
    gMasterBalance = value;

    // tweak mixer level for samples
    volume = (256 - value) << 7;
    if (volume > 0x7FFF)
    {
        volume = 0x7FFF;
    }
    for (i = 0; i < MUSIC_CHANNEL; i++)
    {
        TweakKnob(channels[i].amplitude, volume);
    }

    // tweak mixer level for music
    volume = value << 7;
    if (volume > 0x7FFF)
    {
        volume = 0x7FFF;
    }
    TweakKnob(channels[MUSIC_CHANNEL].amplitude, volume);
}

void SetPitch(enum SfxType id, int32 pitch)
{
    int i;

    if (pitch < 0x0400)
        pitch = 0x0400;
    if (pitch > 0x3FFF)
        pitch = 0x3FFF;

    if (gSfxParams[id].pitch == pitch)
        return;
    gSfxParams[id].pitch = pitch;

    for (i = 0; i < MUSIC_CHANNEL; i++)
    {
        Channel* ch = channels + i;

        if (ch->id == id && ch->playing)
        {
            TweakKnob(ch->frequency, pitch << 1);
        }
    }
}

void SetVolume(enum SfxType id, int32 volL, int32 volR)
{
    int i;

    if (volL > 0x7FFF)
    {
        volL = 0x7FFF;
    }

    if (volR > 0x7FFF)
    {
        volR = 0x7FFF;
    }

    if ((gSfxParams[id].volL == volL) && (gSfxParams[id].volR == volR))
        return;
    gSfxParams[id].volL = volL;
    gSfxParams[id].volR = volR;

    for (i = 0; i < MUSIC_CHANNEL; i++)
    {
        Channel* ch = channels + i;

        if (ch->id == id && ch->playing)
        {
            TweakKnob(ch->gainL, volL);
            TweakKnob(ch->gainR, volR);
        }
    }
}

void FillNotes(int16* samples, int32 count) {}

int MusicIsPlaying(void)
{
    return channels[MUSIC_CHANNEL].playing;
}

int MusicGetTrack(void)
{
    return musicTrack;
}

void MusicPlay(int track)
{
    musicTrack = track;

    if (track >= 0) {
        SendSignal(musicThread, musicSignal);
    }
}

void MusicStop(void)
{
    MusicPlay(-1);
}

void MusicReset(void)
{
    srand(gFrameIndex);
    MusicStop();
}
