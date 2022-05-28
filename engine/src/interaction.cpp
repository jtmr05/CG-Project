#include "interaction.hpp"

extern CameraSettings cs;



static bool fill { true };
static bool show_axis { false };
static bool lighting_enabled { true };

static CartPoint3d direction {};
static bool first_person { false };
static bool first_mouse { true };
static int last_x {};
static int last_y {};

static float yaw { 0.f };
static float pitch { 0.f };

static constexpr float sensitivity { 0.5f };
static constexpr float speed { 0.1f };



void interaction_init(bool lighting_available){

    if(!lighting_available){
        lighting_enabled = false;
        fill = false;
    }

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

    bool nothing { false };

    if(!first_person){

        auto const& [px, py, pz] { cs.position };
        double radius { std::sqrt(pz * pz + px * px + py * py) };

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
            cs.look_at.x = 0.0;
            cs.look_at.y = 0.0;
            cs.look_at.z = 0.0;

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

    if(first_person){

        if (first_mouse){
            last_x = x;
            last_y = y;
            first_mouse = false;
        }

        float const xoffset { static_cast<float>(x - last_x) * sensitivity };
        float const yoffset { static_cast<float>(y - last_y) * sensitivity };
        last_x = x;
        last_y = y;

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