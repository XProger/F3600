#define STRICT
#include <windows.h>
#include <time.h>
#include <stdio.h>

#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "utypes.h"
#include "sfx.h"

#include <math.h>
#include "decl.h"
#include "malloc.h"
#include "render.h"
#include "input.h"

short FrameState = 0;
short trackNum = 0;

void oldmain(void);

BOOL bActive;
HWND hWnd;

int gPad;
int gPadStickX;
int gPadStickY;

int lastKey = 0;

LARGE_INTEGER gTimerFreq;
LARGE_INTEGER gTimerStart;

void timerInit()
{
   QueryPerformanceFrequency(&gTimerFreq);
   QueryPerformanceCounter(&gTimerStart);
}

long osGetTimeMS()
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
typedef struct _XINPUT_GAMEPAD {
   WORD                                wButtons;
   BYTE                                bLeftTrigger;
   BYTE                                bRightTrigger;
   SHORT                               sThumbLX;
   SHORT                               sThumbLY;
   SHORT                               sThumbRX;
   SHORT                               sThumbRY;
} XINPUT_GAMEPAD;

typedef struct _XINPUT_STATE {
   DWORD                               dwPacketNumber;
   XINPUT_GAMEPAD                      Gamepad;
} XINPUT_STATE;

typedef struct _XINPUT_VIBRATION {
   WORD                                wLeftMotorSpeed;
   WORD                                wRightMotorSpeed;
} XINPUT_VIBRATION;

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30

DWORD(WINAPI* _XInputGetState) (DWORD dwUserIndex, XINPUT_STATE* pState);
DWORD(WINAPI* _XInputSetState) (DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

#define JOY_MIN_UPDATE_FX_TIME   50
#define JOY_RUMBLE_FADE          16

typedef struct JoyDevice {
   int32 vL, vR; // current value for left/right motor vibration
   int32 oL, oR; // last applied value
   int32 time;   // time when we can send vibration update
   int32 mask;   // buttons mask
   int32 ready;
} JoyDevice;

JoyDevice joyDevice[INPUT_JOY_COUNT];

int32 osJoyReady(int index) {
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

   if (osGetTimeMS() < joy->time)
      return;

   XINPUT_VIBRATION vibration;
   vibration.wLeftMotorSpeed = (WORD)(joy->vL << 8);
   vibration.wRightMotorSpeed = (WORD)(joy->vR << 8);
   _XInputSetState(index, &vibration);

   joy->oL = joy->vL;
   joy->oR = joy->vR;
   joy->vL -= JOY_RUMBLE_FADE;
   joy->vR -= JOY_RUMBLE_FADE;
   if (joy->vL < 0) joy->vL = 0;
   if (joy->vR < 0) joy->vR = 0;
   joy->time = osGetTimeMS() + JOY_MIN_UPDATE_FX_TIME;
}

void inputInit() {
   memset(joyDevice, 0, sizeof(joyDevice));

   HMODULE h = LoadLibrary("xinput1_3.dll");
   if (h == NULL) {
      h = LoadLibrary("xinput9_1_0.dll");
   }

   if (!h)
      return;

   #define GetProcAddr(lib, x) (x = GetProcAddress(lib, #x + 1))

   GetProcAddr(h, _XInputGetState);
   GetProcAddr(h, _XInputSetState);

   for (int i = 0; i < INPUT_JOY_COUNT; i++)
   {
      XINPUT_STATE state;
      int res = _XInputGetState(i, &state);
      joyDevice[i].ready = (_XInputGetState(i, &state) == ERROR_SUCCESS);

      if (joyDevice[i].ready)
         printf("Gamepad %d is ready\n", i + 1);
   }
}

void inputFree()
{
   memset(joyDevice, 0, sizeof(joyDevice));
}

void inputUpdate()
{
   if (!_XInputGetState)
      return;

   for (int i = 0; i < INPUT_JOY_COUNT; i++)
   {
      if (!joyDevice[i].ready)
         continue;

      joyRumble(i);

      XINPUT_STATE state;
      if (_XInputGetState(i, &state) != ERROR_SUCCESS)
      {
         inputFree();
         inputInit();
         break;
      }

      static const int buttons[] = { IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT, IN_START, IN_SELECT, IN_L, IN_R, IN_LB, IN_RB, 0, 0, IN_A, IN_B, IN_X, IN_Y, IN_LT, IN_RT };

      int32 curMask = state.Gamepad.wButtons;
      int32 oldMask = joyDevice[i].mask;

      gPadStickX = gPadStickY = 256;

      if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
         curMask |= (1 << 16); // IN_LT

      if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
         curMask |= (1 << 17); // IN_RT

      if (state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
      {
         curMask |= (1 << 0); // IN_UP
         gPadStickY = state.Gamepad.sThumbLY >> 6;
      }

      if (state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
      {
         curMask |= (1 << 1); // IN_DOWN
         gPadStickY = -state.Gamepad.sThumbLY >> 6;
      }

      if (state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
      {
         curMask |= (1 << 2); // IN_LEFT
         gPadStickX = -state.Gamepad.sThumbLX >> 7;
      }

      if (state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
      {
         curMask |= (1 << 3); // IN_RIGHT
         gPadStickX = state.Gamepad.sThumbLX >> 7;
      }

      if (gPadStickX > 256) gPadStickX = 256;
      if (gPadStickY > 256) gPadStickY = 256;

      for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
      {
         int wasDown = (oldMask & (1 << i)) != 0;
         int isDown = (curMask & (1 << i)) != 0;

         if (isDown == wasDown)
            continue;

         if (isDown && !wasDown) {
            gPad |= buttons[i];
         } else {
            gPad &= ~buttons[i];
         }
      }

      joyDevice[i].mask = curMask;
   }
}

void setKey(char ScanCode, char KeyDown)
{
   int p = 0;

   lastKey = KeyDown ? ScanCode : -1;

   switch (ScanCode)
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
         p = IN_X;
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

   if (KeyDown) {
      gPad |= p;
   } else {
      gPad &= ~p;
   }
}

unsigned short GetKeyNoBlock()
{
   return lastKey;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg)
   {
      case WM_ACTIVATE:
         bActive = TRUE;// (wParam == WA_ACTIVE);
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
         setKey((char)wParam, (msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN));
         break;

      case WM_DEVICECHANGE:
         inputFree();
         inputInit();
         return 1;

      default:
         return DefWindowProc(hWnd, msg, wParam, lParam);
   }

   return 0;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   RECT r = { 0, 0, 640, 480 };

   AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
   int wx = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
   int wy = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;

   hWnd = CreateWindow("static", "wipEout xport", WS_OVERLAPPEDWINDOW, wx + r.left, wy + r.top, r.right - r.left, r.bottom - r.top, 0, 0, hInstance, 0);

   SetWindowLong(hWnd, GWL_WNDPROC, (LONG)&WndProc);
   ShowWindow(hWnd, SW_SHOWDEFAULT);

   timerInit();
   inputInit();

   srand((int)gTimerStart.QuadPart);

   renderInit();

   CDStop();
   oldmain();

   inputFree();
   renderFree();
   SfxFree();

   DestroyWindow(hWnd);

   return 0;
}

int ProcessMessages(void)
{
   MSG msg;

   inputUpdate();

   if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
         /*TODO proper game loop*/
         exit(0);
         return 0;
      }
   }

   return 1;
}

// dummy functions
uint16 SfxInit(uint16 wCard, uint16 wMixType, uint16 wSampRate) { return 0; }
void SfxFree(void) {}

uint16 SfxInitReverb(uint16 wDelay) { return 0; }
void SfxReverbOn(void) {}
void SfxReverbOff(void) {}
void SfxSetReverbDepth(uint8 bDepth) {}
void SfxUnInitReverb(void) {}

uint16 SfxSetMaxEffects(uint16 wNum) { return 0; }

uint16 SfxGetFxBank(char* filename) { return 0; }
int16 SfxOpenFxBank(int16* pwFirstId) { return 0; }

void SfxSetVolume(uint8 bVolume) {}
void SfxSetCdVolume(uint8 bVolume) {}
void SfxPlay(int16 wSampleId, uint8 bVolume, uint8 bLRPan, uint8 bFBPan, uint16 wPitch, uint8 bFlags) {}
int16 SfxPlayID(int16 wSampleId, uint8 bVolume, uint8 bLRPan, uint8 bFBPan, uint16 wPitch, uint8 bFlags) { return 0; }

void SfxStop(int16 wPlayId) {}
void SfxStopAll() {}
void SfxEffectVol(int16 wPlayId, uint8 bVolume) {}
void SfxEffectPitch(int16 wPlayId, uint16 wPitch) {}
void SfxEffectPan(int16 wPlayId, uint16 bPan) {}

void SfxOn(void) {}
void SfxOff(void) {}
void SfxDoSfx(void) {}
void SfxUnInit(uint8 bFlag) {}

// cd
int CdControl(unsigned char Cmd, struct CdlPos* parm1, unsigned char* parm2) {}
void CDRestart() {}
void CDPlay(short int CurrTrackNo) {}
void CDStop() {}
void CdSetVol(long newVol) {}

short starttrack;
short endtrack;
short int CurrTrkNo;