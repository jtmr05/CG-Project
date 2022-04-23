#include "display.hpp"

using std::vector;
using std::string;
using std::set;



CameraSettings cs {};

static vector<Group> groups_to_draw {};
static VBO* vbo_wrapper { nullptr };



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
    gluPerspective(cs.fov, ratio, cs.near_, cs.far_);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}


static void render_scene(){

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(cs.position.x, cs.position.y, cs.position.z,
              cs.look_at.x, cs.look_at.y, cs.look_at.z,
              cs.up.x, cs.up.y, cs.up.z);

    // put drawing instructions here
    set_axis();
    set_color();
    set_polygon_mode();



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

        //TODO
        /**
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
        } **/


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

    if(argc == 1)
        return ErrorCode::not_enough_args;


    const string filename { argv[1] };
    const ErrorCode code { xml_parser(filename, cs, groups_to_draw) }; //groups_to_draw and cs are global

    if(code == ErrorCode::success){
        interaction_init();
        glut_start(argc, argv);
    }

    return code;
}
