#include "xml_parser.hpp"

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



static CameraSettings parse_camera_settings(TiXmlElement* p_world){

    CameraSettings c;

    TiXmlElement* p_camera { p_world->FirstChildElement("camera") };
    if (p_camera){

        TiXmlElement* p_position { p_camera->FirstChildElement("position") };
        if(p_position){
            const double x { string_to_double(p_position->Attribute("x")) };
            const double y { string_to_double(p_position->Attribute("y")) };
            const double z { string_to_double(p_position->Attribute("z")) };
            c.position = { x, y, z };
        }

        TiXmlElement* p_look_at { p_camera->FirstChildElement("lookAt") };
        if(p_look_at){
            const double x { string_to_double(p_look_at->Attribute("x")) };
            const double y { string_to_double(p_look_at->Attribute("y")) };
            const double z { string_to_double(p_look_at->Attribute("z")) };
            c.look_at = { x, y, z };
        }

        TiXmlElement* p_up { p_camera->FirstChildElement("up") };
        if(p_up){
            const double x { string_to_double(p_up->Attribute("x")) };
            const double y { string_to_double(p_up->Attribute("y")) };
            const double z { string_to_double(p_up->Attribute("z")) };
            c.up = { x, y, z };
        }

        TiXmlElement* p_projection { p_camera->FirstChildElement("projection") };
        if(p_projection){
            const double fov   { string_to_double(p_projection->Attribute("fov")) };
            const double near_ { string_to_double(p_projection->Attribute("near")) };
            const double far_  { string_to_double(p_projection->Attribute("far")) };
            c.fov = fov;
            c.near_ = near_;
            c.far_ = far_;
        }
    }

    return c;
}

static inline bool parse_dynamic_translate(
    TiXmlElement* p_generic_transform,
    unique_ptr<vector<CartPoint3d>> &points
){

    TiXmlElement* p_point { p_generic_transform->FirstChildElement("point") };

    while(p_point){

        const double x { string_to_double(p_point->Attribute("x")) };
        const double y { string_to_double(p_point->Attribute("y")) };
        const double z { string_to_double(p_point->Attribute("z")) };

        points->push_back( { x, y, z } );

        p_point = p_point->NextSiblingElement("point");
    }

    return points->size() >= 4lu;
}

static void parse_groups(TiXmlElement* p_world, const string &dir_prefix, vector<Group> &groups){

    std::stack<TiXmlElement*> group_rollback {};
    unsigned int nest_level { 1 };

    TiXmlElement* p_group { p_world->FirstChildElement("group") };
    while(p_group){

        assert(nest_level > 0);

        Group g { nest_level };

        TiXmlElement* p_transform { p_group->FirstChildElement("transform") };
        if(p_transform){

            TiXmlElement* p_generic_transform { p_transform->FirstChildElement() };
            while(p_generic_transform){

                const string attribute_name { p_generic_transform->Value() };

                const char* angle { p_generic_transform->Attribute("angle") };
                const char* time { p_generic_transform->Attribute("time") };
                const char* align { p_generic_transform->Attribute("align") };

                const double x { string_to_double(p_generic_transform->Attribute("x")) };
                const double y { string_to_double(p_generic_transform->Attribute("y")) };
                const double z { string_to_double(p_generic_transform->Attribute("z")) };

                Transform *pt { NULL };

                if(attribute_name == "rotate"){
                    if(angle){
                        StaticRotate sr { string_to_double(angle), { x, y, z } };
                        pt = &sr;
                    }
                    else if(time){
                        DynamicRotate dr { string_to_double(time), { x, y, z } };
                        pt = &dr;
                    }
                }
                else if(attribute_name == "translate"){
                    if(time){
                        unique_ptr<vector<CartPoint3d>> points { new vector<CartPoint3d>() };

                        if(parse_dynamic_translate(p_generic_transform, points)){
                            DynamicTranslate dt {
                                string_to_double(time),
                                string_to_bool(align),
                                points
                            };
                            pt = &dt;
                        }
                    }
                    else{
                        StaticTranslate st { { x, y, z } };
                        pt = &st;
                    }
                }
                else if(attribute_name == "scale"){
                    Scale s { { x, y, z } };
                    pt = &s;
                }

                if(pt)
                    g.transforms.push_back(*pt);

                p_generic_transform = p_generic_transform->NextSiblingElement();
            }
        }

        TiXmlElement* p_models { p_group->FirstChildElement("models") };
        if(p_models){

            TiXmlElement* p_model { p_models->FirstChildElement("model") };
            while(p_model){

                const string fn { p_model->Attribute("file") };
                if(has_3d_ext(fn))
                    g.models.push_back(dir_prefix + fn);

                p_model = p_model->NextSiblingElement("model");
            }
        }

        groups.push_back(g);

        TiXmlElement* parent { p_group };
        p_group = p_group->FirstChildElement("group");

        if(p_group){
            group_rollback.push(parent);
            ++nest_level;
        }
        else{
            p_group = parent->NextSiblingElement("group");

            if(!p_group && !group_rollback.empty()){
                p_group = group_rollback.top();
                group_rollback.pop();
                --nest_level;
                p_group = p_group->NextSiblingElement("group");
            }
        }
    }
}

ErrorCode xml_parser(const string &path, CameraSettings &c, vector<Group> &groups){

    ErrorCode exit_code { ErrorCode::success };
    TiXmlDocument doc {};

    // Load the XML file into the Doc instance
    if(!has_xml_ext(path))
        exit_code = ErrorCode::invalid_file_extension;

    else if(doc.LoadFile(path.c_str())){

        const std::size_t last_slash_pos { path.find_last_of("/") };
        string directory_path { "" };

        if(last_slash_pos < std::string::npos){
            char aux[last_slash_pos + 1 + 1];
            path.copy(aux, last_slash_pos + 1);
	        aux[last_slash_pos + 1] = '\0';
            directory_path = { aux };
	    }

        // Get root Element
        TiXmlElement* p_world { doc.RootElement() };
        if(p_world){
            c = parse_camera_settings(p_world);
            parse_groups(p_world, directory_path, groups);
        }
        else
            exit_code = ErrorCode::invalid_xml_formatting;
    }
    else
        exit_code = ErrorCode::io_error;

    return exit_code;
}


/**
 * group_level1
 * group_level1
 *      group_level2
 *          group_level3
 *      group_level2
 *
 */