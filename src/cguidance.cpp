#include<iostream>
#include "cguidance.h"

CGuidance::CGuidance() {
    mode = 1;
    subMode = 0;
    Re = 6371000;
    Me = 5.972e24;
}

CGuidance::CGuidance(double Me, double Re) {
    mode = 1;
    subMode = 0;
    this->Me = Me;
    this->Re = Re;
}

void CGuidance::setEarthSpecs(double Me, double Re) {
    this->Me = Me;
    this->Re = Re;
}

void CGuidance::setMode_1_2_change_altitude(double alt) {
    mode_1_2_change_altitude = alt;
}

void CGuidance::setMode_2_3_change_altitude(double alt) {
    mode_2_3_change_altitude = alt;
}

void CGuidance::setMode_2_pitch(double rad) {
    mode_2_pitch = rad;
}

void CGuidance::getApoapsisPeriapsis(phyVector s, phyVector v, double M, double R,double &apoapsis_altitude, double &periapsis_altitude) {
    double r = s.magnitude();
    double v_mag = v.magnitude();
    double specific_energy = (v_mag * v_mag) / 2 - GRAVITATIONAL_CONSTANT * M / r; // Specific orbital energy
    double semi_major_axis = - GRAVITATIONAL_CONSTANT * M / (2 * specific_energy); // Semi-major axis of the orbit
    double eccentricity = sqrt(1 + (2 * specific_energy * (s^v).magnitude() * (s^v).magnitude()) / (GRAVITATIONAL_CONSTANT * M * GRAVITATIONAL_CONSTANT * M)); // Eccentricity of the orbit
    apoapsis_altitude = semi_major_axis * (1 + eccentricity) - R; // Altitude of apoapsis
    periapsis_altitude = semi_major_axis * (1 - eccentricity) - R; // Altitude of periapsis
}

void CGuidance::setAltitudeThresholds(double apo_thr, double peri_thr) {
    apoapsis_threshold = apo_thr;
    periapsis_threshold = peri_thr;
}

void CGuidance::setAltitudeTarget(double apo_tar,double peri_tar) {
    apoapsis_target = apo_tar;
    periapsis_target = peri_tar;
}

void CGuidance::setLogFile(std::ofstream *file) {
    logfile = file;
}

void CGuidance::guidInit(phyVector s, double polar_angle,double inertial_azimuth,double inclination) {
    incV.setXYZ(sin(inclination) * sin(inertial_azimuth),sin(inclination)*cos(inertial_azimuth),cos(inclination));
}

int CGuidance::guidInitFile(std::string fname) {
    std::ifstream file(fname);
    
    // Check if the file opened successfully
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
        std::string key;
        double value;

        // 3. Extract key and value separated by whitespace
        if (ss >> key >> value) {
            configMap[key] = value;
        }
    }

    // 4. Close the file stream
    file.close();

    polar_angle = configMap["Init_Polar_Angle"] * PI/180;
    inertial_azimuth = configMap["Init_Inertial_Azimuth"] * PI /180;
    inclination = configMap["Inclination"] * PI / 180;
    tower_clearance_altitude = configMap["Tower_Clearance_Altitude"];
    mode_1_2_change_altitude = configMap["Alt_1_2"];
    mode_2_3_change_altitude = configMap["Alt_2_3"];
    mode_2_pitch = configMap["Mode_2_Pitch"] * PI / 180;
    apoapsis_target = configMap["Apoapsis_Target"];
    periapsis_target = configMap["Periapsis_Target"];
    incV.setXYZ(sin(inclination) * sin(inertial_azimuth),sin(inclination)*cos(inertial_azimuth),cos(inclination));
}

double CGuidance::getParam(std::string pname) {
    return configMap[pname];
}

int CGuidance::getHeading(phyVector s, phyVector Xv, double *commanded_heading) {
    hv = (s ^ incV)/s.magnitude();

    double kop = hv * Xv;
    if(kop > 1) {
        kop = 1;
    }
    if(kop < -1) {
        kop = -1;
    }

    *commanded_heading = acos(kop);
}

int CGuidance::getGuidanceOutput(phyVector s, phyVector v, double local_FPA,double time,double *commanded_pitch,int *isThrusting) {
    altitude = s.magnitude() - Re;
    getApoapsisPeriapsis(s, v, Me, Re, apoapsis_altitude, periapsis_altitude);
        if(mode == 1) {
            //Mode 1: Vertical ascent
            *commanded_pitch = 90 * PI / 180; // Keep the rocket vertical
            *isThrusting = 1;
            if(altitude >= mode_1_2_change_altitude) {
                mode = 2; // Switch to mode 2 when reaching 1km altitude
            }
            
        } else if (mode == 2) {
            //Mode 2: Gravity turn initiation
            *commanded_pitch = mode_2_pitch; // Start pitching over to the initial pitch angle for gravity turn
            subMode = 1; // Sub-mode for gravity turn initiation
            *isThrusting = 1;
            if(altitude >= mode_2_3_change_altitude) {
                mode = 3; // Switch to mode 3 when reaching the altitude to switch to gravity turn continuation
            }
        } else if (mode == 3) {
            //Mode 3: Gravity turn continuation
            if (subMode == 1) {
                *isThrusting = 1; // Ensure engine is on during gravity turn
                // Make pitch angle equal to flight path angle
                if(v.x != 0 || v.z!= 0) {
                    *commanded_pitch = local_FPA;
                } else {
                    
                }
                if(apoapsis_altitude >= apoapsis_threshold) {
                    apoapsis_threshold += 50e3; // Switch to sub-mode 2 when approaching the altitude to switch to mode 3
                }
                if(apoapsis_altitude >= apoapsis_target) {
                    (*logfile) << "Burn 1 duration = " <<time << std::endl;
                    // *logfile << "Radial Velocity = " << radial_velocity << " m/s" << std::endl;
                    (*logfile) << "Altitude = " << altitude << " m" << std::endl;
                    burn1_end_time = time;
                    subMode = 2; // Switch to sub-mode 2 when approaching target altitude
                }
            } else if (subMode == 2) {
                *isThrusting = 0;
                // go to submode 3 if altitude is reached
                if (altitude >= apoapsis_altitude - 5e2) {
                    coast_end_time = time;
                    // *logfile << "Coast duration = " <<coast_end_time - burn1_end_time << endl;
                    subMode = 3;
                    periapsis_threshold = periapsis_altitude + 100e3;
                }
                
            } else if (subMode == 3) {
                *isThrusting = 1; // Fire until perigee is raised to target altitude
                *commanded_pitch = local_FPA;
                if(periapsis_altitude > periapsis_threshold) {
                    periapsis_threshold += 100e3;
                }
                if(periapsis_altitude >= periapsis_target) {
                    injection_time = time; // Record the time of engine cutoff for reference
                    // res = "Injection time";
                    // std::cout << "Burn 2 duration = " <<(injection_time - coast_end_time) << endl;
                    // std::cout << "Injection time = " << injection_time << endl; 
                    // std::cout << "Injection velocity = " << v.magnitude() <<endl;
                    // std::cout << "Acheived perigee = " << periapsis_altitude << endl;
                    // std::cout << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    (*logfile) << "Burn 2 duration = " <<(injection_time - coast_end_time) << std::endl;
                    (*logfile) << "Injection time = " << injection_time << std::endl; 
                    (*logfile) << "Injection velocity = " << v.magnitude() << std::endl;
                    (*logfile) << "Acheived perigee = " << periapsis_altitude << std::endl;
                    // logfile << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    subMode = 4; // Switch to coasting when perigee is raised to target altitude
                }
            } else if (subMode == 4) {
                isThrusting = 0; // Coasting phase after reaching target orbit
                mode = 4; // Switch to mode 4 for coasting
            }
        } else {
            //Mode 4: Coasting phase
            *isThrusting = 0; // Engine is off during coasting
        }
}

int CGuidance::getGuidanceOutputQuat(phyVector s, phyVector v, double local_FPA,double time,double heading_angle,phyVector Xv, phyVector Yv,phyVector Zv,Quaternion &cmd_q,int *isThrusting) {
    altitude = s.magnitude() - Re;
    getApoapsisPeriapsis(s, v, Me, Re, apoapsis_altitude, periapsis_altitude);
    double commanded_pitch = 0, commanded_heading = heading_angle;
        if(mode == 1) {
            //Mode 1: Vertical ascent
            commanded_pitch = 90 * PI / 180; // Keep the rocket vertical
            *isThrusting = 1;
            if(altitude >= tower_clearance_altitude) {
                (*logfile) << "Tower clearance altitude reached at time = " << time << std::endl;
                commanded_heading = heading_angle;
            } else {
                commanded_heading = 0; // Keep heading straight up during tower clearance
            }
            if(altitude >= mode_1_2_change_altitude) {
                mode = 2; // Switch to mode 2 when reaching 1km altitude
            }
            
        } else if (mode == 2) {
            //Mode 2: Gravity turn initiation
            commanded_pitch = mode_2_pitch; // Start pitching over to the initial pitch angle for gravity turn
            commanded_heading = heading_angle; // Maintain the commanded heading during gravity turn initiation
            subMode = 1; // Sub-mode for gravity turn initiation
            *isThrusting = 1;
            if(altitude >= mode_2_3_change_altitude) {
                mode = 3; // Switch to mode 3 when reaching the altitude to switch to gravity turn continuation
            }
        } else if (mode == 3) {
            commanded_heading = heading_angle; // Maintain the commanded heading during gravity turn continuation
            //Mode 3: Gravity turn continuation
            if (subMode == 1) {
                *isThrusting = 1; // Ensure engine is on during gravity turn
                // Make pitch angle equal to flight path angle
                if(v.x != 0 || v.z!= 0) {
                    commanded_pitch = local_FPA;
                } else {
                    
                }
                if(apoapsis_altitude >= apoapsis_threshold) {
                    apoapsis_threshold += 50e3; // Switch to sub-mode 2 when approaching the altitude to switch to mode 3
                }
                if(apoapsis_altitude >= apoapsis_target) {
                    (*logfile) << "Burn 1 duration = " <<time << std::endl;
                    // *logfile << "Radial Velocity = " << radial_velocity << " m/s" << std::endl;
                    (*logfile) << "Altitude = " << altitude << " m" << std::endl;
                    burn1_end_time = time;
                    subMode = 2; // Switch to sub-mode 2 when approaching target altitude
                }
            } else if (subMode == 2) {
                *isThrusting = 0;
                // go to submode 3 if altitude is reached
                if (altitude >= apoapsis_altitude - 5e2) {
                    coast_end_time = time;
                    // *logfile << "Coast duration = " <<coast_end_time - burn1_end_time << endl;
                    subMode = 3;
                    periapsis_threshold = periapsis_altitude + 100e3;
                }
                
            } else if (subMode == 3) {
                *isThrusting = 1; // Fire until perigee is raised to target altitude
                commanded_pitch = local_FPA;
                if(periapsis_altitude > periapsis_threshold) {
                    periapsis_threshold += 100e3;
                }
                if(periapsis_altitude >= periapsis_target) {
                    injection_time = time; // Record the time of engine cutoff for reference
                    // res = "Injection time";
                    // std::cout << "Burn 2 duration = " <<(injection_time - coast_end_time) << endl;
                    // std::cout << "Injection time = " << injection_time << endl; 
                    // std::cout << "Injection velocity = " << v.magnitude() <<endl;
                    // std::cout << "Acheived perigee = " << periapsis_altitude << endl;
                    // std::cout << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    (*logfile) << "Burn 2 duration = " <<(injection_time - coast_end_time) << std::endl;
                    (*logfile) << "Injection time = " << injection_time << std::endl; 
                    (*logfile) << "Injection velocity = " << v.magnitude() << std::endl;
                    (*logfile) << "Acheived perigee = " << periapsis_altitude << std::endl;
                    // logfile << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    subMode = 4; // Switch to coasting when perigee is raised to target altitude
                }
            } else if (subMode == 4) {
                isThrusting = 0; // Coasting phase after reaching target orbit
                mode = 4; // Switch to mode 4 for coasting
            }
        } else {
            //Mode 4: Coasting phase
            *isThrusting = 0; // Engine is off during coasting
        }
        cmd_x = Zv * sin(commanded_pitch) + Xv * cos(commanded_pitch) *cos(commanded_heading) + Yv * cos(commanded_pitch) * sin(commanded_heading);
        cmd_z = -Zv * cos(commanded_pitch) + Xv * sin(commanded_pitch) *cos(commanded_heading) + Yv * sin(commanded_pitch) * sin(commanded_heading);
        cmd_y = cmd_z ^ cmd_x;
        cmd_q = Quaternion::getRotationQuaternion(cmd_x,cmd_y,cmd_z);
}

double CGuidance::getApoapsisPeriapsis(double *apo, double *peri) {
    *apo = apoapsis_altitude;
    *peri = periapsis_altitude;
}