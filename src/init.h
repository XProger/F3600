typedef struct _TIMlist
{
    void* memBase;
    long memOffset[400];
} TIMlist;

short InitScene(Object** prm, short* prmCount, Skeleton* camPos, ConfigData* gameData);
void getTrackStrNo(char* trackStr, char trackNo);
void getPath(char* trackPath, char trackNo, char* name);
void LoadCmpFiles(TIMlist* timPtr);
TIMlist* LoadCompressedTextureSequence(const char* fileName);
Texture* LoadTexture(const char* fileName);
