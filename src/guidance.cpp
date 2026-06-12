#include "guidance.h"

#include "phyVector.h"

#include <iostream>
#include <cmath>
#include <vector>

#include "matplotlib-cpp/matplotlibcpp.h"

#include <filesystem>

#include <string>

namespace fs = std::filesystem;

using namespace std;

namespace plt = matplotlibcpp;

//define universlal gravitational constant
const double G = 6.67430e-11; // in m^3 kg^-1 s^-2

//define mass of Earth
const double Me = 5.972e24; // in kg

double getAltitudeGain(double predicted_altitude, double target_altitude) {
    double gain = 0;
    double alt_diff = target_altitude - predicted_altitude;
    if (alt_diff < -1000e3) {
        gain = 0.0000001; // Very small gain for altitudes much lower than target
    } else if (alt_diff < -500e3) {
        gain = 0.000001; // Small gain for altitudes moderately lower than target
    } else if (alt_diff < -100e3) {
        gain = 0.00001; // Moderate gain for altitudes slightly lower than target
    } else if (alt_diff < 0) {
        gain = 0.0001; // Higher gain for altitudes just below target
    } else if (alt_diff < 100e3) {
        gain = 0.001; // High gain for altitudes just above target
    } else if (alt_diff < 500e3) {
        gain = 0.0001; // Moderate gain for altitudes moderately above target
    } else if (alt_diff < 1000e3) {
        gain = 0.00001; // Small gain for altitudes slightly above target
    } else {
        gain = 0.000001; // Very small gain for altitudes much higher than target
    }
    return gain;
}

int guid_PEG(phyVector v_init,phyVector s_init, double thrustMag, double tgo, double dt, double mass, double g, double A, double B,double mdot,phyVector *pred_s, phyVector *pred_vtr) {
    // Placeholder implementation for the PEG guidance algorithm
    // This function should compute the predicted position and velocity based on the inputs
    // For now, it simply returns 0 and does not modify pred_s or pred_vtr

    // In a real implementation, you would calculate the guidance commands here
    // using the provided parameters and update pred_s and pred_vtr accordingly

    int total_steps = static_cast<int>(tgo / dt);
    double k,local_time,commanded_pitch,r,current_mass;
    phyVector atr,v,s,thrust,a_thrust,a_grav,a_total,r_unit,t_unit;
    s = s_init;
    v = v_init;
    current_mass = mass;
    for (int i = 0; i < total_steps; ++i) {

        //Get the current time
        local_time = i * dt;
        //Calculate the current mass based on mass flow rate
        current_mass = mass - mdot * local_time;
        //Get radial unit vector
        r_unit = s * (1.0 / s.magnitude());
        //Get tangential unit vector
        t_unit = phyVector(-r_unit.y, r_unit.x, 0); // Assuming 2D motion in the xy-plane
        //normalise tangential unit vector
        t_unit = t_unit * (1.0 / t_unit.magnitude());
        //Calculate commanded pitch based on the guidance law 
        commanded_pitch = A + B * local_time;
        //Calculate thrust vector
        thrust = (t_unit * cos(commanded_pitch) + r_unit * sin(commanded_pitch)) * thrustMag;
        a_thrust = thrust / current_mass;
        r = s.magnitude();
        a_grav = -r_unit * (G * Me / (r * r)); // Gravitational acceleration decreases with distance
        a_total = a_thrust + a_grav;
        //Update velocity and position
        v = v + a_total * dt;
        s = s + v * dt;


    }
    //update the predicted position and velocity at the end of the guidance period
    *pred_s = s;
    *pred_vtr = v;

    return 0; // Return 0 to indicate success (or an appropriate error code)
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

int sim3DOF() {

    fs::path current_path = fs::current_path();

    cout << "Current path: " << current_path << endl;
    fs::path project_root_path = current_path.parent_path(); // Set the project root path

    cout << "Project root path: " << project_root_path << endl;
    fs::path plot_dir = current_path / "plots"; // Set the path for the plots directory
    string plot_dir_str = plot_dir.string();
    cout << "Plot directory path: " << plot_dir_str << endl;
    // Create the plots directory if it doesn't exist
    if (!fs::exists(plot_dir_str)) {
        fs::create_directory(plot_dir_str);
    }

    fs::path plot_path;

    double pi = 3.14159265358979323846;
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
    
    
    phyVector normal = phyVector(0, 1, 0); // Normal vector pointing upwards
    double g_mag = 9.81; // Gravitational acceleration in m/s^2
    double r; // Current distance from the center of the Earth
    double thrustMag = m * Isp * g_mag; // Example thrust magnitude
    phyVector T = phyVector(0, thrustMag, 0); // Thrust vector pointing upwards
    phyVector g = phyVector(0, 0, -g_local); // Gravitational acceleration vector pointing downwards
    phyVector v = phyVector(0, 0, 0); // Initial velocity
    phyVector s = phyVector(0, 0, -Re); // Initial position at Earth's surface
    phyVector pred_s, pred_vtr,r_f,t_f; // Predicted position and velocity
    phyVector r_h,t_h; // Current radial and tangential unit vectors
    phyVector a_thrust,a_grav,a_total; // Acceleration due to thrust, gravity, and total acceleration
    double dt = 0.1; // Time step in seconds
    double target_altitude = 500e3,v_tangential = 0, v_radial; // Target altitude in meters
    double target_velocity = sqrt(G * Me / (Re + target_altitude)); // Circular orbit velocity at target altitude
    double mode = 1;
    double subMode = 0;
    int isThrusting = 1; // Flag to indicate if the engine is currently thrusting
    double coast_time = 0; // Time spent coasting after burnout
    double coast_start_time = 0; // Time when coasting starts
    double apoapsis_altitude = 0; // Altitude of the apoapsis
    double periapsis_altitude = 0; // Altitude of the periapsis
    double commanded_pitch;
    double altitude;
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
    vector<double> r_x;//For plotting earth's surface
    vector<double> r_z;//For plotting earth's surface
    double stop_time = 8000; // Simulation stop time in seconds
    double mode_2_3_change_altitude = 10.5e3; // Altitude at which to switch from mode 2 to mode 3
    double mode_2_pitch = 60 * pi / 180; // Initial pitch angle for mode 2 in radians
    double global_velocity_angle = 0;
    double apoapsis_threshold = 50e3;
    double injection_time = 0;
    double mode_1_2_switch_over_time = 0;
    double mode_2_3_switch_over_time = 0;
    int bp;
    double phi;
    int depletion_flag = 0; // Flag to indicate if fuel is depleted
    double delta = atan2(s.x, s.z); // Initial angle between the position vector and the vertical axis
    double altitude_threshold = 100e3; // Altitude threshold for debug purposes
    double delta_deg;
    double phi_deg;
    double fpa_deg;
    double commanded_pitch_deg;
    string res = "";
    for (size_t i = 0; i < stop_time/dt; i++)
    {
        /* code */
        time.push_back(i*dt);
    }
    //start the simulation loop
    //assuming rocket motion in x-z plane
    
    //populate Earth's surface data for plotting
    for (double angle = 0; angle <= 2 * pi; angle += 0.01) {
        r_x.push_back(Re * cos(angle));
        r_z.push_back(Re * sin(angle));
    }
    for (size_t i = 0; i < time.size(); i++)
    {
        /* code */

        //calculate current altitude
        r = s.magnitude();
        altitude = r - Re;

        //stop the simulation if the rocket crashes back to Earth
        if(altitude <= 0 && i > 20) {
            cout << "Rocket has crashed back to Earth at time: " << time[i] << " seconds." << endl;
            injection_time = time[i]; // Record the time of crash for reference
            res = "Crash time";
            break;
            
        }

        //Calculate local gravity vector
        g_local = G * Me / (r * r);

        global_velocity_angle = atan2(v.z, v.x); // Angle of the velocity vector with respect to the vertical axis
        fpa_deg = global_velocity_angle * (180.0 / pi); // Flight path angle in degrees
        getApoapsisPeriapsis(s, v, Me, Re, apoapsis_altitude, periapsis_altitude);
        if(mode == 1) {
            //Mode 1: Vertical ascent
            commanded_pitch = 90 * pi / 180; // Keep the rocket vertical
            if(altitude >= 1000) {
                mode = 2; // Switch to mode 2 when reaching 1km altitude
                mode_1_2_switch_over_time = time[i]; // Record the time of switching from mode 1 to mode 2 for reference
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
                    commanded_pitch = global_velocity_angle + delta; // Adjust pitch to follow the velocity vector while maintaining the angle with respect to the vertical axis
                } else {
                    global_velocity_angle = 0;
                }
                // getApoapsisPeriapsis(s, v, Me, Re, apoapsis_altitude, periapsis_altitude);
                if(apoapsis_altitude >= apoapsis_threshold) {
                    bp = 1;
                    apoapsis_threshold += 50e3; // Switch to sub-mode 2 when approaching the altitude to switch to mode 3
                }
                if(apoapsis_altitude >= target_altitude) {
                    subMode = 2; // Switch to sub-mode 2 when approaching target altitude
                }
            } else if (subMode == 2) {
                isThrusting = 0;
                // go to submode 3 if altitude is reached
                if (altitude >= apoapsis_altitude - 5e2) {
                    subMode = 3;
                }
                if (altitude >= altitude_threshold) {
                    // Debug print statements for altitude and velocity
                    cout << "Time: " << time[i] << " s, Altitude: " << altitude << " m, Velocity: " << v.magnitude() << " m/s" << endl;
                    altitude_threshold += 50e3; // Increase threshold for next debug print
                }
            } else if (subMode == 3) {
                isThrusting = 1; // Fire until perigee is raised to target altitude
                commanded_pitch = global_velocity_angle + delta; 
                if(periapsis_altitude >= target_altitude) {
                    subMode = 4; // Switch to coasting when perigee is raised to target altitude
                }
            } else if (subMode == 4) {
                isThrusting = 0; // Coasting phase after reaching target orbit
                injection_time = time[i]; // Record the time of engine cutoff for reference
                res = "Injection time";
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
        phi = commanded_pitch - delta;

        phi_deg = phi * (180.0 / pi); // Convert phi to degrees for easier interpretation

        a_total.x = thrustMag * cos(phi) / mass - g_local * sin(delta);
        a_total.z = thrustMag * sin(phi) / mass - g_local * cos(delta);

        v = v + a_total * dt;
        s = s + v * dt;

        delta = atan2(s.x, s.z); // Update the angle between the position vector and the vertical axis
        delta_deg = delta * (180.0 / pi); // Convert delta to degrees for easier interpretation
        
        //decrease mass based on mass flow rate
        if(isThrusting && depletion_flag == 0) {
            Mf -= m * dt;
            mass = Ms + Mp + Mf; // Update total mass of the vehicle
            if(Mf < 0 && depletion_flag == 0) {
                Mf = 0; // Ensure fuel mass does not go negative
                mass = Ms + Mp; // Update total mass when fuel is depleted
                depletion_flag = 1; // Stop thrusting when fuel is depleted
                injection_time = time[i]; // Record the time of fuel depletion for reference
                res = "Fuel depletion time";

            }
        }

        bp = 1;

        if(i == time.size() - 1) {
            // Final debug print statements for altitude, velocity, and orbital parameters
            cout << "Final Time: " << time[i] << " s, Final Altitude: " << altitude << " m, Final Velocity: " << v.magnitude() << " m/s" << endl;
            cout << "Final Apoapsis Altitude: " << apoapsis_altitude << " m, Final Periapsis Altitude: " << periapsis_altitude << " m" << endl;
        }

        // Store altitude, velocity, apoapsis, and periapsis history for plotting
        altitude_history.push_back(altitude);
        velocity_history.push_back(v.magnitude());
        apoapsis_history.push_back(apoapsis_altitude);
        periapsis_history.push_back(periapsis_altitude);

        // Store commanded pitch, flight path angle, phi, delta, and position history for plotting
        commanded_pitch_deg_history.push_back(commanded_pitch_deg);
        fpa_deg_history.push_back(fpa_deg);
        phi_deg_history.push_back(phi_deg);
        delta_deg_history.push_back(delta_deg);
        s_x_history.push_back(s.x);
        s_z_history.push_back(s.z);
    
    }

    // Plotting the results using matplotlib-cpp
    //mark injection time on the plots for reference
    //save the plots as images for later analysis
    time.resize(altitude_history.size()); // Resize time vector to match the size of altitude history for plotting
    plt::figure();
    // plt::subplot(2, 2, 1);
    plt::named_plot("Altitude", time, altitude_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Altitude vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Altitude (m)");
    plt::legend();
    plot_path = plot_dir / "altitude_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();

    // plt::subplot(2, 2, 2);
    plt::figure();
    plt::named_plot("Velocity", time, velocity_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Velocity vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Velocity (m/s)"); 
    plt::legend();
    plot_path = plot_dir / "velocity_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();
    // plt::subplot(2, 2, 3);
    plt::figure();
    plt::named_plot("Apoapsis Altitude", time, apoapsis_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Apoapsis and Periapsis Altitude vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Altitude (m)");
    plt::legend();
    plot_path = plot_dir / "apoapsis_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();

    plt::figure();
    plt::named_plot("Periapsis Altitude", time, periapsis_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Apoapsis and Periapsis Altitude vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Altitude (m)");
    plt::legend();
    plot_path = plot_dir / "periapsis_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();

    //plot commanded pitch, flight path angle, phi, and delta over time separately for better visibility
    plt::figure();
    plt::named_plot("Commanded Pitch (deg)", time, commanded_pitch_deg_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Commanded Pitch vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Angle (degrees)");
    plt::legend();
    plot_path = plot_dir / "commanded_pitch_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();

    plt::figure();
    plt::named_plot("Flight Path Angle (deg)", time, fpa_deg_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Flight Path Angle vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Angle (degrees)");
    plt::legend();
    plot_path = plot_dir / "flight_path_angle_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();

    plt::figure();
    plt::named_plot("Phi (deg)", time, phi_deg_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Phi vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Angle (degrees)");
    plt::legend();
    plot_path = plot_dir / "phi_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();

    plt::figure();
    plt::named_plot("Delta (deg)", time, delta_deg_history);
    plt::axvline( injection_time, 0.0,1.0,
        {
            {"color", "red"},
            {"linestyle", "--"},
            {"label", res}
        }
    );
    plt::axvline( mode_1_2_switch_over_time, 0.0,1.0,
        {
            {"color", "blue"},
            {"linestyle", "--"},
            {"label", "Mode 1 to 2 Switch Time"}
        }
    );
    plt::axvline( mode_2_3_switch_over_time, 0.0,1.0,
        {
            {"color", "green"},
            {"linestyle", "--"},
            {"label", "Mode 2 to 3 Switch Time"}
        }
    );
    plt::title("Delta vs Time");
    plt::xlabel("Time (s)");
    plt::ylabel("Angle (degrees)");
    plt::legend();
    plot_path = plot_dir / "delta_vs_time.png";
    plt::save(plot_path.string());
    plt::draw();

    //plot trajectory of the rocket along with Earth's surface
    plt::figure();
    plt::named_plot("Earth's Surface", r_x, r_z); // Plot Earth's surface
    plt::named_plot("Rocket Trajectory", s_x_history, s_z_history); // Plot rocket trajectory
    plt::title("Rocket Trajectory");
    plt::xlabel("Distance in x (m)");
    plt::ylabel("Distance in z (m)");
    plt::axis("equal");
    plot_path = plot_dir / "trajectory.png";
    plt::save(plot_path.string());
    plt::draw();


    return 0;
}

int main() {
    sim3DOF();
    return 0;
}