#define NAME "WIPEOUT"
#define TITLE "WIPEOUT '98"
#define DBG          0

#ifndef GL_UNSIGNED_SHORT_5_6_5
   #define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif

#ifndef GL_UNSIGNED_SHORT_5_5_5_1
   #define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#endif

#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
   #define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#endif

#ifndef GL_CLAMP_TO_EDGE
   #define GL_CLAMP_TO_EDGE 0x812F
#endif

#define STRICT
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <gl/GL.h>
#include "rgraphics.h"
#include "textures.h"

#include <winuser.h>
#include <mmsystem.h>
#include <direct.h>
#include <vfw.h>
#include <mciavi.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "resource.h"
#include "winh.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "timc.h"

#include <math.h>
#include "decl.h"
#include "malloc.h"

#define IDI_WIPEOUT 101

#define  SCALEIT(x) ((x)*upres)
#define  SCALEZ(z) ((z)*zscale)

#define SMALL 0
#define LOW 1
#define MEDIUM 2
#define HIGH 3

extern TextureFormats availableTextures;

//#define  SCALEIT(x) (x)

//-----------------------------------------------------------------------------
// Global variables for using the D3D sample framework class
//-----------------------------------------------------------------------------
BOOL           g_bActive           = FALSE;
BOOL           g_bReady            = FALSE;
BOOL           g_bExitting         = FALSE;
extern BOOL           g_bAppUseFullScreen;
BOOL           g_bShowStats        = TRUE;
extern BOOL    g_bAppUseZBuffer;
extern BOOL    g_bAppUseBackBuffer;

extern LPDIRECTDRAW4          g_pDD4;
extern LPDIRECTDRAWSURFACE4         g_pddsPrimary;
extern LPDIRECTDRAWSURFACE4         g_pddsBackBuffer;
extern LPDIRECT3DDEVICE3         g_pd3dDevice;

typedef struct
{
   long        id;
   long        flags;
   long        bnum;
   short       textureX, textureY;
   short       textureH, textureW;
} TimData;

extern DynamicHeap *heap;
extern char winLoseIn;
extern int inmenutest;

#define  BLACK_NOT_DRAWN   1
#define  BLACK1_TRANSL  2
#define  BLACK2_TRANSL  3
#define  BLACK3_TRANSL  4
#define  BLACK4_TRANSL  5
#define  BLACK5_TRANSL  6
#define  BLACK6_TRANSL  7
#define  WHITE1_TRANSL  8
#define  WHITE2_TRANSL  9
#define  WHITE3_TRANSL  10
#define  WHITE4_TRANSL  11
#define  WHITE5_TRANSL  12
#define  WHITE6_TRANSL  13
#define  WHITE7_TRANSL  14
#define  WHITE8_TRANSL  15
#define  RED_TRANSL     16
#define  RED_LIGHT_FADE    16
#define  BRIGHT_RED_TRANSL 17
#define  YELLOW_TRANSL  18
#define  YELLOW_WHITE_TRANSL  19
#define  BLUE_TRANSL    20
#define  PURPLE_TRANSL  21
#define  UNUSED1_TRANSL 22
#define  UNUSED2_TRANSL 23
#define  UNUSED3_TRANSL 24
#define  UNUSED4_TRANSL 25
#define ATTRACT 1

#define MCI_MCIAVI_PLAY_FULLSCREEN 0x02000000L
#define DIVIS 0.00195312

#define TEXTURESJR 2621440

typedef struct {
   short xsize,ysize;
   float mult;
}RESOLUTION;

BOOL backFromDead = FALSE;

RESOLUTION reses[]={
   {320,240,1.0f},
   {400,300,1.25f},
   {512,384,1.6f},
   {640,480,2.0f}
};

short screenres=3;
float upres=2.0f;
float zscale=1.0/65536.0f;//1.0/32768.0f;

int menuTexturesIn = 0;
int Keydelay,Keyrepeat,sSaver;
short dodump = 0,FrameState=0;
short PageXOff,PageYOff;

extern long       ctrlNeedEndText;
extern long       ctrlNeedWinLose;
extern long       ctrlNeedBackPoly;
extern long       ctrlNeedRaceInfo;
extern long       ctrlNeedDeSelectScreen;
extern long       ctrlNeedNewHighScores;
extern long       ctrlNeedRacePoints;
extern long       ctrlNeedChampTable;
extern long       ctrlNeedAllVenomComplete;
extern long       ctrlNeedVenomComplete;
extern long       ctrlNeedContinueGameConfirm;
extern long       ctrlNeedEndTextVar;
extern long       ctrlNeedWinLoseVar;
extern long       ctrlNeedInGameReset;
extern long       ctrlNeedInSingleGameConfirm;
extern long       ctrlNeedInGameConfirm;
extern long       ctrlNeedTargetIcon;
extern long       ctrlCDPaused;
extern long       ctrlfirsttime;
extern long ctrlmode;
extern char       ctrlmachinePaused;
extern int           ctrlcdPause;
extern   short   startIceText;
extern   short   startFireText;

typedef struct {
   GLuint Texture;
} TEXTURE;

TEXTURE gTex[500]={0};

char JJSTextureTable[]={0,1,2,3,4,0,1,2,3,4,5,6,7,8,9,10,16,17,18,19,20,11,20,12,13,21,14,15,
         16,17,18,19};
char JJSTextureMenuTable[]={0,1,2,3,4,0,1,2,10,4,5,6,7,8,9,3,16,17,18,19,20,11,20,12,13,21,14,
         15,16,17,18,19};

char lastfile[256];

typedef struct
{
   unsigned long   tag;
   unsigned char   r0, g0, b0, code;
   short    x0, y0;
   short    x1, y1;
   short    x2, y2;
   short    x3, y3;
}         POLY_F4;   /* Flat Quadrangle */


typedef struct
{
   unsigned long   tag;
   unsigned char   r0, g0, b0, code;
   short    x0, y0, z0, pad4;
   unsigned char   u0, v0;
   unsigned short  clut;
   short    x1, y1, z1, pad5;
   unsigned char   u1, v1;
   unsigned short  tpage;
   short    x2, y2, z2, pad6;
   unsigned char   u2, v2;
   unsigned short  pad1;
   short    x3, y3, z3, pad7;
   unsigned char   u3, v3;
   unsigned short  pad2;
}         POLY_FT4;       /* Flat Textured Quadrangle */

typedef struct {
   unsigned long  tag;
   unsigned char  r0, g0, b0, code;
   short x0,   y0;
   unsigned char  r1, g1, b1, pad1;
   short x1,   y1;
   unsigned char  r2, g2, b2, pad2;
   short x2,   y2;
} POLY_G3;           /* Gouraud Triangle */

typedef struct {
   unsigned long  tag;
   unsigned char  r0, g0, b0, code;
   short x0,   y0;
   unsigned char  r1, g1, b1, pad1;
   short x1,   y1;
   unsigned char  r2, g2, b2, pad2;
   short x2,   y2;
   unsigned char  r3, g3, b3, pad3;
   short x3,   y3;
} POLY_G4;           /* Gouraud Quadrangle */

typedef struct {
   unsigned long  tag;
   unsigned char  r0, g0, b0, code;
   short x0,   y0, z0, pad4;
   unsigned char  u0, v0;  unsigned short clut;
   unsigned char  r1, g1, b1, p1;
   short x1,   y1, z1, pad5;
   unsigned char  u1, v1;  unsigned short tpage;
   unsigned char  r2, g2, b2, p2;
   short x2,   y2, z2, pad6;
   unsigned char  u2, v2;  unsigned short pad2;
} POLY_GT3;          /* Gouraud Textured Triangle */

typedef struct {
   unsigned long  tag;
   unsigned char  r0, g0, b0, code;
   short x0,   y0, z0, pad4;
   unsigned char  u0, v0;  unsigned short clut;
   unsigned char  r1, g1, b1, p1;
   short x1,   y1, z1, pad5;
   unsigned char  u1, v1;  unsigned short tpage;
   unsigned char  r2, g2, b2, p2;
   short x2,   y2, z2, pad6;
   unsigned char  u2, v2;  unsigned short pad2;
   unsigned char  r3, g3, b3, p3;
   short x3,   y3, z3, pad7;
   unsigned char  u3, v3;  unsigned short pad3;
} POLY_GT4;          /* Gouraud Textured Quadrangle */

typedef struct {
   unsigned long  tag;
   unsigned char  r0, g0, b0, code;
   short x0, y0, z0, pad4;
   unsigned char  u0, v0;
   unsigned short clut;
   short x1,   y1, z1, pad5;
   unsigned char  u1, v1;
   unsigned short tpage;
   short x2,   y2, z2, pad6;
   unsigned char  u2, v2;
   unsigned short pad1;
} POLY_FT3;          /* Flat Textured Triangle */

typedef struct {
   unsigned long  tag;
   unsigned char  r0, g0, b0, code;
   short x0,   y0;
   unsigned char  u0, v0;  unsigned short clut;
   short w, h;
} SPRT;              /* free size Sprite */

typedef struct {
   unsigned long  tag;
   unsigned char  r0, g0, b0, code;
   short x0,   y0;
   unsigned char  u0, v0;  unsigned short clut;
} SPRT_16;           /* 8x8 Sprite */

typedef SPRT_16 SPRT_8;
char      gszErrMsg[64];
BOOL      gbCIFInit;

static BOOL FirstActivate = TRUE;


//menu graphic detail stuff - SJR
short ft=0;
char MipMap = 0;

short texpage=-1;
short tester=0;
typedef struct {
   unsigned short blue:5;
   unsigned short green:5;
   unsigned short red:5;
} BIT15;

BOOL bTexEn=FALSE;

BOOL trans = FALSE;
BOOL MPEGplaying;
HWND g_hwndMCIWnd;
DWORD dwMCIWndStyle;
HINSTANCE hinst;
HWND      hwnd;
char globalPilot[255]={0};

extern short int CurrTrkNo;
short randlist[2000];
short *randnum;
char *TextureBuffer;
short ctr;
short trackNum = 0;
char *SaveDirBase;
char *DataDirBase;
char *MovieDirBase;
extern UINT wDeviceID;
extern char ctrlshowMenu;
extern long ctrlNeedInGameReset;
short WinPaused = 0;
extern int ctrlchoice;
extern long screenZ;

int dbg_G3, dbg_G4, dbg_F3, dbg_F4, dbg_GT3, dbg_GT4, dbg_FT3, dbg_FT4;

struct sMovie
{
   char szMoviePathName[MAX_PATH];
   int width;
   int height;
   int fullScreen;
};

BOOL shellMoviePlayingG = FALSE;
BOOL shellMovieFinishedG = TRUE;
UINT shellMovieDeviceIDG;

//SJR

float joyModXVal;
float joyModYVal;
extern short JoyCentreX, JoyCentreY;
extern short   left_scale, right_scale;
extern float   left_scale_factor, right_scale_factor;
extern unsigned short JoyXVal, JoyYVal;
short JoyButton1, JoyButton2;

HANDLE gInstance;

#define  HIGH_SENS   8
#define  MEDIUM_SENS 16
#define  LOW_SENS 32

/* Type/Structure Definitions. */
typedef struct {
   unsigned short int   jX;
   unsigned short int   jY;
   unsigned short int   jDir;
   unsigned short int   jButton1;
   unsigned short int   jButton2;
} JOYSTICK;

typedef struct {
   unsigned short int   mX;
   unsigned short int   mY;
   unsigned short int   mDir;
   unsigned short int   mButton1;
   unsigned short int   mButton2;
   unsigned short int   mButton3;
} MOUSE;

typedef struct {
   long int JoyUp;
   long int JoyDown;
   long int JoyLeft;
   long int JoyRight;
   long int JoyButton1;
   long int JoyButton2;
   long int JoyButton3;
   long int JoyButton4;
} JOYSTICK_CONFIG;

typedef struct {
   long int MouseUp;
   long int MouseDown;
   long int MouseLeft;
   long int MouseRight;
   long int MouseLeftButton;
   long int MouseMiddleButton;
   long int MouseRightButton;
   long int MouseDummy;
} MOUSE_CONFIG;

#define _DEVICE_ON 1
#define OK 0
#define SONY_PAD 0x41
#define PAUSED 0
#define NOT_PAUSED 1

extern volatile MOUSE etc_Mouse1;
extern volatile JOYSTICK etc_Joy1;
extern volatile unsigned long int etc_Mouse1Register;
extern volatile unsigned long int etc_Joystick1Register;
extern volatile MOUSE_CONFIG etc_Mouse1Config;
extern volatile JOYSTICK_CONFIG etc_Joy1Config;
extern volatile char *etc_TransferBuffer1;
extern char mouse_sensitivity;
extern volatile unsigned char etc_Mouse1Switch;    /* id = 3 */
extern volatile unsigned char etc_Joystick1Switch;
extern volatile unsigned char etc_InputDeviceSelect;

#define TIMER_ID  1
#define TIMER_RATE      1

//#define   GAME_WIDTH  320
//#define GAME_HEIGHT   240

void     oldmain(void);
void     QuitToDos(void);
void     pmkey(char, char);
void JJSReadTIM(char *filename , char *location, TimData *tim);
//char* DAlloc(void*   heap,long size);
void  LoadPilotIntoVram(char *filename);
void Swap(void);
int JJSLoadSprites (void);
void UpdateNotes(int paused);

extern int      WinPitch;
extern short    WinWidth;
extern short    WinHeight;
extern short    WinHeightX2;
int        RealPitch;
int        RealBackPitch;
char      *RealScreen;

BOOL      bActive;   // is application active?
BOOL backLocked=FALSE;
extern char    *CurBuffer, *CurBufferJ;
char     *backscreen;

extern long     NeedKeys;

extern void     finiSndObjects(void);

HWND hWnd;
HDC hDC;
HGLRC hRC;

void renderInit()
{
   PIXELFORMATDESCRIPTOR pfd;
   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize = sizeof(pfd);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.cColorBits = 32;
   pfd.cRedBits = 8;
   pfd.cGreenBits = 8;
   pfd.cBlueBits = 8;
   pfd.cAlphaBits = 8;
   pfd.cDepthBits = 24;

   hDC = GetDC(hWnd);

   int format = ChoosePixelFormat(hDC, &pfd);
   SetPixelFormat(hDC, format, &pfd);
   hRC = wglCreateContext(hDC);

   wglMakeCurrent(hDC, hRC);

   glClearColor(0, 0, 0, 1);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_FRONT);

   //glEnable(GL_DEPTH_TEST);
   glDisable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);

   glEnable(GL_ALPHA_TEST);
   glAlphaFunc(GL_GREATER, 0.0f);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, 640, 480, 0, 0, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void renderFree()
{
   wglMakeCurrent(0, 0);
   wglDeleteContext(hRC);
   ReleaseDC(hWnd, hDC);
}

void renderSwap()
{
   Sleep(33);
   SwapBuffers(hDC);

   char buf[256];
   //   sprintf(buf, "%d = G3:%d G4:%d F3:%d F4:%d GT3:%d GT4:%d FT3:%d FT4:%d\n", (dbg_G3 + dbg_G4 + dbg_F3 + dbg_F4 + dbg_GT3 + dbg_GT4 + dbg_FT3 + dbg_FT4), dbg_G3, dbg_G4, dbg_F3, dbg_F4, dbg_GT3, dbg_GT4, dbg_FT3, dbg_FT4);
   //   OutputDebugString(buf);
   dbg_G3 = dbg_G4 = dbg_F3 = dbg_F4 = dbg_GT3 = dbg_GT4 = dbg_FT3 = dbg_FT4 = 0;

}

void renderResize(int width, int height)
{
   glViewport(0, 0, width, height);
}

void renderClear()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


static void restoreTextures(void)
{
   switch (trackNum)
   {
   case 0:
      //erm...not got anywhere yet...
      break;
   case 1:
      SJRNewTexture("wipeout\\newgraph\\venom\\altima\\altima.sjr");
      break;
   case 2:
      SJRNewTexture("wipeout\\newgraph\\venom\\karbonis\\karbonis.sjr");
      break;
   case 3:
      SJRNewTexture("wipeout\\newgraph\\venom\\terramax\\terramax.sjr");
      break;
   case 4:
      SJRNewTexture("wipeout\\newgraph\\venom\\korodera\\korodera.sjr");
      break;
   case 5:
      SJRNewTexture("wipeout\\newgraph\\venom\\arridos\\arridos.sjr");
      break;
   case 6:
      SJRNewTexture("wipeout\\newgraph\\venom\\silver\\silver.sjr");
      break;
   case 7:
      SJRNewTexture("wipeout\\newgraph\\venom\\firestar\\firestar.sjr");
      break;
   case 8:
      SJRNewTexture("wipeout\\newgraph\\rapier\\altima\\altima.sjr");
      break;
   case 9:
      SJRNewTexture("wipeout\\newgraph\\rapier\\karbonis\\karbonis.sjr");
      break;
   case 10:
      SJRNewTexture("wipeout\\newgraph\\rapier\\terramax\\terramax.sjr");
      break;
   case 11:
      SJRNewTexture("wipeout\\newgraph\\rapier\\korodera\\korodera.sjr");
      break;
   case 12:
      SJRNewTexture("wipeout\\newgraph\\rapier\\arridos\\arridos.sjr");
      break;
   case 13:
      SJRNewTexture("wipeout\\newgraph\\rapier\\silver\\silver.sjr");
      break;
   case 14:
      SJRNewTexture("wipeout\\newgraph\\rapier\\firestar\\firestar.sjr");
      break;
   case 15:
      SJRNewTexture ("wipeout\\newgraph\\menu\\menus.sjr");
      reloadScr();
      break;
   default:
      break;
   }
   if(globalPilot[0])
      LoadPilotIntoVram(globalPilot) ;
   Swap();
   Swap();
}

/*
 * finiObjects
 *
 * finished with all objects we use; release them
 */
static void finiObjects(void)
{
   renderFree();
   ShellSetPrefs();
// SystemParametersInfo(SPI_SETKEYBOARDDELAY,Keydelay,0,0);
// SystemParametersInfo(SPI_SETKEYBOARDSPEED,Keyrepeat,0,0);
// SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,sSaver,0,0);
}               /* finiObjects */

void changeScreen(void)
{
/*TODO
   int i;

   if (g_pddsPrimary != NULL)
   {
      g_pddsPrimary->Release();
//    g_pddsPrimary = NULL;
   }
   if (lpBack_Buffer != NULL)
   {
      lpBack_Buffer->Release();
      lpBack_Buffer = NULL;
   }
   for(i=0;i<500;++i)
      if (gTex[i].lpDDSTex != NULL)
      {
         gTex[i].Texture->Release();
         gTex[i].lpDDSTex->Release();
         gTex[i].lpDDSTex = NULL;
      }
*/
   WinWidth = reses[screenres].xsize;
   WinHeight = reses[screenres].ysize;
   WinHeightX2 = WinHeight * 2;
   setupScreens();
   restoreTextures();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   char keyhit;

   switch (msg)
   {
      case WM_ACTIVATE:
         bActive = TRUE;// (wParam == WA_ACTIVE);
         break;

      case WM_DESTROY:
         finiObjects();
         finiSndObjects();
         PostQuitMessage(0);
         QuitToDos();
         break;

      case WM_SIZE:
         renderResize(LOWORD(lParam), HIWORD(lParam));
         break;

      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
         pmkey((char)wParam, (msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN));
         break;

      default:
         return DefWindowProc(hWnd, msg, wParam, lParam);
   }

   return 0;
} 

void setupScreens(void)
{
   upres = reses[screenres].mult;

   JJSLoadSprites();
}

void initrandy()
{
   short i;
   time_t t;

   t=time(NULL);

   srand(t);

   for (i = 0; i < 2000; i++)
      randlist[i] = rand() & 0x7FFF;

   randnum = &randlist[0];;
   ctr = 0;
}

int randy(void)
{
   short num;

   num = *randnum++;

   ctr++;
   if (ctr == 2000)
   {
      randnum = &randlist[0];;
      ctr = 0;
   }

   return num;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   long tmp;
   int retval;

   RECT r = { 0, 0, 640, 480 };

   AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
   int wx = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
   int wy = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;

   hWnd = CreateWindow("static", "wipEout xport", WS_OVERLAPPEDWINDOW, wx + r.left, wy + r.top, r.right - r.left, r.bottom - r.top, 0, 0, hInstance, 0);

   SetWindowLong(hWnd, GWL_WNDPROC, (LONG)&WndProc);
   ShowWindow(hWnd, SW_SHOWDEFAULT);

   renderInit();

   initrandy();
   tmp = randy();
   lpCmdLine = lpCmdLine;
   hPrevInstance = hPrevInstance;

   TextureBuffer = (char *)malloc(TEXTURESJR);
   WinWidth = reses[screenres].xsize;
   WinHeight = reses[screenres].ysize;
   WinHeightX2 = WinHeight * 2;
   hinst = hInstance;
   SaveDirBase = (char *)malloc(120);
   retval = ShellGetGameDirectory(SaveDirBase);
   if (retval != ERROR_SUCCESS)
      strcpy(SaveDirBase, "c:\\");

   DataDirBase = (char *)malloc(120);
   retval = ShellGetDataDirectory(DataDirBase);
   if (retval != ERROR_SUCCESS)
      strcpy(DataDirBase, "c:\\");

   MovieDirBase = (char *)malloc(120);
   retval = ShellGetMovieDirectory(MovieDirBase);
   if (retval != ERROR_SUCCESS)
      strcpy(MovieDirBase, "c:\\");

   setupScreens();

   CDStop();
   oldmain();


   DestroyWindow(hwnd);

   return 0;
}

int        ProcessMessages(void)
{
   MSG        msg;
   do
   {
      if(backFromDead)
      {
         backFromDead=FALSE;
         while(PeekMessage(&msg,NULL,MM_JOY1MOVE,MM_JOY2BUTTONUP,PM_REMOVE));
      }
      while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))      // Clear all outstanding messages
      {
         if (GetMessage(&msg, NULL, 0, 0))
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if(PeekMessage(&msg,NULL,MM_JOY1MOVE,MM_JOY2BUTTONUP,PM_REMOVE))
            {
               TranslateMessage(&msg);
               DispatchMessage(&msg);
            }
            if (bActive)
               return (0);
         }
         else
         {
            if (bActive)
               return (1);
         }
      }
   } while (!bActive);

   if (!bActive)
      ProcessMessages();
   return (0);
}

void DDSwap(void)
{
   renderSwap();


}

void WinQuit(void)
{
   finiObjects();
   finiSndObjects();
   PostQuitMessage(0);
}

long numoff=0;
long numsmall=0;

void setRenderBegin(void)
{
   bTexEn = FALSE;
   texpage = 420;
   trans = FALSE;

   renderClear();
}

void setRenderEnd(void)
{

}

void sendList(void)
{

}

short jjstestsrc=0,jjstestdst=0,jjstest3=128;

#if 0
BOOL SetState(C3D_EPRIM Quad, C3D_ESHADE Shade, BOOL BTexEn, BOOL Trans, C3D_ETLIGHT Light)
{
   BOOL change=FALSE,tranfix=FALSE;
   C3D_ETEXOP chkey = C3D_ETEXOP_NONE;
   static BOOL FiltOn=FALSE;

   if(Quad!=quad)
   {
      sendList();
      quad = Quad;
      ATI3DCIF_ContextSetState(ghRC, C3D_ERS_PRIM_TYPE,
                     &quad);
      change = TRUE;
   }
   if(Shade != shade)
   {
      if(!change)
         sendList();
      shade = Shade;
      ATI3DCIF_ContextSetState(ghRC, C3D_ERS_SHADE_MODE,
                     &shade);
      change = TRUE;
   }
   if(BTexEn != bTexEn)
   {
      if(!change)
         sendList();
      bTexEn = BTexEn;
      ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_EN, &bTexEn);
      change = TRUE;
   }
   if(Light != light)
   {
      if(!change)
         sendList();
      light = Light;
      ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_LIGHT, &light);
      change = TRUE;
   }
   if(Trans != trans)
   {
      if(!change)
         sendList();
      trans = Trans;
      if(trans)
      {
         switch(trans)
         {
            case BLACK_NOT_DRAWN:
               chkey = C3D_ETEXOP_CHROMAKEY;//SJR - make 0,0,0 trans
               eDstAlpha = C3D_EADST_ZERO;
               eSrcAlpha = C3D_EASRC_ONE;
               break;
            case BLACK5_TRANSL:
            case RED_TRANSL:
            case BLACK1_TRANSL:
            case BLACK2_TRANSL:
            case BLACK4_TRANSL:
            case BLACK6_TRANSL:
               tranfix=TRUE;
               chkey = C3D_ETEXOP_CHROMAKEY;//SJR - make 0,0,0 trans
            case BLACK3_TRANSL:
               eDstAlpha = C3D_EADST_ZERO;
               eSrcAlpha = C3D_EASRC_DSTCLR;
               break;
            case WHITE1_TRANSL:
            case WHITE2_TRANSL:
            case WHITE3_TRANSL:
            case WHITE4_TRANSL:
            case WHITE5_TRANSL:
            case WHITE6_TRANSL:
            case WHITE7_TRANSL:
            case WHITE8_TRANSL:
            case UNUSED1_TRANSL:
            default:
               eDstAlpha = C3D_EADST_ONE;
               eSrcAlpha = C3D_EASRC_INVDSTCLR;

               break;
         }
      }
      else
      {
         eDstAlpha = C3D_EADST_ZERO;
         eSrcAlpha = C3D_EASRC_ONE;
      }
      if(tranfix && ft && FiltOn)
      {
         ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_FILTER, &eTexFilteroff);
         FiltOn=FALSE;
      }
      if(!tranfix && ft && !FiltOn)
      {
         ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_FILTER, &eTexFilteron);
         FiltOn=TRUE;
      }
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_TMAP_TEXOP, &chkey);
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_ALPHA_DST, &eDstAlpha);
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_ALPHA_SRC, &eSrcAlpha);

      change = TRUE;
   }
   return(change);
}
#endif
short GetThePage(short page,short u,short v)
{
   short newpage;
   char *tex=menuTexturesIn?JJSTextureMenuTable:JJSTextureTable;

   newpage=tex[page&0x1f];
   if(newpage>4)
      newpage+=400;
   else
   if(!menuTexturesIn)
   {
      PageXOff=u>>5;
      PageYOff=v>>5;
      newpage*=64;
      newpage+=PageXOff;
      newpage+=PageYOff<<3;
      PageXOff<<=5;
      PageYOff<<=5;
   }
   return(newpage);
}

void JJSDrawSprite(SPRT_8* sp, short w, short h)
{
   static D3DTLVERTEX vert[4]={0},*v;
   vert[0].color=vert[1].color=vert[2].color=vert[3].color=RGB_MAKE(255,255,255);

   short tpage;
   short nu,nv;
   SPRT_8 s2=*sp;

   nu=s2.u0;
   nv=s2.v0;
   if(s2.y0<0)
   {
      nv-=s2.y0;
      h+=s2.y0;
      s2.y0=0;
   }
   else
   if(s2.y0+h>=reses[screenres].ysize)
   {
      if(s2.y0>=reses[screenres].ysize)
         return;
      h=reses[screenres].ysize-s2.y0;
   }
   tpage=GetThePage(s2.clut,nu,nv);
   if(tpage==415)
      tpage=422;
   if(s2.g0 && tpage == 421)
   {
      tpage = 422;
      nv+=128;
   }

#if 0
   if(texpage !=tpage)
   {
      sendList();
      texpage=tpage;
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_TMAP_SELECT, &(gTex[texpage].hTX));
   }
   if(!SetState(C3D_EPRIM_QUAD,C3D_ESH_FLAT,TRUE,s2.b0==RED_TRANSL?RED_TRANSL:BLACK_NOT_DRAWN,C3D_ETL_NONE))
      if(numvert>(VBUFSIZE-4))
         sendList();
#endif
   v = vert;
   v->sx = s2.x0;
   v->sy = s2.y0;
   v->sz = 0;
   v->tu = (nu)/256.0f;
   v->tv = (nv)/256.0f;
   v->rhw = 1.0;
   ++v;
   v->sx = s2.x0+(float)w;
   v->sy = s2.y0;
   v->sz = 0;
   v->tu = (nu+w)/256.0f;
   v->tv = (nv)/256.0f;
   v->rhw = 1.0;
   ++v;
   v->sx = s2.x0+(float)w;
   v->sy = s2.y0+(float)h;
   v->sz = 0;
   v->tu = (nu+w)/256.0f;
   v->tv = (nv+h)/256.0f;
   v->rhw = 1.0;
   ++v;
   v->sx = s2.x0;
   v->sy = s2.y0+(float)h;
   v->sz = 0;
   v->tu = (nu)/256.0f;
   v->tv = (nv+h)/256.0f;
   v->rhw = 1.0;

   v = vert;
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, gTex[tpage].Texture);

   glColor3f(1, 1, 1);
   glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f(v[0].tu, v[0].tv);
      glVertex2f(s2.x0,     s2.y0);
      glTexCoord2f(v[1].tu, v[1].tv);
      glVertex2f(s2.x0 + w, s2.y0);
      glTexCoord2f(v[2].tu, v[2].tv);
      glVertex2f(s2.x0 + w, s2.y0 + h);
      glTexCoord2f(v[3].tu, v[3].tv);
      glVertex2f(s2.x0,     s2.y0 + h);
   glEnd();
}

void JJS_Sprite(SPRT* sp)
{
   JJSDrawSprite((SPRT_8 *)sp,sp->w,sp->h);
}

void JJS_Sprite8(SPRT_8* sp)
{
   JJSDrawSprite(sp,8,8);
}
/*TODO
void SetColor(D3DTLVERTEX *v,char r, char g, char b)
{
   if(g)
   {
      int r;
      if(g==25)
      {
         v->color=RGB_MAKE(40,40,255);    // Speed up
      }
      else
      {
         r=DepthFadeTble[(g<<8) + WhitePal];
         v->color=RGB_MAKE(ddPalette[r].peRed,ddPalette[r].peGreen,ddPalette[r].peBlue);
      }
   }
   else
   if(b)
   {
      switch(b)
      {
         case BLACK2_TRANSL:
            v->color=RGB_MAKE(180,180,180);
            break;
         case RED_LIGHT_FADE:
            v->color=RGB_MAKE(255,10,0);
            break;
         case WHITE2_TRANSL:
         default:
            v->color=RGB_MAKE(100,100,100);
            break;
      }
   }
   else
   {
      v->color=RGB_MAKE(ddPalette[r].peRed,ddPalette[r].peGreen,ddPalette[r].peBlue);

   }
}
*/
void JJSDrawPolyF4(POLY_F4* p)
{
   dbg_F4++;
   static D3DTLVERTEX vert[4]={0},*v;

#if 0
   if(!SetState(C3D_EPRIM_QUAD,C3D_ESH_FLAT,FALSE,p->b0,C3D_ETL_NONE))
      if(numvert>(VBUFSIZE-4))
         sendList();

#endif
   v = vert;
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x3);
   v->sy = SCALEIT(p->y3);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   v->sz = 0;

   v = vert;
   glDisable(GL_TEXTURE_2D);
   glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
   glBegin(GL_TRIANGLE_FAN);
      glVertex2f(v[0].sx, v[0].sy);
      glVertex2f(v[1].sx, v[1].sy);
      glVertex2f(v[2].sx, v[2].sy);
      glVertex2f(v[3].sx, v[3].sy);
   glEnd();
}

void JJSDrawPolyG4(POLY_G4* p)
{
   dbg_G4++;
   static D3DTLVERTEX vert[4]={0},*v;

#if 0
   if(!SetState(C3D_EPRIM_QUAD,C3D_ESH_FLAT,FALSE,p->b0,C3D_ETL_NONE))
      if(numvert>(VBUFSIZE-4))
         sendList();
#endif
   v = vert;
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x3);
   v->sy = SCALEIT(p->y3);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   v->sz = 0;

   v = vert;
   glDisable(GL_TEXTURE_2D);
   glBegin(GL_TRIANGLE_FAN);
      glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
      glVertex2f(v[0].sx, v[0].sy);
      glColor3f(p->r1 / 255.0f, p->g1 / 255.0f, p->b1 / 255.0f);
      glVertex2f(v[1].sx, v[1].sy);
      glColor3f(p->r3 / 255.0f, p->g3 / 255.0f, p->b3 / 255.0f);
      glVertex2f(v[2].sx, v[2].sy);
      glColor3f(p->r2 / 255.0f, p->g2 / 255.0f, p->b2 / 255.0f);
      glVertex2f(v[3].sx, v[3].sy);
   glEnd();
}

void setst(D3DTLVERTEX *vv,short u,short v,short page)
{

   if(menuTexturesIn || page>=400)
   {
      vv->tu=(u * (1.0f/256.0f));
      vv->tv=(v * (1.0f/256.0f));
   }
   else
   {
      vv->tu=((u-PageXOff) * (1.0f/32.0f));
      vv->tv=((v-PageYOff) * (1.0f/32.0f));
   }
}

void JJSDrawPolyFT4(POLY_FT4* p)
{
   dbg_FT4++;
   static D3DTLVERTEX vert[4]={0},*v;
   vert[0].color=vert[1].color=vert[2].color=vert[3].color=RGB_MAKE(255,255,255);
   short temp;

   temp = GetThePage(p->tpage,(p->u0+p->u1+p->u2+p->u3)/4,(p->v0+p->v1+p->v2+p->v3)/4);
   if(temp==417 || temp==418) // nasty horrible bodge
   {
      if(p->u0==0 && p->v0==24)
      {
         if(temp==417)
         {
            p->u0=0;
            p->v0=64;
            p->u1=127;
            p->v1=64;
            p->u2=0;
            p->v2=203;
            p->u3=127;
            p->v3=203;
         }
         else
         {
            p->u0=128;
            p->v0=64;
            p->u1=255;
            p->v1=64;
            p->u2=128;
            p->v2=203;
            p->u3=255;
            p->v3=203;
         }
         temp=420;
      }
   }
#if 0
   if(texpage != temp)
   {
      sendList();
      texpage=temp;
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_TMAP_SELECT, &(gTex[texpage].hTX));
   }
   if(!SetState(C3D_EPRIM_QUAD,C3D_ESH_FLAT,TRUE,p->b0,p->g0?C3D_ETL_MODULATE:C3D_ETL_NONE))
      if(numvert>(VBUFSIZE-4))
         sendList();
   if(p->pad4 & 0x4000)
   {
      if(!ft&1)
      {
         sendList();
         ATI3DCIF_ContextSetState(ghRC, C3D_ERS_TMAP_FILTER, &eTexFilteron);
      }
   }
#endif

   v = vert;

   if (p->z0 <= 0)
   {
      v->rhw = 1.0;
      v->sz = 0.0;
   }
   else
   {
      v->sz = SCALEZ(p->z0);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   setst(v,p->u0,p->v0,temp);

   ++v;
   if (p->z1 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z1);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   setst(v,p->u1,p->v1,temp);

   ++v;
   if (p->z3 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z3);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x3);
   v->sy = SCALEIT(p->y3);
   setst(v,p->u3,p->v3,temp);

   ++v;
   if (p->z2 <= 0)
   {
      v->sz =0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z2);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   setst(v,p->u2,p->v2,temp);

   v = vert;
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, gTex[temp].Texture);

   if (p->g0)
      glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
   else
      glColor3f(1, 1, 1);

   glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f(v[0].tu, v[0].tv);
      glVertex2f(v[0].sx, v[0].sy);

      glTexCoord2f(v[1].tu, v[1].tv);
      glVertex2f(v[1].sx, v[1].sy);
      
      glTexCoord2f(v[2].tu, v[2].tv);
      glVertex2f(v[2].sx, v[2].sy);
      
      glTexCoord2f(v[3].tu, v[3].tv);
      glVertex2f(v[3].sx, v[3].sy);
   glEnd();
}

void JJSDrawPolyF3(POLY_F4* p)
{
   dbg_F3++;
   static D3DTLVERTEX vert[3]={0},*v;

#if 0
   if(!SetState(C3D_EPRIM_TRI,C3D_ESH_FLAT,FALSE,p->b0,C3D_ETL_NONE))
      if(numvert>(VBUFSIZE-4))
         sendList();
#endif
   v = vert;
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   v->sz = 0;

   v = vert;
   glDisable(GL_TEXTURE_2D);
   glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
   glBegin(GL_TRIANGLES);
      glVertex2f(v[0].sx, v[0].sy);
      glVertex2f(v[1].sx, v[1].sy);
      glVertex2f(v[2].sx, v[2].sy);
   glEnd();
}

void JJSDrawPolyG3(POLY_G3* p)
{
   dbg_G3++;
   static D3DTLVERTEX vert[3]={0},*v;

#if 0
   if(!SetState(C3D_EPRIM_TRI,C3D_ESH_FLAT,FALSE,p->b0,C3D_ETL_NONE))
      if(numvert>(VBUFSIZE-4))
         sendList();
#endif
   v = vert;
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   v->sz = 0;
   ++v;
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   v->sz = 0;

   v = vert;
   glDisable(GL_TEXTURE_2D);
   glBegin(GL_TRIANGLES);
      glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
      glVertex2f(v[0].sx, v[0].sy);
      glColor3f(p->r1 / 255.0f, p->g1 / 255.0f, p->b1 / 255.0f);
      glVertex2f(v[1].sx, v[1].sy);
      glColor3f(p->r2 / 255.0f, p->g2 / 255.0f, p->b2 / 255.0f);
      glVertex2f(v[2].sx, v[2].sy);
   glEnd();
}

void JJSDrawPolyFT3(POLY_FT3* p)
{
   dbg_FT3++;
   static D3DTLVERTEX vert[3]={0},*v;
   vert[0].color=vert[1].color=vert[2].color=RGB_MAKE(255,255,255);
   short temp;

   temp=GetThePage(p->tpage,(p->u0+p->u1+p->u2)/3,(p->v0+p->v1+p->v2)/3);

#if 0
   if(texpage!=temp)
   {
      sendList();
      texpage=temp;
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_TMAP_SELECT, &(gTex[texpage].hTX));
   }
   if(!SetState(C3D_EPRIM_TRI,C3D_ESH_FLAT,TRUE,p->b0,p->g0?C3D_ETL_MODULATE:C3D_ETL_NONE))
      if(numvert>(VBUFSIZE-4))
         sendList();
#endif
   v = vert;
   if (p->z0 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z0);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   setst(v,p->u0,p->v0,temp);

   ++v;
   if (p->z1 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z1);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   setst(v,p->u1,p->v1,temp);

   ++v;
   if (p->z2 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z2);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   setst(v,p->u2,p->v2,temp);

   v = vert;
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, gTex[temp].Texture);

   if (p->g0)
      glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
   else
      glColor3f(1, 1, 1);
   glBegin(GL_TRIANGLES);
      glTexCoord2f(v[0].tu, v[0].tv);
      glVertex2f(v[0].sx, v[0].sy);
      glTexCoord2f(v[1].tu, v[1].tv);
      glVertex2f(v[1].sx, v[1].sy);
      glTexCoord2f(v[2].tu, v[2].tv);
      glVertex2f(v[2].sx, v[2].sy);
   glEnd();
}

void JJSDrawPolyGT3(POLY_GT3 * p)
{
   dbg_GT3++;
   static   D3DTLVERTEX vert[3]={0},*v;
   vert[0].color=vert[1].color=vert[2].color=RGB_MAKE(255,255,255);
   short temp;

   temp=GetThePage(p->tpage,(p->u0+p->u1+p->u2)/3,(p->v0+p->v1+p->v2)/3);

#if 0
   if(texpage!=temp)
   {
      sendList();
      texpage=temp;
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_TMAP_SELECT, &(gTex[texpage].hTX));
   }
   if(!SetState(C3D_EPRIM_TRI,C3D_ESH_FLAT,TRUE,p->b0,p->g0?C3D_ETL_MODULATE:C3D_ETL_NONE))
   {
      if(numvert>(VBUFSIZE-4))
         sendList();
   }
#endif
   v = vert;
   v->sz = SCALEZ(p->z0);
   if (p->z0 <= 0)
   {
      v->sz =0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z0);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   setst(v,p->u0,p->v0,temp);

   ++v;
   if (p->z1 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z1);
      v->rhw = 1.0f / v->sz;

   }
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   setst(v,p->u1,p->v1,temp);

   ++v;
   if (p->z2 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z2);
      v->rhw = 1.0f / v->sz;

   }
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   setst(v,p->u2,p->v2,temp);

   v = vert;
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, gTex[temp].Texture);

   glColor3f(1, 1, 1);
   glBegin(GL_TRIANGLES);
      glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
      glTexCoord2f(v[0].tu, v[0].tv);
      glVertex2f(v[0].sx, v[0].sy);
      
      glColor3f(p->r1 / 255.0f, p->g1 / 255.0f, p->b1 / 255.0f);
      glTexCoord2f(v[1].tu, v[1].tv);
      glVertex2f(v[1].sx, v[1].sy);

      glColor3f(p->r2 / 255.0f, p->g2 / 255.0f, p->b2 / 255.0f);
      glTexCoord2f(v[2].tu, v[2].tv);
      glVertex2f(v[2].sx, v[2].sy);
   glEnd();
}

void JJSDrawPolyGT4(POLY_GT4* p)
{
   dbg_GT4++;
   static   D3DTLVERTEX vert[4]={0},*v;
   vert[0].color=vert[1].color=vert[2].color=vert[3].color=RGB_MAKE(255,255,255);
   short temp;

   temp=GetThePage(p->tpage,(p->u0+p->u1+p->u2)/3,(p->v0+p->v1+p->v2)/3);

#if 0
   if(texpage!=temp)
   {
      sendList();
      texpage=temp;
      ATI3DCIF_ContextSetState (ghRC, C3D_ERS_TMAP_SELECT, &(gTex[texpage].hTX));
   }
   if(!SetState(C3D_EPRIM_TRI,C3D_ESH_FLAT,TRUE,p->b0,p->g0?C3D_ETL_MODULATE:C3D_ETL_NONE))
   {
      if(numvert>(VBUFSIZE-4))
         sendList();
   }
#endif
   v = vert;
   v->sz = SCALEZ(p->z0);
   if (p->z0 <= 0)
   {
      v->sz =0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z0);
      v->rhw = 1.0f / v->sz;
   }
   v->sx = SCALEIT(p->x0);
   v->sy = SCALEIT(p->y0);
   setst(v,p->u0,p->v0,temp);

   ++v;
   if (p->z1 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z1);
      v->rhw = 1.0f / v->sz;

   }
   v->sx = SCALEIT(p->x1);
   v->sy = SCALEIT(p->y1);
   setst(v,p->u1,p->v1,temp);

   ++v;
   if (p->z3 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z3);
      v->rhw = 1.0f / v->sz;

   }
   v->sx = SCALEIT(p->x3);
   v->sy = SCALEIT(p->y3);
   setst(v,p->u3,p->v3,temp);

   ++v;
   if (p->z2 <= 0)
   {
      v->sz = 0.0;
      v->rhw =1.0;
   }
   else
   {
      v->sz = SCALEZ(p->z2);
      v->rhw = 1.0f / v->sz;

   }
   v->sx = SCALEIT(p->x2);
   v->sy = SCALEIT(p->y2);
   setst(v,p->u2,p->v2,temp);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, gTex[temp].Texture);

   v = vert;
   glBegin(GL_TRIANGLE_FAN);
      glColor3f(p->r0 / 255.0f, p->g0 / 255.0f, p->b0 / 255.0f);
      glTexCoord2f(v[0].tu, v[0].tv);
      glVertex2f(v[0].sx, v[0].sy);

      glColor3f(p->r1 / 255.0f, p->g1 / 255.0f, p->b1 / 255.0f);
      glTexCoord2f(v[1].tu, v[1].tv);
      glVertex2f(v[1].sx, v[1].sy);

      glColor3f(p->r3 / 255.0f, p->g3 / 255.0f, p->b3 / 255.0f);
      glTexCoord2f(v[2].tu, v[2].tv);
      glVertex2f(v[2].sx, v[2].sy);

      glColor3f(p->r2 / 255.0f, p->g2 / 255.0f, p->b2 / 255.0f);
      glTexCoord2f(v[3].tu, v[3].tv);
      glVertex2f(v[3].sx, v[3].sy);
   glEnd();
}

BOOL UnloadTexture(TEXTURE* pTex)
{
   if (pTex->Texture == 0)
      return FALSE;

   glDeleteTextures(1, &pTex->Texture);
   pTex->Texture = 0;
   return TRUE;
}

char tempTex[320 * 240 * 2]; /*TODO*/

char *LockPilot(void)
{
   return tempTex;
}

void UnlockPilot(void)
{
   glBindTexture(GL_TEXTURE_2D, gTex[420].Texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tempTex);
}

void ChopUp(char *to, void *from , int which)
{
   char *p1=(char *)from;
   int i,x,y;

   which &= 63;
   x=which&7;
   y=which>>3;
   p1+=y*(512*32);
   p1+=x*64;
   for(i=0;i<32;++i)
   {
      memcpy(to,p1,64);
      p1+=512;
      to+=64;
   }

}

/*
 *  LoadTexture - load a texture map from a file into a texture surface
 */
BOOL SJRLoadTextureBMPmenu(const char* lpszTexFilename)
{
   FILE* fp;
   int i;
   long texsize;
   char* ptr;

   texsize = 32768;//128*128 * sizeof (C3D_UINT16)

   if (strcmp(lastfile, lpszTexFilename))
   {
      fp = fopen(lpszTexFilename, "rb");
      if (!fp)
      {
         //ASSERT(FALSE);
         return FALSE;
      }

      fread((void*)TextureBuffer, TEXTURESJR, 1, fp);
      fclose(fp);
      strcpy(lastfile, lpszTexFilename);
   }

   { /*TODO WTF?*/
      WORD* tp, temp;

      tp = (WORD*)TextureBuffer;
      for (i = 0; i < TEXTURESJR / 2; ++i)
      {
         temp = *tp;
         *tp++ = (temp & 0x1f) | ((temp & 0x7fe0) << 1);
      }
   }

   ptr = TextureBuffer;

   for (i = 0; i < 420; i++)
   {
      if (i >= 5 && i < 405)
         continue;

      glGenTextures(1, &gTex[i].Texture);
      glBindTexture(GL_TEXTURE_2D, gTex[i].Texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, ptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      ptr += texsize * 4;
   }

   return TRUE;
}

/*
 *  LoadTexture - load a texture map from a file into a texture surface
 */
BOOL SJRLoadTextureBMP(const char* lpszTexFilename)
{
   FILE* fp;
   int i;
   long texsize;
   char* ptr;
   char* data;
   short track, other, size;

   track=32;
   other=256;

   texsize = 32768;//128*128 * sizeof (C3D_UINT16)

   if (strcmp(lastfile, lpszTexFilename))
   {
      fp = fopen(lpszTexFilename, "rb");
      if (!fp)
      {
         //ASSERT(FALSE);
         return FALSE;
      }

      fread((void *)TextureBuffer, TEXTURESJR, 1, fp);
      fclose(fp);
      strcpy(lastfile, lpszTexFilename);
   }

   {
      WORD *tp,temp;

      tp=(WORD *)TextureBuffer;
      for(i=0;i<TEXTURESJR/2;++i)
      {
         temp=*tp;
         *tp++=(temp&0x1f)|((temp&0x7fe0)<<1);
      }
   }

   ptr = TextureBuffer;

   for (i = 0; i < 420; i++)
   {
      if (i >= 320 && i < 405)
         continue;

      if (i < 320)
      {
         ChopUp(tempTex, ptr, i);
         data = tempTex;
      }
      else
      {
         data = ptr;
      }

      size = (i < 320) ? track : other;
      glGenTextures(1, &gTex[i].Texture);
      glBindTexture(GL_TEXTURE_2D, gTex[i].Texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (i > 320 || ((i & 63) == 63))
         ptr += texsize*4;
   }

   return TRUE;
}

/*
 *  LoadTexture - load a texture map from a file into a texture surface
 */
int JJSLoadSprites(void)
{
   FILE* fp;
   int   i, sp;

   static char* lpszTexFilename[3]={
      "wipeout\\newgraph\\menu\\page12.sjr",
      "wipeout\\newgraph\\menu\\page15.sjr",
      "wipeout\\newgraph\\menu\\page17.sjr"
   };

   for (sp = 0; sp < 3; sp++)
   {
      fp = fopen(lpszTexFilename[sp], "rb");
      if (!fp)
      {
         //ASSERT(FALSE);
         return FALSE;
      }
      fread(tempTex, 256 * 256 * 2, 1, fp);
      fclose(fp);

      {
         short *tp, temp;

         tp = (short*)tempTex;
         for(i = 0; i < 256 * 256; i++)
         {
            temp = *tp;
            //temp = (temp >> 8) | (temp << 8);
            if ((temp & 0xFFFE) == 0)
            {
               temp = 0;
            }
            else
            {
               temp |= 1;
            }
            *tp++ = temp;
         }
      }

      glGenTextures(1, &gTex[420 + sp].Texture);
      glBindTexture(GL_TEXTURE_2D, gTex[420 + sp].Texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, tempTex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }

   winLoseIn = 0; // TODO WTF?
   return TRUE;
}


BOOL SJRNewTexture(char *filename)
{
   int i;

   // unload all textures
   for (i = 0; i < 420; i++)
   {
      UnloadTexture(gTex + i);
   }

   if(menuTexturesIn)
      return(SJRLoadTextureBMPmenu (filename));
   else
      return(SJRLoadTextureBMP (filename));
}

#if 0
HINSTANCE hInstance;
HWND hwndMCIWnd;

LRESULT FAR PASCAL VideoWndProc(HWND hWnd, UINT iMsg, WPARAM wParam,
LPARAM lParam)
{
   switch(iMsg) {
      case MM_MCINOTIFY:
         switch(wParam) {
            case MCI_NOTIFY_SUCCESSFUL:
            case MCI_NOTIFY_ABORTED:
            case MCI_NOTIFY_SUPERSEDED:
            case MCI_NOTIFY_FAILURE:
               MCIWndClose(hwndMCIWnd);
               MCIWndDestroy(hwndMCIWnd);
               DestroyWindow(hWnd);
               break;
         }
         return 1;
      case WM_DESTROY:
         PostQuitMessage(0);
         return 1;
/*    case MCIWNDM_NOTIFYMODE:
         if(lParam == MCI_MODE_STOP) {
            MCIWndOpen(hwndMCIWnd, "n:\\mmdata\\mpeg\\test.mpg", 0);
            MCIWndPlay(hwndMCIWnd);
         }
         return 1;*/
      default:
         return (DefWindowProc(hWnd, iMsg, wParam, lParam));
   }
}




/*void ShellMoviePlay(char *pName, int width, int height, int fullScreen)
{
  char szMoviePathName[MAX_PATH];
  struct sMovie movie;
  DWORD dwAttributes;
  DWORD result;

  // Build full path name to movie file
  strcpy(szMoviePathName, pName);

  // Convert it to short form for MCI layer
  result = GetShortPathName(
    szMoviePathName,                // points to a null-terminated path string
    movie.szMoviePathName,          // points to a buffer to receive the null-terminated short form of the path
    sizeof(movie.szMoviePathName)   // specifies the size of the buffer pointed to by lpszShortPath
  );

   Can only play movie if the file exists
  dwAttributes = GetFileAttributes(movie.szMoviePathName);
  if (dwAttributes == 0xffffffff)
  {
    // Movie file is not there
    return;
  }

  movie.width = width;
  movie.height = height;
  movie.fullScreen = fullScreen;

  shellMovieFinishedG = FALSE;

  // Ask shell thread to play movie
  while (!PostMessage(hwnd, MESSAGE_MOVIE_PLAY, (WPARAM)0, (LPARAM)&movie) )
  {
    Sleep(50);
  }

  // Wait for movie to finish
  while (!shellMovieFinishedG)
  {
    Sleep(50);
    ShellSync(0);
  }

}*/


 /****************************************************************************
  *
  * Start a movie playing
  *
  ****************************************************************************/

int ShellMovieStart(char *moviename)
{
  DWORD dwMCIWndStyle;
  MCI_PLAY_PARMS playInfo;
  DWORD dwErr;
  int result;
  int width, height;
  int cx, cy;

  cx = GetSystemMetrics(SM_CXSCREEN);
  cy = GetSystemMetrics(SM_CYSCREEN);
  /* Default to fail */
  result = -1;

  /* Set main app window to full screen                  */
  /* DEEP WEIRD MAGIC:                                   */
  /* Setting it to half size then full size seems to get */
  /* rid of the system busy cursor that appeared when    */
  /* DirectDraw was shut down                            */
  //SetWindowPos(hwnd, HWND_TOP, 0, 0, cx/2, cy/2, SWP_NOZORDER);
  SetWindowPos(hwnd, HWND_TOP, 0, 0, cx, cy, 0);
  UpdateWindow(hwnd);

  /* Create MCI window to play movie */
  dwMCIWndStyle = WS_CHILD | WS_VISIBLE | MCIWNDF_NOPLAYBAR | /*MCIWNDF_NOTIFYMODE | */
    CS_BYTEALIGNCLIENT | MCIWNDF_NOMENU | MCIWNDF_NOERRORDLG;

  hwndMCIWnd = MCIWndCreate(hwnd, hinst, dwMCIWndStyle, moviename);
  if (hwndMCIWnd)
  {
    /* Get the ID of the device that will play the movie */
    shellMovieDeviceIDG = MCIWndGetDeviceID(hwndMCIWnd);

    /* Size the movie window */
      width = cx;
      height = cy;

    SetWindowPos(hwndMCIWnd, HWND_TOP, (cx - width) / 2, (cy - height) / 2, width, height, NULL);

    /* Start movie play back */
    playInfo.dwCallback = MAKELONG(hwnd, 0);
    dwErr = mciSendCommand(shellMovieDeviceIDG, MCI_PLAY, MCI_NOTIFY, (DWORD)&playInfo);
    if (dwErr == 0)
    {
      /* Movie is playing */
      shellMoviePlayingG = TRUE;

      /* Ok */
      result = 0;
    }
    else
    {
      /* Movie play failed */
      ShellMovieFinished();
    }
  }

  return(result);
}



 /****************************************************************************
  *
  * Stop movie
  *
  ****************************************************************************/

void ShellMovieStop(void)
{
  MCIERROR mciretval;
  MCI_GENERIC_PARMS mciGenericParms;

  /* If the movie window exists */
  if (hwndMCIWnd)
  {
    /* Stop the MPEG player */
    ZeroMemory(&mciGenericParms, sizeof(mciGenericParms));
    mciretval = mciSendCommand(shellMovieDeviceIDG, MCI_STOP, MCI_WAIT, (DWORD)(LPVOID) &mciGenericParms);
  }
}

 /****************************************************************************
  *
  * Cleanup after movies has finished playing
  *
  ****************************************************************************/

void ShellMovieFinished(void)
{
  /* Close any MCI window playing the movie */
  if (hwndMCIWnd)
  {
    MCIWndClose(hwndMCIWnd);
    MCIWndDestroy(hwndMCIWnd);

    hwndMCIWnd = NULL;
  }

  /* Flag movie finished */
  shellMoviePlayingG = FALSE;
  shellMovieFinishedG = TRUE;
}

 /****************************************************************************/


void mpeganim(void)
{
   int fp;
   char *mpegname;
   int namelen;
   char endchar;

   namelen = strlen(MovieDirBase) + 15;
   GetShortPathName(MovieDirBase,MovieDirBase,200);

   mpegname = (char *)malloc(namelen);

   endchar = *(MovieDirBase + strlen(MovieDirBase) - 1);
   if (endchar != '\\')
      sprintf(mpegname,"%s\\wipeout.mpg", MovieDirBase);
   else
      sprintf(mpegname,"%swipeout.mpg", MovieDirBase);

   //make sure the movie's there
   if ((fp = open(mpegname, O_RDONLY)) == -1)
      return;

   close(fp);

   ShellMovieStart(mpegname);

   while (shellMoviePlayingG)
   {
      ProcessMessages();
   }

   free(mpegname);

}
#endif

void centreMouse(void)
{
   SetCursorPos(128,100);
}

void JJSReadJoystick(void)
{
   JOYINFO myJoy;
   MMRESULT errVal;

   errVal = joyGetPos(JOYSTICKID1,&myJoy);

   JoyXVal = myJoy.wXpos >> 8;
   JoyYVal = myJoy.wYpos >> 8;

   JoyButton1 = myJoy.wButtons & 0x0001;
   JoyButton2 = myJoy.wButtons & 0x0002;
}

BOOL JJSJoyConfig(void)
{
   JOYCAPS mycaps;
   UINT joyrange;
   MMRESULT theres;

   if(joyGetDevCaps(JOYSTICKID1,&mycaps,sizeof(mycaps)) != JOYERR_NOERROR)
      return(FALSE);

   joyrange = mycaps.wXmax - mycaps.wXmin;
   joyModXVal = 256.0f/(float)joyrange;

   joySetThreshold(JOYSTICKID1, joyrange/512);

   joyrange = mycaps.wYmax - mycaps.wYmin;
   joyModYVal = 256.0f/(float)joyrange;

   theres = joySetCapture(hwnd, JOYSTICKID1, 0, TRUE);


   if (theres != JOYERR_NOERROR)
      switch(theres)
      {
         case MMSYSERR_NODRIVER:
            theres = 2;
            break;
         case JOYERR_NOCANDO:
            theres = 2;
            break;
         case JOYERR_UNPLUGGED:
            theres = 2;
            return FALSE;
            break;
      }
      return TRUE;

}

TimData* JJSLoad16BitTexture(char* filename, char location)
{
   TimData* timData;
   DWORD bufsize;
   long texsize;
   char* bmpdata;

   texsize = 320 * 240 * sizeof (WORD);
   bufsize = texsize;

   timData = (TimData*)DAlloc((DynamicHeap*)heap, sizeof(TimData));

   JJSReadTIM(filename, tempTex, timData);

   return timData;
}


