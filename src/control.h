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

void UpdateShips(ShipData* shipIndex, WeaponShape* weaponShapes, Object** shipShapes, WeaponData*);
void GeneralStartProcs(ShipData* playerShip);

void UpdatePlayerShipNorm(ShipData*, Object**, int, WeaponData*);
void UpdateRemoteShip(ShipData*, Object**, int, WeaponData*);
void UpdatePlayerShipNormStart(ShipData*, Object**, int, WeaponData*);
void UpdateRemoteShipStart(ShipData*, Object**, int, WeaponData*);

void HoldCentre(VECTOR* result, ShipData* shipIndex, Face* facePtr, int);
void HoldRight(VECTOR* result, ShipData* shipIndex, Face* facePtr, int);
void HoldLeft(VECTOR* result, ShipData* shipIndex, Face* facePtr, int);
void BlockShip(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int);
void AvoidShip(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int);
void AvoidOtherShips(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int);
void ZigZagShip(VECTOR* blockVector, ShipData* shipIndex, Face* facePtr, int);

void SetSkeletonDirectionHPR(Skeleton* skeleton, short hdg, short pitch, short roll);
void SetSkeletonDirectionRPH(Skeleton* skeleton, short roll, short pitch, short hdg);
