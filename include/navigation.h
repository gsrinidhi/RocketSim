#ifndef NAVIGATION_H
#define NAVIGATION_H

#include<iostream>
#include<fstream>
#include<string>
#include <chrono>
#include <ctime>
#include <map>
#include <filesystem>
#include <vector>

#include"phyVector.h"
#include "quaternion.h"

#include "interface.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef GRAVITATIONAL_CONSTANT
#define GRAVITATIONAL_CONSTANT 6.67430e-11
#endif

class Navigation {

    double wx,wy,wz;
    double Me,Re;
    double altitude;
    double local_FPA;
    double time;
    double heading_angle;
    double theta_x,theta_y,theta_z;
    double polar_angle,azimuth_angle;
    phyVector s,v,a,Xv,Yv,Zv;
    phyVector v_prev,s_prev;
    phyVector body_x,body_y,body_z;
    phyVector x1,x2,x3,y1,y2,y3,z1,z2,z3;
    Quaternion cmd_q;
    std::ofstream *logfile;
    std::ifstream *initfile;
    std::string initfile_name;
    std::map<std::string, double> configMap;
    std::map<std::string, std::string> configMapString;
    public:
    Navigation();
    void setEarthSpecs(double Me, double Re);
    void setLogFile(std::ofstream *file);
    void setBodyAxes(phyVector body_x,phyVector body_y,phyVector body_z);
    void initNav(navInitPkt initPkt);
    void initNavFile(std::string fname);
    void getNavOutput(universalPkt *navPkt, universalPkt *guidPkt);
};

#endif