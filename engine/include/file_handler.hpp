#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include <vector>
#include <string>
#include <fstream>
#include <tuple>
#include <iostream>

#include "error_handler.hpp"
#include "point.hpp"
#include "filters.hpp"


std::tuple<ErrorCode, std::vector<CartPoint3d>, std::vector<CartPoint3d>, std::vector<CartPoint2d>>
files_reader(const std::string &model_fn);

#endif