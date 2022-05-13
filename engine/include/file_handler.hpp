#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include <vector>
#include <string>
#include <fstream>

#include "error_handler.hpp"
#include "point.hpp"
#include "filters.hpp"



ErrorCode files_reader(const std::string &model_fn,
                       std::vector<CartPoint3d> &vertexes,
                       std::vector<CartPoint3d> &normals);

#endif