#ifndef POINT_HPP
#define POINT_HPP

#include <fstream>
#include <cmath>
#include <cassert>
#include <iomanip>
#include <array>

const long double PI { 2.L * std::acos(0.L) };



typedef double angle_t;

struct CartPoint3d {
    double x, y, z;

    CartPoint3d();
    CartPoint3d(double x, double y, double z);

    std::array<double, 3> as_array() const;
};

/**
 * radius is the distance to origin
 * zOx    is the angle in degrees with the posivive z axis (counterclockwise)
 * yOp  is the angle in degrees with the positive y axis (clockwise)
 */
struct PolarPoint3d {
    double radius;
    angle_t zOx, yOp;

    PolarPoint3d();
    PolarPoint3d(double radius, angle_t zOx, angle_t yOp);
};



angle_t degree_to_radian(angle_t degree);
angle_t radian_to_degree(angle_t radian);

PolarPoint3d cart_to_polar(CartPoint3d const &p);
CartPoint3d polar_to_cart(PolarPoint3d const &p);

std::ostream& operator<<(std::ostream& stream, PolarPoint3d const &p);
std::ostream& operator<<(std::ostream& stream, CartPoint3d const &p);
std::istream& operator>>(std::istream& stream, PolarPoint3d& p);
std::istream& operator>>(std::istream& stream, CartPoint3d& p);

#endif