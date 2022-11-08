/* (C) Psygnosis 1994. By Jason Carl Denton
*/

#include "game.h"

void InitSkeleton(Skeleton* skeleton)
{
    /* Set pointers */

    skeleton->super = NULL;

    /* Set translation matrix */

    skeleton->relative.t[0] = 0;
    skeleton->relative.t[1] = 0;
    skeleton->relative.t[2] = 0;

    /* Set rotation matrix top row */

    skeleton->relative.m[0][0] = 1 << FIXED_SHIFT;
    skeleton->relative.m[0][1] = 0;
    skeleton->relative.m[0][2] = 0;

    /* Set rotation matrix middle row */

    skeleton->relative.m[1][0] = 0;
    skeleton->relative.m[1][1] = 1 << FIXED_SHIFT;
    skeleton->relative.m[1][2] = 0;

    /* Set rotation matrix bottom row */

    skeleton->relative.m[2][0] = 0;
    skeleton->relative.m[2][1] = 0;
    skeleton->relative.m[2][2] = 1 << FIXED_SHIFT;

    /* Set flag to indicate absolute matrix needs recalculating */

    skeleton->update = 1;
}

void SetSkeletonPosition(Skeleton* skeleton, long x, long y, long z)
{
    skeleton->relative.t[0] = x;
    skeleton->relative.t[1] = y;
    skeleton->relative.t[2] = z;

    /* Mark skeleton for update */

    skeleton->update = 1;
}

void SetSkeletonDirectionHPR(Skeleton* skeleton, short hdg, short pitch, short roll)
{
    int32 sx, sy, sz;
    int32 cx, cy, cz;

    rsincos(pitch, sx, cx);
    rsincos(hdg, sy, cy);
    rsincos(roll, sz, cz);

    skeleton->relative.m[0][0] = ((cy * cz) + (((sy * sz) >> FIXED_SHIFT) * sx)) >> FIXED_SHIFT;
    skeleton->relative.m[0][1] = ((cy * sz) - (((sy * sx) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;
    skeleton->relative.m[0][2] = (-sy * cx) >> FIXED_SHIFT;

    skeleton->relative.m[1][0] = -(sz * cx) >> FIXED_SHIFT;
    skeleton->relative.m[1][1] = (cx * cz) >> FIXED_SHIFT;
    skeleton->relative.m[1][2] = -sx;

    skeleton->relative.m[2][0] = ((sy * cz) - (((cy * sx) >> FIXED_SHIFT) * sz)) >> FIXED_SHIFT;
    skeleton->relative.m[2][1] = ((sy * sz) + (((cy * sx) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;
    skeleton->relative.m[2][2] = ((cx * cy) >> FIXED_SHIFT);

    /* Mark skeleton for update */

    skeleton->update = 1;
}

void SetSkeletonDirectionRPH(Skeleton* skeleton, short roll, short pitch, short hdg)
{
    int32 sx, sy, sz;
    int32 cx, cy, cz;

    rsincos(pitch, sx, cx);
    rsincos(hdg, sy, cy);
    rsincos(roll, sz, cz);

    skeleton->relative.m[0][0] = ((cy * cz) - (((sz * sx) >> FIXED_SHIFT) * sy)) >> FIXED_SHIFT;
    skeleton->relative.m[0][1] = (sz * cx) >> FIXED_SHIFT;
    skeleton->relative.m[0][2] = (-(sy * cz) - (((sz * sx) >> FIXED_SHIFT) * cy)) >> FIXED_SHIFT;

    skeleton->relative.m[1][0] = (-(sz * cy) - (((sx * cz) >> FIXED_SHIFT) * sy)) >> FIXED_SHIFT;
    skeleton->relative.m[1][1] = (cx * cz) >> FIXED_SHIFT;
    skeleton->relative.m[1][2] = ((sz * sy) - (((sx * cz) >> FIXED_SHIFT) * cy)) >> FIXED_SHIFT;

    skeleton->relative.m[2][0] = (cx * sy) >> FIXED_SHIFT;
    skeleton->relative.m[2][1] = sx;
    skeleton->relative.m[2][2] = (cx * cy) >> FIXED_SHIFT;

    /* Mark skeleton for update */

    skeleton->update = 1;
}

void SetSkeletonDirection(Skeleton* skeleton, short xa, short ya, short za)
{
    int32 sx, sy, sz;
    int32 cx, cy, cz;

    rsincos(xa, sx, cx);
    rsincos(ya, sy, cy);
    rsincos(za, sz, cz);

    /* Top Row */

    skeleton->relative.m[0][0] = (cy * cz) >> FIXED_SHIFT;
    skeleton->relative.m[0][1] = (-cy * sz) >> FIXED_SHIFT;
    skeleton->relative.m[0][2] = -sy;

    /* Middle Row */

    skeleton->relative.m[1][0] = ((cx * sz) - (((sx * sy) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;
    skeleton->relative.m[1][1] = ((cx * cz) + (((sx * sy) >> FIXED_SHIFT) * sz)) >> FIXED_SHIFT;
    skeleton->relative.m[1][2] = (-sx * cy) >> FIXED_SHIFT;

    /* Bottom Row */

    skeleton->relative.m[2][0] = ((sx * sz) + (((cx * sy) >> FIXED_SHIFT) * cz)) >> FIXED_SHIFT;
    skeleton->relative.m[2][1] = ((sx * cz) - (((cx * sy) >> FIXED_SHIFT) * sz)) >> FIXED_SHIFT;
    skeleton->relative.m[2][2] = (cx * cy) >> FIXED_SHIFT;

    /* Mark skeleton for update */

    skeleton->update = 1;
}

MATRIX* GetSkeletonMatrix(Skeleton* skeleton)
{
    MATRIX* m1;
    MATRIX* m2;
    MATRIX* m3;

    if (!(skeleton->super))
    {
        return (&(skeleton->relative));
    }
    else
    {
        if (skeleton->update)
        {
            /* Need to update the absolute matrix */

            m1 = &(skeleton->relative);
            m2 = GetSkeletonMatrix(skeleton->super);
            m3 = &(skeleton->absolute);

            m3->t[0] = ((m1->t[0] * m2->m[0][0] + m1->t[1] * m2->m[0][1] + m1->t[2] * m2->m[0][2]) >> FIXED_SHIFT) + m2->t[0];
            m3->t[1] = ((m1->t[0] * m2->m[1][0] + m1->t[1] * m2->m[1][1] + m1->t[2] * m2->m[1][2]) >> FIXED_SHIFT) + m2->t[1];
            m3->t[2] = ((m1->t[0] * m2->m[2][0] + m1->t[1] * m2->m[2][1] + m1->t[2] * m2->m[2][2]) >> FIXED_SHIFT) + m2->t[2];

            MulMatrix0(m2, m1, m3);

            skeleton->update = 0;
        }

        return (&(skeleton->absolute));
    }
}
