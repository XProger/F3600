void* RAM_LVL;
void* RAM_TEX;
void* RAM_CEL;
void* RAM_SND;

#include "game.h"
#include <filefunctions.h>

Item irqVBL;
Item irqVRAM;
Item irqTimer;

ScreenContext screen;
int32 screenPage;
Item  screenItem;

int gPad;
int gPadStickX;
int gPadStickY;

int lastKey;

int gFrameIndex;
int gLastFrameIndex;

const int chars[10][20] = {
    {
        0,1,1,0,
        1,0,0,1,
        1,0,0,1,
        1,0,0,1,
        0,1,1,0
    }, {
        0,0,1,0,
        0,1,1,0,
        0,0,1,0,
        0,0,1,0,
        0,0,1,0
    }, {
        0,1,1,0,
        1,0,0,1,
        0,0,1,1,
        0,1,0,0,
        1,1,1,1
    }, {
        1,1,1,0,
        0,0,0,1,
        0,0,1,0,
        1,0,0,1,
        0,1,1,0
    }, {
        0,0,0,1,
        0,0,1,1,
        0,1,0,1,
        1,1,1,1,
        0,0,0,1
    }, {
        1,1,1,1,
        1,0,0,0,
        1,1,1,0,
        0,0,0,1,
        1,1,1,0
    }, {
        0,1,1,0,
        1,0,0,0,
        1,1,1,0,
        1,0,0,1,
        0,1,1,0
    }, {
        1,1,1,1,
        0,0,0,1,
        0,0,1,0,
        0,1,0,0,
        0,1,0,0
    }, {
        0,1,1,0,
        1,0,0,1,
        0,1,1,0,
        1,0,0,1,
        0,1,1,0
    }, {
        0,1,1,0,
        1,0,0,1,
        0,1,1,1,
        0,0,0,1,
        0,1,1,0
    }
};

void drawChar(int32 x, int32 y, int32 c)
{
    int32 i, j;
    uint16* ptr = (uint16*)screen.sc_Bitmaps[screenPage]->bm_Buffer;

    ptr += y / 2 * 2 * 320;
    ptr += x * 2;
    ptr += y & 1;

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (chars[c][i * 4 + j])
            {
                ptr[j * 2] = 0xFFFF;
            }
        }

        if ((y + i) & 1) {
            ptr += 320 * 2 - 1;
        } else {
            ptr += 1;
        }
    }
}

void drawInt(int32 x, int32 y, int32 number)
{
    while (number)
    {
        drawChar(x, y, number % 10);
        number /= 10;
        x -= 6;
    }
}

int32 osGetSystemTimeMS()
{
    return GetMSecTime(irqTimer);
}

void osQuit()
{
    //
}

void osJoyVibrate(int index, int L, int R)
{
    //
}

IOInfo clearInfo = {
    FLASHWRITE_CMD, IO_QUICK, 0, 0, -1, 0, 0,
    { NULL, 0 },
    { NULL, 320 * 240 * 2 }
};

void clearFast(int32 page)
{
    clearInfo.ioi_Recv.iob_Buffer = screen.sc_Bitmaps[page]->bm_Buffer;
    SendIO(irqVRAM, &clearInfo);
}

uint32 joyMask;

void inputUpdate()
{
    ControlPadEventData event;
    gPadStickX = gPadStickY = 256;
    gPad = 0;

    event.cped_ButtonBits = 0;
    if (GetControlPad(1, 0, &event)) {
        joyMask = event.cped_ButtonBits;
    }

    if (joyMask & ControlUp)         gPad |= IN_UP;
    if (joyMask & ControlRight)      gPad |= IN_RIGHT;
    if (joyMask & ControlDown)       gPad |= IN_DOWN;
    if (joyMask & ControlLeft)       gPad |= IN_LEFT;
    if (joyMask & ControlA)          gPad |= IN_Y;
    if (joyMask & ControlB)          gPad |= IN_A;
    if (joyMask & ControlC)          gPad |= IN_B;
    if (joyMask & ControlLeftShift)  gPad |= IN_L;
    if (joyMask & ControlRightShift) gPad |= IN_R;
    if (joyMask & ControlStart)      gPad |= IN_START;
    if (joyMask & ControlX)          gPad |= IN_SELECT;
}


#if 1
long osLoadFile(const char* name, char* buffer)
{
    Stream* fs;
    int32 size;

    printf("load file %s\n", name);

    fs = OpenDiskStream((char*)name, buffer ? 2048 : 0);
    if (!fs)
    {
        printf("failed to open file\n");
        return 0;
    }
    size = fs->st_FileLength;
    printf("size = %d\n", size);

    if (ReadDiskStream(fs, buffer, size) < 0)
    {
        printf("failed to read file\n");
        return 0;
    }

    CloseDiskStream(fs);
    return size;
}
#else
long osLoadFile(const char* name, char* buffer)
{
    Item f;
    Item req;
    long size;
    IOInfo params;
    DeviceStatus ds;
    int filesize;
    Stream *fs;






    f = OpenDiskFile((char*)name);

    if (f < 0) {
        printf("failed to open file %s\n", name);
        return 0;
    }

    req = CreateIOReq(NULL, 0, f, 0);
    memset(&params, 0, sizeof(IOInfo));
    memset(&ds, 0, sizeof(DeviceStatus));
    params.ioi_Command = CMD_STATUS;
    params.ioi_Recv.iob_Len = sizeof(DeviceStatus);
    params.ioi_Recv.iob_Buffer = &ds;

    if (DoIO(req, &params) >= 0)
    {
        size = ds.ds_DeviceBlockCount * ds.ds_DeviceBlockSize;
        printf("size = %d\n", size);

        if (buffer)
        {
            memset(&params, 0, sizeof(IOInfo));
            params.ioi_Command = CMD_READ;
            params.ioi_Recv.iob_Len = size;
            params.ioi_Recv.iob_Buffer = buffer;

            if (DoIO(req, &params) < 0)
            {
                printf("can't get file content!\n");
            }
        }
    }

    DeleteIOReq(req);
    CloseDiskFile(f);

    return size;
}
#endif

void memCheck(void)
{
    MemInfo memInfoVRAM;
    AvailMem(&memInfoVRAM, MEMTYPE_DRAM);
    printf("DRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);
    AvailMem(&memInfoVRAM, MEMTYPE_VRAM);
    printf("VRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);
}

void EnableSuperClipping()
{
    int32 i;
    for (i = 0; i < 2; i++)
    {
        SetCEControl(screen.sc_BitmapItems[i], 0xFFFFFFFF, ASCALL);
        DisableHAVG(screen.sc_BitmapItems[i]);
        DisableVAVG(screen.sc_BitmapItems[i]);
    }
}

int main(int argc, char *argv[])
{
    printf("wipEout 3DO\n");

    OpenMathFolio();
    OpenGraphicsFolio();
    OpenAudioFolio();
    InitEventUtility(1, 0, LC_Observer);
    CreateBasicDisplay(&screen, DI_TYPE_DEFAULT, 2);

    //EnableSuperClipping(); // -5 ms but perf spikes and total blackout :(

    irqVBL = GetVBLIOReq();
    irqVRAM = GetVRAMIOReq();
    irqTimer = GetTimerIOReq();

    //sndInit();
    memCheck();

    gDRAM = pDRAM = (char*)AllocMem(DRAM_SIZE, MEMTYPE_DRAM);
    gVRAM = pVRAM = (char*)AllocMem(VRAM_SIZE, MEMTYPE_VRAM);

    if (!gDRAM) printf("DRAM failed!\n");
    if (!gVRAM) printf("VRAM failed!\n");

    memCheck();

    gameInit();

    QueryGraphics(QUERYGRAF_TAG_FIELDCOUNT, &gFrameIndex);

    while (1)
    {
        WaitVBL(irqVBL, 1);
        clearFast(screenPage);

        inputUpdate();

        gLastFrameIndex = gFrameIndex;
        QueryGraphics(QUERYGRAF_TAG_FIELDCOUNT, &gFrameIndex);

        screenItem = screen.sc_ScreenItems[screenPage];

        gameCtrl();

        drawInt(320 - 8, 4 + 16, rand() % 255);

        DisplayScreen(screen.sc_Screens[screenPage], NULL);
        screenPage ^= 1;
    }

    return 0;
}
