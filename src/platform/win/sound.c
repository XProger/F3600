#include "game.h"
#include "stb_vorbis/stb_vorbis.c"

#define MAX_CHANNELS 16
#define PITCH_SHIFT  12
#define BLOCK_END    (28 << PITCH_SHIFT)

typedef struct
{
    enum SfxType id;
    uint8* data;
    int32 pos;
    int32 size;
    int32 s1, s2;
    int32 blockPos;
    int16 block[28];
} Channel;

int32 gFrames[SND_SAMPLES * 2];
Channel gChannels[MAX_CHANNELS];

extern uint8 vagData[29 * 16 * 1024]; // should be enough
extern uint16 vagOffset[SFX_MAX];
extern uint16 vagSize[SFX_MAX];

const int8 SPU_POS[] = { 0, 60, 115,  98, 122 };
const int8 SPU_NEG[] = { 0,  0, -52, -55, -60 };

typedef struct
{
    stb_vorbis* ogg;
    uint8* data;
    stb_vorbis_alloc alloc;
    int32 track;
    int32 playing;
    uint8 temp[256 * 1024];
} Music;

Music music;

int32 clamp(int32 value)
{
    if (value < -32768)
        return -32768;
    if (value > 32767)
        return 32767;
    return value;
}

int16 vagPredicate(int16 value, uint8 pred, uint8 shift, int32* s1, int32* s2)
{
    int32 s = ((*s1) * SPU_POS[pred] + (*s2) * SPU_NEG[pred]) >> 6;
    s += (value >> shift);
    s = clamp(s);

    *s2 = *s1;
    *s1 = s;

    return s;
}

void vagProcessBlock(Channel* channel, int16* dst)
{
    int32 i;
    uint8 d;
    uint8* src = channel->data + channel->pos;
    int32 s1 = channel->s1;
    int32 s2 = channel->s2;

    uint8 pred = *src++;
    uint8 flags = *src++;
    uint8 shift = pred & 0x0F;
    pred >>= 4;

    for (i = 0; i < 14; i++)
    {
        d = *src++;
        *dst++ = vagPredicate((d & 0x0F) << 12, pred, shift, &s1, &s2);
        *dst++ = vagPredicate((d & 0xF0) <<  8, pred, shift, &s1, &s2);
    }

    channel->pos += 16;
    channel->s1 = s1;
    channel->s2 = s2;
}

#if 0
void dump_vags()
{
    typedef struct
    {
        char RIFF[4];
        uint32 rSize;
        char WAVE[4];
        char fmt[4];
        uint32 wSize;

        uint16 format;
        uint16 channels;
        uint32 samplesPerSec;
        uint32 bytesPerSec;
        uint16 block;
        uint16 sampleBits;

        char data[4];
        uint32 dataSize;
    } WaveHeader;

    int32 i;
    FILE* f;
    int32 size;
    uint8* data;
    int16 raw[28];
    char buf[64];

    WaveHeader wav = {
        { 'R', 'I', 'F', 'F' }, 0,
        { 'W', 'A', 'V', 'E' }, { 'f', 'm', 't', ' ' }, 16,
        1, 1, 44100, 44100 * 16 / 8, 0, 16,
        { 'd', 'a', 't', 'a' }, 0
    };

    for (i = 0; i < MAX_VAGS; i++)
    {
        size = vagSize[i] * 8;
        wav.dataSize = (size / 16) * 28 * sizeof(int16);
        wav.rSize = sizeof(WaveHeader) - 8 + wav.dataSize;

        sprintf(buf, "wipeout/sound/vags/%d.wav", i);

        f = fopen(buf, "wb");
        ASSERT(f != 0);
        fwrite(&wav, sizeof(wav), 1, f);

        data = vagData + vagOffset[i] * 8;
        vagReset();
        while (size > 0)
        {
            vagProcessBlock(data, raw);
            fwrite(raw, sizeof(int16), 28, f);
            data += 16;
            size -= 16;
        }

        fclose(f);
    }
}
#endif

int WhatChannel(enum SfxType id)
{
    int i;

    for (i = 0; i < MAX_CHANNELS; i++)
    {
        if ((gChannels[i].data != NULL) && (gChannels[i].id == id))
            return i;
    }

    return -1;
}

int IsPlaying(int channel)
{
    if (channel < 0 || channel >= MAX_CHANNELS)
        return 0;
    return (gChannels[channel].data ? 1 : 0);
}

void InitNotes(void)
{
    music.alloc.alloc_buffer_length_in_bytes = sizeof(music.temp);
    music.alloc.alloc_buffer = music.temp;
}

void KillNote(enum SfxType id)
{
    int i = WhatChannel(id);

    if (IsPlaying(i))
    {
        gChannels[i].data = NULL;
        gChannels[i].blockPos = 0;
    }
}

void KillNotes(void)
{
    int32 i;

    for (i = 0; i < MAX_CHANNELS; i++)
    {
        gChannels[i].data = NULL;
        gChannels[i].blockPos = 0;
    }
}

int PlayNote(enum SfxType id)
{
    Channel* channel;
    int i;

    for (i = 0; i < MAX_CHANNELS; i++)
    {
        channel = gChannels + i;

        if (channel->data)
            continue;

        channel->id = id;
        channel->data = vagData + vagOffset[id] * 8;
        channel->pos = 0;
        channel->size = vagSize[id] * 8;
        channel->s1 = 0;
        channel->s2 = 0;
        channel->blockPos = BLOCK_END;

        return i;
    }

    return -1;
}

void SetMasterBalance(int32 value)
{
    if (value < 0)
    {
        value = 0;
    }
    else if (value > 256)
    {
        value = 256;
    }
    gMasterBalance = value;
}

void SetPitch(enum SfxType id, int32 pitch)
{
    if (pitch < 0x0400)
        pitch = 0x0400;
    if (pitch > 0x3FFF)
        pitch = 0x3FFF;

    gSfxParams[id].pitch = pitch;
}

void SetVolume(enum SfxType id, int32 volL, int32 volR)
{
    ASSERT(volL >= 0 && volL <= SND_MAX_VOLUME);
    ASSERT(volR >= 0 && volR <= SND_MAX_VOLUME);

    gSfxParams[id].volL = volL;
    gSfxParams[id].volR = volR;
}

int MusicIsPlaying(void)
{
    return music.playing;
}

int MusicGetTrack(void)
{
    return music.track;
}

void MusicPlay(int musicTrack)
{
    FILE* f;
    int32 size;
    char path[16];

    ASSERT(musicTrack > 0);
    if (musicTrack == music.track)
    {
        stb_vorbis_seek_start(music.ogg);
        return;
    }
    music.track = musicTrack;

    if (music.ogg)
    {
        stb_vorbis_close(music.ogg);
        music.ogg = NULL;
    }

    if (music.data)
    {
        free(music.data);
        music.data = NULL;
    }

    strcpy(path, "audio/");
    getTrackStrNo(path + 6, musicTrack + 1);
    strcat(path, ".ogg");

    f = fopen(path, "rb");
    ASSERT(f);
    if (!f)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    music.data = (uint8*)malloc(size);
    fread(music.data, 1, size, f);
    fclose(f);

    music.ogg = stb_vorbis_open_memory(music.data, size, NULL, &music.alloc);
    ASSERT(music.ogg);
    music.playing = 1;
}

void MusicStop()
{
    music.playing = 0;
}

void MusicReset(void)
{
    //MusicStop(); // music isn't CD based so we don't need to stop streaming during loading time
}

void RenderVAG(Channel* channel, int32 count, int32 volume)
{
    int32 i, j, value;

    int32* samples32 = gFrames;
    int32 volL = gSfxParams[channel->id].volL * volume >> 8;
    int32 volR = gSfxParams[channel->id].volR * volume >> 8;
    int32 blockPos = channel->blockPos;
    int32 posInc = gSfxParams[channel->id].pitch;

    for (i = 0; i < count; i++)
    {
        if (blockPos >= BLOCK_END)
        {
            blockPos -= BLOCK_END;
            ASSERT(blockPos < BLOCK_END);

            if (channel->pos == channel->size)
            {
                if (gSfxParams[channel->id].loop)
                {
                    channel->pos = 0;
                    channel->s1 = 0;
                    channel->s2 = 0;
                    blockPos = 0;
                }
                else
                {
                    channel->data = NULL;
                    blockPos = 0;
                    break;
                }
            }

            vagProcessBlock(channel, channel->block);
        }

        value = channel->block[blockPos >> PITCH_SHIFT];

        *samples32++ += value * volL >> 15;
        *samples32++ += value * volR >> 15;

        blockPos += posInc;
    }

    channel->blockPos = blockPos;
}

void RenderWAV(Channel* channel, int32 count, int32 volume)
{
    // TODO
}

int32 RenderOGG(int16* temp, int32 count, int32 volume)
{
    int i, j;
    int32* samples32 = gFrames;

    i = 0;
    while (i < count)
    {
        int32 res = stb_vorbis_get_samples_short_interleaved(music.ogg, 2, temp, (count - i) * 2);
        if (!res)
            return 0;
        i += res;

        for (j = 0; j < res * 2; j += 2)
        {
            *samples32++ = temp[j + 0] * volume >> 8;
            *samples32++ = temp[j + 1] * volume >> 8;
        }
    }

    return 1;
}

void FillNotes(int16* samples, int32 count)
{
    Channel* channel;
    int32 i, j;
    int32* samples32 = gFrames;

    memset(gFrames, 0, sizeof(gFrames));

    // music
    if (gMasterBalance > 0)
    {
        if (music.playing)
        {
            music.playing = RenderOGG(samples, count, gMasterBalance);
        }
    }

    // sfx
    if (gMasterBalance < 256)
    {
        for (i = 0; i < MAX_CHANNELS; i++)
        {
            channel = gChannels + i;

            if (!channel->data)
                continue;
            
            if (!gSfxParams[channel->id].volL && !gSfxParams[channel->id].volR)
            {
                if (!gSfxParams[channel->id].loop)
                {
                    channel->data = NULL;
                }
                continue;
            }

            RenderVAG(channel, count, 256 - gMasterBalance);
        }
    }

    samples32 = gFrames;
    for (i = 0; i < count; i++)
    {
        *samples++ = clamp(*samples32++);
        *samples++ = clamp(*samples32++);
    }
}