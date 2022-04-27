#include "xml_parser.hpp"

using std::string;
using std::vector;
using std::unique_ptr;



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

    return points->size() >= 4;
}

static void parse_groups(TiXmlElement* p_world, const string &dir, vector<unique_ptr<Group>> &groups){

    std::stack<TiXmlElement*> group_rollback {};
    unsigned int nest_level { 1 };

    TiXmlElement* p_group { p_world->FirstChildElement("group") };
    while(p_group){

        assert(nest_level > 0);

        Group* g { new Group{ nest_level } };

        TiXmlElement* p_transform { p_group->FirstChildElement("transform") };
        if(p_transform){

            TiXmlElement* p_generic_transform { p_transform->FirstChildElement() };
            while(p_generic_transform){

                const string attribute_name { p_generic_transform->Value() };

                const char* angle { p_generic_transform->Attribute("angle") };
                const char* time { p_generic_transform->Attribute("time") };
                const char* align { p_generic_transform->Attribute("align") };



                if(attribute_name == "rotate"){

                    const double x { string_to_double(p_generic_transform->Attribute("x")) };
                    const double y { string_to_double(p_generic_transform->Attribute("y")) };
                    const double z { string_to_double(p_generic_transform->Attribute("z")) };

                    if(angle)
                        g->transforms.push_back(
                            unique_ptr<Transform>{
                                new StaticRotate { string_to_double(angle), { x, y, z } }
                            }
                        );

                    else if(time){

                        int const itime { string_to_uint(time) };

                        if(itime > 0)
                            g->transforms.push_back(
                                unique_ptr<Transform>{
                                    new DynamicRotate {
                                        static_cast<unsigned>(itime),
                                        { x, y, z }
                                    }
                                }
                            );
                    }

                }
                else if(attribute_name == "translate"){
                    if(time){
                        unique_ptr<vector<CartPoint3d>> points { new vector<CartPoint3d>() };

                        if(parse_dynamic_translate(p_generic_transform, points)){

                            assert(points->size() >= 4);

                            int const itime { string_to_uint(time) };

                            if(itime > 0)
                                g->transforms.push_back(
                                    unique_ptr<Transform>{
                                        new DynamicTranslate {
                                            static_cast<unsigned>(itime),
                                            string_to_bool(align),
                                            points
                                        }
                                    }
                                );
                        }
                    }
                    else{

                        const double x { string_to_double(p_generic_transform->Attribute("x")) };
                        const double y { string_to_double(p_generic_transform->Attribute("y")) };
                        const double z { string_to_double(p_generic_transform->Attribute("z")) };

                        g->transforms.push_back(
                            unique_ptr<Transform>{
                                new StaticTranslate { { x, y, z } }
                            }
                        );
                    }
                }
                else if(attribute_name == "scale"){

                    const double x { string_to_double(p_generic_transform->Attribute("x")) };
                    const double y { string_to_double(p_generic_transform->Attribute("y")) };
                    const double z { string_to_double(p_generic_transform->Attribute("z")) };

                    g->transforms.push_back(
                        unique_ptr<Transform>{
                            new Scale { { x, y, z } }
                        }
                    );
                }


                p_generic_transform = p_generic_transform->NextSiblingElement();
            }
        }

        TiXmlElement* p_models { p_group->FirstChildElement("models") };
        if(p_models){

            TiXmlElement* p_model { p_models->FirstChildElement("model") };
            while(p_model){

                const string fn { p_model->Attribute("file") };
                if(has_3d_ext(fn))
                    g->models.push_back(dir + fn);

                p_model = p_model->NextSiblingElement("model");
            }
        }

        groups.push_back(unique_ptr<Group>{g});

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

ErrorCode xml_parser(const string &path, CameraSettings &c, vector<unique_ptr<Group>> &groups){

    TiXmlDocument doc {};

    // Load the XML file into the Doc instance
    if(!has_xml_ext(path))
        return ErrorCode::invalid_file_extension;

    if(!doc.LoadFile(path.c_str()))
        return ErrorCode::io_error;



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
    if(p_world == nullptr)
        return ErrorCode::invalid_file_formatting;

    c = parse_camera_settings(p_world);
    parse_groups(p_world, directory_path, groups);

    return ErrorCode::success;
}


/**
 * group_level1
 * group_level1
 *      group_level2
 *          group_level3
 *      group_level2
 *
 */