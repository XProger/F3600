#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

int sar(int a, int n);
int GetMagnitude(const VECTOR* vec1);
void MinusVector(const VECTOR* v1, const VECTOR* v2, VECTOR* result);
void CrossProduct(VECTOR* result, const VECTOR* A, const VECTOR* B);
int ScalarProduct(const VECTOR* vec1, const VECTOR* vec2);
int GetAng(const VECTOR* vec1, const VECTOR* vec2);
int ang(int a);
int arccos(int value);

long rcos(long a);
long rsin(long a);
long ratan2(long y, long x);
long SquareRoot0(long a);

void RotTrans(const MATRIX* m, const SVECTOR* v0, VECTOR* v1, long* flag);
long RotTransPers(const MATRIX* m, const SVECTOR* v0, DVECTOR* sxy, long* flag);
void MulMatrix0(const MATRIX* m0, const MATRIX* m1, MATRIX* ret);


int32 SwapLE32(int32 x);
int16 SwapLE16(int16 x);
void IntelLong(long* lvalue);
void IntelShort(short* svalue);

#endif
