typedef struct RescueData
{
    TrackSection* section;
    short modelID;
    vec4i pos;
    vec4i vel;
    vec4i acc;
    short hdg, pitch, roll;
    short vhdg, vpitch, vroll;
    short updateCount;
    void (*droidUpdate)(struct RescueData*, ShipData*, Object**);
} RescueData;

void UpdateRescueDroid(ShipData* shipIndex, int shipNo);
void InitRescueDroid(RescueData* rescueDroid, Object** shipShapes, ShipData* shipIndex);
void UpdateDroidStart(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes);
void UpdateDroidCircle(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes);
void SetDroidVectors(RescueData* rescueDroid, Object** shipShapes);
void UpdateDroidRescue(RescueData* rescueDroid, ShipData* shipIndex, Object** shipShapes);
void DrawDroid(Object* droid, const RescueData* rescueData);