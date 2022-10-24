#define COLLIDE_MODEL    16
#define NOSE_COLLISION   1
#define TAIL_COLLISION   2
#define R_WING_COLLISION 3
#define L_WING_COLLISION 4

void CallShip2ShipCollision(ShipData* shipIndex, Object** shipShapes);
void Ship2ShipCollision(ShipData* shipIndex, Object** shipShapes, int, int);
int CollideLineToPlane(int shipNo1, int shipNo2, Object** shipShapes, ShipData* shipIndex);
int PlaneLineCollision(const vec4i* plane_point, const vec4i* ship_pos, const vec3s* normal);
void Ship2Track(ShipData* playerShip, Face* facePtr);
void WingCollision(ShipData* playerShip, Face* facePtr, int direction, int alpha);
void NoseCollision(ShipData* playerShip, Face* facePtr, int direction, int alpha);
void Reflection(vec4i* incidence, const vec3s* normal);
int AngleOfCollision(const vec4s* normal, const vec4i* ship_point);
int CheckPolygonCollision(const Face* facePtr, const vec4i* pos, int alpha);
