#ifndef QUATERNION_H
#define QUATERNION_H

#include <cmath>
#include "phyVector.h"

class Quaternion {
    double q0, q1, q2, q3;
public:
    Quaternion();
    Quaternion(double, double, double, double);
    Quaternion(double angle, phyVector axis);
    // Add other member functions as needed
    // Function to add two quaternions
    Quaternion operator+(const Quaternion& other) const;
    //function to multiply two quaternions
    Quaternion operator*(const Quaternion& other) const;
    // Function to normalize the quaternion
    void normalize();
    //Function to get the conjugate of the quaternion
    Quaternion conjugate() const ;
    //Function to rotate a vector using the quaternion
    phyVector rotateVector(const phyVector& v) const;
    //function to get the frame rotation quaternion from body frame to inertial frame given the body frame axes in inertial frame
    static Quaternion getRotationQuaternion(phyVector Xb, phyVector Yb, phyVector Zb);
    double getRotationAngle() const;
    phyVector getRotationAxis() const;

};

#endif // QUATERNION_H