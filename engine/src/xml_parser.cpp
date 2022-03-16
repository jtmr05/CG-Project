#include "xml_parser.hpp"

using std::string;
using std::vector;

camera_settings::camera_settings(){
    this->position = {};
    this->look_at = {};
    this->up = {};
    this->fov = {};
    this->near_ = {};
    this->far_ = {};
}


transform::transform(angle_t angle, const CartPoint3d &point){
    this->angle = { angle };
    this->point = { point };
    this->type = TransformType::rotate;
}

transform::transform(TransformType type, const CartPoint3d &point){
    this->angle = {};
    this->point = { point };
    this->type = type;
}



group::group(uint nest_level){
    this->nest_level = nest_level;
    this->models = {};
    this->transforms = {};
}



void parse_camera_settings(CameraSettings &c, TiXmlElement* p_world){

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
}


void parse_groups(TiXmlElement* p_world, const string &dir_prefix, vector<Group> &groups){

    std::stack<TiXmlElement*> group_rollback {};
    uint nest_level {1};

    TiXmlElement* p_group { p_world->FirstChildElement("group") };
    while(p_group){

        Group g { nest_level };

        TiXmlElement* p_transform { p_group->FirstChildElement("transform") };
        if(p_transform){

            TiXmlElement* p_generic_transform { p_transform->FirstChildElement() };
            while(p_generic_transform){

                const char* attribute_name { p_generic_transform->Value() };
                const char* angle { p_generic_transform->Attribute("angle") };

                const double x { string_to_double(p_generic_transform->Attribute("x")) };
                const double y { string_to_double(p_generic_transform->Attribute("y")) };
                const double z { string_to_double(p_generic_transform->Attribute("z")) };

                Transform *pt { NULL };

                if(angle){
                    Transform t { string_to_double(angle), { x, y, z } };
                    pt = &t;
                }
                else if(attribute_name == "translate"){
                    Transform t { TransformType::translate, { x, y, z } };
                    pt = &t;
                }
                else if(attribute_name == "scale"){
                    Transform t { TransformType::scale, { x, y, z } };
                    pt = &t;
                }

                if(pt)
                    g.transforms.push_back(*pt); //potential seg fault because 't' is out of scope

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

        TiXmlElement* parent = p_group;
        p_group = parent->FirstChildElement("group");

        if(p_group){
            group_rollback.push(parent);
            nest_level++;
        }
        else{
            p_group = p_group->NextSiblingElement("group");

            if(!p_group && !group_rollback.empty()){
                p_group = group_rollback.top();
                group_rollback.pop();
                nest_level--;
                p_group = p_group->NextSiblingElement("group");
            }
        }
    }
}

ErrorCode xml_parser(const string &path, CameraSettings &c, vector<Group> &groups){

    ErrorCode exit_code { ErrorCode::success };
    TiXmlDocument doc {};

    // Load the XML file into the Doc instance
    if(has_xml_ext(path) && doc.LoadFile(path.c_str())){

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
            parse_camera_settings(c, p_world);
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