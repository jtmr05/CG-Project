#include "interaction.hpp"

extern CameraSettings cs;



static bool fill { false };
static bool show_axis { false };
static GLubyte rgb[3] { 0, 255, 255};
static GLubyte color_delta { 3 };

static CartPoint3d direction;
static bool first_person { false };
static bool first_mouse { true };
static int last_x;
static int last_y;

static float yaw { 0.f };
static float pitch { 0.f };

static constexpr float sensitivity { 0.5f };
static constexpr float speed { 0.1f };



void interaction_init(){
    const double dx { cs.position.x - cs.look_at.x };
    const double dy { cs.position.y - cs.look_at.y };
    const double dz { cs.position.z - cs.look_at.z };

    yaw = std::atan2(dz, dx);
    pitch = std::atan2(std::sqrt(dz * dz + dx * dx), dy) + PI;

    direction.x = std::sin(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
    direction.y = std::sin(degree_to_radian(pitch));
    direction.z = std::cos(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
}

// function to process special keyboard events
void special_keys_event(int key_code, int, int){

    if(!first_person){

        auto&& [px, py, pz] = cs.position;
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

            cs.position.x = radius * std::sin(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
            cs.position.y = radius * std::sin(degree_to_radian(pitch));
            cs.position.z = radius * std::cos(degree_to_radian(yaw)) * std::cos(degree_to_radian(pitch));
        }

    }
    else{

        bool nothing { false };

        switch (key_code){

        case GLUT_KEY_LEFT:
            cs.position.x += speed * direction.z;
            cs.position.y += speed * direction.y;
            cs.position.z -= speed * direction.x;
            break;

        case GLUT_KEY_RIGHT :
            cs.position.x -= speed * direction.z;
            cs.position.y += speed * direction.y;
            cs.position.z += speed * direction.x;
            break;

        case GLUT_KEY_DOWN:
            cs.position.x -= speed * direction.x;
            cs.position.y -= speed * direction.y;
            cs.position.z -= speed * direction.z;
            break;

        case GLUT_KEY_UP:
            cs.position.x += speed * direction.x;
            cs.position.y += speed * direction.y;
            cs.position.z += speed * direction.z;
            break;

        default:
            nothing = true;
            break;
        }

        if(!nothing){
            cs.look_at.x = cs.position.x + direction.x;
            cs.look_at.y = cs.position.y + direction.y;
            cs.look_at.z = cs.position.z + direction.z;
        }
    }

    glutPostRedisplay();
}

// function to process normal keyboard events
void keys_event(unsigned char key, int, int){

    switch(key){

    case 'p':
        if(first_person) {
            cs.look_at.x = 0.0f;
            cs.look_at.y = 0.0f;
            cs.look_at.z = 0.0f;

            const double dz { cs.position.z - cs.look_at.z };
            const double dy { cs.position.y - cs.look_at.y };
            const double dx { cs.position.x - cs.look_at.x };

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
        glutLeaveMainLoop();
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

// function to process mouse events
void mouse_event(int x, int y){

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

        cs.look_at.x = cs.position.x + direction.x;
        cs.look_at.y = cs.position.y + direction.y;
        cs.look_at.z = cs.position.z + direction.z;
    }

    glutPostRedisplay();
}



void set_axis(){
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
}

void set_color(){
    glColor3ub(rgb[0], rgb[1], rgb[2]);
}

void set_polygon_mode(){
    if(fill)
        glPolygonMode(GL_FRONT, GL_FILL);
    else
        glPolygonMode(GL_FRONT, GL_LINE);
}