#include "game.h"

typedef struct
{
    int32 volL, volR;
    int32 loop;
    int32 pitch;
} SfxParam;

extern SfxParam gSfxParams[SFX_MAX];

void SfxInit(void);
void SfxFree(void);

void AdjustDirectionalNote(enum SfxType id, int32 x, int32 z);
void AdjustRemoteEngineSound(ShipData* shipIndex);
void AdjustEngineSound(ShipData* ship, TrackCamera* camera);
int PlayDirectionalNote(enum SfxType id, const vec3i* pos);
