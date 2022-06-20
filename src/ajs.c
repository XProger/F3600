//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Some 'PlayStation on the PC' functions
// Andy Satterthwaite, March / April 1995
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// OK these are the finished functions I've written. Hope it all works ...
// There may be problems in LoadImage, StoreImage and MoveImage 'cause I haven't tested them
// they look ok though.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// typedefs to get includes working under watcom
#include "standard.h"

#define _SIZE_T_DEFINED_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protos.h"

#define addPrim(ot, p)  setaddr(p, getaddr(ot)), setaddr(ot, p)

char winLoseIn=0;

char winPic[35840];
char losePic[35840];
extern char inmenu,Reallyinmenu;

//#define   LOADTEXTURES   1              // standard load stuff (LoadImage4, LoadImage8 etc.);
//#define   LOADAS24BIT 1                 // if you want to load vram as 1 byte red, 1 byte green & 1 byte blue (requires 6 mb vram)
#define  BLANKFUNCTIONS 1              // if loading vram as pcx

int      g_mask;              // global display mask variable set in SetDispMask


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Most of the in function comments are taken straight from the manuals - so blame them if they're
// crap !
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// PLAYSTATION FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~
// functions originally declared in libgpu.h

/*

  DISPENV      *GetDispEnv(DISPENV *env);    // NOT USED IN RIDGERACER

   DRAWENV     *GetDrawEnv(DRAWENV *env);    // NOT USED IN RIDGERACER

     DISPENV      *SetDefDispEnv(DISPENV *disp, int x, int y, int w, int h);

      DRAWENV     *SetDefDrawEnv(DRAWENV *draw, long x, long y, long w, long h);

        void      SetDrawEnv(DR_ENV *dr_env, DRAWENV *env);

         void     SetDrawMode(DR_MODE *dr_mode, int dfe, int dtd, unsigned short tpage, RECT *tw);

           void      LoadImage(RECT *recp, unsigned long *p);

            void     MoveImage(RECT *rect, int x, int y);   // NOT USED IN RIDGERACER

              void      StoreImage(RECT   *recp, unsigned long *p);

               long     DrawSync(long  mode);

                 void      ResetGraph(int mode);

                  void     SetGraphDebug(int level);

                    void      InterpolByte(u_char*, u_char*, long, u_char*);

*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int      DrawSync(int   mode)
{
   // function used for parallel actions on PSX
   // defined so games compile on PC
   return(1);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FUNCTIONS WHICH JUST CALL THEIR EQUIVALENT MACROS

void  AddPrim(P_TAG *ordt, u_long *p)
{
   addPrim(ordt, p);
}

u_short GetTPage(int tp, int abr, int x, int y)
{
   return(getTPage(tp,abr,x,y));
}


void SetLineG2(LINE_G2 *p)
{
   setLineG2(p);
}

void SetPolyF3(POLY_F3  *p)
{
   setPolyF3(p);
}

void SetPolyFT3(POLY_FT3 *p)
{
   setPolyFT3(p);
}

void SetPolyG3(POLY_G3  *p)
{
   setPolyG3(p);
}

void SetPolyF4(POLY_F4  *p)
{
   setPolyF4(p);
}

void SetPolyFT4(POLY_FT4 *p)
{
   setPolyFT4(p);
}

void SetPolyG4(POLY_G4  *p)
{
   setPolyG4(p);
}

void SetPolyGT4(POLY_GT4 *p)
{
   setPolyGT4(p);
}

void SetSemiTrans(P_TAG *p, long abe)
{
   setSemiTrans(p,abe);
}

void SetShadeTex(P_TAG *p, long tge)
{
   setShadeTex(p,tge);
}



void SetSprt(SPRT    *p)
{
   setSprt(p);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void           SetDispMask(int mask)
{
   // Puts display mask into the status specified by mask. The following values can be used for mask
   // 0 - Not displayed on screen
   // 1 - Displayed on screen

   g_mask=mask;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DISPENV  *SetDefDispEnv(DISPENV *disp, int x, int y, int w, int h)
{
   // Sets display environment structure members using the point on the top left of the display
   // area and width and height of the display area (values set as in table below)
   //
   // MASK        CONTENT           VALUE
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // disp        Display Area      (x,y,w,h)
   // ds          Display Offset    (0,0)
   // isinter     Interlace Flag    0
   // isrgb       24bit Mode Flag   0

   disp->disp.x = x;
   disp->disp.y = y;
   disp->disp.w = w;
   disp->disp.h = h;

   disp->screen.x = 0;
   disp->screen.y = 0;
   disp->screen.w = 0;
   disp->screen.h = 0;

   disp->isinter = 0;
   disp->isrgb24 = 0;

   return(disp);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DRAWENV  *SetDefDrawEnv(DRAWENV *draw, int x, int y, int w, int h)
{
   // Sets standard drawing environment structure members judging from the point on the left top
   // of the drawing area, and width and height of the drawing area (values set as in table below)
   //
   // MEMBER      CONTENT                                       VALUE
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // clip        Drawing area                                  (x,y,w,h)
   // ofs[2]      Drawing offset                                (x,y)
   // tw          Texture Window                                (0,0,0,0)
   // tpage[2]    Texture page (tp, abr, tx, ty)                (0,0,640,0)
   // dtd         Dither processing flag                        1 (ON)
   // dfe         Permission flag for drawing on display area   0 (clear: OFF)
   // isbg        Draw area clear flag                          0 (clear: OFF)
   // r0,g0,b0    Background Colour                             1 (0,0,0)

   draw->clip.x = x;
   draw->clip.y = y;
   draw->clip.w = w;
   draw->clip.h = h;

   draw->ofs[0] = x;
   draw->ofs[1] = y;

   draw->tw.x = 0;
   draw->tw.y = 0;
   draw->tw.w = 0;
   draw->tw.h = 0;

   draw->tpage = 10; /* (0,0,640,0) */
   draw->dtd = 1;
   draw->dfe = 0;
   draw->isbg = 0;

   draw->r0 = 0;
   draw->g0 = 0;
   draw->b0 = 0;

   // ignored the dr_env bit 'cause we don't know what it does

   return(draw);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void     SetDrawMode(DR_MODE *dr_mode, int dfe, int dtd, int tpage, GRECT *tw)
{
   // Initialises the draw mode primitive.
   // The format below was designed by me (ajs) if you don't like it - design your own !

   // FORMAT FOR DR_MODE.CODE ARRAY
   // 1 byte  : dfe
   // 1 byte  : dtd
   // 2 bytes : tpage
   // 1 u_long: *tw

   dr_mode->code[1] = (u_char)dfe<<24;
   dr_mode->code[1] += (u_char)dtd<<16;
   dr_mode->code[1] += tpage;
   dr_mode->code[0] = 0x90;         // set first byte (code byte) to 0x90
   dr_mode->code[0]=dr_mode->code[0]<<24;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void     StoreImage(GRECT   *recp, unsigned long *p)
{
   // Transfers the rectangular area in the frame bufffer specified by recp to the contiguous area
   // in main memory at address p

#ifdef ajs
   u_short  count;
   u_long   *source;
   u_short  width;
   u_long   val;

   source=video_start_addr+(recp->x*2)+(recp->y*VRAM_WIDTH);
   width=recp->w*2;

   for (count=0;count<recp->h;count++)
   {
      val=count*VRAM_WIDTH;
      memcpy(p+val,source+val,recp->w);
   }
#endif
}

void SetBlockFill(BLK_FILL *p)
{
   setBlockFill(p);
}

void  SetPolyGT3(POLY_GT3 *p)
{
   setPolyGT3(p);
}

//*****************************************************************

void LoadClut4(GRECT  *rect, u_short *clutptr, CLUT *cluts)
{
   int loop;
   u_short *ptr;

   ptr = (u_short *)cluts;
   for (loop =0;loop < 16 ;loop++)
   {
      *ptr++ = *clutptr++;
   }
}

void LoadClut8(GRECT  *rect, u_short *clutptr, CLUT *cluts)
{
   int loop;
   u_short *ptr;

   ptr = (u_short *)cluts;
   for (loop =0;loop < 256 ;loop++)
   {
      *ptr++ = *clutptr++;
   }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef LOADTEXTURES

int      LoadImage8(RECT   *recp, unsigned long *p, CLUT *cluts)
{
   /***** NOTE: THIS SHOULD WORK APART FROM DIFFERENT SIZE TEXTURE PAGE COORDINATE *****/
   char  *dest;
   char  *source;

   char  tpage_start, tpage_end, tpage;
   char  vert_split;

   short src_start_off;
   short x,y;
   short width,height;
   short xs, xe, ys, ye;
   char red, green, blue;
   char val;

   char *source_byte;
   short line;
   char  *ptr;

   char  used[16];
   int   loop_used;

   printf("!!!!!! LOAD IMAGE 8 !!!!!!\n");
#define  TPAGE_WIDTH 64 // this is in WORDS, 'cause that's how the recp coord come in (I THINK!)

   // START OF CODE

   ptr=VRam;         // pointer to start of our (steve's) VRAM

   x=recp->x;              // co-ordinates for original PS Vram
   y=recp->y;
   width=recp->w;
   height=recp->h;

   if (y>255)              // check to see if texture is split over texture pages vertically
      vert_split=0;        // i.e top half in texture pages 0-15, bottom half in corresponding
   else                    // texture pages 16-32 : vert_split set to 1 if this is the case
      if ((y+height)>256)  // 0 otherwise
         vert_split=1;
      else
         vert_split=0;

      if (!vert_split)        // we're no split over texture pages vertically (easy case)
      {
         tpage_start=x/TPAGE_WIDTH;
         tpage_end=(x+width-1)/TPAGE_WIDTH;
         if (y>255)
         {
            tpage_start+=16;
            tpage_end+=16;
            y -= 256;
         }

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=y;line<y+height;line++)
            {
               dest=ptr+(tpage*65536)+(xs*2)+(line*256);          // destination in 4 bit co-ords
               source=(char*)p+src_start_off+((line-y)*(width*2));      // source in 16 bit co-ords  Width must be in bytes
               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)
               {
                  val = (*source_byte) & 0xff;        /*low 8 bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);

                  //             *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                  //                                (char)(cluts[val].green)<<3,
                  //                                (char)(cluts[val].blue)<<3);
               }
            }
            src_start_off += ((xe-xs) * 2);                          //64 coords is 128 bytes!
         }
      }
      else        // split over texture pages vertically (awkward bastards!)
      {
         // FIRST STEP DO THE 0 TO 15 TOP HALF BIT THING WHATIST

         tpage_start=x/TPAGE_WIDTH;
         tpage_end=(x+width-1)/TPAGE_WIDTH;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=y;line<256;line++)
            {
               dest=ptr+(tpage*65536)+(xs * 2)+(line*256);
               source=(char*)p+src_start_off+((line-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) * 2));source_byte++) //*2 for 16bit pixel values
               {
                  val = (*source_byte) & 0xff;        /*low 8 bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);

                  //             *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                  //                                (char)(cluts[val].green)<<3,
                  //                                (char)(cluts[val].blue)<<3);
               }
            }
            src_start_off += ((xe-xs) *2);
         }

         // NOW DO BOTTOM HALF !!

         tpage_start+=16;
         tpage_end+=16;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=0;line<height-(256-y);line++)               //(256-y) is the amount drawn in the top half
            {
               dest=ptr+(tpage*65536)+(xs * 2)+(line*256);
               source=(char*)p+src_start_off+((line+256-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)
               {
                  val = (*source_byte) & 0xff;        /*low 8 bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);

                  //             *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                  //                                (char)(cluts[val].green)<<3,
                  //                                (char)(cluts[val].blue)<<3);
               }
            }
            src_start_off += ((xe-xs) * 2);
         }
      }
      return(1);
}

//*****************************************************************


int      LoadImage4(RECT   *recp, unsigned long *p, CLUT *cluts)
{
   char  *dest;
   char  *source;

   char  tpage_start, tpage_end, tpage;
   char  vert_split;

   short src_start_off;
   short x,y;
   short width,height;
   short xs, xe, ys, ye;
   char red, green, blue;
   char val;

   char *source_byte;
   short line;
   char  *ptr;

   char  used[16];
   int   loop_used;

#define  TPAGE_WIDTH 64 // this is in WORDS, 'cause that's how the recp coord come in (I THINK!)

   // START OF CODE

   ptr=VRam;         // pointer to start of our (steve's) VRAM

   x=recp->x;              // co-ordinates for original PS Vram
   y=recp->y;
   width=recp->w;
   height=recp->h;

   if (y>255)              // check to see if texture is split over texture pages vertically
      vert_split=0;        // i.e top half in texture pages 0-15, bottom half in corresponding
   else                    // texture pages 16-32 : vert_split set to 1 if this is the case
      if ((y+height)>256)  // 0 otherwise
         vert_split=1;
      else
         vert_split=0;

      if (!vert_split)        // we're no split over texture pages vertically (easy case)
      {
         tpage_start=x/TPAGE_WIDTH;          // get the starting texture page
         tpage_end=(x+width-1)/TPAGE_WIDTH;  // get the end texture page
         if (y>255)
         {
            tpage_start+=16;                 // if in bottom half of VRam
            tpage_end+=16;
            y -= 256;
         }

         src_start_off = 0;                  // the x co-ord of where we are in our source buffer
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;     // starting x co-ord in texture page
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;  // end x co-ord in texture page
            else  xe=TPAGE_WIDTH;

            for (line=y;line<y+height;line++)
            {
               dest=ptr+(tpage*65536)+(xs*4)+(line*256);          // destination in 4 bit co-ords
               source=(char*)p+src_start_off+((line-y)*(width*2));      // source in 16 bit co-ords  Width must be in bytes

               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)   // read in bytes for whole line
               {                                                              // converting from 4 bit to 8 bit on the way
                  val = (*source_byte) & 0xf;         /*low 4 bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);


                  val = ((*source_byte) & 0xf0)>>4;  /*high four bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char) (cluts[val].green)<<3,
                     (char) (cluts[val].blue)<<3);
               }
            }
            src_start_off += ((xe-xs) * 2);                          //64 coords is 128 bytes!  // increment the source start offset
         }
      }
      else        // split over texture pages vertically (awkward bastards!)
      {
         // FIRST STEP DO THE 0 TO 15 TOP HALF BIT THING WHATIST

         tpage_start=x/TPAGE_WIDTH;
         tpage_end=(x+width-1)/TPAGE_WIDTH;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=y;line<256;line++)
            {
               dest=ptr+(tpage*65536)+(xs * 4)+(line*256);
               source=(char*)p+src_start_off+((line-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) * 2));source_byte++) //*2 for 16bit pixel values
               {
                  val = (*source_byte) & 0xf;         /*low 4 bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);
                  val = ((*source_byte) & 0xf0)>>4;  /*high four bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);
               }
            }
            src_start_off += ((xe-xs) *2);
         }

         // NOW DO BOTTOM HALF !!

         tpage_start+=16;
         tpage_end+=16;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=0;line<height-(256-y);line++)               //(256-y) is the amount drawn in the top half
            {
               dest=ptr+(tpage*65536)+(xs * 4)+(line*256);
               source=(char*)p+src_start_off+((line+256-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)
               {
                  val = (*source_byte) & 0xf;         /*low 4 bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);
                  val = ((*source_byte) & 0xf0)>>4;  /*high four bits*/
                  *dest++ = GET_PALLETE((char)(cluts[val].red)<<3,
                     (char)(cluts[val].green)<<3,
                     (char)(cluts[val].blue)<<3);
               }

            }
            src_start_off += ((xe-xs) * 2);
         }
      }
      return(1);
}

#endif      // 16 bit dump stuff
#ifdef   LOADAS24BIT
int      LoadImage8(RECT   *recp, unsigned long *p, CLUT *cluts)
{
   char  *dest;
   char  *source;

   char  tpage_start, tpage_end, tpage;
   char  vert_split;

   short src_start_off;
   short x,y;
   short width,height;
   short xs, xe, ys, ye;
   char red, green, blue;
   char val;

   char *source_byte;
   short line;
   char  *ptr;

   char  used[16];
   int   loop_used;

   printf("!!!!!! LOAD IMAGE 8 !!!!!!\n");
#define  TPAGE_WIDTH 64 // this is in WORDS, 'cause that's how the recp coord come in (I THINK!)

   // START OF CODE

   ptr=VRam;         // pointer to start of our (steve's) VRAM

   x=recp->x;              // co-ordinates for original PS Vram
   y=recp->y;
   width=recp->w;
   height=recp->h;

   if (y>255)              // check to see if texture is split over texture pages vertically
      vert_split=0;        // i.e top half in texture pages 0-15, bottom half in corresponding
   else                    // texture pages 16-32 : vert_split set to 1 if this is the case
      if ((y+height)>256)  // 0 otherwise
         vert_split=1;
      else
         vert_split=0;

      if (!vert_split)        // we're no split over texture pages vertically (easy case)
      {
         tpage_start=x/TPAGE_WIDTH;
         tpage_end=(x+width-1)/TPAGE_WIDTH;
         if (y>255)
         {
            tpage_start+=16;
            tpage_end+=16;
            y -= 256;
         }

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=y;line<y+height;line++)
            {
               dest=(char*)(ptr+((long)tpage*196608)+((long)xs*6)+((long)line*768));                  // 65536 = size of tpage, 2 = number of pixels per word of source, 256 = width of VRAM
               //          dest=ptr+(tpage*65536)+(xs*2)+(line*256);                // 65536 = size of tpage, 2 = number of pixels per word of source, 256 = width of VRAM
               source=(char*)p+src_start_off+((line-y)*(width*2));      // source in 16 bit co-ords  Width must be in bytes
               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)
               {
                  val = (*source_byte) & 0xff;        /*low 8 bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3,
                     *dest++ = (char)(cluts[val].blue)<<3;
               }
            }
            src_start_off += ((xe-xs) * 2);                          //64 coords is 128 bytes!
         }
      }
      else        // split over texture pages vertically (awkward bastards!)
      {
         // FIRST STEP DO THE 0 TO 15 TOP HALF BIT THING WHATIST

         tpage_start=x/TPAGE_WIDTH;
         tpage_end=(x+width-1)/TPAGE_WIDTH;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=y;line<256;line++)
            {
               dest=(char*)(ptr+((long)tpage*196608)+((long)xs*6)+((long)line*768));                  // 65536 = size of tpage, 2 = number of pixels per word of source, 256 = width of VRAM
               //          dest=ptr+(tpage*65536)+(xs * 2)+(line*256);
               source=(char*)p+src_start_off+((line-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) * 2));source_byte++) //*2 for 16bit pixel values
               {
                  val = (*source_byte) & 0xff;        /*low 8 bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3,
                     *dest++ = (char)(cluts[val].blue)<<3;
               }
            }
            src_start_off += ((xe-xs) *2);
         }

         // NOW DO BOTTOM HALF !!

         tpage_start+=16;
         tpage_end+=16;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=0;line<height-(256-y);line++)               //(256-y) is the amount drawn in the top half
            {
               dest=(char*)(ptr+((long)tpage*196608)+((long)xs*6)+((long)line*768));                  // 65536 = size of tpage, 2 = number of pixels per word of source, 256 = width of VRAM
               //          dest=ptr+(tpage*65536)+(xs * 2)+(line*256);
               source=(char*)p+src_start_off+((line+256-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)
               {
                  val = (*source_byte) & 0xff;        /*low 8 bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3,
                     *dest++ = (char)(cluts[val].blue)<<3;
               }
            }
            src_start_off += ((xe-xs) * 2);
         }
      }
      return(1);
}

//*****************************************************************

int      LoadImage4(RECT   *recp, unsigned long *p, CLUT *cluts)
{
   char  *dest;
   char  *source;

   char  tpage_start, tpage_end, tpage;
   char  vert_split;

   short src_start_off;
   short x,y;
   short width,height;
   short xs, xe, ys, ye;
   char red, green, blue;
   char val;

   char *source_byte;
   short line;
   char  *ptr;

   char  used[16];
   int   loop_used;

#define  TPAGE_WIDTH 64 // this is in WORDS, 'cause that's how the recp coord come in (I THINK!)

   // START OF CODE

   ptr=VRam;         // pointer to start of our (steve's) VRAM

   x=recp->x;              // co-ordinates for original PS Vram
   y=recp->y;
   width=recp->w;
   height=recp->h;

   if (y>255)              // check to see if texture is split over texture pages vertically
      vert_split=0;        // i.e top half in texture pages 0-15, bottom half in corresponding
   else                    // texture pages 16-32 : vert_split set to 1 if this is the case
      if ((y+height)>256)  // 0 otherwise
         vert_split=1;
      else
         vert_split=0;

      if (!vert_split)        // we're no split over texture pages vertically (easy case)
      {
         tpage_start=x/TPAGE_WIDTH;          // get the starting texture page
         tpage_end=(x+width-1)/TPAGE_WIDTH;  // get the end texture page
         if (y>255)
         {
            tpage_start+=16;                 // if in bottom half of VRam
            tpage_end+=16;
            y -= 256;
         }

         src_start_off = 0;                  // the x co-ord of where we are in our source buffer
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;     // starting x co-ord in texture page
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;  // end x co-ord in texture page
            else  xe=TPAGE_WIDTH;

            for (line=y;line<y+height;line++)
            {
               dest=(char*)(ptr+((long)tpage*196608)+((long)xs*12)+((long)line*768));                 // 65536 = size of tpage, 2 = number of pixels per word of source, 256 = width of VRAM
               //          dest=ptr+(tpage*65536)+(xs*4)+(line*256);          // destination in 4 bit co-ords
               source=(char*)p+src_start_off+((line-y)*(width*2));      // source in 16 bit co-ords  Width must be in bytes

               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)   // read in bytes for whole line
               {                                                              // converting from 4 bit to 8 bit on the way
                  val = (*source_byte) & 0xf;         /*low 4 bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3,
                     *dest++ = (char)(cluts[val].blue)<<3;


                  val = ((*source_byte) & 0xf0)>>4;  /*high four bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3;
                  *dest++ = (char)(cluts[val].blue)<<3;
               }
            }
            src_start_off += ((xe-xs) * 2);                          //64 coords is 128 bytes!  // increment the source start offset
         }
      }
      else        // split over texture pages vertically (awkward bastards!)
      {
         // FIRST STEP DO THE 0 TO 15 TOP HALF BIT THING WHATIST

         tpage_start=x/TPAGE_WIDTH;
         tpage_end=(x+width-1)/TPAGE_WIDTH;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=y;line<256;line++)
            {
               dest=(char*)(ptr+((long)tpage*196608)+((long)xs*12)+((long)line*768));                 // 65536 = size of tpage, 2 = number of pixels per word of source, 256 = width of VRAM
               //          dest=ptr+(tpage*65536)+(xs * 4)+(line*256);
               source=(char*)p+src_start_off+((line-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) * 2));source_byte++) //*2 for 16bit pixel values
               {
                  val = (*source_byte) & 0xf;         /*low 4 bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3,
                     *dest++ = (char)(cluts[val].blue)<<3;


                  val = ((*source_byte) & 0xf0)>>4;  /*high four bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3;
                  *dest++ = (char)(cluts[val].blue)<<3;
               }
            }
            src_start_off += ((xe-xs) *2);
         }

         // NOW DO BOTTOM HALF !!

         tpage_start+=16;
         tpage_end+=16;

         src_start_off = 0;
         for (tpage=tpage_start;tpage<=tpage_end;tpage++ )
         {
            if (tpage==tpage_start) xs=x%TPAGE_WIDTH;
            else  xs=0;

            if (tpage==tpage_end)   xe=((x+width-1)%TPAGE_WIDTH)+1;
            else  xe=TPAGE_WIDTH;

            for (line=0;line<height-(256-y);line++)               //(256-y) is the amount drawn in the top half
            {
               dest=(char*)(ptr+((long)tpage*196608)+((long)xs*12)+((long)line*768));                 // 65536 = size of tpage, 2 = number of pixels per word of source, 256 = width of VRAM
               //          dest=ptr+(tpage*65536)+(xs * 4)+(line*256);
               source=(char*)p+src_start_off+((line+256-y)*(width*2));
               for (source_byte = source; source_byte <(source + ((xe-xs) *2));source_byte++)
               {
                  val = (*source_byte) & 0xf;         /*low 4 bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3,
                     *dest++ = (char)(cluts[val].blue)<<3;


                  val = ((*source_byte) & 0xf0)>>4;  /*high four bits*/
                  *dest++ = (char)(cluts[val].red)<<3;
                  *dest++ = (char)(cluts[val].green)<<3;
                  *dest++ = (char)(cluts[val].blue)<<3;
               }
            }
            src_start_off += ((xe-xs) * 2);
         }
      }
      return(1);
}

#endif
#ifdef BLANKFUNCTIONS

int      LoadImage4(GRECT   *recp, unsigned long *p, CLUT *cluts)
{
   return(1);
}

int      LoadImage8(GRECT   *recp, unsigned long *p, CLUT *cluts)
{
   return(1);
}
#endif

void setWinLose(void)
{
   char  *ptr;
   short i,j;

   if(!winLoseIn)
   {
      ptr=LockPilot();

      for (i=64,j=0;i<204;i++,j++) // 140 = pcx height // 7748 before removal of 10 blank tpages
      {
         memcpy(ptr+(i*512),winPic+(j*256),256); // do we really need the offset by 10
      }

      for (i=64,j=0;i<204;i++,j++) // 7704 before removal of 10 blank tpages
      {
         memcpy(ptr+(i*512)+256,losePic+(j*256),256);
      }

      UnlockPilot();
      winLoseIn = 1;
   }
}

void  LoadPilotIntoVram(char *filename)
{
   FILE  *fp;
   char  picfile[256];

   strcpy( picfile, filename);
   strcat( picfile, "win.sjr");
   fp=fopen(picfile,"rb");
   fread(winPic,35840,1,fp);
   fclose(fp);

   strcpy( picfile, filename);
   strcat( picfile, "los.sjr");
   fp=fopen(picfile,"rb");
   fread(losePic,35840,1,fp);
   fclose(fp);
}
