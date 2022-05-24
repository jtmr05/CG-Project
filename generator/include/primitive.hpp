#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <string>
#include <regex>
#include <fstream>
#include <map>
#include <array>
#include <vector>

#include "error_handler.hpp"
#include "point.hpp"
#include "matrix.hpp"
#include "filters.hpp"


ErrorCode primitive_writer(int size, const std::string args[]);

#endif
