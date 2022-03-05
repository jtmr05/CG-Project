#ifndef POINT_HPP
#define POINT_HPP

#include <fstream>
#include <cmath>

const double PI { 2 * acos(0.0) };


typedef double angle_t;


typedef struct cart_point3d {
    double x, y, z;
} CartPoint3d;

/**
 * radius is the distance to origin
 * theta  is the angle in degrees with the positive y axis (counterclockwise)
 * phi    is the angle in degrees with the posivive z axis (counterclockwise)
 */
typedef struct polar_point3d {
    double radius;
    angle_t phi, theta;
} PolarPoint3d;


PolarPoint3d cart_to_polar(CartPoint3d const &p);
CartPoint3d polar_to_cart(PolarPoint3d const &p);

std::ostream& operator<<(std::ostream& stream, PolarPoint3d const &p);
std::ostream& operator<<(std::ostream& stream, CartPoint3d const &p);
std::istream& operator>>(std::istream& stream, PolarPoint3d& p);
std::istream& operator>>(std::istream& stream, CartPoint3d& p);

#endif