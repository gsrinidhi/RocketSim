#include "navigation.h"

Navigation::Navigation() {
    wx = 0;
    wy = 0;
    wz = 0;
    Me = 5.972e24; // Default mass of Earth in kg
    Re = 6371000; // Default radius of Earth in meters
    altitude = 0;
    local_FPA = 0;
    time = 0;
    heading_angle = 0;
    s = phyVector(0,0,0);
    v = phyVector(0,0,0);
    Xv = phyVector(1,0,0);
    Yv = phyVector(0,1,0);
    Zv = phyVector(0,0,1);
    body_x = phyVector(1,0,0);
    body_y = phyVector(0,1,0);
    body_z = phyVector(0,0,1);
}

void Navigation::setEarthSpecs(double Me, double Re) {
    this->Me = Me;
    this->Re = Re;
}

void Navigation::setLogFile(std::ofstream *file) {
    logfile = file;
}

void Navigation::setBodyAxes(phyVector body_x, phyVector body_y, phyVector body_z) {
    this->body_x = body_x;
    this->body_y = body_y;
    this->body_z = body_z;
}

void Navigation::initNav(navInitPkt initPkt) {
    s = initPkt.s;
    v = initPkt.v;
    Xv = initPkt.Xv;
    Yv = initPkt.Yv;
    Zv = initPkt.Zv;
    Me = initPkt.Me;
    Re = initPkt.Re;
    local_FPA = initPkt.local_FPA;
    time = initPkt.time;
    heading_angle = initPkt.heading_angle;
    body_x = initPkt.body_x;
    body_y = initPkt.body_y;
    body_z = initPkt.body_z;
    s_prev = s;
    v_prev = v;
}

void Navigation::getNavOutput(universalPkt *navPkt, universalPkt *guidPkt) {
    //Rotate body y and body x axis by wz * dt to get y1 and x1
    theta_x = navPkt->wx * navPkt->dt;
    theta_y = navPkt->wy * navPkt->dt;
    theta_z = navPkt->wz * navPkt->dt;

    x1 = body_x * cos(theta_z) + body_y * sin(theta_z);
    y1 = body_y * cos(theta_z) - body_x * sin(theta_z);
    z1 = body_z;

    //Rotate x1 and z1 by theta_y to get x2 and z2
    x2 = x1 * cos(theta_y) - z1 * sin(theta_y);
    z2 = z1 * cos(theta_y) + x1 * sin(theta_y);
    y2 = y1;

    //Rotate z2 and y2 by theta_x to get z3 and y3
    x3 = x2;
    y3 = y2 * cos(theta_x) + z2 * sin(theta_x);
    z3 = z2 * cos(theta_x) - y2 * sin(theta_x);

    body_x = x3;
    body_y = y3;
    body_z = z3;

    //Get acceleration in inertial frame
    a = x3 * navPkt->ax + y3 * navPkt->ay + z3 * navPkt->az;
    v = v_prev + a * navPkt->dt;
    s = s_prev + v * navPkt->dt;

    v_prev = v;
    s_prev = s;

    polar_angle = atan2(s.magnitude(1,1,0),s.z);
    azimuth_angle = atan2(s.y,s.x);

    Xv.x = -cos(polar_angle) * cos(azimuth_angle);
    Xv.y = -cos(polar_angle) * sin(azimuth_angle);
    Xv.z = sin(polar_angle);

    Yv.x = sin(azimuth_angle);
    Yv.y = -cos(azimuth_angle);

    Zv.x = sin(polar_angle) * cos(azimuth_angle);
    Zv.y = sin(polar_angle) * sin(azimuth_angle);
    Zv.z = cos(polar_angle);

    double vxv = v * Xv;
    double vyv = v * Yv;
    double vzv = v * Zv;

    local_FPA = atan2(vzv,sqrt(vxv * vxv + vyv * vyv));

    time = time + navPkt->dt;


    // Populate the guidance packet with current state
    guidPkt->s = s;
    guidPkt->v = v;
    guidPkt->Xv = Xv;
    guidPkt->Yv = Yv;
    guidPkt->Zv = Zv;
    guidPkt->local_FPA = local_FPA;
    guidPkt->time = time;
}