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
    CartPoint3d normalize() const;
};

struct CartPoint2d {
    double x, y;

    CartPoint2d();
    CartPoint2d(double x, double y);
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

    CartPoint3d normalize() const;
};

struct PolarPoint2d {
    double radius;
    angle_t xOy;

    PolarPoint2d();
    PolarPoint2d(double radius, angle_t xOy);
};



angle_t degree_to_radian(angle_t degree);
angle_t radian_to_degree(angle_t radian);



CartPoint3d polar_to_cart(PolarPoint3d const &p);
CartPoint2d polar_to_cart(PolarPoint2d const &p);
PolarPoint3d cart_to_polar(CartPoint3d const &p);
PolarPoint2d cart_to_polar(CartPoint2d const &p);



std::ostream& operator<<(std::ostream& stream, PolarPoint3d const &p);
std::ostream& operator<<(std::ostream& stream, PolarPoint2d const &p);
std::ostream& operator<<(std::ostream& stream, CartPoint3d const &p);
std::ostream& operator<<(std::ostream& stream, CartPoint2d const &p);
std::istream& operator>>(std::istream& stream, CartPoint3d& p);
std::istream& operator>>(std::istream& stream, CartPoint2d& p);


CartPoint3d operator*(const CartPoint3d &p, double d);
CartPoint3d operator*(double d, const CartPoint3d &p);
CartPoint3d operator+(const CartPoint3d &p1, const CartPoint3d &p2);
void operator+=(CartPoint3d &p1, const CartPoint3d &p2);

CartPoint3d cross_product(const CartPoint3d &p1, const CartPoint3d &p2);

#endif