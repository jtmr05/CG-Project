#include "point.hpp"

using std::array;

//arbitrary number of decimal places output to a file when using << operator
static const int PRECISION { 30 };


angle_t degree_to_radian(angle_t degree){
    return (degree * 2.0 * PI) / 360.0;
}

angle_t radian_to_degree(angle_t radian){
    return (radian * 360.0) / (2.0 * PI);
}



CartPoint3d::CartPoint3d() :
    x(0.0), y(0.0), z(0.0) {}

CartPoint3d::CartPoint3d(double x, double y, double z) :
    x(x), y(y), z(z) {}

array<double, 3> CartPoint3d::as_array() const {
    return { this->x, this->y, this->z };
}



CartPoint2d::CartPoint2d() :
    x(0.0), y(0.0) {}

CartPoint2d::CartPoint2d(double x, double y) :
    x(x), y(y) {}



PolarPoint3d::PolarPoint3d() :
    radius(0.0), zOx(0.0), yOp(0.0) {}

PolarPoint3d::PolarPoint3d(double radius, angle_t zOx, angle_t yOp) :
    radius(radius), zOx(zOx), yOp(yOp)
{
    assert(yOp >= 0.0 && yOp <= 180.0);
}


PolarPoint2d::PolarPoint2d() :
    radius(0.0), xOy(0.0) {}

PolarPoint2d::PolarPoint2d(double radius, angle_t xOy) :
    radius(radius), xOy(xOy) {}



//https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

CartPoint3d polar_to_cart(PolarPoint3d const &p){

    const angle_t zOx_rad { degree_to_radian(p.zOx) };
    const angle_t yOp_rad { degree_to_radian(p.yOp) };

    const CartPoint3d res {
        p.radius * std::sin(zOx_rad) * std::sin(yOp_rad),
        p.radius * std::cos(yOp_rad),
        p.radius * std::cos(zOx_rad) * std::sin(yOp_rad)
    };

    return res;
}

CartPoint2d polar_to_cart(PolarPoint2d const &p){

    const angle_t xOy_rad { degree_to_radian(p.xOy) };

    const CartPoint2d res {
        p.radius * std::cos(xOy_rad),
        p.radius * std::sin(xOy_rad)
    };

    return res;
}

PolarPoint3d cart_to_polar(CartPoint3d const &p){

    PolarPoint3d res{};

    res.radius = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);

    if(res.radius > 0.0)
        res.yOp = radian_to_degree(std::acos(p.y / res.radius));

    res.zOx = radian_to_degree(std::atan2(p.x, p.z));
    res.zOx = (res.zOx < 0.0) ? res.zOx + 360.0 : res.zOx;

    return res;
}

PolarPoint2d cart_to_polar(CartPoint2d const &p){

    PolarPoint2d res{};

    res.radius = std::sqrt(p.x * p.x + p.y * p.y);

    res.xOy = radian_to_degree(std::atan2(p.y, p.x));
    res.xOy = (res.xOy < 0.0) ? res.xOy + 360.0 : res.xOy;

    return res;
}


std::ostream& operator<<(std::ostream& stream, PolarPoint3d const &p){
    stream << polar_to_cart(p);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, PolarPoint2d const &p){
    stream << polar_to_cart(p);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, CartPoint3d const &p){
    stream << std::fixed << std::setprecision(PRECISION) << p.x << "; " << p.y << "; " << p.z << '\n';
    return stream;
}

std::ostream& operator<<(std::ostream& stream, CartPoint2d const &p){
    stream << std::fixed << std::setprecision(PRECISION) << p.x << "; " << p.y << '\n';
    return stream;
}



std::istream& operator>>(std::istream& stream, CartPoint3d& p){

    CartPoint3d tmp {};
    char comma {};    //used to ignore ';'

    if(stream >> tmp.x >> comma >> tmp.y >> comma >> tmp.z){
        p.x = tmp.x;
        p.y = tmp.y;
        p.z = tmp.z;
    }

    return stream;
}

std::istream& operator>>(std::istream& stream, CartPoint2d& p){

    CartPoint2d tmp {};
    char comma {};    //used to ignore ';'

    if(stream >> tmp.x >> comma >> tmp.y){
        p.x = tmp.x;
        p.y = tmp.y;
    }

    return stream;
}



// https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal

CartPoint3d normal(const CartPoint3d &a, const CartPoint3d &b, const CartPoint3d &c) {

    const CartPoint3d u { b.x - a.x, b.y - a.y, b.z - a.z };
    const CartPoint3d v { c.x - a.x, c.y - a.y, c.z - a.z };

    return CartPoint3d{
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x
    };
}

CartPoint3d normal(const PolarPoint3d &a, const PolarPoint3d &b, const PolarPoint3d &c){
    return normal(polar_to_cart(a), polar_to_cart(b), polar_to_cart(c));
}