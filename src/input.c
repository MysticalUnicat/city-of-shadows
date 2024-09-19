#include "input.h"
#include "math.h"
#include "camera.h"
#include "display.h"
#include "transform.h"

#define MAX_INPUT_FRONTEND_SETS 8

static uint32_t _input_backend_pair_count = 0;
static const struct input_BackendPair * _input_backend_pairs = NULL;

static uint32_t _input_binding_count = 0;
static R * _input_bindings = NULL;

void input_set_player_backend(uint32_t player_index, const struct input_BackendPair * pairs, uint32_t pair_count) {
  (void)player_index;
  _input_backend_pair_count = pair_count;
  _input_backend_pairs = pairs;

  uint32_t max_binding_index = 0;
  for(uint32_t i = 0; i < pair_count; i++) {
    if(pairs[i].binding > max_binding_index) {
      max_binding_index = pairs[i].binding;
    }
  }

  if(max_binding_index + 1 > _input_binding_count) {
    _input_bindings = memory_realloc(
        _input_bindings
      , sizeof(*_input_bindings) * _input_binding_count
      , sizeof(*_input_bindings) * (max_binding_index + 1)
      , alignof(*_input_bindings)
      );
    _input_binding_count = max_binding_index + 1;
  }
}

static struct {
  uint32_t count;
  struct input_Signal * signals;
} _input_frontends[MAX_INPUT_FRONTEND_SETS] = { 0 };

uint32_t input_add_frontend(uint32_t player_index, struct input_Signal * signals, uint32_t signal_count) {
  (void)player_index;

  uint32_t index = 0;
  for(; index < MAX_INPUT_FRONTEND_SETS; index++) {
    if(_input_frontends[index].count == 0) {
      break;
    }
  }
  if(index < MAX_INPUT_FRONTEND_SETS) {
    _input_frontends[index].count = signal_count;
    _input_frontends[index].signals = signals;
    return index;
  }
  return -1;
}

void input_remove_frontend(uint32_t player_index, uint32_t index) {
  (void)player_index;

  if(index < MAX_INPUT_FRONTEND_SETS) {
    _input_frontends[index].count = 0;
    _input_frontends[index].signals = NULL;
  }
}

void input_iterate(void) {
  double xpos, ypos;
  display_get_cursor_pos(&xpos, &ypos);

  memory_clear(_input_bindings, sizeof(*_input_bindings) * _input_binding_count);

  for(uint32_t i = 0; i < _input_backend_pair_count; i++) {
    enum input_Device device = _input_backend_pairs[i].device;
    R * binding = &_input_bindings[_input_backend_pairs[i].binding];

    switch(device) {
    case input_Device_keyboardApostrophe ... input_Device_keyboardPadEqual:
      if(display_get_key_down(device)) {
        *binding = R_ONE;
      }
      break;
    case input_Device_mouseLeftButton ... input_Device_mouseRightButton:
      if(display_get_mouse_button_down(device)) {
        *binding = R_ONE;
      }
      break;
    case input_Device_mousePositionX:
      *binding = xpos;
      break;
    case input_Device_mousePositionY:
      *binding = ypos;
      break;
    case input_Device_COUNT:
      break;
    }
  }

  for(uint32_t i = 0; i < MAX_INPUT_FRONTEND_SETS; i++) {
    for(uint32_t j = 0; j < _input_frontends[i].count; j++) {
      struct input_Signal * signal = &_input_frontends[i].signals[j];
      switch(signal->type) {
      case input_Signal_pass:
        {
          signal->boolean = _input_bindings[signal->bindings[0]] > R_ZERO;
          break;
        }
      case input_Signal_up:
        {
          bool value = _input_bindings[signal->bindings[0]] > R_ZERO;
          signal->boolean = !signal->internal.up && value;
          signal->internal.up = value;
          break;
        }
      case input_Signal_down:
        {
          bool value = _input_bindings[signal->bindings[0]] > R_ZERO;
          signal->boolean = signal->internal.down && !value;
          signal->internal.down = value;
          break;
        }
      case input_Signal_direction:
        {
          signal->direction = pga2d_direction(_input_bindings[signal->bindings[0]], _input_bindings[signal->bindings[1]]);
          break;
        }
      case input_Signal_point:
        {
          signal->point = pga2d_point(_input_bindings[signal->bindings[0]], _input_bindings[signal->bindings[1]]);
          break;
        }
      case input_Signal_viewportPoint:
        {
          R px = _input_bindings[signal->bindings[0]];
          R py = _input_bindings[signal->bindings[1]];

          const struct Camera * camera = Camera_read(signal->click_camera);
          const struct LocalToWorld2D * transform = LocalToWorld2D_read(signal->click_camera);

          uint32_t r_width, r_height;
          display_dimensions(&r_width, &r_height);

          R minx = pga2d_point_x(camera->viewport_min) * r_width;
          R miny = pga2d_point_y(camera->viewport_min) * r_height;
          R maxx = pga2d_point_x(camera->viewport_max) * r_width;
          R maxy = pga2d_point_y(camera->viewport_max) * r_height;
          R width = maxx - minx;
          R height = maxy - miny;

          px -= minx;
          py -= miny;
          if(px < 0 || py < 0 || px > width || py > height) {
            break;
          }

          px -= width / 2;
          py -= height / 2;
          
          R cx = pga2d_point_x(transform->position);
          R cy = pga2d_point_y(transform->position);

          px += cx;
          py += cy;

          signal->point = pga2d_point(px, py);
          
          break;
        }
      }
    }
  }
}
