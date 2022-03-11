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



ErrorCode xml_parser(const string &path, CameraSettings &c, vector<string> &files_3d){

    ErrorCode exit_code { ErrorCode::success };
    TiXmlDocument doc {};

    // Load the XML file into the Doc instance
    if(has_xml_ext(path) && doc.LoadFile(path.c_str())){

        // Get root Element
        TiXmlElement* p_world { doc.RootElement() };
        if(p_world){

            TiXmlElement* p_camera { p_world->FirstChildElement("camera") };
            if (p_camera){

                TiXmlElement* p_position { p_camera->FirstChildElement("position") };
                if(p_position){
                    const double x { static_cast<double>(string_to_int(p_position->Attribute("x"))) };
                    const double y { static_cast<double>(string_to_int(p_position->Attribute("y"))) };
                    const double z { static_cast<double>(string_to_int(p_position->Attribute("z"))) };

                    c.position = { x, y, z };
                }

                TiXmlElement* p_look_at { p_camera->FirstChildElement("lookAt") };
                if(p_look_at){
                    const double x { static_cast<double>(string_to_int(p_look_at->Attribute("x"))) };
                    const double y { static_cast<double>(string_to_int(p_look_at->Attribute("y"))) };
                    const double z { static_cast<double>(string_to_int(p_look_at->Attribute("z"))) };

                    c.look_at = { x, y, z };
                }

                TiXmlElement* p_up { p_camera->FirstChildElement("up") };
                if(p_up){
                    const double x { static_cast<double>(string_to_int(p_up->Attribute("x"))) };
                    const double y { static_cast<double>(string_to_int(p_up->Attribute("y"))) };
                    const double z { static_cast<double>(string_to_int(p_up->Attribute("z"))) };

                    c.up = { x, y, z };
                }

                TiXmlElement* p_projection { p_camera->FirstChildElement("projection") };
                if(p_projection){
                    const double fov { static_cast<double>(string_to_int(p_projection->Attribute("fov"))) };
                    const double near_ { static_cast<double>(string_to_int(p_projection->Attribute("near"))) };
                    const double far_ { static_cast<double>(string_to_int(p_projection->Attribute("far"))) };

                    c.fov = fov;
                    c.near_ = near_;
                    c.far_ = far_;
                }
            }

            TiXmlElement* p_group { p_world->FirstChildElement("group") };
            if(p_group){

                TiXmlElement* p_models { p_group->FirstChildElement("models") };
                if(p_models){

                    TiXmlElement* p_model { p_models->FirstChildElement("model") };
                    while(p_model){

                        const string fn { p_model->Attribute("file") };
                        if(has_3d_ext(fn))
                            files_3d.push_back(fn);

                        p_model = p_model->NextSiblingElement("model");
                    }
                }
            }
        }
        else
            exit_code = ErrorCode::invalid_xml_formatting;
    }
    else
        exit_code = ErrorCode::io_error;

    return exit_code;
}




