#include "game.h"

void SfxInit(void);
void SfxFree(void);

short AdjustDirectionalNote(enum SfxType id, const VECTOR* pos);
short AdjustRemoteEngineSound(ShipData* shipIndex, Object** shipShapes, TrackCamera* camera);
void AdjustEngineSound(ShipData* playerShip, TrackCamera* camera);
int PlayDirectionalNote(enum SfxType id, const VECTOR* pos);
