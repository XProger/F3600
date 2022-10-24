/*    */

TrackSection* FindAllNearestSection(vec4i pos);

void UpdateShipRemote(ShipData* shipIndex, int shipNo, WeaponData* weaponIndex);
void UpdateShipLinearInterpolateSpline(ShipData* shipIndex, int shipNo, WeaponData* weaponIndex);

void UpdateCameraLinearInterpolateSpline(TrackCamera* camera, ShipData* ship, RescueData* rescueDroid);
void UpdateCameraHighUp(TrackCamera* camera, ShipData* ship, RescueData* rescueDroid);

void UpdateCameraExtroAttract(TrackCamera* camera, ShipData* camShip, RescueData* rescueDroid);

void InitFireCongratulations(void);
void InitIceCongratulations(void);

void ShowFireCongratulations(void);
void ShowIceCongratulations(void);
