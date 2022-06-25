
/* (C) Psygnosis 1994. By Jason Carl Denton
*/

#include "standard.h"
#include <string.h>

#include "malloc.h"
#include "tim.h"
#include <stdio.h>
#include "object.h"
#include "file.h"
#include "track.h"
#include "ships.h"
#include "weapon.h"
#include "control.h"
#include "droid.h"
#include "camera.h"
#include "sound.h"
#include "global.h"
#include "error.h"


long FileLength( char* name )
{
   FILE   *fd;
   long  length;

/* Open file */
   char *cleanname;

   cleanname = (char *)strchr(name,':');
   if (cleanname != NULL)
      {
      cleanname++;
      }
   else
      {
      cleanname = name;
      }


   fd = fopen( cleanname, "rb" );
   if ( fd == NULL )
   {
      return( NULL );
   }

/* Seek to end of file to get length */

      fseek(fd,0,SEEK_END);
   length = ftell( fd);
    rewind(fd);

/* Close file */

   fclose( fd );

/* Return file length */

   return( length );
}

long LoadFile( char* name, char* buffer )
{
   FILE *fd;
   long  length;

/* Open file */
   char *cleanname;

   cleanname = (char *)strchr(name,':');
   if (cleanname != NULL)
      {
      cleanname++;
      }
   else
      {
      cleanname = name;
      }


   fd = fopen( cleanname, "rb" );
   if ( fd == NULL )
   {
      printf( "File.c:LoadFile(): Bad file name %s\n", name );
      return( 0 );
   }

/* Seek to end of file to get length */

      fseek(fd,0,SEEK_END);
   length = ftell( fd);
/* Seek back to the start */
    rewind(fd);

/* Read file into buffer */

   if ( fread( buffer, 1, length, fd ) != length )
   {
      printf( "File.c:LoadFile(): Failed to load file %s\n", name );
      fclose( fd );
      return( 0 );
   }

/* Close file */

   fclose( fd );

/* Return file length */

   return( length );
}
