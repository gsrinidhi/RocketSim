#ifndef PHYVECTOR_H
#define PHYVECTOR_H

#include <cmath>

class phyVector {
public:
    float x;
    float y;
    float z;

    phyVector();
    phyVector(float x, float y, float z);

    phyVector operator+(const phyVector& other) const;
    phyVector operator-(const phyVector& other) const;
    phyVector operator*(float scalar) const;
    phyVector operator/(float scalar) const;
    phyVector operator-() const; // Unary minus operator for negation
    double operator*(const phyVector& other) const; // Dot product operator
    phyVector operator^(const phyVector& other) const; // Cross product operator
    double magnitude() const;
    double getVerticalAngle() const;
    double getverticalAngle2D() const;
    double getVerticalAngle2DDegrees() const;
};

#endif // PHYVECTOR_H
