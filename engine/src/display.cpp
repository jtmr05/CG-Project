#include "display.hpp"

using std::vector;
using std::string;
using std::map;
using std::pair;

static vector<Group> groups_to_draw {};
static map<string, vector<CartPoint3d>> points_to_draw {};
static double offset_y {};
//static angle_t zOp {};

static bool fill { false };
static bool show_axis { false };


static GLubyte rgb[3] { 0, 255, 255};
static GLubyte color_delta { 3 };

static double fov {}, near_ {}, far_ {};
static CartPoint3d position;
static CartPoint3d look_at;
static CartPoint3d up;
static CartPoint3d direction;
static bool first_person {false};

bool firstMouse = true;
int lastX;
int lastY;

float yaw = 0.0f;
float pitch = 0.0f;

const float sensitivity = 0.5f;
const float speed = 0.1f;

float radians(float degree){
    return degree * (PI / 180);
}

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
                glRotated(t->angle.value(), t->point.x, t->point.y, t->point.z);
                break;

            case TransformType::scale:
                glScaled(t->point.x, t->point.y, t->point.z);
                break;

            case TransformType::translate:
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

    if(!first_person){

        float radius = sqrt(position.z * position.z + position.x * position.x);

        switch (key_code){

        case GLUT_KEY_LEFT:
            yaw -= 0.1;
            break;
        case GLUT_KEY_RIGHT :
            yaw += 0.1;
            break;
        case GLUT_KEY_DOWN:
            pitch -= 0.1;
            break;
        case GLUT_KEY_UP:
            pitch += 0.1;
            break;
        case GLUT_KEY_PAGE_DOWN:
            radius -= 0.1;
            break;
        case GLUT_KEY_PAGE_UP:
            radius += 0.1;
            break;
        default:
            break;
        }

        position.x = radius * (radians(yaw)) * cos(radians(pitch));
        position.y = radius * sin(radians(pitch));
        position.z = radius * cos(radians(yaw)) * cos(radians(pitch));

    }
    else{

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
            break;
        }

        look_at.x = position.x + direction.x;
        look_at.y = position.y + direction.y;
        look_at.z = position.z + direction.z;
    }

    glutPostRedisplay();
}

// function to process normal keyboard events
void keys_event(unsigned char key, int x, int y){

    switch(key){

    case 'p':
        if(first_person) {
            look_at.x = 0.0f;
            look_at.y = 0.0f;
            look_at.z = 0.0f;
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

    default:
        break;
    }

    glutPostRedisplay();
}



// function to process mouse events

void mouse_event(int x, int y){

    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }
  
    float xoffset = x - lastX;
    float yoffset = y - lastY; 
    lastX = x;
    lastY = y;

    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    yaw   -= xoffset;
    pitch -= yoffset;
/*
    if(x>50){
        yaw += 0.00001f;
    }
    else if(x<-50){
        yaw -= 0.00001f;
    }

    if(y>50){
        pitch += 0.00001f;
    }
    else if(y<-50){
        pitch -= 0.00001f;
    }
*/
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    direction.x = sin(radians(yaw)) * cos(radians(pitch));
    direction.y = sin(radians(pitch));
    direction.z = cos(radians(yaw)) * cos(radians(pitch));

    look_at.x = position.x + direction.x;
    look_at.y = position.y + direction.y;
    look_at.z = position.z + direction.z;

    glutPostRedisplay();
}


void glut_start(int argc, char** argv){

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG Phase 2");


    // Required callback registry
    glutDisplayFunc(render_scene);
    glutReshapeFunc(change_size);

    // put here the registration of the keyboard and mouse callbacks
    glutKeyboardFunc(keys_event);
    glutSpecialFunc(special_keys_event);
    glutPassiveMotionFunc(mouse_event);


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
            //zOp = std::atan2(cs.position.x, cs.position.z);


            float dZ = position.z - look_at.z;
            float dY = position.y - look_at.y;
            float dX = position.x - look_at.x;

            yaw = atan2(dZ, dX);
            pitch = atan2(sqrt(dZ * dZ + dX * dX), dY) + PI;

            direction.x = sin(radians(yaw)) * cos(radians(pitch));
            direction.y = sin(radians(pitch));
            direction.z = cos(radians(yaw)) * cos(radians(pitch));


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