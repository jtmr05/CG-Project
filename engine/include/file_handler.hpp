#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include <vector>
#include <string>
#include <fstream>

#include "error_handler.hpp"
#include "point.hpp"



ErrorCode point_reader(
    const std::string &model_fn,
    std::vector<CartPoint3d> &vertexes
    /*std::vector<CartPoint3d> &normals,
    std::vector<CartPoint3d> &text_coords*/
);

#endif