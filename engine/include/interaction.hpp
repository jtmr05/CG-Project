#ifndef INTERACTION_HPP
#define INTERACTION_HPP


#include "data_structures.hpp"

#include <GL/glut.h>
#include <GL/freeglut.h>


void interaction_init();

void special_keys_event(int key_code, int, int);
void keys_event(unsigned char key, int, int);
void mouse_event(int x, int y);

void set_axis();
void set_polygon_mode();
bool set_lighting();

#endif