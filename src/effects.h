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
    VECTOR pos;
    SVECTOR vel;
    short count;
    short type;
    char u, v;
    short size;
} Effect;

typedef struct Shadow
{
    /*TODO shadow*/
    int TODO;
} Shadow;

void InitEffects(Effect* effects, Skeleton* camPos, Texture** effectTextures);
void UpdateEffects(Effect* effects);
void AddEffects(Effect* effects, WeaponData* weaponIndex, Texture** effectTextures, ShipData* shipIndex);
void ColourStartBoom(int noToLight);
void ResetStartBoom(void);
void InitStartBoom(Object** prm, short prmCount);
Shadow* InitShadow(Skeleton* camPos, Texture** effectTextures);
void DrawShadow(Object** shipShapes, ShipData* shipIndex, Shadow* shadow);
void SetShake(int duration);
void ShakeScreen(void);
void InitAuroraBorialis(Object* object);
void AuroraBorialis(Object* object);
void ColourRedLight(Object* redLight);
void ColourDroidLights(Object* droid);
void MoveDonkey(Object** donkey);
void DrawEffects(Effect* effects);
void DrawWeaponSprtIcons(Texture** weaponIconTextures, ShipData* playerShip);
void InitRedLight(Object** prm, short prmCount, Object** redLight);
void InitDonkey(Object** prm, short prmCount, Object** donkey);
void InitWeaponSprtIcons(Texture** weaponIconTextures);