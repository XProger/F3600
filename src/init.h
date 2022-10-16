short InitScene(Object** prm, short* prmCount, Skeleton* camPos, ConfigData* gameData);
void getTrackStrNo(char* trackStr, char trackNo);
void getPath(char* trackPath, char trackNo, char* name);
void LoadCompressedTextureSequence(const char* fileName, int filter);
Texture* LoadTexture(const char* fileName);
