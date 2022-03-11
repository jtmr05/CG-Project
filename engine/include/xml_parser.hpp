#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include <string>
#include <vector>

#include "point.hpp"
#include "tinyxml.h"
#include "error_handler.hpp"
#include "filters.hpp"


typedef struct camera_settings {

    CartPoint3d position, look_at, up;
    double fov, near_, far_;

    camera_settings();

} CameraSettings;

ErrorCode xml_parser(const std::string &path, CameraSettings &c, std::vector<std::string> &files_3d);

#endif