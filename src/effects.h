#define MAX_EFFECTS 64

#define WEAPON_FX_EXPLOSION_SPRITES 16

void ResetEffects(void);
void UpdateEffects(void);
void ColourStartBoom(int noToLight);
void ResetStartBoom(void);
void InitShadow(void);
void DrawShadows(ShipData* shipIndex);

void ShakeUpdate(ShipData* ship);
void ShakeOffset(ShipData* ship);

void InitAuroraBorialis(Object* object);
void AuroraBorialis(Object* object);
void ColourRedLight(void);
void ColourDroidLights(Object* droid);
void MoveDonkey(void);
void DrawEffects(void);

void AddTrail(WeaponData* weapon);
void AddExplosion(WeaponData* weapon, int32 targetShip);