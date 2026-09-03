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

    logFName1 = sys.argv[1]

    logFName2 = sys.argv[2]

    fname1 = "/logs/" + logFName1

    fname2 = "/logs/" + logFName2

    logFName = "diff"

    #Obtain time data from time_history.csv. Time is the first column in the csv file altitude_history.csv
    time1 = np.loadtxt(project_dir + fname1, delimiter=',', usecols=0)

    #Read altitude from altitude_history.csv
    altitude1 = np.loadtxt(project_dir + fname1, delimiter=',',usecols=1)

    #Read Position (x,y,z) from log file
    sx1 = np.loadtxt(project_dir + fname1, delimiter=',',usecols=2)
    sy1 = np.loadtxt(project_dir + fname1, delimiter=',',usecols=3)
    sz1 = np.loadtxt(project_dir + fname1, delimiter=',',usecols=4)

    #Read velocity from velocity_history.csv using project_dir
    vx1 = np.loadtxt(project_dir + fname1, delimiter=',',usecols=5)
    vy1 = np.loadtxt(project_dir + fname1, delimiter=',',usecols=6)
    vz1 = np.loadtxt(project_dir + fname1, delimiter=',',usecols=7)

    #Read local FPA
    local_FPA1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=8)

    heading_angle1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=9)

    Xvx1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=10)
    Xvy1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=11)
    Xvz1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=12)

    Yvx1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=13)
    Yvy1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=14)
    Yvz1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=15) 

    Zvx1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=16)
    Zvy1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=17)
    Zvz1 = np.loadtxt(project_dir + fname1,delimiter=',',usecols=18)

    velocity1 = np.sqrt(vx1 ** 2 + vy1 ** 2 + vz1 ** 2)

   #Obtain time data from time_history.csv. Time is the first column in the csv file altitude_history.csv
    time2 = np.loadtxt(project_dir + fname2, delimiter=',', usecols=0)

    #Read altitude from altitude_history.csv
    altitude2 = np.loadtxt(project_dir + fname2, delimiter=',',usecols=1)

    #Read Position (x,y,z) from log file
    sx2 = np.loadtxt(project_dir + fname2, delimiter=',',usecols=2)
    sy2 = np.loadtxt(project_dir + fname2, delimiter=',',usecols=3)
    sz2 = np.loadtxt(project_dir + fname2, delimiter=',',usecols=4)

    #Read velocity from velocity_history.csv using project_dir
    vx2 = np.loadtxt(project_dir + fname2, delimiter=',',usecols=5)
    vy2 = np.loadtxt(project_dir + fname2, delimiter=',',usecols=6)
    vz2 = np.loadtxt(project_dir + fname2, delimiter=',',usecols=7)

    #Read local FPA
    local_FPA2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=8)

    heading_angle2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=9)

    Xvx2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=10)
    Xvy2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=11)
    Xvz2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=12)

    Yvx2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=13)
    Yvy2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=14)
    Yvz2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=15) 

    Zvx2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=16)
    Zvy2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=17)
    Zvz2 = np.loadtxt(project_dir + fname2,delimiter=',',usecols=18)

    velocity2 = np.sqrt(vx2 ** 2 + vy2 ** 2 + vz2 ** 2)

    length = min(len(time1),len(time2))

    time_diff = time1[0:length] - time2[0:length]

    #Read altitude from altitude_history.csv
    altitude_diff = altitude1[0:length] - altitude2[0:length]

    #Read Position (x,y,z) from log file
    sx_diff = sx1[0:length] - sx2[0:length]
    sy_diff = sy1[0:length] - sy2[0:length]
    sz_diff = sz1[0:length] - sz2[0:length]

    #Read velocity from velocity_history.csv using project_dir
    vx_diff = vx1[0:length] - vx2[0:length]
    vy_diff = vy1[0:length] - vy2[0:length]
    vz_diff = vz1[0:length] - vz2[0:length]

    #Read local FPA
    local_FPA_diff = local_FPA1[0:length] - local_FPA2[0:length]

    heading_angle_diff = heading_angle1[0:length] - heading_angle2[0:length]

    Xvx_diff = Xvx1[0:length] - Xvx2[0:length]
    Xvy_diff = Xvy1[0:length] - Xvy2[0:length]
    Xvz_diff = Xvz1[0:length] - Xvz2[0:length]

    Yvx_diff = Yvx1[0:length] - Yvx2[0:length]
    Yvy_diff = Yvy1[0:length] - Yvy2[0:length]
    Yvz_diff = Yvz1[0:length] - Yvz2[0:length]

    Zvx_diff = Zvx1[0:length] - Zvx2[0:length]
    Zvy_diff = Zvy1[0:length] - Zvy2[0:length]
    Zvz_diff = Zvz1[0:length] - Zvz2[0:length]

    velocity_diff = velocity1[0:length] - velocity2[0:length]

    #Plot all the parameters using the plot_parameters function in different figures
    plot_parameters(time1[0:length], [altitude_diff], ['Altitude (m)'],logFName + "_" + "altitude_plot.png")
    plot_parameters(time1[0:length], [velocity_diff], ['Velocity (m/s)'],logFName + "_" + "velocity_plot.png")
    plot_parameters(time1[0:length], [Xvx_diff, Yvx_diff, Zvx_diff], ['Xvx (m)', 'Yvx (m)', 'Zvx (m)'],logFName + "_" + "Comp_x.png")
    plot_parameters(time1[0:length], [Xvy_diff, Yvy_diff, Zvy_diff], ['Xvy (m)', 'Yvy (m)', 'Zvy (m)'],logFName + "_" + "Comp_y.png")
    plot_parameters(time1[0:length], [Xvz_diff, Yvz_diff, Zvz_diff], ['Xvz (m)', 'Yvz (m)', 'Zvz (m)'],logFName + "_" + "Comp_z.png")
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
    ax.scatter3D(sx1[0],sy1[0],sz1[0]);
    ax.plot3D(sx1,sy1,sz1)
    ax.plot3D(sx2,sy2,sz2)
    ax.set_xlabel('X (m)')
    ax.set_ylabel('Y (m)')
    ax.set_zlabel('Z (m)')
    ax.set_title('Earth Surface')
    plt.show()
    
    