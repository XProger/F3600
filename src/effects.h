#define NO_OF_EFFECTS 140

enum FxType
{
    FX_FIRE,
    FX_FIRE_WHITE,
    FX_SMOKE,
    FX_EBOLT,
    FX_HALO,
    FX_GREENY,
    FX_MAX
};

typedef struct Effect
{
    int x, y, z;
    int vx, vy, vz;
    short count;
    short type;
    short size;
} Effect;

void InitEffects(void);
void ResetEffects(void);
void UpdateEffects(void);
void AddEffects(WeaponData* weaponIndex, ShipData* shipIndex);
void ColourStartBoom(int noToLight);
void ResetStartBoom(void);
void InitStartBoom(Object** prm, short prmCount);
void InitShadow(void);
void DrawShadows(ShipData* shipIndex);
void SetShake(int duration);
void ShakeScreen(void);
void InitAuroraBorialis(Object* object);
void AuroraBorialis(Object* object);
void ColourRedLight(Object* redLight);
void ColourDroidLights(Object* droid);
void MoveDonkey(Object** donkey);
void DrawEffects(void);
void InitRedLight(Object** prm, short prmCount, Object** redLight);
void InitDonkey(Object** prm, short prmCount, Object** donkey);
