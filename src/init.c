#include "game.h"

const int skyPos[15] = { -820, -2520, -1930, -5000, -5000, 0, -2260, -40, -2700, 0, -240, -2120, -2700, 0, 0 };

short InitScene(Object** prm, short* prmCount, Skeleton* camPos, ConfigData* gameData)
{
    int sceneTextures;
    int skyTextures;
    int i;
    Object* obj;
    int sky;
    char trackPath[64];

    sceneTextures = TextureTableCount;
    getPath(trackPath, gameData->actTrackNum, "/scene.cmp");
    LoadCompressedTextureSequence(trackPath, 1);

    skyTextures = TextureTableCount;
    getPath(trackPath, gameData->actTrackNum, "/sky.cmp");
    LoadCompressedTextureSequence(trackPath, 1);

    getPath(trackPath, gameData->actTrackNum, "/scene.prm");

    /*
       objectTable = LoadPrm( trackPath , sceneTextures );
    */
    prm[(*prmCount)++] = LoadPrm(trackPath, sceneTextures);

    for (i = 0; i < *prmCount; i++)
    {
        obj = prm[i];
        while (obj)
        {
            obj->skeleton->super = camPos;
            obj = obj->next;
        }
    }

    sky = *prmCount;
    getPath(trackPath, gameData->actTrackNum, "/sky.prm");
    prm[(*prmCount)++] = LoadPrm(trackPath, skyTextures);
    obj = prm[sky];
    /*   while ( obj )   */
    {
        obj->skeleton->super = camPos->super;
        /*   obj = obj->next; */
    }
    SetSkeletonPosition(prm[sky]->skeleton, 0, skyPos[trackNo - 1], 0);

    SceneRadiusCheck(prm[0]);

    return (sky);
}

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
        trackStr[1] = '0' + (trackNo % 10);
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

void LoadCmpFiles(TIMlist* timPtr, int numTex)
{
    int i;

    ASSERT(TextureTableCount + numTex <= TextureTableMax);

    for (i = 0; i < numTex; i++)
    {
        TextureTable[TextureTableCount++] = renderCreateTexture((Tim*)(timPtr->memOffset[i]));
    }
}

uint8 gCompBuffer[128 * 1024];

void LoadCompressedTextureSequence(const char* fileName, int filter)
{
    int texNum;
    int numTex;
    static TIMlist timList;
    long timSize, timTotalSize;

    osLoadFile(fileName, (char*)gCompBuffer);
    numTex = SwapLE32(*(int*)gCompBuffer);

    timTotalSize = 0;

    /***** Set offsets from timList.memBase & allocate memory for all tims *****/

    for (texNum = 0; texNum < numTex; texNum++)
    {
        timList.memOffset[texNum] = timTotalSize;
        timSize = SwapLE32(*(int*)(gCompBuffer + 4 + texNum * 4));
        timTotalSize += timSize;
    }

    timList.memBase = allocVRAM(timTotalSize);

    for (texNum = 0; texNum < numTex; texNum++)
    {
        timList.memOffset[texNum] += (long)timList.memBase;
    }

    ExpandData((uint8*)gCompBuffer + 4 + numTex * 4, (uint8*)timList.memBase);

    HACK_TEX_FILTER = filter;
    LoadCmpFiles(&timList, numTex);
    HACK_TEX_FILTER = 0;
}

Texture* LoadTexture(const char* fileName)
{
    long length;
    Tim* tim;
    Texture* tex;

    length = osLoadFile(fileName, pVRAM);
    if (length <= 0)
    {
        ASSERT(0);
        return NULL;
    }

    tim = (Tim*)allocVRAM(length);

    tex = renderCreateTexture(tim);

    return tex;
}
