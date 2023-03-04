#include "menus.h"

#define NO_OF_SHIPS     8

#define SHIP_MASS       150
#define SHIP_MASS_INV   ((1 << 16) / SHIP_MASS)

#define IN_TOW          0x1
#define REMOTE_VIEW     0x2
#define VIEW_IN         0x4
#define DIRECTION       0x8

#define FLYING          0x10
#define LSIDE           0x20
#define RACING          0x40
#define SHIP_COLL       0x80

#define VISIBLE         0x200
#define RESCUE          0x400
#define OVERTAKE        0x800

#define JUST_IN_FRONT   0x1000
#define JUNC_LEFT       0x2000
#define SHIELDED        0x4000
#define ELECTROED       0x8000

#define REVCONNED       0x10000
#define SPECIALED       0x20000

#define RESCUE_DROID    16

typedef struct ShipData
{
    /***** Dynamic Attributes *****/
    TrackSection *nearTrkSect, *prevShipSect;
    Face* face;
    int32 attr; /* Ship attributes */
    vec3i unitVecNose; /* Unit vector along ships nose */
    vec3i unitVecWing; /* Unit vector along ships nose */
    vec3i vel;
    vec3i thrust; /* Thrust vector */

    vec3i pos; /* 32bit ship position */
    int32 pitch, hdg, roll; /* Orientation attributes */
#ifdef USE_FRAME_LERP
    vec3i _pos;
    int32 _pitch, _hdg, _roll;
#endif

    int16 vhdg, vpitch, vroll;
    int16 ahdg;
    int16 brakeRotR, brakeRotL; /* Air brakes */
    uint16 speed; /* speed */
    int16 thrustMag; /* Thrust magnitude */
    int16 maxThrust; /* Miximum thrust */
    int16 startThrust;
    int16 headingInc, maxHeading;
    int16 resistance;
    int16 skid;

    /***** Remote Ship Attributes *****/
    int16 numtospeed;
    uint16 remoteThrustMag; /* Thrust magnitude */
    uint16 remoteMaxThrust; /* Miximum thrust */
    int16 DPASectDiff;

    /***** Weapon Attributes *****/
    int16 weaponType;
    int16 electroCount;
    int16 revConCount;
    int16 specialCount;

    /***** Race Control Attributes *****/
    int32 lapTime;
    int32 lapNo;

    int16 nearSecNo;
    int16 prevSecNo;

    int8 order;
    int8 racePosition;
    int8 racePoints;
    int8 totalPoints;

    int32 updateCount;

    int32 crossedLine;
    int32 shakeCount;

    uint32 color;

    Object* shape;
    void* player;

    struct ShipData* targetShip;

    /***** Control Routines *****/
    void (*updateControl)(vec3i*, struct ShipData*, Face*, int);
    void (*update)(struct ShipData*, int);

} ShipData;

void LoadShipData(void);
void DrawShips(ShipData* shipIndex);
void InitShipData(TrackSection* section, ShipData* shipIndex);
