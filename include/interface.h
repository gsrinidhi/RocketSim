#ifndef INTERFACE_H
#define INTERFACE_H

#include "phyVector.h"
#include "quaternion.h"

typedef struct simToGuidPkt {
    phyVector s;
    phyVector v;
    phyVector Xv;
    phyVector Yv;
    phyVector Zv;
    double local_FPA;
    double time;
    double heading_angle;
} __attribute__((packed)) ;

typedef struct simToGuidInitPkt {
    double Me;
    double Re;
    phyVector s;
    double phi_init;
    double theta_init;
    double inclination;
} __attribute__((packed)) ;

typedef struct guidToSimPkt {
    double heading_angle;
    Quaternion cmd_q;
    int isThrusting;
    double apogee;
    double perigee;
}__attribute__((packed)) ;


#endif