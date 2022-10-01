#include <windows.h>
#include <mmsystem.h>
#include "game.h"

BOOL isActive;
HWND hWnd;

int gPad;
int gPadStickX;
int gPadStickY;

int gLastKey;

int gFrameIndex;
int gLastFrameIndex;

LARGE_INTEGER gTimerFreq;
LARGE_INTEGER gTimerStart;

void timerInit()
{
    QueryPerformanceFrequency(&gTimerFreq);
    QueryPerformanceCounter(&gTimerStart);
}

long osGetSystemTimeMS()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (long)((count.QuadPart - gTimerStart.QuadPart) * 1000L / gTimerFreq.QuadPart);
}

void osQuit()
{
    PostQuitMessage(0);
}

#define INPUT_JOY_COUNT 4

// gamepad
typedef struct _XINPUT_GAMEPAD
{
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
} XINPUT_GAMEPAD;

typedef struct _XINPUT_STATE
{
    DWORD dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE;

typedef struct _XINPUT_VIBRATION
{
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
} XINPUT_VIBRATION;

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD 30

DWORD(WINAPI* _XInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
DWORD(WINAPI* _XInputSetState)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

#define JOY_MIN_UPDATE_FX_TIME 50
#define JOY_RUMBLE_FADE 16

typedef struct JoyDevice
{
    int32 vL, vR; // current value for left/right motor vibration
    int32 oL, oR; // last applied value
    int32 time; // time when we can send vibration update
    int32 mask; // buttons mask
    int32 ready;
    int32 readyLegacy;
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
    JoyDevice* joy = joyDevice + index;

    if (!joy->ready)
        return;

    if (!(joy->vL + joy->vR + joy->oL + joy->oR))
        return;

    if (osGetSystemTimeMS() < joy->time)
        return;

    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = (WORD)(joy->vL << 8);
    vibration.wRightMotorSpeed = (WORD)(joy->vR << 8);
    _XInputSetState(index, &vibration);

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

    HMODULE h = LoadLibrary("xinput1_3.dll");
    if (h == NULL)
    {
        h = LoadLibrary("xinput9_1_0.dll");
    }

    if (!h)
        return;

#define GetProcAddr(lib, x) ((FARPROC)x = GetProcAddress(lib, #x + 1))

    GetProcAddr(h, _XInputGetState);
    GetProcAddr(h, _XInputSetState);

    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        JOYINFOEX info;
        info.dwSize  = sizeof(info);
        info.dwFlags = JOY_RETURNALL;
        joyDevice[i].readyLegacy = (joyGetPosEx(i, &info) == JOYERR_NOERROR);

        if (_XInputGetState)
        {
            XINPUT_STATE state;
            joyDevice[i].ready = (_XInputGetState(i, &state) == ERROR_SUCCESS);
        }
    }
}

void inputFree()
{
    memset(joyDevice, 0, sizeof(joyDevice));
}

void inputReset()
{
    int index;
    for (index = 0; index < INPUT_JOY_COUNT; index++)
    {
        if (!joyDevice[index].ready)
            return;

        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed = 0;
        vibration.wRightMotorSpeed = 0;
        _XInputSetState(index, &vibration);
    }

    gLastKey = -1;
    gPad = 0;
}

enum JoyMode
{
    JOY_MODE_XINPUT,
    JOY_MODE_NEGCON
};

int32 joyMode = JOY_MODE_XINPUT;

void inputUpdate()
{
    if (!isActive)
        return;

    gPadStickX = gPadStickY = 256;

    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        int32 curMask = 0;
        int32 oldMask = joyDevice[i].mask;

        if (joyMode == JOY_MODE_NEGCON)
        {
            JOYCAPS caps;
            JOYINFOEX info;

            if (!joyDevice[i].readyLegacy)
                continue;

            info.dwSize  = sizeof(info);
            info.dwFlags = JOY_RETURNALL;

            if (joyGetPosEx(i, &info) != JOYERR_NOERROR)
                continue;

            joyGetDevCaps(i, &caps, sizeof(caps));

            if (caps.wNumAxes > 2)
            {
                float ax = ((info.dwXpos - caps.wXmin) / (float)(caps.wXmax - caps.wXmin)) * 2.0f - 1.0f;
                float ay = ((info.dwYpos - caps.wYmin) / (float)(caps.wYmax - caps.wYmin)) * 2.0f - 1.0f;
                float az = ((info.dwZpos - caps.wZmin) / (float)(caps.wZmax - caps.wZmin)) * 2.0f - 1.0f;

            // twist
                if (fabsf(ax) > 0.05f)
                {
                    if (ax < 0.0f)
                    {
                        curMask |= IN_LEFT;
                        gPadStickX = (int32)(-ax * 256);
                    }
                    else
                    {
                        curMask |= IN_RIGHT;
                        gPadStickX = (int32)(ax * 256);
                    }
                }

            // accelerate / fire
                if (fabsf(ay) > 0.01f)
                {
                    if (ay < 0.0f)
                    {
                        curMask |= IN_A;
                    }
                    else
                    {
                        curMask |= IN_B;
                    }
                }

            // left air brake (analog)
                if (az < 0.0f)
                {
                    curMask |= IN_LT;
                }
            }

            uint32 p = ((caps.wCaps & JOYCAPS_HASPOV) && (info.dwPOV != JOY_POVCENTERED)) ? (1 + info.dwPOV / 4500) : 0;

            if (p == 8 || p == 1 || p == 2) curMask |= IN_UP;
            if (p == 6 || p == 7 || p == 8) curMask |= IN_LEFT;

            if (!(curMask & (IN_LEFT | IN_RIGHT))) // if no twist - use d-pad
            {
                if (p == 2 || p == 3 || p == 4) curMask |= IN_RIGHT;
                if (p == 4 || p == 5 || p == 6) curMask |= IN_DOWN;
            }

            if (info.dwButtons & (1 << 0)) curMask |= IN_B;
            if (info.dwButtons & (1 << 1)) curMask |= IN_A;

            if (info.dwButtons & (1 <<  9)) curMask |= IN_START;
            if (info.dwButtons & (1 <<  7)) curMask |= IN_RT;
        }
        else
        {
            XINPUT_STATE state;

            if (!joyDevice[i].ready)
                continue;

            joyRumble(i);

            if (_XInputGetState(i, &state) != ERROR_SUCCESS)
            {
                inputFree();
                inputInit();
                break;
            }

            static const int buttons[] = { IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT, IN_START, IN_SELECT, IN_L, IN_R, IN_LB, IN_RB, 0, 0, IN_A, IN_B, IN_X, IN_Y, IN_LT, IN_RT };
            for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
            {
                if (state.Gamepad.wButtons & (1 << i))
                {
                    curMask |= buttons[i];
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
                gPadStickX = state.Gamepad.sThumbLX >> 7;
            }
        }

        if (gPadStickX > 256)
            gPadStickX = 256;
        if (gPadStickY > 256)
            gPadStickY = 256;

        for (int i = 0; i < 18; i++)
        {
            int wasDown = (oldMask & (1 << i)) != 0;
            int isDown = (curMask & (1 << i)) != 0;

            if (isDown == wasDown)
                continue;

            if (isDown && !wasDown)
            {
                gPad |= (1 << i);
            }
            else
            {
                gPad &= ~(1 << i);
            }
        }

        joyDevice[i].mask = curMask;
        break;
    }
}

void setKey(uint8 code, uint8 KeyDown)
{
    int p = 0;

    gLastKey = KeyDown ? code : -1;

    switch (code)
    {
        case 'Z':
            p = IN_LT;
            break;
        case 'X':
            p = IN_RT;
            break;
        case 'A':
            p = IN_LB;
            break;
        case 'S':
            p = IN_RB;
            break;
        case VK_RETURN:
        case VK_CONTROL:
            p = IN_A;
            break;
        case VK_MENU:
            p = IN_B;
            break;
        case VK_TAB:
            p = IN_Y;
            break;
        case VK_ESCAPE:
            p = IN_START;
            break;
        case VK_LEFT:
            p = IN_LEFT;
            break;
        case VK_RIGHT:
            p = IN_RIGHT;
            break;
        case VK_UP:
            p = IN_UP;
            break;
        case VK_DOWN:
            p = IN_DOWN;
            break;
    }

    if (KeyDown)
    {
        gPad |= p;
    }
    else
    {
        gPad &= ~p;
    }
}

unsigned short GetKeyNoBlock(void)
{
    return gLastKey;
}

// sound
int16 soundBuffer[2 * 2 * SND_SAMPLES];
HWAVEOUT waveOut;
WAVEHDR waveBuf[2];
uint32 curSoundBuffer = 0;

const WAVEFORMATEX waveFmt = {
    WAVE_FORMAT_PCM,
    SND_CHANNELS,
    SND_OUTPUT_FREQ,
    sizeof(soundBuffer[0]) * SND_OUTPUT_FREQ * SND_CHANNELS,
    sizeof(soundBuffer[0]) * SND_CHANNELS,
    sizeof(soundBuffer[0]) * 8,
    sizeof(waveFmt)
};

void soundInit()
{
    if (waveOutOpen(&waveOut, WAVE_MAPPER, &waveFmt, (INT_PTR)hWnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR)
        return;

    for (int i = 0; i < 2; i++)
    {
        WAVEHDR *waveHdr = waveBuf + i;
        waveHdr->dwBufferLength = SND_SAMPLES * SND_CHANNELS * sizeof(int16);
        waveHdr->lpData = (LPSTR)(soundBuffer + i * SND_SAMPLES * SND_CHANNELS);
        waveOutPrepareHeader(waveOut, waveHdr, sizeof(WAVEHDR));
        waveOutWrite(waveOut, waveHdr, sizeof(WAVEHDR));
    }
}

void soundFill()
{
    WAVEHDR *waveHdr = waveBuf + curSoundBuffer;
    waveOutUnprepareHeader(waveOut, waveHdr, sizeof(WAVEHDR));
    FillNotes((int16*)waveHdr->lpData, SND_SAMPLES);
    waveOutPrepareHeader(waveOut, waveHdr, sizeof(WAVEHDR));
    waveOutWrite(waveOut, waveHdr, sizeof(WAVEHDR));
    curSoundBuffer ^= 1;
}

// filesystem
long osLoadFile(const char* name, char* buffer)
{
    FILE* f;
    long size;

    ASSERT(buffer);
    f = fopen(name, "rb");
    if (!f)
    {
        ASSERT(0);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);

    printf("load file %s\n", name);
    fseek(f, 0, SEEK_SET);
    if (fread(buffer, 1, size, f) != size)
    {
        fclose(f);
        ASSERT(0);
        return 0;
    }

    fclose(f);
    return size;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_ACTIVATE:
            isActive = (wParam != WA_INACTIVE);
            if (!isActive)
            {
                inputReset();
            }
            break;

        case WM_DESTROY:
            osQuit();
            break;

        case WM_SIZE:
            renderResize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            if (msg == WM_SYSKEYDOWN)
            {
                // Alt + Enter - switch to fullscreen or window
                if (wParam == VK_RETURN)
                {
                    static WINDOWPLACEMENT pLast;
                    DWORD style = GetWindowLong(hWnd, GWL_STYLE);
                    if (style & WS_OVERLAPPEDWINDOW)
                    {
                        MONITORINFO mInfo;
                        mInfo.cbSize = sizeof(mInfo);
                        if (GetWindowPlacement(hWnd, &pLast) && GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mInfo))
                        {
                            const RECT* r = &mInfo.rcMonitor;
                            SetWindowLong(hWnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                            MoveWindow(hWnd, r->left, r->top, r->right - r->left, r->bottom - r->top, FALSE);
                            ShowCursor(FALSE);
                        }
                    }
                    else
                    {
                        SetWindowLong(hWnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
                        SetWindowPlacement(hWnd, &pLast);
                        ShowCursor(TRUE);
                    }
                    break;
                }

                // Alt + F4 - close application
                if (wParam == VK_F4)
                {
                    osQuit();
                    break;
                }
            }

            setKey(wParam, (msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN));
            break;

        case WM_DEVICECHANGE:
            inputFree();
            inputInit();
            return 1;

        case MM_WOM_DONE:
            soundFill();
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

#define WND_WIDTH 1280
#define WND_HEIGHT 720

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
    AllocConsole();
    FILE* fp = NULL;
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

    gDRAM = pDRAM = (char*)malloc(DRAM_SIZE);
    gVRAM = pVRAM = (char*)malloc(VRAM_SIZE);

    MSG msg;
    RECT r = { 0, 0, WND_WIDTH, WND_HEIGHT };

    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    int wx = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
    int wy = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;

    hWnd = CreateWindow("static", "wipEout xport", WS_OVERLAPPEDWINDOW, wx + r.left, wy + r.top, r.right - r.left, r.bottom - r.top, 0, 0, hInstance, 0);

    SetWindowLong(hWnd, GWL_WNDPROC, (LONG)&WndProc);
    ShowWindow(hWnd, SW_SHOWDEFAULT);

    timerInit();
    inputInit();
    soundInit();

    srand((int)gTimerStart.QuadPart);

    renderInit();

    gameInit();

    do {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            gLastFrameIndex = gFrameIndex;
            gFrameIndex = osGetSystemTimeMS() * 3 / 50; // * 60 / 1000

            inputUpdate();

            gameCtrl();

            renderSwap();
        }
    } while (msg.message != WM_QUIT);

    inputFree();
    renderFree();
    SfxFree();

    DestroyWindow(hWnd);

    free(gDRAM);
    free(gVRAM);

    return 0;
}
