#include "game.h"

/* reduce this later !!! */
#define ObjectListMax 128

/*TODO precalc this shit*/
void SceneRadiusCheck(Object* objects, int32 count)
{
    TrackSection* section;
    TrackSection* visSection;
    int32 i, j, index;
    Object* object;
    vec3i* sectionCentre;
    vec4i vector;
    int32 distance;
    int16 objectList[ObjectListMax];
    int16 withinLocal;
    int16 withinGlobal;
    int16 allCount;
    int32 objectCount;

    /* allocate and fill in a global object table */

    /* do the radius check */

    section = gTrack.sections;

    for (i = 0; i < gTrack.sectionCount; i++)
    {
        objectCount = 0;
        allCount = 0;

        for (index = 0; index < count; index++)
        {
            object = objects + index;

            withinLocal = 0;
            withinGlobal = 0;

            sectionCentre = &section->centre;

            vector.x = object->pos.x - sectionCentre->x;
            vector.y = object->pos.y - sectionCentre->y;
            vector.z = object->pos.z - sectionCentre->z;

            if (section->flags & Jump)
            {
                if ((abs(vector.x) < (section->globalRadius * 2)) && (abs(vector.y) < (section->globalRadius * 2)) && (abs(vector.z) < (section->globalRadius * 2)))
                {
                    distance = rsqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));

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
                if ((abs(vector.x) < section->globalRadius) && (abs(vector.y) < section->globalRadius) && (abs(vector.z) < section->globalRadius))
                {
                    distance = rsqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));

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
                    sectionCentre = &visSection->centre;

                    vector.x = object->pos.x - sectionCentre->x;
                    vector.y = object->pos.y - sectionCentre->y;
                    vector.z = object->pos.z - sectionCentre->z;

                    if ((abs(vector.x) < section->localRadius) && (abs(vector.y) < section->localRadius) && (abs(vector.z) < section->localRadius))
                    {
                        distance = rsqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));

                        distance = distance - object->extent;

                        if (distance < section->localRadius)
                        {
                            /* within the local radius */
                            withinLocal = 1;
                        }
                    }
                    visSection = visSection->nextSection;
                }

                /* Check distance backwards along the track */

                visSection = section;
                for (j = 0; (!withinLocal) && (j < 30); j++)
                {
                    sectionCentre = &visSection->centre;

                    vector.x = object->pos.x - sectionCentre->x;
                    vector.y = object->pos.y - sectionCentre->y;
                    vector.z = object->pos.z - sectionCentre->z;

                    if ((abs(vector.x) < section->localRadius) && (abs(vector.y) < section->localRadius) && (abs(vector.z) < section->localRadius))
                    {
                        distance = rsqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));

                        distance = distance - object->extent;

                        if (distance < section->localRadius)
                        {
                            /* within the local radius */
                            withinLocal = 1;
                        }
                    }
                    visSection = visSection->prevSection;
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

            allCount++;
        }

        section->objects = (short*)allocDRAM(objectCount * sizeof(short));

        if (!section->objects)
        {
            ASSERT(0);
        }
        section->objectsCount = objectCount;

        for (j = 0; j < objectCount; j++)
        {
            section->objects[j] = objectList[j];
        }

        section = NextIndexSection(section);
    }
}
