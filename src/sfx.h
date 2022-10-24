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

short AdjustDirectionalNote(enum SfxType id, const vec4i* pos);
short AdjustRemoteEngineSound(ShipData* shipIndex, Object** shipShapes, TrackCamera* camera);
void AdjustEngineSound(ShipData* playerShip, TrackCamera* camera);
int PlayDirectionalNote(enum SfxType id, const vec4i* pos);
