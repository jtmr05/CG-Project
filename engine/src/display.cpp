#include "display.hpp"

using std::vector;
using std::string;
using std::set;
using std::unique_ptr;
using std::array;


CameraSettings cs {};

static vector<unique_ptr<Group>> groups_to_draw {};
static VBO* vbo_wrapper { nullptr };

static int const tesselation { 100 };


template<class T, size_t rows, size_t columns>
struct matrix {

    array<array<T, columns>, rows> m;

    constexpr array<T, columns>& operator[](size_t i){
        return m[i];
    }
};


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
    matrix<double, left_size, inner_size> &m1,
    matrix<double, inner_size, right_size> &m2,
    matrix<double, left_size, right_size> &res
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
    matrix<double, 1, 3> &pos,
    matrix<double, 1, 3> &deriv
){

    // catmull-rom matrix
    matrix<double, 4, 4> m {};
    m[0] = {-0.5f,  1.5f, -1.5f,  0.5f};
    m[1] = { 1.0f, -2.5f,  2.0f, -0.5f};
    m[2] = {-0.5f,  0.0f,  0.5f,  0.0f};
    m[3] = { 0.0f,  1.0f,  0.0f,  0.0f};

    matrix<double, 4, 3> p {};
    p[0] = {p0[0], p0[1], p0[2]};
    p[1] = {p1[0], p1[1], p1[2]};
    p[2] = {p2[0], p2[1], p2[2]};
    p[3] = {p3[0], p3[1], p3[2]};

    // Compute A = M * P
    // M is 4x4, P is 4x3, therefore A is 4x3
    matrix<double, 4, 3> a {};
    mult_matrixes(m, p, a);



    // Compute pos = T * A
    matrix<double, 1, 4> t_vector {};
    t_vector[0] = { t * t * t, t * t, t, 1.f };

    mult_matrixes(t_vector, a, pos);



    // compute deriv = T' * A
    matrix<double, 1, 4> t_vector_deriv {};
    t_vector_deriv[0] = { 3.f * t * t, 2.f * t, 1.f, 0.f };

    mult_matrixes(t_vector_deriv, a, deriv);
}

static void get_global_catmull_rom_point(
    const vector<CartPoint3d> &points,
    double gt,
    matrix<double, 1, 3> &pos,
    matrix<double, 1, 3> &deriv
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

    matrix<double, 1, 3> pos {}, deriv {};
    const double step { 1.0 / static_cast<float>(tesselation) };

    glBegin(GL_LINE_LOOP);

        for(double gt {}; gt < 1.0; gt += step){

            get_global_catmull_rom_point(points, gt, pos, deriv);
            glVertex3f(pos[0][0], pos[0][1], pos[0][2]);
        }

    glEnd();
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

                int const current_seconds { glutGet(GLUT_ELAPSED_TIME) / 1000 };

                double const time_delta {
                    static_cast<double>(100) /
                    static_cast<double>(dt->time * 1000)
                };

                double const current_time {
                    static_cast<double>(
                        current_seconds % static_cast<ssize_t>(dt->time * 1000)
                    )
                    * time_delta
                };

                matrix<double, 1, 3> pos {};
                matrix<double, 1, 3> deriv {};
                get_global_catmull_rom_point(*(dt->points), current_time, pos, deriv);
                glTranslated(pos[0][0], pos[0][1], pos[0][2]);

                if(dt->align){
                    static array<double, 3> Y { 0.f, 1.f, 0.f };

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


        for(auto const& m : group->models)
            vbo_wrapper->render(m);
    }

    while(curr_nest_level > 0){
        glPopMatrix();
        --curr_nest_level;
    }

    // End of frame
    glutSwapBuffers();
}



static void glut_start(int argc, char** argv){

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
    glutCreateWindow("CG Phase 3");

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
    set<string> models_set {};

    for(auto const& group : groups_to_draw)
        for(auto const& elem : group->models)
            if(has_3d_ext(elem))
                models_set.insert(elem);

    vbo_wrapper = VBO::get_instance(models_set);

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
    const ErrorCode code { xml_parser(filename, cs, groups_to_draw) }; //groups_to_draw and cs are global

    if(code == ErrorCode::success){
        interaction_init();
        glut_start(argc, argv);
    }

    delete vbo_wrapper;

    return code;
}
