#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pspkernel.h>
#include <psppower.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psprtc.h>
#include <pspaudiolib.h>
#include <pspaudio.h>
#include <pspgu.h>
#include <pspgum.h>

PSP_MODULE_INFO("F3600", 0, 1, 1);
PSP_HEAP_SIZE_KB(20 * 1024);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU | PSP_THREAD_ATTR_NO_FILLSTACK);

#include "game.h"

int32 isQuit;

int exitCallback(int arg1, int arg2, void *common)
{
    isQuit = 1;
    return 0;
}

int callbackThread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int setupCallbacks(void) 
{
    int thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
    sceKernelStartThread(thid, 0, 0);
    return thid;
}

// timing
u64 gTimerStart;
uint32 gTimerFreq;

void timerInit(void)
{
    gTimerFreq = sceRtcGetTickResolution();
    sceRtcGetCurrentTick(&gTimerStart);
}

int32 osGetSystemTimeMS(void)
{
    u64 time;
    sceRtcGetCurrentTick(&time);
    return (int32)((time - gTimerStart) * 1000 / gTimerFreq);
}

void osQuit(void)
{
    isQuit = 1;
}

// input
int32 osJoyReady(int32 index) 
{
    return index == 0;
}

void osJoyVibrate(int32 index, int32 L, int32 R) {
    //
}

void setDown(int32 key, int32 isDown)
{
    if (isDown)
    {
        gPad |= key;
    }
    else
    {
        gPad &= ~key;
    }
}

void inputInit(void)
{
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

void inputUpdate(void)
{
    SceCtrlData pad;
    int32 Lx, Ly;

    sceCtrlReadBufferPositive(&pad, 1);

    setDown(IN_UP,     (pad.Buttons & PSP_CTRL_UP));
    setDown(IN_DOWN,   (pad.Buttons & PSP_CTRL_DOWN));
    setDown(IN_LEFT,   (pad.Buttons & PSP_CTRL_LEFT));
    setDown(IN_RIGHT,  (pad.Buttons & PSP_CTRL_RIGHT));
    setDown(IN_A,      (pad.Buttons & PSP_CTRL_CROSS));
    setDown(IN_B,      (pad.Buttons & PSP_CTRL_CIRCLE));
    setDown(IN_X,      (pad.Buttons & PSP_CTRL_SQUARE));
    setDown(IN_Y,      (pad.Buttons & PSP_CTRL_TRIANGLE));
    setDown(IN_LT,     (pad.Buttons & PSP_CTRL_LTRIGGER));
    setDown(IN_RT,     (pad.Buttons & PSP_CTRL_RTRIGGER));
    setDown(IN_START,  (pad.Buttons & PSP_CTRL_START));
    setDown(IN_SELECT, (pad.Buttons & PSP_CTRL_SELECT));

    gPadStickX = gPadStickY = 256;

    Lx = pad.Lx - 128;
    Ly = pad.Ly - 128;

    if (abs(Lx) > 32)
    {
        gPadStickX = abs(Lx) << 1;
        setDown(IN_LEFT,  Lx < 0);
        setDown(IN_RIGHT, Lx > 0);
    }

    if (abs(Ly) > 32)
    {
        gPadStickY = abs(Ly) << 1;
        setDown(IN_UP,   Ly < 0);
        setDown(IN_DOWN, Ly > 0);
    }
}

// sound
void soundFill(void* buf, unsigned int length, void *userdata)
{
    FillNotes((int16*)buf, length);
}

void soundInit(void)
{
    pspAudioInit();
    pspAudioSetChannelCallback(0, soundFill, NULL);
}

// filesystem
int32 osLoadFile(const char* name, void* buffer)
{
    SceUID f;
    int32 size;

#if 0
    char path[256];
    strcpy(path, "ms0:/PSP/GAME/wipEout/");
    strcat(path, name);
    f = sceIoOpen(path, PSP_O_RDONLY, 0777);
#else
    f = sceIoOpen(name, PSP_O_RDONLY, 0777);
#endif

    if (f < 0)
    {
        LOG("can't open file %s\n", name);
        ASSERT(0);
        return 0;
    }

    size = sceIoLseek32(f, 0, SEEK_END);

    if (buffer)
    {
        int32 bytesRead;
        sceIoLseek32(f, 0, SEEK_SET);
        bytesRead = sceIoRead(f, buffer, size);
        if (bytesRead != size)
        {
            LOG("can't read file %s\n", name);
            sceIoClose(f);
            ASSERT(0);
            return 0;
        }
    }

    sceIoClose(f);
    return size;
}

int main(void)
{
    LOG("F3600 PSP\n");

    scePowerSetClockFrequency(333, 333, 166);
    setupCallbacks();

    gDRAM = pDRAM = (uint8*)memalign(64, DRAM_SIZE);
    gVRAM = pVRAM = (uint8*)memalign(64, VRAM_SIZE);

    timerInit();
    inputInit();

    gameInit();

    while (!isQuit)
    {
        inputUpdate();

        if (gPad & IN_SELECT) isQuit = 1;

        gLastFrameIndex = gFrameIndex;
        gFrameIndex = sceDisplayGetVcount();
        gFrameDelta = (gFrameIndex & 1) ? 128 : 0;
        gFrameIndex >>= 1; // to 30 Hz

        gameCtrl();

        renderSwap();
    }

    gameFree();


    sceKernelExitGame();

    return 0;
}