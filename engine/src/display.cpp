#include "display.hpp"

using std::vector;
using std::string;
using std::map;
using std::pair;

static vector<Group> groups_to_draw {};
static map<string, vector<CartPoint3d>> points_to_draw {};
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

    if(fill)
        glPolygonMode(GL_FRONT, GL_FILL);
    else
        glPolygonMode(GL_FRONT, GL_LINE);

    glBegin(GL_TRIANGLES);

        glVertex3d(p1.x, p1.y, p1.z);
        glVertex3d(p2.x, p2.y, p2.z);
        glVertex3d(p3.x, p3.y, p3.z);

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

    const double radius { std::sqrt(position.z * position.z + position.x * position.x) };
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

    uint curr_nest_level { 0 };

    for(auto g { groups_to_draw.begin() }; g != groups_to_draw.end(); ++g){

        /**
         * After popping something, curr_nest_level < g->nest_level (second nested loop condition).
         * So, we needed to come back to the first loop, store the matrix that we just popped
         * and increment curr_nest_level so that it actually has the correct value.
         */

        bool popped { false };

        do{
            popped = false;
            if(curr_nest_level < g->nest_level)
                while(curr_nest_level < g->nest_level){
                    glPushMatrix();
                    curr_nest_level++;
                }
            else
                while(curr_nest_level >= g->nest_level){
                    popped = true;
                    glPopMatrix();
                    curr_nest_level--;
                }
        }
        while(popped);



        const vector<Transform> transforms { g->transforms };
        for(auto t { transforms.begin() }; t != transforms.end(); ++t){

            switch(t->type){

            case TransformType::rotate:
                #ifdef WITH_DEBUG
                std::cout << "Rotate: " << '(' << t->angle.value() << ", " << t->point.x << ", " << t->point.y << ", " << t->point.z << ")\n";
                #endif
                glRotated(t->angle.value(), t->point.x, t->point.y, t->point.z);
                break;

            case TransformType::scale:
                #ifdef WITH_DEBUG
                std::cout << "Scale: " << '(' << t->point.x << ", " << t->point.y << ", " << t->point.z << ")\n";
                #endif
                glScaled(t->point.x, t->point.y, t->point.z);
                break;

            case TransformType::translate:
                #ifdef WITH_DEBUG
                std::cout << "Translate: " << '(' << t->point.x << ", " << t->point.y << ", " << t->point.z << ")\n";
                #endif
                glTranslated(t->point.x, t->point.y, t->point.z);
                break;

            default:
                break;
            }
        }

        const vector<string> models { g->models };
        for(auto m { models.begin() }; m != models.end(); ++m){

            if(points_to_draw.count(*m) > 0){
                const vector<CartPoint3d> points { points_to_draw.at(*m) };
                for(auto p { points.begin() }; p != points.end(); p += 3)
                    draw_triangle(p[0], p[1], p[2]);
            }
        }
    }

    while(curr_nest_level > 0){
        glPopMatrix();
        curr_nest_level--;
    }

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
        const string filename { argv[1] };

        exit_code = xml_parser(filename, cs, groups_to_draw); //groups_to_draw is global

        if(exit_code == ErrorCode::success){

            fov = cs.fov;
            far_ = cs.far_;
            near_ = cs.near_;
            position = cs.position;
            look_at = cs.look_at;
            up = cs.up;
            zOp = std::atan2(cs.position.x, cs.position.z);

            for(auto g { groups_to_draw.begin() }; g != groups_to_draw.end(); ++g){

                const vector<string> models { g->models };
                for(auto m { models.begin() }; m != models.end(); ++m){

                    if(points_to_draw.count(*m) == 0){    //returns number of mappings for this key

                        std::ifstream file{};
                        file.open(*m, std::ios::in);

                        if(file.is_open()){

                            vector<CartPoint3d> value {};
                            CartPoint3d p1{}, p2{}, p3{};

                            while(file >> p1 >> p2 >> p3){
                                value.push_back(p1);
                                value.push_back(p2);
                                value.push_back(p3);
                            }

                            points_to_draw.insert(pair<string, vector<CartPoint3d>>(*m, value));
                            file.close();
                        }
                    }
                }
            }

            glut_start(argc, argv);
        }
    }
    else
        exit_code = ErrorCode::not_enough_args;


    return exit_code;
}
