#ifndef CGUIDANCE_H
#define CGUIDANCE_H

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

class CGuidance {
    int mode;
    int subMode;
    double Me,Re;
    double altitude;
    double tower_clearance_altitude;
    double mode_1_2_change_altitude = 1000;
    double mode_2_3_change_altitude = 10.5e3; // Altitude at which to switch from mode 2 to mode 3
    double mode_2_pitch = 60 * PI / 180; // Initial pitch angle for mode 2 in radians
    double isThrusting;
    double apoapsis_altitude,periapsis_altitude;
    double apoapsis_threshold,periapsis_threshold;
    double apoapsis_target,periapsis_target;
    double burn1_end_time,coast_end_time,injection_time;
    double polar_angle,inertial_azimuth,inclination;
    std::map<std::string, double> configMap;
    phyVector incV,hv;
    phyVector cmd_x,cmd_y,cmd_z;
    std::ofstream *logfile;
    std::ofstream *dataLogFile;
public:
    
    CGuidance();
    CGuidance(double Me, double Re);
    void setEarthSpecs(double Me, double Re);
    void setMode_1_2_change_altitude(double alt);
    void setMode_2_3_change_altitude(double alt);
    void setMode_2_pitch(double rad);
    void setAltitudeThresholds(double apo_thr, double peri_thr);
    void setAltitudeTarget(double apo_tar,double peri_tar);
    void setLogFile(std::ofstream *file);
    void guidInit(phyVector s, double polar_angle,double inertial_azimuth,double inclination);
    int guidInitFile(std::string fname);
    int getGuidanceOutput(phyVector s, phyVector v, double local_FPA,double time,double *commanded_pitch,int *isThrusting) ;
    int getGuidanceOutputQuat(phyVector s, phyVector v, double local_FPA,double time,double heading_angle,phyVector Xv, phyVector Yv,phyVector Zv,Quaternion &cmd_q,int *isThrusting) ;
    void getApoapsisPeriapsis(phyVector s, phyVector v, double M, double R,double &apoapsis_altitude, double &periapsis_altitude);
    int getHeading(phyVector s, phyVector Xv, double *commanded_heading);
    double getApoapsisPeriapsis(double *apo, double *peri);
    double getParam(std::string pname);
    void guidInit(guidInitPkt guidInitPacket);
    void setDataLogFile(std::ofstream *file);
    ~CGuidance();
};

#endif
