#include "phyVector.h"

phyVector::phyVector() : x(0), y(0), z(0) {}

phyVector::phyVector(float x, float y, float z) : x(x), y(y), z(z) {}

phyVector phyVector::operator+(const phyVector& other) const {
    return phyVector(x + other.x, y + other.y, z + other.z);
}

phyVector phyVector::operator-(const phyVector& other) const {
    return phyVector(x - other.x, y - other.y, z - other.z);
}

phyVector phyVector::operator*(float scalar) const {
    return phyVector(x * scalar, y * scalar, z * scalar);
}

phyVector phyVector::operator/(float scalar) const {
    return phyVector(x / scalar, y / scalar, z / scalar);
}

phyVector phyVector::operator-() const {
    return phyVector(-x, -y, -z);
}

phyVector phyVector::operator^(const phyVector& other) const {
    return phyVector(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

double phyVector::operator*(const phyVector& other) const {
    return x * other.x + y * other.y + z * other.z;
}

double phyVector::magnitude() const {
    return sqrt(x * x + y * y + z * z);
}

double phyVector::getVerticalAngle() const {
    return atan2(z, sqrt(x * x + y * y));
}

double phyVector::getverticalAngle2D() const {
    return atan2(y, x);
}

double phyVector::getVerticalAngle2DDegrees() const {
    return getVerticalAngle() * (180.0 / M_PI);
}
