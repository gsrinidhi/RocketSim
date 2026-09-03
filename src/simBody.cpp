#include "simBody.hpp"

SimBody::SimBody() {
    body_x = phyVector(1, 0, 0);
    body_y = phyVector(0, 1, 0);
    body_z = phyVector(0, 0, 1);
}

void SimBody::addObject( SimObject& obj,  phyVector& offset, Quaternion orientation) {
    objects.push_back(obj);
    offset_positions.push_back(offset);
    orientations.push_back(orientation);
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

void SimBody::updateState(Quaternion cmd_q, phyVector acceleration,double dt,double scale) {
    a = acceleration;
    v = v + acceleration * dt;
    s = s + v * dt;
    body_x = cmd_q.rotateVector(phyVector(1,0,0));
    body_y = cmd_q.rotateVector(phyVector(0,1,0));
    body_z = cmd_q.rotateVector(phyVector(0,0,1));
    for (size_t i = 0; i < objects.size(); ++i) {
        offset = body_x * offset_positions[i].x + body_y * offset_positions[i].y + body_z * offset_positions[i].z;
        objects[i].updateState(cmd_q * orientations[i],s,dt,offset);
    }
}

void SimBody::drawUsingState(double scale,const glm::mat4& view, const glm::mat4& projection, GLuint shaderProgram) {
    for (size_t i = 0; i < objects.size(); ++i) {
        if (isDrawable.size() > i && isDrawable[i] == 0) {
            continue; // Skip drawing this object if it's marked as not drawable
        }
        objects[i].drawUsingState(scale,view,projection,shaderProgram);
    }
}

void SimBody::setPosition(phyVector s) {
    this->s = s;
}

void SimBody::setVelocity(phyVector v) {
    this->v = v;
}

void SimBody::rotateAlongBodyAxis(int axis,double angle) {
    phyVector rot_Axis;
    if(axis == 1) {
        rot_Axis = body_x;
    } else if(axis == 2) {
        rot_Axis = body_y;
    } else {
        rot_Axis = body_z;
    }

    Quaternion q(angle,rot_Axis);
    body_x = q.rotateVector(body_x);
    body_y = q.rotateVector(body_y);
    body_z = q.rotateVector(body_z);
}

void SimBody::getBodyAxes(phyVector *x,phyVector *y, phyVector *z) {
    *x = body_x;
    *y = body_y;
    *z = body_z;
}

// void SimBody::getRotationRates(Quaternion *cmd_q, double time,double *roll_rate, double *pitch_rate, double *yaw_rate) {
//     //cmd_q is the new orientation of the body frame with respect to the inertial frame. The rotation rates are the angular velocities about the body frame axes.
//     phyVector body_x_new = cmd_q->rotateVector(phyVector(1,0,0));
//     phyVector body_y_new = cmd_q->rotateVector(phyVector(0,1,0));
//     phyVector body_z_new = cmd_q->rotateVector(phyVector(0,0,1));
//     glm::mat3 R_new = glm::mat3(
//         glm::vec3(body_x_new.x, body_x_new.y, body_x_new.z),
//         glm::vec3(body_y_new.x, body_y_new.y, body_y_new.z),
//         glm::vec3(body_z_new.x, body_z_new.y, body_z_new.z)
//     );
//     glm::mat3 R_old = glm::mat3(
//         glm::vec3(body_x.x, body_x.y, body_x.z),
//         glm::vec3(body_y.x, body_y.y, body_y.z),
//         glm::vec3(body_z.x, body_z.y, body_z.z)
//     );
//     glm::mat3 R_delta = glm::transpose(R_new) * R_old;
//     *roll_rate = atan2(R_delta[2][1], R_delta[2][2]) / time;
//     *pitch_rate = -asin(R_delta[2][0]) / time;
//     *yaw_rate = atan2(R_delta[1][0], R_delta[0][0]) / time;
//     phyVector delta_x = body_x_new - body_x;
// }

// void SimBody::getAcceleration(double *ax,double *ay,double *az) {
//     *ax = a * body_x;
//     *ay = a * body_y;
//     *az = a * body_z;
// }

// void SimBody::iterateFunction(void *args) {
//     // Placeholder for the iterate function implementation
//     // This function can be used to perform iterative updates or calculations on the SimBody
//     imuInput.cmd_q = Quaternion(1,0,0,0); // Identity quaternion
// }