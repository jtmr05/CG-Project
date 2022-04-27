#include "data.hpp"

using std::string;
using std::vector;
using std::unique_ptr;
using std::array;



CameraSettings::CameraSettings(){
    this->position = {};
    this->look_at = {};
    this->up = { 0.0, 1.0, 0.0 };
    this->fov = { 60.0 };
    this->near_ = { 1.0 };
    this->far_ = { 1000.0 };
}



Transform::Transform(){}



StaticRotate::StaticRotate(angle_t angle, const CartPoint3d &point){
    this->angle = { angle };
    this->point = { point };
}

TransformType StaticRotate::get_type() const {
    return TransformType::static_rotate;
}



DynamicRotate::DynamicRotate(unsigned time, const CartPoint3d &point){
    this->time = { time };
    this->point = { point };
}

TransformType DynamicRotate::get_type() const {
    return TransformType::dynamic_rotate;
}



StaticTranslate::StaticTranslate(const CartPoint3d &point){
    this->point = { point };
}

TransformType StaticTranslate::get_type() const {
    return TransformType::static_translate;
}



DynamicTranslate::DynamicTranslate(unsigned time, bool align, unique_ptr<vector<CartPoint3d>> &points){
    this->time = { time };
    this->align = { align };
    this->points = { points.release() };
}

DynamicTranslate::~DynamicTranslate(){
    delete this->points;
}

TransformType DynamicTranslate::get_type() const {
    return TransformType::dynamic_translate;
}



Scale::Scale(const CartPoint3d &point){
    this->point = { point };
}

TransformType Scale::get_type() const {
    return TransformType::scale;
}



Group::Group(unsigned int nest_level){
    this->nest_level = { nest_level };
}