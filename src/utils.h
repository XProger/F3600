#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

#define DIV_TABLE_SIZE   (4096 + 1) // to compare with #4096 without extra LDR
#define FixedInvS(x)     ((x < 0) ? -divTable[abs(x)] : divTable[x])
#define FixedInvU(x)     divTable[x]
typedef uint32 divTableInt;
extern const divTableInt divTable[DIV_TABLE_SIZE];

int GetMagnitude(const vec3i* vec1);
int32 GetMagnitudeApprox(int32 dx, int32 dy);
void MinusVector(const vec3i* v1, const vec3i* v2, vec3i* result);
void CrossProduct(vec3i* result, const vec3i* A, const vec3i* B);
int ScalarProduct(const vec3i* vec1, const vec3i* vec2);
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

void RotTrans(const MATRIX* m, const vec3s* v0, vec3i* v1, long* flag);
long RotTransPers(const MATRIX* m, const vec3s* v0, vec2s* sxy, long* flag);
void MulMatrix0(const MATRIX* m0, const MATRIX* m1, MATRIX* ret);


int32 SwapLE32(int32 x);
int16 SwapLE16(int16 x);
void IntelLong(long* lvalue);
void IntelShort(short* svalue);

#endif
