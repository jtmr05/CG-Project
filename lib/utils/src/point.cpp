#include "point.hpp"


angle_t degree_to_radian(angle_t degree){

    while(degree < 0.0)
        degree += 360.0;

    while(degree > 360.0)
        degree -= 360.0;

    return (degree * 2.0 * PI) / 360.0;
}

angle_t radian_to_degree(angle_t radian){

    const double two_pi = 2.0 * PI;

    while(radian < 0.0)
        radian += two_pi;

    while(radian > two_pi)
        radian -= two_pi;

    return (radian * 360.0) / two_pi;
}

CartPoint3d polar_to_cart(PolarPoint3d const &p){

    angle_t phi { degree_to_radian(p.phi) }, theta { degree_to_radian(p.theta) };

    CartPoint3d res { p.radius, p.radius, p.radius };

    res.x *= sin(p.phi) * sin(p.theta);
    res.y *= cos(p.theta);
    res.z *= cos(p.phi) * sin(p.theta);

    //https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

    return res;
}

PolarPoint3d cart_to_polar(CartPoint3d const &p){

    PolarPoint3d res{};

    res.radius = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);

    if(res.radius > 0.0)
        res.theta = radian_to_degree(acos(p.y / res.radius));

    res.phi = radian_to_degree(atan2(p.x, p.z) + PI);

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