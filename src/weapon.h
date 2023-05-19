/***** Weapon Defines *****/

enum WeaponShapeType
{
    WEAPON_SHAPE_ROCKET,
    WEAPON_SHAPE_MINE,
    WEAPON_SHAPE_MISSILE,
    WEAPON_SHAPE_SHIELD,
    WEAPON_SHAPE_EBOLT,
    WEAPON_SHAPE_MAX
};

#define MINE_DURATION       (FR10 * 45)
#define ROCKET_DURATION     (FR10 * 20)
#define ELEC_DURATION       (FR10 * 14)
#define REV_CON_DURATION    (FR10 * 6)
#define MISSILE_DURATION    (FR10 * 20)
#define SHIELD_DURATION     (FR10 * 20)
#define FLARE_DURATION      (FR10 * 20)
#define SPECIAL_DURATION    (FR10 * 40)

#define MINES_COUNT         5
#define MINE_RELEASE_RATE   3
#define WEAPON_DELAY        (FR40)

enum WeaponType
{
    WPN_NONE,
    WPN_MINE,
    WPN_MISSILE,
    WPN_ROCKET,
    WPN_SPECIAL,    // TODO multiplayer
    WPN_ELEC_BOLT,
    WPN_FLARE,      // TODO unused
    WPN_REV_CON,    // TODO multiplayer
    WPN_SHIELD,
    WPN_TURBO
};
\
#define MAX_WEAPONS 32

/***** Weapon Grid defines *****/

#define DEACTIVATED 0
#define ACTIVE 1

typedef struct WeaponData
{
    int16 count;
    int16 currentWeapon;
    TrackSection* nearTrkSect;
    vec3i acc;
    vec3i vel;

    vec3i pos;
    int32 pitch, hdg, roll;
#ifdef USE_FRAME_LERP
    vec3i _pos;
    int32 _pitch, _hdg, _roll;
#endif
    ShipData* fireShip;

    uint8 type;

    ShipData* targetShip;

    void (*update)(ShipData*, struct WeaponData*);
    struct WeaponData* next;
    struct WeaponData* prev;
} WeaponData;

extern WeaponData* gWeaponsActive;

void UpdateWeapons(ShipData* shipIndex);
void FireWeapon(ShipData* ship, int32 shipNo, int32 decision);
void MissileLock(ShipData* ship, ShipData* shipIndex);
void ResetWeapons(void);
void UpdateWeaponGrids(ShipData* shipIndex, WeaponGrid* weaponGrid, int noWeaponGrids);
void DrawWeapons(void);
void DrawTargetIcon(ShipData* targetShip);
