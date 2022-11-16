#include "game.h"

typedef struct
{
    int32 volL, volR;
    int32 loop;
    int32 pitch;
} SfxParam;

extern SfxParam gSfxParams[SFX_MAX];

void soundInit(void);

void SfxInit(void);
void SfxFree(void);

void AdjustDirectionalNote(enum SfxType id, const vec3i* pos);
void AdjustRemoteEngineSound(ShipData* shipIndex, Object** shipShapes);
void AdjustEngineSound(ShipData* playerShip, TrackCamera* camera);
int PlayDirectionalNote(enum SfxType id, const vec3i* pos);
