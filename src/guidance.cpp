#include "guidance.h"
#include "phyVector.h"
#include <iostream>
#include <cmath>
#include <vector>
#include<fstream>
#include <filesystem>
#include <string>
#include <chrono>
#include <ctime>
#include <map>

namespace fs = std::filesystem;

using namespace std;

//define universlal gravitational constant
const double G = 6.67430e-11; // in m^3 kg^-1 s^-2

//define mass of Earth
const double Me = 5.972e24; // in kg

int readInputFile(string fname, std::map<std::string, double> *configMap) {
    // 1. Open the file stream
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
            (*configMap)[key] = value;
        }
    }

    // 4. Close the file stream
    file.close();

    return 0;
}

void getApoapsisPeriapsis(phyVector s, phyVector v, double M, double R,double &apoapsis_altitude, double &periapsis_altitude) {
    double r = s.magnitude();
    double v_mag = v.magnitude();
    double specific_energy = (v_mag * v_mag) / 2 - G * M / r; // Specific orbital energy
    double semi_major_axis = -G * M / (2 * specific_energy); // Semi-major axis of the orbit
    double eccentricity = sqrt(1 + (2 * specific_energy * (s^v).magnitude() * (s^v).magnitude()) / (G * M * G * M)); // Eccentricity of the orbit
    apoapsis_altitude = semi_major_axis * (1 + eccentricity) - R; // Altitude of apoapsis
    periapsis_altitude = semi_major_axis * (1 - eccentricity) - R; // Altitude of periapsis
}

double getPolarAngle(phyVector s) {
    double phi = 0;
    double xy_projection = s.magnitude(1,1,0);
    if(s.z != 0) {
        phi = atan2(xy_projection,s.z);
    }
    return phi;
}

double getAzimuthAngle(phyVector s) {
    return atan2(s.y,s.x);
}

int guidance(phyVector s, phyVector v, double M, double R, double *mode,double *commanded_pitch, double *commanded_heading) {

}

int sim3DOF() {

    std::map<std::string, double> configMap;

    string inputFileName = "input/Input_Params.txt";

    int k = readInputFile(inputFileName,&configMap);
    if(k) {
        cout << "Could not read input file" << endl;
        return 1;
    } else {
        cout << "Read input file successfully" << endl;
    }

    double pi = 3.14159265358979323846;
    double dt = 0.01; // Time step in seconds
    double Mfo = 1800000;//Mass of fuel at the start of the simulation in kg
    double Ms = 150000;//Mass of the structure in kg
    double Mp = 50000;//Mass of the payload in kg
    double mass = Mfo + Ms + Mp; // Total mass of the vehicle in kg
    double Mf = Mfo; // Mass of fuel at the current time step
    double Isp = 400; // Specific impulse of the engine in seconds
    double mo = 10000; // Initial mass flow rate in kg/s (this is a simple assumption, in reality it would depend on the engine and fuel properties)
    double frac = 1.2;
    double m = (frac/1.5) * mo;
    double burn_time = Mfo / m; // Total burn time based on initial mass flow rate
    double Re = 6371000; // Earth's radius in meters
    double g_local = G * Me / (Re * Re); // Local gravitational acceleration at Earth's surface in m/s^2
    double g_mag = 9.81; // Gravitational acceleration in m/s^2
    double r; // Current distance from the center of the Earth
    double thrustMag = m * Isp * g_mag; // Example thrust magnitude
    double sxo,syo,szo;
    double phi_init;
    double theta_init;
    double inclination;
    double apoapsis_target = 500e3,periapsis_target = 500e3;
    double mode = 1;
    double subMode = 0;
    int isThrusting = 1; // Flag to indicate if the engine is currently thrusting
    double coast_time = 0; // Time spent coasting after burnout
    double coast_start_time = 0; // Time when coasting starts
    double apoapsis_altitude = 0; // Altitude of the apoapsis
    double periapsis_altitude = 0; // Altitude of the periapsis
    double commanded_pitch;
    double altitude;
    double radial_velocity;
    double stop_time = 8000; // Simulation stop time in seconds
    double mode_1_2_change_altitude = 1000;
    double mode_2_3_change_altitude = 10.5e3; // Altitude at which to switch from mode 2 to mode 3
    double mode_2_pitch = 60 * pi / 180; // Initial pitch angle for mode 2 in radians
    double global_velocity_angle = 0;
    double apoapsis_threshold = 50e3,periapsis_threshold = 50e3;
    double injection_time = 0;
    double mode_1_2_switch_over_time = 0;
    double mode_2_3_switch_over_time = 0;
    int bp;
    int depletion_flag = 0; // Flag to indicate if fuel is depleted
    double altitude_threshold = 100e3; // Altitude threshold for debug purposes
    double commanded_pitch_deg;
    double polar_angle,azimuth_angle,heading_angle = 0,xvi = 0;
    double polar_angle_deg,azimuth_angle_deg,heading_angle_deg,xvi_deg;
    double vxv,vyv,vzv;
    double local_FPA,local_FPA_deg;
    double tv_angle;
    double kop;
    double burn1_end_time, coast_end_time;
    vector<double> time;
    vector<double> altitude_history;
    vector<double> velocity_history;
    vector<double> apoapsis_history;
    vector<double> periapsis_history;
    vector<double> commanded_pitch_deg_history;
    vector<double> fpa_deg_history;
    vector<double> phi_deg_history;
    vector<double> delta_deg_history;
    vector<double> s_x_history;
    vector<double> s_z_history;
    vector<double> s_y_history;
    vector<double> tv_history;
    vector<double> fuel_mass_history;
    vector<double> heading_history;
    vector<double> radial_velocity_history;
    std::ofstream altitude_file("csv/altitude_history.csv");
    std::ofstream velocity_file("csv/velocity_history.csv");
    std::ofstream apoapsis_file("csv/apoapsis_history.csv");
    std::ofstream periapsis_file("csv/periapsis_history.csv");
    std::ofstream commanded_pitch_file("csv/commanded_pitch_history.csv");
    std::ofstream fpa_file("csv/fpa_history.csv");
    std::ofstream phi_file("csv/phi_history.csv");
    std::ofstream delta_file("csv/delta_history.csv");
    std::ofstream s_x_file("csv/s_x_history.csv");
    std::ofstream s_z_file("csv/s_z_history.csv");
    std::ofstream s_y_file("csv/s_y_history.csv");
    std::ofstream psi_file("csv/psi_history.csv");
    std::ofstream fmass_file("csv/fmass_history.csv");
    std::ofstream heading_file("csv/heading_history.csv");
    std::ofstream radial_velocity_file("csv/radial_velocity_history.csv");
    vector<double> r_x;//For plotting earth's surface
    vector<double> r_z;//For plotting earth's surface
    vector<double> r_y;//For plotting earth's surface
    

    string res = "";
    
    phyVector T = phyVector(0, thrustMag, 0); // Thrust vector pointing upwards
    phyVector v = phyVector(0, 0, 0); // Initial velocity
    phyVector Xv = phyVector(0,0,0);
    phyVector Yv = phyVector(0,0,0);
    phyVector Zv = phyVector(0,0,0);
    phyVector hv = phyVector(0,0,0);
    phyVector incV;
    phyVector s;
    phyVector a_total;

    dt = configMap["Timestep"];
    Mfo = configMap["Init_Fuel_Mass"];
    Ms = configMap["Init_Str_Mass"];
    Mp = configMap["Payload_Mass"];
    Isp = configMap["Isp"];
    m = configMap["Mass_Flow_Rate"];
    Re = configMap["Earth_Radius"];
    apoapsis_target = configMap["Apoapsis_Target"];
    periapsis_target = configMap["Periapsis_Target"];
    stop_time = configMap["Stop_Time"];
    mode_1_2_change_altitude = configMap["Alt_1_2"];
    mode_2_3_change_altitude = configMap["Alt_2_3"];
    mode_2_pitch = configMap["Mode_2_Pitch"] * pi / 180;
    phi_init = configMap["Init_Polar_Angle"] * pi / 180;
    theta_init = configMap["Init_Inertial_Azimuth"] * pi / 180;
    inclination = configMap["Inclination"] * pi / 180;

    mass = Mfo + Ms + Mp;
    Mf = Mfo;

    // 1. Get the current time point
    auto now = std::chrono::system_clock::now();

    // 2. Convert to a legacy time_t structure
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // 3. Convert to local time structure
    std::tm* localTime = std::localtime(&currentTime);

    std::ostringstream logfilename;
    logfilename <<"logs/"<< std::put_time(localTime, "%Y_%m_%d_%H_%M_%S")
             << ".txt";

    std::cout<< "Log file name = " << logfilename.str() << endl;

    std::ofstream logfile(logfilename.str());

    // std::ifstream input_params("input/Input_Params.txt");
    
    // input_params >> phi_init;
    // input_params >> theta_init;
    // input_params >> inclination;

    std::cout<<"Input Polar angle = " << phi_init <<endl;
    std::cout<<"Input Azimuth= " << theta_init <<endl;
    std::cout<<"Input Inclination= " << inclination <<endl;

    logfile << "Simulation Date: " << (localTime->tm_year + 1900) << "_" << (localTime->tm_mon + 1) << "_" << localTime->tm_mday << endl;
    logfile << "Simulation Time: " << (localTime->tm_hour) << "_" << localTime->tm_min << "_" << localTime->tm_sec << endl;
    logfile << "Timestep = " << dt << "s" << endl;
    logfile << "Initial Mass of Fuel: " << Mf << "kg" << endl;
    logfile << "Initial Structural Mass: " << Ms << "kg" <<  endl;
    logfile << "Payload Mass: " << Mp << "kg" <<  endl;
    logfile << "ISP: " << Isp << "s" <<  endl;
    logfile << "Mass flow rate: " << m << "kg/s" <<  endl;
    logfile << "Radius of Earth: " << Re << "m" <<  endl;
    logfile << "Target Apogee: " << apoapsis_target << "m" <<  endl;
    logfile << "Target Perigee:" << periapsis_target << "m" <<  endl;
    logfile << "Stop Time: " << stop_time << "s" << endl;
    logfile << "Mode 2 Pitch angle: " << mode_2_pitch * 180/pi << "deg" << endl;
    logfile << "Mode 1 to 2 change altitude: " << mode_1_2_change_altitude << "m" << endl;
    logfile << "Mode 2 to 3 change altitude: " << mode_2_3_change_altitude << "m" << endl;
    logfile<<"Input Polar angle = " << phi_init * 180/pi <<endl;
    logfile<<"Input Azimuth= " << theta_init * 180/pi <<endl;
    logfile<<"Input Inclination= " << inclination * 180/pi <<endl;

    incV.setXYZ(sin(inclination) * sin(theta_init),sin(inclination)*cos(theta_init),cos(inclination));
    sxo = Re*sin(phi_init)*cos(theta_init);
    syo = Re*sin(phi_init)*sin(theta_init);
    szo = Re*cos(phi_init);
    s.setXYZ(sxo, syo, szo); // Initial position at Earth's surface
    
    
    
    
    for (size_t i = 0; i < stop_time/dt; i++)
    {
        /* code */
        time.push_back(i*dt);
    }
    //start the simulation loop
    //assuming rocket motion in x-z plane
    for (size_t i = 0; i < time.size(); i++)
    {
        /* code */

        //calculate current altitude
        r = s.magnitude();
        altitude = r - Re;

        if (altitude >= altitude_threshold) {
            // Debug print statements for altitude and velocity
            cout << "Time: " << time[i] << " s, Altitude: " << altitude << " m, Velocity: " << v.magnitude() << " m/s" << "Radial Velocity = " << radial_velocity << " m/s" << endl;
            logfile << "Time: " << time[i] << " s, Altitude: " << altitude << " m, Velocity: " << v.magnitude() << " m/s" << " Apogee: " << apoapsis_altitude << "m " << "Perigee: " << periapsis_altitude << "m" << " Radial Velocity = " << radial_velocity << " m/s" << endl;
            altitude_threshold += 50e3; // Increase threshold for next debug print
        }

        polar_angle = atan2(s.magnitude(1,1,0),s.z);
        azimuth_angle = atan2(s.y,s.x);

        polar_angle_deg = polar_angle * 180/pi;
        azimuth_angle_deg = azimuth_angle * 180/pi;

        Xv.x = -cos(polar_angle) * cos(azimuth_angle);
        Xv.y = -cos(polar_angle) * sin(azimuth_angle);
        Xv.z = sin(polar_angle);

        Yv.x = sin(azimuth_angle);
        Yv.y = -cos(azimuth_angle);

        Zv.x = sin(polar_angle) * cos(azimuth_angle);
        Zv.y = sin(polar_angle) * sin(azimuth_angle);
        Zv.z = cos(polar_angle);

        // Xv.x = sin(heading_angle) * sin(azimuth_angle) - cos(heading_angle) * cos(polar_angle) * cos(azimuth_angle);
        // Xv.y = -(sin(heading_angle) * cos(azimuth_angle) + cos(heading_angle) * cos(polar_angle) * sin(azimuth_angle));
        // Xv.z = cos(heading_angle) * sin(polar_angle);

        // Yv.x = sin(heading_angle) * cos(polar_angle) * cos(azimuth_angle) + cos(heading_angle) * sin(azimuth_angle);
        // Yv.y = sin(heading_angle) * cos(polar_angle) * sin(azimuth_angle) - cos(heading_angle) * cos(azimuth_angle);

        // Zv.x = sin(polar_angle) * cos(azimuth_angle);
        // Zv.y = sin(polar_angle) * sin(azimuth_angle);
        // Zv.z = cos(polar_angle);

        vxv = v * Xv;
        vyv = v * Yv;
        vzv = v * Zv;

        local_FPA = atan2(vzv,sqrt(vxv * vxv + vyv * vyv));

        local_FPA_deg = local_FPA * 180/pi;

        hv = (s ^ incV)/s.magnitude();

        kop = hv * Xv;
        if(kop > 1) {
            kop = 1;
        }
        if(kop < -1) {
            kop = -1;
        }

        heading_angle = acos(kop);

        heading_angle_deg = heading_angle * 180/pi;


        //stop the simulation if the rocket crashes back to Earth
        if(altitude <= 0 && i > 20) {
            cout << "Rocket has crashed back to Earth at time: " << time[i] << " seconds." << endl;
            logfile << "Rocket has crashed back to Earth at time: " << time[i] << " seconds." << endl;
            injection_time = time[i]; // Record the time of crash for reference
            res = "Crash time";
            break;
            
        }

        //Calculate local gravity vector
        g_local = G * Me / (r * r);
        getApoapsisPeriapsis(s, v, Me, Re, apoapsis_altitude, periapsis_altitude);
        if(mode == 1) {
            //Mode 1: Vertical ascent
            commanded_pitch = 90 * pi / 180; // Keep the rocket vertical
            if(altitude >= 1000) {
                mode = 2; // Switch to mode 2 when reaching 1km altitude
                mode_1_2_switch_over_time = time[i]; // Record the time of switching from mode 1 to mode 2 for reference
                // heading_angle = inclination;
            }
            
        } else if (mode == 2) {
            //Mode 2: Gravity turn initiation
            commanded_pitch = mode_2_pitch; // Start pitching over to the initial pitch angle for gravity turn
            subMode = 1; // Sub-mode for gravity turn initiation

            if(altitude >= mode_2_3_change_altitude) {
                mode = 3; // Switch to mode 3 when reaching the altitude to switch to gravity turn continuation
                mode_2_3_switch_over_time = time[i]; // Record the time of switching from mode 2 to mode 3 for reference
            }
        } else if (mode == 3) {
            //Mode 3: Gravity turn continuation
            if (subMode == 1) {
                isThrusting = 1; // Ensure engine is on during gravity turn
                // Make pitch angle equal to flight path angle
                if(v.x != 0 || v.z!= 0) {
                    commanded_pitch = local_FPA;
                } else {
                    global_velocity_angle = 0;
                }
                if(apoapsis_altitude >= apoapsis_threshold) {
                    bp = 1;
                    apoapsis_threshold += 50e3; // Switch to sub-mode 2 when approaching the altitude to switch to mode 3
                }
                if(apoapsis_altitude >= apoapsis_target) {
                    std::cout << "Burn 1 duration = " <<time[i] << endl;
                    std::cout << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    std::cout << "Altitude = " << altitude << " m" << endl;
                    logfile << "Burn 1 duration = " <<time[i] << endl;
                    logfile << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    logfile << "Altitude = " << altitude << " m" << endl;
                    burn1_end_time = time[i];
                    subMode = 2; // Switch to sub-mode 2 when approaching target altitude
                }
            } else if (subMode == 2) {
                isThrusting = 0;
                // go to submode 3 if altitude is reached
                if (altitude >= apoapsis_altitude - 5e2) {
                    coast_end_time = time[i];
                    std::cout << "Coast duration = " <<coast_end_time - burn1_end_time << endl;
                    logfile << "Coast duration = " <<coast_end_time - burn1_end_time << endl;
                    subMode = 3;
                    periapsis_threshold = periapsis_altitude + 100e3;
                }
                
            } else if (subMode == 3) {
                isThrusting = 1; // Fire until perigee is raised to target altitude
                commanded_pitch = local_FPA;
                if(periapsis_altitude > periapsis_threshold) {
                    periapsis_threshold += 100e3;
                }
                if(periapsis_altitude >= periapsis_target) {
                    injection_time = time[i]; // Record the time of engine cutoff for reference
                    res = "Injection time";
                    std::cout << "Burn 2 duration = " <<(injection_time - coast_end_time) << endl;
                    std::cout << "Injection time = " << injection_time << endl; 
                    std::cout << "Injection velocity = " << v.magnitude() <<endl;
                    std::cout << "Acheived perigee = " << periapsis_altitude << endl;
                    std::cout << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    logfile << "Burn 2 duration = " <<(injection_time - coast_end_time) << endl;
                    logfile << "Injection time = " << injection_time << endl; 
                    logfile << "Injection velocity = " << v.magnitude() <<endl;
                    logfile << "Acheived perigee = " << periapsis_altitude << endl;
                    logfile << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                    subMode = 4; // Switch to coasting when perigee is raised to target altitude
                }
            } else if (subMode == 4) {
                isThrusting = 0; // Coasting phase after reaching target orbit
                mode = 4; // Switch to mode 4 for coasting
            }
        } else {
            //Mode 4: Coasting phase
            isThrusting = 0; // Engine is off during coasting
        }

        if(isThrusting && depletion_flag == 0) {
            thrustMag = m * Isp * g_mag; // Update thrust magnitude based on current mass flow rate
        } else {
            thrustMag = 0; // No thrust during coasting
        }

        commanded_pitch_deg = commanded_pitch * (180.0 / pi); // Convert commanded pitch to degrees for easier interpretation

        T = Xv * thrustMag * cos(local_FPA) * cos(heading_angle) + Yv * thrustMag * cos(local_FPA) * sin(heading_angle) + Zv * (thrustMag * sin(local_FPA));

        kop = T * v / (T.magnitude() * v.magnitude());
        if(kop > 1) {
            kop = 1;
        }
        if(kop < -1) {
            kop = -1;
        }

        tv_angle = acos(kop) * 180/pi;

        a_total = Xv * thrustMag/mass * cos(commanded_pitch) * cos(heading_angle) + Yv * thrustMag/mass * cos(commanded_pitch) * sin(heading_angle) + Zv * (thrustMag/mass * sin(commanded_pitch) - g_local);

        v = v + a_total * dt;
        s = s + v * dt;

        radial_velocity = s * v / s.magnitude();
        
        //decrease mass based on mass flow rate
        if(isThrusting && depletion_flag == 0) {
            Mf -= m * dt;
            mass = Ms + Mp + Mf; // Update total mass of the vehicle
            if(Mf < 0 && depletion_flag == 0) {
                Mf = 0; // Ensure fuel mass does not go negative
                mass = Ms + Mp; // Update total mass when fuel is depleted
                depletion_flag = 1; // Stop thrusting when fuel is depleted
                injection_time = time[i]; // Record the time of fuel depletion for reference
                std::cout << "Burn 2 duration = " <<(injection_time - coast_end_time) << endl;
                std::cout << "Depletion time = " << injection_time << endl; 
                std::cout << "Injection velocity = " << v.magnitude() <<endl;
                logfile << "Burn 2 duration = " <<(injection_time - coast_end_time) << endl;
                logfile << "Depletion time = " << injection_time << endl; 
                logfile << "Injection velocity = " << v.magnitude() <<endl;
                logfile << "Radial Velocity = " << radial_velocity << " m/s" << endl;
                res = "Fuel depletion time";
                mode = 4;

            }
        }

        bp = 1;

        if(i == time.size() - 1) {
            // Final debug print statements for altitude, velocity, and orbital parameters
            cout << "Final Time: " << time[i] << " s, Final Altitude: " << altitude << " m, Final Velocity: " << v.magnitude() << " m/s" << endl;
            cout << "Final Apoapsis Altitude: " << apoapsis_altitude << " m, Final Periapsis Altitude: " << periapsis_altitude << " m" << endl;
            logfile << "Final Time: " << time[i] << " s, Final Altitude: " << altitude << " m, Final Velocity: " << v.magnitude() << " m/s" << endl;
            logfile << "Final Apoapsis Altitude: " << apoapsis_altitude << " m, Final Periapsis Altitude: " << periapsis_altitude << " m" << endl;
        }

        // Store altitude, velocity, apoapsis, and periapsis history for plotting
        altitude_history.push_back(altitude);
        velocity_history.push_back(v.magnitude());
        apoapsis_history.push_back(apoapsis_altitude);
        periapsis_history.push_back(periapsis_altitude);

        // Store commanded pitch, flight path angle, phi, delta, and position history for plotting
        commanded_pitch_deg_history.push_back(commanded_pitch_deg);
        fpa_deg_history.push_back(local_FPA_deg);
        phi_deg_history.push_back(polar_angle_deg);
        delta_deg_history.push_back(azimuth_angle_deg);
        tv_history.push_back(tv_angle * 180/pi);
        fuel_mass_history.push_back(Mf);
        heading_history.push_back(heading_angle_deg);
        radial_velocity_history.push_back(radial_velocity);
        
        s_x_history.push_back(s.x);
        s_z_history.push_back(s.z);
        s_y_history.push_back(s.y);
    
    }

    // Plotting the results using matplotlib-cpp
    //mark injection time on the plots for reference
    //save the plots as images for later analysis
    time.resize(altitude_history.size()); // Resize time vector to match the size of altitude history for plotting

    std::cout<<"Writing data to files" << endl;


    //store all the data in csv files for later analysis
    for (size_t i = 0; i < altitude_history.size(); i++) {
        altitude_file << time[i] << "," << altitude_history[i] << endl;
        velocity_file << time[i] << "," << velocity_history[i] << endl;
        apoapsis_file << time[i] << "," << apoapsis_history[i] << endl;
        periapsis_file << time[i] << "," << periapsis_history[i] << endl;
        commanded_pitch_file << time[i] << "," << commanded_pitch_deg_history[i] << endl;
        fpa_file << time[i] << "," << fpa_deg_history[i] << endl;
        phi_file << time[i] << "," << phi_deg_history[i] << endl;
        delta_file << time[i] << "," << delta_deg_history[i] << endl;
        s_x_file << time[i] << "," << s_x_history[i] << endl;
        s_z_file << time[i] << "," << s_z_history[i] << endl;
        s_y_file << time[i] << "," << s_y_history[i] << endl;
        psi_file << time[i] << "," << tv_history[i] << endl;
        fmass_file << time[i] << "," << fuel_mass_history[i] << endl;
        heading_file << time[i] << "," << heading_history[i] <<endl;
        radial_velocity_file << time[i] << "," << radial_velocity_history[i] << endl;
    }

    std::cout<<"Finished Writing data to files" << endl;

    //Close all files

    logfile.close();
    altitude_file.close();
    velocity_file.close();
    apoapsis_file.close();
    periapsis_file.close();
    commanded_pitch_file.close();
    fpa_file.close();
    phi_file.close();
    delta_file.close();
    s_x_file.close();
    s_z_file.close();
    s_y_file.close();
    psi_file.close();
    fmass_file.close();
    heading_file.close();

    return 0;
}

int main() {
    sim3DOF();
    return 0;
}