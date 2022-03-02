#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <string>
#include <regex>
#include <fstream>
#include <iostream>

#include "error_handler.hpp"
#include "point.hpp"


using std::string;

typedef enum primitive {
    plane,
    box,
    sphere,
    cone,
    __invalid,
} Primitive;

const int PLANE_ARGS {5};
const int BOX_ARGS {5};
const int CONE_ARGS {7};
const int SPHERE_ARGS {6};



int primitive_writer(const string[], const int);

#endif
