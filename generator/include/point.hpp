#ifndef POINT_HPP
#define POINT_HPP

#include <fstream>

typedef struct point3d {
    double x, y, z;
} Point3d;

std::ostream& operator<<(std::ostream& stream, Point3d const &p);
std::istream& operator>>(std::istream& stream, Point3d& p);

#endif