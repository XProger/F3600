#include "game.h"

/* reduce this later !!! */
#define ObjectListMax 128

/*TODO precalc this shit*/
void SceneRadiusCheck(Object* objects)
{
    TrackSection* section;
    TrackSection* visSection;
    short i, j;
    Object* object;
    MATRIX* matrix;
    Vector* sectionCentre;
    short objectCount;
    VECTOR vector;
    long distance;
    short objectList[ObjectListMax];
    short withinLocal;
    short withinGlobal;
    short allCount;

    /* allocate and fill in a global object table */

    object = objects;
    objectCount = 0;
    while (object)
    {
        object = object->next;
        objectCount++;
    }

    ASSERT(objectCount <= ObjectTableMax);

    object = objects;
    for (i = 0; i < objectCount; i++)
    {
        objectTable[i] = object;
        object = object->next;
    }

    /* do the radius check */

    section = track->sections;

    for (i = 0; i < track->sectionCount; i++)
    {
        object = objects;
        objectCount = 0;
        allCount = 0;
        while (object)
        {
            withinLocal = 0;
            withinGlobal = 0;

            matrix = &object->skeleton->relative;
            sectionCentre = &section->centre;

            vector.vx = matrix->t[0] - sectionCentre->vx;
            vector.vy = matrix->t[1] - sectionCentre->vy;
            vector.vz = matrix->t[2] - sectionCentre->vz;

            if (section->flags & Jump)
            {
                if ((abs(vector.vx) < (section->globalRadius * 2)) && (abs(vector.vy) < (section->globalRadius * 2)) && (abs(vector.vz) < (section->globalRadius * 2)))
                {
                    distance = SquareRoot0((vector.vx * vector.vx) + (vector.vy * vector.vy) + (vector.vz * vector.vz));

                    distance = distance - object->extent;

                    if (distance < section->globalRadius)
                    {
                        /* within the global radius */
                        withinGlobal = 1;
                    }
                }
            }
            else
            {
                if ((abs(vector.vx) < section->globalRadius) && (abs(vector.vy) < section->globalRadius) && (abs(vector.vz) < section->globalRadius))
                {
                    distance = SquareRoot0((vector.vx * vector.vx) + (vector.vy * vector.vy) + (vector.vz * vector.vz));

                    distance = distance - object->extent;

                    if (distance < section->globalRadius)
                    {
                        /* within the global radius */
                        withinGlobal = 1;
                    }
                }
            }

            if (withinGlobal)
            {
                /* Check distance forwards along the track */

                visSection = section;
                for (j = 0; (!withinLocal) && (j < 30); j++)
                {
                    matrix = &object->skeleton->relative;
                    sectionCentre = &visSection->centre;

                    vector.vx = matrix->t[0] - sectionCentre->vx;
                    vector.vy = matrix->t[1] - sectionCentre->vy;
                    vector.vz = matrix->t[2] - sectionCentre->vz;

                    if ((abs(vector.vx) < section->localRadius) && (abs(vector.vy) < section->localRadius) && (abs(vector.vz) < section->localRadius))
                    {
                        distance = SquareRoot0((vector.vx * vector.vx) + (vector.vy * vector.vy) + (vector.vz * vector.vz));

                        distance = distance - object->extent;

                        if (distance < section->localRadius)
                        {
                            /* within the local radius */
                            withinLocal = 1;
                        }
                    }
                    visSection = visSection->nextSection.ptr;
                }

                /* Check distance backwards along the track */

                visSection = section;
                for (j = 0; (!withinLocal) && (j < 30); j++)
                {
                    matrix = &object->skeleton->relative;
                    sectionCentre = &visSection->centre;

                    vector.vx = matrix->t[0] - sectionCentre->vx;
                    vector.vy = matrix->t[1] - sectionCentre->vy;
                    vector.vz = matrix->t[2] - sectionCentre->vz;

                    if ((abs(vector.vx) < section->localRadius) && (abs(vector.vy) < section->localRadius) && (abs(vector.vz) < section->localRadius))
                    {
                        distance = SquareRoot0((vector.vx * vector.vx) + (vector.vy * vector.vy) + (vector.vz * vector.vz));

                        distance = distance - object->extent;

                        if (distance < section->localRadius)
                        {
                            /* within the local radius */
                            withinLocal = 1;
                        }
                    }
                    visSection = visSection->prevSection.ptr;
                }
            }

            if (withinLocal)
            {
                if (objectCount < ObjectListMax)
                {
                    objectList[objectCount++] = allCount;
                }
                else
                {
                    ASSERT(0);
                }
            }

            object = object->next;
            allCount++;
        }

        section->objects = (short*)allocDRAM(objectCount * sizeof(short));

        if (!section->objects)
        {
            ASSERT(0);
        }
        section->objectCount = objectCount;

        for (j = 0; j < objectCount; j++)
        {
            section->objects[j] = objectList[j];
        }

        section = NextIndexSection(section);
    }
}
