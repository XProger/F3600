typedef struct RescueData
{
    TrackSection* section;
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

    ShipData* ship;
    void (*update)(struct RescueData*);
} RescueData;

void UpdateRescueDroid(ShipData* shipIndex, int shipNo);
void InitRescueDroid(RescueData* rescueDroid, ShipData* ship);
void UpdateDroidStart(RescueData* rescueDroid);
void UpdateDroidCircle(RescueData* rescueDroid);
void SetDroidVectors(RescueData* rescueDroid);
void UpdateDroidRescue(RescueData* rescueDroid);
void DrawDroid(Object* droid, const RescueData* rescueData);