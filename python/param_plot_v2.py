#File to plot the parameters of the simulation
import matplotlib.pyplot as plt
import numpy as np
import sys

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

    logFName = sys.argv[1]

    fname = "/logs/" + logFName

    #Obtain time data from time_history.csv. Time is the first column in the csv file altitude_history.csv
    time = np.loadtxt(project_dir + fname, delimiter=',', usecols=0)

    #Read altitude from altitude_history.csv
    altitude = np.loadtxt(project_dir + fname, delimiter=',',usecols=1)

    #Read Position (x,y,z) from log file
    sx = np.loadtxt(project_dir + fname, delimiter=',',usecols=2)
    sy = np.loadtxt(project_dir + fname, delimiter=',',usecols=3)
    sz = np.loadtxt(project_dir + fname, delimiter=',',usecols=4)

    #Read velocity from velocity_history.csv using project_dir
    vx = np.loadtxt(project_dir + fname, delimiter=',',usecols=5)
    vy = np.loadtxt(project_dir + fname, delimiter=',',usecols=6)
    vz = np.loadtxt(project_dir + fname, delimiter=',',usecols=7)

    #Read local FPA
    local_FPA = np.loadtxt(project_dir + fname,delimiter=',',usecols=8)

    heading_angle = np.loadtxt(project_dir + fname,delimiter=',',usecols=9)

    Xvx = np.loadtxt(project_dir + fname,delimiter=',',usecols=10)
    Xvy = np.loadtxt(project_dir + fname,delimiter=',',usecols=11)
    Xvz = np.loadtxt(project_dir + fname,delimiter=',',usecols=12)

    Yvx = np.loadtxt(project_dir + fname,delimiter=',',usecols=13)
    Yvy = np.loadtxt(project_dir + fname,delimiter=',',usecols=14)
    Yvz = np.loadtxt(project_dir + fname,delimiter=',',usecols=15) 

    Zvx = np.loadtxt(project_dir + fname,delimiter=',',usecols=16)
    Zvy = np.loadtxt(project_dir + fname,delimiter=',',usecols=17)
    Zvz = np.loadtxt(project_dir + fname,delimiter=',',usecols=18)

    velocity = np.sqrt(vx ** 2 + vy ** 2 + vz ** 2)

    #Plot all the parameters using the plot_parameters function in different figures
    plot_parameters(time, [altitude], ['Altitude (m)'],logFName + "_" + "altitude_plot.png")
    plot_parameters(time, [velocity], ['Velocity (m/s)'],logFName + "_" + "velocity_plot.png")
    plot_parameters(time, [Xvx, Yvx, Zvx], ['Xvx (m)', 'Yvx (m)', 'Zvx (m)'],logFName + "_" + "Comp_x.png")
    plot_parameters(time, [Xvy, Yvy, Zvy], ['Xvy (m)', 'Yvy (m)', 'Zvy (m)'],logFName + "_" + "Comp_y.png")
    plot_parameters(time, [Xvz, Yvz, Zvz], ['Xvz (m)', 'Yvz (m)', 'Zvz (m)'],logFName + "_" + "Comp_z.png")
    # plot_parameters(time, [commanded_pitch], ['Commanded Pitch (deg)'],"cmd_pitch_plot.png")
    # plot_parameters(time, [delta], ['Azimuth angle (deg)'],"delta_plot.png")
    # plot_parameters(time, [phi], ['Polar Angle (deg)'],"phi_plot.png")
    # plot_parameters(time, [fpa], ['Flight Path Angle (deg)'],"fpa_plot.png")
    # plot_parameters(time, [psi], ['TV angle (deg)'],"psi_plot.png")
    # plot_parameters(time, [fmass], ['Fuel mass (kg)'],"fmass.png")
    # plot_parameters(time, [heading], ['heading (deg)'],"heading.png")
    # plot_parameters(time, [s_x, s_y, s_z], ['s_x (m)', 's_y (m)', 's_z (m)'],"s_plot.png")
    # plot_parameters(time, [radial_velocity], ['Radial Velocity (m/s)'],"radial_velocity.png")
    # plot_parameters(time, [body_x_x, body_x_y, body_x_z], ['Body X X (m)', 'Body X Y (m)', 'Body X Z (m)'],"body_x_plot.png")
    # plot_parameters(time, [body_y_x, body_y_y, body_y_z], ['Body Y X (m)', 'Body Y Y (m)', 'Body Y Z (m)'],"body_y_plot.png")
    # plot_parameters(time, [body_z_x, body_z_y, body_z_z], ['Body Z X (m)', 'Body Z Y (m)', 'Body Z Z (m)'],"body_z_plot.png")

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
    ax.scatter3D(sx[0],sy[0],sz[0]);
    ax.plot3D(sx,sy,sz)
    ax.set_xlabel('X (m)')
    ax.set_ylabel('Y (m)')
    ax.set_zlabel('Z (m)')
    ax.set_title('Earth Surface')
    plt.show()
    
    