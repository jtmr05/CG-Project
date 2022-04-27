#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include <string>
#include <vector>

#include "point.hpp"
#include "tinyxml.h"
#include "error_handler.hpp"
#include "filters.hpp"
#include "data.hpp"



ErrorCode xml_parser(
    const std::string &path,
    CameraSettings &c,
    std::vector<std::unique_ptr<Group>> &groups
);

#endif