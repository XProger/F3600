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

#define ON_JUNCTION     0x100
#define VISIBLE         0x200
#define RESCUE          0x400
#define OVERTAKE        0x800

#define JUST_IN_FRONT   0x1000
#define JUNC_LEFT       0x2000
#define SHIELDED        0x4000
#define ELECTROED       0x8000

#define REVCONNED       0x1
#define SPECIALED       0x2

#define RESCUE_DROID    16

struct WeaponData;
struct TrackCamera;

typedef struct ShipData
{
    /***** Graphics Attributes *****/
    int16 highResTableIndex; /* Index into shipShapes for high-res model */
    int16 lowResTableIndex; /* Index into shipShapes for low-res model */

    /***** Dynamic Attributes *****/
    TrackSection *nearTrkSect, *prevShipSect;
    int32 attr; /* Ship attributes */
    int32 combatAttr; /* Ship attributes */
    vec3i unitVecNose; /* vec4s  ? */ /* Unit vector along ships nose */
    vec3i unitVecWing; /* vec4s  ? */ /* Unit vector along ships nose */
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
    int16 r_brake_rot, l_brake_rot; /* Air brakes */
    uint16 speed; /* speed */
    int16 thrust_mag; /* Thrust magnitude */
    int16 max_thrust; /* Miximum thrust */
    int16 headingInc, maxHeading;
    int16 resistance;
    int16 skid;

    /***** Remote Ship Attributes *****/
    int16 fightBack, numtospeed;
    uint16 remoteThrustMag; /* Thrust magnitude */
    uint16 remoteMaxThrust; /* Miximum thrust */
    int16 DPASectDiff;

    /***** Weapon Attributes *****/
    uint8 weaponType;
    uint8 targetShip;
    int16 haveFired;
    int16 electroCount;
    int16 revConCount;
    int16 specialCount;

    /***** Race Control Attributes *****/
    int32 lapTime;
    int8 lapNo;
    int8 position;
    int16 nearSecNo, prevSecNo;
    int32 lapTimes[5];
    int32 updateCount;

    uint32 color;

    /***** Control Routines *****/
    void (*updateControl)(vec3i*, struct ShipData*, Face*, int);
    void (*update)(struct ShipData*, int);

} ShipData;

void LoadShipData(ShipData* shipIndex, Object** shipShapes);
void DrawShips(ShipData* shipIndex, Object** shipShapes, int camShip);
void InitShipData(TrackSection* section, ShipData* shipIndex);
