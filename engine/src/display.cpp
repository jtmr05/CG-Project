#include "display.hpp"

static vector<CartPoint3d> points_to_draw {};
static double posy {};
static double zOp { PI / 4};

static bool fill { false };
static bool show_axis { false };

static GLubyte rgb[3] { 255, 255, 0};
static GLubyte color_delta { 3 };

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
    gluPerspective(45.f, ratio, 1.f ,1000.f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}


void render_scene(){

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(5.0 * std::sin(zOp), 5.0 + posy, 5.0 * std::cos(zOp),
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

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

    for(auto i = points_to_draw.begin(); i != points_to_draw.end(); i += 3)
        draw_triangle(i[0], i[1], i[2]);


    // End of frame
    glutSwapBuffers();
}

// write function to process keyboard events
void special_keys_event(int key_code, int x, int y){

    switch (key_code){

    case GLUT_KEY_LEFT:
        zOp -= 0.01;
        break;

    case GLUT_KEY_RIGHT :
        zOp += 0.01;
        break;

    case GLUT_KEY_DOWN:
        posy -= 0.1;
        break;

    case GLUT_KEY_UP:
        posy += 0.1;
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

void keys_event(unsigned char key, int x, int y){

    switch(key){

    case 'w':
        posy -= 0.1f;
        break;

    case 's':
        posy += 0.1f;
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



ErrorCode start(char** filenames, int N){

    ErrorCode exit_code { ErrorCode::default__ };

    for(int i = 1; i < N; ++i){

        std::ifstream file{};
        file.open(filenames[i], std::ios::in);

        if(file.is_open()){

            CartPoint3d p1{}, p2{}, p3{};
            int n {};

            while(file >> p1 >> p2 >> p3){
                n += 3;
                points_to_draw.push_back(p1);
                points_to_draw.push_back(p2);
                points_to_draw.push_back(p3);
            }

            std::cout << "number of points: " << n << '\n';

            file.close();
        }
    }



    // init GLUT and the window
    glutInit(&N, filenames);
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

    return exit_code;
}
