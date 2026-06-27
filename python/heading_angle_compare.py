import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

project_dir = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim"

sx1 = pd.read_csv(project_dir + "/csv/" + "s_x_history.csv",header=None)
sx2 = pd.read_csv(project_dir + "/csv/" + "s_x_history_h_pi_b_4.csv",header=None)
sy1 = pd.read_csv(project_dir + "/csv/" + "s_y_history.csv",header=None)
sy2 = pd.read_csv(project_dir + "/csv/" + "s_y_history_h_pi_b_4.csv",header=None)
sz1 = pd.read_csv(project_dir + "/csv/" + "s_z_history.csv",header=None)
sz2 = pd.read_csv(project_dir + "/csv/" + "s_z_history_h_pi_b_4.csv",header=None)
mass1 = pd.read_csv(project_dir + "/csv/" + "fmass_history.csv",header=None)
mass2 = pd.read_csv(project_dir + "/csv/" + "fmass_history_h_pi_b_4.csv",header=None)

t1 = list(sx1[0])
t2 = list(sx2[0])
m1 = list(mass1[1])
m2 = list(mass2[1])
s_x1 = list(sx1[1])
s_x2 = list(sx2[1])
s_y1 = list(sy1[1])
s_y2 = list(sy2[1])
s_z1 = list(sz1[1])
s_z2 = list(sz2[1])

plt.figure()
plt.plot(t1,m1,label=" heading = 0")
plt.plot(t2,m2,label=" heading = pi/4")
plt.legend()
plt.show()

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
ax.scatter3D(s_x1[0],s_y1[0],s_z1[0]);
ax.plot3D(s_x1,s_y1,s_z1)
ax.plot3D(s_x2,s_y2,s_z2)
ax.set_xlabel('X (m)')
ax.set_ylabel('Y (m)')
ax.set_zlabel('Z (m)')
ax.set_title('Earth Surface')
plt.show()