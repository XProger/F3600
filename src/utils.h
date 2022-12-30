#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

#define DIV_TABLE_SIZE   (4096 + 1) // to compare with #4096 without extra LDR
#define FixedInvS(x)     ((x < 0) ? -divTable[abs(x)] : divTable[x])
#define FixedInvU(x)     divTable[x]
typedef uint32 divTableInt;
extern const divTableInt divTable[DIV_TABLE_SIZE];

int32 GetMagnitude(const vec3i* v);
int32 GetMagnitudeSqr(const vec3i* v);

int32 GetMagnitudeApprox(int32 dx, int32 dy);
int dot(const vec3i* vec1, const vec3i* vec2);
int GetAng(const vec3i* vec1, const vec3i* vec2);
int ang(int a);
int arccos(int value);

extern const int32 gSinCosTable[4096];

#define rsin(x) (gSinCosTable[(uint32)((x) << (16 + 4)) >> 20] >> (16 + 2))

#define rcos(x) (gSinCosTable[(uint32)((x) << (16 + 4)) >> 20] << 16 >> (16 + 2))

#define rsincos(x,s,c) {\
    uint32 sc = gSinCosTable[(uint32)((x) << (16 + 4)) >> 20];\
    s = ((int32)sc) >> (16 + 2);\
    c = ((int32)sc) << 16 >> (16 + 2);\
}

int32 ratan2(int32 y, int32 x);
uint32 rsqrt(uint32 x);

#define FIXED_SHIFT 12 /* number of bits to shift for 4096 */

void ApplyMatrix(const MATRIX* m, const vec3s* v0, vec3i* v1);
void RotTrans(const MATRIX* m, const vec3s* v0, vec3i* v1, int32* flag);
int32 RotTransPers(const MATRIX* m, const vec3s* v0, vec2s* sxy, int32* flag);
void MulMatrix0(const MATRIX* m0, const MATRIX* m1, MATRIX* ret);

uint16 swap16(uint16 x);
uint32 swap32(uint32 x);
void swap16p(void* ptr);
void swap32p(void* ptr);

int32 SwapLE32(int32 x);
int16 SwapLE16(int16 x);

typedef struct Matrix
{
#if defined(__3DO__)
    int32 e00, e10, e20;
    int32 e01, e11, e21;
    int32 e02, e12, e22;
    int32 e03, e13, e23;
#elif defined(USE_MATRIX_INT16)
    int16 e00, e01, e02;
    int16 e10, e11, e12;
    int16 e20, e21, e22;
    int16 e03, e13, e23;
#else
    int32 e00, e01, e02, e03;
    int32 e10, e11, e12, e13;
    int32 e20, e21, e22, e23;
#endif
} Matrix;

#define MAX_MATRICES 2

extern Matrix gMatrixStack[MAX_MATRICES];
extern Matrix* gMatrixPtr;

void matrixPush(void);
void matrixSetViewMenu(void);
void matrixSetViewInternal(const vec3i* pos, int32 angleX, int32 angleY, int32 angleZ);
void matrixSetViewExternal(const vec3i* pos, int32 angleX, int32 angleY);
void matrixTranslate(int32 x, int32 y, int32 z);
void matrixRotateYXZ(int32 angleX, int32 angleY, int32 angleZ);

#define matrixPop() gMatrixPtr--

#endif
