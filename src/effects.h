#define MAX_EFFECTS 64

#define WEAPON_FX_EXPLOSION_SPRITES 16

void InitEffects(void);
void ResetEffects(void);
void UpdateEffects(void);
void ColourStartBoom(int noToLight);
void ResetStartBoom(void);
void InitShadow(void);
void DrawShadows(ShipData* shipIndex);
void SetShake(int duration);
void ShakeScreen(void);
void InitAuroraBorialis(Object* object);
void AuroraBorialis(Object* object);
void ColourRedLight(void);
void ColourDroidLights(Object* droid);
void MoveDonkey(void);
void DrawEffects(void);

void AddTrail(WeaponData* weapon);
void AddExplosion(WeaponData* weapon, int32 targetShip);