#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP

#include <string>
#include <vector>
#include <memory>
#include <array>
#include <optional>

#include "point.hpp"



/** Camera **/

struct CameraSettings {

    CartPoint3d position, look_at, up;
    double fov, near_, far_;

    CameraSettings();
};



/** Transforms **/

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
        std::unique_ptr<std::vector<CartPoint3d>> points;

        DynamicTranslate(
            unsigned time, bool align,
            std::unique_ptr<std::vector<CartPoint3d>> &points
        );

        TransformType get_type() const;
};

class Scale : public Transform {

    public:
        CartPoint3d point;

        Scale(const CartPoint3d &point);
        TransformType get_type() const;
};



/** Models **/

enum RGBIndex { r = 0, g = 1, b = 2 };

class RGB {

    private:
        std::array<uint8_t, 3> arr;

    public:
        constexpr uint8_t& operator[](size_t i){
            return this->arr[i];
        }

        constexpr uint8_t& operator[](RGBIndex i){
            return this->arr[static_cast<size_t>(i)];
        }

        RGB(uint8_t r, uint8_t g, uint8_t b);
};

struct Color {

    RGB diffuse;
    RGB ambient;
    RGB specular;
    RGB emissive;
    unsigned shininess;

    Color();
    Color(const RGB& diffuse, const RGB& ambient,
          const RGB& specular, const RGB& emissive,
          unsigned shininess);
};

struct Model {

    std::string model_filename;
    std::optional<std::string> texture_filename;
    Color color;

    Model(std::string&& model_fn, const Color& color);
    Model(std::string&& model_fn, std::string&& texture_fn, const Color& color);
};



/** Groups **/

struct Group {

    std::vector<std::unique_ptr<Transform>> transforms;
    std::vector<Model> models;
    unsigned nest_level;

    Group(unsigned nest_level);
};



/** Lights **/

enum LightType {
    point,
    directional,
    spotlight,
};

class Light {

    public:
        Light();
        virtual LightType get_type() const = 0;
};

class PointLight : public Light {

    public:
        CartPoint3d pos;

        PointLight(const CartPoint3d &pos);
        LightType get_type() const;
};

class DirectionalLight : public Light {

    public:
        CartPoint3d dir;

        DirectionalLight(const CartPoint3d &dir);
        LightType get_type() const;
};

class Spotlight : public Light {

    public:
        CartPoint3d pos;
        CartPoint3d dir;
        unsigned cutoff;

        Spotlight(const CartPoint3d &pos, const CartPoint3d &dir, unsigned cutoff);
        LightType get_type() const;
};



/** Matrix **/

template<class T, size_t rows, size_t columns>
struct Matrix {

    std::array<std::array<T, columns>, rows> m;

    constexpr std::array<T, columns>& operator[](size_t i){
        return this->m[i];
    }
};



/** Runtime Constants **/

class InvalidAssignment : public std::exception {

    virtual const char* what() const throw(){
        return "Illegal assignment";
    }
};

template <typename T>
class Constant {

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
                throw InvalidAssignment{};

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