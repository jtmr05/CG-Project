#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include <string>
#include <vector>
#include <tuple>

#include "point.hpp"
#include "tinyxml.hpp"
#include "error_handler.hpp"
#include "filters.hpp"
#include "data_structures.hpp"



std::tuple<
    ErrorCode,
    CameraSettings,
    std::vector<std::unique_ptr<Group>>,
    std::vector<std::unique_ptr<Light>>>
xml_parser(const std::string &xml_path);

#endif