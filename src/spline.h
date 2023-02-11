/*    */

TrackSection* FindAllNearestSection(const vec3i* pos);

void UpdateShipRemote(ShipData* shipIndex, int shipNo);
void UpdateShipLinearInterpolateSpline(ShipData* shipIndex, int shipNo);

void UpdateCameraLinearInterpolateSpline(TrackCamera* camera, ShipData* ship, RescueData* rescueDroid);
void UpdateCameraHighUp(TrackCamera* camera, ShipData* ship, RescueData* rescueDroid);

void UpdateCameraExtroAttract(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid);

void InitFireCongratulations(void);
void InitIceCongratulations(void);

void ShowCongratulations(void);
