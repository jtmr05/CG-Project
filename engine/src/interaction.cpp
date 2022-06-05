#include "interaction.hpp"

extern CameraSettings cs;



//options
static bool fill { true };
static bool lighting_enabled { true };
static bool show_axis { false };
static bool first_person { false };

static constexpr double speed { 0.1 };
static constexpr angle_t fps_angle_delta { 0.5 };
static constexpr angle_t fixed_mode_angle_delta { 2.5 };


void interaction_init(bool lighting_available){
    lighting_enabled = lighting_available;
    fill = lighting_available;
}

// function to process special keyboard events
void special_keys_event(int key_code, int, int){

    const CartPoint3d dir { cs.look_at - cs.position };

    if(!first_person){

        PolarPoint3d polar_dir { cart_to_polar(dir) };

        switch (key_code){

        case GLUT_KEY_LEFT:
            polar_dir.zOx -= fixed_mode_angle_delta;
            break;

        case GLUT_KEY_RIGHT:
            polar_dir.zOx += fixed_mode_angle_delta;
            break;

        case GLUT_KEY_DOWN:
            polar_dir.yOp = std::max(0.1, std::min(polar_dir.yOp - fixed_mode_angle_delta, 179.9));
            break;

        case GLUT_KEY_UP:
            polar_dir.yOp = std::max(0.1, std::min(polar_dir.yOp + fixed_mode_angle_delta, 179.9));
            break;

        default:
            break;
        }

        cs.position = cs.look_at - polar_to_cart(polar_dir);
    }


    else {

        const CartPoint3d right   { speed * cross_product(dir, cs.up).normalize() };
        const CartPoint3d real_up { speed * cross_product(right, dir).normalize() };

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

        case GLUT_KEY_DOWN: {
            const CartPoint3d weighted_dir { -1.0 * speed * dir.normalize() };
            cs.position += weighted_dir;
            cs.look_at  += weighted_dir;
            break;
        }

        case GLUT_KEY_UP: {
            const CartPoint3d weighted_dir { speed * dir.normalize() };
            cs.position += weighted_dir;
            cs.look_at  += weighted_dir;
            break;
        }

        default:
            break;
        }
    }

    glutPostRedisplay();
}

// function to process mouse events
void mouse_event(int x, int y){

    static int last_x { x }, last_y { y }; //initialized only once

    if(first_person){

        const double delta_x { static_cast<double>(x - last_x) };
        const double delta_y { static_cast<double>(y - last_y) };

        PolarPoint3d dir { cart_to_polar(cs.look_at - cs.position) };

        dir.zOx += -1.0 * fps_angle_delta * delta_x;
        dir.yOp = std::max(0.0, std::min(dir.yOp + (fps_angle_delta * delta_y), 179.9));

        cs.look_at = polar_to_cart(dir) + cs.position;
    }

    last_x = x;
    last_y = y;

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

    case 'w':
        special_keys_event(GLUT_KEY_UP, 0, 0);
        break;

    case 's':
        special_keys_event(GLUT_KEY_DOWN, 0, 0);
        break;

    case 'a':
        special_keys_event(GLUT_KEY_LEFT, 0, 0);
        break;

    case 'd':
        special_keys_event(GLUT_KEY_RIGHT, 0, 0);
        break;

    default:
        break;
    }

    glutPostRedisplay();
}



void set_axis(){

    glDisable(GL_LIGHTING);

    if(show_axis){

        glBegin(GL_LINES);

            // X axis in red
            glColor3ub(255, 0, 0);
            glVertex3d(-1000.0, 0.0, 0.0);
            glVertex3d( 1000.0, 0.0, 0.0);

            // Y Axis in Green
            glColor3ub(0, 255, 0);
            glVertex3d(0.0, -1000.0, 0.0);
            glVertex3d(0.0,  1000.0, 0.0);

            // Z Axis in Blue
            glColor3ub(0, 0, 255);
            glVertex3d(0.0, 0.0, -1000.0);
            glVertex3d(0.0, 0.0,  1000.0);

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