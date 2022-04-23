#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <string>
#include <fstream>
#include <cmath>
#include <vector>
#include <set>

#include "point.hpp"
#include "error_handler.hpp"
#include "xml_parser.hpp"
#include "vbo.hpp"
#include "interaction.hpp" //must be below vbo.hpp !!

#include <GL/glut.h>


ErrorCode start(int argc, char** argv);

#endif