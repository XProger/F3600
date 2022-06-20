#define NORMALPAD 0
#define HRP 0

#define JUST_FRONT_DELAY   (15*FR10)
#define JUST_BEHIND_DELAY  (20*FR10)
#define IN_SIGHT_DELAY     (20*FR10)
#define STALL_DELAY        (9 * FR10)
#define START_TIME         (20*FR10)
#define THREE              (15*FR10)
#define RED_LIGHT          (10*FR10)
#define AMBER_LIGHT        (5*FR10)
#define RACE_START         0

#define SWITCH_TO_RACE_VIEW 100

#define STALL              -1

void  UpdateShips(ShipData *shipIndex, WeaponShape *weaponShapes, Object** shipShapes, WeaponData*);
int   InitController();
void  GeneralStartProcs(ShipData *playerShip);

void  UpdatePlayerShipNorm       (ShipData*, Object**, int, WeaponData*);
void  UpdateRemoteShip           (ShipData*, Object**, int, WeaponData*);
void  UpdatePlayerShipNormStart     (ShipData*, Object**, int, WeaponData*);
void  UpdateRemoteShipStart         (ShipData*, Object**, int, WeaponData*);

void  HoldCentre                 (VECTOR *result, ShipData *shipIndex, Face *facePtr, int);
void  HoldRight                  (VECTOR *result, ShipData *shipIndex, Face *facePtr, int);
void  HoldLeft                   (VECTOR *result, ShipData *shipIndex, Face *facePtr, int);
void  BlockShip                  (VECTOR *blockVector, ShipData *shipIndex, Face *facePtr, int);
void  AvoidShip                  (VECTOR *blockVector, ShipData *shipIndex, Face *facePtr, int);
void  AvoidOtherShips            (VECTOR *blockVector, ShipData *shipIndex, Face *facePtr, int);
void  ZigZagShip                 (VECTOR *blockVector, ShipData *shipIndex, Face *facePtr, int);

void SetSkeletonDirectionHPR( Skeleton* skeleton, short hdg, short pitch, short roll  );
void SetSkeletonDirectionRPH( Skeleton* skeleton, short roll, short pitch, short hdg  );

void DoDetailKeys(void);
