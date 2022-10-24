typedef struct
{
    short tex16[16];
    short tex4[4];
    short tex1[1];
} TexTemplate;

extern TexTemplate* ctrltexTemplate;
extern short ctrlnumLib;
extern short ctrllibraryTextures;

TexTemplate* LoadTtfFile(char* file);
