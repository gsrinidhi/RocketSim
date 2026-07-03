#ifndef SIMBODY_HPP
#define SIMBODY_HPP

#include "simObject.hpp"

class SimBody {
    std::vector<SimObject> objects;
    std::vector<phyVector> offset_positions; // Offset positions for each object
    std::vector<int> isDrawable; // Flags to indicate if each object should be drawn
    phyVector body_x, body_y, body_z; // Body frame axes
    phyVector offset;
public:
    SimBody();
    void addObject( SimObject& obj,  phyVector& offset);
    void setBodyFrameAxes(const phyVector& x, const phyVector& y, const phyVector& z);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, GLuint shaderProgram);
    void setLoc(int modelLoc, int viewLoc, int projectionLoc,int colorLoc,int offsetLoc);
    void setColor(double r, double g, double b);
    void updateDrawableFlags(const std::vector<int>& flags);
};

#endif // SIMBODY_HPP