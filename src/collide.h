#define NOSE_COLLISION   1

void CallShip2ShipCollision(ShipData* shipIndex);
int PlaneLineCollision(const vec3i* plane_point, const vec3i* ship_pos, const vec3s* normal);
void Ship2Track(ShipData* ship, Face* facePtr);
void WingCollision(ShipData* ship, Face* facePtr, int direction, int alpha);
void NoseCollision(ShipData* ship, Face* facePtr, int direction, int alpha);
void Reflection(vec3i* incidence, const vec3s* normal);
int AngleOfCollision(const vec3s* normal, const vec3i* ship_point);
int CheckPolygonCollision(const Face* facePtr, const vec3i* pos, int alpha);
