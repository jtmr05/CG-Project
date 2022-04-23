#ifndef DATA_HPP
#define DATA_HPP

#include <string>
#include <vector>
#include <memory>

#include "point.hpp"


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

    private:
        TransformType type;

    protected:                    //visibility limited to subclasses
        Transform(TransformType type);

    public:
        TransformType get_type() const;
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

#endif