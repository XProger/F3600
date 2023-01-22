#include "game.h"
#include "../../libs/stb_vorbis/stb_vorbis.c"

#define MAX_CHANNELS    16
#define PITCH_SHIFT     12
#define VAG_BLOCK_END   (28 << PITCH_SHIFT)
#define OGG_BLOCK_END   (1024 << PITCH_SHIFT)

typedef struct
{
    enum SfxType id;
    uint8* data;
    int32 pos;
    int32 size;
    int32 s1, s2;
    int32 blockPos;
    int16 block[VAG_BLOCK_END >> PITCH_SHIFT];
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
    int32 pitch;
    int32 blockPos;
    int32 blockSize;
    int16 block[2 * (OGG_BLOCK_END >> PITCH_SHIFT)];
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
    src++; // skip flags
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
    int32 i;

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
    music.alloc.alloc_buffer = (char*)music.temp;
}

void KillNote(enum SfxType id)
{
    int32 i = WhatChannel(id);

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
    int32 i;

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
        channel->blockPos = VAG_BLOCK_END;

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

#if SND_OUTPUT_FREQ != 44100
    pitch = pitch * 44100 / SND_OUTPUT_FREQ;
#endif

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

void MusicPlay(int track)
{
    int32 size;
    char path[16];

    ASSERT(track > 0);
    if (track == music.track)
    {
        if (music.ogg)
        {
            stb_vorbis_seek_start(music.ogg);
        }
        return;
    }
    music.track = track;

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
    getTrackStrNo(path + 6, track + 1);
    strcat(path, ".ogg");

    size = osLoadFile(path, NULL);
    if (size == 0)
        return;

    music.blockPos = music.blockSize = OGG_BLOCK_END;
    music.pitch = (1 << PITCH_SHIFT) * 44100 / SND_OUTPUT_FREQ;
    music.data = (uint8*)malloc(size);

    osLoadFile(path, music.data);

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

void decodeVAG(int32* dst, Channel* channel, int32 count, int32 volume)
{
    int32 i, value;

    int32 volL = gSfxParams[channel->id].volL * volume >> 8;
    int32 volR = gSfxParams[channel->id].volR * volume >> 8;
    int32 blockPos = channel->blockPos;
    int32 pitch = gSfxParams[channel->id].pitch;

    for (i = 0; i < count; i++)
    {
        if (blockPos >= VAG_BLOCK_END)
        {
            blockPos -= VAG_BLOCK_END;
            ASSERT(blockPos < VAG_BLOCK_END);

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

        *dst++ += value * volL >> 15;
        *dst++ += value * volR >> 15;

        blockPos += pitch;
    }

    channel->blockPos = blockPos;
}

void decodeWAV(Channel* channel, int32 count, int32 volume)
{
    // TODO
}

int32 decodeOGG(int32* dst, int32 count, int32 volume)
{
    int32 blockPos = music.blockPos;
    int32 blockSize = music.blockSize;
    int32 pitch = music.pitch;

    while (count--)
    {
        if (blockPos >= blockSize)
        {
            if (blockSize != OGG_BLOCK_END)
            {
                music.blockPos = 0;
                music.blockSize = 0;
                return 0;
            }

            blockPos -= OGG_BLOCK_END;
            blockSize = stb_vorbis_get_samples_short_interleaved(music.ogg, 2, music.block, 2 * (OGG_BLOCK_END >> PITCH_SHIFT)) << PITCH_SHIFT;
        }

        // TODO resample (lerp)
        *dst++ = music.block[((blockPos >> PITCH_SHIFT) << 1) + 0] * volume >> 8;
        *dst++ = music.block[((blockPos >> PITCH_SHIFT) << 1) + 1] * volume >> 8;

        blockPos += pitch;
    }

    music.blockPos = blockPos;
    music.blockSize = blockSize;

    return 1;
}

void FillNotes(int16* samples, int32 count)
{
    Channel* channel;
    int32 i;
    int32* samples32 = gFrames;

    ASSERT(count == SND_SAMPLES);

    memset(gFrames, 0, sizeof(gFrames));

    // music
    if (gMasterBalance > 0)
    {
        if (music.playing)
        {
            music.playing = decodeOGG(samples32, count, gMasterBalance);
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

            // TODO resample (lerp)
            decodeVAG(samples32, channel, count, 256 - gMasterBalance);
        }
    }

    for (i = 0; i < count; i++)
    {
        *samples++ = clamp(*samples32++);
        *samples++ = clamp(*samples32++);
    }
}
