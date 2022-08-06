#include "game.h"

#define END_LAP 3

TexChar CharSet16[40];
TexChar CharSet12[40];
TexChar CharSet8[40];

#define speedo_x    (179 + gBorder)
#define speedo_y    (195)

const uint16 Text16[] = { 25, 24, 17, 24, 24, 17, 25, 18, 7, 17, 117, 117, 128, 117, 124, 124, 126, 124, 118, 117, 217, 217, 229, 224, 217, 218, 224, 210, 218, 217, 218, 318, 318, 318, 322, 325, 307, 307 };
const uint16 Text12[] = { 19, 19, 14, 19, 19, 13, 19, 14, 6, 13, 14, 14, 122, 114, 119, 118, 120, 119, 115, 114, 113, 113, 122, 219, 213, 214, 219, 208, 215, 213, 214, 215, 214, 215, 218, 219, 205, 205 };
const uint16 Text8[] = { 13, 13, 10, 13, 13, 9, 13, 10, 4, 9, 10, 10, 16, 10, 13, 13, 14, 113, 110, 109, 109, 109, 116, 114, 109, 110, 113, 106, 111, 110, 110, 111, 110, 110, 112, 114, 104, 104 };

extern HiScores scoreTable[];
extern ChampionShip winTable[];

extern int short startLinePos[];
extern const char gTrackList[15];

Texture* speedoTex;

Texture* gWeaponIconTextures[NO_OF_WEAPON_TYPES];

void AddQuad(int x, int y, int w, int h, int u0, int v0, int u1, int v1, const Texture* tex)
{
    Quad quad;

    quad.x = x;
    quad.y = y;
    quad.w = w;
    quad.h = h;
    quad.u0 = u0;
    quad.v0 = v0;
    quad.u1 = u1;
    quad.v1 = v1;
    quad.texture = tex;

    renderQuad(&quad);
}

void AddGlyph(int x, int y, int w, int h, int u, int v, const Texture* tex)
{
    AddQuad(x, y, w, h, u, v, u + w, v + h, tex);
}

void InitText(void)
{
    int i;
    extern int FontLoadHack;

    /**********************************************************/
    /***        Initialise In Game Character Set            ***/
    /**********************************************************/
    FontLoadHack = 1;

    ASSERT(TextureTableCount == 0);

    LoadCompressedTextureSequence("wipeout\\textures\\drfonts.cmp");

    ASSERT(TextureTableCount <= FontTextureTableMax);
    for (i = 0; i < TextureTableCount; i++)
        FontTextureTable[i] = TextureTable[i];
    TextureTableCount = 0;

    FontLoadHack = 0;
}

typedef struct 
{
    uint8 tx, ty;
    uint8 bx, by;
    uint8 r, g, b, unused;
} BarPoint;

const BarPoint gBarPoint[NUM_BARS + 1] = { 
    {   6, 18,      1, 27,     52,  17,  46, 0 },
    {  13, 18,      8, 27,    111,  31,  71, 0 },
    {  20, 18,     15, 27,    140,  46, 144, 0 },
    {  27, 18,     22, 27,    110,  84, 199, 0 },
    {  34, 18,     29, 27,     77, 137, 187, 0 },
    {  42, 17,     37, 27,     69, 173, 136, 0 },
    {  50, 16,     45, 27,     71, 195,  61, 0 },
    {  60, 13,     55, 25,    181, 218,  48, 0 },
    {  70, 10,     64, 23,    240, 134,  32, 0 },
    {  82,  8,     74, 22,    255, 193,  44, 0 },
    {  95,  7,     87, 22,    255, 215, 107, 0 },
    { 110,  7,    102, 22,    255, 234, 170, 0 },
    { 125,  7,    118, 22,    255, 255, 255, 0 }
};

void InitSpeedo(void)
{
    speedoTex = LoadTexture("wipeout\\textures\\speedo.tim");
}

#define BAR_THRUST      70
#define BAR_SPEED       2000
#define LERP_16(a,b,t)  a + ((b - a) * t >> 16)

void fill_speedo(int speed, int thrust)
{
    ColorQuad q;
    const BarPoint* p;
    int i, t;
    int sx, sy;

    sx = speedo_x;
    sy = speedo_y;

    if (thrust)
    {
        renderSetBlend(BLEND_ADD);

        q.r0 = q.r1 = q.r2 = q.r3 = 128;
        q.g0 = q.g1 = q.g2 = q.g3 = 0;
        q.b0 = q.b1 = q.b2 = q.b3 = 0;

        for (i = 0; i < NUM_BARS; i++)
        {
            p = gBarPoint + i;

            q.x0 = sx + p[0].tx;
            q.y0 = sy + p[0].ty;
            q.x3 = sx + p[0].bx;
            q.y3 = sy + p[0].by;

            if (thrust < BAR_THRUST)
            {
                t = thrust * ((1 << 16) / BAR_THRUST);
                q.x1 = sx + LERP_16(p[0].tx, p[1].tx, t);
                q.y1 = sy + LERP_16(p[0].ty, p[1].ty, t);
                q.x2 = sx + LERP_16(p[0].bx, p[1].bx, t);
                q.y2 = sy + LERP_16(p[0].by, p[1].by, t);
            }
            else
            {
                q.x1 = sx + p[1].tx;
                q.y1 = sy + p[1].ty;
                q.x2 = sx + p[1].bx;
                q.y2 = sy + p[1].by;
            }
       
            renderColorQuad(&q);

            thrust -= BAR_THRUST;
            if (thrust <= 0)
                break;
        }

        renderSetBlend(BLEND_NONE);
    }

    if (speed)
    {
        for (i = 0; i < NUM_BARS; i++)
        {
            p = gBarPoint + i;

            q.r0 = q.r3 = p[0].r;
            q.g0 = q.g3 = p[0].g;
            q.b0 = q.b3 = p[0].b;
            q.r1 = q.r2 = p[1].r;
            q.g1 = q.g2 = p[1].g;
            q.b1 = q.b2 = p[1].b;

            q.x0 = sx + p[0].tx;
            q.y0 = sy + p[0].ty;
            q.x3 = sx + p[0].bx;
            q.y3 = sy + p[0].by;

            if (speed < BAR_SPEED)
            {
                t = speed * ((1 << 16) / BAR_SPEED);
                q.x1 = sx + LERP_16(p[0].tx, p[1].tx, t);
                q.y1 = sy + LERP_16(p[0].ty, p[1].ty, t);
                q.x2 = sx + LERP_16(p[0].bx, p[1].bx, t);
                q.y2 = sy + LERP_16(p[0].by, p[1].by, t);
            }
            else
            {
                q.x1 = sx + p[1].tx;
                q.y1 = sy + p[1].ty;
                q.x2 = sx + p[1].bx;
                q.y2 = sy + p[1].by;
            }

            renderColorQuad(&q);

            speed -= BAR_SPEED;
            if (speed <= 0)
                break;
        }
    }
}

void AddTime(int time, TEXT_DATA* pos_data, int colour)
{
    int j;
    int digit[20];
    int x, y;
    int mins = 0, secs = 0, tenths = 0;
    Texture **charTex;
    int small_num_size;
    int size, charNum;
    uint16 currLett = 0;
    TexChar* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

    size = pos_data->size;
    small_num_size = size >> 1;

    switch (size)
    {
        case 16:
            charSet = &(CharSet16[0]);
            charTex = FontTextureTable + DRTEXT_16;
            break;

        case 12:
            charSet = &(CharSet12[0]);
            charTex = FontTextureTable + DRTEXT_12;
            break;

        case 8:
        default:
            charSet = &(CharSet8[0]);
            charTex = FontTextureTable + DRTEXT_8;
            break;
    }

    if (colour != WhiteText)
        charTex += DRTEXT_CLUT;

    tenths = (time / FR6) % 10;
    time = time / FR60;

    secs = time % 60;
    mins = time / 60;

    digit[0] = mins % 10;

    if (mins > 9)
    {
        mins = mins / 10;
        digit[1] = mins % 10;
    }
    else
        digit[1] = 0;

    for (j = 1; j >= 0; j--)
    {

        charNum = (digit[j] + 26);
        currLett = charSet[charNum].width;

        AddGlyph(x, y, currLett, size, charSet[charNum].vramX, charSet[charNum].vramY, *charTex);

        x += currLett;
    }

    currLett = charSet[36].width;

    AddGlyph(x, y, currLett, size, charSet[36].vramX, charSet[36].vramY, *charTex);

    x += currLett;

    digit[0] = secs % 10;
    if (secs > 9)
    {
        secs = secs / 10;
        digit[1] = secs % 10;
    }
    else
        digit[1] = 0;

    for (j = 1; j >= 0; j--)
    {
        charNum = (digit[j] + 26);
        currLett = charSet[charNum].width;

        AddGlyph(x, y, currLett, size, charSet[charNum].vramX, charSet[charNum].vramY, *charTex);

        x += currLett;
    }

    currLett = charSet[37].width;

    AddGlyph(x, y, currLett, size, charSet[37].vramX, charSet[37].vramY, *charTex);

    x += currLett;

    charSet = &(CharSet8[0]);
    charTex = FontTextureTable + DRTEXT_8;

    if (size != 8)
    {
        size = 8;
        y += 8;
    }

    currLett = charSet[(tenths + 26)].width;

    AddGlyph(x, y, currLett, size, charSet[tenths + 26].vramX, charSet[tenths + 26].vramY, *charTex);

    x += currLett;
}

void AddText(const char* text_ptr, TEXT_DATA* pos_data, int colour)
{
    uint8 letter = 0;
    char AValue = 0;
    Texture** charTex;
    int size;
    int x, y;
    short j = 0;
    uint16 currLett = 0;
    TexChar* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

    size = pos_data->size;

    switch (size)
    {
        case 16:
            charSet = &(CharSet16[0]);
            charTex = FontTextureTable + DRTEXT_16;
            break;

        case 12:
            charSet = &(CharSet12[0]);
            charTex = FontTextureTable + DRTEXT_12;
            break;

        case 8:
        default:
            charSet = &(CharSet8[0]);
            charTex = FontTextureTable + DRTEXT_8;
            break;
    }

    if (colour != WhiteText)
        charTex += DRTEXT_CLUT;

    while (1)
    {
        letter = *text_ptr;
        text_ptr++;

        if (letter != 32)
        {
            if (letter == 0)
                return;

            AValue = 'A';
            if (letter >= '0' && letter <= '9')
                j = letter - '0' + 26;
            else
                j = letter - AValue;

            currLett = charSet[j].width;

            AddGlyph(x, y, currLett, size, charSet[j].vramX, charSet[j].vramY, *charTex);

            x += currLett;
        }
        else
        {
            x += 8;
        }
    }
}

void AddNumber(int num, TEXT_DATA* pos_data, int colour)
{
    int i, j;
    int digit[12];
    int x, y;
    Texture** charTex;
    int size, charNum;
    uint16 currLett = 0;
    TexChar* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;
    size = pos_data->size;

    switch (size)
    {
        case 16:
            charSet = &(CharSet16[0]);
            charTex = FontTextureTable + DRTEXT_16;
            break;

        case 12:
            charSet = &(CharSet12[0]);
            charTex = FontTextureTable + DRTEXT_12;
            break;

        case 8:
        default:
            charSet = &(CharSet8[0]);
            charTex = FontTextureTable + DRTEXT_8;
            break;
    }

    if (colour != WhiteText)
        charTex += DRTEXT_CLUT;

    for (i = 0; i < 12; i++)
    {
        digit[i] = num % 10;
        num = num / 10;
        if (num == 0)
            break;
    }

    for (j = i; j >= 0; j--)
    {
        charNum = (digit[j] + 26);
        currLett = charSet[charNum].width;

        AddGlyph(x, y, currLett, size, charSet[charNum].vramX, charSet[charNum].vramY, *charTex);

        x += currLett;
    }
}

void AddDelEnd(char dispChar, TEXT_DATA* pos_data, int colour)
{
    Texture** endTex;
    Texture** charTex;
    int size;
    int x, y;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

    size = pos_data->size;

    endTex = FontTextureTable + DRTEXT_RED + 3;

    if (!dispChar)
    {
        endTex = FontTextureTable + DRTEXT_RED + 4; // delTex
    }

    charTex = endTex;
    if (colour != WhiteText)
        charTex += DRTEXT_CLUT;

    AddGlyph(x, y, 24, size, 0, 0, *charTex);
}

void SetCharTextures(char size)
{
    short currLett;
    short i, j;
    short vramY, vramX;
    short searchLett, vramRow;
    const uint16* charSet;

    switch (size)
    {
        case 8:
            charSet = &(Text8[0]);
            break;

        case 12:
            charSet = &(Text12[0]);
            break;

        case 16:
        default:
            charSet = &(Text16[0]);
            break;
    }

    for (j = 0; j < 40; j++)
    {
        currLett = charSet[j];

        vramY = 0;

        vramRow = (currLett / 100);

        vramY += (vramRow * size);
        vramX = 0;

        for (i = 0; i < j; i++)
        {
            searchLett = charSet[i];
            if ((searchLett / 100) == vramRow)
                vramX += (searchLett % 100);
        }

        switch (size)
        {
            case 8:
                CharSet8[j].vramX = vramX;
                CharSet8[j].vramY = vramY;
                CharSet8[j].width = (currLett % 100);
                break;
            case 12:
                CharSet12[j].vramX = vramX;
                CharSet12[j].vramY = vramY;
                CharSet12[j].width = (currLett % 100);
                break;
            case 16:
                CharSet16[j].vramX = vramX;
                CharSet16[j].vramY = vramY;
                CharSet16[j].width = (currLett % 100);
                break;
        }
    }
}

TEXT_DATA* text_data(int start_pos_x, int start_pos_y, int size)
{
    static TEXT_DATA text;

    text.start_pos_x = start_pos_x;
    text.start_pos_y = start_pos_y;
    text.size = size;

    return (&text);
}

void UpdateRaceHud(ShipData* shipIndex, int choice, ConfigData* gameData)
{
    int i, j;
    int position = NO_OF_SHIPS;
    int ownSecNo;
    int secNo;
    SortTable sortTable[NO_OF_SHIPS];
    char tempShip;
    char tempPoints;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i == ownShip)
        {
            if ((shipIndex[i].prevSecNo == (startLinePos[trackNo] + 1)) && (shipIndex[i].nearSecNo <= startLinePos[trackNo]))
            {
                crossedLine--;
            }
        }

        if ((shipIndex[i].prevSecNo == startLinePos[trackNo]) && ((shipIndex[i].nearSecNo > startLinePos[trackNo]) && (shipIndex[i].nearSecNo < (startLinePos[trackNo] + 10))))
        {
            if (i == ownShip)
            {
                if (shipIndex[i].lapNo == crossedLine)
                    shipIndex[i].lapNo++;

                crossedLine++;
                if (raceType == TIME_TRIAL)
                    shipIndex[i].weaponType = TURBO;
            }
            else
            {
                shipIndex[i].lapNo++;
            }
        }
    }

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        secNo = shipIndex[i].nearSecNo - (startLinePos[trackNo] + 1);

        if (i == ownShip)
        {
            if (secNo < 0)
            {
                secNo = secNo + (crossedLine * NoOfMainSections);
            }
            else
            {
                secNo = secNo + ((crossedLine - 1) * NoOfMainSections);
            }
        }
        else
        {
            if (secNo < 0)
            {
                secNo = secNo + (shipIndex[i].lapNo * NoOfMainSections);
            }
            else
            {
                secNo = secNo + ((shipIndex[i].lapNo - 1) * NoOfMainSections);
            }
        }

        shipIndex[i].DPASectDiff = secNo;
    }

    if (shipIndex[ownShip].nearSecNo <= startLinePos[trackNo])
        ownSecNo = shipIndex[ownShip].nearSecNo + track->sectionCount;
    else
        ownSecNo = shipIndex[ownShip].nearSecNo;

    position = CalcPosition(shipIndex);

    if (shipIndex[ownShip].lapNo > 0)
    {
        if ((choice == -1) && (shipIndex[ownShip].lapNo < END_LAP + 1))
        {
            shipIndex[ownShip].lapTimes[(shipIndex[ownShip].lapNo - 1)] += 2;
        }
    }

    if ((shipIndex[ownShip].lapNo != shipIndex[ownShip].prevLapNo) && (shipIndex[ownShip].lapNo > 1) && (shipIndex[ownShip].lapNo <= END_LAP + 1))
    {
        if ((CheckLapRecord(gameData, &(shipIndex[0]), ownShip)))
        {
            gameData->newLapRecord = 1;
        }
    }
    shipIndex[ownShip].prevLapNo = shipIndex[ownShip].lapNo;

    /************ HAVE FINISHED RACE *****************/
    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        if (i != ownShip)
        {
            if ((shipIndex[i].lapNo > END_LAP) && (shipIndex[i].attr & RACING))
            {
                shipIndex[i].attr &= ~RACING;
            }
        }
    }

    if (shipIndex[ownShip].lapNo > END_LAP /*&& ownSecNo == (startLinePos[ trackNo ]+1))*/ && (shipIndex[ownShip].attr & RACING))
    {
        gameData->Position = winTable[ownShip].racePosition;

        gameData->EndOfGame = 1;

        if (gameData->gameType == SINGLE_RACE || gameData->gameType == TIME_TRIAL)
        {
            gameData->showPilotCount = 120;
        }

        if (gameData->gameType == CHAMPIONSHIP)
        {
            gameData->showPilotCount = 120;

            if ((gameData->Position >= 1 && gameData->Position <= 3) && gameData->Lives > 0)
            {

                /* Find position during race */
                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    switch (winTable[i].racePosition)
                    {
                        case 1:
                            winTable[i].points += 9;
                            winTable[i].racePoints = 9;
                            break;

                        case 2:
                            winTable[i].points += 7;
                            winTable[i].racePoints = 7;
                            break;

                        case 3:
                            winTable[i].points += 5;
                            winTable[i].racePoints = 5;
                            break;

                        case 4:
                            winTable[i].points += 3;
                            winTable[i].racePoints = 3;
                            break;

                        case 5:
                            winTable[i].points += 2;
                            winTable[i].racePoints = 2;
                            break;

                        case 6:
                            winTable[i].points += 1;
                            winTable[i].racePoints = 1;
                            break;

                        case 7:
                            winTable[i].points += 0;
                            winTable[i].racePoints = 0;
                            break;

                        case 8:
                            winTable[i].points += 0;
                            winTable[i].racePoints = 0;
                            break;
                    }
                }

                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    sortTable[i].shipNum = i;
                    sortTable[i].points = winTable[i].points;
                }

                for (i = 0; i < NO_OF_SHIPS; i++)
                {

                    for (j = 0; j < (NO_OF_SHIPS - 1); j++)
                    {
                        if (sortTable[j].points < sortTable[j + 1].points)
                        {
                            tempShip = sortTable[j].shipNum;
                            tempPoints = sortTable[j].points;

                            sortTable[j].shipNum = sortTable[j + 1].shipNum;
                            sortTable[j].points = sortTable[j + 1].points;

                            sortTable[j + 1].shipNum = tempShip;
                            sortTable[j + 1].points = tempPoints;
                        }
                    }
                }

                for (i = 0; i < NO_OF_SHIPS; i++)
                {
                    winTable[(sortTable[i].shipNum)].position = (i + 1);
                }

                gameData->showPointCount = 120;
                gameData->showChampCount = 120;
            }

            gameData->gameOverCount = 120;

            if (gameData->Position > 3)
            {
                gameData->Lives--;
                gameData->trackInc = 0;
            }
            else
            {
                if (gameData->trackNum == NumTracks)
                {
                    if (winTable[ownShip].position == 1)
                    {
                        gameData->RapierClass = 1;
                        gameData->Class = 1;
                        if (gameData->showBonusTrack == 0)
                        {
                            gameData->trackNum += 2;
                            gameData->trackInc = 2;
                        }
                        else
                        {
                            gameData->trackNum++;
                            gameData->trackInc = 1;
                        }
                        gameData->actTrackNum = gTrackList[(gameData->trackNum - 1)];
                        gameData->venomComplete = 1;
                        gameData->gotoMenu = 1;
                    }
                    else
                    {
                        gameData->lostGame = 1;
                        gameData->trackInc = 0;
                    }
                }
                else

                    if (gameData->trackNum == totalTracks)
                {
                    if (winTable[ownShip].position == 1)
                    {
                        gameData->gameComplete = 1;
                        totalTracks = 14;
                        NumTracks = 7;
                        gameData->showBonusTrack = 1;
                    }
                    else
                    {
                        gameData->lostGame = 1;
                        gameData->trackInc = 0;
                    }
                }

                else
                {
                    gameData->trackInc = 1;
                    gameData->trackNum++;
                    gameData->actTrackNum = gTrackList[(gameData->trackNum - 1)];
                }
            }
        }

        shipIndex[ownShip].update = UpdateRemoteShip;
        shipIndex[ownShip].updateCount = 0;
        shipIndex[ownShip].remoteMaxThrust = 3160;
        shipIndex[ownShip].remoteThrustMag = 32;
        shipIndex[ownShip].speed = 3160;
        shipIndex[ownShip].attr &= ~SHIELDED;
        shipIndex[ownShip].attr &= ~VIEW_IN;
        shipIndex[ownShip].attr &= ~RACING;

        CameraUpdate = UpdateCameraAttractMode;

        tempLapTimes[0] = shipIndex[ownShip].lapTimes[0];
        tempLapTimes[1] = shipIndex[ownShip].lapTimes[1];
        tempLapTimes[2] = shipIndex[ownShip].lapTimes[2];
        tempLapTimes[3] = shipIndex[ownShip].lapTimes[3];
        tempLapTimes[4] = shipIndex[ownShip].lapTimes[4];

        if (gameData->gameType == CHAMPIONSHIP)
            attractDelay = 1;
        else
            attractDelay = 300;
    }
}

int CalcPosition(ShipData* shipIndex)
{
    int i, j;
    int tempPosition;
    VECTOR B, C, AB, AC, trackVect;
    int angleB, angleC, distanceB, distanceC;
    int position;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        tempPosition = 1;

        for (j = 0; j < NO_OF_SHIPS; j++)
        {
            if (i == j)
                continue;

            if (((shipIndex[j].attr & RACING) == 0))
                tempPosition++;

            else if (shipIndex[i].DPASectDiff == shipIndex[j].DPASectDiff)
            {
                B.vx = shipIndex[i].pos.vx;
                B.vy = shipIndex[i].pos.vy;
                B.vz = shipIndex[i].pos.vz;

                C.vx = shipIndex[j].pos.vx;
                C.vy = shipIndex[j].pos.vy;
                C.vz = shipIndex[j].pos.vz;

                AB.vx = B.vx - shipIndex[i].nearTrkSect->centre.vx;
                AB.vy = B.vy - shipIndex[i].nearTrkSect->centre.vy;
                AB.vz = B.vz - shipIndex[i].nearTrkSect->centre.vz;

                AC.vx = C.vx - shipIndex[j].nearTrkSect->centre.vx;
                AC.vy = C.vy - shipIndex[j].nearTrkSect->centre.vy;
                AC.vz = C.vz - shipIndex[j].nearTrkSect->centre.vz;

                trackVect.vx = shipIndex[i].nearTrkSect->nextSection.ptr->centre.vx - shipIndex[i].nearTrkSect->centre.vx;
                trackVect.vy = shipIndex[i].nearTrkSect->nextSection.ptr->centre.vy - shipIndex[i].nearTrkSect->centre.vy;
                trackVect.vz = shipIndex[i].nearTrkSect->nextSection.ptr->centre.vz - shipIndex[i].nearTrkSect->centre.vz;

                angleB = GetAng(&AB, &trackVect);
                angleC = GetAng(&AC, &trackVect);
                distanceB = (rcos(angleB >> 3) * GetMagnitude(&AB)) >> 12;
                distanceC = (rcos(angleC >> 3) * GetMagnitude(&AC)) >> 12;

                if ((distanceB == distanceC) && (i > j))
                    tempPosition++;

                else if (distanceB < distanceC)
                    tempPosition++;
            }
            else if (shipIndex[i].DPASectDiff < shipIndex[j].DPASectDiff)
            {
                tempPosition++;
            }
        }

        if (i == ownShip)
            position = tempPosition;

        if (shipIndex[i].attr & RACING)
            winTable[i].racePosition = tempPosition;
    }
    return (position);
}

void ScreenFooter(int x, int y, int colour)
{
    /*TODO screen footer
       Texture     *symbolTex, *redTex ;
       int         i ;
       int         offSet = 110 ;

       for (i=0; i < 2; i++)
       {
          symbolTex = (Texture *)FontTextureTable [DRTEXT_RED + (i+1)] ;

          SetPolyFT4(&(symbols[i]));
          symbols[i].r0=230;
          symbols[i].g0=0;
          symbols[i].b0=BLACK_NOT_DRAWN;

          if (i == 0)
          {
             setXY4(&(symbols[i]),  x, y,
                                    x+20, y,
                                    x, y+16,
                                    x+20, y+16) ;

             AddText ("SELECT",text_data (x+22,y+4, 8),WhiteText) ;

          }
          else
          {
             setXY4(&(symbols[i]),  x+offSet, y,
                                    x+20+offSet, y,
                                    x+offSet, y+16,
                                    x+20+offSet, y+16) ;

             AddText ("BACK",text_data (x+22+offSet,y+4, 8),WhiteText) ;
          }

          setUV4(&(symbols[i]),
                symbolTex->u0, symbolTex->v0,
                symbolTex->u1, symbolTex->v1,
                symbolTex->u2, symbolTex->v2+1,
                symbolTex->u3, symbolTex->v3+1) ;

          setTPage(&(symbols[i]),0,0,symbolTex->textureX,symbolTex->textureY);

          if (colour == WhiteText)
             symbols[i].clut = symbolTex->cba;
          else
          {
             redTex = (Texture *)FontTextureTable [DRTEXT_RED] ;
             symbols[i].clut = redTex->cba;
          }

          AddPrim( OT[ CurrentScreen ] , ( ulong* ) &(symbols[i]) );


       }
    */
}

void CentreText(const char* the_text_ptr, TEXT_DATA* pos_data, int colour)
{
    uint8 letter = 0;
    char AValue = 0;
    const char* tmpPtr;
    Texture** charTex;
    int size;
    int x, y;
    short j = 0;
    uint16 currLett = 0;
    TexChar* charSet;
    short totSize = 0;

    y = pos_data->start_pos_y;

    size = pos_data->size;

    switch (size)
    {
        case 16:
            charSet = &(CharSet16[0]);
            charTex = FontTextureTable + DRTEXT_16;
            break;

        case 12:
            charSet = &(CharSet12[0]);
            charTex = FontTextureTable + DRTEXT_12;
            break;

        case 8:
        default:
            charSet = &(CharSet8[0]);
            charTex = FontTextureTable + DRTEXT_8;
            break;
    }

    if (colour != WhiteText)
        charTex += DRTEXT_CLUT;

    tmpPtr = the_text_ptr;

    letter = *tmpPtr;

    while (letter != 0)
    {
        if (letter >= '0' && letter <= '9')
            j = letter - '0' + 26;
        else
            j = letter - 'A';

        if (letter != 32)
            totSize += charSet[j].width;
        else
            totSize += 8;

        tmpPtr++;
        letter = *tmpPtr;
    }

    x = 160 - (totSize >> 1);

    tmpPtr = the_text_ptr;
    while (1)
    {
        letter = *tmpPtr++;

        if (letter != 32)
        {
            if (letter == 0)
                return;

            AValue = 'A';

            if (letter >= '0' && letter <= '9')
                j = letter - '0' + 26;
            else
                j = letter - AValue;

            currLett = charSet[j].width;

            AddGlyph(x, y, currLett, size, charSet[j].vramX, charSet[j].vramY, *charTex);

            x += currLett;
        }
        else
        {
            x += 8;
        }
    }
}

void DisplayLives(ConfigData* gameData)
{
    int i;
    Texture** livesTex = FontTextureTable + DRTEXT_RED + 5;

    for (i = 0; i < gameData->Lives; i++)
    {
        AddGlyph(speedo_x + 115 - (13 * i), speedo_y - 5, 11, 9, 0, 0, *livesTex);
    }
}

void InitChampTable(void)
{
    int i;

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        winTable[i].points = 0;
        winTable[i].position = 1;
    }

    strcpy(winTable[0].pilot, "JOHN DEKKA");
    strcpy(winTable[1].pilot, "DANIEL CHANG");
    strcpy(winTable[2].pilot, "ARIAL TETSUO");
    strcpy(winTable[3].pilot, "ANASTASIA CHEROVOSKI");
    strcpy(winTable[4].pilot, "KEL SOLAAR");
    strcpy(winTable[5].pilot, "ARIAN TETSUO");
    strcpy(winTable[6].pilot, "SOFIA DE LA RENTE");
    strcpy(winTable[7].pilot, "PAUL JACKSON");
}

void DisplayChampTable(void)
{
    short i;

    CentreText("CHAMPIONSHIP TABLE", text_data(60, 40, 8), RedText);
    AddText("PILOT NAME", text_data(85, 65, 8), RedText);
    AddText("POINTS", text_data(240, 65, 8), RedText);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        int color = ((gFrameIndex & 1) || (i != ownShip)) ? WhiteText : RedText;

        AddText(winTable[i].pilot, text_data(35, 80 + ((winTable[i].position) * 15), 8), color);
        AddNumber(winTable[i].points, text_data(255, 80 + ((winTable[i].position) * 15), 8), color);
    }
}

void DisplayRacePoints(void)
{
    short i;

    CentreText("RACE POINTS", text_data(60, 40, 8), RedText);
    AddText("PILOT NAME", text_data(85, 65, 8), RedText);
    AddText("POINTS", text_data(240, 65, 8), RedText);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        int color = ((gFrameIndex & 1) || (i != ownShip)) ? WhiteText : RedText;

        AddText(winTable[i].pilot, text_data(35, 80 + ((winTable[i].racePosition) * 15), 8), color);
        AddNumber(winTable[i].racePoints, text_data(255, 80 + ((winTable[i].racePosition) * 15), 8), color);
    }
}

void InitScreenTex(Texture** screenTex, const char* fileName)
{
    int tablePos;
    char modelFile[64];

    strcpy(modelFile, "wipeout\\textures\\");
    strcat(modelFile, fileName);
    strcat(modelFile, ".cmp");

    tablePos = TextureTableCount;

    LoadCompressedTextureSequence(modelFile);

    screenTex[0] = TextureTable[tablePos++];
    screenTex[1] = TextureTable[tablePos++];
}

void InitWeaponSprtIcons()
{
    int tablePos = TextureTableCount;

    LoadCompressedTextureSequence("wipeout\\common\\wicons.cmp");

    gWeaponIconTextures[MINE - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[MISSILE - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[ROCKET - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[SPECIAL - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[ELEC_BOLT - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[FLARE - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[REV_CON - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[SHIELD - 1] = TextureTable[tablePos++];
    gWeaponIconTextures[TURBO - 1] = TextureTable[tablePos++];
}

void DrawWeaponSprtIcons(ShipData* playerShip)
{
    int icon = playerShip->weaponType;

    if (icon != NO_WEAPON)
    {
        icon -= 1;
        AddGlyph(144, 20, 32, 32, 0, 0, gWeaponIconTextures[icon]);
    }
}

void DisplayPicture(Texture* pic, short x, short y, short width, short height)
{
    AddQuad(x, y, width, height, 0, 0, width, height, pic);
}

void DisplayVenomComplete(void)
{
    CentreText("WELL DONE", text_data(60, 40, 8), WhiteText);

    CentreText("VENOM CLASS", text_data(85, 65, 8), WhiteText);
    CentreText("COMPETENCE ACHIEVED", text_data(85, 77, 8), WhiteText);
    CentreText("YOU HAVE NOW QUALIFIED", text_data(240, 89, 8), WhiteText);
    CentreText("FOR THE ULTRA FAST", text_data(240, 101, 8), WhiteText);
    CentreText("RAPIER CLASS", text_data(240, 125, 8), WhiteText);
    CentreText("WE RECOMMEND YOU", text_data(240, 200, 8), WhiteText);
    CentreText("SAVE YOUR CURRENT GAME", text_data(240, 212, 8), WhiteText);
}

void DisplayAllVenomComplete(void)
{
    CentreText("AMAZING", text_data(60, 40, 8), RedText);

    CentreText("YOU HAVE COMPLETED THE FULL", text_data(85, 65, 8), WhiteText);
    CentreText("VENOM CLASS CHAMPIONSHIP", text_data(240, 77, 8), WhiteText);
    CentreText("WELL DONE", text_data(240, 89, 8), WhiteText);
    CentreText("YOU ARE A GREAT PILOT", text_data(240, 114, 8), WhiteText);
    CentreText("NOW TAKE ON THE FULL", text_data(240, 126, 8), WhiteText);
    CentreText("RAPIER CLASS CHAMPIONSHIP", text_data(240, 138, 8), WhiteText);
    CentreText("KEEP GOING", text_data(240, 162, 8), WhiteText);
}

void InitTrackTex(Texture** trackTex)
{
    int tablePos = TextureTableCount;

    LoadCompressedTextureSequence("wipeout\\textures\\track.cmp");

    trackTex[0] = TextureTable[tablePos++];
    trackTex[1] = TextureTable[tablePos++];
    trackTex[2] = TextureTable[tablePos++];
    trackTex[3] = TextureTable[tablePos++];
    trackTex[4] = TextureTable[tablePos++];
    trackTex[5] = TextureTable[tablePos++];
    trackTex[6] = TextureTable[tablePos++];
}

void DisplayRaceInfo(ConfigData* gameData, HiScoreData* hiScoreData)
{
    short i;

    AddText("RACE STATISTICS", text_data(15, 80, 8), RedText);

    for (i = 0; i < 3; i++)
    {
        AddText("LAP", text_data(25, 95 + (i * 12), 8), RedText);
        AddNumber((i + 1), text_data(65, 95 + (i * 12), 8), RedText);
        AddTime(tempLapTimes[i], text_data(85, 95 + (i * 12), 8), WhiteText);
    }

    AddText("RACE POSITION", text_data(15, 50, 8), RedText);
    AddNumber(gameData->Position, text_data(155, 50, 8), WhiteText);

    /*TODO make some beauty*/
    if (gameData->gameType == SINGLE_RACE)
    {
        if ((CheckHiScore(gameData, hiScoreData)) && gameData->Position == 1)
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), RedText);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), RedText);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
        }
    }
    else if (gameData->gameType == CHAMPIONSHIP)
    {
        if ((CheckHiScore(gameData, hiScoreData)) && (gameData->Position >= 1 && gameData->Position <= 3))
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), RedText);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), RedText);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
        }
    }
    else if (gameData->gameType == TIME_TRIAL)
    {
        if (CheckHiScore(gameData, hiScoreData))
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), RedText);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), RedText);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
        }
    }
    else
    {
        if (CheckHiScore(gameData, hiScoreData))
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), RedText);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), RedText);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), WhiteText);
        }
    }

    if (gameData->gameType == CHAMPIONSHIP)
    {
        if (gameData->newLapRecord == 1)
        {
            if (BLINK_SLOW)
            {
                AddText("LAP RECORD", text_data(15, 194, 8), RedText);
                if (gameData->Position >= 1 && gameData->Position <= 3)
                    AddTime(scoreTable[((gameData->trackNum - 1) - gameData->trackInc)].lapRecord, text_data(25, 206, 8), WhiteText);
                else
                    AddTime(scoreTable[((gameData->trackNum - 1) - gameData->trackInc)].lapRecord, text_data(25, 206, 8), WhiteText);
            }
        }
        else
        {
            AddText("LAP RECORD", text_data(15, 194, 8), RedText);
            AddTime(scoreTable[((gameData->trackNum - 1) - gameData->trackInc)].lapRecord, text_data(25, 206, 8), WhiteText);
        }
    }
    else if (gameData->gameType == TIME_TRIAL)
    {
        if (gameData->newLapRecord == 1)
        {
            if (BLINK_SLOW)
            {
                AddText("LAP RECORD", text_data(15, 194, 8), RedText);
                AddTime(scoreTable[((gameData->trackNum - 1) + HiScoreTracks)].lapRecord, text_data(25, 206, 8), WhiteText);
            }
        }
        else
        {
            AddText("LAP RECORD", text_data(15, 194, 8), RedText);
            AddTime(scoreTable[((gameData->trackNum - 1) + HiScoreTracks)].lapRecord, text_data(25, 206, 8), WhiteText);
        }
    }
    else
    {
        if (gameData->newLapRecord == 1)
        {
            if (BLINK_SLOW)
            {
                AddText("LAP RECORD", text_data(15, 194, 8), RedText);
                AddTime(scoreTable[(gameData->trackNum - 1)].lapRecord, text_data(25, 206, 8), WhiteText);
            }
        }
        else
        {
            AddText("LAP RECORD", text_data(15, 194, 8), RedText);
            AddTime(scoreTable[(gameData->trackNum - 1)].lapRecord, text_data(25, 206, 8), WhiteText);
        }
    }
}

void DrawHud(ShipData* shipIndex, int choice, ConfigData* gameData)
{
    int i;
    int width;
    ShipData* playerShip = shipIndex + ownShip;

    if (gameData->gameType == CHAMPIONSHIP)
        DisplayLives(gameData);

    DrawWeaponSprtIcons(playerShip);

    if (playerShip->lapNo > 0) // gone past start line
    {
        if (playerShip->attr & RACING) // in race mode
        {
            if (playerShip->lapNo < END_LAP + 1) // not the last lap
                AddTime(playerShip->lapTimes[playerShip->lapNo - 1], text_data(16 - gBorder, 210, 16), WhiteText);
            else
                AddTime(playerShip->lapTimes[2], text_data(16 - gBorder, 210, 16), WhiteText);
        }
    }
    if (playerShip->attr & RACING)
    {
        if (playerShip->lapNo > 1)
        {
            for (i = 1; i < playerShip->lapNo; i++)
            {
                if (i < END_LAP)
                {
                    AddTime(playerShip->lapTimes[i - 1], text_data(16 - gBorder, 205 - (10 * i), 8), RedText);
                }
            }
        }
    }

    if (playerShip->attr & RACING)
    {
        AddText("LAP", text_data(15 - gBorder, 8, 8), RedText); // PCwipeout
        AddNumber(playerShip->lapNo, text_data(10 - gBorder, 19, 16), WhiteText); // PCwipeout

        width = Text16[(playerShip->lapNo + 26)] % 100;

        AddText("OF", text_data((10 + width) - gBorder, 27, 8), RedText);

        width += 22;

        AddNumber(END_LAP, text_data((10 + width) - gBorder, 19, 16), WhiteText);

        if (raceType != TIME_TRIAL)
        {
            AddText("POSITION", text_data(230 + gBorder, 8, 8), RedText);
            AddNumber(winTable[ownShip].racePosition, text_data(260 + gBorder, 19, 16), WhiteText);
        }

        AddText("LAP RECORD", text_data(15 - gBorder, 43, 8), RedText);
        if (gameData->gameType == TIME_TRIAL)
            AddTime(scoreTable[((gameData->trackNum - 1) + HiScoreTracks)].lapRecord, text_data(15 - gBorder, 55, 8), WhiteText);
        else
            AddTime(scoreTable[(gameData->trackNum - 1)].lapRecord, text_data(15 - gBorder, 55, 8), WhiteText);
        if ((playerShip->attr & DIRECTION) == 0)
        {
            CentreText("WRONG WAY", text_data(0, 110, 16), RedText);
        }
    }

    if (playerShip->attr & RACING)
    {
        fill_speedo(playerShip->speed, playerShip->thrust_mag);
        AddGlyph(speedo_x, speedo_y, SPEEDO_WIDTH, SPEEDO_HEIGHT, 0, 0, speedoTex);
    }
}
