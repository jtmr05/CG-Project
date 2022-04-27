#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP

#include <string>
#include <vector>
#include <memory>
#include <array>
#include <optional>

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



template <typename T> class Constant {

    private:
        std::optional<T> v;

    public:
        Constant() = default;
        Constant(const Constant&) = delete;
        Constant& operator=(const Constant&) = delete;

        Constant& operator=(const T& x){
            if (!this->v.has_value())
                this->v = std::make_optional<T>(x);
            else
                std::terminate();

            return *this;

        }

        bool has_value() const {
            return this->v.has_value();
        }

        const T& value() const {
            return this->v.value();
        }
};

#endif