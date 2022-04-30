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


PolarPoint3d::PolarPoint3d() :
    radius(0.0), zOx(0.0), yOp(0.0) {}

PolarPoint3d::PolarPoint3d(double radius, angle_t zOx, angle_t yOp) :
    radius(radius), zOx(zOx), yOp(yOp)
{
    assert(yOp >= 0.0 && yOp <= 180.0);
}



CartPoint3d polar_to_cart(PolarPoint3d const &p){

    angle_t zOx { degree_to_radian(p.zOx) }, yOp { degree_to_radian(p.yOp) };

    CartPoint3d res { p.radius, p.radius, p.radius };

    res.x *= std::sin(zOx) * std::sin(yOp);
    res.y *= std::cos(yOp);
    res.z *= std::cos(zOx) * std::sin(yOp);

    //https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

    return res;
}

PolarPoint3d cart_to_polar(CartPoint3d const &p){

    PolarPoint3d res{};

    res.radius = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);

    if(res.radius > 0.0)
        res.yOp = radian_to_degree(std::acos(p.y / res.radius));

    res.zOx = radian_to_degree(std::atan2(p.x, p.z));
    res.zOx = (res.zOx < 0.0) ? res.zOx + 360.0 : res.zOx;

    //https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

    return res;
}

std::ostream& operator<<(std::ostream& stream, PolarPoint3d const &p){
    stream << polar_to_cart(p);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, CartPoint3d const &p){
    stream << std::fixed << std::setprecision(PRECISION) << p.x << "; " << p.y << "; " << p.z << '\n';
    return stream;
}

std::istream& operator>>(std::istream& stream, PolarPoint3d& p){
    CartPoint3d tmp {};
    stream >> tmp;
    p = cart_to_polar(tmp);
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