#include "display.hpp"

using std::vector;
using std::string;
using std::set;

static vector<Group> groups_to_draw {};
static VBO* vbo_wrapper { NULL };

static bool fill { false };
static bool show_axis { false };
static GLubyte rgb[3] { 0, 255, 255};
static GLubyte color_delta { 3 };



static double fov {}, near_ {}, far_ {};
static CartPoint3d position;
static CartPoint3d look_at;
static CartPoint3d up;

static CartPoint3d direction;
static bool first_person { false };
static bool first_mouse { true };
static int last_x;
static int last_y;

static float yaw { 0.f };
static float pitch { 0.f };

static const float sensitivity { 0.5f };
static const float speed { 0.1f };



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
    gluPerspective(fov, ratio, near_, far_);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}


static void render_scene(){

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(position.x, position.y, position.z,
              look_at.x, look_at.y, look_at.z,
              up.x, up.y, up.z);

    // put drawing instructions here
    if(show_axis){

        glBegin(GL_LINES);

            // X axis in red
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(-100.0f, 0.0f, 0.0f);
            glVertex3f( 100.0f, 0.0f, 0.0f);

            // Y Axis in Green
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.0f, -100.0f, 0.0f);
            glVertex3f(0.0f, 100.0f, 0.0f);

            // Z Axis in Blue
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3f(0.0f, 0.0f, -100.0f);
            glVertex3f(0.0f, 0.0f, 100.0f);

        glEnd();
    }

    glColor3ub(rgb[0], rgb[1], rgb[2]);

    if(fill)
        glPolygonMode(GL_FRONT, GL_FILL);
    else
        glPolygonMode(GL_FRONT, GL_LINE);

    unsigned int curr_nest_level { 0 };

    for(auto const& [transforms, models, nest_level] : groups_to_draw){

        /**
         * After popping something, curr_nest_level < g->nest_level (second nested loop condition).
         * So, we needed to come back to the first loop, store the matrix that we just popped
         * and increment curr_nest_level so that it actually has the correct value.
         */

        bool has_popped {};

        do{
            has_popped = false;
            if(curr_nest_level < nest_level)
                while(curr_nest_level < nest_level){
                    glPushMatrix();
                    ++curr_nest_level;
                }
            else
                while(curr_nest_level >= nest_level){
                    has_popped = true;
                    glPopMatrix();
                    --curr_nest_level;
                }
        }
        while(has_popped);


        for(auto const& [point, angle, type] : transforms){

            switch(type){

            case TransformType::rotate:
                glRotated(angle.value(), point.x, point.y, point.z);
                break;

            case TransformType::scale:
                glScaled(point.x, point.y, point.z);
                break;

            case TransformType::translate:
                glTranslated(point.x, point.y, point.z);
                break;

            default:
                break;
            }
        }


        for(auto const& m : models)
            vbo_wrapper->render(m);
    }

    while(curr_nest_level > 0){
        glPopMatrix();
        --curr_nest_level;
    }

    // End of frame
    glutSwapBuffers();
}

// function to process special keyboard events
static void special_keys_event(int key_code, int, int){

    if(!first_person){

        auto&& [px, py, pz] = position;
        double radius { std::sqrt(pz * pz + px * px + py * py) };
        bool nothing { false };

        switch (key_code){

        case GLUT_KEY_LEFT:
            yaw -= 1.0f;
            break;

        case GLUT_KEY_RIGHT:
            yaw += 1.0f;
            break;

        case GLUT_KEY_DOWN:
            pitch -= 1.0f;
            break;

        case GLUT_KEY_UP:
            pitch += 1.0f;
            break;

        case GLUT_KEY_PAGE_DOWN:
            radius -= 1.0f;
            nothing = true;
            break;

        case GLUT_KEY_PAGE_UP:
            radius += 1.0f;
            nothing = true;
            break;

        default:
            nothing = true;
            break;
        }

        if(!nothing){
            if(pitch > 89.0f)
                pitch = 89.0f;
            if(pitch < -89.0f)
                pitch = -89.0f;

            position.x = radius * std::sin(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
            position.y = radius * std::sin(degree_to_radian(pitch));
            position.z = radius * std::cos(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
        }

    }
    else{

        bool nothing { false };

        switch (key_code){

        case GLUT_KEY_LEFT:
            position.x += speed * direction.z;
            position.y += speed * direction.y;
            position.z -= speed * direction.x;
            break;

        case GLUT_KEY_RIGHT :
            position.x -= speed * direction.z;
            position.y += speed * direction.y;
            position.z += speed * direction.x;
            break;

        case GLUT_KEY_DOWN:
            position.x -= speed * direction.x;
            position.y -= speed * direction.y;
            position.z -= speed * direction.z;
            break;

        case GLUT_KEY_UP:
            position.x += speed * direction.x;
            position.y += speed * direction.y;
            position.z += speed * direction.z;
            break;

        default:
            nothing = true;
            break;
        }

        if(!nothing){
            look_at.x = position.x + direction.x;
            look_at.y = position.y + direction.y;
            look_at.z = position.z + direction.z;
        }
    }

    glutPostRedisplay();
}

// function to process normal keyboard events
static void keys_event(unsigned char key, int, int){

    switch(key){

    case 'p':
        if(first_person) {
            look_at.x = 0.0f;
            look_at.y = 0.0f;
            look_at.z = 0.0f;

            const double dz { position.z - look_at.z };
            const double dy { position.y - look_at.y };
            const double dx { position.x - look_at.x };

            yaw = std::atan2(dz, dx);
            pitch = std::atan2(std::sqrt(dz * dz + dx * dx), dy) + PI;

            direction.x = std::sin(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
            direction.y = std::sin(degree_to_radian(pitch));
            direction.z = std::cos(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
        }
        first_person = !first_person;
        break;

    case 'f':
        fill = !fill;
        break;

    case '1':
        if(rgb[0] <= 255 - color_delta)
            rgb[0] += color_delta;
        break;

    case '2':
        if(rgb[0] >= color_delta)
            rgb[0] -= color_delta;
        break;

    case '3':
        if(rgb[1] <= 255 - color_delta)
            rgb[1] += color_delta;
        break;

    case '4':
        if(rgb[1] >= color_delta)
            rgb[1] -= color_delta;
        break;

    case '5':
        if(rgb[2] <= 255 - color_delta)
            rgb[2] += color_delta;
        break;

    case '6':
        if(rgb[2] >= color_delta)
            rgb[2] -= color_delta;
        break;

    case 'x':
        show_axis = !show_axis;
        break;

    case 'e':
        std::exit(0);
        break;

    default:
        break;
    }

    glutPostRedisplay();
}



// function to process mouse events
static void mouse_event(int x, int y){

    if(first_person){

        if (first_mouse){

            last_x = x;
            last_y = y;
            first_mouse = false;
        }

        float xoffset { static_cast<float>(x - last_x) };
        float yoffset { static_cast<float>(y - last_y) };
        last_x = x;
        last_y = y;

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw   -= xoffset;
        pitch -= yoffset;

        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;

        direction.x = std::sin(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
        direction.y = std::sin(degree_to_radian(pitch));
        direction.z = std::cos(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));

        look_at.x = position.x + direction.x;
        look_at.y = position.y + direction.y;
        look_at.z = position.z + direction.z;
    }

    glutPostRedisplay();
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

    for(auto const& [_, models_vector, __] : groups_to_draw)
        for(auto const& elem : models_vector)
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

    ErrorCode exit_code { ErrorCode::success };

    if(argc > 1){

        CameraSettings cs {};
        const string filename { argv[1] };

        exit_code = xml_parser(filename, cs, groups_to_draw); //groups_to_draw is global

        if(exit_code == ErrorCode::success){

            fov = cs.fov;
            far_ = cs.far_;
            near_ = cs.near_;
            position = cs.position;
            look_at = cs.look_at;
            up = cs.up;



            const double dx { position.x - look_at.x };
            const double dy { position.y - look_at.y };
            const double dz { position.z - look_at.z };

            yaw = std::atan2(dz, dx);
            pitch = std::atan2(std::sqrt(dz * dz + dx * dx), dy) + PI;

            direction.x = std::sin(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
            direction.y = std::sin(degree_to_radian(pitch));
            direction.z = std::cos(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));

            glut_start(argc, argv);
        }
    }
    else
        exit_code = ErrorCode::not_enough_args;


    return exit_code;
}
