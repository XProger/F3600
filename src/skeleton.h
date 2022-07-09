/* (C) Psygnosis 1994. By Jason Carl Denton
*/

void InitSkeleton(Skeleton* skeleton);
Skeleton* NewSkeleton(void);

void SetSkeletonPosition(Skeleton* skeleton, long x, long y, long z);

void SetSkeletonDirection(Skeleton* skeleton, short xa, short ya, short za);

MATRIX* GetSkeletonMatrix(Skeleton* skeleton);
