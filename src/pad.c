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
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "droid.h"
#include "camera.h"
#include "sound.h"
#include "global.h"

extern int andyTimeout;

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
// right
   if (pad & padRight)
      playerShip->r_brake_rot += FR32U;
   else
      if(playerShip->r_brake_rot > 0)
         playerShip->r_brake_rot -= FR32U;

   if(playerShip->r_brake_rot < 0)
      playerShip->r_brake_rot = 0;

   if (playerShip->r_brake_rot > 256)
      playerShip->r_brake_rot = 256;

// left
   if (pad & padLeft)
      playerShip->l_brake_rot += FR32U;
   else
      if(playerShip->l_brake_rot > 0)
         playerShip->l_brake_rot -= FR32U;

   if(playerShip->l_brake_rot < 0)
      playerShip->l_brake_rot = 0;

   if (playerShip->l_brake_rot > 256)
      playerShip->l_brake_rot = 256;
}

void SetView(ShipData *playerShip, short padView)
{
   if (xpad & padView)
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
   if ((xpad & padFire) && (playerShip->weaponType != NO_WEAPON))
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
         if (xpad & (PadCircle0 | PadStart)) andyTimeout=0;
         return(xpad & (PadCircle0 | PadStart));
      case M_SELECT:
         if (xpad & PadCross0) andyTimeout=0;
         return(xpad & PadCross0);
      case M_START:
         if (xpad & PadStart) andyTimeout=0;
         return(xpad & PadStart);
      default:
         break;
   }
   return(1);  // PCwipeout
}
