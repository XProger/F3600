#include "standard.h"

#define  LANGUAGE_C  // PCwipeout
#include "wipesrc\psx26\include\kernel.h"



#include "malloc.h"
#include "tim.h"
#include "object.h"
#include "skeleton.h"
#include "track.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "droid.h"
#include "camera.h"
#include "combat.h"
#include "menus.h"
#include "handlers.h"
#include "dynam.h"
#include "sound.h"
#include "sparks2.h"
#include "global.h"


#define WriteSIOData( b, l )        write( fw, b, l )
#define ReadSIOData( b, l )         read( fr, b, l )


char                    first = SINGLE ;
long           fr,fw;      /* file descriptor */
static unsigned long    ev1, ev2;   /* read/write event descriptor */
static unsigned long    linkev1, linkev2; /* read/write event descriptor */

static long             initfr,initfw;    /* file descriptor */
static unsigned long    initev1, initev2; /* read/write event descriptor */

static unsigned long    errorev ;   /* error event descriptor */
static unsigned long    timeev ; /* timeout event descriptor */

static unsigned long    ev_r, ev_e, ev_w; /* read/error event descriptor */

int    writeFlag = 0 ;
static readMFlag = 1 ;
static readSFlag = 1 ;


uchar             transmitBuf[DATA_PACKET_SIZE] ;  /* transmit buffer */
uchar             receiveBuf[DATA_PACKET_SIZE] ;   /* receive buffer */









void SetSerialData(ShipData *shipIndex, Object **shipShapes)
{
   #ifdef   PCwipeout
   int            i, j;
   TrackSection   *section;
   register short    sx, sy, sz;
   register short    cx, cy, cz;


   for(i= 0; i < NO_OF_SHIPS; i++)
   {
      sy = rsin( shipIndex[i].hdg );
      cy = rcos( shipIndex[i].hdg );
      sx = rsin( shipIndex[i].pitch );
      cx = rcos( shipIndex[i].pitch );
      sz = rsin( shipIndex[i].roll );
      cz = rcos( shipIndex[i].roll );

      if (gameType == MASTER)
      {
         if(i == serialShip)
         {
            SetSkeletonPosition( shipShapes[ shipIndex[i].highResTableIndex ]->skeleton,
               shipIndex[i].ppivot.vx,
               shipIndex[i].ppivot.vy,
               shipIndex[i].ppivot.vz);

            SetSkeletonDirectionHPR(shipShapes[ shipIndex[i].highResTableIndex ]->skeleton,
                shipIndex[i].hdg,
                shipIndex[i].pitch,
                shipIndex[i].roll );

            shipIndex[i].unitVecNose.vx = -frac(sy,cx);
            shipIndex[i].unitVecNose.vy = -sx;
            shipIndex[i].unitVecNose.vz = frac(cy,cx);

            shipIndex[i].unitVecWing.vx = ((cy * cz) >> 12) + ((((sy * sz) >> 12) * sx) >> 12);
            shipIndex[i].unitVecWing.vy = -(sz * cx) >> 12;
            shipIndex[i].unitVecWing.vz = ((sy * cz) >> 12) - ((((cy * sx) >> 12) * sz) >> 12);
#if 1
            shipIndex[serialShip].vpivot.vx = (shipIndex[serialShip].ppivot.vx - shipIndex[serialShip].vpivot.vx)<<6;
            shipIndex[serialShip].vpivot.vy = (shipIndex[serialShip].ppivot.vy - shipIndex[serialShip].vpivot.vy)<<6;
            shipIndex[serialShip].vpivot.vz = (shipIndex[serialShip].ppivot.vz - shipIndex[serialShip].vpivot.vz)<<6;
#endif
         }
      }
      else if (gameType == SLAVE)
      {
         if(i != ownShip)
         {
            SetSkeletonPosition( shipShapes[ shipIndex[i].highResTableIndex ]->skeleton,
               shipIndex[i].ppivot.vx,
               shipIndex[i].ppivot.vy,
               shipIndex[i].ppivot.vz);

            SetSkeletonDirectionHPR(shipShapes[ shipIndex[i].highResTableIndex ]->skeleton,
                shipIndex[i].hdg,
                shipIndex[i].pitch,
                shipIndex[i].roll );

            shipIndex[i].unitVecNose.vx = -frac(sy,cx);
            shipIndex[i].unitVecNose.vy = -sx;
            shipIndex[i].unitVecNose.vz = frac(cy,cx);

            shipIndex[i].unitVecWing.vx = ((cy * cz) >> 12) + ((((sy * sz) >> 12) * sx) >> 12);
            shipIndex[i].unitVecWing.vy = -(sz * cx) >> 12;
            shipIndex[i].unitVecWing.vz = ((sy * cz) >> 12) - ((((cy * sx) >> 12) * sz) >> 12);
#if 1
            shipIndex[i].vpivot.vx = (shipIndex[i].ppivot.vx - shipIndex[i].vpivot.vx)<<6;
            shipIndex[i].vpivot.vy = (shipIndex[i].ppivot.vy - shipIndex[i].vpivot.vy)<<6;
            shipIndex[i].vpivot.vz = (shipIndex[i].ppivot.vz - shipIndex[i].vpivot.vz)<<6;
#endif
         }
      }

   }
   section = track->sections;
   for(i =  0; i < track->sectionCount; i++)
   {
      if (gameType == SLAVE)
      {
         for(j  = 0; j < NO_OF_SHIPS; j++)
         {
            if(j != ownShip)
            {
               if(shipIndex[j].nearSecNo == section->secNo)
               {
                  shipIndex[j].nearTrkSect = section;
                  break;
               }
            }
         }
      }
      if (gameType == MASTER)
      {
         if(shipIndex[serialShip].nearSecNo == section->secNo)
         {
            shipIndex[serialShip].nearTrkSect = section;
            break;
         }
      }
      section = section->nextSection.ptr;
   }
#endif
}

void MasterSlaveSIO (MusicPrims *musicPrims, SlideTextures *musicTex, SelectionData *selectData, char loadGames[][9], ConfigData *gameData)
{
#ifdef   PCwipeout
   long ctl = ReadCTSDSR();


   if (!selectData->combatComplete)
   {

      CentreText ("INITIALIZING SERIAL LINK UP",
               text_data (1,116, 8), RedText) ;
   }

   if (neg1.head == 8960)
   {
      DisplayButtons ( 130, 210, NBACK) ;
   }
   else
   {
      DisplayButtons ( 130, 210, PBACK) ;
   }
/*
   ScreenFooter (70, 210, WhiteText) ;
*/
   if (selectData->combatInit > 30 && !selectData->combatComplete)
   {
      switch( ctl )
      {
         case 0:
            /* PSX is booting Up.. become master */
            gameType = MASTER;
            SetRTSDTR( SIOCL_RTSON );
            WaitForSlave();
            selectData->combatComplete = 1 ;
            break;

         case 1:
            /* other PSX seems to think it's slave already */
            gameType = MASTER;
            SetRTSDTR( SIOCL_RTSON );
            WaitForSlave();
            selectData->combatComplete = 1 ;
            break;

         case 2:
            /* other PSX is master */
            gameType = SLAVE;
            SetRTSDTR( SIOCL_DTRON );
            WaitForMaster();
            selectData->combatComplete = 1 ;
            break;

         case 3:
            /* No PSX attached */
            gameType = SINGLE ;
            selectData->combatComplete = 1 ;
            break;
      }

      if (gameType == SLAVE)
         printf ("I am SLAVE\n") ;

      if (gameType == MASTER)
         printf ("I am MASTER\n") ;
   }

   selectData->combatInit ++ ;

   if (selectData->combatComplete > 0 && selectData->combatComplete < 32)
   {
      switch (gameType)
      {
         case MASTER:
                     CentreText ("SERIAL LINK ESTABLISHED",
                        text_data (1,116, 8), RedText) ;
                     CentreText ("YOU ARE PLAYER ONE",
                        text_data (1,150, 8), RedText) ;
                     break ;
         case SLAVE:
                     CentreText ("SERIAL LINK ESTABLISHED",
                        text_data (1,116, 8), RedText) ;
                     CentreText ("YOU ARE PLAYER TWO",
                        text_data (1,150, 8), RedText) ;
                     break ;
         case SINGLE:
                     CentreText ("NO OTHER PLAYSTATION PRESENT",
                        text_data (1,116, 8), RedText) ;
                     break ;

      }

      selectData->combatComplete ++ ;

   }
#endif   // PCwipeout
}


void static _error()
{
#ifdef PCwipeout
   printf("e(stat:%04x)",_comb_control(0,0,0)&0x3ff);
   _comb_control(2,0,0);  /* reset error and cancel read request */
   readMFlag = 1 ;
   readSFlag = 1 ;
   printf("(stat:%04x)",_comb_control(0,0,0)&0x3ff);
   printf("0\n");
#endif   // PCwipeout
}


void _start_remote()
{
#ifdef PCwipeout
   EnableEvent(ev_e);
#if 0
   EnableEvent(ev_r);
   EnableEvent(ev_w);
#endif
   _comb_control(1,3,BR);
   printf("baud rate = %d\n",_comb_control(0,3,0));
#endif // PCwipeout
}


void _init_cont()
{
#ifdef PCwipeout
   /* open and enable an event to detect the end of read operation */
#if 0
   ev_r = OpenEvent(HwSIO,EvSpIOER,EvMdNOINTR,NULL);

   ev_w = OpenEvent(HwSIO,EvSpIOEW,EvMdNOINTR,NULL);
#endif
   /* open and enable an event to detect the error */
   ev_e = OpenEvent(HwSIO,EvSpERROR,EvMdINTR,_error);

   /* attacth the SIO driver to the kernel */
   AddCOMB();

   /* open a write path and set baud-rate */
   fw = open("sio:",O_WRONLY);

   /* open a read path and set baud-rate */
   fr = open("sio:",O_RDONLY);

   _start_remote () ;

#endif // PCwipeout
}


void InitializeCombatLink (void)
{
#ifdef   PCwipeout
   SetRTSDTR( 0 ) ;
#endif //   PCwipeout
}





