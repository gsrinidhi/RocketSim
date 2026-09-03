#ifndef IMUCLASS_HPP
#define IMUCLASS_HPP

#include "phyVector.h"
#include "quaternion.h"
#include "interface.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

class imuClass {
    Quaternion cmd_q;
    phyVector a;
    phyVector body_x, body_y, body_z;
    double dt;
    double roll_rate, pitch_rate, yaw_rate;
    double ax, ay, az;
    std::string initfileName;
    std::ifstream initfile;
    std::ifstream inputFile;
    std::ofstream *logfile;
    std::ofstream outputFile;
    public:
    imuClass();
    void setLogFile(std::ofstream *file);
    void getRotationRates(universalPkt imuInput);
    void getAcceleration(universalPkt imuInput);
    void getIMUOutput(universalPkt imuInput, universalPkt *navPkt);
    void setBodyAxes(Quaternion cmd_q);
};

#endif // IMUCLASS_HPP