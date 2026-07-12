#ifndef SIMBODY_HPP
#define SIMBODY_HPP

#include "simObject.hpp"

class SimBody {
    std::vector<SimObject> objects;
    std::vector<phyVector> offset_positions; // Offset positions for each object
    std::vector<int> isDrawable; // Flags to indicate if each object should be drawn
    phyVector body_x, body_y, body_z; // Body frame axes
    phyVector offset;
    phyVector s,v,a;
public:
    SimBody();
    void addObject( SimObject& obj,  phyVector& offset);
    void setBodyFrameAxes(const phyVector& x, const phyVector& y, const phyVector& z);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, GLuint shaderProgram);
    void setLoc(int modelLoc, int viewLoc, int projectionLoc,int colorLoc,int offsetLoc);
    void setColor(double r, double g, double b);
    void updateDrawableFlags(const std::vector<int>& flags);
    void updateState(Quaternion cmd_q, phyVector acceleration,double dt,double scale);
    void drawUsingState(double scale,const glm::mat4& view, const glm::mat4& projection, GLuint shaderProgram);
    void setPosition(phyVector s);
    void setVelocity(phyVector v);
    void rotateAlongBodyAxis(int axis,double angle);
    void getBodyAxes(phyVector *x,phyVector *y, phyVector *z);
};

#endif // SIMBODY_HPP