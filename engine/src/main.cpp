
/**
 * Just for testing purposes for now :)
 *
 */

#include <string>
#include <fstream>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>


#include "point.hpp"

using std::string;
using std::vector;

std::vector<CartPoint3d> points_to_draw;
float posx{}, posy{}, posz{};

void drawTriangle(const CartPoint3d &p1, const CartPoint3d &p2, const CartPoint3d &p3){

    glBegin(GL_LINES);

        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);

    glEnd();

    glBegin(GL_LINES);

        glVertex3f(p2.x, p2.y, p2.z);
        glVertex3f(p3.x, p3.y, p3.z);

    glEnd();

    glBegin(GL_LINES);

        glVertex3f(p3.x, p3.y, p3.z);
        glVertex3f(p1.x, p1.y, p1.z);

    glEnd();
}

void changeSize(int w, int h){

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if(h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = w * 1.0 / h;

    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);

    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}


void renderScene(){

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(5.0+posx,5.0+posy,5.0+posz,
              0.0,0.0,0.0,
              0.0f,1.0f,0.0f);

    // geometric transformations here
    //glTranslatef(posx, posy, posz);

    // put drawing instructions here
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

    glColor3f(1.f, 1.f, 1.f);

    for(auto i = points_to_draw.begin(); i != points_to_draw.end(); i += 3)
        drawTriangle(*i, *(i+1), *(i+2));



    // End of frame
    glutSwapBuffers();
}

// write function to process keyboard events
void specialKeysEvent(int key_code, int x, int y){

    switch (key_code){

    case GLUT_KEY_LEFT:
        posz -= 0.1f;
        break;

    case GLUT_KEY_RIGHT :
        posz += 0.1f;
        break;

    case GLUT_KEY_DOWN:
        posx += 0.1f;
        break;

    case GLUT_KEY_UP:
        posx -= 0.1f;
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

void keysEvent(unsigned char key, int x, int y){

    switch(key){

    case 'w':
        posy -= 0.1f;
        break;

    case 's':
        posy += 0.1f;
        break;

    default:
        break;

    }

    glutPostRedisplay();
}

int main(int argc, char **argv) {

    if(argc > 1){

        string filename = { argv[1] };
        std::ifstream file{};
        file.open(filename, std::ios::in);

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
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800,800);
    glutCreateWindow("pain.exe");

// Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

// put here the registration of the keyboard callbacks
    glutKeyboardFunc(keysEvent);
    glutSpecialFunc(specialKeysEvent);


//  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

// enter GLUT's main cycle
    glutMainLoop();

    return 0;
}


