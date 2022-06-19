
/* (C) Psygnosis 1994. By Jason Carl Denton
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <io.h>
#include "standard.h"

#include "malloc.h"
#include "tim.h"
#include "object.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "ttf.h"
#include "trackasm.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "droid.h"
#include "camera.h"
#include "draw.h"
#include "scene.h"
#include "sound.h"
#include "sparks2.h"
#include "global.h"
#include "asmtrk.h"
#include "wipesrc\win95\daves\pragmas.h"
#include "libgte.h"
#include "ajs.h"

void Test16Bit(long *Test16bitX, long *Test16bitY, long *Test16bitZ);

#define     DepthShift     2
#define     DepthMod    0

extern      char TrackDirection( TrackCamera* camera );
extern short   Shielded;
static int fp;
char str[40];

Surface     TrackSurface[ 24 ][ 2 ];         /* 20k */


PolyFT4*    TrackLo[ 2 ];
PolyFT4*    TrackMed[ 2 ];
PolyFT4*    TrackHi[ 2 ];

short       TrackLoCount = 0;
short       TrackMedCount = 0;
short       TrackHiCount = 0;                /* 82k total */

PolyFT4     TrackLo0[ TrackLoTiles ];        /* 17k */
PolyFT4     TrackLo1[ TrackLoTiles ];
/* 9k */
PolyFT4     TrackMed0[ TrackMedTiles ];
PolyFT4     TrackMed1[ TrackMedTiles ];
/* 36k */
PolyFT4     TrackHi0[ TrackHiTiles ];
PolyFT4     TrackHi1[ TrackHiTiles ];
extern short startLinePos[ ];




/* other bss stuff */
/* sound                                        21k */
/* display text                                 20k */
/* ot                                           8k */





/* Primitives for lo res track
Assuming max of 32 sections by 8 faces
which is 256 polys in total
*/

#define     ClipFlags      ( CrtClipped | ZLarge | ZNegative | ZSmall )
#define     ShortClipFlags (( CrtClipped | ZLarge | ZNegative | ZSmall ) >> 12 )
#define     MaxJumps       10


void AsmLowResTile
(
 TexTemplate*  Template,
 short          index,
 short          reversed
 )
{
   Poly           primitives;
   Poly           poly;

   Tile*          tile;

   Texture*       texture;

   /* Create Tile */

   texture = TextureTable[ Template[ index ].tex1[ 0 ] ];

   tile = &( TrackSurface[ index ][ reversed ].low[ 0 ] );

   tile->cba = texture->cba;
   tile->tsb = texture->tsb;

   if ( reversed )
   {
      tile->u0 = texture->u1;
      tile->u1 = texture->u0;
      tile->u2 = texture->u3;
      tile->u3 = texture->u2;

      tile->v0 = texture->v1;
      tile->v1 = texture->v0;
      tile->v2 = texture->v3;
      tile->v3 = texture->v2;
   }
   else
   {
      tile->u0 = texture->u0;
      tile->u1 = texture->u1;
      tile->u2 = texture->u2;
      tile->u3 = texture->u3;

      tile->v0 = texture->v0;
      tile->v1 = texture->v1;
      tile->v2 = texture->v2;
      tile->v3 = texture->v3;
   }
}


void AsmMediumResTile
(
 TexTemplate*  Template,
 short          index,
 short          reversed
 )
{
   Poly           primitives;
   Poly           poly;

   Tile*          tile;

   Texture*       texture;

   short          i;

   short          tex;

   /* Create Tile */

   for ( i=0; i<4; i++ )
   {
      if ( reversed )
      {
         tex = (i&2) + 1 - (i&1);
      }
      else
      {
         tex = i;
      }

      texture = TextureTable[ Template[ index ].tex4[ tex ] ];
      tile = &( TrackSurface[ index ][ reversed ].med[ i ] );

      tile->cba = texture->cba;
      tile->tsb = texture->tsb;

      if ( reversed )
      {
         tile->u0 = texture->u1;
         tile->u1 = texture->u0;
         tile->u2 = texture->u3;
         tile->u3 = texture->u2;

         tile->v0 = texture->v1;
         tile->v1 = texture->v0;
         tile->v2 = texture->v3;
         tile->v3 = texture->v2;
      }
      else
      {
         tile->u0 = texture->u0;
         tile->u1 = texture->u1;
         tile->u2 = texture->u2;
         tile->u3 = texture->u3;

         tile->v0 = texture->v0;
         tile->v1 = texture->v1;
         tile->v2 = texture->v2;
         tile->v3 = texture->v3;
      }
   }
}







void AsmHighResTile
(
 TexTemplate*  Template,
 short          index,
 short          reversed
 )
{
   Poly           primitives;
   Poly           poly;

   Tile*          tile;

   Texture*       texture;

   short          i;

   short          tex;

   /* Create Tile */

   for ( i=0; i<16; i++ )
   {
      if ( reversed )
      {
         tex = (3 - (i&3)) + (i&12);
      }
      else
      {
         tex = i;
      }

      texture = TextureTable[ Template[ index ].tex16[ tex ] ];
      tile = &( TrackSurface[ index ][ reversed ].high[ i ] );

      tile->cba = texture->cba;
      tile->tsb = texture->tsb;

      if ( reversed )
      {
         tile->u0 = texture->u1;
         tile->u1 = texture->u0;
         tile->u2 = texture->u3;
         tile->u3 = texture->u2;

         tile->v0 = texture->v1;
         tile->v1 = texture->v0;
         tile->v2 = texture->v3;
         tile->v3 = texture->v2;
      }
      else
      {
         tile->u0 = texture->u0;
         tile->u1 = texture->u1;
         tile->u2 = texture->u2;
         tile->u3 = texture->u3;

         tile->v0 = texture->v0;
         tile->v1 = texture->v1;
         tile->v2 = texture->v2;
         tile->v3 = texture->v3;
      }
   }
}







void AsmGenerateTiles
(
   TexTemplate*   texTemplate,
   short          numLib
   )
{
   short       i;
   PolyFT4*    ft4;


   for ( i=0; i<numLib; i++ )
   {
      AsmLowResTile( texTemplate, i, 0 );
      AsmLowResTile( texTemplate, i, 1 );

      AsmMediumResTile( texTemplate, i, 0 );
      AsmMediumResTile( texTemplate, i, 1 );

      AsmHighResTile( texTemplate, i, 0 );
      AsmHighResTile( texTemplate, i, 1 );
   }

   TrackLo[ 0 ] = TrackLo0;
   TrackLo[ 1 ] = TrackLo1;

   TrackMed[ 0 ] = TrackMed0;
   TrackMed[ 1 ] = TrackMed1;

   TrackHi[ 0 ] = TrackHi0;
   TrackHi[ 1 ] = TrackHi1;

   for ( i=0; i < TrackLoTiles; i++ )
   {
      ft4 = &( TrackLo0[ i ] );
      SetPolyFT4( ( POLY_FT4* ) ft4 );

      ft4 = &( TrackLo1[ i ] );
      SetPolyFT4( ( POLY_FT4* ) ft4 );
   }

   for ( i=0; i < TrackMedTiles; i++ )
   {
      ft4 = &( TrackMed0[ i ] );
      SetPolyFT4( ( POLY_FT4* ) ft4 );

      ft4 = &( TrackMed1[ i ] );
      SetPolyFT4( ( POLY_FT4* ) ft4 );
   }

   for ( i=0; i < TrackHiTiles; i++ )
   {
      ft4 = &( TrackHi0[ i ] );
      SetPolyFT4( ( POLY_FT4* ) ft4 );

      ft4 = &( TrackHi1[ i ] );
      SetPolyFT4( ( POLY_FT4* ) ft4 );
   }
}




void AsmNextFrame( )
{
   short    i;
   short    error;

   /* Initialise lo track polygon counter */

   TrackLoCount = 0;

   /* Initialise med track polygon counter */

   TrackMedCount = 0;

   /* Initialise med track polygon counter */

   TrackHiCount = 0;
}


TrackSection *AsmTransformTrackLo(TrackSection* firstSection,
                          Skeleton* camera, short loCount, short dir)
{
/*
Variables with greatest need for optimisation
Listed in decreasing order of priority
   */

   VECTOR*              vertex;

   VECTOR               camposData;
   VECTOR*              campos = &camposData;
   SVECTOR              svector[ 4 ];
   DVECTOR              screen[ 4 ];
   long                 depth[ 4 ];
   long                 flag[ 4 ];
   long                 interpolate[ 4 ];
   ushort               depth2Data[ 4 ];
   ushort               depth2[ 4 ];
   ushort               flag2[ 4 ];

   short                i, j, k;

   TrackSection*        section;
   Face*                face;

   VECTOR               *vertices;
   short                faceCount;

   PolyFT4*             ft4;

   Surface*             surface;

   Tile*                tile;

   short                a, b, c;

   long              Test16bitX;
   long              Test16bitY;
   long              Test16bitZ;
   short          var;


   /* Set transformation matrix */

   SetSkeletonTransform( camera->super );

   /* Copy camera position into fast ram */

   campos->vx = camera->relative.t[ 0 ];
   campos->vy = camera->relative.t[ 1 ];
   campos->vz = camera->relative.t[ 2 ];

   /* Calculate non-meshed track sections */

   vertices = track->vertices;
   section = firstSection;

   for ( i=0; i < loCount; i++ )
   {
      face = track->faces + section->faceStart;
      faceCount = section->faceCount;

      for ( j=0; j < faceCount; j++ )
      {
         vertex = vertices + face->vertex[ 0 ];
         Test16bitX = vertex->vx + campos->vx;
         Test16bitY = vertex->vy + campos->vy;
         Test16bitZ = vertex->vz + campos->vz;
         Test16Bit(&Test16bitX, &Test16bitY, &Test16bitZ);

         svector[ 0 ].vx = (short)Test16bitX;
         svector[ 0 ].vy = (short)Test16bitY;
         svector[ 0 ].vz = (short)Test16bitZ;


         vertex = vertices + face->vertex[ 1 ];
         Test16bitX = vertex->vx + campos->vx;
         Test16bitY = vertex->vy + campos->vy;
         Test16bitZ = vertex->vz + campos->vz;
         Test16Bit(&Test16bitX, &Test16bitY, &Test16bitZ);

         svector[ 1 ].vx = (short)Test16bitX;
         svector[ 1 ].vy = (short)Test16bitY;
         svector[ 1 ].vz = (short)Test16bitZ;


         vertex = vertices + face->vertex[ 2 ];
         Test16bitX = vertex->vx + campos->vx;
         Test16bitY = vertex->vy + campos->vy;
         Test16bitZ = vertex->vz + campos->vz;
         Test16Bit(&Test16bitX, &Test16bitY, &Test16bitZ);

         svector[ 2 ].vx = (short)Test16bitX;
         svector[ 2 ].vy = (short)Test16bitY;
         svector[ 2 ].vz = (short)Test16bitZ;


         vertex = vertices + face->vertex[ 3 ];
         Test16bitX = vertex->vx + campos->vx;
         Test16bitY = vertex->vy + campos->vy;
         Test16bitZ = vertex->vz + campos->vz;
         Test16Bit(&Test16bitX, &Test16bitY, &Test16bitZ);

         svector[ 3 ].vx = (short)Test16bitX;
         svector[ 3 ].vy = (short)Test16bitY;
         svector[ 3 ].vz = (short)Test16bitZ;

         RotTransPersN(svector, screen, depth2, flag2, 4);


         if (face->flags & TRACK_BASE)
            var = 1;
         else
            var = 0;

         //    if ( !( *flag & ClipFlags ) )
         if ( !( ( flag2[ 0 ] | flag2[ 1 ] | flag2[ 2 ] | flag2[ 3 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackLo[ CurrentScreen ][ TrackLoCount++ ] );
            SetPolyFT4((POLY_FT4 *)ft4);

            if ( face->flags & FlipTexture )
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            else
               surface = &( TrackSurface[ face->texture ][ 0 ] );

            tile = &( surface->low[ 0 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;


            if (NormalClip(screen[0],screen[1],screen[2])<0)
            {
               ft4->u0 = tile->u0;
               ft4->v0 = tile->v0;
               ft4->u1 = tile->u1;
               ft4->v1 = tile->v1;
               ft4->u2 = tile->u2;
               ft4->v2 = tile->v2;
               ft4->u3 = tile->u3;
               ft4->v3 = tile->v3;

               switch (dir)
               {
               case ShipForwards:
                  screen [0].vy -= var;
                  screen [1].vy -= var;
                  break;
               case ShipBackwards:
                  screen [0].vy += var;
                  screen [1].vy += var;
                  break;
               case ShipRight:
                  screen [0].vy += var;
                  screen [3].vy += var;
                  break;
               case ShipLeft:
                  screen [0].vy -= var;
                  screen [3].vy -= var;
                  break;
               }

               ft4->xy0.vx = screen[ 1 ].vx;
               ft4->xy0.vy = screen[ 1 ].vy;
               ft4->xy0.vz = depth2[ 1 ];
               ft4->xy1.vx = screen[ 0 ].vx;
               ft4->xy1.vy = screen[ 0 ].vy;
               ft4->xy1.vz = depth2[ 0 ];
               ft4->xy2.vx = screen[ 2 ].vx;
               ft4->xy2.vy = screen[ 2 ].vy;
               ft4->xy2.vz = depth2[ 2 ];
               ft4->xy3.vx = screen[ 3 ].vx;
               ft4->xy3.vy = screen[ 3 ].vy;
               ft4->xy3.vz = depth2[ 3 ];
               ft4->xy0.pad = 0;
               ft4->xy1.pad = 0;
               ft4->xy2.pad = 0;
               ft4->xy3.pad = 0;
               //ft4->xy1 = screen[ 0 ];
               //ft4->xy2 = screen[ 2 ];
               //ft4->xy3 = screen[ 3 ];
            }
            else
            {
               ft4->u1 = tile->u1;
               ft4->v1 = tile->v1;
               ft4->u0 = tile->u0;
               ft4->v0 = tile->v0;
               ft4->u3 = tile->u3;
               ft4->v3 = tile->v3;
               ft4->u2 = tile->u2;
               ft4->v2 = tile->v2;

               switch (dir)
               {
               case ShipForwards:
                  screen [0].vy -= var;
                  screen [1].vy -= var;
                  break;
               case ShipBackwards:
                  screen [0].vy += var;
                  screen [1].vy += var;
                  break;
               case ShipRight:
                  screen [0].vy += var;
                  screen [3].vy += var;
                  break;
               case ShipLeft:
                  screen [0].vy -= var;
                  screen [3].vy -= var;
                  break;
               }

               //ft4->xy1 = screen[ 1 ];
               //ft4->xy0 = screen[ 0 ];
               //ft4->xy3 = screen[ 2 ];
               //ft4->xy2 = screen[ 3 ];
               ft4->xy1.vx = screen[ 1 ].vx;
               ft4->xy1.vy = screen[ 1 ].vy;
               ft4->xy1.vz = depth2[ 1 ];
               ft4->xy0.vx = screen[ 0 ].vx;
               ft4->xy0.vy = screen[ 0 ].vy;
               ft4->xy0.vz = depth2[ 0 ];
               ft4->xy3.vx = screen[ 2 ].vx;
               ft4->xy3.vy = screen[ 2 ].vy;
               ft4->xy3.vz = depth2[ 2 ];
               ft4->xy2.vx = screen[ 3 ].vx;
               ft4->xy2.vy = screen[ 3 ].vy;
               ft4->xy2.vz = depth2[ 3 ];
               ft4->xy0.pad = 0;
               ft4->xy1.pad = 0;
               ft4->xy2.pad = 0;
               ft4->xy3.pad = 0;
            }


            /* Just use one corner for the depth info */

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //            AddPrim( OT[ CurrentScreen ] + ( *depth ), ( ulong* ) ft4 );
            AddPrim( OT[ CurrentScreen ] + ( (depth2[0] >> DepthShift) - DepthMod), ( ulong* ) ft4 );
         }

         face++;
      }

      section = section->nextSection.ptr;
   }

   return( section );
}

TrackSection *AsmTransformTrackMed
(
 TrackSection*     firstSection,
 Skeleton*         camera,
 short             medCount,
 short          dir
 )
{
/*
Variables with greatest need for optimisation
Listed in decreasing order of priority
   */

   VECTOR*              vertex;

   VECTOR               camposData;
   VECTOR*              campos = &camposData;
   SVECTOR              svector[ 9 ];
   DVECTOR              screen[ 9 ];
   ushort               depth[ 9 ];
   ushort               flag[ 9 ];
   VECTOR               meshVector[ 9 ];

   /* original 32 bit vertices */

   short                i, j, k;

   TrackSection*        section;
   Face*                face;

   VECTOR               *vertices, *v;
   short                faceCount;

   PolyFT4*             ft4;

   Surface*             surface;

   Tile*                tile;
   short                var = 0;


   /* Set transformation matrix */

   SetSkeletonTransform( camera->super );

   /* Copy camera position into fast ram */

   campos->vx = camera->relative.t[ 0 ];
   campos->vy = camera->relative.t[ 1 ];
   campos->vz = camera->relative.t[ 2 ];

   /* Calculate Med res meshed track sections */

   vertices = track->vertices;
   section = firstSection;

   for ( i=0; i < medCount; i++ )
   {
      face = track->faces + section->faceStart;
      faceCount = section->faceCount;

      for ( j=0; j < faceCount; j++ )
      {
         /* get original 32 bit faces coords */
         v = vertices + face->vertex[1];
         svector[0].vx = v->vx + campos->vx;
         svector[0].vy = v->vy + campos->vy;
         svector[0].vz = v->vz + campos->vz;

         v = vertices + face->vertex[0];
         svector[2].vx = v->vx + campos->vx;
         svector[2].vy = v->vy + campos->vy;
         svector[2].vz = v->vz + campos->vz;

         v = vertices + face->vertex[2];
         svector[6].vx = v->vx + campos->vx;
         svector[6].vy = v->vy + campos->vy;
         svector[6].vz = v->vz + campos->vz;

         v = vertices + face->vertex[3];
         svector[8].vx = v->vx + campos->vx;
         svector[8].vy = v->vy + campos->vy;
         svector[8].vz = v->vz + campos->vz;

         /* mesh top */

         svector[ 1 ].vx = ( svector[ 0 ].vx + svector[ 2 ].vx ) >> 1;
         svector[ 1 ].vy = ( svector[ 0 ].vy + svector[ 2 ].vy ) >> 1;
         svector[ 1 ].vz = ( svector[ 0 ].vz + svector[ 2 ].vz ) >> 1;

         /* mesh left */

         svector[ 3 ].vx = ( svector[ 0 ].vx + svector[ 6 ].vx ) >> 1;
         svector[ 3 ].vy = ( svector[ 0 ].vy + svector[ 6 ].vy ) >> 1;
         svector[ 3 ].vz = ( svector[ 0 ].vz + svector[ 6 ].vz ) >> 1;

         /* mesh right */

         svector[ 5 ].vx = ( svector[ 2 ].vx + svector[ 8 ].vx ) >> 1;
         svector[ 5 ].vy = ( svector[ 2 ].vy + svector[ 8 ].vy ) >> 1;
         svector[ 5 ].vz = ( svector[ 2 ].vz + svector[ 8 ].vz ) >> 1;

         /* mesh bottom */

         svector[ 7 ].vx = ( svector[ 6 ].vx + svector[ 8 ].vx ) >> 1;
         svector[ 7 ].vy = ( svector[ 6 ].vy + svector[ 8 ].vy ) >> 1;
         svector[ 7 ].vz = ( svector[ 6 ].vz + svector[ 8 ].vz ) >> 1;

         /* mesh centre */

         svector[ 4 ].vx = ( svector[ 3 ].vx + svector[ 5 ].vx ) >> 1;
         svector[ 4 ].vy = ( svector[ 3 ].vy + svector[ 5 ].vy ) >> 1;
         svector[ 4 ].vz = ( svector[ 3 ].vz + svector[ 5 ].vz ) >> 1;

         RotTransPersN( svector, screen, depth, flag, 9);


         /* Top left polygon */

         if ( !( ( flag[ 0 ] | flag[ 1 ] | flag[ 3 ] | flag[ 4 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackMed[ CurrentScreen ][ TrackMedCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }
            if (face->flags & TRACK_BASE)
               var = 1;
            else
               var = 0;
            tile = &( surface->med[ 0 ] );     // PCwipeout, was 2

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;



            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;
            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;
            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;
            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            switch (dir)
            {
            case ShipForwards:
               screen [0].vy -= var;
               screen [1].vy -= var;
               break;
            case ShipBackwards:
               screen [0].vy += var;
               screen [1].vy += var;
               break;
            case ShipRight:
               screen [0].vy += var;
               screen [3].vy += var;
               break;
            case ShipLeft:
               screen [0].vy -= var;
               screen [3].vy -= var;
               break;
            }
            //ft4->xy0 = screen[ 0 ] ;
            //ft4->xy1 = screen[ 1 ] ;
            //ft4->xy2 = screen[ 3 ];
            //ft4->xy3 = screen[ 4 ];
            ft4->xy0.vx = screen[ 0 ].vx;
            ft4->xy0.vy = screen[ 0 ].vy;
            ft4->xy0.vz = depth[ 0 ];
            ft4->xy1.vx = screen[ 1 ].vx;
            ft4->xy1.vy = screen[ 1 ].vy;
            ft4->xy1.vz = depth[ 1 ];
            ft4->xy2.vx = screen[ 3 ].vx;
            ft4->xy2.vy = screen[ 3 ].vy;
            ft4->xy2.vz = depth[ 3 ];
            ft4->xy3.vx = screen[ 4 ].vx;
            ft4->xy3.vy = screen[ 4 ].vy;
            ft4->xy3.vz = depth[ 4 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            AddPrim( OT[ CurrentScreen ] + ((depth[ 0 ] >> DepthShift) - DepthMod ), ( ulong* ) ft4 );
         }

         /* Top right polygon */

         if ( !( ( flag[ 1 ] | flag[ 2 ] | flag[ 4 ] | flag[ 5 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackMed[ CurrentScreen ][ TrackMedCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->med[ 1 ] ); // PCwipeout, was 3

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;


            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;
            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;
            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;
            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            switch (dir)
            {
            case ShipForwards:
               screen [2].vy -= var;
               screen [1].vy -= var;
               break;
            case ShipBackwards:
               screen [2].vy += var;
               screen [1].vy += var;
               break;
            case ShipRight:
               screen [2].vy -= var;
               screen [5].vy -= var;
               break;
            case ShipLeft:
               screen [3].vy += var;
               screen [5].vy += var;
               break;
            }
            //ft4->xy0 = screen[ 1 ];
            //ft4->xy1 = screen[ 2 ];
            //ft4->xy2 = screen[ 4 ];
            //ft4->xy3 = screen[ 5 ];
            ft4->xy0.vx = screen[ 1 ].vx;
            ft4->xy0.vy = screen[ 1 ].vy;
            ft4->xy0.vz = depth[ 1 ];
            ft4->xy1.vx = screen[ 2 ].vx;
            ft4->xy1.vy = screen[ 2 ].vy;
            ft4->xy1.vz = depth[ 2 ];
            ft4->xy2.vx = screen[ 4 ].vx;
            ft4->xy2.vy = screen[ 4 ].vy;
            ft4->xy2.vz = depth[ 4 ];
            ft4->xy3.vx = screen[ 5 ].vx;
            ft4->xy3.vy = screen[ 5 ].vy;
            ft4->xy3.vz = depth[ 5 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;
            AddPrim( OT[ CurrentScreen ] + (( depth[ 1 ] >> DepthShift) - DepthMod ), ( ulong* ) ft4 );
         }

         /* Bottom left polygon */

         if ( !( ( flag[ 3 ] | flag[ 4 ] | flag[ 6 ] | flag[ 7 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackMed[ CurrentScreen ][ TrackMedCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->med[ 2 ] ); // PCwipeout, was 0

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;
            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;
            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;
            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            switch (dir)
            {
            case ShipForwards:
               screen [6].vy += var;
               screen [7].vy += var;
               break;
            case ShipBackwards:
               screen [6].vy -= var;
               screen [7].vy -= var;
               break;
            case ShipRight:
               screen [6].vy += var;
               screen [3].vy += var;
               break;
            case ShipLeft:
               screen [6].vy -= var;
               screen [3].vy -= var;
               break;
            }

            //ft4->xy0 = screen[ 3 ];
            //ft4->xy1 = screen[ 4 ];
            //ft4->xy2 = screen[ 6 ];
            //ft4->xy3 = screen[ 7 ];
            ft4->xy0.vx = screen[ 3 ].vx;
            ft4->xy0.vy = screen[ 3 ].vy;
            ft4->xy0.vz = depth[ 3 ];
            ft4->xy1.vx = screen[ 4 ].vx;
            ft4->xy1.vy = screen[ 4 ].vy;
            ft4->xy1.vz = depth[ 4 ];
            ft4->xy2.vx = screen[ 6 ].vx;
            ft4->xy2.vy = screen[ 6 ].vy;
            ft4->xy2.vz = depth[ 6 ];
            ft4->xy3.vx = screen[ 7 ].vx;
            ft4->xy3.vy = screen[ 7 ].vy;
            ft4->xy3.vz = depth[ 7 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 3 ] >> DepthShift) - DepthMod ), ( ulong* ) ft4 );
         }

         /* Bottom right polygon */

         if ( !( ( flag[ 4 ] | flag[ 5 ] | flag[ 7 ] | flag[ 8 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackMed[ CurrentScreen ][ TrackMedCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->med[ 3 ] ); // PCwipeout, was 1

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;
            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;
            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;
            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            switch (dir)
            {
            case ShipForwards:
               screen [7].vy += var;
               screen [8].vy += var;
               break;
            case ShipBackwards:
               screen [7].vy -= var;
               screen [8].vy -= var;
               break;
            case ShipRight:
               screen [5].vy -= var;
               screen [8].vy -= var;
               break;
            case ShipLeft:
               screen [5].vy += var;
               screen [8].vy += var;
               break;
            }
            //ft4->xy0 = screen[ 4 ];
            //ft4->xy1 = screen[ 5 ];
            //ft4->xy2 = screen[ 7 ];
            //ft4->xy3 = screen[ 8 ];
            ft4->xy0.vx = screen[ 4 ].vx;
            ft4->xy0.vy = screen[ 4 ].vy;
            ft4->xy0.vz = depth[ 4 ];
            ft4->xy1.vx = screen[ 5 ].vx;
            ft4->xy1.vy = screen[ 5 ].vy;
            ft4->xy1.vz = depth[ 5 ];
            ft4->xy2.vx = screen[ 7 ].vx;
            ft4->xy2.vy = screen[ 7 ].vy;
            ft4->xy2.vz = depth[ 7 ];
            ft4->xy3.vx = screen[ 8 ].vx;
            ft4->xy3.vy = screen[ 8 ].vy;
            ft4->xy3.vz = depth[ 8 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 4 ] >> DepthShift) - DepthMod ), ( ulong* ) ft4 );

         }
         face++;
      }

      section = section->nextSection.ptr;
   }
   return( section );
}
#undef ShortClipFlags
#define ShortClipFlags 0

TrackSection *AsmTransformTrackHi
(
 TrackSection*     firstSection,
 Skeleton*         camera,
 AutoMeshData*     autoMesh,
 short             highCount,
 short          var
 )
{
/*
Variables with greatest need for optimisation
Listed in decreasing order of priority
   */

   VECTOR*              vertex;

   VECTOR               camposData;
   VECTOR*              campos = &camposData;
   SVECTOR              svector[ 25 ];
   DVECTOR              screen[ 25 ];
   ushort               depth[ 25 ];
   ushort               flag[ 25 ];

   /* original 32 bit vertices */

   short                i, j, k;

   TrackSection*        section;
   Face*                face;

   VECTOR               *vertices, *v;
   short                faceCount;

   PolyFT4*             ft4;

   Surface*             surface;

   Tile*                tile;

   long                 flags;

   short                x, y;

   short                index;
   short          varadd;



   /* Set transformation matrix */

   SetSkeletonTransform( camera->super );

   /* Copy camera position into fast ram */

   campos->vx = camera->relative.t[ 0 ];
   campos->vy = camera->relative.t[ 1 ];
   campos->vz = camera->relative.t[ 2 ];

   /* Calculate Hi res meshed track sections */

   vertices = track->vertices;
   section = firstSection;

   for ( i=0; i < highCount; i++ )
   {
      face = track->faces + section->faceStart;
      faceCount = section->faceCount;

      for ( j=0; j < faceCount; j++ )
      {
         v = vertices + face->vertex[1];
         svector[0].vx = v->vx + campos->vx;
         svector[0].vy = v->vy + campos->vy;
         svector[0].vz = v->vz + campos->vz;

         v = vertices + face->vertex[0];
         svector[4].vx = v->vx + campos->vx;
         svector[4].vy = v->vy + campos->vy;
         svector[4].vz = v->vz + campos->vz;

         v = vertices + face->vertex[2];
         svector[20].vx = v->vx + campos->vx;
         svector[20].vy = v->vy + campos->vy;
         svector[20].vz = v->vz + campos->vz;

         v = vertices + face->vertex[3];
         svector[24].vx = v->vx + campos->vx;
         svector[24].vy = v->vy + campos->vy;
         svector[24].vz = v->vz + campos->vz;

         /* mesh top */

         svector[ 2 ].vx = ( svector[ 0 ].vx + svector[ 4 ].vx ) >> 1;
         svector[ 2 ].vy = ( svector[ 0 ].vy + svector[ 4 ].vy ) >> 1;
         svector[ 2 ].vz = ( svector[ 0 ].vz + svector[ 4 ].vz ) >> 1;

         svector[ 1 ].vx = ( svector[ 0 ].vx + svector[ 2 ].vx ) >> 1;
         svector[ 1 ].vy = ( svector[ 0 ].vy + svector[ 2 ].vy ) >> 1;
         svector[ 1 ].vz = ( svector[ 0 ].vz + svector[ 2 ].vz ) >> 1;

         svector[ 3 ].vx = ( svector[ 2 ].vx + svector[ 4 ].vx ) >> 1;
         svector[ 3 ].vy = ( svector[ 2 ].vy + svector[ 4 ].vy ) >> 1;
         svector[ 3 ].vz = ( svector[ 2 ].vz + svector[ 4 ].vz ) >> 1;

         /* mesh left */

         svector[ 10 ].vx = ( svector[ 0 ].vx + svector[ 20 ].vx ) >> 1;
         svector[ 10 ].vy = ( svector[ 0 ].vy + svector[ 20 ].vy ) >> 1;
         svector[ 10 ].vz = ( svector[ 0 ].vz + svector[ 20 ].vz ) >> 1;

         svector[ 5 ].vx = ( svector[ 0 ].vx + svector[ 10 ].vx ) >> 1;
         svector[ 5 ].vy = ( svector[ 0 ].vy + svector[ 10 ].vy ) >> 1;
         svector[ 5 ].vz = ( svector[ 0 ].vz + svector[ 10 ].vz ) >> 1;

         svector[ 15 ].vx = ( svector[ 10 ].vx + svector[ 20 ].vx ) >> 1;
         svector[ 15 ].vy = ( svector[ 10 ].vy + svector[ 20 ].vy ) >> 1;
         svector[ 15 ].vz = ( svector[ 10 ].vz + svector[ 20 ].vz ) >> 1;

         /* mesh right */

         svector[ 14 ].vx = ( svector[ 4 ].vx + svector[ 24 ].vx ) >> 1;
         svector[ 14 ].vy = ( svector[ 4 ].vy + svector[ 24 ].vy ) >> 1;
         svector[ 14 ].vz = ( svector[ 4 ].vz + svector[ 24 ].vz ) >> 1;

         svector[ 9 ].vx = ( svector[ 4 ].vx + svector[ 14 ].vx ) >> 1;
         svector[ 9 ].vy = ( svector[ 4 ].vy + svector[ 14 ].vy ) >> 1;
         svector[ 9 ].vz = ( svector[ 4 ].vz + svector[ 14 ].vz ) >> 1;

         svector[ 19 ].vx = ( svector[ 14 ].vx + svector[ 24 ].vx ) >> 1;
         svector[ 19 ].vy = ( svector[ 14 ].vy + svector[ 24 ].vy ) >> 1;
         svector[ 19 ].vz = ( svector[ 14 ].vz + svector[ 24 ].vz ) >> 1;

         /* mesh bottom */

         svector[ 22 ].vx = ( svector[ 20 ].vx + svector[ 24 ].vx ) >> 1;
         svector[ 22 ].vy = ( svector[ 20 ].vy + svector[ 24 ].vy ) >> 1;
         svector[ 22 ].vz = ( svector[ 20 ].vz + svector[ 24 ].vz ) >> 1;

         svector[ 21 ].vx = ( svector[ 20 ].vx + svector[ 22 ].vx ) >> 1;
         svector[ 21 ].vy = ( svector[ 20 ].vy + svector[ 22 ].vy ) >> 1;
         svector[ 21 ].vz = ( svector[ 20 ].vz + svector[ 22 ].vz ) >> 1;

         svector[ 23 ].vx = ( svector[ 22 ].vx + svector[ 24 ].vx ) >> 1;
         svector[ 23 ].vy = ( svector[ 22 ].vy + svector[ 24 ].vy ) >> 1;
         svector[ 23 ].vz = ( svector[ 22 ].vz + svector[ 24 ].vz ) >> 1;

         /* mesh centre */

         svector[ 12 ].vx = ( svector[ 10 ].vx + svector[ 14 ].vx ) >> 1;
         svector[ 12 ].vy = ( svector[ 10 ].vy + svector[ 14 ].vy ) >> 1;
         svector[ 12 ].vz = ( svector[ 10 ].vz + svector[ 14 ].vz ) >> 1;

         svector[ 7 ].vx = ( svector[ 2 ].vx + svector[ 12 ].vx ) >> 1;
         svector[ 7 ].vy = ( svector[ 2 ].vy + svector[ 12 ].vy ) >> 1;
         svector[ 7 ].vz = ( svector[ 2 ].vz + svector[ 12 ].vz ) >> 1;

         svector[ 11 ].vx = ( svector[ 10 ].vx + svector[ 12 ].vx ) >> 1;
         svector[ 11 ].vy = ( svector[ 10 ].vy + svector[ 12 ].vy ) >> 1;
         svector[ 11 ].vz = ( svector[ 10 ].vz + svector[ 12 ].vz ) >> 1;

         svector[ 13 ].vx = ( svector[ 12 ].vx + svector[ 14 ].vx ) >> 1;
         svector[ 13 ].vy = ( svector[ 12 ].vy + svector[ 14 ].vy ) >> 1;
         svector[ 13 ].vz = ( svector[ 12 ].vz + svector[ 14 ].vz ) >> 1;

         svector[ 17 ].vx = ( svector[ 12 ].vx + svector[ 22 ].vx ) >> 1;
         svector[ 17 ].vy = ( svector[ 12 ].vy + svector[ 22 ].vy ) >> 1;
         svector[ 17 ].vz = ( svector[ 12 ].vz + svector[ 22 ].vz ) >> 1;

         svector[ 6 ].vx = ( svector[ 5 ].vx + svector[ 7 ].vx ) >> 1;
         svector[ 6 ].vy = ( svector[ 5 ].vy + svector[ 7 ].vy ) >> 1;
         svector[ 6 ].vz = ( svector[ 5 ].vz + svector[ 7 ].vz ) >> 1;

         svector[ 8 ].vx = ( svector[ 7 ].vx + svector[ 9 ].vx ) >> 1;
         svector[ 8 ].vy = ( svector[ 7 ].vy + svector[ 9 ].vy ) >> 1;
         svector[ 8 ].vz = ( svector[ 7 ].vz + svector[ 9 ].vz ) >> 1;

         svector[ 16 ].vx = ( svector[ 15 ].vx + svector[ 17 ].vx ) >> 1;
         svector[ 16 ].vy = ( svector[ 15 ].vy + svector[ 17 ].vy ) >> 1;
         svector[ 16 ].vz = ( svector[ 15 ].vz + svector[ 17 ].vz ) >> 1;

         svector[ 18 ].vx = ( svector[ 17 ].vx + svector[ 19 ].vx ) >> 1;
         svector[ 18 ].vy = ( svector[ 17 ].vy + svector[ 19 ].vy ) >> 1;
         svector[ 18 ].vz = ( svector[ 17 ].vz + svector[ 19 ].vz ) >> 1;

         RotTransPersN( svector, screen, depth, flag, 25);


         /* polygon 0 */
         if ( !( ( flag[ 0 ] | flag[ 1 ] | flag[ 5 ] | flag[ 6 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 0 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 0 ];
            //ft4->xy1 = screen[ 1 ];
            //ft4->xy2 = screen[ 5 ];
            //ft4->xy3 = screen[ 6 ];
            ft4->xy0.vx = screen[ 0 ].vx;
            ft4->xy0.vy = screen[ 0 ].vy;
            ft4->xy1.vx = screen[ 1 ].vx;
            ft4->xy1.vy = screen[ 1 ].vy;
            ft4->xy2.vx = screen[ 5 ].vx;
            ft4->xy2.vy = screen[ 5 ].vy;
            ft4->xy3.vx = screen[ 6 ].vx;
            ft4->xy3.vy = screen[ 6 ].vy;
            ft4->xy0.vz = depth[ 0 ];
            ft4->xy1.vz = depth[ 1 ];
            ft4->xy2.vz = depth[ 5 ];
            ft4->xy3.vz = depth[ 6 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 0 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 1 */

         if (!( ( flag[ 1 ] | flag[ 2 ] | flag[ 6 ] | flag[ 7 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 1 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;


            //ft4->xy0 = screen[ 1 ];
            //ft4->xy1 = screen[ 2 ];
            //ft4->xy2 = screen[ 6 ];
            //ft4->xy3 = screen[ 7 ];
            ft4->xy0.vx = screen[ 1 ].vx;
            ft4->xy0.vy = screen[ 1 ].vy;
            ft4->xy1.vx = screen[ 2 ].vx;
            ft4->xy1.vy = screen[ 2 ].vy;
            ft4->xy2.vx = screen[ 6 ].vx;
            ft4->xy2.vy = screen[ 6 ].vy;
            ft4->xy3.vx = screen[ 7 ].vx;
            ft4->xy3.vy = screen[ 7 ].vy;
            ft4->xy0.vz = depth[ 1 ];
            ft4->xy1.vz = depth[ 2 ];
            ft4->xy2.vz = depth[ 6 ];
            ft4->xy3.vz = depth[ 7 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 1 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 2 */

         if (!( ( flag[ 2 ] | flag[ 3 ] | flag[ 7 ] | flag[ 8 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 2 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 2 ];
            //ft4->xy1 = screen[ 3 ];
            //ft4->xy2 = screen[ 7 ];
            //ft4->xy3 = screen[ 8 ];
            ft4->xy0.vx = screen[ 2 ].vx;
            ft4->xy0.vy = screen[ 2 ].vy;
            ft4->xy1.vx = screen[ 3 ].vx;
            ft4->xy1.vy = screen[ 3 ].vy;
            ft4->xy2.vx = screen[ 7 ].vx;
            ft4->xy2.vy = screen[ 7 ].vy;
            ft4->xy3.vx = screen[ 8 ].vx;
            ft4->xy3.vy = screen[ 8 ].vy;
            ft4->xy0.vz = depth[ 2 ];
            ft4->xy1.vz = depth[ 3 ];
            ft4->xy2.vz = depth[ 7 ];
            ft4->xy3.vz = depth[ 8 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 2 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 3 */

         if (!( ( flag[ 3 ] | flag[ 4 ] | flag[ 8 ] | flag[ 9 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 3 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 3 ];
            //ft4->xy1 = screen[ 4 ];
            //ft4->xy2 = screen[ 8 ];
            //ft4->xy3 = screen[ 9 ];
            ft4->xy0.vx = screen[ 3 ].vx;
            ft4->xy0.vy = screen[ 3 ].vy;
            ft4->xy1.vx = screen[ 4 ].vx;
            ft4->xy1.vy = screen[ 4 ].vy;
            ft4->xy2.vx = screen[ 8 ].vx;
            ft4->xy2.vy = screen[ 8 ].vy;
            ft4->xy3.vx = screen[ 9 ].vx;
            ft4->xy3.vy = screen[ 9 ].vy;
            ft4->xy0.vz = depth[ 3 ];
            ft4->xy1.vz = depth[ 4 ];
            ft4->xy2.vz = depth[ 8 ];
            ft4->xy3.vz = depth[ 9 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 3 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 4 */

         if (!( ( flag[ 5 ] | flag[ 6 ] | flag[ 10 ] | flag[ 11 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 4 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 5 ];
            //ft4->xy1 = screen[ 6 ];
            //ft4->xy2 = screen[ 10 ];
            //ft4->xy3 = screen[ 11 ];
            ft4->xy0.vx = screen[ 5 ].vx;
            ft4->xy0.vy = screen[ 5 ].vy;
            ft4->xy1.vx = screen[ 6 ].vx;
            ft4->xy1.vy = screen[ 6 ].vy;
            ft4->xy2.vx = screen[ 10 ].vx;
            ft4->xy2.vy = screen[ 10 ].vy;
            ft4->xy3.vx = screen[ 11 ].vx;
            ft4->xy3.vy = screen[ 11 ].vy;
            ft4->xy0.vz = depth[ 5 ];
            ft4->xy1.vz = depth[ 6 ];
            ft4->xy2.vz = depth[ 10 ];
            ft4->xy3.vz = depth[ 11 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 5 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 5 */

         if (!( ( flag[ 6 ] | flag[ 7 ] | flag[ 11 ] | flag[ 12 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 5 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 6 ];
            //ft4->xy1 = screen[ 7 ];
            //ft4->xy2 = screen[ 11 ];
            //ft4->xy3 = screen[ 12 ];
            ft4->xy0.vx = screen[ 6 ].vx;
            ft4->xy0.vy = screen[ 6 ].vy;
            ft4->xy1.vx = screen[ 7 ].vx;
            ft4->xy1.vy = screen[ 7 ].vy;
            ft4->xy2.vx = screen[ 11 ].vx;
            ft4->xy2.vy = screen[ 11 ].vy;
            ft4->xy3.vx = screen[ 12 ].vx;
            ft4->xy3.vy = screen[ 12 ].vy;
            ft4->xy0.vz = depth[ 6 ];
            ft4->xy1.vz = depth[ 7 ];
            ft4->xy2.vz = depth[ 11 ];
            ft4->xy3.vz = depth[ 12 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 6 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 6 */

         if (!( ( flag[ 7 ] | flag[ 8 ] | flag[ 12 ] | flag[ 13 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 6 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 7 ];
            //ft4->xy1 = screen[ 8 ];
            //ft4->xy2 = screen[ 12 ];
            //ft4->xy3 = screen[ 13 ];
            ft4->xy0.vx = screen[ 7 ].vx;
            ft4->xy0.vy = screen[ 7 ].vy;
            ft4->xy1.vx = screen[ 8 ].vx;
            ft4->xy1.vy = screen[ 8 ].vy;
            ft4->xy2.vx = screen[ 12 ].vx;
            ft4->xy2.vy = screen[ 12 ].vy;
            ft4->xy3.vx = screen[ 13 ].vx;
            ft4->xy3.vy = screen[ 13 ].vy;
            ft4->xy0.vz = depth[ 7 ];
            ft4->xy1.vz = depth[ 8 ];
            ft4->xy2.vz = depth[ 12 ];
            ft4->xy3.vz = depth[ 13 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 7 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 7 */

         if (!( ( flag[ 8 ] | flag[ 9 ] | flag[ 13 ] | flag[ 14 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 7 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 8 ];
            //ft4->xy1 = screen[ 9 ];
            //ft4->xy2 = screen[ 13 ];
            //ft4->xy3 = screen[ 14 ];
            ft4->xy0.vx = screen[ 8 ].vx;
            ft4->xy0.vy = screen[ 8 ].vy;
            ft4->xy1.vx = screen[ 9 ].vx;
            ft4->xy1.vy = screen[ 9 ].vy;
            ft4->xy2.vx = screen[ 13 ].vx;
            ft4->xy2.vy = screen[ 13 ].vy;
            ft4->xy3.vx = screen[ 14 ].vx;
            ft4->xy3.vy = screen[ 14 ].vy;
            ft4->xy0.vz = depth[ 8 ];
            ft4->xy1.vz = depth[ 9 ];
            ft4->xy2.vz = depth[ 13 ];
            ft4->xy3.vz = depth[ 14 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 8 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 8 */

         if (!( ( flag[ 10 ] | flag[ 11 ] | flag[ 15 ] | flag[ 16 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 8 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 10 ];
            //ft4->xy1 = screen[ 11 ];
            //ft4->xy2 = screen[ 15 ];
            //ft4->xy3 = screen[ 16 ];
            ft4->xy0.vx = screen[ 10 ].vx;
            ft4->xy0.vy = screen[ 10 ].vy;
            ft4->xy1.vx = screen[ 11 ].vx;
            ft4->xy1.vy = screen[ 11 ].vy;
            ft4->xy2.vx = screen[ 15 ].vx;
            ft4->xy2.vy = screen[ 15 ].vy;
            ft4->xy3.vx = screen[ 16 ].vx;
            ft4->xy3.vy = screen[ 16 ].vy;
            ft4->xy0.vz = depth[ 10 ];
            ft4->xy1.vz = depth[ 11 ];
            ft4->xy2.vz = depth[ 15 ];
            ft4->xy3.vz = depth[ 16 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 10 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 9 */

         if (!( ( flag[ 11 ] | flag[ 12 ] | flag[ 16 ] | flag[ 17 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 9 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 11 ];
            //ft4->xy1 = screen[ 12 ];
            //ft4->xy2 = screen[ 16 ];
            //ft4->xy3 = screen[ 17 ];
            ft4->xy0.vx = screen[ 11 ].vx;
            ft4->xy0.vy = screen[ 11 ].vy;
            ft4->xy1.vx = screen[ 12 ].vx;
            ft4->xy1.vy = screen[ 12 ].vy;
            ft4->xy2.vx = screen[ 16 ].vx;
            ft4->xy2.vy = screen[ 16 ].vy;
            ft4->xy3.vx = screen[ 17 ].vx;
            ft4->xy3.vy = screen[ 17 ].vy;
            ft4->xy0.vz = depth[ 11 ];
            ft4->xy1.vz = depth[ 12 ];
            ft4->xy2.vz = depth[ 16 ];
            ft4->xy3.vz = depth[ 17 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 11 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 10 */

         if (!( ( flag[ 12 ] | flag[ 13 ] | flag[ 17 ] | flag[ 18 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 10 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 12 ];
            //ft4->xy1 = screen[ 13 ];
            //ft4->xy2 = screen[ 17 ];
            //ft4->xy3 = screen[ 18 ];
            ft4->xy0.vx = screen[ 12 ].vx;
            ft4->xy0.vy = screen[ 12 ].vy;
            ft4->xy1.vx = screen[ 13 ].vx;
            ft4->xy1.vy = screen[ 13 ].vy;
            ft4->xy2.vx = screen[ 17 ].vx;
            ft4->xy2.vy = screen[ 17 ].vy;
            ft4->xy3.vx = screen[ 18 ].vx;
            ft4->xy3.vy = screen[ 18 ].vy;
            ft4->xy0.vz = depth[ 12 ];
            ft4->xy1.vz = depth[ 13 ];
            ft4->xy2.vz = depth[ 17 ];
            ft4->xy3.vz = depth[ 18 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 12 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 11 */

         if (!( ( flag[ 13 ] | flag[ 14 ] | flag[ 18 ] | flag[ 19 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 11 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 13 ];
            //ft4->xy1 = screen[ 14 ];
            //ft4->xy2 = screen[ 18 ];
            //ft4->xy3 = screen[ 19 ];
            ft4->xy0.vx = screen[ 13 ].vx;
            ft4->xy0.vy = screen[ 13 ].vy;
            ft4->xy1.vx = screen[ 14 ].vx;
            ft4->xy1.vy = screen[ 14 ].vy;
            ft4->xy2.vx = screen[ 18 ].vx;
            ft4->xy2.vy = screen[ 18 ].vy;
            ft4->xy3.vx = screen[ 19 ].vx;
            ft4->xy3.vy = screen[ 19 ].vy;
            ft4->xy0.vz = depth[ 13 ];
            ft4->xy1.vz = depth[ 14 ];
            ft4->xy2.vz = depth[ 18 ];
            ft4->xy3.vz = depth[ 19 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 13 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 12 */

         if (!( ( flag[ 15 ] | flag[ 16 ] | flag[ 20 ] | flag[ 21 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 12 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 15 ];
            //ft4->xy1 = screen[ 16 ];
            //ft4->xy2 = screen[ 20 ];
            //ft4->xy3 = screen[ 21 ];
            ft4->xy0.vx = screen[ 15 ].vx;
            ft4->xy0.vy = screen[ 15 ].vy;
            ft4->xy1.vx = screen[ 16 ].vx;
            ft4->xy1.vy = screen[ 16 ].vy;
            ft4->xy2.vx = screen[ 20 ].vx;
            ft4->xy2.vy = screen[ 20 ].vy;
            ft4->xy3.vx = screen[ 21 ].vx;
            ft4->xy3.vy = screen[ 21 ].vy;
            ft4->xy0.vz = depth[ 15 ];
            ft4->xy1.vz = depth[ 16 ];
            ft4->xy2.vz = depth[ 20 ];
            ft4->xy3.vz = depth[ 21 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 15 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 13 */

         if (!( ( flag[ 16 ] | flag[ 17 ] | flag[ 21 ] | flag[ 22 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 13 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 16 ];
            //ft4->xy1 = screen[ 17 ];
            //ft4->xy2 = screen[ 21 ];
            //ft4->xy3 = screen[ 22 ];
            ft4->xy0.vx = screen[ 16 ].vx;
            ft4->xy0.vy = screen[ 16 ].vy;
            ft4->xy1.vx = screen[ 17 ].vx;
            ft4->xy1.vy = screen[ 17 ].vy;
            ft4->xy2.vx = screen[ 21 ].vx;
            ft4->xy2.vy = screen[ 21 ].vy;
            ft4->xy3.vx = screen[ 22 ].vx;
            ft4->xy3.vy = screen[ 22 ].vy;
            ft4->xy0.vz = depth[ 16 ];
            ft4->xy1.vz = depth[ 17 ];
            ft4->xy2.vz = depth[ 21 ];
            ft4->xy3.vz = depth[ 22 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 16 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 14 */

         if (!( ( flag[ 17 ] | flag[ 18 ] | flag[ 22 ] | flag[ 23 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 14 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 17 ];
            //ft4->xy1 = screen[ 18 ];
            //ft4->xy2 = screen[ 22 ];
            //ft4->xy3 = screen[ 23 ];
            ft4->xy0.vx = screen[ 17 ].vx;
            ft4->xy0.vy = screen[ 17 ].vy;
            ft4->xy1.vx = screen[ 18 ].vx;
            ft4->xy1.vy = screen[ 18 ].vy;
            ft4->xy2.vx = screen[ 22 ].vx;
            ft4->xy2.vy = screen[ 22 ].vy;
            ft4->xy3.vx = screen[ 23 ].vx;
            ft4->xy3.vy = screen[ 23 ].vy;
            ft4->xy0.vz = depth[ 17 ];
            ft4->xy1.vz = depth[ 18 ];
            ft4->xy2.vz = depth[ 22 ];
            ft4->xy3.vz = depth[ 23 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 17 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         /* polygon 15 */

         if (!( ( flag[ 18 ] | flag[ 19 ] | flag[ 23 ] | flag[ 24 ] ) & ShortClipFlags ) )
         {
            ft4 = &( TrackHi[ CurrentScreen ][ TrackHiCount++ ] );

            SetPolyFT4( ( POLY_FT4* ) ft4 );

            if ( face->flags & FlipTexture )
            {
               surface = &( TrackSurface[ face->texture ][ 1 ] );
            }
            else
            {
               surface = &( TrackSurface[ face->texture ][ 0 ] );
            }

            tile = &( surface->high[ 15 ] );

            ft4->cba = tile->cba;
            ft4->tsb = tile->tsb;

            ft4->u0 = tile->u0;
            ft4->v0 = tile->v0;

            ft4->u1 = tile->u1;
            ft4->v1 = tile->v1;

            ft4->u2 = tile->u2;
            ft4->v2 = tile->v2;

            ft4->u3 = tile->u3;
            ft4->v3 = tile->v3;

            ft4->colour0.r = face->red;
            ft4->colour0.g = face->green;
            ft4->colour0.b = face->blue;

            //ft4->xy0 = screen[ 18 ];
            //ft4->xy1 = screen[ 19 ];
            //ft4->xy2 = screen[ 23 ];
            //ft4->xy3 = screen[ 24 ];
            ft4->xy0.vx = screen[ 18 ].vx;
            ft4->xy0.vy = screen[ 18 ].vy;
            ft4->xy1.vx = screen[ 19 ].vx;
            ft4->xy1.vy = screen[ 19 ].vy;
            ft4->xy2.vx = screen[ 23 ].vx;
            ft4->xy2.vy = screen[ 23 ].vy;
            ft4->xy3.vx = screen[ 24 ].vx;
            ft4->xy3.vy = screen[ 24 ].vy;
            ft4->xy0.vz = depth[ 18 ];
            ft4->xy1.vz = depth[ 19 ];
            ft4->xy2.vz = depth[ 23 ];
            ft4->xy3.vz = depth[ 24 ];
            ft4->xy0.pad = 1;
            ft4->xy1.pad = 1;
            ft4->xy2.pad = 1;
            ft4->xy3.pad = 1;

            AddPrim( OT[ CurrentScreen ] + (( depth[ 18 ] >> DepthShift) - DepthMod  ), ( ulong* ) ft4 );
         }

         face++;
      }

      section = section->nextSection.ptr;
   }



   return( section );
}

void Test16Bit(long *Test16bitX, long *Test16bitY, long *Test16bitZ)
{
   if(*Test16bitX > 32767)
      *Test16bitX = 32767;
   else if(*Test16bitX < -32767)
      *Test16bitX = -32767;

   if(*Test16bitY > 32767)
      *Test16bitY = 32767;
   else if(*Test16bitY < -32767)
      *Test16bitY = -32767;

   if(*Test16bitZ > 32767)
      *Test16bitZ = 32767;
   else if(*Test16bitZ < -32767)
      *Test16bitZ = -32767;
}




