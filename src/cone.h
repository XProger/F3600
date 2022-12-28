typedef struct Cone
{
    int16 coneIndex[3][8];
    vec3s oldVertex[3][8];
    Object* object[8];
    ShipData* ship[8];
} Cone;

extern Cone ctrlcone;

void InitCone(Cone* cone);
void ResetCone(Cone* cone);
void UpdateCone(Cone* cone);
