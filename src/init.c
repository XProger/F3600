#include "game.h"

void getTrackStrNo(char* trackStr, char trackNo)
{
    if (trackNo <= 9)
    {
        trackStr[0] = '0';
        trackStr[1] = '0' + trackNo;
    }
    else
    {
        trackStr[0] = '1';
        trackStr[1] = '0' + (trackNo - 10);
    }
    trackStr[2] = 0;
}

void getPath(char* trackPath, char trackNo, char* name)
{
    char No[3];

    strcpy(trackPath, "wipeout/track");
    getTrackStrNo(No, trackNo);
    strcat(trackPath, No);
    strcat(trackPath, name);
}

int32 getTexSize(uint8* ptr)
{
    uint32 info = *(uint32*)ptr;
    uint32 type = GET_TEX_TYPE(info);
    int32 size = GET_TEX_WIDTH(info) * GET_TEX_HEIGHT(info);

    switch (type)
    {
        case TEX_TYPE_4:
        {
            size = 4 + 16 * sizeof(uint16) + (size >> 1);
            break;
        }        
        case TEX_TYPE_16:
        {
            size = 4 + (size << 1);
            break;
        }
    }

    return size;
}

#ifdef USE_COMPRESSION

#ifdef _DEBUG
void convertTim3DO(FILE* f, Tim* tim)
{
    int32 i, j, w, h, iw;
    uint32 info;
    uint8 zero = 0;

    if (TimType(tim) == TIM_4)
    {
        Tim4* t = &tim->tim4;
        uint8* src = (uint8*)(t + 1);

        w = t->textureW * 4;
        h = t->textureH;

        iw = (w + 7) & ~7;

        info = INIT_TEX_INFO(iw, h, TEX_TYPE_4);

        IntelLong(&info);
        fwrite(&info, sizeof(info), 1, f);

        for (i = 0; i < 16; i++)
        {
            uint16 p = t->clut[i];
            uint8 r = p & 31;
            uint8 g = (p >> 5) & 31;
            uint8 b = (p >> 10) & 31;
            p = b | (g << 5) | (r << 10);
            IntelShort(&p);
            fwrite(&p, sizeof(p), 1, f);
        }

        for (i = 0; i < h; i++)
        {
            uint8 index;

            for (j = 0; j < (w >> 1); j++)
            {
                index = *src++;
                index = ((index & 15) << 4) | (index >> 4);
                fwrite(&index, sizeof(index), 1, f);
            }

            index = 0;
            for (j = (w >> 1); j < (iw >> 1); j++)
            {
                fwrite(&index, sizeof(index), 1, f);
            }
        }
    }
    else if (TimType(tim) == TIM_8) // TODO 8-bpp textures for 3DO
    {
        Tim8* t = &tim->tim8;
        uint8* src = (uint8*)(t + 1);

        w = t->textureW * 2;
        h = t->textureH;

        iw = (w + 1) & ~1;

        info = INIT_TEX_INFO(iw, h, TEX_TYPE_16);
        IntelLong(&info);
        fwrite(&info, sizeof(info), 1, f);

        for (i = 0; i < h; i++)
        {
            uint16 p;

            for (j = 0; j < w; j++)
            {
                p = t->clut[*src++];
                uint8 r = p & 31;
                uint8 g = (p >> 5) & 31;
                uint8 b = (p >> 10) & 31;
                p = b | (g << 5) | (r << 10);
                IntelShort(&p);
                fwrite(&p, sizeof(p), 1, f);
            }

            p = 0;
            for (j = w; j < iw; j++)
            {
                fwrite(&p, sizeof(p), 1, f);
            }
        }
    }
    else if (TimType(tim) == TIM_16)
    {
        Tim16* t = &tim->tim16;
        uint16* src = (uint16*)(t + 1);

        w = t->textureW;
        h = t->textureH;

        iw = (w + 1) & ~1;

        info = INIT_TEX_INFO(iw, h, TEX_TYPE_16);
        IntelLong(&info);
        fwrite(&info, sizeof(info), 1, f);

        for (i = 0; i < h; i++)
        {
            uint16 p;

            for (j = 0; j < w; j++)
            {
                p = *src++;
                uint8 r = p & 31;
                uint8 g = (p >> 5) & 31;
                uint8 b = (p >> 10) & 31;
                p = b | (g << 5) | (r << 10);
                IntelShort(&p);
                fwrite(&p, sizeof(p), 1, f);
            }

            p = 0;
            for (j = w; j < iw; j++)
            {
                fwrite(&p, sizeof(p), 1, f);
            }
        }
    }
    else
    {
        ASSERT(0);
    }
}
#endif

void LoadCompressedTextureSequence(const char* fileName, int filter)
{
    int i;
    int numTex;
    int timSize, timTotalSize;
    char* compBuffer = pDRAM; // use available DRAM as scratch buffer

    osLoadFile(fileName, compBuffer);
    numTex = *(int*)compBuffer;
    SwapLE32(numTex);

    ExpandData((uint8*)compBuffer + 4 + numTex * 4, (uint8*)pVRAM);

    timTotalSize = 0;
    HACK_TEX_FILTER = filter;
    for (i = 0; i < numTex; i++)
    {
        renderCreateTexture((uint8*)(pVRAM + timTotalSize));
        timSize = *(int*)(compBuffer + 4 + i * 4);
        SwapLE32(timSize);
        timTotalSize += timSize;
    }
    HACK_TEX_FILTER = 0;

#ifdef _DEBUG
    {
        FILE* f;
        char path[256];
        int32 numTexBE = numTex;

        strcpy(path, "../src/platform/3do/CD/");
        strcat(path, fileName);

        f = fopen(path, "wb");

        IntelLong(&numTexBE);
        fwrite(&numTexBE, 1, 4, f);

        timTotalSize = 0;
        for (i = 0; i < numTex; i++)
        {
            convertTim3DO(f, (Tim*)(pVRAM + timTotalSize));

            timSize = *(int*)(compBuffer + 4 + i * 4);
            SwapLE32(timSize);
            timTotalSize += timSize;
        }

        fclose(f);
    }
#endif

    allocVRAM(timTotalSize);
}
#else
void LoadCompressedTextureSequence(const char* fileName, int filter)
{
    int32 i;
    int32 size;
    uint8* ptr;
    int32 texCount;

    size = osLoadFile(fileName, pVRAM);
    ptr = (uint8*)allocVRAM(size);

    texCount = *(int32*)ptr;
    ptr += 4;

    for (i = 0; i < texCount; i++)
    {
        renderCreateTexture(ptr);
        ptr += getTexSize(ptr);
    }
}
#endif

Texture* LoadTexture(const char* fileName)
{
    long size;
    Tim* tim;
    Texture* tex = gTexturePtr;

    size = osLoadFile(fileName, pVRAM);
    tim = (Tim*)allocVRAM(size);

    renderCreateTexture((uint8*)tim);

#ifdef _DEBUG
    {
        FILE* f;
        char path[256];

        strcpy(path, "../src/platform/3do/CD/");
        strcat(path, fileName);

        f = fopen(path, "wb");
        convertTim3DO(f, tim);
        fclose(f);
    }
#endif

    return tex;
}
