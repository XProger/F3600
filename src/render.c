#define STRICT
#include <stdio.h>
#include <windows.h>
#include <gl/GL.h>
#include "standard.h"
#include "malloc.h"
#include "render.h"
#include "decl.h"

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

extern short         WinWidth;
extern short         WinHeight;

int gWidth, gHeight;

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

#define  SCALEIT(x) (x)
#define  SCALEZ(z) ((z)*zscale)

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

int dbg_G3, dbg_G4, dbg_F3, dbg_F4, dbg_GT3, dbg_GT4, dbg_FT3, dbg_FT4;

short texpage = -1;

extern short trackNum;

extern DynamicHeap* heap;
void Swap(void);
void LoadPilotIntoVram(char* filename);

char globalPilot[255] = { 0 };

#define TEXTURESJR 2621440

typedef struct {
   GLuint Texture;
} TEXTURE;

TEXTURE gTex[500]={0};
char lastfile[256];

char tempTex[320 * 240 * 2]; /*TODO*/
char* TextureBuffer;

/*TODO WTF? swap 3 and 10? */
char JJSTextureTable[]={0,1,2,3,4,0,1,2,3,4,5,6,7,8,9,10,16,17,18,19,20,11,20,12,13,21,14,15,16,17,18,19};
char JJSTextureMenuTable[]={0,1,2,3,4,0,1,2,10,4,5,6,7,8,9,3,16,17,18,19,20,11,20,12,13,21,14,15,16,17,18,19};

float zscale = 1.0 / 65536.0f;//1.0/32768.0f;
int menuTexturesIn = 0;
short PageXOff, PageYOff;

typedef struct
{
   long        id;
   long        flags;
   long        bnum;
   short       textureX, textureY;
   short       textureH, textureW;
} TimData;

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

void JJSReadTIM(char* filename, char* location, TimData* tim);

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

typedef struct Vertex
{
   float sx;            /* Screen coordinates */
   float sy;
   float sz;
   float rhw;           /* Reciprocal of homogeneous w */
   DWORD color;         /* Vertex color */
   DWORD specular;      /* Specular component of vertex */
   float tu;            /* Texture coordinates */
   float tv;
} Vertex;

#define RGB_MAKE(r, g, b)       ((DWORD) (((r) << 16) | ((g) << 8) | (b)))

void JJSDrawSprite(SPRT_8* sp, short w, short h)
{
   static Vertex vert[4]={0},*v;
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
   if(s2.y0+h>=240)
   {
      if(s2.y0>=240)
         return;
      h=240-s2.y0;
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
void SetColor(Vertex *v,char r, char g, char b)
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
   static Vertex vert[4]={0},*v;

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
   static Vertex vert[4]={0},*v;

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

void setst(Vertex*vv,short u,short v,short page)
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
   static Vertex vert[4]={0},*v;
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
   static Vertex vert[3]={0},*v;

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
   static Vertex vert[3]={0},*v;

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
   static Vertex vert[3]={0},*v;
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
   static   Vertex vert[3]={0},*v;
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
   static   Vertex vert[4]={0},*v;
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

void renderBegin(void)
{
   texpage = 420;
   renderClear();
}

void renderEnd(void)
{

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

void setupScreens(void)
{
   JJSLoadSprites();
}

void changeScreen(void)
{
   WinWidth = 320;
   WinHeight = 240;
   setupScreens();
   restoreTextures();
}

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
   glOrtho(0, 320, 240, 0, 0, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   WinWidth = 320;
   WinHeight = 240;

   setupScreens();

   TextureBuffer = (char*)malloc(TEXTURESJR);
}

void renderFree()
{
   wglMakeCurrent(0, 0);
   wglDeleteContext(hRC);
   ReleaseDC(hWnd, hDC);
}

void renderSwap()
{
   Sleep(1);
   SwapBuffers(hDC);

   char buf[256];
   //   sprintf(buf, "%d = G3:%d G4:%d F3:%d F4:%d GT3:%d GT4:%d FT3:%d FT4:%d\n", (dbg_G3 + dbg_G4 + dbg_F3 + dbg_F4 + dbg_GT3 + dbg_GT4 + dbg_FT3 + dbg_FT4), dbg_G3, dbg_G4, dbg_F3, dbg_F4, dbg_GT3, dbg_GT4, dbg_FT3, dbg_FT4);
   //   OutputDebugString(buf);
   dbg_G3 = dbg_G4 = dbg_F3 = dbg_F4 = dbg_GT3 = dbg_GT4 = dbg_FT3 = dbg_FT4 = 0;
}

void renderResize(int width, int height)
{
   gWidth = width;
   gHeight = height;
   glViewport(0, 0, width, height);
}

void renderOffset(int x, int y)
{
   glViewport(x * gWidth / 320, y * gHeight / 240, gWidth, gHeight);
}

void renderClear()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

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
