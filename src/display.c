#include "display.h"
#include "log.h"
#include "configuration.h"
#include "render.h"
#include "platform.h"

#define GLAD_GL_IMPLEMENTATION

#include "glad.h"
#include <GLFW/glfw3.h>

#define SOURCE_NAMESPACE display

#define WINDOW_MODE_WINDOWED   0
#define WINDOW_MODE_FULLSCREEN 1
#define WINDOW_MODE_BORDERLESS 2

static CONFIGURATION_INTEGER(SOURCE_NAMESPACE, window_width, 800);
static CONFIGURATION_INTEGER(SOURCE_NAMESPACE, window_height, 600);
static CONFIGURATION_INTEGER(SOURCE_NAMESPACE, window_mode, 0);
static CONFIGURATION_INTEGER(SOURCE_NAMESPACE, monitor_index, -1);
static CONFIGURATION_STRING(SOURCE_NAMESPACE, window_title, "merrin engine");

static GLFWwindow * glfw_window;

static void _gl_debug_message_callback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char *message, const void *ud) {
  const char *source_str = source == GL_DEBUG_SOURCE_API               ? "API"
                           : source == GL_DEBUG_SOURCE_WINDOW_SYSTEM   ? "Window System"
                           : source == GL_DEBUG_SOURCE_SHADER_COMPILER ? "Shader Compiler"
                           : source == GL_DEBUG_SOURCE_THIRD_PARTY     ? "Third Party"
                           : source == GL_DEBUG_SOURCE_APPLICATION     ? "Application"
                           : source == GL_DEBUG_SOURCE_OTHER           ? "Other"
                                                                  : "Invalid";

  const char *type_str = type == GL_DEBUG_TYPE_ERROR                 ? "Error"
                         : type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ? "Deprecated Behavior"
                         : type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  ? "Undefined Behavior"
                         : type == GL_DEBUG_TYPE_PORTABILITY         ? "Portability"
                         : type == GL_DEBUG_TYPE_PERFORMANCE         ? "Performance"
                         : type == GL_DEBUG_TYPE_MARKER              ? "Marker"
                         : type == GL_DEBUG_TYPE_PUSH_GROUP          ? "Push Group"
                         : type == GL_DEBUG_TYPE_POP_GROUP           ? "Pop Group"
                         : type == GL_DEBUG_TYPE_OTHER               ? "Other"
                                                                  : "Invalid";

  switch(severity) {
  case GL_DEBUG_SEVERITY_HIGH:
  default:
    ERROR(SOURCE_NAMESPACE, "%s from %s: %.*s", type_str, source_str, length, message);
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    WARNING(SOURCE_NAMESPACE, "%s from %s: %.*s", type_str, source_str, length, message);
    break;
  case GL_DEBUG_SEVERITY_LOW:
    DEBUG(SOURCE_NAMESPACE, "%s from %s: %.*s", type_str, source_str, length, message);
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    TRACE(SOURCE_NAMESPACE, "%s from %s: %.*s", type_str, source_str, length, message);
    break;
  }
}

static bool _iterate(void) {
  if(glfwWindowShouldClose(glfw_window)) {
    return false;
  }

  glfwPollEvents();

  input_iterate();

  if(!render_iterate()) {
    return false;
  }

  glfwSwapBuffers(glfw_window);

  return true;
}

bool display_initialize(void) {
  if(!glfwInit()) {
    ERROR(SOURCE_NAMESPACE, "glfw: failed to initialize");
    return false;
  }

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  int width = window_width;
  int height = window_height;

  GLFWmonitor *primary = glfwGetPrimaryMonitor();

  int count;
  GLFWmonitor **monitors = glfwGetMonitors(&count);

  GLFWmonitor *monitor = monitor_index == -1 ? primary : monitors[monitor_index < 0 ? 0 : monitor_index > count ? count : monitor_index];

  const GLFWvidmode *mode = glfwGetVideoMode(monitor);

  if(window_mode == WINDOW_MODE_WINDOWED) {
    monitor = NULL;

    if(width == 0) {
      width = mode->width / 2;
    }

    if(height == 0) {
      height = mode->height / 2;
    }
  } else if(window_mode == WINDOW_MODE_FULLSCREEN) {
  } else if(window_mode == WINDOW_MODE_BORDERLESS) {
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    width = mode->width;
    height = mode->height;
  }

  glfw_window = glfwCreateWindow(width, height, window_title, monitor, NULL);
  if(glfw_window == NULL) {
    ERROR(SOURCE_NAMESPACE, "glfw: could not create window");
    return false;
  }

  glfwMakeContextCurrent(glfw_window);

  if(gladLoadGL(glfwGetProcAddress) == 0) {
    ERROR(SOURCE_NAMESPACE, "glad: could not load");
    return false;
  }

  glDebugMessageCallback(_gl_debug_message_callback, NULL);

  if(!render_initialize()) {
    ERROR(SOURCE_NAMESPACE, "render: failed initialization");
    return false;
  }

  platform_schedule_interval(_iterate, (uint64_t)(1000 / 16)); // 60 fps

  return true;
}

void display_shutdown(void) {
  render_shutdown();
}

void display_dimensions(uint32_t * width, uint32_t * height) {
  *width = window_width;
  *height = window_height;
}

void display_get_cursor_pos(double * xpos, double * ypos) {
  glfwGetCursorPos(glfw_window, xpos, ypos);
}

bool display_get_mouse_button_down(enum input_Device device) {
  int source_to_mouse[] = {
    [input_Device_mouseLeftButton] = GLFW_MOUSE_BUTTON_1,
    [input_Device_mouseRightButton] = GLFW_MOUSE_BUTTON_2
  };
  int state = glfwGetMouseButton(glfw_window, source_to_mouse[device]);
  return state == GLFW_PRESS;
}

bool display_get_key_down(enum input_Device device) {
  static int source_to_key[] = {
    [input_Device_keyboardApostrophe] = GLFW_KEY_APOSTROPHE,
    [input_Device_keyboardComma] = GLFW_KEY_COMMA,
    [input_Device_keyboardMinus] = GLFW_KEY_MINUS,
    [input_Device_keyboardPeriod] = GLFW_KEY_PERIOD,
    [input_Device_keyboardSlash] = GLFW_KEY_SLASH,
    [input_Device_keyboard0] = GLFW_KEY_0,
    [input_Device_keyboard1] = GLFW_KEY_1,
    [input_Device_keyboard2] = GLFW_KEY_2,
    [input_Device_keyboard3] = GLFW_KEY_3,
    [input_Device_keyboard4] = GLFW_KEY_4,
    [input_Device_keyboard5] = GLFW_KEY_5,
    [input_Device_keyboard6] = GLFW_KEY_6,
    [input_Device_keyboard7] = GLFW_KEY_7,
    [input_Device_keyboard8] = GLFW_KEY_8,
    [input_Device_keyboard9] = GLFW_KEY_9,
    [input_Device_keyboardSemicolon] = GLFW_KEY_SEMICOLON,
    [input_Device_keyboardEqual] = GLFW_KEY_EQUAL,
    [input_Device_keyboardA] = GLFW_KEY_A,
    [input_Device_keyboardB] = GLFW_KEY_B,
    [input_Device_keyboardC] = GLFW_KEY_C,
    [input_Device_keyboardD] = GLFW_KEY_D,
    [input_Device_keyboardE] = GLFW_KEY_E,
    [input_Device_keyboardF] = GLFW_KEY_F,
    [input_Device_keyboardG] = GLFW_KEY_G,
    [input_Device_keyboardH] = GLFW_KEY_H,
    [input_Device_keyboardI] = GLFW_KEY_I,
    [input_Device_keyboardJ] = GLFW_KEY_J,
    [input_Device_keyboardK] = GLFW_KEY_K,
    [input_Device_keyboardL] = GLFW_KEY_L,
    [input_Device_keyboardM] = GLFW_KEY_M,
    [input_Device_keyboardN] = GLFW_KEY_N,
    [input_Device_keyboardO] = GLFW_KEY_O,
    [input_Device_keyboardP] = GLFW_KEY_P,
    [input_Device_keyboardQ] = GLFW_KEY_Q,
    [input_Device_keyboardR] = GLFW_KEY_R,
    [input_Device_keyboardS] = GLFW_KEY_S,
    [input_Device_keyboardT] = GLFW_KEY_T,
    [input_Device_keyboardU] = GLFW_KEY_U,
    [input_Device_keyboardV] = GLFW_KEY_V,
    [input_Device_keyboardW] = GLFW_KEY_W,
    [input_Device_keyboardX] = GLFW_KEY_X,
    [input_Device_keyboardY] = GLFW_KEY_Y,
    [input_Device_keyboardZ] = GLFW_KEY_Z,
    [input_Device_keyboardSpace] = GLFW_KEY_SPACE,
    [input_Device_keyboardEscape] = GLFW_KEY_ESCAPE,
    [input_Device_keyboardEnter] = GLFW_KEY_ENTER,
    [input_Device_keyboardTab] = GLFW_KEY_TAB,
    [input_Device_keyboardBackspace] = GLFW_KEY_BACKSPACE,
    [input_Device_keyboardInsert] = GLFW_KEY_INSERT,
    [input_Device_keyboardDelete] = GLFW_KEY_DELETE,
    [input_Device_keyboardRight] = GLFW_KEY_RIGHT,
    [input_Device_keyboardLeft] = GLFW_KEY_LEFT,
    [input_Device_keyboardDown] = GLFW_KEY_DOWN,
    [input_Device_keyboardUp] = GLFW_KEY_UP,
    [input_Device_keyboardPageUp] = GLFW_KEY_PAGE_UP,
    [input_Device_keyboardPageDown] = GLFW_KEY_PAGE_DOWN,
    [input_Device_keyboardHome] = GLFW_KEY_HOME,
    [input_Device_keyboardEnd] = GLFW_KEY_END,
    [input_Device_keyboardCapsLock] = GLFW_KEY_CAPS_LOCK,
    [input_Device_keyboardScrollLock] = GLFW_KEY_SCROLL_LOCK,
    [input_Device_keyboardNumLock] = GLFW_KEY_NUM_LOCK,
    [input_Device_keyboardPrintScreen] = GLFW_KEY_PRINT_SCREEN,
    [input_Device_keyboardPause] = GLFW_KEY_PAUSE,
    [input_Device_keyboardF1] = GLFW_KEY_F1,
    [input_Device_keyboardF2] = GLFW_KEY_F2,
    [input_Device_keyboardF3] = GLFW_KEY_F3,
    [input_Device_keyboardF4] = GLFW_KEY_F4,
    [input_Device_keyboardF5] = GLFW_KEY_F5,
    [input_Device_keyboardF6] = GLFW_KEY_F6,
    [input_Device_keyboardF7] = GLFW_KEY_F7,
    [input_Device_keyboardF8] = GLFW_KEY_F8,
    [input_Device_keyboardF9] = GLFW_KEY_F9,
    [input_Device_keyboardF10] = GLFW_KEY_F10,
    [input_Device_keyboardF11] = GLFW_KEY_F11,
    [input_Device_keyboardF12] = GLFW_KEY_F12,
    [input_Device_keyboardLeftShift] = GLFW_KEY_LEFT_SHIFT,
    [input_Device_keyboardLeftControl] = GLFW_KEY_LEFT_CONTROL,
    [input_Device_keyboardLeftAlt] = GLFW_KEY_LEFT_ALT,
    [input_Device_keyboardLeftMeta] = GLFW_KEY_LEFT_SUPER,
    [input_Device_keyboardRightShift] = GLFW_KEY_RIGHT_SHIFT,
    [input_Device_keyboardRightControl] = GLFW_KEY_RIGHT_CONTROL,
    [input_Device_keyboardRightAlt] = GLFW_KEY_RIGHT_ALT,
    [input_Device_keyboardRightMeta] = GLFW_KEY_RIGHT_SUPER,
    [input_Device_keyboardMenu] = GLFW_KEY_MENU,
    [input_Device_keyboardLeftBracket] = GLFW_KEY_LEFT_BRACKET,
    [input_Device_keyboardBackslash] = GLFW_KEY_BACKSLASH,
    [input_Device_keyboardRightBracket] = GLFW_KEY_RIGHT_BRACKET,
    [input_Device_keyboardGrave] = GLFW_KEY_GRAVE_ACCENT,
    [input_Device_keyboardPad0] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad1] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad2] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad3] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad4] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad5] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad6] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad7] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad8] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPad9] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPadPeriod] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPadDivide] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPadMultiply] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPadMinus] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPadAdd] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPadEnter] = GLFW_KEY_UNKNOWN,
    [input_Device_keyboardPadEqual] = GLFW_KEY_UNKNOWN
  };
  int state = glfwGetKey(glfw_window, source_to_key[device]);
  return state == GLFW_PRESS;
}
