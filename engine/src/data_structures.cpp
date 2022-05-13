#include "data_structures.hpp"

using std::string;
using std::vector;
using std::unique_ptr;
using std::array;
using std::optional;



/** Camera **/

CameraSettings::CameraSettings() :
    position( {} ), look_at( {} ), up(0.0, 1.0, 0.0),
    fov(60.0), near(1.0), far(1000.0) {}



/** Transforms **/

Transform::Transform(){}



StaticRotate::StaticRotate(angle_t angle, const CartPoint3d &point) :
    angle(angle), point(point) {}

TransformType StaticRotate::get_type() const {
    return TransformType::static_rotate;
}



DynamicRotate::DynamicRotate(unsigned time, const CartPoint3d &point) :
    time(time), point(point) {}

TransformType DynamicRotate::get_type() const {
    return TransformType::dynamic_rotate;
}



StaticTranslate::StaticTranslate(const CartPoint3d &point) :
    point(point) {}

TransformType StaticTranslate::get_type() const {
    return TransformType::static_translate;
}



DynamicTranslate::DynamicTranslate(unsigned time, bool align, unique_ptr<vector<CartPoint3d>> &points) :
    time(time), align(align), points(points.release()) {}

TransformType DynamicTranslate::get_type() const {
    return TransformType::dynamic_translate;
}



Scale::Scale(const CartPoint3d &point) :
    point(point) {}

TransformType Scale::get_type() const {
    return TransformType::scale;
}



/** Models **/

RGB::RGB(uint8_t r, uint8_t g, uint8_t b) :
    arr( { r, g, b } ) {}

array<float, 4> RGB::as_float_array() const {

    constexpr float uint8_t_max_valuef { static_cast<float>(std::numeric_limits<uint8_t>::max()) };

    return {
        static_cast<float>(this->operator[](0)) / uint8_t_max_valuef,
        static_cast<float>(this->operator[](1)) / uint8_t_max_valuef,
        static_cast<float>(this->operator[](2)) / uint8_t_max_valuef,
        1.f
    };
}



Color::Color() :
    diffuse(200, 200, 200), ambient(50, 50, 50),
    specular(0, 0, 0), emissive(0, 0, 0),
    shininess(0) {}

Color::Color(const RGB& diffuse, const RGB& ambient,
             const RGB& specular, const RGB& emissive,
             unsigned shininess) :
    diffuse(diffuse), ambient(ambient),
    specular(specular), emissive(emissive),
    shininess(shininess) {}



Model::Model(string&& model_fn, const Color& color) :
    model_filename(model_fn),

    texture_filename(
        std::move(optional<string>{})
    ),

    color(color) {}

Model::Model(string&& model_fn, string&& texture_fn, const Color& color) :
    model_filename(model_fn),

    texture_filename(
        std::move(std::make_optional(std::move(texture_fn)))
    ),

    color(color) {}



/** Groups **/

Group::Group(unsigned nest_level) :
    transforms(), models(), nest_level(nest_level) {}



/** Lights **/

Light::Light(){}



PointLight::PointLight(const CartPoint3d& pos) :
    pos(pos) {}

LightType PointLight::get_type() const {
    return LightType::point;
}



DirectionalLight::DirectionalLight(const CartPoint3d& dir) :
    dir(dir) {}

LightType DirectionalLight::get_type() const {
    return LightType::directional;
}



Spotlight::Spotlight(const CartPoint3d &pos, const CartPoint3d &dir, int cutoff) :
    pos(pos), dir(dir), cutoff(cutoff)
{
    assert((cutoff >= 0 && cutoff <= 90) || cutoff == 180);
}

LightType Spotlight::get_type() const {
    return LightType::spotlight;
}