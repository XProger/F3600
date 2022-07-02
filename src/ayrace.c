#define TIME_CONSTANT   33

#include "standard.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "tilelib.h"
#include "ttf.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "droid.h"
#include "camera.h"
#include "draw.h"
#include "scene.h"
#include "dynam.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "sound.h"
#include "effects.h"
#include "fileops.h"
#include "options.h"
#include "global.h"
#include "cone.h"
#include "race.h"
#include "spline.h"
#include "racectrl.h"
#include "credits.h"
#include "render.h"

#define START_EXTRO 0
#define UPDATE_EXTRO 1
#define DONE_EXTRO 2

#define PrmMax       64

extern long fr ;
extern ChampionShip  winTable[] ;
extern char winLoseIn;

extern short         ctrlsky;
extern Object        *ctrlshipShapes[];
extern Effect        *ctrleffects;
extern TrackCamera   ctrlcamera;
extern Shadow        *ctrlShadow;
extern RescueData    ctrlrescueDroid;
extern char          ctrlshowGameOver;
extern long          ctrlNeedEndText;
extern long          ctrlNeedWinLose;
extern long          ctrlNeedBackPoly;
extern long          ctrlNeedRaceInfo;
extern long          ctrlNeedDeSelectScreen;
extern long          ctrlNeedNewHighScores;
extern long          ctrlNeedRacePoints;
extern long          ctrlNeedChampTable;
extern long          ctrlNeedAllVenomComplete;
extern long          ctrlNeedVenomComplete;
extern long          ctrlNeedContinueGameConfirm;
extern long          ctrlNeedEndTextVar;
extern long          ctrlNeedWinLoseVar;
extern long          ctrlNeedInGameReset;
extern long          ctrlNeedInSingleGameConfirm;
extern long          ctrlNeedInGameConfirm;
extern long          ctrlNeedTargetIcon;
extern long          ctrlCDPaused;
extern long          ctrlfirsttime;
extern char          ctrlmachinePaused;
extern int           ctrlcdPause;
extern short         startIceText;
extern short         startFireText;
extern Speedo        ctrlspeedo;

extern ConfigData    *ctrlgameData;
extern long          ctrlmode;
extern long          ctrltimeout;
extern char          ctrlconfirm;
extern char          ctrllConfirm;
extern Texture       *ctrlweaponIconTextures[NO_OF_WEAPON_TYPES];
extern ShipData      *ctrlshipIndex;
extern Object        *ctrlprm[PrmMax];
extern int           ctrlchoice;
extern Object        *ctrlredLight;
extern Object        *ctrldonkey[2];
extern Texture       *ctrleffectTextures[NO_OF_EFFECT_TEXTURES];
extern WeaponShape   *ctrlweaponShapes;
extern Texture       *ctrlwinLose [2];
extern ulong         ctrltoggleDisplay;
extern long          ctrlCDPaused;
extern int           ctrlplayCredits;
extern WeaponShape   *ctrlweaponShapes;
extern WeaponData    *ctrlweaponIndex;
extern HiScoreData   ctrlhiScoreData;
extern long          confirmRestart;
extern long          ctrlNeedTargetIcon;
extern short         Shielded;
extern short         CurrTrkNo;

long  ctrl_init;
long  ctrl_says_quit;
long  CDRestartNeeded;
long  CDloopCheck;
unsigned long  start_time;
unsigned long  end_time;
unsigned long  duration;
unsigned long  timeloop;
unsigned long  ctrl_frame;
unsigned long  ctrl_remainder;

void race( ConfigData *gameData, int mode, int timeout )
{
   ctrl_init = 0;
   ctrlgameData = gameData;      //archive off the gamedata;
   ctrlmode = mode;
   ctrltimeout = timeout;

   StopNotes();
   raceinit();
   UpdateEffects(ctrleffects);

   /*TODO audio*/
   //CdControlB(CdlStop, 0, 0);

   UpdateNotes(NOT_PAUSED);

   CDRestartNeeded = 0;
   CDloopCheck = 0;
   start_time = end_time = osGetSystemTimeMS();
   racectrl();
   racectrl();
   racectrl();
   ctrl_frame = 1;   //exactly one frame
   ctrl_remainder  = 0;

   while(1)    //GAME LOOP
   {
      start_time = (osGetSystemTimeMS() - ctrl_remainder);
      for (timeloop = 0; timeloop < ctrl_frame; timeloop++)
      {
         racectrl();
      }
      DrawnShieldBefore = 0;     //Reset the shield prim.

      if (ctrl_says_quit)
      {
#if CD_AUDIO
         /*TODO audio*/
         //CdControlB(CdlStop, 0, 0);
#endif
         return;
      }

      if (CDRestartNeeded)
      {
         CDRestart();
         CDRestartNeeded = 0;
         ctrlCDPaused = 0;
      }

      // if (CDloopCheck++ > 20)
      //    {
      //    CDLoopCheck();
      //    CDloopCheck = 0;
      //    }
#if CD_AUDIO
      switch (ctrlCDPaused)
      {
      case 0:
         break;
      case 1:
         CdPause();
         break;
      case 2:
         CdPlay();
         ctrlCDPaused = 0;
         break;
      }
#endif

      renderClear();

      // if(trackNo == 13)
      //    AuroraBorialis(ctrlprm[ctrlsky]);
      if((trackNo == 8) || (trackNo == 11))
      {
         ColourRedLight(ctrlredLight);
         MoveDonkey(ctrldonkey);
      }
      else if((trackNo == 1)/* || (trackNo == 6)*/)
      {
         ColourRedLight(ctrlredLight);
      }
      // ColourDroidLights(ctrlshipShapes[RESCUE_DROID]);

      renderBegin3D();

      //The Sky
      ctrlprm[ctrlsky]->skeleton->update = 1;
      renderSky(ctrlprm[ctrlsky]);

      //LANDSCAPE
      DrawLandscape( &(ctrlshipIndex[ownShip]), &ctrlcamera );

      //TRACK
      renderTrack();

      //SHIPS
      DrawShips(ctrlshipIndex, ctrlshipShapes, ownShip, ctrlcamera);

      //SHADOW
      if (ctrlshipIndex[ownShip].attr & RACING) /* TODO enable for cinematics? */
         DrawShadow(ctrlshipShapes, ctrlshipIndex, ctrlShadow);

      //DROID
      SetSkeletonPosition( ctrlshipShapes[RESCUE_DROID]->skeleton,
         ctrlrescueDroid.pos.vx,
         ctrlrescueDroid.pos.vy,
         ctrlrescueDroid.pos.vz);

      SetSkeletonDirectionHPR(ctrlshipShapes[RESCUE_DROID]->skeleton,
         ctrlrescueDroid.hdg,
         ctrlrescueDroid.pitch,
         ctrlrescueDroid.roll );

      ctrlshipShapes[RESCUE_DROID]->skeleton->update = 1;
      TransformStaticObject( ctrlshipShapes[RESCUE_DROID] );

      //WEAPONS
      DrawWeapons(ctrlshipIndex, ctrlweaponShapes, ctrlweaponIndex);
      if (ctrlNeedTargetIcon == 1)
         DrawTargetIcon(&(ctrlshipIndex[ownShip]), ctrlshipIndex, ctrlshipShapes);

      //Effects
      AddEffects(ctrleffects, ctrlweaponIndex, ctrleffectTextures, ctrlshipIndex);
      DrawEffects(ctrleffects);

      renderEnd3D();

      if (ctrlmode == ATTRACT)
      {
         if (ctrlplayCredits == 1)
            ShowCredits(attractDelay);
      }


      //All game overs
      if ((ctrlshowGameOver == 1) || (gameData->showGameOver == 1) || (gameData->lostGame == 1))
         CentreText ("GAME OVER",text_data (60, 112, 16),RedText) ;


      //Text at end.
      if (ctrlNeedEndText)
      {
         switch (ctrlNeedEndTextVar)
         {
         case 1:
            CentreText ("CONGRATULATIONS",text_data (15, 20, 8),RedText) ;
            break;
         case 2:
            CentreText ("BAD LUCK",text_data (15, 20, 8),RedText) ;
            break;
         case 3:
            CentreText ("FAILED TO QUALIFY",text_data (15, 20, 8),RedText) ;
            break;
         default:
            CentreText ("LOST THE VARIABLE",text_data (15, 20, 8),RedText) ;
            break;
         }
      }
      //Pictures at end
      if (ctrlNeedWinLose)
      {
         setWinLose();
         DisplayPicture(ctrlwinLose[ctrlNeedWinLoseVar], 180, 50, 128, 140);
      }
      //High score bollox
      if (ctrlNeedRaceInfo)
         DisplayRaceInfo (gameData, &ctrlhiScoreData) ;
      if (ctrlNeedNewHighScores)
         NewHighScoreDraw (gameData, &ctrlhiScoreData) ;
      if (ctrlNeedRacePoints)
         DisplayRacePoints () ;
      if (ctrlNeedChampTable)
         DisplayChampTable () ;
      if (ctrlNeedAllVenomComplete)
         DisplayAllVenomComplete () ;
      if (ctrlNeedVenomComplete)
         DisplayVenomComplete () ;
      if (ctrlNeedContinueGameConfirm)
         ContinueGameConfirm (&(ctrlhiScoreData.confirm), xpad, &(ctrlhiScoreData.lConfirm), &ctrltoggleDisplay) ;
      if (ctrlNeedInSingleGameConfirm)
         RestartGameConfirm (&(ctrlhiScoreData.confirm), xpad, &(ctrlhiScoreData.lConfirm), &ctrltoggleDisplay) ;
      if (ctrlNeedInGameReset)
         InGameReset(ctrlchoice, &ctrltoggleDisplay, ctrlmachinePaused, gameData, &ctrlcdPause);
      if (ctrlNeedInGameConfirm)
         InGameConfirm (&ctrlconfirm, ctrlchoice, xpad, &ctrllConfirm, &ctrltoggleDisplay) ;
      /*TODO deselect screen?
      if (ctrlNeedDeSelectScreen)
         DeSelectScreen (ctrlselectBox, ctrlselectTrans) ;
         */
      if ( startIceText )
      {
         ShowIceCongratulations( );
      }
      if ( startFireText )
      {
         ShowFireCongratulations( );
      }


      //Semi trans bit.
      /*TODO back poly (end race?)
      if (ctrlNeedBackPoly)
         BackPoly (&(ctrlbackFrame[0]), &(ctrlbackFrameTrans[0]), 180, 50, 128, 140) ;
         */

      //HUD display
      if(ctrlshipIndex[ownShip].attr & RACING)
      {
         if (gameData->gameType == CHAMPIONSHIP)
            DisplayLives(gameData);

         DrawWeaponSprtIcons(ctrlweaponIconTextures, &(ctrlshipIndex[ownShip]));
         DrawHud(ctrlshipIndex, &ctrlspeedo, ctrlchoice, gameData);
      }

      Swap( );
      //do
      //{
      end_time = osGetSystemTimeMS();
      //}
      //while((end_time - start_time) <=TIME_CONSTANT);  //never faster than 30fps
      duration = end_time - start_time;
      ctrl_frame = duration / TIME_CONSTANT;
      ctrl_remainder = duration - (ctrl_frame * TIME_CONSTANT);
   }/*Main while loop*/
}

void Restart(  ShipData       *shipIndex,
          ConfigData       *gameData,
          WeaponData       *weaponIndex,
          WeaponShape   *weaponShapes,
          Effect        *effects,
          RescueData    *rescueDroid,
          Object        **shipShapes)
{

   int            i;


   if(winLoseIn)
      JJSLoadSprites();
   Shielded = 0;
   KillNotes();
   StopNotes();
   initShipData(track->sections, shipIndex, gameData);
   InitRescueDroid(rescueDroid, shipShapes, &(shipIndex[ownShip]));
   cameraPtr->section = shipIndex[ownShip].nearTrkSect;
   ResetStartBoom();
   for(i = 0; i < NO_ACTIVE_WEAPONS; i++)
   {
      weaponIndex[i].count = 0;
      weaponIndex[i].type = NO_WEAPON;
   }
   for(i = 0; i < NO_OF_WEAPONS; i++)
   {
      weaponShapes[i].used = 0;
   }
   for(i = 0; i < NO_OF_EFFECTS; i++)
   {
      effects[i].count = 0;
   }

   /*TODO audio*/
   //r_attr.depth.left = 2000;
   //r_attr.depth.right = 2000;

   CameraUpdate = UpdateCameraRaceStart;
   CDRestartNeeded = 1;
}



void QuitGame()
{
   int i;

   /*TODO audio
   for(i = 0; i < 10; i++)
   {
      c_attr.cd.volume.left  -= c_attr.cd.volume.left >> 3 ;   // Master volume (left)
      c_attr.cd.volume.right -= c_attr.cd.volume.right >> 3;   // Master volume (right)
      UpdateNotes(NOT_PAUSED);
   }

   r_attr.depth.left = 2000;
   r_attr.depth.right = 2000;
   */

   Shielded = 0;
   KillNotes();
}
