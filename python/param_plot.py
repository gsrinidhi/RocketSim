#File to plot the parameters of the simulation
import matplotlib.pyplot as plt
import numpy as np

from mpl_toolkits.mplot3d import Axes3D

# Function to plot the parameters of the simulation

project_dir = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim"

def plot_parameters(time, parameters, labels,figname):
    """
    Plots the parameters of the simulation over time.
    
    Parameters:
    time (array-like): Array of time values.
    parameters (list of array-like): List of parameter arrays to plot.
    labels (list of str): List of labels for each parameter.
    """
    plt.figure(figsize=(10, 6))
    
    for param, label in zip(parameters, labels):
        plt.plot(time, param, label=label)
    
    plt.xlabel('Time (s)')
    plt.ylabel('Parameter Value')
    plt.title('Simulation Parameters Over Time')
    plt.legend()
    plt.grid()
    plt.savefig(project_dir+"/plots/"+figname)
    # plt.show()

# Use this function to plot the parameters after running the simulation and collecting the data in csv files. You can read the data from the csv files using numpy or pandas and then call this function to visualize the parameters.

if __name__ == "__main__":

    #Obtain time data from time_history.csv. Time is the first column in the csv file altitude_history.csv
    time = np.loadtxt(project_dir + "/csv/altitude_history.csv", delimiter=',', usecols=0)

    #Read altitude from altitude_history.csv
    altitude = np.loadtxt(project_dir + "/csv/altitude_history.csv", delimiter=',',usecols=1)

    #Read velocity from velocity_history.csv using project_dir
    velocity = np.loadtxt(project_dir + "/csv/velocity_history.csv", delimiter=',',usecols=1)

    #Read apoapsis and periapsis from apoapsis_history.csv and periapsis_history.csv
    apoapsis = np.loadtxt(project_dir + "/csv/apoapsis_history.csv", delimiter=',',usecols=1)
    periapsis = np.loadtxt(project_dir + "/csv/periapsis_history.csv", delimiter=',',usecols=1)

    #Read commanded pitch, delta, phi, fpa and psi from commanded_pitch_history.csv
    commanded_pitch = np.loadtxt(project_dir + "/csv/commanded_pitch_history.csv", delimiter=',',usecols=1)
    delta = np.loadtxt(project_dir + "/csv/delta_history.csv", delimiter=',',usecols=1)
    phi = np.loadtxt(project_dir + "/csv/phi_history.csv", delimiter=',',usecols=1)
    fpa = np.loadtxt(project_dir + "/csv/fpa_history.csv", delimiter=',',usecols=1)
    psi = np.loadtxt(project_dir + "/csv/psi_history.csv", delimiter=',',usecols=1)
    fmass = np.loadtxt(project_dir + "/csv/fmass_history.csv", delimiter=',',usecols=1)
    heading = np.loadtxt(project_dir + "/csv/heading_history.csv", delimiter=',',usecols=1)
    radial_velocity = heading = np.loadtxt(project_dir + "/csv/radial_velocity_history.csv", delimiter=',',usecols=1)

    #Read s_x, s_y and s_z from s_x_history.csv, s_y_history.csv and s_z_history.csv
    s_x = np.loadtxt(project_dir + "/csv/s_x_history.csv", delimiter=',',usecols=1)
    s_y = np.loadtxt(project_dir + "/csv/s_y_history.csv", delimiter=',',usecols=1)
    s_z = np.loadtxt(project_dir + "/csv/s_z_history.csv", delimiter=',',usecols=1)

    #Plot all the parameters using the plot_parameters function in different figures
    plot_parameters(time, [altitude], ['Altitude (m)'],"altitude_plot.png")
    plot_parameters(time, [velocity], ['Velocity (m/s)'],"velocity_plot.png")
    plot_parameters(time, [apoapsis, periapsis], ['Apoapsis (m)', 'Periapsis (m)'],"apoapsis_plot.png")
    plot_parameters(time, [commanded_pitch], ['Commanded Pitch (deg)'],"cmd_pitch_plot.png")
    plot_parameters(time, [delta], ['Azimuth angle (deg)'],"delta_plot.png")
    plot_parameters(time, [phi], ['Polar Angle (deg)'],"phi_plot.png")
    plot_parameters(time, [fpa], ['Flight Path Angle (deg)'],"fpa_plot.png")
    plot_parameters(time, [psi], ['TV angle (deg)'],"psi_plot.png")
    plot_parameters(time, [fmass], ['Fuel mass (kg)'],"fmass.png")
    plot_parameters(time, [heading], ['heading (deg)'],"heading.png")
    plot_parameters(time, [s_x, s_y, s_z], ['s_x (m)', 's_y (m)', 's_z (m)'],"s_plot.png")
    plot_parameters(time, [radial_velocity], ['Radial Velocity (m/s)'],"radial_velocity.png")

    Re = 6371000 #Radius of Earth in meters
    #Plot the earths surface in 3d
    fig = plt.figure(figsize=(10, 10))
    ax = fig.add_subplot(111, projection='3d')
    u = np.linspace(0, 2 * np.pi, 100)
    v = np.linspace(0, np.pi, 100)
    x = Re * np.outer(np.cos(u), np.sin(v))
    y = Re * np.outer(np.sin(u), np.sin(v))
    z = Re * np.outer(np.ones(np.size(u)), np.cos(v))

    #Plot s_x,s_y,s_z history in 3d
    
    ax.plot_surface(x, y, z, color='b', alpha=0.5)
    ax.scatter3D(s_x[0],s_y[0],s_z[0]);
    ax.plot3D(s_x,s_y,s_z)
    ax.set_xlabel('X (m)')
    ax.set_ylabel('Y (m)')
    ax.set_zlabel('Z (m)')
    ax.set_title('Earth Surface')
    plt.show()
    
    