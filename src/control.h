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

void UpdateShips(ShipData* shipIndex);
void GeneralStartProcs(ShipData* playerShip);

void UpdatePlayerShipNorm(ShipData* shipIndex, int shipNo);
void UpdateRemoteShip(ShipData* shipIndex, int shipNo);
void UpdatePlayerShipNormStart(ShipData* shipIndex, int shipNo);
void UpdateRemoteShipStart(ShipData* shipIndex, int shipNo);

void HoldCentre(vec3i* result, ShipData* shipIndex, Face* facePtr, int);
void HoldRight(vec3i* result, ShipData* shipIndex, Face* facePtr, int);
void HoldLeft(vec3i* result, ShipData* shipIndex, Face* facePtr, int);
void BlockShip(vec3i* blockVector, ShipData* shipIndex, Face* facePtr, int);
void AvoidShip(vec3i* blockVector, ShipData* shipIndex, Face* facePtr, int);
void AvoidOtherShips(vec3i* blockVector, ShipData* shipIndex, Face* facePtr, int);
void ZigZagShip(vec3i* blockVector, ShipData* shipIndex, Face* facePtr, int);
