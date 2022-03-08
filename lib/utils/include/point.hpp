#ifndef POINT_HPP
#define POINT_HPP

#include <fstream>
#include <cmath>
#include <cassert>
#include <iomanip>

const long double PI { 2.L * std::acos(0.L) };



typedef double angle_t;

typedef struct cart_point3d {
    double x, y, z;

    cart_point3d();
    cart_point3d(double x, double y, double z);

} CartPoint3d;

/**
 * radius is the distance to origin
 * zOx    is the angle in degrees with the posivive z axis (counterclockwise)
 * yOp  is the angle in degrees with the positive y axis (clockwise)
 */
typedef struct polar_point3d {
    double radius;
    angle_t zOx, yOp;

    polar_point3d();
    polar_point3d(double radius, angle_t zOx, angle_t yOp);

} PolarPoint3d;



PolarPoint3d cart_to_polar(CartPoint3d const &p);
CartPoint3d polar_to_cart(PolarPoint3d const &p);

std::ostream& operator<<(std::ostream& stream, PolarPoint3d const &p);
std::ostream& operator<<(std::ostream& stream, CartPoint3d const &p);
std::istream& operator>>(std::istream& stream, PolarPoint3d& p);
std::istream& operator>>(std::istream& stream, CartPoint3d& p);

#endif