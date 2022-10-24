#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

#ifdef USE_DIV_TABLE
    #define DIV_TABLE_SIZE   (4096 + 1) // to compare with #4096 without extra LDR
    #define FixedInvS(x)     ((x < 0) ? -divTable[abs(x)] : divTable[x])
    #define FixedInvU(x)     divTable[x]
    typedef uint32 divTableInt;
    extern const divTableInt divTable[DIV_TABLE_SIZE];
#endif

int sar(int a, int n);
int GetMagnitude(const vec4i* vec1);
void MinusVector(const vec4i* v1, const vec4i* v2, vec4i* result);
void CrossProduct(vec4i* result, const vec4i* A, const vec4i* B);
int ScalarProduct(const vec4i* vec1, const vec4i* vec2);
int GetAng(const vec4i* vec1, const vec4i* vec2);
int ang(int a);
int arccos(int value);

long rcos(long a);
long rsin(long a);
long ratan2(long y, long x);
long SquareRoot0(long a);

void RotTrans(const MATRIX* m, const vec4s* v0, vec4i* v1, long* flag);
long RotTransPers(const MATRIX* m, const vec4s* v0, DVECTOR* sxy, long* flag);
void MulMatrix0(const MATRIX* m0, const MATRIX* m1, MATRIX* ret);


int32 SwapLE32(int32 x);
int16 SwapLE16(int16 x);
void IntelLong(long* lvalue);
void IntelShort(short* svalue);

#endif
