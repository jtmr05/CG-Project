#include "point.hpp"


angle_t degree_to_radian(angle_t degree){
    return (degree * 2.0 * PI) / 360.0;
}

angle_t radian_to_degree(angle_t radian){
    return (radian * 360.0) / (2.0 * PI);
}



cart_point3d::cart_point3d(){
    CartPoint3d::x = 0.0;
    CartPoint3d::y = 0.0;
    CartPoint3d::z = 0.0;
}

cart_point3d::cart_point3d(double x, double y, double z){
    CartPoint3d::x = x;
    CartPoint3d::y = y;
    CartPoint3d::z = z;
}

polar_point3d::polar_point3d(){
    PolarPoint3d::radius = 0.0;
    PolarPoint3d::zOx = 0.0;
    PolarPoint3d::yOp = 0.0;
}

polar_point3d::polar_point3d(double radius, angle_t zOx, angle_t yOp){

    assert(yOp >= 0.0 && yOp <= 180.0);

    PolarPoint3d::radius = radius;
    PolarPoint3d::zOx = zOx;
    PolarPoint3d::yOp = yOp;
}



CartPoint3d polar_to_cart(PolarPoint3d const &p){

    angle_t zOx { degree_to_radian(p.zOx) }, yOp { degree_to_radian(p.yOp) };

    CartPoint3d res { p.radius, p.radius, p.radius };

    res.x *= sin(zOx) * sin(yOp);
    res.y *= cos(yOp);
    res.z *= cos(zOx) * sin(yOp);

    //https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

    return res;
}

PolarPoint3d cart_to_polar(CartPoint3d const &p){

    PolarPoint3d res{};

    res.radius = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);

    if(res.radius > 0.0)
        res.yOp = radian_to_degree(acos(p.y / res.radius));

    res.zOx = radian_to_degree(atan2(p.x, p.z));
    res.zOx = (res.zOx < 0.0) ? res.zOx + 360.0 : res.zOx;

    //https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

    return res;
}

std::ostream& operator<<(std::ostream& stream, PolarPoint3d const &p){
    stream << polar_to_cart(p);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, CartPoint3d const &p){
    stream << p.x << ";" << p.y << ";" << p.z << '\n';
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
    char c {};    //used to ignore ';' and '\n'

    if(stream >> tmp.x >> c >> tmp.y >> c >> tmp.z >> c){
        p.x = tmp.x;
        p.y = tmp.y;
        p.z = tmp.z;
    }

    return stream;
}