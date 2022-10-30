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

#define NEW_SHIP        0
#define RESCUE_DROID    16

struct WeaponData;
struct TrackCamera;

typedef struct ShipData
{
    /***** Graphics Attributes *****/
    short highResTableIndex; /* Index into shipShapes for high-res model */
    short lowResTableIndex; /* Index into shipShapes for low-res model */

    /***** Dynamic Attributes *****/
    TrackSection *nearTrkSect, *prevShipSect;
    int attr; /* Ship attributes */
    int combatAttr; /* Ship attributes */
    vec3i unitVecNose; /* vec4s  ? */ /* Unit vector along ships nose */
    vec3i unitVecWing; /* vec4s  ? */ /* Unit vector along ships nose */
    vec3i pos; /* 32bit ship position */
    vec3i vel, acc; /* Velocity, accn vectors for ship */
    vec3i thrust; /* Thrust vector */
    short hdg, pitch, roll; /* Orientation attributes */
    short vhdg, vpitch, vroll;
    short ahdg;
    short r_brake_rot, l_brake_rot; /* Air brakes */
    uint16 speed; /* speed */
    short thrust_mag; /* Thrust magnitude */
    short max_thrust; /* Miximum thrust */
    short headingInc, maxHeading;
    short resistance;
    short skid;

    /***** Remote Ship Attributes *****/
    short fightBack, numtospeed;
    uint16 remoteThrustMag; /* Thrust magnitude */
    uint16 remoteMaxThrust; /* Miximum thrust */
    short DPASectDiff;

    /***** Weapon Attributes *****/
    uint8 weaponType;
    uint8 targetShip;
    short haveFired;
    short electroCount;
    short revConCount;
    short specialCount;

    /***** Race Control Attributes *****/
    int lapTime;
    char lapNo;
    char prevLapNo;
    char position;
    short nearSecNo, prevSecNo;
    int lapTimes[5];
    int updateCount;

    uint32 color;

    /***** Control Routines *****/
    void (*updateControl)(vec3i*, struct ShipData*, Face*, int);
    void (*update)(struct ShipData*, int);

} ShipData;

void loadShipData(ShipData* shipIndex, Object** shipShapes, Skeleton* camPos);
void DrawShips(ShipData* shipIndex, Object** shipShapes, int camShip);
void initShipData(TrackSection* section, ShipData* shipIndex, ConfigData* gameData);
