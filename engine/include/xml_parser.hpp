#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include <string>
#include <vector>

#include "point.hpp"
#include "tinyxml.h"
#include "error_handler.hpp"
#include "filters.hpp"


struct CameraSettings {

    CartPoint3d position, look_at, up;
    double fov, near_, far_;

    CameraSettings();
};



enum TransformType {
    static_rotate,
    dynamic_rotate,
    static_translate,
    dynamic_translate,
    scale,
};

class Transform {

    public:
        const TransformType type;

    protected:                    //visibility limited to subclasses
        Transform(TransformType type);
};



class StaticRotate : public Transform {

    public:
        angle_t angle;
        CartPoint3d point;

        StaticRotate(angle_t angle, const CartPoint3d &point);
};

class DynamicRotate : public Transform {

    public:
        double time;
        CartPoint3d point;

        DynamicRotate(double time, const CartPoint3d &point);
};

class StaticTranslate : public Transform {

    public:
        CartPoint3d point;

        StaticTranslate(const CartPoint3d &point);
};

class DynamicTranslate : public Transform {

    public:
        double time;
        bool align;
        std::vector<CartPoint3d> *points;

        DynamicTranslate(
            double time, bool align,
            std::unique_ptr<std::vector<CartPoint3d>> &points
        );

        ~DynamicTranslate(); //destrutor to free points
};

class Scale : public Transform {

    public:
        CartPoint3d point;

        Scale(const CartPoint3d &point);
};



struct Group {

    std::vector<Transform> transforms;
    std::vector<std::string> models;
    unsigned int nest_level;

    Group(unsigned int nest_level);
};

ErrorCode xml_parser(const std::string &path, CameraSettings &c, std::vector<Group> &groups);

#endif