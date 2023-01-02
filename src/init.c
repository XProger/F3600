#include "game.h"

#ifdef USE_COMPRESSION
void LoadCompressedTextureSequence(const char* fileName, int filter)
{
    uint32 i;
    uint32 numTex;
    uint32 timSize, timTotalSize;
    uint8* compBuffer = pDRAM; // use available DRAM as scratch buffer

    osLoadFile(fileName, compBuffer);
    numTex = *(uint32*)compBuffer;
#ifdef CPU_BIG_ENDIAN
    numTex = swap32(numTex);
#endif

    ExpandData((uint8*)compBuffer + 4 + numTex * 4, (uint8*)pVRAM);

    timTotalSize = 0;
    HACK_TEX_FILTER = filter;
    for (i = 0; i < numTex; i++)
    {
        renderCreateTexture((uint8*)(pVRAM + timTotalSize));
        timSize = *(uint32*)(compBuffer + 4 + i * 4);
    #ifdef CPU_BIG_ENDIAN
        timSize = swap32(timSize);
    #endif
        timTotalSize += timSize;
    }
    HACK_TEX_FILTER = 0;

    allocVRAM(timTotalSize);
}
#else
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
    int32 size;
    Tim* tim;
    Texture* tex = gTexturePtr;

    size = osLoadFile(fileName, pVRAM);
    tim = (Tim*)allocVRAM(size);

    renderCreateTexture((uint8*)tim);

    return tex;
}

extern Cone ctrlcone;

extern Glyph glyphEnd;
extern Glyph glyphDel;
extern Object* ctrlshipShapes[];
extern ShipData ctrlshipIndex[];
extern const short shipShapeOrder[];
extern Texture* gShadowTex;
extern Texture* gSpeedoTex;
extern Texture* gWinLoseTex;
extern Texture* gWeaponIcon;
Texture* gTargetTex;
Texture* gEffectTextures;

extern Object* audioShapes;
extern Object* pilotShapes;
extern Object* teamShapes;

extern Object* clsShapes;
extern Object* gameShapes;
extern Object* optShapes;
extern Object* padShapes;

extern Texture* gButtonTex;
extern Texture* slideTex;
extern Texture* sliderTex;
extern Texture* menuScreen;

extern Texture* gTrackIcon;

#define TTF_SIZE_MAX    1024

char gTTF[TTF_SIZE_MAX];

TexTemplate* ctrltexTemplate;
int32 ctrlnumLib;
int32 ctrllibraryTextures;

WeaponGrid gWeaponGrid[64]; // TODO remove

void LoadShipData(ShipData* shipIndex, Object** shipShapes)
{
    Texture* shipTextures;
    Texture* droidTextures;
    Texture* colTextures;
    Object* obj;
    int32 i, objCount;
    int32 count = 0;

    /******************************************************/
    /** Load compressed texture files, first to allow    **/
    /** temporary memory to be allocated                 **/
    /******************************************************/

    shipTextures = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/allsh.cmp", 1);

    obj = LoadPrm("wipeout/common/allsh.prm", shipTextures, &objCount, NULL);
    ASSERT(objCount == NO_OF_SHIPS);

    for (i = 0; i < objCount; i++)
    {
        shipIndex[count].highResTableIndex = shipShapeOrder[count];
        shipShapes[count] = obj + i;

        count += 1;
    }

    colTextures = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/alcol.cmp", 1);

    obj = LoadPrm("wipeout/common/alcol.prm", colTextures, &objCount, NULL);
    ASSERT(objCount == NO_OF_SHIPS);

    for (i = 0; i < objCount; i++)
    {
        shipIndex[count - NO_OF_SHIPS].lowResTableIndex = count;
        shipShapes[count] = obj + i;

        count += 1;
    }

    droidTextures = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/rescu.cmp", 1);

    shipShapes[RESCUE_DROID] = LoadPrm("wipeout/common/rescu.prm", droidTextures, NULL, NULL);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        ctrlcone.object[i] = shipShapes[shipIndex[i].highResTableIndex];
        ctrlcone.ship[i] = &shipIndex[i];
    }

    InitCone(&ctrlcone);
}

Object* LoadModel(const char* fileName)
{
    char modelFile[64];
    strcpy(modelFile, "wipeout/common/");
    strcat(modelFile, fileName);
    strcat(modelFile, ".prm");

    return LoadPrm(modelFile, NULL, NULL, NULL);
}

Object* LoadTexModel(const char* fileName)
{
    Texture* modelTextures;
    char modelFile[64];

    /******************************************************/
    /** Load compressed texture files, first to allow    **/
    /** temporary memory to be allocated                 **/
    /******************************************************/

    strcpy(modelFile, "wipeout/common/");
    strcat(modelFile, fileName);
    strcat(modelFile, ".cmp");

    modelTextures = gTexturePtr;
    LoadCompressedTextureSequence(modelFile, 0);

    strcpy(modelFile, "wipeout/common/");
    strcat(modelFile, fileName);
    strcat(modelFile, ".prm");

    return LoadPrm(modelFile, modelTextures, NULL, NULL);
}

#ifdef __3DO__
void InitShadow(void)
{
}
#else
void InitShadow(void)
{
    const char* name;

    if (ownShip < 2)
        name = "wipeout/textures/shad1.tim";
    else if (ownShip < 4)
        name = "wipeout/textures/shad2.tim";
    else if (ownShip < 6)
        name = "wipeout/textures/shad3.tim";
    else
        name = "wipeout/textures/shad4.tim";

    HACK_LOAD_SHADOW = 1;
    gShadowTex = gTexturePtr;
    LoadTexture(name);
    HACK_LOAD_SHADOW = 0;
}
#endif

#ifdef _DEBUG
    //#define CONV_FMT
#endif

#ifdef CONV_FMT
void writeTextures(FILE* f, Texture* tex, Texture* last)
{
    int32 count = last - tex;
    fwrite(&count, sizeof(count), 1, f);
    LOG("write textures: %d\n", count);

    while (tex < last)
    {
        int32 size = 0;
        switch (TimType(tex->tim))
        {
            case TIM_4:
            {
                Tim4* t = (Tim4*)tex->tim;
                size = sizeof(*t) + t->textureH * t->textureW * sizeof(uint16);
                break;
            }
            case TIM_8:
            {
                Tim8* t = (Tim8*)tex->tim;
                size = sizeof(*t) + t->textureH * t->textureW * sizeof(uint16);
                break;
            }
            case TIM_16:
            {
                Tim16* t = (Tim16*)tex->tim;
                size = sizeof(*t) + t->textureH * t->textureW * sizeof(uint16);
                break;
            }
        }
        fwrite(tex->tim, size, 1, f);
        tex++;
    }
}

void writeObjects(FILE* f, Object* obj, Object* last)
{
    int32 count = last - obj;
    fwrite(&count, sizeof(count), 1, f);
    LOG("write objects: %d\n", count);

    while (obj < last)
    {
        fwrite(&obj->vCount, sizeof(obj->vCount), 1, f);
        fwrite(&obj->fCount, sizeof(obj->fCount), 1, f);
        fwrite(&obj->sCount, sizeof(obj->sCount), 1, f);
        fwrite(&obj->extent, sizeof(obj->extent), 1, f);
        fwrite(&obj->pos.x, sizeof(obj->pos.x), 1, f);
        fwrite(&obj->pos.y, sizeof(obj->pos.y), 1, f);
        fwrite(&obj->pos.z, sizeof(obj->pos.z), 1, f);

        fwrite(obj->vertices, sizeof(obj->vertices[0]), obj->vCount, f);
        fwrite(obj->faces, sizeof(obj->faces[0]), obj->fCount, f);
        fwrite(obj->sprites, sizeof(obj->sprites[0]), obj->sCount, f);

        obj++;
    }
}

void writeWAD(const char* fileName, Texture* firstTex, Texture* lastTex, Object* firstObj, Object* lastObj)
{
    FILE* f = fopen(fileName, "wb");
    ASSERT(f);
    writeTextures(f, firstTex, lastTex);
    writeObjects(f, firstObj, lastObj);
    fclose(f);
}
#endif

void LoadCommonOld(void)
{
#ifdef CONV_FMT
    Texture* firstTex = gTexturePtr;
    Object* firstObj = gObjectPtr;
#endif

    Texture* weaponTextures;

    ASSERT(gTextures == gTexturePtr);

    LoadCompressedTextureSequence("wipeout/textures/drfonts.cmp", 0);

    LoadShipData(ctrlshipIndex, ctrlshipShapes);

    gSpeedoTex = LoadTexture("wipeout/textures/speedo.tim");

    gWeaponIcon = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/wicons.cmp", 0);

    gEffectTextures = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/effects.cmp", 1);

    weaponTextures = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/mine.cmp", 1);

    ctrlweaponShapes[WEAPON_SHAPE_ROCKET]  = LoadPrm("wipeout/common/rock.prm", weaponTextures, NULL, NULL);
    ctrlweaponShapes[WEAPON_SHAPE_MINE]    = LoadPrm("wipeout/common/mine.prm", weaponTextures, NULL, NULL);
    ctrlweaponShapes[WEAPON_SHAPE_MISSILE] = LoadPrm("wipeout/common/miss.prm", weaponTextures, NULL, NULL);
    ctrlweaponShapes[WEAPON_SHAPE_SHIELD]  = LoadPrm("wipeout/common/shld.prm", weaponTextures, NULL, NULL);
    ctrlweaponShapes[WEAPON_SHAPE_EBOLT]   = LoadPrm("wipeout/common/ebolt.prm", weaponTextures, NULL, NULL);

    gTargetTex = LoadTexture("wipeout/textures/target2.tim");

#ifdef CONV_FMT
    ASSERT(firstTex == gTextures);
    ASSERT(firstObj == gObjects);
    writeWAD("wipeout/common.wad", firstTex, gTexturePtr, firstObj, gObjectPtr);
#endif
}

void LoadMainMenuOld(void)
{
#ifdef CONV_FMT
    Texture* firstTex = gTexturePtr;
    Object* firstObj = gObjectPtr;
#endif

    pilotShapes = LoadTexModel("pilot");
    audioShapes = LoadTexModel("cdobj");

#ifdef VER_PSX
    teamShapes = LoadModel("teams");
    clsShapes = LoadModel("class");
    padShapes = LoadTexModel("pad1");
    gameShapes = LoadModel("champ");
    optShapes = LoadTexModel("alopt");
#else
    teamShapes = LoadTexModel("team");
    clsShapes = LoadTexModel("leeg");
    padShapes = LoadTexModel("cntrl");
    gameShapes = LoadTexModel("msdos");
    optShapes = LoadTexModel("misc");
#endif

    gTrackIcon = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/textures/track.cmp", 0);

    gButtonTex = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/textures/buttons.cmp", 0);

    slideTex = LoadTexture("wipeout/textures/slider1.tim");
    sliderTex = LoadTexture("wipeout/textures/slide1.tim");
    menuScreen = LoadTexture("wipeout/textures/wipeout1.tim");

#ifdef CONV_FMT
    writeWAD("wipeout/menu.wad", firstTex, gTexturePtr, firstObj, gObjectPtr);
#endif
}

TexTemplate* LoadTtfFile(char* file)
{
    int32 i;
    uint16* tex;

    int32 size = osLoadFile(file, gTTF);
    ASSERT(size <= TTF_SIZE_MAX);

    ctrlnumLib = 0;

    tex = (uint16*)gTTF;
    while (tex < ((uint16*)(gTTF + size)))
    {
        for (i = 0; i < 21; i++)
        {
        #ifndef CPU_BIG_ENDIAN
            swap16p(tex);
        #endif
            tex += 1;
        }
        ctrlnumLib++; /* count of textures */
    }

    return ((TexTemplate*)gTTF);
}

void SortObjects(uint8* types)
{
    int i, swapped;

    do
    {
        swapped = 0;
        for (i = 1; i < gTrack.objCount; i++)
        {
            if (types[i - 1] > types[i])
            {
                uint8 tmpType = types[i - 1];
                Object tmpObj = gTrack.objects[i - 1];

                types[i - 1] = types[i];
                types[i] = tmpType;

                gTrack.objects[i - 1] = gTrack.objects[i];
                gTrack.objects[i] = tmpObj;

                swapped = 1;
            }
        }
    } while (swapped);
}

void InitObjects(uint8* types)
{
    int32 i;

    gTrack.startCount = 0;
    gTrack.lostCount = 0;
    gTrack.redCount = 0;
    gTrack.donkeyCount = 0;

    for (i = 0; i < gTrack.objCount; i++)
    {
        int32 type = types[i];

        if (type == OBJ_TYPE_START)
            gTrack.startCount++;
        else if (type == OBJ_TYPE_LOST)
            gTrack.lostCount++;
        else if (type == OBJ_TYPE_RED)
            gTrack.redCount++;
        else if (type == OBJ_TYPE_DONKEY)
            gTrack.donkeyCount++;
        else
            break;
    }

    gTrack.start = gTrack.objects;
    gTrack.lost = gTrack.start + gTrack.startCount;
    gTrack.red = gTrack.lost + gTrack.lostCount;
    gTrack.donkey = gTrack.red + gTrack.redCount;
}

void InitWeaponGrids(void)
{
    int32 i, j;
    int32 ctrlcheck = 1;
    int32 ctrljcheck = 1;
    int32 weaponsCount;
    TrackSection* ctrlsection;
    TrackSection* ctrljsection;

    weaponsCount = 0;
    ctrlsection = gTrack.sections;

    i = 0;
    NoOfMainSections = 0;

    // TODO pre-calc secNo and weapon grids for 3DO
    while (ctrlcheck)
    {
        weaponsCount = SetTrackAttributes(ctrlsection, i, gTrack.weapons, weaponsCount);
        ctrlsection->secNo = i;
        if (ctrlsection->junction)
        {
            ctrljsection = ctrlsection;
            j = i;
            ctrlsection = ctrlsection->junction;
            while (ctrljcheck)
            {
                i++;
                weaponsCount = SetTrackAttributes(ctrlsection, i, gTrack.weapons, weaponsCount);
                ctrlsection->secNo = i;
                ctrlsection = ctrlsection->nextSection;
                if (ctrlsection->junction)
                    ctrljcheck = 0;
            }
            ctrljcheck = 1;
            ctrlsection = ctrljsection;
            i = j;
        }
        i++;
        NoOfMainSections++;
        ctrlsection = ctrlsection->nextSection;
        if (ctrlsection == gTrack.sections)
            ctrlcheck = 0;
    }

    gTrack.weaponsCount = weaponsCount;
}

void InitVertices(const char* fileName)
{
    int32 i;
    int32 size = osLoadFile(fileName, pDRAM);
    vec4i* v4 = (vec4i*)pDRAM;
    vec3i* v3 = (vec3i*)pDRAM;

    gTrack.vertexCount = size / sizeof(vec4i);
    gTrack.vertices = v3;

    allocDRAM(sizeof(vec3i) * gTrack.vertexCount);

    // convert from vec4i to vec3i
    for (i = 0; i < gTrack.vertexCount; i++, v4++, v3++)
    {
    #ifndef CPU_BIG_ENDIAN
        swap32p(&v4->x);
        swap32p(&v4->y);
        swap32p(&v4->z);
    #endif
        v3->x = v4->x;
        v3->y = v4->y;
        v3->z = v4->z;
    }
}

void InitFaces(const char* fileName)
{
    int32 size = osLoadFile(fileName, pDRAM);
    gTrack.faces = (Face*)allocDRAM(size);
    gTrack.faceCount = size / sizeof(Face);

#ifndef CPU_BIG_ENDIAN
    {
        int32 i;
        Face* face = gTrack.faces;
        for (i = 0; i < gTrack.faceCount; i++, face++)
        {
            swap16p(&face->vertex[0]);
            swap16p(&face->vertex[1]);
            swap16p(&face->vertex[2]);
            swap16p(&face->vertex[3]);

            swap16p(&face->normal.x);
            swap16p(&face->normal.y);
            swap16p(&face->normal.z);
        }
    }
#endif
}

void InitSections(const char* fileName)
{
    TrackSection* section;
    TrackSectionPSX* sectionPSX;

    int32 i, j;
    int32 size = osLoadFile(fileName, pDRAM);
    uint8* start;

    start = (uint8*)malloc(size);
    memcpy(start, pDRAM, size);

    gTrack.sectionCount = size / sizeof(TrackSectionPSX);
    gTrack.sections = (TrackSection*)allocDRAM(sizeof(TrackSection) * gTrack.sectionCount);

    sectionPSX = (TrackSectionPSX*)start;
    section = gTrack.sections;
    for (i = 0; i < gTrack.sectionCount; i++, section++, sectionPSX++)
    {
    #ifndef CPU_BIG_ENDIAN
        swap32p(&sectionPSX->junctionIndex);
        swap32p(&sectionPSX->prevSectionIndex);
        swap32p(&sectionPSX->nextSectionIndex);

        swap32p(&sectionPSX->centre.x);
        swap32p(&sectionPSX->centre.y);
        swap32p(&sectionPSX->centre.z);

        for (j = 0; j < 3; j++)
        {
            swap16p(&sectionPSX->northViewCount[j]);
            swap16p(&sectionPSX->southViewCount[j]);
            swap16p(&sectionPSX->eastViewCount[j]);
            swap16p(&sectionPSX->westViewCount[j]);
            swap16p(&sectionPSX->allViewCount[j]);
        }

        swap16p(&sectionPSX->faceStart);
        swap16p(&sectionPSX->faceCount);

        swap16p(&sectionPSX->globalRadius);
        swap16p(&sectionPSX->localRadius);

        swap16p(&sectionPSX->flags);
    #endif

        section->centre.x = sectionPSX->centre.x;
        section->centre.y = sectionPSX->centre.y;
        section->centre.z = sectionPSX->centre.z;

        for (j = 0; j < 3; j++)
        {
            section->northViewCount[j] = sectionPSX->northViewCount[j];
            section->southViewCount[j] = sectionPSX->southViewCount[j];
            section->eastViewCount[j]  = sectionPSX->eastViewCount[j];
            section->westViewCount[j]  = sectionPSX->westViewCount[j];
            section->allViewCount[j]   = sectionPSX->allViewCount[j];
        }

        section->faceStart = sectionPSX->faceStart;
        section->faceCount = sectionPSX->faceCount;

        section->globalRadius = sectionPSX->globalRadius;
        section->localRadius = sectionPSX->localRadius;

        section->flags = sectionPSX->flags;

        section->prevSection = gTrack.sections + sectionPSX->prevSectionIndex;
        section->nextSection = gTrack.sections + sectionPSX->nextSectionIndex;

        if (sectionPSX->junctionIndex != -1)
        {
            section->junction = gTrack.sections + sectionPSX->junctionIndex;
        }
        else
        {
            section->junction = NULL;
        }
    }

    free(start);
}

void InitViewList(char* file)
{
    TrackSection* section;
    int32 total;
    int32 i, j;
    int32 viewListCount;

    int32 size = osLoadFile(file, pDRAM);
    gTrack.viewList = (int16*)allocDRAM(size);
    viewListCount = size / 2;

#ifndef CPU_BIG_ENDIAN
    for (i = 0; i < viewListCount; i++)
    {
        swap16p(&gTrack.viewList[i]);
    }
#endif

    section = gTrack.sections;
    total = 0;
    for (i = 0; i < gTrack.sectionCount; i++)
    {
        for (j = 0; j < 3; j++)
        {
            section->northViewSection[j] = gTrack.viewList + total;
            total += section->northViewCount[j];

            section->southViewSection[j] = gTrack.viewList + total;
            total += section->southViewCount[j];

            section->eastViewSection[j] = gTrack.viewList + total;
            total += section->eastViewCount[j];

            section->westViewSection[j] = gTrack.viewList + total;
            total += section->westViewCount[j];

            section->allViewSection[j] = gTrack.viewList + total;
            total += section->allViewCount[j];
        }

        section++;
    }
}

#define CONV_COLOR(c) (c.r | (c.g << 8) | (c.b << 16) | 0xFF000000)

#define SET_FACE_IDX_T(f, i)        f->index[0] = i->coords[0]; f->index[1] = i->coords[1]; f->index[2] = i->coords[2]; f->index[3] = i->coords[2];
#define SET_FACE_IDX_Q(f, i)        f->index[0] = i->coords[0]; f->index[1] = i->coords[1]; f->index[2] = i->coords[3]; f->index[3] = i->coords[2];
#define SET_FACE_TEX_Z(f)           f->flags = NO_TEXTURE; memset(f->uv, 0, sizeof(f->uv))
#define SET_FACE_TEX_T(f, p)        f->flags = p->texture; f->uv[0][0] = p->u0; f->uv[0][1] = p->v0; f->uv[1][0] = p->u1; f->uv[1][1] = p->v1; f->uv[2][0] = p->u2; f->uv[2][1] = p->v2; f->uv[3][0] = f->uv[3][1] = 0
#define SET_FACE_TEX_Q(f, p)        f->flags = p->texture; f->uv[0][0] = p->u0; f->uv[0][1] = p->v0; f->uv[1][0] = p->u1; f->uv[1][1] = p->v1; f->uv[2][0] = p->u3; f->uv[2][1] = p->v3; f->uv[3][0] = p->u2; f->uv[3][1] = p->v2
#define SET_FACE_COLOR_FLAT(f, p)   f->color[0] = f->color[1] = f->color[2] = f->color[3] = CONV_COLOR(p->colour)
#define SET_FACE_COLOR_T(f, p)      f->color[0] = CONV_COLOR(p->colour[0]); f->color[1] = CONV_COLOR(p->colour[1]); f->color[2] = CONV_COLOR(p->colour[2]); f->color[3] = 0
#define SET_FACE_COLOR_Q(f, p)      f->color[0] = CONV_COLOR(p->colour[0]); f->color[1] = CONV_COLOR(p->colour[1]); f->color[2] = CONV_COLOR(p->colour[3]); f->color[3] = CONV_COLOR(p->colour[2])

#define SET_SPRITE_IDX(s, p)        s->index = p->coord
#define SET_SPRITE_TEX(s, p)        s->flags = p->texture
#define SET_SPRITE_SIZE(s, p);      s->width = p->width; s->height = p->height
#define SET_SPRITE_COLOR(s, p)      s->color = CONV_COLOR(p->colour)

void IntelPrim(PrmPSX prm)
{
    swap16p(&prm.f3->type);
    swap16p(&prm.f3->flag);

    switch (prm.f3->type)
    {
        case TypeF3:
            swap16p(&prm.f3->coords[0]);
            swap16p(&prm.f3->coords[1]);
            swap16p(&prm.f3->coords[2]);
            break;

        case TypeF4:
            swap16p(&prm.f4->coords[0]);
            swap16p(&prm.f4->coords[1]);
            swap16p(&prm.f4->coords[2]);
            swap16p(&prm.f4->coords[3]);
            break;

        case TypeFT3:
            swap16p(&prm.ft3->coords[0]);
            swap16p(&prm.ft3->coords[1]);
            swap16p(&prm.ft3->coords[2]);
            swap16p(&prm.ft3->texture);
            break;

        case TypeFT4:
            swap16p(&prm.ft4->coords[0]);
            swap16p(&prm.ft4->coords[1]);
            swap16p(&prm.ft4->coords[2]);
            swap16p(&prm.ft4->coords[3]);
            swap16p(&prm.ft4->texture);
            break;

        case TypeG3:
            swap16p(&prm.g3->coords[0]);
            swap16p(&prm.g3->coords[1]);
            swap16p(&prm.g3->coords[2]);
            break;

        case TypeG4:
            swap16p(&prm.g4->coords[0]);
            swap16p(&prm.g4->coords[1]);
            swap16p(&prm.g4->coords[2]);
            swap16p(&prm.g4->coords[3]);
            break;

        case TypeGT3:
            swap16p(&prm.gt3->coords[0]);
            swap16p(&prm.gt3->coords[1]);
            swap16p(&prm.gt3->coords[2]);
            swap16p(&prm.gt3->texture);
            break;

        case TypeGT4:
            swap16p(&prm.gt4->coords[0]);
            swap16p(&prm.gt4->coords[1]);
            swap16p(&prm.gt4->coords[2]);
            swap16p(&prm.gt4->coords[3]);
            swap16p(&prm.gt4->texture);
            break;

        case TypeTSPR:
        case TypeBSPR:
            swap16p(&prm.spr->coord);
            swap16p(&prm.spr->width);
            swap16p(&prm.spr->height);
            swap16p(&prm.spr->texture);
            break;

        case TypeLSF3:
            swap16p(&prm.lsf3->coords[0]);
            swap16p(&prm.lsf3->coords[1]);
            swap16p(&prm.lsf3->coords[2]);
            swap16p(&prm.lsf3->normal);
            break;

        case TypeLSFT3:
            swap16p(&prm.lsft3->coords[0]);
            swap16p(&prm.lsft3->coords[1]);
            swap16p(&prm.lsft3->coords[2]);
            swap16p(&prm.lsft3->normal);
            swap16p(&prm.lsft3->texture);
            break;

        case TypeLSF4:
            swap16p(&prm.lsf4->coords[0]);
            swap16p(&prm.lsf4->coords[1]);
            swap16p(&prm.lsf4->coords[2]);
            swap16p(&prm.lsf4->coords[3]);
            swap16p(&prm.lsf4->normal);
            break;

        case TypeLSFT4:
            swap16p(&prm.lsft4->coords[0]);
            swap16p(&prm.lsft4->coords[1]);
            swap16p(&prm.lsft4->coords[2]);
            swap16p(&prm.lsft4->coords[3]);
            swap16p(&prm.lsft4->normal);
            swap16p(&prm.lsft4->texture);
            break;

        case TypeLSG3:
            swap16p(&prm.lsg3->coords[0]);
            swap16p(&prm.lsg3->coords[1]);
            swap16p(&prm.lsg3->coords[2]);
            swap16p(&prm.lsg3->normals[0]);
            swap16p(&prm.lsg3->normals[1]);
            swap16p(&prm.lsg3->normals[2]);
            break;

        case TypeLSGT3:
            swap16p(&prm.lsgt3->coords[0]);
            swap16p(&prm.lsgt3->coords[1]);
            swap16p(&prm.lsgt3->coords[2]);
            swap16p(&prm.lsgt3->normals[0]);
            swap16p(&prm.lsgt3->normals[1]);
            swap16p(&prm.lsgt3->normals[2]);
            swap16p(&prm.lsgt3->texture);
            break;

        case TypeLSG4:
            swap16p(&prm.lsg4->coords[0]);
            swap16p(&prm.lsg4->coords[1]);
            swap16p(&prm.lsg4->coords[2]);
            swap16p(&prm.lsg4->coords[3]);
            swap16p(&prm.lsg4->normals[0]);
            swap16p(&prm.lsg4->normals[1]);
            swap16p(&prm.lsg4->normals[2]);
            swap16p(&prm.lsg4->normals[3]);
            break;

        case TypeLSGT4:
            swap16p(&prm.lsgt4->coords[0]);
            swap16p(&prm.lsgt4->coords[1]);
            swap16p(&prm.lsgt4->coords[2]);
            swap16p(&prm.lsgt4->coords[3]);
            swap16p(&prm.lsgt4->normals[0]);
            swap16p(&prm.lsgt4->normals[1]);
            swap16p(&prm.lsgt4->normals[2]);
            swap16p(&prm.lsgt4->normals[3]);
            swap16p(&prm.lsgt4->texture);
            break;

        case TypeSpline:
            swap32p(&prm.spline->control1.x);
            swap32p(&prm.spline->control1.y);
            swap32p(&prm.spline->control1.z);
            swap32p(&prm.spline->position.x);
            swap32p(&prm.spline->position.y);
            swap32p(&prm.spline->position.z);
            swap32p(&prm.spline->control2.x);
            swap32p(&prm.spline->control2.y);
            swap32p(&prm.spline->control2.z);
            break;

        case TypeInfiniteLight:
        case TypePointLight:
        case TypeSpotLight:
            // we don't use them
            break;

        default:
            ASSERT(0);
            break;
    }
}

Object* LoadPrm(const char* name, Texture* texturePtr, int32* count, uint8* types)
{
    int32 textureStart = texturePtr - gTextures;
    int32 size = osLoadFile(name, pDRAM); // use DRAM as temp buffer
    uint8* start = (uint8*)malloc(size);
    uint8* ptr = start;
    uint8* end = start + size;

    uint8* data = pDRAM;
    Object* obj = gObjectPtr;
    Object* retObj;

    // copy PSX data to the temporary buffer
    memcpy(start, pDRAM, size);

    // count objects
    while (ptr < end)
    {
        int32 i;
        PrmPSX prm;
        ObjectPSX* objectPSX;
        SkeletonPSX* skeletonPSX;
        ObjFace* f;

        objectPSX = (ObjectPSX*)ptr;
        ptr += sizeof(ObjectPSX);

        skeletonPSX = (SkeletonPSX*)ptr;
        ptr += sizeof(SkeletonPSX);

    #ifndef CPU_BIG_ENDIAN
        swap16p(&objectPSX->vertexCount);
        swap16p(&objectPSX->normalCount);
        swap16p(&objectPSX->primitiveCount);
        swap32p(&objectPSX->extent);

        swap32p(skeletonPSX->relative.t + 0);
        swap32p(skeletonPSX->relative.t + 1);
        swap32p(skeletonPSX->relative.t + 2);
    #endif

        // get object type by name
        if (types)
        {
            if (!strncmp(objectPSX->name, "start", 5))
                *types++ = OBJ_TYPE_START;
            else if (!strncmp(objectPSX->name, "lost", 4))
                *types++ = OBJ_TYPE_LOST;
            else if (!strncmp(objectPSX->name, "redl", 4))
                *types++ = OBJ_TYPE_RED;
            else if (!strncmp(objectPSX->name, "donkey", 6))
                *types++ = OBJ_TYPE_DONKEY;
            else
                *types++ = OBJ_TYPE_MAX;
        }

        obj->pos.x = skeletonPSX->relative.t[0];
        obj->pos.y = skeletonPSX->relative.t[1];
        obj->pos.z = skeletonPSX->relative.t[2];
        obj->extent = objectPSX->extent;

        obj->vertices = (vec3s*)data;
        data += sizeof(vec3s) * objectPSX->vertexCount;
        if (objectPSX->vertexCount & 1) // extra alignment
            data += sizeof(int16);

        // read vertices
        obj->vCount = objectPSX->vertexCount;
        for (i = 0; i < objectPSX->vertexCount; i++)
        {
            int16* v = (int16*)ptr;

        #ifndef CPU_BIG_ENDIAN
            swap16p(v + 0);
            swap16p(v + 1);
            swap16p(v + 2);
        #endif

            obj->vertices[i].x = v[0];
            obj->vertices[i].y = v[1];
            obj->vertices[i].z = v[2];
            ptr += sizeof(vec4s);
        }

        // skip normals
        ptr += sizeof(vec4s) * objectPSX->normalCount;

        // read primitives
        prm.primitive = (PrimitivePSX*)ptr;
        obj->faces = (ObjFace*)data;
        f = obj->faces;

        for (i = 0; i < objectPSX->primitiveCount; i++)
        {
        #ifndef CPU_BIG_ENDIAN
            IntelPrim(prm);
        #endif

            switch (prm.primitive->type)
            {
                case TypeF3:
                    SET_FACE_IDX_T(f, prm.f3);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_FLAT(f, prm.f3);
                    prm.f3++;
                    f++;
                    break;

                case TypeF4:
                    SET_FACE_IDX_Q(f, prm.f4);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_FLAT(f, prm.f4);
                    prm.f4++;
                    f++;
                    break;

                case TypeFT3:
                    prm.ft3->texture += textureStart;
                    SET_FACE_IDX_T(f, prm.ft3);
                    SET_FACE_TEX_T(f, prm.ft3);
                    SET_FACE_COLOR_FLAT(f, prm.ft3);

                    if (prm.ft3->flag & ShipEngine)
                    {
                        f->flags |= FACE_FLAG_ENGINE;
                    }

                    prm.ft3++;
                    f++;
                    break;

                case TypeFT4:
                    prm.ft4->texture += textureStart;
                    SET_FACE_IDX_Q(f, prm.ft4);
                    SET_FACE_TEX_Q(f, prm.ft4);
                    SET_FACE_COLOR_FLAT(f, prm.ft4);
                    prm.ft4++;
                    f++;
                    break;

                case TypeG3:
                    SET_FACE_IDX_T(f, prm.g3);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_T(f, prm.g3);
                    prm.g3++;
                    f++;
                    break;

                case TypeG4:
                    SET_FACE_IDX_Q(f, prm.g4);
                    SET_FACE_TEX_Z(f);
                    SET_FACE_COLOR_Q(f, prm.g4);
                    prm.g4++;
                    f++;
                    break;

                case TypeGT3:
                    prm.gt3->texture += textureStart;
                    SET_FACE_IDX_T(f, prm.gt3);
                    SET_FACE_TEX_T(f, prm.gt3);
                    SET_FACE_COLOR_T(f, prm.gt3);

                    if (prm.gt3->flag & ShipEngine)
                    {
                        f->flags |= FACE_FLAG_ENGINE;
                    }

                    prm.gt3++;
                    f++;
                    break;

                case TypeGT4:
                    prm.gt4->texture += textureStart;
                    SET_FACE_IDX_Q(f, prm.gt4);
                    SET_FACE_TEX_Q(f, prm.gt4);
                    SET_FACE_COLOR_Q(f, prm.gt4);
                    prm.gt4++;
                    f++;
                    break;

                case TypeTSPR:
                case TypeBSPR:
                    prm.spr->texture += textureStart;
                    // TODO sprites
                    prm.spr++;
                    break;

                case TypeLSF3:
                    prm.lsf3++;
                    break;

                case TypeLSFT3:
                    prm.lsft3->texture += textureStart;
                    // TODO sprites
                    prm.lsft3++;
                    break;

                case TypeLSF4:
                    // TODO
                    prm.lsf4++;
                    break;

                case TypeLSFT4:
                    prm.lsft4->texture += textureStart;
                    // TODO
                    prm.lsft4++;
                    break;

                case TypeLSG3:
                    // TODO
                    prm.lsg3++;
                    break;

                case TypeLSGT3:
                    prm.lsgt3->texture += textureStart;
                    // TODO
                    prm.lsgt3++;
                    break;

                case TypeLSG4:
                    // TODO
                    prm.lsg4++;
                    break;

                case TypeLSGT4:
                    prm.lsgt4->texture += textureStart;
                    // TODO
                    prm.lsgt4++;
                    break;

                case TypeSpline:
                    prm.spline++;
                    break;

                case TypeInfiniteLight:
                    prm.infiniteLight++;
                    break;

                case TypePointLight:
                    prm.pointLight++;
                    break;

                case TypeSpotLight:
                    prm.spotLight++;
                    break;

                default:
                    ASSERT(0);
            }
        }
        obj->fCount = f - obj->faces;
        data += sizeof(ObjFace) * obj->fCount;
        ptr = (uint8*)prm.primitive;

        obj->sCount = 0; // TODO sprites

        obj++;
        ASSERT(obj - gObjects < MAX_OBJECTS);
    }

    free(start);

    allocDRAM(data - pDRAM);

    if (count)
    {
        *count = (obj - gObjectPtr);
    }

    retObj = gObjectPtr;
    gObjectPtr = obj;

    return retObj;
}

void LoadSceneOld(int32 trackNum)
{
#ifdef CONV_FMT
    Texture* firstTex = gTexturePtr;
    Object* firstObj = gObjectPtr;
#endif

    Texture* sceneTextures;
    Texture* skyTextures;

    char trackPath[64];
    uint8 types[MAX_OBJECTS];

    getPath(trackPath, trackNum, "/library.ttf");
    ctrltexTemplate = LoadTtfFile(trackPath);

    HACK_LOAD_TRACK = 1;
    ctrllibraryTextures = gTexturePtr - gTextures;
    getPath(trackPath, trackNum, "/library.cmp");
    LoadCompressedTextureSequence(trackPath, 0);
    HACK_LOAD_TRACK = 0;

    getPath(trackPath, trackNum, "/track.trv");
    InitVertices(trackPath);
    getPath(trackPath, trackNum, "/track.trf");
    InitFaces(trackPath);
    getPath(trackPath, trackNum, "/track.trs");
    InitSections(trackPath);
    getPath(trackPath, trackNum, "/track.vew");
    InitViewList(trackPath);

    gTrack.weapons = gWeaponGrid;
    InitWeaponGrids();

    sceneTextures = gTexturePtr;
    getPath(trackPath, trackNum, "/scene.cmp");
    LoadCompressedTextureSequence(trackPath, 1);
    getPath(trackPath, trackNum, "/scene.prm");
    gTrack.objects = LoadPrm(trackPath, sceneTextures, &gTrack.objCount, types);

    LOG("OBJECTS: %d\n", gTrack.objCount);

    SortObjects(types);
    InitObjects(types);

    if (gTrack.redCount)
    {
        ASSERT((trackNo == 8) || (trackNo == 11) || (trackNo == 1));
    }

    skyTextures = gTexturePtr;
    getPath(trackPath, trackNum, "/sky.cmp");
    LoadCompressedTextureSequence(trackPath, 1);
    getPath(trackPath, trackNum, "/sky.prm");
    gTrack.sky = LoadPrm(trackPath, skyTextures, NULL, NULL);

    SceneRadiusCheck(gTrack.objects, gTrack.objCount);

    /* silverstream */
    if (trackNo == 13)
    {
        camSpline[4] = LoadPrm("wipeout/common/cam5s.prm", NULL, NULL, NULL);
        shipSpline[4] = LoadPrm("wipeout/common/shp5s.prm", NULL, NULL, NULL);

        camSpline[5] = LoadPrm("wipeout/common/cam6s.prm", NULL, NULL, NULL);
        shipSpline[5] = LoadPrm("wipeout/common/shp6s.prm", NULL, NULL, NULL);

        camSpline[6] = LoadPrm("wipeout/common/cam7s.prm", NULL, NULL, NULL);
        shipSpline[6] = LoadPrm("wipeout/common/shp7s.prm", NULL, NULL, NULL);
    }

    /* firestar */
    if (trackNo == 14)
    {
        camSpline[0] = LoadPrm("wipeout/common/cam1s.prm", NULL, NULL, NULL);
        shipSpline[0] = LoadPrm("wipeout/common/shp1s.prm", NULL, NULL, NULL);

        camSpline[1] = LoadPrm("wipeout/common/cam2s.prm", NULL, NULL, NULL);
        shipSpline[1] = LoadPrm("wipeout/common/shp2s.prm", NULL, NULL, NULL);

        camSpline[2] = LoadPrm("wipeout/common/cam3s.prm", NULL, NULL, NULL);
        shipSpline[2] = LoadPrm("wipeout/common/shp3s.prm", NULL, NULL, NULL);

        camSpline[3] = LoadPrm("wipeout/common/cam4s.prm", NULL, NULL, NULL);
        shipSpline[3] = LoadPrm("wipeout/common/shp4s.prm", NULL, NULL, NULL);
    }

#ifdef CONV_FMT
    getPath(trackPath, trackNum, ".wad");
    writeWAD(trackPath, firstTex, gTexturePtr, firstObj, gObjectPtr);
#endif
}

void LoadWinLoseTex(const char* fileName)
{
    char pilotPath[64];

    strcpy(pilotPath, "wipeout/textures/");
    strcat(pilotPath, fileName);
    strcat(pilotPath, ".cmp");

    gWinLoseTex = gTexturePtr;
    LoadCompressedTextureSequence(pilotPath, 0);

#ifdef CONV_FMT
    strcpy(pilotPath, "wipeout/");
    strcat(pilotPath, fileName);
    strcat(pilotPath, ".tex");
    {
        FILE* f = fopen(pilotPath, "wb");
        writeTextures(f, gWinLoseTex, gTexturePtr);
        fclose(f);
    }
#endif
}

void LoadCommon(void)
{
    LoadCommonOld();

    SetCharTextures(8);
    SetCharTextures(12);
    SetCharTextures(16);

    glyphEnd.x = 0;
    glyphEnd.y = 0;
    glyphEnd.w = 24;
    glyphEnd.h = 16;
    glyphEnd.texture = gTextures + DRTEXT_RED + 3;

    glyphDel.x = 0;
    glyphDel.y = 0;
    glyphDel.w = 24;
    glyphDel.h = 16;
    glyphDel.texture = gTextures + DRTEXT_RED + 4;
}

void LoadMainMenu(void)
{
    LoadMainMenuOld();
}

void LoadScene(int32 trackNum)
{
    InitShadow();

    LoadSceneOld(trackNum);

    if (trackNo == 13)
    {
        InitAuroraBorialis(gTrack.sky);
    }
}