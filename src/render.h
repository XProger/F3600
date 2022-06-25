typedef struct Quad
{
   signed short   x, y;
   
   unsigned short w, h;
   
   unsigned char  u0, v0;
   unsigned char  u1, v1;

   unsigned short clut;
   unsigned short page;
} Quad;

void renderInit();
void renderFree();
void renderSwap();
void renderResize(int width, int height);
void renderOffset(int x, int y);
void renderClear();
void renderBegin(void);
void renderEnd(void);
void renderQuad(const Quad* spr);

void renderPrepare();
void renderTrack();
void renderObject(const Object* object, int roll);
void renderSky(const Object* object);
void renderBegin3D();
void renderEnd3D();