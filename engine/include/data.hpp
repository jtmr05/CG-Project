#ifndef DATA_HPP
#define DATA_HPP

#include <string>
#include <vector>
#include <memory>
#include <array>

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

    public:
        Transform();
        virtual TransformType get_type() const = 0;
};



class StaticRotate : public Transform {

    public:
        angle_t angle;
        CartPoint3d point;

        StaticRotate(angle_t angle, const CartPoint3d &point);
        TransformType get_type() const;
};

class DynamicRotate : public Transform {

    public:
        unsigned time;
        CartPoint3d point;

        DynamicRotate(unsigned time, const CartPoint3d &point);
        TransformType get_type() const;
};

class StaticTranslate : public Transform {

    public:
        CartPoint3d point;

        StaticTranslate(const CartPoint3d &point);
        TransformType get_type() const;
};

class DynamicTranslate : public Transform {

    public:
        unsigned time;
        bool align;
        std::vector<CartPoint3d>* points;

        DynamicTranslate(
            unsigned time, bool align,
            std::unique_ptr<std::vector<CartPoint3d>> &points
        );

        ~DynamicTranslate(); //destrutor to free points

        TransformType get_type() const;
};

class Scale : public Transform {

    public:
        CartPoint3d point;

        Scale(const CartPoint3d &point);
        TransformType get_type() const;
};



struct Group {

    std::vector<std::unique_ptr<Transform>> transforms;
    std::vector<std::string> models;
    unsigned int nest_level;

    Group(unsigned int nest_level);
};



template<class T, size_t rows, size_t columns>
struct Matrix {

    std::array<std::array<T, columns>, rows> m;

    constexpr std::array<T, columns>& operator[](size_t i){
        return this->m[i];
    }
};

#endif