#include <sys/systime.h>
#include <sys/process.h>
#include <sys/thread.h>
#include <sysutil/sysutil.h>
#include <lv2/sysfs.h>
#include <rsx/gcm_sys.h>
#include <io/pad.h>
#include <audio/audio.h>

#include "game.h"

int32 isQuit;

void osQuit(void)
{
    isQuit = 1;
}

// timing
s64 gTimerStart;

void timerInit(void)
{
    gTimerStart = sysGetSystemTime();
}

int32 osGetSystemTimeMS(void)
{
    return (int32)((sysGetSystemTime() - gTimerStart) / 1000);
}

// input
#define PAD_STATUS_CONNECTED        (1 << 0)
#define PAD_CAPABILITY_ACTUATOR     (1 << 4)

#define JOY_MIN_UPDATE_FX_TIME      50
#define JOY_RUMBLE_FADE             16

typedef struct {
    int32 time;
    int32 vL, vR; // current value for left/right motor vibration
    int32 oL, oR; // last applied value
} JoyDevice;

JoyDevice joyDevice[1];

int32 osJoyReady(int32 index)
{
    return index == 0;
}

void osJoyVibrate(int32 index, int32 L, int32 R)
{
    joyDevice[index].vL = L;
    joyDevice[index].vR = R;
}

void joyRumble(int index)
{
    padActParam act;
    JoyDevice* joy = joyDevice + index;

    if (!osJoyReady(index))
        return;

    if (!(joy->vL + joy->vR + joy->oL + joy->oR))
        return;

    if (osGetSystemTimeMS() < joy->time)
        return;

    memset(&act, 0, sizeof(act));
    act.motor[0] = joy->vL;
    act.motor[1] = joy->vR;
    ioPadSetActDirect(index, &act);

    joy->oL = joy->vL;
    joy->oR = joy->vR;
    joy->vL -= JOY_RUMBLE_FADE;
    joy->vR -= JOY_RUMBLE_FADE;
    if (joy->vL < 0)
        joy->vL = 0;
    if (joy->vR < 0)
        joy->vR = 0;
    joy->time = osGetSystemTimeMS() + JOY_MIN_UPDATE_FX_TIME;
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
    ioPadInit(7);
}

void inputFree(void)
{
    ioPadEnd();
}

void inputUpdate(void)
{
    padInfo2 info;
    padData data;
    int32 Lx, Ly;

    int32 index = 0;

    if (ioPadGetInfo2(&info) != 0)
        return;

    if (!(info.port_status[index] & PAD_STATUS_CONNECTED))
    {
        gLastKey = -1;
        gPad = 0;
        return;
    }

    if (info.device_capability[index] & PAD_CAPABILITY_ACTUATOR)
    {
        joyRumble(index);
    }

    if ((ioPadGetData(index, &data) != 0) || (data.len <= 0))
        return;

    setDown(IN_LEFT,    data.BTN_LEFT);
    setDown(IN_DOWN,    data.BTN_DOWN);
    setDown(IN_RIGHT,   data.BTN_RIGHT);
    setDown(IN_UP,      data.BTN_UP);
    setDown(IN_START,   data.BTN_START);
    setDown(IN_R,       data.BTN_R3);
    setDown(IN_L,       data.BTN_L3);
    setDown(IN_SELECT,  data.BTN_SELECT);
    setDown(IN_X,       data.BTN_SQUARE);
    setDown(IN_A,       data.BTN_CROSS);
    setDown(IN_B,       data.BTN_CIRCLE);
    setDown(IN_Y,       data.BTN_TRIANGLE);
    setDown(IN_RB,      data.BTN_R1);
    setDown(IN_LB,      data.BTN_L1);
    setDown(IN_RT,      data.BTN_R2);
    setDown(IN_LT,      data.BTN_L2);

    gPadStickX = gPadStickY = 256;

    Lx = data.ANA_L_H - 128;
    Ly = data.ANA_L_V - 128;

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
uint32 audioPort;
float* audioPortAddr;
sys_ppu_thread_t audioThreadId;
sys_event_queue_t audioQueue;
sys_ipc_key_t audioQueueKey;
int16 audioBuffer[AUDIO_BLOCK_SAMPLES * AUDIO_PORT_2CH];

void audioThread(u64 arg)
{
    int32 block = 0;
    int16* src;
    float* dst;

    memset(audioPortAddr, 0, AUDIO_BLOCK_SAMPLES * 2 * 8 * sizeof(float));

    audioCreateNotifyEventQueue(&audioQueue, &audioQueueKey);
    audioSetNotifyEventQueue(audioQueueKey);

    sysEventQueueDrain(audioQueue);

    audioPortStart(audioPort);

    while (!isQuit)
    {
        int32 i;
        sys_event_t event;
        sysEventQueueReceive(audioQueue, &event, 4 * 1000 * 1000);

        FillNotes(audioBuffer, AUDIO_BLOCK_SAMPLES);

        src = audioBuffer;
        dst = audioPortAddr + 2 * block * AUDIO_BLOCK_SAMPLES;
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            *dst++ = *src++ / 32768.0f;
            *dst++ = *src++ / 32768.0f;
        }

        block = (block + 1) & (AUDIO_BLOCK_8 - 1);
    }

    audioPortStop(audioPort);

    audioRemoveNotifyEventQueue(audioQueueKey);
    sysEventQueueDestroy(audioQueue, 0);
    audioPortClose(audioPort);
    audioQuit();

    sysThreadExit(0);
}

void soundInit(void)
{
    audioPortParam portParam;
    audioPortConfig portConfig;

    audioInit();

    memset(&portParam, 0, sizeof(portParam));
    portParam.numChannels = AUDIO_PORT_2CH;
    portParam.numBlocks = AUDIO_BLOCK_8;
    audioPortOpen(&portParam, &audioPort);
    audioGetPortConfig(audioPort, &portConfig);

    audioPortAddr = (float*)portConfig.audioDataStart;

    sysThreadCreate(&audioThreadId, (void*)audioThread, 0, 100, 0x8000, THREAD_JOINABLE, "audioThread");
}

// filesystem
#define MOUNT_DIR   "/dev_hdd0/game/WIPE12052/USRDIR"
#define MAX_FILES   4096
char* gFiles[MAX_FILES];
int32 gFilesCount;

void addDir(char* path)
{
    char* fileName;
    int32 dir;
    sysFSDirent e;
    uint64_t r;

    if (sysFsOpendir(path, &dir) != 0)
    {
        ASSERT(0);
        return;
    }

    int32 pathLen = strlen(path);
    path[pathLen] = '/';

    while (1)
    {
        sysFsReaddir(dir, &e, &r);
        if (r == 0)
            break;

        if (e.d_type == 1) // is directory
        {
            if (e.d_name[0] != '.')
            {
                strcpy(path + 1 + pathLen, e.d_name);
                addDir(path);
            }
        }
        else
        {
            ASSERT(gFilesCount < MAX_FILES);
            if (gFilesCount < MAX_FILES)
            {
                strcpy(path + 1 + pathLen, e.d_name);
                fileName = (char*)malloc(strlen(path) + 1);
                gFiles[gFilesCount++] = strcpy(fileName, path);
            }
        }
    }

    sysFsClosedir(dir);

    path[pathLen] = '\0';
}

void streamInit()
{
    char path[256];
#if 1
    strcpy(path, MOUNT_DIR);
#else
    strcpy(path, SYS_APP_HOME);
#endif
    addDir(path);
}

void streamFree()
{
    int32 i;
    for (i = 0; i < gFilesCount; i++)
    {
        free(gFiles[i]);
    }
}

const char* getFileName(const char* fileName)
{
    int32 i;
    for (i = 0; i < gFilesCount; i++)
    {
        if (!strcasecmp(fileName, gFiles[i]))
        {
            //LOG("open file %s -> %s\n", fileName, gFiles[i]);
            return gFiles[i];
        }
    }
    //LOG("file not found %s\n", fileName);
    return NULL;
}

int osLoadFile(const char* name, void* buffer)
{
    int f = 0;
    sysFSStat stat;
    char path[256];

#if 1
    strcpy(path, MOUNT_DIR);
#else
    strcpy(path, SYS_APP_HOME);
#endif
    strcat(path, "/");
    strcat(path, name);

    name = getFileName(path);
    ASSERT(name);
    if (!name)
    {
        osQuit();
        return 0;
    }

    if (sysFsOpen(name, SYS_O_RDONLY, &f, NULL, 0) != 0)
    {
        osQuit();
        ASSERT(0);
        return 0;
    }

    if (sysFsFstat(f, &stat) != 0)
    {
        osQuit();
        ASSERT(0);
        return 0;
    }

    if (buffer)
    {
        uint64_t bytesRead;
        if ((sysFsRead(f, (void*)buffer, stat.st_size, &bytesRead) != 0) || (bytesRead != stat.st_size))
        {
            osQuit();
            sysFsClose(f);
            ASSERT(0);
            return 0;
        }
    }

    sysFsClose(f);
    return (int32)stat.st_size;
}


void sysutilProc(uint64_t status, uint64_t param, void* userdata)
{
    if (status == SYSUTIL_EXIT_GAME)
    {
        osQuit();
    }
}

//#define DEVELOPER

int main(void)
{
    LOG("F3600 PS3\n");

    sysUtilRegisterCallback(0, sysutilProc, NULL);

    streamInit();
    timerInit();
    inputInit();

    srand((int32)gTimerStart);

    gameInit();
    gFrameIndex = (int32)gcmGetVBlankCount();

    while (!isQuit)
    {
        sysUtilCheckCallback();

        inputUpdate();

    #ifdef DEVELOPER
        if (gPad & IN_SELECT) isQuit = 1;
    #endif

        gLastFrameIndex = gFrameIndex;
        gFrameIndex = (int32)gcmGetVBlankCount();
        gFrameDelta = (gFrameIndex & 1) ? 128 : 0;
        gFrameIndex >>= 1; // to 30 Hz
        gameCtrl();

        renderSwap();
    }

    sysThreadJoin(audioThreadId, NULL);
    gameFree();
    inputFree();
    streamFree();

#ifdef DEVELOPER
    sysProcessExitSpawn2("/dev_hdd0/game/PSL145310/RELOAD.SELF", NULL, NULL, NULL, 0, 1001, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
#endif

    return 0;
}
