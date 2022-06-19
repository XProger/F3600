#include "standard.h"

#include "malloc.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "dynam.h"
#include "combat.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "droid.h"
#include "camera.h"
#include "sound.h"
#include "sparks2.h"
#include "global.h"

extern int           andyTimeout;
extern unsigned short   OriginalKeyCodes[16];



void InitPadSettings()
{
   pCon.thrust    = PadCross0;
   pCon.view      = PadTriangle0;
   pCon.fire      = PadCircle0;
   pCon.lBrake    = PadTopRight2;
   pCon.rBrake    = PadTopLeft2;
   pCon.up        = PadUp0;
   pCon.down      = PadDown0;
   pCon.left      = PadLeft0;
   pCon.right     = PadRight0;
}

void SetBrake(ShipData *playerShip, short padLeft, short padRight)
{
   if (pad & pCon.rBrake)  playerShip->r_brake_rot += FR32U;
   else if(playerShip->r_brake_rot > 0) playerShip->r_brake_rot -= FR32U;

   if(playerShip->r_brake_rot < 0) playerShip->r_brake_rot = 0;

// if (pad & pCon.rBrake)  playerShip->r_brake_rot += 32;
// else if(playerShip->r_brake_rot > 0) playerShip->r_brake_rot -= 32;

   if (playerShip->r_brake_rot > 256)
      playerShip->r_brake_rot = 256;

   if (pad & pCon.lBrake)  playerShip->l_brake_rot += FR32U;
   else if(playerShip->l_brake_rot > 0) playerShip->l_brake_rot -= FR32U;

   if(playerShip->l_brake_rot < 0) playerShip->l_brake_rot = 0;

   if (playerShip->l_brake_rot > 256)
      playerShip->l_brake_rot = 256;
}



void SetView(ShipData *playerShip, short padView)
{
   if (xpad & pCon.view)
   {
      if((playerShip->attr & VIEW_IN) == 0)
      {
         CameraUpdate = UpdateCameraRaceInternal;
         playerShip->attr |= VIEW_IN;
      }
      else
      {
         CameraUpdate = UpdateCameraRaceExternal;
         playerShip->attr &= ~VIEW_IN;
      }
   }
}



void SetFire(ShipData *playerShip, short padFire, WeaponData *weaponIndex, int shipNo)
{
   if ((xpad & pCon.fire) && (playerShip->weaponType != NO_WEAPON))
   {
      if((playerShip->attr & SHIELDED) == 0)
         FireWeapon(playerShip, weaponIndex, shipNo, playerShip->weaponType);
      else
         PlayNote(MENU_MOVE, 1, 20);
   }
}



short GetMenuKey(short key)
{
   switch(key)
   {
      case M_UP:
         if (xpad & PadUp0) andyTimeout=0;
         return(xpad & PadUp0);
      case M_DOWN:
         if (xpad & PadDown0) andyTimeout=0;
         return(xpad & PadDown0);
      case M_LEFT:
         if (xpad & PadLeft0) andyTimeout=0;
         return(xpad & PadLeft0);
      case M_RIGHT:
         if (xpad & PadRight0) andyTimeout=0;
         return(xpad & PadRight0);
      case M_BACK:
         if (xpad & PadTopRight1) andyTimeout=0;
         return(xpad & PadTopRight1);
      case M_SELECT:
         if (xpad & PadSelect) andyTimeout=0;
         return(xpad & PadSelect);
      case M_START:
         if (xpad & PadStart) andyTimeout=0;
         return(xpad & PadStart);
      case M_QUIT:
         if (xpad & PadTopRight1) andyTimeout=0;
         return(xpad & PadTopRight1);
      default:
         break;
   }
   return(1);  // PCwipeout
}
