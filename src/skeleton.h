/* (C) Psygnosis 1994. By Jason Carl Denton
*/

void InitSkeleton(Skeleton* skeleton);
Skeleton* NewSkeleton(void);

void SetSkeletonPosition(Skeleton* skeleton, long x, long y, long z);

void SetSkeletonDirectionHPR(Skeleton* skeleton, short hdg, short pitch, short roll);
void SetSkeletonDirectionRPH(Skeleton* skeleton, short roll, short pitch, short hdg);
void SetSkeletonDirection(Skeleton* skeleton, short xa, short ya, short za);

MATRIX* GetSkeletonMatrix(Skeleton* skeleton);
