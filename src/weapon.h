/***** Weapon Defines *****/

enum WeaponShapeType
{
    WEAPON_SHAPE_ROCKET,
    WEAPON_SHAPE_MINE,
    WEAPON_SHAPE_MISSILE,
    WEAPON_SHAPE_SHIELD,
    WEAPON_SHAPE_EBOLT,
    WEAPON_SHAPE_NAX
};

#define MINE_DURATION       (FR10 * 45)
#define ROCKET_DURATION     (FR10 * 20)
#define ELEC_DURATION       (FR10 * 14)
#define REV_CON_DURATION    (FR10 * 6)
#define MISSILE_DURATION    (FR10 * 20)
#define SHIELD_DURATION     (FR10 * 20)
#define FLARE_DURATION      (FR10 * 20)
#define SPECIAL_DURATION    (FR10 * 40)

#define MINE_RELEASE_RATE  3
#define WEAPON_DELAY       (FR40)

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

#define SHIP_EXPLODE -1
#define TRACK_EXPLODE -2
#define NON_ACTIVE 0

#define NO_ACTIVE_WEAPONS 32

/***** Weapon Grid defines *****/

#define DEACTIVATED 0
#define ACTIVE 1

typedef struct WeaponShape
{
    Object* shapePtr;
} WeaponShape;

typedef struct WeaponData
{
    short count;
    short currentWeapon;
    TrackSection* nearTrkSect;
    vec4i acc;
    vec4i vel;
    vec4i pos;
    short hdg, pitch;
    signed char targetShip;
    char fireShip;
    char type;
    void (*updateWeapon)(ShipData*, WeaponShape*, struct WeaponData*, struct WeaponData*);
} WeaponData;

typedef struct WeaponGrid
{
    TrackSection* section;
    short attr;
    short cycleCount;
    int count;
    CVECTOR colour;
} WeaponGrid;

void UpdateWeapons(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weaponIndex);
void FireWeapon(ShipData*, WeaponData*, int, char);
void MissileLock(ShipData* playerShip, ShipData* shipIndex, Object** shipShapes);
void InitWeapons(WeaponShape* weaponShapes);
void ResetWeapons(WeaponShape* weaponShapes, Skeleton* camPos, WeaponData* weaponIndex);

void UpdateRocket(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void InitRocket(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void UpdateMissile(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void InitMissile(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void UpdateMine(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void InitMine(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void InitElecBolt(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void UpdateElecBolt(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void InitShield(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void UpdateShield(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void InitRevCon(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void UpdateRevCon(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);
void InitSpecial(ShipData* playerShip, WeaponShape*, WeaponData*, WeaponData*);

void Weapon2Mine(WeaponData* weapon, WeaponData* weaponIndex, WeaponShape* weaponShapes);
void GetFireVector(vec4i* result, vec4i target, Face* facePtr, vec4i shipPos);
void CycleWeaponGrids(ShipData* shipIndex, WeaponGrid* weaponGrid, int noWeaponGrids);
void DrawWeapons(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weaponIndex);
void DrawTargetIcon(ShipData* playerShip, ShipData* shipIndex, Object** shipShapes);
void SetTargetShip(ShipData* shipIndex, int shipNo);
int GetNewWeapon(void);
