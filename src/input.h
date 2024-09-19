#ifndef input_h_INCLUDED
#define input_h_INCLUDED

#include "math.h"
#include "ecs.h"
#include "camera.h"
#include "transform.h"

enum input_Device {
  input_Device_keyboardApostrophe,
  input_Device_keyboardComma,
  input_Device_keyboardMinus,
  input_Device_keyboardPeriod,
  input_Device_keyboardSlash,
  input_Device_keyboard0,
  input_Device_keyboard1,
  input_Device_keyboard2,
  input_Device_keyboard3,
  input_Device_keyboard4,
  input_Device_keyboard5,
  input_Device_keyboard6,
  input_Device_keyboard7,
  input_Device_keyboard8,
  input_Device_keyboard9,
  input_Device_keyboardSemicolon,
  input_Device_keyboardEqual,
  input_Device_keyboardA,
  input_Device_keyboardB,
  input_Device_keyboardC,
  input_Device_keyboardD,
  input_Device_keyboardE,
  input_Device_keyboardF,
  input_Device_keyboardG,
  input_Device_keyboardH,
  input_Device_keyboardI,
  input_Device_keyboardJ,
  input_Device_keyboardK,
  input_Device_keyboardL,
  input_Device_keyboardM,
  input_Device_keyboardN,
  input_Device_keyboardO,
  input_Device_keyboardP,
  input_Device_keyboardQ,
  input_Device_keyboardR,
  input_Device_keyboardS,
  input_Device_keyboardT,
  input_Device_keyboardU,
  input_Device_keyboardV,
  input_Device_keyboardW,
  input_Device_keyboardX,
  input_Device_keyboardY,
  input_Device_keyboardZ,
  input_Device_keyboardSpace,
  input_Device_keyboardEscape,
  input_Device_keyboardEnter,
  input_Device_keyboardTab,
  input_Device_keyboardBackspace,
  input_Device_keyboardInsert,
  input_Device_keyboardDelete,
  input_Device_keyboardRight,
  input_Device_keyboardLeft,
  input_Device_keyboardDown,
  input_Device_keyboardUp,
  input_Device_keyboardPageUp,
  input_Device_keyboardPageDown,
  input_Device_keyboardHome,
  input_Device_keyboardEnd,
  input_Device_keyboardCapsLock,
  input_Device_keyboardScrollLock,
  input_Device_keyboardNumLock,
  input_Device_keyboardPrintScreen,
  input_Device_keyboardPause,
  input_Device_keyboardF1,
  input_Device_keyboardF2,
  input_Device_keyboardF3,
  input_Device_keyboardF4,
  input_Device_keyboardF5,
  input_Device_keyboardF6,
  input_Device_keyboardF7,
  input_Device_keyboardF8,
  input_Device_keyboardF9,
  input_Device_keyboardF10,
  input_Device_keyboardF11,
  input_Device_keyboardF12,
  input_Device_keyboardLeftShift,
  input_Device_keyboardLeftControl,
  input_Device_keyboardLeftAlt,
  input_Device_keyboardLeftMeta,
  input_Device_keyboardRightShift,
  input_Device_keyboardRightControl,
  input_Device_keyboardRightAlt,
  input_Device_keyboardRightMeta,
  input_Device_keyboardMenu,
  input_Device_keyboardLeftBracket,
  input_Device_keyboardBackslash,
  input_Device_keyboardRightBracket,
  input_Device_keyboardGrave,
  input_Device_keyboardPad0,
  input_Device_keyboardPad1,
  input_Device_keyboardPad2,
  input_Device_keyboardPad3,
  input_Device_keyboardPad4,
  input_Device_keyboardPad5,
  input_Device_keyboardPad6,
  input_Device_keyboardPad7,
  input_Device_keyboardPad8,
  input_Device_keyboardPad9,
  input_Device_keyboardPadPeriod,
  input_Device_keyboardPadDivide,
  input_Device_keyboardPadMultiply,
  input_Device_keyboardPadMinus,
  input_Device_keyboardPadAdd,
  input_Device_keyboardPadEnter,
  input_Device_keyboardPadEqual,

  input_Device_mouseLeftButton,
  input_Device_mouseRightButton,
  input_Device_mousePositionX,
  input_Device_mousePositionY,

  input_Device_COUNT
};

struct input_BackendPair {
  enum input_Device device;
  uint32_t binding;
};

struct input_Signal {
  enum {
    input_Signal_pass,
    input_Signal_up,
    input_Signal_down,
    input_Signal_direction,
    input_Signal_point,
    input_Signal_viewportPoint
  } type;

  // inputs
  uint32_t bindings[2];
  union {
    ecs_EntityHandle click_camera;
  };

  // output
  union {
    bool boolean;
    pga2d_Direction direction;
    pga2d_Point point;
  };
  union {
    bool up;
    bool down;
  } internal;
};

#define INPUT_SIGNAL_PASS(BINDING)                                                                              \
  (struct input_Signal) { .type = input_Signal_pass, .bindings[0] = BINDING }
#define INPUT_SIGNAL_UP(BINDING)                                                                                \
  (struct input_Signal) { .type = input_Signal_up, .bindings[0] = BINDING }
#define INPUT_SIGNAL_DOWN(BINDING)                                                                              \
  (struct input_Signal) { .type = input_Signal_down, .bindings[0] = BINDING }
#define INPUT_SIGNAL_DIRECTION(BINDING_X, BINDING_Y)                                                            \
  (struct input_Signal) {                                                                                       \
    .type = input_Signal_direction, .bindings[0] = BINDING_X, .bindings[1] = BINDING_Y                          \
  }
#define INPUT_SIGNAL_POINT(BINDING_X, BINDING_Y)                                                                \
  (struct input_Signal) { .type = input_Signal_point, .bindings[0] = BINDING_X, .bindings[1] = BINDING_Y }
#define INPUT_SIGNAL_VIEWPORT_POINT(BINDING_X, BINDING_Y, CAMERA)                                               \
  (struct input_Signal) {                                                                                       \
    .type = input_Signal_viewportPoint, .bindings[0] = BINDING_X, .bindings[1] = BINDING_Y,                     \
    .click_camera = CAMERA                                                                                             \
  }

void input_set_player_backend(uint32_t player_index, const struct input_BackendPair * pairs, uint32_t pair_count);
uint32_t input_add_frontend(uint32_t player_index, struct input_Signal * signals, uint32_t signal_count);
void input_remove_frontend(uint32_t player_index, uint32_t index);
void input_iterate(void);

#endif // input_h_INCLUDED
