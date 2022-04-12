#include "point.hpp"


//arbitrary number of decimal places output to a file when using << operator
static const int PRECISION { 30 };


angle_t degree_to_radian(angle_t degree){
    return (degree * 2.0 * PI) / 360.0;
}

angle_t radian_to_degree(angle_t radian){
    return (radian * 360.0) / (2.0 * PI);
}



cart_point3d::cart_point3d(){
    this->x = 0.0;
    this->y = 0.0;
    this->z = 0.0;
}

cart_point3d::cart_point3d(double x, double y, double z){
    this->x = x;
    this->y = y;
    this->z = z;
}

polar_point3d::polar_point3d(){
    this->radius = 0.0;
    this->zOx = 0.0;
    this->yOp = 0.0;
}

polar_point3d::polar_point3d(double radius, angle_t zOx, angle_t yOp){

    assert(yOp >= 0.0 && yOp <= 180.0);

    this->radius = radius;
    this->zOx = zOx;
    this->yOp = yOp;
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
    stream << std::fixed << std::setprecision(PRECISION) << p.x << ";" << p.y << ";" << p.z << '\n';
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
    char c {};    //used to ignore ';'

    if(stream >> tmp.x >> c >> tmp.y >> c >> tmp.z){
        p.x = tmp.x;
        p.y = tmp.y;
        p.z = tmp.z;
    }

    return stream;
}