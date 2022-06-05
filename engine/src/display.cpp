#include "display.hpp"

using std::vector;
using std::string;
using std::set;
using std::unique_ptr;
using std::shared_ptr;
using std::array;
using std::map;
using std::pair;
using std::optional;
using std::tuple;


CameraSettings cs {};

static Constant<vector<unique_ptr<Group>>> groups {};

static Constant<bool> as_vbo {};
static Constant<shared_ptr<VBO>> vbo_wrapper {};

static Constant<shared_ptr<TexturesHandler>> textures_wrapper {};

static Constant<int> tesselation {};

static Constant<map<string, vector<CartPoint3d>>> points_to_draw {};
static Constant<map<string, vector<CartPoint3d>>> normals_to_draw {};
static Constant<map<string, vector<CartPoint2d>>> text_coords_to_draw {};


static Constant<vector<unique_ptr<Light>>> lights {};
static constexpr array<unsigned, 8> gl_lights {
    GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
    GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7
};

static const string TITLE { "CG Phase 4" };



static void change_size(int w, int h){

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if(h == 0)
        h = 1;

    // compute window's aspect ratio
    const float ratio { static_cast<float>(w) / static_cast<float>(h) };

    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);

    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(cs.fov, ratio, cs.near, cs.far);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

static array<double, 16> build_rotation_matrix(const array<double, 3> &x,
                                               const array<double, 3> &y,
                                               const array<double, 3> &z){

    array<double, 16> m {};

    m[0]  = x[0]; m[1]  = x[1]; m[2]  = x[2]; m[3]  = 0;
    m[4]  = y[0]; m[5]  = y[1]; m[6]  = y[2]; m[7]  = 0;
    m[8]  = z[0]; m[9]  = z[1]; m[10] = z[2]; m[11] = 0;
    m[12] = 0;    m[13] = 0;    m[14] = 0;    m[15] = 1;

    return m;
}

static array<double, 3> cross(const array<double, 3> &a, const array<double, 3> &b){

    array<double, 3> res {};

    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];

    return res;
}

static void normalize(array<double, 3> &a) {

    const double norm { std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) };
    a[0] /= norm;
    a[1] /= norm;
    a[2] /= norm;
}

static
tuple<Matrix<double, 1, 3>, Matrix<double, 1, 3>>
get_catmull_rom_point(double t, const array<double, 3> &p0, const array<double, 3> &p1,
                                const array<double, 3> &p2, const array<double, 3> &p3){

    // catmull-rom matrix
    const Matrix<double, 4, 4> m {
        array{ -0.5,  1.5, -1.5,  0.5 },
        array{  1.0, -2.5,  2.0, -0.5 },
        array{ -0.5,  0.0,  0.5,  0.0 },
        array{  0.0,  1.0,  0.0,  0.0 }
    };

    const Matrix<double, 4, 3> p {
        array{ p0[0], p0[1], p0[2] },
        array{ p1[0], p1[1], p1[2] },
        array{ p2[0], p2[1], p2[2] },
        array{ p3[0], p3[1], p3[2] }
    };

    // Compute A = M * P
    // M is 4x4, P is 4x3, therefore A is 4x3
    const Matrix<double, 4, 3> a { m * p };



    const Matrix<double, 1, 4> t_vector {
        array{ t * t * t, t * t, t, 1.0 }
    };

    const Matrix<double, 1, 4> t_vector_deriv {
        array{ 3.0 * t * t, 2.0 * t, 1.0, 0.0 }
    };

    // Compute pos = T * A and
    // Compute deriv = T' * A
    return { t_vector * a, t_vector_deriv * a };
}

static
tuple<Matrix<double, 1, 3>, Matrix<double, 1, 3>>
get_global_catmull_rom_point(const vector<CartPoint3d> &points, double gt){

    const size_t point_count { points.size() };

    double t { gt * static_cast<double>(point_count) }; // this is the real global t
    const unsigned index { static_cast<unsigned>(std::floor(t)) };  // which segment
    t -= static_cast<double>(index); // where within the segment

    // indices store the points
    array<unsigned, 4> indices {};
    indices[0] = index % point_count;
    indices[1] = (indices[0] + 1) % point_count;
    indices[2] = (indices[1] + 1) % point_count;
    indices[3] = (indices[2] + 1) % point_count;


    return get_catmull_rom_point(
        t,
        points[indices[0]].as_array(),
        points[indices[1]].as_array(),
        points[indices[2]].as_array(),
        points[indices[3]].as_array()
    );
}

static void render_catmull_rom_curve(const vector<CartPoint3d> &points){

    const double step { 1.0 / static_cast<double>(tesselation.value()) };

    glBegin(GL_LINE_LOOP);

        for(double gt {}; gt < 1.0; gt += step){

            auto&& [pos, _] { get_global_catmull_rom_point(points, gt) };
            glVertex3d(pos[0][0], pos[0][1], pos[0][2]);
        }

    glEnd();
}

#ifdef BENCH
#include <algorithm>
#include <numeric>
#endif

static inline void compute_fps(){

    static unsigned frames { 0 };
    ++frames;

    static int begin_millis { glutGet(GLUT_ELAPSED_TIME) };
    int const current_millis { glutGet(GLUT_ELAPSED_TIME) };
    int const diff_millis { current_millis - begin_millis };

#ifdef BENCH
    static array<double, 15> fps_values {};
    static auto fps_values_iter { fps_values.begin() };
#endif

    if(diff_millis > 1000){

        double const fps {
            static_cast<double>(frames * 1000) /
            static_cast<double>(diff_millis)
        };

        std::stringstream string_buffer {};
        string_buffer << TITLE
                      << " --- FPS: "
                      << fps;

        glutSetWindowTitle(string_buffer.str().c_str());

        frames = 0;
        begin_millis = current_millis;

#ifdef BENCH
        if(fps_values_iter != fps_values.end())
            *fps_values_iter++ = fps;
        else {

            double const max { *(std::max_element(fps_values.begin(), fps_values.end())) };
            double const min { *(std::min_element(fps_values.begin(), fps_values.end())) };
            double const avg {
                std::accumulate(fps_values.begin(), fps_values.end(), 0.0) /
                static_cast<double>(fps_values.size())
            };

            std::cout << "\tmax: " << max
                      << " | min: " << min
                      << " | average[" << fps_values.size() << "]: " << avg
                      << " | VBOs enabled: "
                      << (as_vbo.value() ? "yes" : "no") << '\n';
            glutLeaveMainLoop();
	    }
#endif
    }
}

static inline void set_lights(){

    for(unsigned i{}; i < lights.value().size(); ++i){

        auto const& light { lights.value()[i] };

        switch (light->get_type()){

        case LightType::spotlight: {

            Spotlight const *sl { dynamic_cast<Spotlight*>(light.get()) };
            assert(sl != nullptr);


            const array<float, 4> pos {
                static_cast<float>(sl->pos.x),
                static_cast<float>(sl->pos.y),
                static_cast<float>(sl->pos.z),
                1.f
            };
            glLightfv(gl_lights[i], GL_POSITION, pos.data());

            const array<float, 3> dir {
                static_cast<float>(sl->dir.x),
                static_cast<float>(sl->dir.y),
                static_cast<float>(sl->dir.z)
            };
            glLightfv(gl_lights[i], GL_SPOT_DIRECTION, dir.data());

            glLighti(gl_lights[i], GL_SPOT_CUTOFF, sl->cutoff);

            break;
        }

        case LightType::point: {

            PointLight const *pl { dynamic_cast<PointLight*>(light.get()) };
            assert(pl != nullptr);


            const array<float, 4> pos {
                static_cast<float>(pl->pos.x),
                static_cast<float>(pl->pos.y),
                static_cast<float>(pl->pos.z),
                1.f
            };
            glLightfv(gl_lights[i], GL_POSITION, pos.data());

            break;
        }

        case LightType::directional: {

            DirectionalLight const *dl { dynamic_cast<DirectionalLight*>(light.get()) };
            assert(dl != nullptr);


            const array<float, 4> dir {
                static_cast<float>(dl->dir.x),
                static_cast<float>(dl->dir.y),
                static_cast<float>(dl->dir.z),
                0.f
            };
            glLightfv(gl_lights[i], GL_POSITION, dir.data());

            break;
        }

        }
    }
}

static inline void set_material_color(const Color& color){

    glMaterialfv(GL_FRONT, GL_DIFFUSE,  color.diffuse.as_float_array().data());
    glMaterialfv(GL_FRONT, GL_AMBIENT,  color.ambient.as_float_array().data());
    glMaterialfv(GL_FRONT, GL_SPECULAR, color.specular.as_float_array().data());
    glMaterialfv(GL_FRONT, GL_EMISSION, color.emissive.as_float_array().data());
    glMateriali(GL_FRONT, GL_SHININESS, static_cast<int>(color.shininess));
}

static void render_scene(){

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(
        cs.position.x, cs.position.y, cs.position.z,
        cs.look_at.x,  cs.look_at.y,  cs.look_at.z,
        cs.up.x,       cs.up.y,       cs.up.z
    );

    set_axis();
    glColor3ub(255, 255, 255);

    const bool lighting_enabled { set_lighting() };
    if(lighting_enabled)
        set_lights();

    set_polygon_mode();


    unsigned curr_nest_level { 0 };

    for(auto const& group : groups.value()){

        /**
         * After popping something, curr_nest_level < g->nest_level (second nested loop condition).
         * So, we needed to come back to the first loop, store the matrix that we just popped
         * and increment curr_nest_level so that it actually has the correct value.
         */

        bool has_popped {};

        do{
            has_popped = false;
            if(curr_nest_level < group->nest_level)
                while(curr_nest_level < group->nest_level){
                    glPushMatrix();
                    ++curr_nest_level;
                }
            else
                while(curr_nest_level >= group->nest_level){
                    has_popped = true;
                    glPopMatrix();
                    --curr_nest_level;
                }
        }
        while(has_popped);


        for(auto const& t : group->transforms)

            switch(t->get_type()){

            case TransformType::static_rotate: {

                StaticRotate const *sr { dynamic_cast<StaticRotate*>(t.get()) };
                assert(sr != nullptr);

                glRotated(sr->angle, sr->point.x, sr->point.y, sr->point.z);
                break;
            }

            case TransformType::scale: {

                Scale const *s { dynamic_cast<Scale*>(t.get()) };
                assert(s != nullptr);

                glScaled(s->point.x, s->point.y, s->point.z);
                break;
            }

            case TransformType::static_translate: {

                StaticTranslate const *st { dynamic_cast<StaticTranslate*>(t.get()) };
                assert(st != nullptr);

                glTranslated(st->point.x, st->point.y, st->point.z);
                break;
            }

            case TransformType::dynamic_rotate: {

                DynamicRotate const *dr { dynamic_cast<DynamicRotate*>(t.get()) };
                assert(dr != nullptr);

                int const current_millis { glutGet(GLUT_ELAPSED_TIME) };

                angle_t const angle_delta { 360.0 / static_cast<angle_t>(dr->time * 1000) };
                angle_t const angle {
                    static_cast<double>(
                        current_millis % static_cast<ssize_t>(dr->time * 1000)
                    )
                    * angle_delta
                };

                glRotated(angle, dr->point.x, dr->point.y, dr->point.z);
                break;
            }

            case TransformType::dynamic_translate: {

                DynamicTranslate const *dt { dynamic_cast<DynamicTranslate*>(t.get()) };
                assert(dt != nullptr);

                render_catmull_rom_curve(*(dt->points));

                double const t {
                    static_cast<double>(glutGet(GLUT_ELAPSED_TIME)) /
                    static_cast<double>(dt->time * 1000)
                };

                const auto& [pos, deriv] { get_global_catmull_rom_point(*(dt->points), t) };

                glTranslated(pos.at(0, 0), pos.at(0, 1), pos.at(0, 2));

                if(dt->align){

                    static array<double, 3> Y { 0.0, 1.0, 0.0 };
                    array<double, 3> X { deriv.at(0, 0), deriv.at(0, 1), deriv.at(0, 2) };
                    array<double, 3> Z { cross(X, Y)};

                    Y = cross(Z, X);

                    normalize(X);
                    normalize(Y);
                    normalize(Z);

                    const array<double, 16> m { build_rotation_matrix(X, Y, Z) };
                    glMultMatrixd(m.data());
                }

                break;
            }

            default:
                break;
            }


        if(as_vbo.value()){

            vbo_wrapper.value()->enable_client_state();

            for(auto const& m : group->models){

                if(lighting_enabled)
                    set_material_color(m.color);

                if(vbo_wrapper.value()->has_texture(m.model_filename))
                    textures_wrapper.value()->bind(m.texture_filename.value_or(""));
                else
                    textures_wrapper.value()->clear();

                vbo_wrapper.value()->render(m.model_filename);
            }

            vbo_wrapper.value()->disable_client_state();
        }
        else

            for(auto const& m : group->models){

                if(lighting_enabled)
                    set_material_color(m.color);

                const string& model_fn { m.model_filename };

                const bool has_vertexes { points_to_draw.value().count(model_fn) > 0 };
                if(has_vertexes){

                    const bool has_normals { normals_to_draw.value().count(model_fn) > 0 };
                    const bool has_text_coords { text_coords_to_draw.value().count(model_fn) > 0 };

                    const vector<CartPoint3d>& vertexes {
                        points_to_draw.value().at(model_fn)
                    };


                    if(has_text_coords)
                        textures_wrapper.value()->bind(m.texture_filename.value_or(""));
                    else
                        textures_wrapper.value()->clear();


                    glBegin(GL_TRIANGLES);

                    for(unsigned i{}; i < vertexes.size(); ++i){

                        if(has_text_coords){

                            const vector<CartPoint2d>& text_coords {
                                text_coords_to_draw.value().at(model_fn)
                            };

                            glTexCoord2d(text_coords.at(i).x, text_coords.at(i).y);
                        }

                        if(has_normals){

                            const vector<CartPoint3d>& normals {
                                normals_to_draw.value().at(model_fn)
                            };

                            glNormal3d(normals.at(i).x, normals.at(i).y, normals.at(i).z);
                        }

                        glVertex3d(vertexes.at(i).x, vertexes.at(i).y, vertexes.at(i).z);
                    }

                    glEnd();
                }
            }
    }

    while(curr_nest_level > 0){
        glPopMatrix();
        --curr_nest_level;
    }

    compute_fps();

    // End of frame
    glutSwapBuffers();
}

static void gl_start(int argc, char** argv){

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
    glutCreateWindow(TITLE.c_str());

    // Required callback registry
    glutDisplayFunc(render_scene);
    glutIdleFunc(render_scene);
    glutReshapeFunc(change_size);

    // put here the registration of the keyboard and mouse callbacks
    glutKeyboardFunc(keys_event);
    glutSpecialFunc(special_keys_event);
    glutPassiveMotionFunc(mouse_event);
    glutMotionFunc(mouse_event);



     set<string> images_set {};

    if(!as_vbo.value()){

        map<string, vector<CartPoint3d>> tmp_points_to_draw {};
        map<string, vector<CartPoint3d>> tmp_normals_to_draw {};
        map<string, vector<CartPoint2d>> tmp_text_coords_to_draw {};

        for(auto const& group : groups.value())

            for(auto const& m : group->models){

                const string& model_fn { m.model_filename };

                //count returns number of mappings for the given key

                if(tmp_points_to_draw.count(model_fn)  == 0 &&
                   tmp_normals_to_draw.count(model_fn) == 0){

                    auto const& [code, vertexes, normals, text_coords] { files_reader(model_fn) };

                    if(code == ErrorCode::success){
                        tmp_points_to_draw.insert(
                            { model_fn, std::move(vertexes) }
                        );

                        if(normals.size() > 0)
                            tmp_normals_to_draw.insert(
                                { model_fn, std::move(normals) }
                            );

                        if(text_coords.size() > 0)
                            tmp_text_coords_to_draw.insert(
                                { model_fn, std::move(text_coords) }
                            );
                    }
                }

                if(m.texture_filename.has_value())
                    images_set.insert(m.texture_filename.value());
            }

        points_to_draw  = std::move(tmp_points_to_draw);
        normals_to_draw = std::move(tmp_normals_to_draw);
        text_coords_to_draw = std::move(tmp_text_coords_to_draw);
    }
    else{

        /**
        * Avoid repeated elements i.e. multiples references
        * to the same .3d file
        */

        set<string> models_set {};

        for(auto const& group : groups.value())

            for(auto const& m : group->models){

                models_set.insert(m.model_filename);

                if(m.texture_filename.has_value())
                    images_set.insert(m.texture_filename.value());
            }

        VBO::init(models_set);
        vbo_wrapper = VBO::get_instance();
    }

    TexturesHandler::init(images_set);
    textures_wrapper = TexturesHandler::get_instance();


    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glEnable(GL_RESCALE_NORMAL);
    glEnable(GL_TEXTURE_2D);



    const array<float, 4> dark  { 0.2f, 0.2f, 0.2f, 1.0f };
    const array<float, 4> white { 1.0f, 1.0f, 1.0f, 1.0f };
    const array<float, 4> black { 0.0f, 0.0f, 0.0f, 0.0f };

    for(unsigned i {}; i < lights.value().size() && i < gl_lights.size(); ++i){

        // enable each light as needed...
        glEnable(gl_lights[i]);

        // ...and set the colors
        glLightfv(gl_lights[i], GL_AMBIENT,  dark.data());
        glLightfv(gl_lights[i], GL_DIFFUSE,  white.data());
        glLightfv(gl_lights[i], GL_SPECULAR, white.data());
    }

    // controls global ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black.data());

    // enter GLUT's main cycle
    glutMainLoop();
}



ErrorCode start(int argc, char** argv){

    if(argc == 1)
        return ErrorCode::not_enough_args;


    const string filename { argv[1] };

    if(argc >= 3){
        if(argv[2][0] == 'n' || argv[2][0] == 'N')
            as_vbo = false;
        else if(argv[2][0] == 'y' || argv[2][0] == 'Y')
            as_vbo = true;
        else
            return ErrorCode::invalid_argument;
    }
    else
        as_vbo = true;

    if(argc >= 4){
        const int aux_tess { string_to_uint(argv[3]) };
        if(aux_tess > 0)
            tesselation = aux_tess;
        else
            return ErrorCode::invalid_argument;
    }
    else
        tesselation = 100;


    auto&& [code, cs_tmp, groups_tmp, lights_tmp] { xml_parser(filename) };

    if(code == ErrorCode::success){
        cs = cs_tmp;
        groups = std::move(groups_tmp);
        lights = std::move(lights_tmp);
        interaction_init(lights.value().size() > 0); //are there lights?
        gl_start(argc, argv);
    }

    return code;
}
