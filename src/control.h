#define NORMALPAD 0
#define HRP 0

#define JUST_FRONT_DELAY    (FR10 * 15)
#define JUST_BEHIND_DELAY   (FR10 * 20)
#define IN_SIGHT_DELAY      (FR10 * 20)
#define STALL_DELAY         (FR10 * 9)
#define START_TIME          (FR10 * 20)
#define THREE               (FR10 * 15)
#define RED_LIGHT           (FR10 * 10)
#define AMBER_LIGHT         (FR10 * 5)
#define RACE_START          0

#define SWITCH_TO_RACE_VIEW 100

#define STALL -1

void UpdateShips(ShipData* shipIndex, WeaponData* weaponIndex);
void GeneralStartProcs(ShipData* playerShip);

void UpdatePlayerShipNorm(ShipData*, int, WeaponData*);
void UpdateRemoteShip(ShipData*, int, WeaponData*);
void UpdatePlayerShipNormStart(ShipData*, int, WeaponData*);
void UpdateRemoteShipStart(ShipData*, int, WeaponData*);

void HoldCentre(vec4i* result, ShipData* shipIndex, Face* facePtr, int);
void HoldRight(vec4i* result, ShipData* shipIndex, Face* facePtr, int);
void HoldLeft(vec4i* result, ShipData* shipIndex, Face* facePtr, int);
void BlockShip(vec4i* blockVector, ShipData* shipIndex, Face* facePtr, int);
void AvoidShip(vec4i* blockVector, ShipData* shipIndex, Face* facePtr, int);
void AvoidOtherShips(vec4i* blockVector, ShipData* shipIndex, Face* facePtr, int);
void ZigZagShip(vec4i* blockVector, ShipData* shipIndex, Face* facePtr, int);
