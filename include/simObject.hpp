#ifndef SIMOBJECT_HPP
#define SIMOBJECT_HPP

#include "phyVector.h"
#include "quaternion.h"
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SimObject {
    phyVector position;
    phyVector velocity;
    phyVector acceleration;
    double mass;
    phyVector body_x, body_y, body_z; // Body frame axes
    unsigned int vao,vbo,ebo; // OpenGL buffers for rendering
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int modelLoc, viewLoc, projectionLoc; // Uniform locations for shader
    int colorLoc, offsetLoc; // Uniform locations for color and offset
    double color[3]; // RGB color for rendering
    int drawLines; // Flag to determine if lines should be drawn along with filled polygons
    double rot_angle;
    phyVector rot_axis;
    public:
    SimObject();
    SimObject(phyVector position, phyVector velocity, phyVector acceleration, double mass);
    void setPosition(phyVector position);
    void setVelocity(phyVector velocity);
    void setAcceleration(phyVector acceleration);
    void setMass(double mass);
    phyVector getPosition() const;
    phyVector getVelocity() const;
    phyVector getAcceleration() const;
    double getMass() const;
    void setVerticesAndIndices(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    void setBodyFrameAxes(const phyVector& x, const phyVector& y, const phyVector& z);
    
    void initOpenGLBuffers();
    void setLoc(int modelLoc, int viewLoc, int projectionLoc,int colorLoc,int offsetLoc);
    void setColor(double r, double g, double b);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, GLuint shaderProgram);
    phyVector getBodyFrameX();
    phyVector getBodyFrameY();
    phyVector getBodyFrameZ();
    void addVertex(float x, float y, float z);
    void addIndex(unsigned int index);
    void setDrawLines(int drawLines);
    void updateState(Quaternion cmd_q, phyVector acceleration,double dt,double scale,phyVector offset);
    void updateState(Quaternion cmd_q, phyVector posRef,double dt,phyVector offset);
    void drawUsingState(double scale,const glm::mat4& view, const glm::mat4& projection, GLuint shaderProgram);
    // void updateState(phyVector position, phyVector velocity, phyVector acceleration, double mass);
    // void updateOrientation(const phyVector& x, const phyVector& y, const phyVector& z);
};

#endif // SIMOBJECT_HPP