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
   pCon.thrust    = IN_A;
   pCon.view      = IN_Y;
   pCon.fire      = IN_B;
   pCon.lBrake    = IN_RT;
   pCon.rBrake    = IN_LT;
   pCon.up        = IN_UP;
   pCon.down      = IN_DOWN;
   pCon.left      = IN_LEFT;
   pCon.right     = IN_RIGHT;
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
         if (xpad & IN_UP) andyTimeout=0;
         return(xpad & IN_UP);
      case M_DOWN:
         if (xpad & IN_DOWN) andyTimeout=0;
         return(xpad & IN_DOWN);
      case M_LEFT:
         if (xpad & IN_LEFT) andyTimeout=0;
         return(xpad & IN_LEFT);
      case M_RIGHT:
         if (xpad & IN_RIGHT) andyTimeout=0;
         return(xpad & IN_RIGHT);
      case M_BACK:
         if (xpad & (IN_B | IN_START)) andyTimeout=0;
         return(xpad & (IN_B | IN_START));
      case M_SELECT:
         if (xpad & IN_A) andyTimeout=0;
         return(xpad & IN_A);
      case M_START:
         if (xpad & IN_START) andyTimeout=0;
         return(xpad & IN_START);
      default:
         break;
   }
   return(1);  // PCwipeout
}
