#include "display.hpp"

using std::vector;
using std::string;

static vector<CartPoint3d> points_to_draw {};
static double offset_y {};
static angle_t zOp {};

static bool fill { false };
static bool show_axis { false };

static GLubyte rgb[3] { 0, 255, 255};
static GLubyte color_delta { 3 };

static double fov {}, near_ {}, far_ {};
static CartPoint3d position;
static CartPoint3d look_at;
static CartPoint3d up;


void draw_triangle(const CartPoint3d &p1, const CartPoint3d &p2, const CartPoint3d &p3){

    if(fill){

        glBegin(GL_TRIANGLES);

        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);
        glVertex3f(p3.x, p3.y, p3.z);
    }
    else{

        glBegin(GL_LINES);

        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);

        glVertex3f(p2.x, p2.y, p2.z);
        glVertex3f(p3.x, p3.y, p3.z);

        glVertex3f(p3.x, p3.y, p3.z);
        glVertex3f(p1.x, p1.y, p1.z);
    }

    glEnd();
}

void change_size(int w, int h){

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if(h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = static_cast<float>(w) / static_cast<float>(h);

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


void render_scene(){

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    double radius { std::sqrt(position.z * position.z + position.x * position.x) };
    gluLookAt(radius * std::sin(zOp), position.y + offset_y, radius * std::cos(zOp),
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

    for(auto i { points_to_draw.begin() }; i != points_to_draw.end(); i += 3)
        draw_triangle(i[0], i[1], i[2]);


    // End of frame
    glutSwapBuffers();
}

// function to process special keyboard events
void special_keys_event(int key_code, int x, int y){

    switch (key_code){

    case GLUT_KEY_LEFT:
        zOp -= 0.01;
        break;

    case GLUT_KEY_RIGHT :
        zOp += 0.01;
        break;

    case GLUT_KEY_DOWN:
        offset_y -= 0.1;
        break;

    case GLUT_KEY_UP:
        offset_y += 0.1;
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

// function to process normal keyboard events
void keys_event(unsigned char key, int x, int y){

    switch(key){

    case 'w':
        offset_y -= 0.1f;
        break;

    case 's':
        offset_y += 0.1f;
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

    default:
        break;

    }

    glutPostRedisplay();
}



void glut_start(int argc, char** argv){

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG Phase 1");


    // Required callback registry
    glutDisplayFunc(render_scene);
    glutReshapeFunc(change_size);

    // put here the registration of the keyboard callbacks
    glutKeyboardFunc(keys_event);
    glutSpecialFunc(special_keys_event);


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
        vector<string> files_3d {};
        const string filename { argv[1] };

        exit_code = xml_parser(filename, cs, files_3d);

        if(exit_code == ErrorCode::success){

            fov = cs.fov;
            far_ = cs.far_;
            near_ = cs.near_;
            position = cs.position;
            look_at = cs.look_at;
            up = cs.up;
            zOp = std::atan2(cs.position.x, cs.position.z);

            for(auto i { files_3d.begin() }; i != files_3d.end(); ++i){

                std::ifstream file{};
                file.open(*i, std::ios::in);

                if(file.is_open()){

                    CartPoint3d p1{}, p2{}, p3{};
                    while(file >> p1 >> p2 >> p3){
                        points_to_draw.push_back(p1);
                        points_to_draw.push_back(p2);
                        points_to_draw.push_back(p3);
                    }

                    file.close();
                }
            }

            glut_start(argc, argv);
        }
    }
    else
        exit_code = ErrorCode::not_enough_args;


    return exit_code;
}
