#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <string>

using std::string;

typedef enum primitive {
    plane,
    box,
    sphere,
    cone,
    __invalid,
} Primitive;

Primitive from_string(const string);
int primitive_switch(string[], const int);

#endif
