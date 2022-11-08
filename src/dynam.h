#define GRAVITY 80000 /* Gravitational force              - 800    */
#define MIN_RESISTANCE 20 /* 12 */
#define MAX_RESISTANCE 74
#define VELOCITY_SHIFT 6
#define TRACK_MAGNET 64 /* 64 */
#define MIN_WING_SPEED 2000
#define TARGETHEIGHT 256

#define COLLIDE 0x1
#define frac(x, y) (((x) * (y)) >> FIXED_SHIFT) /* 9 */

void ShipCalcAmbient(ShipData* playerShip);
void ShipTrkReaction(ShipData* playerShip);
void GetWorldPoint(vec3i* output, Object* object, Object* objectCollide, int pointNo);
void GetTargetPos(vec3i* result, const vec3i* pos, const vec3i* cent1, const vec3i* cent2);
void SetUnitVectors(ShipData* shipIndex);
int SetTrackAttributes(TrackSection* section, short i, WeaponGrid* weaponGrid, int noWeaponGrids);
void ApplyMagnet(vec3i* acc, const vec3s* normal, int32 alpha);
