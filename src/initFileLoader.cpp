#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <stdlib.h>
#include <signal.h>
#include "interface.h"
#include <fcntl.h>
#include <unistd.h>
#include <phyVector.h>
#include "navigation.h"
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

int main (int argc, char *argv[]) {

    std::string configFile;
    std::map<std::string, double> configMap;
    std::map<std::string, std::string> configMap_string;

    guidInitPkt guidInitPacket;
    navInitPkt navInitPAcket;

    std::string navInitFIFO,guidInitFIFO;

    struct sigaction sa;

    int navInit_fd,guidInit_fd;

    

    if(argc > 1) {
        configFile = argv[1];
    } else {
        configFile = "input/initFile.txt";
    }

    std::ifstream file(configFile);

    std::cout<<"Config file is " << configFile <<std::endl;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return 1;
    }

    std::string line;

    // 2. Read the file line by line
    while (std::getline(file, line)) {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string ftype;
        std::string key;
        std::string str_value;
        double value;

        // 3. Extract key and value separated by whitespace
        // if (ss >> key >> value) {
        //     configMap[key] = value;
        // }
        ss >> ftype;
        if (ftype == "STRING"){
            ss >> key >> str_value;
            configMap_string[key] = str_value;
        } else if(ftype == "DOUBLE") {
            ss >> key >> value;
            configMap[key] = value;
        }
    }

    // 4. Close the file stream
    file.close();

    guidInitPacket.polar_angle = configMap["Init_Polar_Angle"];
    guidInitPacket.inertial_azimuth = configMap["Init_Inertial_Azimuth"];
    guidInitPacket.inclination = configMap["Inclination"];
    guidInitPacket.tower_clearance_altitude = configMap["Tower_Clearance_Altitude"];
    guidInitPacket.mode_1_2_change_altitude = configMap["Alt_1_2"];
    guidInitPacket.mode_2_3_change_altitude = configMap["Alt_2_3"];
    guidInitPacket.mode_2_pitch = configMap["Mode_2_Pitch"];
    guidInitPacket.apoapsis_target = configMap["Apoapsis_Target"];
    guidInitPacket.periapsis_target = configMap["Periapsis_Target"];
    guidInitPacket.Me = configMap["Me"];
    guidInitPacket.Re = configMap["Re"];
    if(configMap_string.size() > 190) {
        std::cout << "Log file name out of bounds" << std::endl;
        return 0;
    }
    strcpy(guidInitPacket.dataLogFile,configMap_string["dataLogFile"].data());
    guidInitPacket.dataLogFile[configMap_string.size()] = '\0';

    navInitPAcket.Me = configMap["Me"];
    navInitPAcket.Re = configMap["Re"];
    navInitPAcket.phi_init = configMap["Init_Polar_Angle"];
    navInitPAcket.theta_init = configMap["Init_Inertial_Azimuth"];

    navInitFIFO = configMap_string["navInitFIFO"];
    guidInitFIFO = configMap_string["guidInitFIFO"];

    int write_bytes = 0;

    navInit_fd = open(navInitFIFO.data(),O_WRONLY);
    write_bytes = write(navInit_fd,&navInitPAcket,sizeof(navInitPkt));
    while(write_bytes == -1) {
        write_bytes = write(navInit_fd,&navInitPAcket,sizeof(navInitPkt));
    }
    close(navInit_fd);

    guidInit_fd = open(guidInitFIFO.data(),O_WRONLY);
    write_bytes = write(guidInit_fd,&guidInitPacket,sizeof(guidInitPkt));
    while(write_bytes == -1) {
        write_bytes = write(guidInit_fd,&guidInitPacket,sizeof(guidInitPkt));
    }
    close(guidInit_fd);





    
    return 0;
}