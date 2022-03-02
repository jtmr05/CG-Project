#include "point.hpp"


std::ostream& operator<<(std::ostream& stream, Point3d const &p){
    stream << p.x << ";" << p.y << ";" << p.z << '\n';
    return stream;
}

std::istream& operator>>(std::istream& stream, Point3d& p){

    Point3d tmp {};
    char c {};

    if(stream >> tmp.x >> c >> tmp.y >> c >> tmp.z >> c){
        p.x = tmp.x;
        p.y = tmp.y;
        p.z = tmp.z;
    }

    return stream;
}