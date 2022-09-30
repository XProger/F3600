#include "game.h"

int gWidth, gHeight, gBorder;

void renderInit(void) {}
void renderFree(void) {}
void renderSwap(void) {}
void renderResize(int width, int height) {}
void renderOffset(int x, int y) {}
void renderClear(void) {}
void renderQuad(const Quad* q) {}
void renderShadow(const Shadow* s) {}
void renderColorQuad(const ColorQuad* q) {}
void renderBillboard(const Billboard* b) {}
void renderBackFill(int x, int y, int w, int h) {}

void renderPrepare(void) {}
void renderFrameBegin(void) {}
void renderFrameEnd(void) {}
void renderSetMatrix(const MATRIX* matrix, int x, int y, int z) {}
void renderSetSkeleton(Skeleton* skeleton, int x, int y, int z) {}
void renderTrack(void) {}
void renderObject(const Object* object) {}
void renderShip(const Object* object, uint32 color) {}
void renderShield(const Object* object, int32 counter) {}
void renderSky(const Object* object) {}
void renderSetMode(int renderMode) {}
void renderSetBlend(int blend) {}
void renderDepthWrite(int enable) {}
void renderNextLayer(void) {}
Texture* renderCreateTexture(Tim* tim) { return NULL; }