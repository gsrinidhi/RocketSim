#ifndef INTERFACE_H
#define INTERFACE_H

#include "phyVector.h"
#include "quaternion.h"
#include <string>

typedef struct simToGuidPkt {
    phyVector s;
    phyVector v;
    phyVector Xv;
    phyVector Yv;
    phyVector Zv;
    double local_FPA;
    double time;
    double heading_angle;
};

typedef struct guidInitPkt {
    double Me;
    double Re;
    phyVector s;
    double phi_init;
    double theta_init;
    double inclination;
    double polar_angle;
    double inertial_azimuth;
    double tower_clearance_altitude;
    double mode_1_2_change_altitude;
    double mode_2_3_change_altitude;
    double mode_2_pitch;
    double apoapsis_target;
    double periapsis_target;
    char dataLogFile[200];
};

typedef struct guidToSimPkt {
    double heading_angle;
    Quaternion cmd_q;
    int isThrusting;
    double apogee;
    double perigee;
};

typedef struct navInitPkt {
    phyVector s;
    phyVector v;
    phyVector Xv;
    phyVector Yv;
    phyVector Zv;
    phyVector body_x;
    phyVector body_y;
    phyVector body_z;
    double Me;
    double Re;
    double local_FPA;
    double time;
    double heading_angle;
    double phi_init;
    double theta_init;
    double inclination;
};

typedef struct imuToNavPkt {
    double wx;
    double wy;
    double wz;
    double ax;
    double ay;
    double az;
    double dt;
};

typedef struct navToGuidPkt {
    phyVector s;
    phyVector v;
    phyVector Xv;
    phyVector Yv;
    phyVector Zv;
    double local_FPA;
    double time;
};

typedef struct imuInputPkt {
    Quaternion cmd_q;
    phyVector a;
    double dt;
};

typedef struct universalPkt {
    double local_FPA;
    double time;
    double heading_angle;
    double dt;
    double wx;
    double wy;
    double wz;
    double ax;
    double ay;
    double az;
    int isThrusting;
    double apogee;
    double perigee;
    phyVector s;
    phyVector v;
    phyVector a;
    phyVector Xv;
    phyVector Yv;
    phyVector Zv;
    Quaternion cmd_q;
};

#endif