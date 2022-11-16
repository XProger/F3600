void* RAM_LVL;
void* RAM_TEX;
void* RAM_CEL;
void* RAM_SND;

#include "game.h"
#include <filefunctions.h>

ScreenContext screen;

Item irqTimer;

int gPad;
int gPadStickX;
int gPadStickY;

int lastKey;

int gFrameIndex;
int gLastFrameIndex;

int32 osGetSystemTimeMS(void)
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
    if (joyMask & ControlLeftShift)  gPad |= IN_LT;
    if (joyMask & ControlRightShift) gPad |= IN_RT;
    if (joyMask & ControlStart)      gPad |= IN_START;
    if (joyMask & ControlX)          gPad |= IN_SELECT;
}

long osLoadFile(const char* name, char* buffer)
{
    Stream* fs;
    int32 size;

    fs = OpenDiskStream((char*)name, buffer ? 2048 : 0);
    if (!fs)
    {
        printf("failed to open file %s\n", name);
        return 0;
    }
    size = fs->st_FileLength;

    if (ReadDiskStream(fs, buffer, size) < 0)
    {
        printf("failed to read file %s\n", name);
        return 0;
    }

    CloseDiskStream(fs);
    return size;
}

void memCheck(void)
{
    MemInfo memInfoVRAM;
    AvailMem(&memInfoVRAM, MEMTYPE_DRAM);
    printf("DRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);
    AvailMem(&memInfoVRAM, MEMTYPE_VRAM);
    printf("VRAM: %d\n", (int)memInfoVRAM.minfo_SysFree);
}

extern void drawInt(int32 x, int32 y, int32 number);

int main(int argc, char *argv[])
{
    int32 fpsFrameIndex;
    int32 fpsFrame = 0;
    int32 fps = 0;

    printf("wipEout 3DO\n");

    OpenMathFolio();
    OpenGraphicsFolio();
    OpenAudioFolio();
    InitEventUtility(1, 0, LC_Observer);
    CreateBasicDisplay(&screen, DI_TYPE_DEFAULT, 2);

    irqTimer = GetTimerIOReq();

    memCheck();

    gDRAM = pDRAM = (char*)AllocMem(DRAM_SIZE, MEMTYPE_DRAM);
    gVRAM = pVRAM = (char*)AllocMem(VRAM_SIZE, MEMTYPE_VRAM);

    if (!gDRAM) printf("DRAM failed!\n");
    if (!gVRAM) printf("VRAM failed!\n");

    memCheck();

    srand(ReadHardwareRandomNumber());

    QueryGraphics(QUERYGRAF_TAG_FIELDCOUNT, &gFrameIndex);

    fpsFrameIndex = gFrameIndex;

    gameInit();

    while (1)
    {
        gLastFrameIndex = gFrameIndex;

        do
        {
            QueryGraphics(QUERYGRAF_TAG_FIELDCOUNT, &gFrameIndex);
        } while (gFrameIndex - gLastFrameIndex < 2);

        dbg_frameTime = osGetSystemTimeMS();

        inputUpdate();

        gameCtrl();

        dbg_frameTime = osGetSystemTimeMS() - dbg_frameTime;

        fpsFrame++;
        if (gFrameIndex >= fpsFrameIndex + 60)
        {
            fpsFrameIndex = gFrameIndex;
            fps = fpsFrame;
            fpsFrame = 0;
        }

        drawInt(320 - 8, 20, fps);
        drawInt(320 - 8, 28, dbg_update);
        drawInt(320 - 8, 36, dbg_render);
        drawInt(320 - 8, 36 + 12, dbg_frameTime);
        drawInt(320 - 8, 36 + 12 + 12, dbg_v);
        drawInt(320 - 8, 36 + 12 + 20, dbg_f);

        renderSwap();
    }

    return 0;
}
