#ifndef PHYSIM_H
#define PHYSIM_H

#include"phyVector.h"
#include<iostream>
#include<fstream>
#include<string>
#include <chrono>
#include <ctime>
#include <map>
#include <filesystem>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<math.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <fstream>
#include <sstream>
#include <string>

#include <signal.h>
#include <time.h>
#include <iostream>

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "cguidance.h"
#include "quaternion.h"
#include "simObject.hpp"
#include "simBody.hpp"

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define GRAVITATIONAL_CONSTANT 6.67430e-11

class phySim {
    double polar_angle,azimuth_angle;
    double g_local;
    double Re,Me;
    double dt; // Time step in seconds
    double Mfo;//Mass of fuel at the start of the simulation in kg
    double Ms;//Mass of the structure in kg
    double Mp;//Mass of the payload in kg
    double mass; // Total mass of the vehicle in kg
    double Mf ; // Mass of fuel at the current time step
    double Isp; // Specific impulse of the engine in seconds
    double mo ; 
    double m;
    double stop_time;
    double phi_init,theta_init,inclination;
    double thrustMag;
    double g_mag;
    double depletion_time;
    int depletion_flag;
    double local_FPA;
    double heading_angle,commanded_pitch,commanded_heading;
    double time;
    int sim_iter;
    int isThrusting;
    int sim_run_flag;
    phyVector Xv,Yv,Zv;
    phyVector s,v,a;
    std::map<std::string, double> configMap;
    std::ofstream *logfile;

    GLFWwindow* window;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model,model_rocket;
    unsigned int earth_vao,earth_vbo,rocket_vao,rocket_vbo,earth_ebo,rocket_ebo;
    double rocket_radius,rocket_height,rocket_scale_factor;
    std::vector<float> earth_vertices,rocket_vertices;
    std::vector<unsigned int> earth_indices,rocket_indices;
    float fov;
    bool isDragging,firstMouse;
    double lastX, lastY;
    glm::vec3 up_vector;
    glm::vec3 right_vector;
    glm::vec3 forward_vector;
    double Re_sim,Re_scale;
    double dx,dy;

    int cycle_flag;

    CGuidance *g1;

    phyVector camoffset,upVector,forwardVector,rightVector,camPos;

    Quaternion cam_quat;

    float camDistance;

    double windowHeight,windowWidth;

    int simMode;
    

    
public:
    phySim();
    int simInitFile(std::string fname);
    int initRender();
    void setLogFile(std::ofstream *logfile);
    void simInit(phyVector s, phyVector v);
    void updateLocalFrame(phyVector v,double *local_FPA);
    void updatePosition(double pitch,double heading,double isThrusting);
    void updatePositionQuat(Quaternion cmd_q,double isThrusting);
    void getLocalFrame(phyVector *Xv,phyVector *Yv,phyVector *Zv);
    double getPolarAngle();
    double getAzimuthAngle();
    double getFuelMass();
    double getRadialVelocity();
    double getTimeStep();
    double getParam(std::string pname);
    phyVector getS();
    phyVector getV();
    void resize(int width, int height);
    void scroll(double xoffset,double yoffset);
    void mouse_button_action(int button, int action, int mods);
    void cursor_position_action(double xpos, double ypos);
    void checkExitStatus(GLFWwindow* window,int key);
    void transformBody(GLFWwindow *window,glm::vec3 *offset,glm::mat4 *view );
    void renderEarth();
    std::string readFile(const char* path);
    void simLoop();
    void populateSphereVertices(double radius,std::vector<float> *vertices,std::vector<unsigned int> *indices);
    void populateCylinderVertices(double radius,double height,std::vector<float> *vertices,std::vector<unsigned int> *indices);
    void populateCylinderVertices(double radius, double height, SimObject *obj);
    void populateConeVertices(double radius, double height, SimObject *obj);
    int testGL();
    void guidInit(std::string gInitFile);
    void setGuidObj(CGuidance *g);
    void checkSymStart(GLFWwindow* window,int key);
    void checkSymStop(GLFWwindow* window,int key);


};

#endif