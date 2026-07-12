#include "quaternion.h"

Quaternion::Quaternion()  {
    q0 = 0;
    q1 = 0;
    q2 = 0;
    q3 = 0;
}

Quaternion::Quaternion(double q0, double q1, double q2, double q3){
    this->q0 = q0;
    this->q1 = q1;
    this->q2 = q2;
    this->q3 = q3;
}

Quaternion Quaternion::operator+(const Quaternion& other) const {
    return Quaternion(q0 + other.q0, q1 + other.q1, q2 + other.q2, q3 + other.q3);
}  

Quaternion Quaternion::operator*(const Quaternion& other) const {
    double new_q0 = q0 * other.q0 - q1 * other.q1 - q2 * other.q2 - q3 * other.q3;
    double new_q1 = q0 * other.q1 + q1 * other.q0 + q2 * other.q3 - q3 * other.q2;
    double new_q2 = q0 * other.q2 - q1 * other.q3 + q2 * other.q0 + q3 * other.q1;
    double new_q3 = q0 * other.q3 + q1 * other.q2 - q2 * other.q1 + q3 * other.q0;

    return Quaternion(new_q0, new_q1, new_q2, new_q3);
}

void Quaternion::normalize() {
    double norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    if (norm > 0) {
        q0 /= norm;
        q1 /= norm;
        q2 /= norm;
        q3 /= norm;
    }
}

Quaternion Quaternion::conjugate() const {
    return Quaternion(q0, -q1, -q2, -q3);
}

phyVector Quaternion::rotateVector(const phyVector& v) const {
    Quaternion v_quat(0, v.x, v.y, v.z);
    Quaternion q_conjugate(q0, -q1, -q2, -q3);

    Quaternion rotated_quat = (*this) * v_quat * q_conjugate;

    return phyVector(rotated_quat.q1, rotated_quat.q2, rotated_quat.q3);
}

Quaternion Quaternion::getRotationQuaternion(phyVector Xb, phyVector Yb, phyVector Zb) {
    // Assuming Xb, Yb, Zb are orthonormal vectors representing the body frame axes in the inertial frame
    double trace = Xb.x + Yb.y + Zb.z;
    double q0, q1, q2, q3;

    if (trace > 0) {
        double s = 0.5 / sqrt(trace + 1.0);
        q0 = 0.25 / s;
        q1 = (Yb.z - Zb.y) * s;
        q2 = (Zb.x - Xb.z) * s;
        q3 = (Xb.y - Yb.x) * s;
    } else {
        if (Xb.x > Yb.y && Xb.x > Zb.z) {
            double s = 2.0 * sqrt(1.0 + Xb.x - Yb.y - Zb.z);
            q0 = (Yb.z - Zb.y) / s;
            q1 = 0.25 * s;
            q2 = (Yb.x + Xb.y) / s;
            q3 = (Zb.x + Xb.z) / s;
        } else if (Yb.y > Zb.z) {
            double s = 2.0 * sqrt(1.0 + Yb.y - Xb.x - Zb.z);
            q0 = (Zb.x - Xb.z) / s;
            q1 = (Yb.x + Xb.y) / s;
            q2 = 0.25 * s;
            q3 = (Zb.y + Yb.z) / s;
        } else {
            double s = 2.0 * sqrt(1.0 + Zb.z - Xb.x - Yb.y);
            q0 = (Xb.y - Yb.x) / s;
            q1 = (Zb.x + Xb.z) / s;
            q2 = (Zb.y + Yb.z) / s;
            q3 = 0.25 * s;
        }
    }

    return Quaternion(q0, q1, q2, q3);
}

double Quaternion::getRotationAngle() const {
    return 2 * acos(q0);
}

phyVector Quaternion::getRotationAxis() const {
    double sin_half_angle = sqrt(1 - q0 * q0);
    if (sin_half_angle == 0) {
        return phyVector(0, 0, 1); // Default axis if no rotation
    }
    return phyVector(q1 / sin_half_angle, q2 / sin_half_angle, q3 / sin_half_angle);
}

Quaternion::Quaternion(double angle, phyVector axis) {
    double half_angle = angle / 2.0;
    q0 = cos(half_angle);
    double sin_half_angle = sin(half_angle);
    q1 = axis.x * sin_half_angle;
    q2 = axis.y * sin_half_angle;
    q3 = axis.z * sin_half_angle;
}