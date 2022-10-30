#include "game.h"

#define END_LAP 3

#define speedo_x    (179 + gBorder)
#define speedo_y    (195)

const uint16 glyphPos[3][40] = { // 8, 12, 16
    { 13, 13, 10, 13, 13, 9, 13, 10, 4, 9, 10, 10, 16, 10, 13, 13, 14, 113, 110, 109, 109, 109, 116, 114, 109, 110, 113, 106, 111, 110, 110, 111, 110, 110, 112, 114, 104, 104 },
    { 19, 19, 14, 19, 19, 13, 19, 14, 6, 13, 14, 14, 122, 114, 119, 118, 120, 119, 115, 114, 113, 113, 122, 219, 213, 214, 219, 208, 215, 213, 214, 215, 214, 215, 218, 219, 205, 205 },
    { 25, 24, 17, 24, 24, 17, 25, 18, 7, 17, 117, 117, 128, 117, 124, 124, 126, 124, 118, 117, 217, 217, 229, 224, 217, 218, 224, 210, 218, 217, 218, 318, 318, 318, 322, 325, 307, 307 }
};

Glyph glyphs[3][40]; // 8, 12, 16
Glyph glyphEnd;
Glyph glyphDel;

extern HiScores scoreTable[];
extern ChampionShip winTable[];

extern int short startLinePos[];

Texture* speedoTex;

Texture* gWeaponIcon;

void InitText(void)
{
    int i;

    LoadCompressedTextureSequence("wipeout/textures/drfonts.cmp", 0);

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
    speedoTex = LoadTexture("wipeout/textures/speedo.tim");
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

        q.color[0].r = q.color[1].r = q.color[2].r = q.color[3].r = 64;
        q.color[0].g = q.color[1].g = q.color[2].g = q.color[3].g = 0;
        q.color[0].b = q.color[1].b = q.color[2].b = q.color[3].b = 0;

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

        renderSetBlend(BLEND_ALPHA);
    }

    if (speed)
    {
        for (i = 0; i < NUM_BARS; i++)
        {
            p = gBarPoint + i;

            q.color[0].r = q.color[3].r = p[0].r >> 1;
            q.color[0].g = q.color[3].g = p[0].g >> 1;
            q.color[0].b = q.color[3].b = p[0].b >> 1;
            q.color[1].r = q.color[2].r = p[1].r >> 1;
            q.color[1].g = q.color[2].g = p[1].g >> 1;
            q.color[1].b = q.color[2].b = p[1].b >> 1;

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

void AddTime(int time, TEXT_DATA* pos_data, uint32 colour)
{
    int j;
    int digit[20];
    int x, y;
    int mins = 0, secs = 0, tenths = 0;
    int small_num_size;
    int size, charNum;
    Glyph* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

    size = pos_data->size;
    small_num_size = size >> 1;

    charSet = glyphs[(size - 8) >> 2];

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
        charNum = digit[j] + 26;
        renderGlyph(charSet + charNum, x, y, colour);
        x += charSet[charNum].w;
    }

    renderGlyph(charSet + 36, x, y, colour);
    x += charSet[36].w;

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
        charNum = digit[j] + 26;
        renderGlyph(charSet + charNum, x, y, colour);
        x += charSet[charNum].w;
    }

    renderGlyph(charSet + 37, x, y, colour);
    x += charSet[37].w;

    charSet = glyphs[0]; // 8

    if (size != 8)
    {
        size = 8;
        y += 8;
    }

    renderGlyph(charSet + tenths + 26, x, y, TEXT_COLOR_WHITE);
}

void AddText(const char* text_ptr, TEXT_DATA* pos_data, uint32 colour)
{
    uint8 letter;
    int size;
    int x, y;
    short j;
    Glyph* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

    size = pos_data->size;

    charSet = glyphs[(size - 8) >> 2];

    while (1)
    {
        letter = *text_ptr;
        text_ptr++;

        if (letter != 32)
        {
            if (letter == 0)
                return;

            if (letter >= '0' && letter <= '9')
                j = letter - '0' + 26;
            else
                j = letter - 'A';

            renderGlyph(charSet + j, x, y, colour);
            x += charSet[j].w;
        }
        else
        {
            x += 8;
        }
    }
}

void AddNumber(int num, TEXT_DATA* pos_data, uint32 colour)
{
    int i, j;
    int digit[12];
    int x, y;
    int size, charNum;
    uint16 currLett = 0;
    Glyph* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;
    size = pos_data->size;

    charSet = glyphs[(size - 8) >> 2];

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
        renderGlyph(charSet + charNum, x, y, colour);
        x += charSet[charNum].w;
    }
}

void AddDelEnd(char dispChar, int32 x, int32 y, uint32 colour)
{
    renderGlyph(dispChar ? &glyphEnd : &glyphDel, x, y, colour);
}

void SetCharTextures(int32 size)
{
    int32 currLett;
    int32 i, j;
    int32 x, y;
    int32 searchLett, row;
    const Texture* texture;
    const uint16* pos;
    Glyph* glyph;

    if (size == 8)
    {
        texture = gTextures + DRTEXT_8;
    }
    else if (size == 12)
    {
        texture = gTextures + DRTEXT_12;
    }
    else
    {
        texture = gTextures + DRTEXT_16;
    }

    pos = glyphPos[(size - 8) >> 2];
    glyph = glyphs[(size - 8) >> 2];

    for (j = 0; j < 40; j++)
    {
        currLett = pos[j];

        y = 0;

        row = (currLett / 100);

        y += (row * size);
        x = 0;

        // TODO WTF?
        for (i = 0; i < j; i++)
        {
            searchLett = pos[i];
            if ((searchLett / 100) == row)
                x += (searchLett % 100);
        }

        glyph[j].x = x;
        glyph[j].y = y;
        glyph[j].w = currLett % 100;
        glyph[j].h = size;
        glyph[j].texture = texture;
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
                    shipIndex[i].weaponType = WPN_TURBO;
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

    if (shipIndex[ownShip].lapNo > END_LAP && (shipIndex[ownShip].attr & RACING))
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
                {
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
    // TODO WTF!
    int i, j;
    int tempPosition;
    vec3i B, C, AB, AC, trackVect;
    int angleB, angleC, distanceB, distanceC;

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
                B.x = shipIndex[i].pos.x;
                B.y = shipIndex[i].pos.y;
                B.z = shipIndex[i].pos.z;

                C.x = shipIndex[j].pos.x;
                C.y = shipIndex[j].pos.y;
                C.z = shipIndex[j].pos.z;

                AB.x = B.x - shipIndex[i].nearTrkSect->centre.x;
                AB.y = B.y - shipIndex[i].nearTrkSect->centre.y;
                AB.z = B.z - shipIndex[i].nearTrkSect->centre.z;

                AC.x = C.x - shipIndex[j].nearTrkSect->centre.x;
                AC.y = C.y - shipIndex[j].nearTrkSect->centre.y;
                AC.z = C.z - shipIndex[j].nearTrkSect->centre.z;

                trackVect.x = shipIndex[i].nearTrkSect->nextSection->centre.x - shipIndex[i].nearTrkSect->centre.x;
                trackVect.y = shipIndex[i].nearTrkSect->nextSection->centre.y - shipIndex[i].nearTrkSect->centre.y;
                trackVect.z = shipIndex[i].nearTrkSect->nextSection->centre.z - shipIndex[i].nearTrkSect->centre.z;

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

        if (shipIndex[i].attr & RACING)
            winTable[i].racePosition = tempPosition;
    }

    return winTable[ownShip].racePosition;
}

void CentreText(const char* the_text_ptr, TEXT_DATA* pos_data, uint32 colour)
{
    uint8 letter = 0;
    const char* tmpPtr;
    int x, y;
    short j = 0;
    Glyph* charSet;
    short totSize = 0;

    y = pos_data->start_pos_y;

    charSet = glyphs[(pos_data->size - 8) >> 2];

    tmpPtr = the_text_ptr;

    letter = *tmpPtr;

    while (letter != 0)
    {
        if (letter >= '0' && letter <= '9')
            j = letter - '0' + 26;
        else
            j = letter - 'A';

        if (letter != 32)
            totSize += charSet[j].w;
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

            if (letter >= '0' && letter <= '9')
                j = letter - '0' + 26;
            else
                j = letter - 'A';

            renderGlyph(charSet + j, x, y, colour);
            x += charSet[j].w;
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
    const Texture* livesTex = gTextures + DRTEXT_RED + 5;

    for (i = 0; i < gameData->Lives; i++)
    {
        renderImage(livesTex, speedo_x + 115 - (13 * i), speedo_y - 5);
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
    int i;

    CentreText("CHAMPIONSHIP TABLE", text_data(60, 40, 8), TEXT_COLOR_HIGHLIGHT);
    AddText("PILOT NAME", text_data(85, 65, 8), TEXT_COLOR_HIGHLIGHT);
    AddText("POINTS", text_data(240, 65, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        int color = ((gFrameIndex & 1) || (i != ownShip)) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT;

        AddText(winTable[i].pilot, text_data(35, 80 + ((winTable[i].position) * 15), 8), color);
        AddNumber(winTable[i].points, text_data(255, 80 + ((winTable[i].position) * 15), 8), color);
    }
}

void DisplayRacePoints(void)
{
    int i;

    CentreText("RACE POINTS", text_data(60, 40, 8), TEXT_COLOR_HIGHLIGHT);
    AddText("PILOT NAME", text_data(85, 65, 8), TEXT_COLOR_HIGHLIGHT);
    AddText("POINTS", text_data(240, 65, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        int color = ((gFrameIndex & 1) || (i != ownShip)) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT;

        AddText(winTable[i].pilot, text_data(35, 80 + ((winTable[i].racePosition) * 15), 8), color);
        AddNumber(winTable[i].racePoints, text_data(255, 80 + ((winTable[i].racePosition) * 15), 8), color);
    }
}

void InitScreenTex(Texture** screenTex, const char* fileName)
{
    int tablePos;
    char modelFile[64];
    Texture* tex = gTexturePtr;

    strcpy(modelFile, "wipeout/textures/");
    strcat(modelFile, fileName);
    strcat(modelFile, ".cmp");

    LoadCompressedTextureSequence(modelFile, 0);

    screenTex[0] = tex++;
    screenTex[1] = tex++;
}

void InitWeaponSprtIcons(void)
{
    gWeaponIcon = gTexturePtr;
    LoadCompressedTextureSequence("wipeout/common/wicons.cmp", 0);
}

void DrawWeaponSprtIcons(ShipData* playerShip)
{
    int icon = playerShip->weaponType;

    if (icon != WPN_NONE)
    {
        icon -= 1;
        renderImage(gWeaponIcon + icon, 144, 20);
    }
}

void DisplayVenomComplete(void)
{
    CentreText("WELL DONE", text_data(60, 40, 8), TEXT_COLOR_WHITE);

    CentreText("VENOM CLASS", text_data(85, 65, 8), TEXT_COLOR_WHITE);
    CentreText("COMPETENCE ACHIEVED", text_data(85, 77, 8), TEXT_COLOR_WHITE);
    CentreText("YOU HAVE NOW QUALIFIED", text_data(240, 89, 8), TEXT_COLOR_WHITE);
    CentreText("FOR THE ULTRA FAST", text_data(240, 101, 8), TEXT_COLOR_WHITE);
    CentreText("RAPIER CLASS", text_data(240, 125, 8), TEXT_COLOR_WHITE);
    CentreText("WE RECOMMEND YOU", text_data(240, 200, 8), TEXT_COLOR_WHITE);
    CentreText("SAVE YOUR CURRENT GAME", text_data(240, 212, 8), TEXT_COLOR_WHITE);
}

void DisplayAllVenomComplete(void)
{
    CentreText("AMAZING", text_data(60, 40, 8), TEXT_COLOR_HIGHLIGHT);

    CentreText("YOU HAVE COMPLETED THE FULL", text_data(85, 65, 8), TEXT_COLOR_WHITE);
    CentreText("VENOM CLASS CHAMPIONSHIP", text_data(240, 77, 8), TEXT_COLOR_WHITE);
    CentreText("WELL DONE", text_data(240, 89, 8), TEXT_COLOR_WHITE);
    CentreText("YOU ARE A GREAT PILOT", text_data(240, 114, 8), TEXT_COLOR_WHITE);
    CentreText("NOW TAKE ON THE FULL", text_data(240, 126, 8), TEXT_COLOR_WHITE);
    CentreText("RAPIER CLASS CHAMPIONSHIP", text_data(240, 138, 8), TEXT_COLOR_WHITE);
    CentreText("KEEP GOING", text_data(240, 162, 8), TEXT_COLOR_WHITE);
}

void DisplayRaceInfo(ConfigData* gameData, HiScoreData* hiScoreData)
{
    short i;

    AddText("RACE STATISTICS", text_data(15, 80, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < 3; i++)
    {
        AddText("LAP", text_data(25, 95 + (i * 12), 8), TEXT_COLOR_HIGHLIGHT);
        AddNumber((i + 1), text_data(65, 95 + (i * 12), 8), TEXT_COLOR_HIGHLIGHT);
        AddTime(tempLapTimes[i], text_data(85, 95 + (i * 12), 8), TEXT_COLOR_WHITE);
    }

    AddText("RACE POSITION", text_data(15, 50, 8), TEXT_COLOR_HIGHLIGHT);
    AddNumber(gameData->Position, text_data(155, 50, 8), TEXT_COLOR_WHITE);

    /*TODO make some beauty*/
    if (gameData->gameType == SINGLE_RACE)
    {
        if ((CheckHiScore(gameData, hiScoreData)) && gameData->Position == 1)
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }
    else if (gameData->gameType == CHAMPIONSHIP)
    {
        if ((CheckHiScore(gameData, hiScoreData)) && (gameData->Position >= 1 && gameData->Position <= 3))
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }
    else if (gameData->gameType == TIME_TRIAL)
    {
        if (CheckHiScore(gameData, hiScoreData))
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }
    else
    {
        if (CheckHiScore(gameData, hiScoreData))
        {
            if (BLINK_SLOW)
            {
                AddText("RACE RECORD", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText("RACE TIME", text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }

    if (gameData->gameType == CHAMPIONSHIP)
    {
        if ((gameData->newLapRecord != 1) || BLINK_SLOW)
        {
            AddText("LAP RECORD", text_data(15, 194, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(scoreTable[((gameData->trackNum - 1) - gameData->trackInc)].lapRecord, text_data(25, 206, 8), TEXT_COLOR_WHITE);
        }
    }
    else if (gameData->gameType == TIME_TRIAL)
    {
        if ((gameData->newLapRecord != 1) || BLINK_SLOW)
        {
            AddText("LAP RECORD", text_data(15, 194, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(scoreTable[((gameData->trackNum - 1) + HiScoreTracks)].lapRecord, text_data(25, 206, 8), TEXT_COLOR_WHITE);
        }
    }
    else
    {
        if ((gameData->newLapRecord != 1) || BLINK_SLOW)
        {
            AddText("LAP RECORD", text_data(15, 194, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(scoreTable[(gameData->trackNum - 1)].lapRecord, text_data(25, 206, 8), TEXT_COLOR_WHITE);
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
                AddTime(playerShip->lapTimes[playerShip->lapNo - 1], text_data(16 - gBorder, 210, 16), TEXT_COLOR_WHITE);
            else
                AddTime(playerShip->lapTimes[2], text_data(16 - gBorder, 210, 16), TEXT_COLOR_WHITE);
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
                    AddTime(playerShip->lapTimes[i - 1], text_data(16 - gBorder, 205 - (10 * i), 8), TEXT_COLOR_HIGHLIGHT);
                }
            }
        }
    }

    if (playerShip->attr & RACING)
    {
        AddText("LAP", text_data(15 - gBorder, 8, 8), TEXT_COLOR_HIGHLIGHT);
        AddNumber(playerShip->lapNo, text_data(10 - gBorder, 19, 16), TEXT_COLOR_WHITE);

        width = glyphPos[2][(playerShip->lapNo + 26)] % 100; // 16

        AddText("OF", text_data((10 + width) - gBorder, 27, 8), TEXT_COLOR_HIGHLIGHT);

        width += 22;

        AddNumber(END_LAP, text_data((10 + width) - gBorder, 19, 16), TEXT_COLOR_WHITE);

        if (raceType != TIME_TRIAL)
        {
            AddText("POSITION", text_data(230 + gBorder, 8, 8), TEXT_COLOR_HIGHLIGHT);
            AddNumber(winTable[ownShip].racePosition, text_data(260 + gBorder, 19, 16), TEXT_COLOR_WHITE);
        }

        AddText("LAP RECORD", text_data(15 - gBorder, 43, 8), TEXT_COLOR_HIGHLIGHT);
        if (gameData->gameType == TIME_TRIAL)
            AddTime(scoreTable[((gameData->trackNum - 1) + HiScoreTracks)].lapRecord, text_data(15 - gBorder, 55, 8), TEXT_COLOR_WHITE);
        else
            AddTime(scoreTable[(gameData->trackNum - 1)].lapRecord, text_data(15 - gBorder, 55, 8), TEXT_COLOR_WHITE);
        if ((playerShip->attr & DIRECTION) == 0)
        {
            CentreText("WRONG WAY", text_data(0, 110, 16), TEXT_COLOR_HIGHLIGHT);
        }
    }

    if (playerShip->attr & RACING)
    {
        fill_speedo(playerShip->speed, playerShip->thrust_mag);
        renderImage(speedoTex, speedo_x, speedo_y);
    }
}
