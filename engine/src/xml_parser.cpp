#include "xml_parser.hpp"

using std::string;
using std::vector;
using std::unique_ptr;



static CameraSettings parse_camera_settings(TiXmlElement* const world_tag){

    CameraSettings c {};

    TiXmlElement* const camera_tag { world_tag->FirstChildElement("camera") };
    if (camera_tag){

        TiXmlElement* position_tag { camera_tag->FirstChildElement("position") };
        if(position_tag){
            const double x { string_to_double(position_tag->Attribute("x")) };
            const double y { string_to_double(position_tag->Attribute("y")) };
            const double z { string_to_double(position_tag->Attribute("z")) };
            c.position = { x, y, z };
        }

        TiXmlElement* look_at_tag { camera_tag->FirstChildElement("lookAt") };
        if(look_at_tag){
            const double x { string_to_double(look_at_tag->Attribute("x")) };
            const double y { string_to_double(look_at_tag->Attribute("y")) };
            const double z { string_to_double(look_at_tag->Attribute("z")) };
            c.look_at = { x, y, z };
        }

        TiXmlElement* up_tag { camera_tag->FirstChildElement("up") };
        if(up_tag){
            const double x { string_to_double(up_tag->Attribute("x")) };
            const double y { string_to_double(up_tag->Attribute("y")) };
            const double z { string_to_double(up_tag->Attribute("z")) };
            c.up = { x, y, z };
        }

        TiXmlElement* projection_tag { camera_tag->FirstChildElement("projection") };
        if(projection_tag){
            const double fov  { string_to_double(projection_tag->Attribute("fov")) };
            const double near { string_to_double(projection_tag->Attribute("near")) };
            const double far  { string_to_double(projection_tag->Attribute("far")) };
            c.fov = fov;
            c.near = near;
            c.far = far;
        }
    }

    return c;
}

static inline bool parse_dynamic_translate(TiXmlElement* const generic_transform_tag,
                                           unique_ptr<vector<CartPoint3d>> &points){

    TiXmlElement* point_tag { generic_transform_tag->FirstChildElement("point") };
    while(point_tag){

        const double x { string_to_double(point_tag->Attribute("x")) };
        const double y { string_to_double(point_tag->Attribute("y")) };
        const double z { string_to_double(point_tag->Attribute("z")) };

        points->push_back( { x, y, z } );

        point_tag = point_tag->NextSiblingElement("point");
    }

    return points->size() >= 4;
}

static void parse_groups(TiXmlElement* const world_tag,
                         const string &dir,
                         vector<unique_ptr<Group>> &groups){

    const string rotate_s { "rotate" };
    const string translate_s { "translate" };
    const string scale_s { "scale" };

    std::stack<TiXmlElement*> group_rollback {};
    unsigned nest_level { 1 };

    TiXmlElement* group_tag { world_tag->FirstChildElement("group") };
    while(group_tag){

        assert(nest_level > 0);

        Group* g { new Group{ nest_level } };

        TiXmlElement* const transform_tag { group_tag->FirstChildElement("transform") };
        if(transform_tag){

            TiXmlElement* generic_transform_tag { transform_tag->FirstChildElement() };
            while(generic_transform_tag){

                const string tag_name { generic_transform_tag->Value() };

                const char* angle { generic_transform_tag->Attribute("angle") };
                const char* time { generic_transform_tag->Attribute("time") };
                const char* align { generic_transform_tag->Attribute("align") };



                if(tag_name == rotate_s){

                    const double x { string_to_double(generic_transform_tag->Attribute("x")) };
                    const double y { string_to_double(generic_transform_tag->Attribute("y")) };
                    const double z { string_to_double(generic_transform_tag->Attribute("z")) };

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
                else if(tag_name == translate_s){
                    if(time){
                        unique_ptr<vector<CartPoint3d>> points { new vector<CartPoint3d>() };

                        if(parse_dynamic_translate(generic_transform_tag, points)){

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

                        const double x { string_to_double(generic_transform_tag->Attribute("x")) };
                        const double y { string_to_double(generic_transform_tag->Attribute("y")) };
                        const double z { string_to_double(generic_transform_tag->Attribute("z")) };

                        g->transforms.push_back(
                            unique_ptr<Transform>{
                                new StaticTranslate { { x, y, z } }
                            }
                        );
                    }
                }
                else if(tag_name == scale_s){

                    const double x { string_to_double(generic_transform_tag->Attribute("x")) };
                    const double y { string_to_double(generic_transform_tag->Attribute("y")) };
                    const double z { string_to_double(generic_transform_tag->Attribute("z")) };

                    g->transforms.push_back(
                        unique_ptr<Transform>{
                            new Scale { { x, y, z } }
                        }
                    );
                }


                generic_transform_tag = generic_transform_tag->NextSiblingElement();
            }
        }

        TiXmlElement* const models_tag { group_tag->FirstChildElement("models") };
        if(models_tag){

            TiXmlElement* model_tag { models_tag->FirstChildElement("model") };
            while(model_tag){

                const string model_fn { model_tag->Attribute("file") };

                TiXmlElement* const texture_tag { model_tag->FirstChildElement("texture") };
                const char* texture_fn { (texture_tag ? texture_tag->Attribute("file") : nullptr) };

                Color c {};
                TiXmlElement* const color_tag { model_tag->FirstChildElement("color") };
                if(color_tag){

                    TiXmlElement* const diffuse_tag { color_tag->FirstChildElement("diffuse") };
                    TiXmlElement* const ambient_tag { color_tag->FirstChildElement("ambient") };
                    TiXmlElement* const specular_tag { color_tag->FirstChildElement("specular") };
                    TiXmlElement* const emissive_tag { color_tag->FirstChildElement("emissive") };
                    TiXmlElement* const shininess_tag { color_tag->FirstChildElement("shininess") };

                    c.diffuse = {
                        static_cast<uint8_t>(string_to_uint(diffuse_tag->Attribute("R"))),
                        static_cast<uint8_t>(string_to_uint(diffuse_tag->Attribute("G"))),
                        static_cast<uint8_t>(string_to_uint(diffuse_tag->Attribute("B"))),
                    };

                    c.ambient = {
                        static_cast<uint8_t>(string_to_uint(ambient_tag->Attribute("R"))),
                        static_cast<uint8_t>(string_to_uint(ambient_tag->Attribute("G"))),
                        static_cast<uint8_t>(string_to_uint(ambient_tag->Attribute("B"))),
                    };

                    c.specular = {
                        static_cast<uint8_t>(string_to_uint(specular_tag->Attribute("R"))),
                        static_cast<uint8_t>(string_to_uint(specular_tag->Attribute("G"))),
                        static_cast<uint8_t>(string_to_uint(specular_tag->Attribute("B"))),
                    };

                    c.emissive = {
                        static_cast<uint8_t>(string_to_uint(emissive_tag->Attribute("R"))),
                        static_cast<uint8_t>(string_to_uint(emissive_tag->Attribute("G"))),
                        static_cast<uint8_t>(string_to_uint(emissive_tag->Attribute("B"))),
                    };

                    c.shininess = {
                        static_cast<unsigned>(string_to_uint(shininess_tag->Attribute("value")))
                    };
                }


                if(has_3d_ext(model_fn)){
                    if(texture_fn)
                        g->models.push_back(
                            Model{
                                dir + model_fn,
                                texture_fn,
                                c
                            }
                        );
                    else
                        g->models.push_back(
                            Model{
                                dir + model_fn,
                                c
                            }
                        );
                }

                model_tag = model_tag->NextSiblingElement("model");
            }
        }

        groups.push_back(unique_ptr<Group>{ g });

        TiXmlElement* const parent { group_tag };
        group_tag = group_tag->FirstChildElement("group");

        if(group_tag){
            group_rollback.push(parent);
            ++nest_level;
        }
        else{
            group_tag = parent->NextSiblingElement("group");

            if(!group_tag && !group_rollback.empty()){
                group_tag = group_rollback.top()->NextSiblingElement("group");
                group_rollback.pop();
                --nest_level;
            }
        }
    }
}

static void parse_lights(TiXmlElement* const world_tag, vector<unique_ptr<Light>> &lights){

    const string point_s { "point" };
    const string directional_s { "directional" };
    const string spotlight_s { "spotlight" };

    TiXmlElement* const lights_tag { world_tag->FirstChildElement("lights") };
    if(lights_tag){

        TiXmlElement* light_tag { lights_tag->FirstChildElement() };
        while(light_tag){

            const string typeof_light { light_tag->Attribute("type") };
            CartPoint3d pos {};
            CartPoint3d dir {};

            if(typeof_light == point_s || typeof_light == spotlight_s){

                const double x { string_to_double(light_tag->Attribute("posx")) };
                const double y { string_to_double(light_tag->Attribute("posy")) };
                const double z { string_to_double(light_tag->Attribute("posz")) };

                pos = { x, y, z };
            }

            if(typeof_light == directional_s || typeof_light == spotlight_s){

                const double x { string_to_double(light_tag->Attribute("dirx")) };
                const double y { string_to_double(light_tag->Attribute("diry")) };
                const double z { string_to_double(light_tag->Attribute("dirz")) };

                dir = { x, y, z };
            }

            if(typeof_light == spotlight_s){

                const int cutoff {
                    string_to_uint(light_tag->Attribute("cutoff"))
                };

                lights.push_back(
                    unique_ptr<Light>{
                        new Spotlight { pos, dir, cutoff }
                    }
                );
            }

            else if(typeof_light == point_s)
                lights.push_back(
                    unique_ptr<Light>{
                        new PointLight { pos }
                    }
                );

            else if(typeof_light == directional_s)
                lights.push_back(
                    unique_ptr<Light>{
                        new DirectionalLight { dir }
                    }
                );


            light_tag = light_tag->NextSiblingElement("light");
        }
    }
}

ErrorCode xml_parser(const string &xml_path, CameraSettings &c,
                     vector<unique_ptr<Group>> &groups,
                     vector<unique_ptr<Light>> &lights){

    if(!has_xml_ext(xml_path))
        return ErrorCode::invalid_file_extension;

    TiXmlDocument doc {};

    // Load the XML file into the Doc instance
    if(!doc.LoadFile(xml_path.c_str(), TIXML_ENCODING_UTF8))
        return ErrorCode::io_error;


    /**
     * 3d files pathes specified in the XML should be relative to the XML itself
     * Therefore we need to yield the directory path
     */
    const size_t last_slash_pos { xml_path.find_last_of('/') };
    const string directory_path {
        (last_slash_pos < string::npos)
            ? std::move(xml_path.substr(0, last_slash_pos + 1))
            : ""
    };

    // Get root Element
    TiXmlElement* const world_tag { doc.RootElement() };
    if(world_tag == nullptr)
        return ErrorCode::invalid_file_formatting;

    c = parse_camera_settings(world_tag);
    parse_groups(world_tag, directory_path, groups);
    parse_lights(world_tag, lights);


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