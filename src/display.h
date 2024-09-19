#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "input.h"

bool display_initialize(void);
void display_shutdown(void);

void display_dimensions(uint32_t * width, uint32_t * height);
void display_get_cursor_pos(double * xpos, double * ypos);
bool display_get_mouse_button_down(enum input_Device device);bool display_get_key_down(enum input_Device device);
