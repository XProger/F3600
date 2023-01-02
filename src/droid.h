typedef struct RescueData
{
    TrackSection* section;
    short modelID;
    vec3i vel;
    vec3i acc;

    vec3i pos;
    int32 pitch, hdg, roll;
#ifdef USE_FRAME_LERP
    vec3i _pos;
    int32 _pitch, _hdg, _roll;
#endif

    int16 vhdg, vpitch, vroll;
    int16 updateCount;
    void (*droidUpdate)(struct RescueData*, ShipData*, Object**);
} RescueData;

void UpdateRescueDroid(ShipData* shipIndex, int shipNo);
void InitRescueDroid(RescueData* rescueDroid, Object** shipShapes, ShipData* shipIndex);
void UpdateDroidStart(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes);
void UpdateDroidCircle(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes);
void SetDroidVectors(RescueData* rescueDroid, Object** shipShapes);
void UpdateDroidRescue(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes);
void DrawDroid(Object* droid, const RescueData* rescueData);