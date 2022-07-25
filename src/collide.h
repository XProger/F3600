#define COLLIDE_MODEL    16
#define NOSE_COLLISION   1
#define TAIL_COLLISION   2
#define R_WING_COLLISION 3
#define L_WING_COLLISION 4

void CallShip2ShipCollision(ShipData* shipIndex, Object** shipShapes);
void Ship2ShipCollision(ShipData* shipIndex, Object** shipShapes, int, int);
int CollideLineToPlane(int shipNo1, int shipNo2, Object** shipShapes, ShipData* shipIndex);
int PlaneLineCollision(const VECTOR* plane_point, const VECTOR* ship_pos, const SVector* normal);
void Ship2Track(ShipData* playerShip, Face* facePtr, Object** shipShapes);
void WingCollision(ShipData* playerShip, Face* facePtr, int direction, int alpha);
void NoseCollision(ShipData* playerShip, Face* facePtr, int direction, int alpha);
void Reflection(VECTOR* incidence, const SVector* normal);
int AngleOfCollision(const SVECTOR* normal, const VECTOR* ship_point);
int CheckPolygonCollision(const Face* facePtr, const VECTOR* pos, int alpha);
