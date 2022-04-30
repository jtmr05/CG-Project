#include "display.hpp"

using std::vector;
using std::string;
using std::set;
using std::unique_ptr;
using std::array;
using std::map;


CameraSettings cs {};

static vector<unique_ptr<Group>> groups_to_draw {};

static Constant<VBO*> vbo_wrapper {};
static Constant<int> tesselation {};
static Constant<map<string, vector<CartPoint3d>>> points_to_draw {};
static Constant<bool> as_vbo {};

static const string TITLE { "CG Phase 3" };



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
    gluPerspective(cs.fov, ratio, cs.near_, cs.far_);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

static void build_rotation_matrix(
    const array<double, 3> &x,
    const array<double, 3> &y,
    const array<double, 3> &z,
    array<double, 16> &m) {

        m[0]  = x[0]; m[1]  = x[1]; m[2]  = x[2]; m[3]  = 0;
        m[4]  = y[0]; m[5]  = y[1]; m[6]  = y[2]; m[7]  = 0;
        m[8]  = z[0]; m[9]  = z[1]; m[10] = z[2]; m[11] = 0;
        m[12] = 0;    m[13] = 0;    m[14] = 0;    m[15] = 1;
}

static void cross(const array<double, 3> &a, const array<double, 3> &b, array<double, 3> &res) {

    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
}

static void normalize(array<double, 3> &a) {

    const double norm { std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) };
    a[0] = a[0] / norm;
    a[1] = a[1] / norm;
    a[2] = a[2] / norm;
}


template<size_t left_size, size_t inner_size, size_t right_size>
static void mult_matrixes(
    Matrix<double, left_size, inner_size> &m1,
    Matrix<double, inner_size, right_size> &m2,
    Matrix<double, left_size, right_size> &res
){

    for(size_t l {}; l < left_size; ++l){
        for(size_t r {}; r < right_size; ++r){

            double value {};

            for(size_t i {}; i < inner_size; ++i)
                value += m1[l][i] * m2[i][r];

            res[l][r] = value;
        }
    }
}

static void get_catmull_rom_point(
    double t,
    const array<double, 3> &p0,
    const array<double, 3> &p1,
    const array<double, 3> &p2,
    const array<double, 3> &p3,
    Matrix<double, 1, 3> &pos,
    Matrix<double, 1, 3> &deriv
){

    // catmull-rom matrix
    Matrix<double, 4, 4> m {};
    m[0] = {-0.5,  1.5, -1.5,  0.5};
    m[1] = { 1.0, -2.5,  2.0, -0.5};
    m[2] = {-0.5,  0.0,  0.5,  0.0};
    m[3] = { 0.0,  1.0,  0.0,  0.0};

    Matrix<double, 4, 3> p {};
    p[0] = {p0[0], p0[1], p0[2]};
    p[1] = {p1[0], p1[1], p1[2]};
    p[2] = {p2[0], p2[1], p2[2]};
    p[3] = {p3[0], p3[1], p3[2]};

    // Compute A = M * P
    // M is 4x4, P is 4x3, therefore A is 4x3
    Matrix<double, 4, 3> a {};
    mult_matrixes(m, p, a);



    // Compute pos = T * A
    Matrix<double, 1, 4> t_vector {};
    t_vector[0] = { t * t * t, t * t, t, 1.0 };

    mult_matrixes(t_vector, a, pos);



    // compute deriv = T' * A
    Matrix<double, 1, 4> t_vector_deriv {};
    t_vector_deriv[0] = { 3.0 * t * t, 2.0 * t, 1.0, 0.0 };

    mult_matrixes(t_vector_deriv, a, deriv);
}

static void get_global_catmull_rom_point(
    const vector<CartPoint3d> &points,
    double gt,
    Matrix<double, 1, 3> &pos,
    Matrix<double, 1, 3> &deriv
){

    size_t const point_count { points.size() };

    double t { gt * static_cast<double>(point_count) }; // this is the real global t
    const unsigned index { static_cast<unsigned>(std::floor(t)) };  // which segment
    t = t - static_cast<double>(index); // where within  the segment

    // indices store the points
    array<unsigned, 4> indices {};
    indices[0] = (index + point_count - 1) % point_count;
    indices[1] = (indices[0] + 1) % point_count;
    indices[2] = (indices[1] + 1) % point_count;
    indices[3] = (indices[2] + 1) % point_count;

    get_catmull_rom_point(
        t,
        points[indices[0]].as_array(),
        points[indices[1]].as_array(),
        points[indices[2]].as_array(),
        points[indices[3]].as_array(),
        pos, deriv
    );
}

static void render_catmull_rom_curve(const vector<CartPoint3d> &points){

    Matrix<double, 1, 3> pos {}, deriv {};
    const double step { 1.0 / static_cast<double>(tesselation.value()) };

    glBegin(GL_LINE_LOOP);

        for(double gt {}; gt < 1.0; gt += step){

            get_global_catmull_rom_point(points, gt, pos, deriv);
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
    static bool once { true };
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
        else if(once){

            once = false;

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

    // put drawing instructions here
    set_axis();
    set_color();
    set_polygon_mode();


    unsigned int curr_nest_level { 0 };

    for(auto const& group : groups_to_draw){

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

        //TODO

        for(const auto &t : group->transforms){

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
                    static_cast<angle_t>(
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

                Matrix<double, 1, 3> pos {};
                Matrix<double, 1, 3> deriv {};
                get_global_catmull_rom_point(*(dt->points), t, pos, deriv);

                glTranslated(pos[0][0], pos[0][1], pos[0][2]);

                if(dt->align){
                    static array<double, 3> Y { 0.0, 1.0, 0.0 };

                    array<double, 3> X { deriv[0] }, Z {};

                    cross(X, Y, Z);
                    cross(Z, X, Y);

                    normalize(X);
                    normalize(Y);
                    normalize(Z);

                    array<double, 16> m {};
                    build_rotation_matrix(X, Y, Z, m);
                    glMultMatrixd(m.data());
                }

                break;
            }

            default:
                break;
            }
        }


        for(auto const& m : group->models){

            if(as_vbo.value())
                vbo_wrapper.value()->render(m);

            else if(points_to_draw.value().count(m) > 0){

                const vector<CartPoint3d> points { points_to_draw.value().at(m) };

                glBegin(GL_TRIANGLES);

                for(auto p { points.begin() }; p != points.end(); p += 3){
                    glVertex3d(p[0].x, p[0].y, p[0].z);
                    glVertex3d(p[1].x, p[1].y, p[1].z);
                    glVertex3d(p[2].x, p[2].y, p[2].z);
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



static void glut_start(int argc, char** argv){

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

    /**
     * Avoid repeated elements i.e. multiples references
     * to the same .3d file
     */

    if(!as_vbo.value()){

        map<string, vector<CartPoint3d>> aux_points_to_draw {};

        for(auto const& group : groups_to_draw)

            for(auto const& m : group->models)

                if(aux_points_to_draw.count(m) == 0){    //returns number of mappings for this key

                    std::ifstream file{};
                    file.open(m, std::ios::in);

                    if(file.is_open()){

                        vector<CartPoint3d> value {};
                        CartPoint3d p1{}, p2{}, p3{};

                        while(file >> p1 >> p2 >> p3){
                            value.push_back(p1);
                            value.push_back(p2);
                            value.push_back(p3);
                        }

                        aux_points_to_draw.insert(std::pair<string, vector<CartPoint3d>>(m, value));
                        file.close();
                    }
                }

        points_to_draw = std::move(aux_points_to_draw);
    }
    else{

        set<string> models_set {};

        for(auto const& group : groups_to_draw)
            for(auto const& m : group->models)
                if(has_3d_ext(m))
                    models_set.insert(m);

        vbo_wrapper = VBO::get_instance(models_set);
    }

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

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



    const ErrorCode code { xml_parser(filename, cs, groups_to_draw) }; //groups_to_draw and cs are global

    if(code == ErrorCode::success){
        interaction_init();
        glut_start(argc, argv);

        if(vbo_wrapper.has_value())
            delete vbo_wrapper.value();
    }

    return code;
}
