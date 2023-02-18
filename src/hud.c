#include "game.h"

#define END_LAP 3

#define speedo_x    (guiR - (320 - 179))
#define speedo_y    (guiB - (240 - 195))

const uint16 glyphPos[3][40] = { // 8, 12, 16
    { 13, 13, 10, 13, 13, 9, 13, 10, 4, 9, 10, 10, 16, 10, 13, 13, 14, 113, 110, 109, 109, 109, 116, 114, 109, 110, 113, 106, 111, 110, 110, 111, 110, 110, 112, 114, 104, 104 },
    { 19, 19, 14, 19, 19, 13, 19, 14, 6, 13, 14, 14, 122, 114, 119, 118, 120, 119, 115, 114, 113, 113, 122, 219, 213, 214, 219, 208, 215, 213, 214, 215, 214, 215, 218, 219, 205, 205 },
    { 25, 24, 17, 24, 24, 17, 25, 18, 7, 17, 117, 117, 128, 117, 124, 124, 126, 124, 118, 117, 217, 217, 229, 224, 217, 218, 224, 210, 218, 217, 218, 318, 318, 318, 322, 325, 307, 307 }
};

Glyph glyphs[3][40]; // 8, 12, 16
Glyph glyphEnd;
Glyph glyphDel;

extern HiScores scoreTable[];

extern int short startLinePos[];

int32 gRacePos[NO_OF_SHIPS] = { 0, 1, 2, 3, 4, 5, 6, 7 };

Texture* gSpeedoTex;
Texture* gWinLoseTex;
Texture* gWeaponIcon;

typedef struct 
{
    uint8 tx, ty;
    uint8 bx, by;
    uint32 color;
} BarPoint;

#define BAR_COLOR(r, g, b) (r) | ((g) << 8) | ((b) << 16) | 0xFF000000

const BarPoint gBarPoint[NUM_BARS + 1] = { 
    {   6, 18,      1, 27,    BAR_COLOR(  52,  17,  46 ) },
    {  13, 18,      8, 27,    BAR_COLOR( 111,  31,  71 ) },
    {  20, 18,     15, 27,    BAR_COLOR( 140,  46, 144 ) },
    {  27, 18,     22, 27,    BAR_COLOR( 110,  84, 199 ) },
    {  34, 18,     29, 27,    BAR_COLOR(  77, 137, 187 ) },
    {  42, 17,     37, 27,    BAR_COLOR(  69, 173, 136 ) },
    {  50, 16,     45, 27,    BAR_COLOR(  71, 195,  61 ) },
    {  60, 13,     55, 25,    BAR_COLOR( 181, 218,  48 ) },
    {  70, 10,     64, 23,    BAR_COLOR( 240, 134,  32 ) },
    {  82,  8,     74, 22,    BAR_COLOR( 255, 193,  44 ) },
    {  95,  7,     87, 22,    BAR_COLOR( 255, 215, 107 ) },
    { 110,  7,    102, 22,    BAR_COLOR( 255, 234, 170 ) },
    { 125,  7,    118, 22,    BAR_COLOR( 255, 255, 255 ) }
};

#define BAR_THRUST      70
#define BAR_SPEED       2000
#define LERP_16(a,b,t)  a + ((b - a) * t >> 16)

void DisplaySpeedo(int32 speed, int32 thrust)
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

        q.color[0] =
        q.color[1] =
        q.color[2] =
        q.color[3] = BAR_COLOR(128, 0, 0);

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

            q.color[0] = q.color[3] = p[0].color;
            q.color[1] = q.color[2] = p[1].color;

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
    int size, charNum;
    Glyph* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

    size = pos_data->size;

#ifdef USE_FONT_SCALE
    charSet = glyphs[2];
#else
    charSet = glyphs[(size - 8) >> 2];
#endif

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
        x += renderGlyph(charSet + charNum, x, y, pos_data->size, colour);
    }

    x += renderGlyph(charSet + 36, x, y, pos_data->size, colour);

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
        x += renderGlyph(charSet + charNum, x, y, pos_data->size, colour);
    }

    x += renderGlyph(charSet + 37, x, y, pos_data->size, colour);

#ifndef USE_FONT_SCALE
    charSet = glyphs[0]; // 8
#endif

    if (size != 8)
    {
        size = 8;
        y += 8;
    }

    renderGlyph(charSet + tenths + 26, x, y, 8, TEXT_COLOR_WHITE);
}

void AddTextStr(const char* text_ptr, TEXT_DATA* pos_data, uint32 colour)
{
    uint8 letter;
    int x, y;
    short j;
    Glyph* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

#ifdef USE_FONT_SCALE
    charSet = glyphs[2];
#else
    charSet = glyphs[(pos_data->size - 8) >> 2];
#endif

    while (1)
    {
        letter = *text_ptr++;

        if (letter == 0)
            break;

        if (letter == '\'') // TODO special chars
            continue;

        if (letter == ' ')
        {
        #ifdef USE_FONT_SCALE
            x += 8 * pos_data->size / 8;
        #else
            x += 8;
        #endif
            continue;
        }

        if (letter >= '0' && letter <= '9')
            j = letter - '0' + 26;
        else
            j = letter - 'A';

        x += renderGlyph(charSet + j, x, y, pos_data->size, colour);
    }
}

void AddText(int32 strId, TEXT_DATA* pos_data, uint32 colour)
{
    AddTextStr(STR[strId], pos_data, colour);
}

void AddNumber(int num, TEXT_DATA* pos_data, uint32 colour)
{
    int i, j;
    int digit[12];
    int x, y;
    int charNum;
    Glyph* charSet;

    x = pos_data->start_pos_x;
    y = pos_data->start_pos_y;

#ifdef USE_FONT_SCALE
    charSet = glyphs[2];
#else
    charSet = glyphs[(pos_data->size - 8) >> 2];
#endif

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
        x += renderGlyph(charSet + charNum, x, y, pos_data->size, colour);
    }
}

void AddDelEnd(char dispChar, int32 x, int32 y, uint32 colour)
{
    renderGlyph(dispChar ? &glyphEnd : &glyphDel, x, y, 16, colour);
}

void SetCharTextures(int32 size)
{
    int32 currLett;
    int32 i, j;
    int32 x, y;
    int32 searchLett, row;
    Glyph* glyph;
    const Texture* texture;
    const uint16* pos;

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

void UpdateShipSecNo(ShipData* ship)
{
    int32 secNo;
    int32 startSecNo = startLinePos[gConfig.trackNum];

    if (ship->player)
    {
        if ((ship->prevSecNo == (startSecNo + 1)) && (ship->nearSecNo <= startSecNo))
        {
            ship->crossedLine--;
        }
    }

    if ((ship->prevSecNo == startSecNo) && ((ship->nearSecNo > startSecNo) && (ship->nearSecNo < (startSecNo + 10))))
    {
        if (ship->player)
        {
            if (ship->lapNo == ship->crossedLine)
            {
                ship->lapNo++;

                if ((ship->lapNo > 1) && (ship->lapNo <= END_LAP + 1))
                {
                    if (CheckLapRecord(ship))
                    {
                        gConfig.newLapRecord = 1; // TODO ship attr?
                    }
                }
            }

            ship->crossedLine++;
            if (gConfig.raceType == TIME_TRIAL)
            {
                ship->weaponType = WPN_TURBO;
            }
        }
        else
        {
            ship->lapNo++;

            if ((ship->lapNo > END_LAP) && (ship->attr & RACING))
            {
                ship->attr &= ~RACING;
            }
        }
    }

    secNo = ship->nearSecNo - (startSecNo + 1);

    if (ship->player)
    {
        if (secNo < 0)
        {
            secNo = secNo + (ship->crossedLine * NoOfMainSections);
        }
        else
        {
            secNo = secNo + ((ship->crossedLine - 1) * NoOfMainSections);
        }
    }
    else
    {
        if (secNo < 0)
        {
            secNo = secNo + (ship->lapNo * NoOfMainSections);
        }
        else
        {
            secNo = secNo + ((ship->lapNo - 1) * NoOfMainSections);
        }
    }

    ship->DPASectDiff = secNo;
}

void UpdateRaceHud(ShipData* shipIndex, int choice)
{
    int32 i, end;

    if (gConfig.raceType != TIME_TRIAL)
    {
        for (i = 0; i < NO_OF_SHIPS; i++)
        {
            UpdateShipSecNo(shipIndex + i);
        }
        CalcPosition(shipIndex);
    }
    else
    {
        for (i = 0; i < MAX_PLAYERS; i++)
        {
            if (gPlayers[i].ship)
            {
                UpdateShipSecNo(gPlayers[i].ship);
            }
        }
    }

    end = 1;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerData* player = gPlayers + i;
        ShipData* ship = player->ship;
        if (!ship)
            continue;

        if (ship->lapNo > 0)
        {
            if ((choice == -1) && (ship->lapNo < END_LAP + 1))
            {
                ship->lapTimes[ship->lapNo - 1] += 2;
            }
        }

        if (ship->lapNo > END_LAP)
        {
            if (ship->attr & RACING)
            {
                ship->update = UpdateRemoteShip;
                ship->updateCount = 0;
                ship->remoteMaxThrust = 3160;
                ship->remoteThrustMag = 32;
                ship->speed = 3160;
                ship->attr &= ~(SHIELDED | VIEW_IN | RACING);

                player->camera.update = UpdateCameraAttractMode;
            }
        }
        else
        {
            end = 0;
        }
    }

    if (end)
    {
        raceEnd();
    }
}

int ComparePosision(ShipData* a, ShipData* b)
{
    if (((a->attr | b->attr) & RACING) == 0)
        return 0;

    if ((a->attr & RACING) == 0)
        return -1;

    if ((b->attr & RACING) == 0)
        return 1;

    if (a->DPASectDiff < b->DPASectDiff)
    {
        return 1;
    }

    if (a->DPASectDiff > b->DPASectDiff)
    {
        return -1;
    }

    // a->DPASectDiff == b->DPASectDiff
    if (a->nearTrkSect == b->nearTrkSect)
    {
        vec3i ca, cb, dir;
        int32 da, db;
        int32 cx, cy, cz;

        cx = a->nearTrkSect->centre.x;
        cy = a->nearTrkSect->centre.y;
        cz = a->nearTrkSect->centre.z;

        //ASSERT(a->nearTrkSect == b->nearTrkSect);

        ca.x = a->pos.x - cx;
        ca.y = a->pos.y - cy;
        ca.z = a->pos.z - cz;

        cb.x = b->pos.x - cx;
        cb.y = b->pos.y - cy;
        cb.z = b->pos.z - cz;

        dir.x = a->nearTrkSect->nextSection->centre.x - cx;
        dir.y = a->nearTrkSect->nextSection->centre.y - cy;
        dir.z = a->nearTrkSect->nextSection->centre.z - cz;

        da = GetMagnitude(&ca);
        db = GetMagnitude(&cb);

        da = da ? (dot(&ca, &dir) / da) : 0;
        db = db ? (dot(&cb, &dir) / db) : 0;

        if (da < db)
            return 1;

        if (da > db)
            return -1;
    }

    return 0;
}

void CalcPosition(ShipData* shipIndex)
{
    int i, swapped;

    do
    {
        swapped = 0;
        for (i = 1; i < NO_OF_SHIPS; i++)
        {
            if (ComparePosision(shipIndex + gRacePos[i - 1], shipIndex + gRacePos[i]) > 0)
            {
                int32 t = gRacePos[i - 1];
                gRacePos[i - 1] = gRacePos[i];
                gRacePos[i] = t;
                swapped = 1;
            }
        }
    } while (swapped);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        winTable[gRacePos[i]].racePosition = i + 1;
    }
}

int32 GetTextWidth(Glyph* charSet, const char* text, int32 size)
{
    int32 index, width = 0;
    char c;

    while ((c = *text++) != 0)
    {
        if (c == '\'') // TODO special chars
            continue;

        if (c == ' ')
        {
        #ifdef USE_FONT_SCALE
            width += 8 * size / 8;
        #else
            width += 8;
        #endif
            continue;
        }

        if (c >= '0' && c <= '9')
            index = c - '0' + 26;
        else
            index = c - 'A';

    #ifdef USE_FONT_SCALE
        width += glyphs[2][index].w * size / 16;
    #else
        width += charSet[index].w;
    #endif
    }

    return width;
}

void CentreTextStr(const char* the_text_ptr, TEXT_DATA* pos_data, uint32 colour)
{
    uint8 letter = 0;
    const char* tmpPtr;
    int32 x, y, j, width;
    Glyph* charSet;

    y = pos_data->start_pos_y;

#ifdef USE_FONT_SCALE
    charSet = glyphs[2];
#else
    charSet = glyphs[(pos_data->size - 8) >> 2];
#endif

    tmpPtr = the_text_ptr;

    width = GetTextWidth(charSet, the_text_ptr, pos_data->size);


    x = 160 - (width >> 1) + pos_data->start_pos_x;

    tmpPtr = the_text_ptr;
    while (1)
    {
        letter = *tmpPtr++;

        if (letter == 0)
            break;

        if (letter == '\'') // TODO special chars
            continue;

        if (letter == ' ')
        {
        #ifdef USE_FONT_SCALE
            x += 8 * pos_data->size / 8;
        #else
            x += 8;
        #endif
            continue;
        }

        if (letter >= '0' && letter <= '9')
            j = letter - '0' + 26;
        else
            j = letter - 'A';

        x += renderGlyph(charSet + j, x, y, pos_data->size, colour);
    }
}

void CentreText(int32 strId, TEXT_DATA* pos_data, uint32 colour)
{
    CentreTextStr(STR[strId], pos_data, colour);
}

void DisplayLives(void)
{
    int i;
    const Texture* livesTex = gTextures + DRTEXT_RED + 5;

    for (i = 0; i < gConfig.lives; i++)
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

    strcpy(winTable[0].pilot, STR[STR_PILOT_JOHN]);
    strcpy(winTable[1].pilot, STR[STR_PILOT_DANIEL]);
    strcpy(winTable[2].pilot, STR[STR_PILOT_ARIAL]);
    strcpy(winTable[3].pilot, STR[STR_PILOT_ANASTASIA]);
    strcpy(winTable[4].pilot, STR[STR_PILOT_KEL]);
    strcpy(winTable[5].pilot, STR[STR_PILOT_ARIAN]);
    strcpy(winTable[6].pilot, STR[STR_PILOT_SOFIA]);
    strcpy(winTable[7].pilot, STR[STR_PILOT_PAUL]);
}

void DisplayChampTable(void)
{
    int32 i;
    uint32 color;

    CentreText(STR_CHAMPIONSHIP_TABLE, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
    AddText(STR_PILOT_NAME, text_data(85, 65, 8), TEXT_COLOR_HIGHLIGHT);
    AddText(STR_POINTS, text_data(240, 65, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        color = ((gFrameIndex & 1) || !ctrlshipIndex[i].player) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT;

        AddTextStr(winTable[i].pilot, text_data(35, 80 + ((winTable[i].position) * 15), 8), color);
        AddNumber(winTable[i].points, text_data(255, 80 + ((winTable[i].position) * 15), 8), color);
    }
}

void DisplayRacePoints(void)
{
    int32 i;
    uint32 color;

    CentreText(STR_RACE_POINTS, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
    AddText(STR_PILOT_NAME, text_data(85, 65, 8), TEXT_COLOR_HIGHLIGHT);
    AddText(STR_POINTS, text_data(240, 65, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < NO_OF_SHIPS; i++)
    {
        color = ((gFrameIndex & 1) || !ctrlshipIndex[i].player) ? TEXT_COLOR_WHITE : TEXT_COLOR_HIGHLIGHT;

        AddTextStr(winTable[i].pilot, text_data(35, 80 + ((winTable[i].racePosition) * 15), 8), color);
        AddNumber(winTable[i].racePoints, text_data(255, 80 + ((winTable[i].racePosition) * 15), 8), color);
    }
}

void DrawWeaponIcon(ShipData* ship)
{
    int icon = ship->weaponType;

    if (icon != WPN_NONE)
    {
        icon -= 1;
        renderImage(gWeaponIcon + icon, 144, guiT + 20);
    }
}

void DisplayVenomComplete(void)
{
    CentreText(STR_VENOM_COMP_1, text_data(0, 40, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_COMP_2, text_data(0, 65, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_COMP_3, text_data(0, 77, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_COMP_4, text_data(0, 89, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_COMP_5, text_data(0, 101, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_COMP_6, text_data(0, 125, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_COMP_7, text_data(0, 200, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_COMP_8, text_data(0, 212, 8), TEXT_COLOR_WHITE);
}

void DisplayAllVenomComplete(void)
{
    CentreText(STR_VENOM_ALL_1, text_data(0, 40, 8), TEXT_COLOR_HIGHLIGHT);
    CentreText(STR_VENOM_ALL_2, text_data(0, 65, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_ALL_3, text_data(0, 77, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_ALL_4, text_data(0, 89, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_ALL_5, text_data(0, 114, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_ALL_6, text_data(0, 126, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_ALL_7, text_data(0, 138, 8), TEXT_COLOR_WHITE);
    CentreText(STR_VENOM_ALL_8, text_data(0, 162, 8), TEXT_COLOR_WHITE);
}

void DisplayWinLose(int32 index)
{
    renderImage(gWinLoseTex + index, 180, 50);
}

void DisplayRaceInfo(HiScoreData* hiScoreData)
{
    int32 i;

    AddText(STR_RACE_STATISTICS, text_data(15, 80, 8), TEXT_COLOR_HIGHLIGHT);

    for (i = 0; i < 3; i++)
    {
        AddText(STR_LAP, text_data(25, 95 + (i * 12), 8), TEXT_COLOR_HIGHLIGHT);
        AddNumber((i + 1), text_data(65, 95 + (i * 12), 8), TEXT_COLOR_HIGHLIGHT);
        AddTime(tempLapTimes[i], text_data(85, 95 + (i * 12), 8), TEXT_COLOR_WHITE);
    }

    AddText(STR_RACE_POSITION, text_data(15, 50, 8), TEXT_COLOR_HIGHLIGHT);
    AddNumber(gConfig.Position, text_data(155, 50, 8), TEXT_COLOR_WHITE);

    /*TODO make some beauty*/
    if (gConfig.raceType == SINGLE_RACE)
    {
        if ((CheckHiScore(hiScoreData)) && gConfig.Position == 1)
        {
            if (BLINK_SLOW)
            {
                AddText(STR_RACE_RECORD, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText(STR_RACE_TIME, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }
    else if (gConfig.raceType == CHAMPIONSHIP)
    {
        if ((CheckHiScore(hiScoreData)) && (gConfig.Position >= 1 && gConfig.Position <= 3))
        {
            if (BLINK_SLOW)
            {
                AddText(STR_RACE_RECORD, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText(STR_RACE_TIME, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }
    else if (gConfig.raceType == TIME_TRIAL)
    {
        if (CheckHiScore(hiScoreData))
        {
            if (BLINK_SLOW)
            {
                AddText(STR_RACE_RECORD, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText(STR_RACE_TIME, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }
    else
    {
        if (CheckHiScore(hiScoreData))
        {
            if (BLINK_SLOW)
            {
                AddText(STR_RACE_RECORD, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
                AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
            }
        }
        else
        {
            AddText(STR_RACE_TIME, text_data(15, 170, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(hiScoreData->bestLap, text_data(25, 182, 8), TEXT_COLOR_WHITE);
        }
    }

    if (gConfig.raceType == CHAMPIONSHIP)
    {
        if ((gConfig.newLapRecord != 1) || BLINK_SLOW)
        {
            AddText(STR_LAP_RECORD, text_data(15, 194, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(scoreTable[gConfig.trackNum - gConfig.trackInc].lapRecord, text_data(25, 206, 8), TEXT_COLOR_WHITE);
        }
    }
    else if (gConfig.raceType == TIME_TRIAL)
    {
        if ((gConfig.newLapRecord != 1) || BLINK_SLOW)
        {
            AddText(STR_LAP_RECORD, text_data(15, 194, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(scoreTable[gConfig.trackNum + HiScoreTracks].lapRecord, text_data(25, 206, 8), TEXT_COLOR_WHITE);
        }
    }
    else
    {
        if ((gConfig.newLapRecord != 1) || BLINK_SLOW)
        {
            AddText(STR_LAP_RECORD, text_data(15, 194, 8), TEXT_COLOR_HIGHLIGHT);
            AddTime(scoreTable[gConfig.trackNum].lapRecord, text_data(25, 206, 8), TEXT_COLOR_WHITE);
        }
    }
}

void DrawHud(ShipData* ship)
{
    int i;
    int width;

    if (gConfig.raceType == CHAMPIONSHIP)
        DisplayLives();

    DrawWeaponIcon(ship);

    if (ship->lapNo > 0) // gone past start line
    {
        if (ship->attr & RACING) // in race mode
        {
            if (ship->lapNo < END_LAP + 1) // not the last lap
                AddTime(ship->lapTimes[ship->lapNo - 1], text_data(guiL + 16, guiB - (240 - 210), 16), TEXT_COLOR_WHITE);
            else
                AddTime(ship->lapTimes[2], text_data(guiL + 16, guiB - (240 - 210), 16), TEXT_COLOR_WHITE);
        }
    }
    if (ship->attr & RACING)
    {
        if (ship->lapNo > 1)
        {
            for (i = 1; i < ship->lapNo; i++)
            {
                if (i < END_LAP)
                {
                    AddTime(ship->lapTimes[i - 1], text_data(guiL + 16, guiB - (240 - 205) - (10 * i), 8), TEXT_COLOR_HIGHLIGHT);
                }
            }
        }
    }

    if (ship->attr & RACING)
    {
        AddText(STR_LAP, text_data(guiL + 15, guiT + 8, 8), TEXT_COLOR_HIGHLIGHT);
        AddNumber(ship->lapNo, text_data(guiL + 10, guiT + 19, 16), TEXT_COLOR_WHITE);

        width = glyphPos[2][(ship->lapNo + 26)] % 100; // 16

        AddText(STR_LAP_OF, text_data(guiL + (10 + width), guiT + 27, 8), TEXT_COLOR_HIGHLIGHT);

        width += 22;

        AddNumber(END_LAP, text_data(guiL + (10 + width), guiT + 19, 16), TEXT_COLOR_WHITE);

        if (gConfig.raceType != TIME_TRIAL)
        {
        #ifdef USE_FONT_SCALE
            #define POS_TEXT_OFFSET 235
        #else
            #define POS_TEXT_OFFSET 230
        #endif
            AddText(STR_POSITION, text_data(guiR - (320 - POS_TEXT_OFFSET), guiT + 8, 8), TEXT_COLOR_HIGHLIGHT);
            AddNumber(winTable[ship - ctrlshipIndex].racePosition, text_data(guiR - (320 - 260), guiT + 19, 16), TEXT_COLOR_WHITE);
        }

        AddText(STR_LAP_RECORD, text_data(guiL + 15, guiT + 43, 8), TEXT_COLOR_HIGHLIGHT);

        if (gConfig.raceType == TIME_TRIAL)
            AddTime(scoreTable[gConfig.trackNum + HiScoreTracks].lapRecord, text_data(guiL + 15, guiT + 55, 8), TEXT_COLOR_WHITE);
        else
            AddTime(scoreTable[gConfig.trackNum].lapRecord, text_data(guiL + 15, guiT + 55, 8), TEXT_COLOR_WHITE);

        if ((ship->attr & DIRECTION) == 0)
        {
            CentreText(STR_WRONG_WAY, text_data(0, 110, 16), TEXT_COLOR_HIGHLIGHT);
        }
    }

    if (ship->attr & RACING)
    {
        DisplaySpeedo(ship->speed, ship->thrustMag);
        renderImage(gSpeedoTex, speedo_x, speedo_y);
    }
}

void UpdateGameDisplay(int32 width, int32 height)
{
    int32 scale, dx, dy;

    if (3 * width > 4 * height)
    {
        scale = height / 240;
    }
    else
    {
        scale = width / 320;
    }

    if (scale < 1)
    {
        scale = 1;
    }

    dx = ((width - scale * 320) + scale - 1) / scale;
    dy = ((height - scale * 240) + scale - 1) / scale;

    guiL = -(dx >> 1);
    guiT = -(dy >> 1);
    guiR = guiL + 320 + dx;
    guiB = guiT + 240 + dy;

#ifdef USE_WIDE_STRETCH
    {
        extern float gAspect;
        float f = gAspect * height / width;
        int32 w = (int32)((guiR - guiL) * (f - 1.0f) * 0.5f);
        guiL -= w;
        guiR += w;
    }
#endif
}