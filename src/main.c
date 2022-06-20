//#include <libspu.h>
#include "wipesrc\psx26\include\kernel.h"
#include "standard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "malloc.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "ttf.h"
#include "ships.h"
#include "weapon.h"
#include "droid.h"
#include "camera.h"
#include "draw.h"
#include "scene.h"
#include "fileops.h"
#include "sound.h"
#include "control.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "global.h"
#include "ajs.h"
#include "drawot.h"
#include "decl.h"
#include "libgte.h"
#include "sfx.h"
#include "ayrace.h"
#include "card.h"
#include "utils.h"

extern short trackNum; //SJR
extern int menuTexturesIn;

short    drawenv_tpage = 5;      // PCwipeout
short    BG_Colour = 0;  // PCwipeout

extern TimData *JJSLoad16BitTexture(char *filename, char location);


/*
 * for 2 meg mode we need this in the obj file:: 2mbyte.obj
 */

char      copyRight[/*24*/] = "copyright_Psygnosis_1995";

char      inmenu = 0, Reallyinmenu = 0;
char      trackdir[21];

extern char     andybodge;

long      DrawDistance = HIGH_DISTANCE;
TimData  *titleScreen;

void reloadStartScr(void)
{
   int i;
   titleScreen = JJSLoad16BitTexture("wipeout\\textures\\wiptitle.tim", VRAM);
   inmenu = 1;
   for (i = 0; i < 2; i++)
   {
      ClearOTagR(OT[CurrentScreen], OT_SIZE);
      Swap();
   }
   inmenu = 1;
   for (i = 0; i < 2; i++)
   {
      CentreText("PRESS ENTER", text_data(0, 200, 8), RedText);
      Swap();
   }
}


extern int TOut;


void oldmain(void)
{
   int        i;
   ConfigData      gameData;
   int        mode, timeout;
   int        toggle = 0;
   char      loadPath[48];
   char      loadNo[2];
   int        titleTimeout;
   static int      firstTime = 1;


   heap = (DynamicHeap *) malloc(HEAP_SIZE * 1024);
   if (heap == NULL)
   {
      printf("Not Enough Memory Available (1)\n");
      exit(-1);
   }
   InitDynamicMem(heap, HEAP_SIZE * 1024);

   CreateSineTable();

   Set_Device(0);   // PCwipeout

   InitSystem();

   InitController();

   InitialiseAmbientColour();
   InitialiseColourMatrix();
   InitialiseLightMatrix();

   InitSonyDisplay();

   InitGameDisplay();
   titleScreen = JJSLoad16BitTexture("wipeout\\textures\\wiptitle.tim", SCREEN);
   inmenu = 1;
   for (i = 0; i < 2; i++)
   {
      ClearOTagR(OT[CurrentScreen], OT_SIZE);
      Swap();
   }
   inmenu = 1;
   for (i = 0; i < 2; i++)
      Swap();

   InitSpuSound();


   SetCharTextures(8);
   SetCharTextures(12);
   SetCharTextures(16);
   HighScore(0);

   TextureTableCount = 0;
   InitText();

   srand(0);

   InitGameData(&gameData);

/*TODO saves
   LoadCardData(lfilename, 0, &gameData, (SelectionData *) 0);
*/

   SfxSetVolume((char) ((0x3fff - (75 * gameData.currSliderPos)) >> 6));

   InitPadSettings();
   InitChampionship();

   timeout = FR30 * 120;

   mode = ATTRACT;

   SetCharTextures(8);
   SetCharTextures(12);
   SetCharTextures(16);

   titleTimeout = 0;

   trackNum = 16; //SJR
   if ((gameData.gameType != CHAMPIONSHIP) || (mode == ATTRACT))
   {
      titleScreen = JJSLoad16BitTexture("wipeout\\textures\\wiptitle.tim", VRAM);
      inmenu = 1;
      for (i = 0; i < 2; i++)
      {
         ClearOTagR(OT[CurrentScreen], OT_SIZE);
         Swap();
      }
      inmenu = 1;
      for (i = 0; i < 2; i++)
      {
         CentreText("PRESS ENTER", text_data(0, 200, 8), RedText);
         Swap();
      }

      toggle = 0;

      while (1)
      {
         opad = pad;
         pad = gPad;
         xpad = (pad ^ opad) & pad;

         // ClearOTagR( OT[ CurrentScreen ], OT_SIZE );
         textPrim = CurrentScreen * 300;

         titleTimeout++;

         if (firstTime == 1)
            TOut = 30;
         else
            TOut = 5;

         if (titleTimeout > 1)
         {
            if (GetMenuKey(M_SELECT))
            {
               PlayNote(MENU_SELECT, 1, 20);
               for (i = 0; i < 10; i++)
               {
                  UpdateNotes(NOT_PAUSED);
               }
               mode = INTERACTIVE;
               firstTime = 1;
               break;
            }
         }

         Swap();

         toggle++;
      }
   }

   for (i = 0; i < 2; i++)
   {
      ClearOTagR(OT[CurrentScreen], OT_SIZE);
      Swap();
   }

   gameType = SINGLE;

   gameData.Lives = 3;
   gameData.EndOfGame = 0;
   gameData.continueConfirm = 1;
   gameData.Position = 8;

   /*TODO cheats*/
   /* Enable cheat permanently */
    //gameData.RapierClass = 1 ;
    //totalTracks = 14 ;
    //NumTracks = 7 ;
    //gameData.showBonusTrack = 1 ; //SJR

   /****************************/

   while (1)
   {
      for (i = 0; i < 2; i++)
      {
         ClearOTagR(OT[CurrentScreen], OT_SIZE);
         Swap();
      }

      if ((gameData.gameType != CHAMPIONSHIP) && (mode != ATTRACT))
      {
         SetDrawDistance(HIGH_DISTANCE);
         MenuControl(&(gameData), &mode, timeout);
      }

      if (andybodge == 2)
      {
         mode = ATTRACT;
         gameType = SINGLE;
         ownShip = randy() % 8;
         serialShip = -1;
         firstTime = 0;
         andybodge = 1;
      }

      SetDrawDistance(DrawDistance);

      if (mode == ATTRACT)
      {
         switch (randy() % 6)
            // switch(1)
         {
         case 0:
            trackNo = 2;    // Altima
            gameData.actTrackNum = 2;
            gameData.trackNum = 1;
            break;

         case 1:
            trackNo = 4;    // karbonis
            gameData.actTrackNum = 4;
            gameData.trackNum = 2;
            break;

         case 2:
            trackNo = 1;    // terramax
            gameData.actTrackNum = 1;
            gameData.trackNum = 3;
            break;

         case 3:
            trackNo = 12;    // korodera
            gameData.actTrackNum = 12;
            gameData.trackNum = 4;
            break;

         case 4:
            trackNo = 8;    // arridos
            gameData.actTrackNum = 8;
            gameData.trackNum = 5;
            break;

         case 5:
            trackNo = 9;    // silver
            gameData.actTrackNum = 9;
            gameData.trackNum = 6;
            break;
         }
      }
      else
      {
         trackNo = gameData.actTrackNum;
      }
      CdControlB(CdlStop, 0, 0);


      /* load screen stuff */
      strcpy(loadPath, "wipeout\\textures\\load");

      if (gameData.actTrackNum <= 9)
      {
         sprintf(loadNo, "%d", gameData.actTrackNum);
      }
      else
      {
         sprintf(loadNo, "%2d", gameData.actTrackNum);
      }
      strcat(loadPath, loadNo);
      strcat(loadPath, ".tim");
      ClearOTagR(OT[CurrentScreen], OT_SIZE);
      JJSLoad16BitTexture(loadPath, SCREEN);
      inmenu = 0;
      for (i = 0; i < 2; i++)
      {
         ClearOTagR(OT[CurrentScreen], OT_SIZE);
         Swap();
      }
      inmenu = 1;
      for (i = 0; i < 2; i++)
         Swap();

      /*
       * VsyncStart( loadPath );
       */

      menuTexturesIn = 0;
      trackNum = gameData.trackNum; //for restoring - SJR
      switch (gameData.trackNum)
      {
      case 1:
         SJRNewTexture("wipeout\\newgraph\\venom\\altima\\altima.sjr");
         strcpy(trackdir, "venom\\altima\\");
         break;
      case 2:
         SJRNewTexture("wipeout\\newgraph\\venom\\karbonis\\karbonis.sjr");
         strcpy(trackdir, "venom\\karbonis\\");
         break;
      case 3:
         SJRNewTexture("wipeout\\newgraph\\venom\\terramax\\terramax.sjr");
         strcpy(trackdir, "venom\\terramax\\");
         break;
      case 4:
         SJRNewTexture("wipeout\\newgraph\\venom\\korodera\\korodera.sjr");
         strcpy(trackdir, "venom\\korodera\\");
         break;
      case 5:
         SJRNewTexture("wipeout\\newgraph\\venom\\arridos\\arridos.sjr");
         strcpy(trackdir, "venom\\arridos\\");
         break;
      case 6:
         SJRNewTexture("wipeout\\newgraph\\venom\\silver\\silver.sjr");
         strcpy(trackdir, "venom\\silver\\");
         break;
      case 7:
         SJRNewTexture("wipeout\\newgraph\\venom\\firestar\\firestar.sjr");
         strcpy(trackdir, "venom\\firestar\\");
         break;
      case 8:
         SJRNewTexture("wipeout\\newgraph\\rapier\\altima\\altima.sjr");
         strcpy(trackdir, "rapier\\altima\\");
         break;
      case 9:
         SJRNewTexture("wipeout\\newgraph\\rapier\\karbonis\\karbonis.sjr");
         strcpy(trackdir, "rapier\\karbonis\\");
         break;
      case 10:
         SJRNewTexture("wipeout\\newgraph\\rapier\\terramax\\terramax.sjr");
         strcpy(trackdir, "rapier\\terramax\\");
         break;
      case 11:
         SJRNewTexture("wipeout\\newgraph\\rapier\\korodera\\korodera.sjr");
         strcpy(trackdir, "rapier\\korodera\\");
         break;
      case 12:
         SJRNewTexture("wipeout\\newgraph\\rapier\\arridos\\arridos.sjr");
         strcpy(trackdir, "rapier\\arridos\\");
         break;
      case 13:
         SJRNewTexture("wipeout\\newgraph\\rapier\\silver\\silver.sjr");
         strcpy(trackdir, "rapier\\silver\\");
         break;
      case 14:
         SJRNewTexture("wipeout\\newgraph\\rapier\\firestar\\firestar.sjr");
         strcpy(trackdir, "rapier\\firestar\\");
         break;
      default:
         printf("what track ?\n");
         exit(0);
         break;
      }

      /* InitializeGame (&packetData, &gameData) ; */

      textureX = 320;
      textureY = 0;

      clutX = 320;
      clutY = 256;

      TextureTableCount = 0;

      attractDelay = 0;

      tempLapTimes[0] = 0;
      tempLapTimes[1] = 0;
      tempLapTimes[2] = 0;
      tempLapTimes[3] = 0;
      tempLapTimes[4] = 0;


      gameData.newLapRecord = 0;

      inmenu = 0;
      Reallyinmenu = 0;

      race(&gameData, mode, timeout);
      // inmenu=1;
      mode = INTERACTIVE;

      InitDynamicMem(heap, HEAP_SIZE * 1024); /* The left over bit in
                      * the middle */
      TextureTableCount = 0;
      InitText();
   }
}
