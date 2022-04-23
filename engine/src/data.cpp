#include "data.hpp"

using std::string;
using std::vector;
using std::unique_ptr;



CameraSettings::CameraSettings(){
    this->position = {};
    this->look_at = {};
    this->up = { 0.0, 1.0, 0.0 };
    this->fov = { 60.0 };
    this->near_ = { 1.0 };
    this->far_ = { 1000.0 };
}



Transform::Transform(TransformType type) : type{ type } {}

TransformType Transform::get_type() const {
    return this->type;
}



StaticRotate::StaticRotate(angle_t angle, const CartPoint3d &point) :
    Transform(TransformType::static_rotate){       //call super constructor
            this->angle = { angle };
            this->point = { point };
}

DynamicRotate::DynamicRotate(double time, const CartPoint3d &point) :
    Transform(TransformType::dynamic_rotate){
        this->time = { time };
        this->point = { point };
}



StaticTranslate::StaticTranslate(const CartPoint3d &point) :
    Transform(TransformType::static_rotate){
        this->point = { point };
}

DynamicTranslate::DynamicTranslate(double time, bool align, unique_ptr<vector<CartPoint3d>> &points) :
    Transform(TransformType::dynamic_translate){

        this->time = { time };
        this->align = { align };
        this->points = { points.release() };
}

DynamicTranslate::~DynamicTranslate(){
    delete this->points;
}



Scale::Scale(const CartPoint3d &point) :
    Transform(TransformType::scale){
        this->point = { point };
}



Group::Group(unsigned int nest_level){
    this->nest_level = { nest_level };
    this->models = {};
    this->transforms = {};
}