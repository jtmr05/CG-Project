#include "interaction.hpp"

extern CameraSettings cs;



static bool fill { true };
static bool lighting_enabled { true };
static bool show_axis { false };

static bool first_person { false };

static constexpr float sensitivity { 0.25f };
static constexpr float speed { 0.1f };
static constexpr angle_t angle_delta { 1.0 };


void interaction_init(bool lighting_available){
    lighting_enabled = lighting_available;
    fill = lighting_available;
}

// function to process special keyboard events
void special_keys_event(int key_code, int, int){

    const CartPoint3d dir { cs.look_at - cs.position };
    const CartPoint3d right { speed * cross_product(dir, cs.up).normalize() };
    const CartPoint3d real_up { speed * cross_product(right, dir).normalize() };

    if(!first_person){

        switch (key_code){

        case GLUT_KEY_LEFT: {
            const CartPoint3d left { -1.0 * right };
            cs.position += left;
            break;
        }

        case GLUT_KEY_RIGHT:
            cs.position += right;
            break;

        case GLUT_KEY_DOWN: {
            const CartPoint3d real_down { -1.0 * real_up };
            cs.position += real_down;
            break;
        }

        case GLUT_KEY_UP:
            cs.position += real_up;
            break;

        default:
            break;
        }
    }


    else {

        switch (key_code){

        case GLUT_KEY_LEFT: {
            const CartPoint3d left { -1.0 * right };
            cs.position += left;
            cs.look_at  += left;
            break;
        }

        case GLUT_KEY_RIGHT:
            cs.position += right;
            cs.look_at  += right;
            break;

        case GLUT_KEY_DOWN:
            cs.position += -1.0 * speed * dir.normalize();
            cs.look_at  += -1.0 * speed * dir.normalize();
            break;

        case GLUT_KEY_UP:
            cs.position += speed * dir.normalize();
            cs.look_at  += speed * dir.normalize();
            break;

        default:
            break;
        }
    }

    glutPostRedisplay();
}

// function to process normal keyboard events
void keys_event(unsigned char key, int, int){

    switch(key){

    case 'p':
        first_person = !first_person;
        break;

    case 'f':
        fill = !fill;
        break;

    case 'l':
        lighting_enabled = !lighting_enabled;
        break;

    case 'x':
        show_axis = !show_axis;
        break;

    case 'e':
        glutLeaveMainLoop();
        break;

    case 'q':
        std::exit(1);
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

// function to process mouse events
void mouse_event(int x, int y){

    static int last_x { x }, last_y { y }; //initialized only once

    if(first_person){

        const double diff_x { static_cast<double>(x - last_x) };
        const double diff_y { static_cast<double>(y - last_y) };

        PolarPoint3d dir { cart_to_polar(cs.look_at - cs.position) };

        dir.zOx += -1.0 * angle_delta * diff_x;
        dir.yOp = std::max(0.0, std::min(dir.yOp + (angle_delta * diff_y), 180.0));

        cs.look_at = polar_to_cart(dir);


        last_x = x;
        last_y = y;
    }

    glutPostRedisplay();
}



void set_axis(){

    glDisable(GL_LIGHTING);

    if(show_axis){

        glBegin(GL_LINES);

            // X axis in red
            glColor3ub(255, 0, 0);
            glVertex3f(-1000.0f, 0.0f, 0.0f);
            glVertex3f( 1000.0f, 0.0f, 0.0f);

            // Y Axis in Green
            glColor3ub(0, 255, 0);
            glVertex3f(0.0f, -1000.0f, 0.0f);
            glVertex3f(0.0f,  1000.0f, 0.0f);

            // Z Axis in Blue
            glColor3ub(0, 0, 255);
            glVertex3f(0.0f, 0.0f, -1000.0f);
            glVertex3f(0.0f, 0.0f,  1000.0f);

        glEnd();
    }

    if(lighting_enabled)
        glEnable(GL_LIGHTING);
}

void set_polygon_mode(){
    glPolygonMode(GL_FRONT, fill ? GL_FILL : GL_LINE);
}

bool set_lighting(){

    if(lighting_enabled)
        glEnable(GL_LIGHTING);
    else
        glDisable(GL_LIGHTING);

    return lighting_enabled;
}