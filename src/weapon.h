/***** Weapon Defines *****/

#define NO_OF_WEAPONS 29

#define NO_OF_MINES     15
#define NO_OF_ROCKETS   4
#define NO_OF_MISSILES  4
#define NO_OF_SHIELDS   3
#define NO_OF_EBOLTS    3

#define MINE_OFFSET     4
#define ROCKET_OFFSET   0
#define MISSILE_OFFSET  19
#define SHIELD_OFFSET   23
#define EBOLT_OFFSET    26

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

#define NO_WEAPON 0

#define ROCKET    1
#define MISSILE   2
#define MINE      3
#define SPECIAL   4
#define ELEC_BOLT 5
#define FLARE     6
#define REV_CON   7
#define SHIELD    8
#define TURBO     9

#define NO_OF_WEAPON_TYPES 9

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
    short used;
} WeaponShape;

typedef struct WeaponData
{
    short count;
    short currentWeapon;
    TrackSection* nearTrkSect;
    VECTOR acc;
    VECTOR vel;
    VECTOR pos;
    short hdg, pitch;
    signed char targetShip;
    char fireShip;
    char type;
    char availible;
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
void LaunchWeapon(ShipData* shipIndex, WeaponData* weaponIndex, int shipNo);
void FireWeapon(ShipData*, WeaponData*, int, char);
void MissileLock(ShipData* playerShip, ShipData* shipIndex, Object** shipShapes);
void initWeapons(WeaponShape* weaponShapes, Skeleton* camPos, WeaponData* weaponIndex);

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
void GetFireVector(VECTOR* result, VECTOR target, Face* facePtr, VECTOR shipPos);
void CycleWeaponGrids(ShipData* shipIndex, WeaponGrid* weaponGrid, int noWeaponGrids);
void DrawWeapons(ShipData* shipIndex, WeaponShape* weaponShapes, WeaponData* weaponIndex);
void DrawTargetIcon(ShipData* playerShip, ShipData* shipIndex, Object** shipShapes);
void SetTargetShip(ShipData* shipIndex, int shipNo);
int GetNewWeapon(void);
