#include "simBody.hpp"

SimBody::SimBody() {
    body_x = phyVector(1, 0, 0);
    body_y = phyVector(0, 1, 0);
    body_z = phyVector(0, 0, 1);
}

void SimBody::addObject( SimObject& obj,  phyVector& offset) {
    objects.push_back(obj);
    offset_positions.push_back(offset);
    isDrawable.push_back(1); // By default, mark the object as drawable
}

void SimBody::setBodyFrameAxes(const phyVector& x, const phyVector& y, const phyVector& z) {
    body_x = x;
    body_y = y;
    body_z = z;
}

void SimBody::setLoc(int modelLoc, int viewLoc, int projectionLoc,int colorLoc,int offsetLoc) {
    for (auto& obj : objects) {
        obj.setLoc(modelLoc, viewLoc, projectionLoc,colorLoc,offsetLoc);
    }
}

void SimBody::draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, GLuint shaderProgram) {
    for (size_t i = 0; i < objects.size(); ++i) {
        if (isDrawable.size() > i && isDrawable[i] == 0) {
            continue; // Skip drawing this object if it's marked as not drawable
        }
        offset = body_x * offset_positions[i].x + body_y * offset_positions[i].y + body_z * offset_positions[i].z;
        glm::mat4 model_with_offset = glm::translate(model, glm::vec3(offset.x, offset.y, offset.z));
        objects[i].draw(view, projection, model_with_offset, shaderProgram);
    }
}

void SimBody::updateDrawableFlags(const std::vector<int>& flags) {
    isDrawable = flags;
}