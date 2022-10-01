#define NO_OF_EFFECTS 140
#define NO_OF_EFFECT_TEXTURES 7

#define FX_FIRE        0
#define FX_SMOKE       1
#define FX_FIRE_WHITE  2
#define FX_SHADOW      3
#define FX_EBOLT       4
#define FX_HALO        5
#define FX_GREENY      6

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
