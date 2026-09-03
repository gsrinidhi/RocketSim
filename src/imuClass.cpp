#include "imuClass.hpp"

imuClass::imuClass() {
    
}

void imuClass::setLogFile(std::ofstream *file) {
    logfile = file;
}

void imuClass::getRotationRates(universalPkt imuInput) {
    cmd_q = imuInput.cmd_q;
    dt = imuInput.dt;
    phyVector body_x_new = cmd_q.rotateVector(phyVector(1,0,0));
    phyVector body_y_new = cmd_q.rotateVector(phyVector(0,1,0));
    phyVector body_z_new = cmd_q.rotateVector(phyVector(0,0,1));
    glm::mat3 R_new = glm::mat3(
        glm::vec3(body_x_new.x, body_x_new.y, body_x_new.z),
        glm::vec3(body_y_new.x, body_y_new.y, body_y_new.z),
        glm::vec3(body_z_new.x, body_z_new.y, body_z_new.z)
    );
    glm::mat3 R_old = glm::mat3(
        glm::vec3(body_x.x, body_x.y, body_x.z),
        glm::vec3(body_y.x, body_y.y, body_y.z),
        glm::vec3(body_z.x, body_z.y, body_z.z)
    );
    glm::mat3 R_delta = glm::transpose(R_new) * R_old;
    roll_rate = atan2(R_delta[2][1], R_delta[2][2]) / dt;
    pitch_rate = -asin(R_delta[2][0]) / dt;
    yaw_rate = atan2(R_delta[1][0], R_delta[0][0]) / dt;
    body_x = body_x_new;
    body_y = body_y_new;
    body_z = body_z_new;
}

void imuClass::getAcceleration(universalPkt imuInput) {
    ax = imuInput.a * body_x;
    ay = imuInput.a * body_y;
    az = imuInput.a * body_z;
}

void imuClass::getIMUOutput(universalPkt imuInput, universalPkt *navPkt) {
    getRotationRates(imuInput);
    getAcceleration(imuInput);
    navPkt->ax = ax;
    navPkt->ay = ay;
    navPkt->az = az;
    navPkt->wx = roll_rate;
    navPkt->wy = pitch_rate;
    navPkt->wz = yaw_rate;
    navPkt->dt = dt;
}

void imuClass::setBodyAxes(Quaternion cmd_q) {
    body_x = cmd_q.rotateVector(phyVector(1,0,0));
    body_y = cmd_q.rotateVector(phyVector(0,1,0));
    body_z = cmd_q.rotateVector(phyVector(0,0,1));
}