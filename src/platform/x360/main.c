#include <xtl.h>
#include <xaudio2.h>
#include "game.h"

// timing
LARGE_INTEGER gTimerFreq;
LARGE_INTEGER gTimerStart;

void timerInit()
{
    QueryPerformanceFrequency(&gTimerFreq);
    QueryPerformanceCounter(&gTimerStart);
}

int32 osGetSystemTimeMS(void)
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (int32)((count.QuadPart - gTimerStart.QuadPart) * 1000L / gTimerFreq.QuadPart);
}

// system
int32 isQuit;

void osQuit()
{
    isQuit = 1;
}

#define GetProcAddr(lib, x) ((FARPROC)x = (lib ? GetProcAddress(lib, #x + 1) : NULL))

// gamepad
#define INPUT_JOY_COUNT 4

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD 30

#define JOY_MIN_UPDATE_FX_TIME 50
#define JOY_RUMBLE_FADE 16

typedef struct JoyDevice
{
    int32 vL, vR; // current value for left/right motor vibration
    int32 oL, oR; // last applied value
    int32 time; // time when we can send vibration update
    int32 mask; // buttons mask
    int32 ready;
} JoyDevice;

JoyDevice joyDevice[INPUT_JOY_COUNT];

int32 osJoyReady(int index)
{
    return joyDevice[index].ready;
}

void osJoyVibrate(int index, int L, int R)
{
    joyDevice[index].vL = L;
    joyDevice[index].vR = R;
}

void joyRumble(int index)
{
    XINPUT_VIBRATION vibration;
    JoyDevice* joy = joyDevice + index;

    if (!joy->ready)
        return;

    if (!(joy->vL + joy->vR + joy->oL + joy->oR))
        return;

    if (osGetSystemTimeMS() < joy->time)
        return;

    vibration.wLeftMotorSpeed = (WORD)(joy->vL << 8);
    vibration.wRightMotorSpeed = (WORD)(joy->vR << 8);
    XInputSetState(index, &vibration);

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

void inputInit()
{
    memset(joyDevice, 0, sizeof(joyDevice));
}

void inputFree()
{
}

void inputReset()
{
    XINPUT_VIBRATION vibration;
    int32 index;

    for (index = 0; index < INPUT_JOY_COUNT; index++)
    {
        if (!joyDevice[index].ready)
            break;
        vibration.wLeftMotorSpeed = 0;
        vibration.wRightMotorSpeed = 0;
        XInputSetState(index, &vibration);
    }

    gLastKey = -1;
    gPad = 0;
}

int32 joyIndex = 0;

void inputUpdate()
{
    static const int buttons[] = { IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT, IN_START, IN_SELECT, IN_L, IN_R, IN_LB, IN_RB, 0, 0, IN_A, IN_B, IN_X, IN_Y, IN_LT, IN_RT };

    int32 i, btn;

    gPadStickX = gPadStickY = 256;

    i = joyIndex;
    //for (i = 0; i < INPUT_JOY_COUNT; i++)
    {
        int32 curMask = 0;
        int32 oldMask = joyDevice[i].mask;

        XINPUT_STATE state;

        if (XInputGetState(i, &state) != ERROR_SUCCESS)
        {
            inputFree();
            inputInit();
            return;
        }

        joyRumble(i);

        for (btn = 0; btn < sizeof(buttons) / sizeof(buttons[0]); btn++)
        {
            if (state.Gamepad.wButtons & (1 << btn))
            {
                curMask |= buttons[btn];
            }
        }

        if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            curMask |= IN_LT;

        if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            curMask |= IN_RT;

        if (state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            curMask |= IN_UP;
            gPadStickY = state.Gamepad.sThumbLY >> 6;
        }

        if (state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            curMask |= IN_DOWN;
            gPadStickY = -state.Gamepad.sThumbLY >> 6;
        }

        if (state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            curMask |= IN_LEFT;
            gPadStickX = -state.Gamepad.sThumbLX >> 7;
        }

        if (state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            curMask |= IN_RIGHT;
            gPadStickX = (state.Gamepad.sThumbLX + 1) >> 7;
        }

        if (gPadStickX > 256)
            gPadStickX = 256;
        if (gPadStickY > 256)
            gPadStickY = 256;

        for (btn = 0; btn < 18; btn++)
        {
            int32 wasDown = (oldMask & (1 << btn)) != 0;
            int32 isDown = (curMask & (1 << btn)) != 0;

            if (isDown == wasDown)
                continue;

            if (isDown && !wasDown)
            {
                gPad |= (1 << btn);
            }
            else
            {
                gPad &= ~(1 << btn);
            }
        }

        joyDevice[i].mask = curMask;
        joyDevice[i].ready = 1;
    }
}

// sound
#define SND_SIZE        SND_SAMPLES*2*sizeof(int16)
#define SND_MAX_BUFFERS 2

uint8 soundBuffer[SND_SIZE * SND_MAX_BUFFERS];
HANDLE soundThread;
HANDLE soundEvent;

static void WINAPI OnVoiceProcessingPassStart(IXAudio2VoiceCallback *This, UINT32 BytesRequired) {}
static void WINAPI OnVoiceProcessingPassEnd(IXAudio2VoiceCallback *This) {}
static void WINAPI OnStreamEnd(IXAudio2VoiceCallback *This) {}
static void WINAPI OnBufferStart(IXAudio2VoiceCallback *This, void *pBufferContext) {}
static void WINAPI OnLoopEnd(IXAudio2VoiceCallback *This, void *pBufferContext) {}
static void WINAPI OnVoiceError(IXAudio2VoiceCallback *This, void *pBuffercontext, HRESULT Error) {}
static void WINAPI OnBufferEnd(IXAudio2VoiceCallback *This, void *pBufferContext)
{
    SetEvent(soundEvent);
}

static IXAudio2VoiceCallbackVtbl vcb_vtbl = {
    OnVoiceProcessingPassStart,
    OnVoiceProcessingPassEnd,
    OnStreamEnd,
    OnBufferStart,
    OnBufferEnd,
    OnLoopEnd,
    OnVoiceError
};

static IXAudio2VoiceCallback vcb = { &vcb_vtbl };

DWORD WINAPI soundFill(LPVOID lpParam)
{
    const WAVEFORMATEX waveFmt = { WAVE_FORMAT_PCM, 2, 44100, 44100 * 4, 4, 16, sizeof(waveFmt) };

    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* masteringVoice;
    IXAudio2SourceVoice* sourceVoice;
    XAUDIO2_BUFFER buffer;
    uint32 bufferIndex = 0;

    soundEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    memset(&buffer, 0, sizeof(buffer));

    if (FAILED(XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        ASSERT(0);
        return 0;
    }

    if (FAILED(IXAudio2_CreateMasteringVoice(pXAudio2, &masteringVoice, 0, 0, 0, 0, NULL)))
    {
        ASSERT(0);
        return 0;
    }

    if (FAILED(IXAudio2_CreateSourceVoice(pXAudio2, &sourceVoice, &waveFmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &vcb, NULL, NULL)))
    {
        ASSERT(0);
        return 0;
    }

    if (FAILED(IXAudio2SourceVoice_Start(sourceVoice, 0, XAUDIO2_COMMIT_NOW)))
    {
        ASSERT(0);
        return 0;
    }

    while (!isQuit)
    {
        XAUDIO2_VOICE_STATE state;
        IXAudio2SourceVoice_GetState(sourceVoice, &state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

        if (state.BuffersQueued < SND_MAX_BUFFERS)
        {
            buffer.AudioBytes = SND_SIZE;
            buffer.pAudioData = soundBuffer + SND_SIZE * bufferIndex;

            FillNotes((int16*)buffer.pAudioData, buffer.AudioBytes / 4);

            bufferIndex = (bufferIndex + 1) % SND_MAX_BUFFERS;

            IXAudio2SourceVoice_SubmitSourceBuffer(sourceVoice, &buffer, NULL);
        } else {
            WaitForSingleObject(soundEvent, INFINITE);
        }
    }

    IXAudio2SourceVoice_Stop(sourceVoice, 0, 0);
    IXAudio2SourceVoice_DestroyVoice(sourceVoice);
    IXAudio2MasteringVoice_DestroyVoice(masteringVoice);

    return 0;
}

void soundInit()
{
    soundThread = CreateThread(NULL, 0, soundFill, NULL, 0, NULL);
}

// filesystem
int32 osLoadFile(const char* name, void* buffer)
{
    FILE* f;
    int32 size;
    char path[256];

    { // slash -> backslash
        const char* src = name;
        char* dst = path + 6;
        char c;

        strcpy(path, "game:\\");
        while ((c = *src++) != 0)
        {
            if (c == '/')
            {
                c = '\\';
            }
            *dst++ = c;
        }
        *dst = 0;
    }

    f = fopen(path, "rb");
    if (!f)
    {
        ASSERT(0);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);

    if (buffer)
    {
        fseek(f, 0, SEEK_SET);
        if (fread(buffer, 1, size, f) != size)
        {
            fclose(f);
            ASSERT(0);
            return 0;
        }
    }

    fclose(f);
    return size;
}

#define DEVELOPER

void main()
{
    LOG("F3600 X360\n");

    gDRAM = pDRAM = (uint8*)malloc(DRAM_SIZE);
    gVRAM = pVRAM = (uint8*)malloc(VRAM_SIZE);

    timerInit();
    inputInit();

    srand((int32)gTimerStart.QuadPart);

    gameInit();

    while (!isQuit)
    {
        int32 time = osGetSystemTimeMS();
        gLastFrameIndex = gFrameIndex;
        gFrameIndex = time * 30 / 1000;
        gFrameDelta = (time - gFrameIndex * 1000 / 30) * 256 * 30 / 1000;

        inputUpdate();

    #ifdef DEVELOPER
        if (gPad & IN_SELECT) osQuit();
    #endif

        gameCtrl();

        renderSwap();
    };

    SetEvent(soundEvent);
    WaitForSingleObject(soundThread, INFINITE);
    CloseHandle(soundEvent);

    inputFree();
    gameFree();

    free(gDRAM);
    free(gVRAM);
}
