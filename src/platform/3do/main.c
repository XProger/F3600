void* RAM_LVL;
void* RAM_TEX;
void* RAM_CEL;
void* RAM_SND;

#include "game.h"

#define BLOCK_SIZE_DRAM     (32 * 1024)
#define BLOCK_SIZE_VRAM     (16 * 1024)

#define MAX_RAM_DRAM        (BLOCK_SIZE_DRAM * 29)
#define MAX_RAM_VRAM        (BLOCK_SIZE_VRAM * 44)

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
/*
#include <filefunctions.h>
void* readFile(char* fileName, void* buffer, int32 bufferSize)
{
    Item f;
    IOInfo params;
    DeviceStatus ds;
    Item req;
    void* ptr;
    int size;

    printf("readFile %s\n", fileName);

    f = OpenDiskFile(fileName);

    if (f < 0) {
        printf("failed to open file %s\n", fileName);
        return NULL;
    }

    req = CreateIOReq(NULL, 0, f, 0);
    memset(&params, 0, sizeof(IOInfo));
    memset(&ds, 0, sizeof(DeviceStatus));
    params.ioi_Command = CMD_STATUS;
    params.ioi_Recv.iob_Buffer = &ds;
    params.ioi_Recv.iob_Len = sizeof(DeviceStatus);

    ptr = buffer;

    if (DoIO(req, &params) >= 0)
    {
        size = ds.ds_DeviceBlockCount * ds.ds_DeviceBlockSize;

        if (bufferSize < size)
        {
            printf("not enough buffer size for %s. %d bytes required!\n", fileName, size);
            ptr = NULL;
        }

        if (ptr != NULL)
        {
            memset(&params, 0, sizeof(IOInfo));
            params.ioi_Command = CMD_READ;
            params.ioi_Recv.iob_Len = size;
            params.ioi_Recv.iob_Buffer = ptr;

            if (DoIO(req, &params) < 0)
            {
                printf("can't get file content!\n");
                ptr = NULL;
            }
        }
    }

    DeleteIOReq(req);
    CloseDiskFile(f);

    return ptr;
}*/

int main(int argc, char *argv[])
{
    uint8* memVRAM;
    uint8* memDRAM;
    int32 i;
    MemInfo memInfoVRAM;

    printf("wipEout 3DO\n");
    AvailMem(&memInfoVRAM, MEMTYPE_DRAM);
    printf("DRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);
    AvailMem(&memInfoVRAM, MEMTYPE_VRAM);
    printf("VRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);

    OpenMathFolio();
    OpenGraphicsFolio();
    OpenAudioFolio();
    InitEventUtility(1, 0, LC_Observer);
    CreateBasicDisplay(&screen, DI_TYPE_DEFAULT, 2);

    for (i = 0; i < 2; i++)
    {
        //SetCEControl(screen.sc_BitmapItems[i], 0xFFFFFFFF, ASCALL); // -5 ms but perf spikes and total blackout :(
        //DisableHAVG(screen.sc_BitmapItems[i]);
        //DisableVAVG(screen.sc_BitmapItems[i]);
    }

    irqVBL = GetVBLIOReq();
    irqVRAM = GetVRAMIOReq();
    irqTimer = GetTimerIOReq();

    memVRAM = (uint8*)AllocMem(MAX_RAM_VRAM, MEMTYPE_VRAM);
    memDRAM = (uint8*)AllocMem(MAX_RAM_DRAM, MEMTYPE_DRAM);

    if (!memVRAM) printf("VRAM failed!\n");
    if (!memDRAM) printf("DRAM failed!\n");
    /*
    RAM_TEX = memVRAM;
    RAM_LVL = memDRAM;
    RAM_CEL = memDRAM + MAX_RAM_LVL;
    RAM_SND = memDRAM + MAX_RAM_LVL + MAX_RAM_CEL;
    */
    //sndInit();

    gameInit();

    AvailMem(&memInfoVRAM, MEMTYPE_DRAM);
    printf("DRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);
    AvailMem(&memInfoVRAM, MEMTYPE_VRAM);
    printf("VRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);

    QueryGraphics(QUERYGRAF_TAG_FIELDCOUNT, &gFrameIndex);

    while (1)
    {
        WaitVBL(irqVBL, 1);
        clearFast(screenPage);

        inputUpdate();

        gLastFrameIndex = gFrameIndex;
        QueryGraphics(QUERYGRAF_TAG_FIELDCOUNT, &gFrameIndex);

        screenItem = screen.sc_ScreenItems[screenPage];

        //gameCtrl();

        drawInt(320 - 8, 4 + 16, rand() % 255);

        DisplayScreen(screen.sc_Screens[screenPage], NULL);
        screenPage ^= 1;
    }

    return 0;
}
