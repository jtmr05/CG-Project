#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include <string>
#include <vector>
#include <optional>

#include "point.hpp"
#include "tinyxml.h"
#include "error_handler.hpp"
#include "filters.hpp"


typedef struct camera_settings {

    CartPoint3d position, look_at, up;
    double fov, near_, far_;

    camera_settings();

} CameraSettings;

typedef enum transform_type {
    rotate,
    translate,
    scale,
} TransformType;

typedef struct transform {

    CartPoint3d point;
    std::optional<angle_t> angle;
    TransformType type;

    transform(angle_t angle, const CartPoint3d &point);
    transform(TransformType type, const CartPoint3d &point);

} Transform;

typedef struct group {

    std::vector<Transform> transforms;
    std::vector<std::string> models;
    unsigned int nest_level;

    group(unsigned int nest_level);

} Group;

ErrorCode xml_parser(const std::string &path, CameraSettings &c, std::vector<Group> &groups);

#endif