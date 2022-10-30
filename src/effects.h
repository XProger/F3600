#define MAX_EFFECTS 64

#define WEAPON_FX_EXPLOSION_SPRITES 16

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
    vec3i pos;
    vec3i vel;
    int32 count;
    short type;
    short size;
    struct Effect* next;
    struct Effect* prev;
} Effect;

void InitEffects(void);
void ResetEffects(void);
void UpdateEffects(void);
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

void AddTrail(WeaponData* weapon);
void AddExplosion(WeaponData* weapon, int32 targetShip);