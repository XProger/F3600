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

short FrameState=0;

//menu graphic detail stuff - SJR
short ft=0;
char MipMap = 0;

HWND hwnd;


short trackNum = 0;

void oldmain(void);

BOOL      bActive;   // is application active?

HWND hWnd;


int gPad;

int lastKey = 0;

LARGE_INTEGER timerFreq;
LARGE_INTEGER timerStart;

void timerInit()
{
   QueryPerformanceFrequency(&timerFreq);
   QueryPerformanceCounter(&timerStart);
}

long osGetTimeMS()
{
   LARGE_INTEGER count;
   QueryPerformanceCounter(&count);
   return (long)((count.QuadPart - timerStart.QuadPart) * 1000L / timerFreq.QuadPart);
}

void osQuit()
{
   PostQuitMessage(0);
}

void setKey(char ScanCode, char KeyDown)
{
   int p = 0;

   lastKey = KeyDown ? ScanCode : -1;

   switch (ScanCode)
   {
      case 'Z':
         p = PadTopLeft2;
         break;
      case 'X':
         p = PadTopRight2;
         break;
      case 'P':
         p = PadStart;
         break;
      case VK_CONTROL:
         p = PadCross0;
         break;
      case VK_MENU:
         p = PadCircle0;
         break;
      case VK_TAB:
         p = PadTriangle0;
         break;
      case VK_RETURN:
         p = PadSelect;
         break;
      case VK_LEFT:
         p = PadLeft0;
         break;
      case VK_RIGHT:
         p = PadRight0;
         break;
      case VK_UP:
         p = PadUp0;
         break;
      case VK_DOWN:
         p = PadDown0;
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
   srand((int)timerStart.QuadPart);

   renderInit();

   CDStop();
   oldmain();

   renderFree();
   SfxFree();

   DestroyWindow(hwnd);

   return 0;
}

int ProcessMessages(void)
{
   MSG msg;

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