#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

int sar(int a, int n);
int GetMagnitude(VECTOR vec1);
void MinusVector(VECTOR v1, VECTOR v2, VECTOR* result);
void CrossProduct(VECTOR* result, VECTOR A, VECTOR B);
int ScalarProduct(VECTOR vec1, VECTOR vec2);
int GetAng(VECTOR vec1, VECTOR vec2);
int ang(int a);
int arccos(int value);

long rcos(long a);
long rsin(long a);
long ratan2(long y, long x);
long SquareRoot0(long a);

void RotTrans(const MATRIX* m, const SVECTOR* v0, VECTOR* v1, long* flag);
long RotTransPers(const MATRIX* m, const SVECTOR* v0, DVECTOR* sxy, long* flag);
void MulMatrix0(const MATRIX* m0, const MATRIX* m1, MATRIX* ret);

#endif
