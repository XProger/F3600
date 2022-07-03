#define NO_OF_EFFECTS 140
#define NO_OF_EFFECT_TEXTURES 7

#define FIRE 0
#define SMOKE 1
#define FIRE_WHITE 2
#define SHADOW 3
#define EBOLT 4
#define HALO 5
#define GREENY 6

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
void ResetStartBoom();
void InitStartBoom(Object** prm, short prmCount);
Shadow* InitShadow(Skeleton* camPos, Texture** effectTextures);
void DrawShadow(Object** shipShapes, ShipData* shipIndex, Shadow* shadow);
void SetShake(int duration);
void ShakeScreen();
void InitAuroraBorialis(Object* object);
void AuroraBorialis(Object* object);
void ColourRedLight(Object* redLight);
void MoveDonkey(Object** donkey);
void DrawEffects(Effect* effects);
void DrawWeaponSprtIcons(Texture** weaponIconTextures, ShipData* playerShip);
void InitRedLight(Object** prm, short prmCount, Object** redLight);
void InitDonkey(Object** prm, short prmCount, Object** donkey);
void InitWeaponSprtIcons(Texture** weaponIconTextures);