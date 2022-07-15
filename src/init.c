#include "game.h"

int FontLoadHack;

const int skyPos[15] = { -820, -2520, -1930, -5000, -5000, 0, -2260, -40, -2700, 0, -240, -2120, -2700, 0, 0 };
const uint16 RED_TEXT_CLUT[] = { 0x0000, 0x0043, 0x0465, 0x0487, 0x04ca, 0x090c, 0x092e, 0x0970, 0x0db2, 0x0dd4, 0x0e16, 0x0e57, 0x0e79, 0x0e9a, 0x0edc, 0x0f1e };

short InitScene(Object** prm, short* prmCount, Skeleton* camPos, ConfigData* gameData)
{
    int sceneTextures;
    int skyTextures;
    int i;
    Object* obj;
    int sky;
    char trackPath[64];

    sceneTextures = TextureTableCount;
    getPath(trackPath, gameData->actTrackNum, "\\scene.cmp");
    LoadCompressedTextureSequence(trackPath);

    skyTextures = TextureTableCount;
    getPath(trackPath, gameData->actTrackNum, "\\sky.cmp");
    LoadCompressedTextureSequence(trackPath);

    getPath(trackPath, gameData->actTrackNum, "\\scene.prm");

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
    getPath(trackPath, gameData->actTrackNum, "\\sky.prm");
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

    strcpy(trackPath, "wipeout\\track");
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

    if (FontLoadHack)
    {
        for (i = 0; i < 10; i++)
        {
            Tim4* tim = (Tim4*)(timPtr->memOffset[i]);
            memcpy(tim->clut, RED_TEXT_CLUT, sizeof(RED_TEXT_CLUT));
            TextureTable[TextureTableCount++] = renderCreateTexture((Tim*)tim);
        }
    }
}

void LoadCompressedTextureSequence(const char* fileName)
{
    int texNum;
    int numTex;
    char* fileBuffer;
    long fileSize;
    static TIMlist timList;
    long timSize, timTotalSize;

    fileSize = FileLength(fileName);
    if (fileSize <= 0)
    {
        ASSERT(0);
    }

    fileBuffer = (char*)malloc(fileSize + 32);

    if (!fileBuffer)
    {
        ASSERT(0);
    }
    if (LoadFile(fileName, (char*)fileBuffer) != fileSize)
    {
        ASSERT(0);
    }

    numTex = *(long*)fileBuffer;

    timTotalSize = 0;

    /***** Set offsets from timList.memBase & allocate memory for all tims *****/

    for (texNum = 0; texNum < numTex; texNum++)
    {
        timList.memOffset[texNum] = timTotalSize;
        timSize = *((long*)(fileBuffer + 4 + texNum * 4));
        timTotalSize += timSize;
    }

    timList.memBase = (char*)malloc(timTotalSize);

    for (texNum = 0; texNum < numTex; texNum++)
    {
        timList.memOffset[texNum] += (long)timList.memBase;
    }

    ExpandData((uint8*)fileBuffer + 4 + numTex * 4, (uint8*)timList.memBase);

    free(fileBuffer); /* free compressed file memory */

    LoadCmpFiles(&timList, numTex);

    free(timList.memBase);
}

Texture* LoadTexture(const char* fileName)
{
    long length;
    Tim* tim;
    Texture* tex;

    length = FileLength(fileName);
    if (length <= 0)
    {
        ASSERT(0);
        return NULL;
    }

    tim = (Tim*)malloc(length + 32);

    if (!tim)
    {
        ASSERT(0);
    }

    if (LoadFile(fileName, (char*)tim) != length)
    {
        /* File Error! */
        ASSERT(0);
    }

    tex = renderCreateTexture(tim);

    free(tim);

    return tex;
}
