typedef struct
{
    short coneIndex[3][8];

    vec4s oldVertex[3][8];

    Object* object[8];
    ShipData* ship[8];
} Cone;

void InitCone(Cone* cone);
void ResetCone(Cone* cone);
void UpdateCone(Cone* cone);
