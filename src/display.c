#include "display.h"
#include "log.h"
#include "configuration.h"
#include "render.h"

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

bool display_iterate(void) {
  if(glfwWindowShouldClose(glfw_window)) {
    return false;
  }

  glfwPollEvents();

  // process inputs

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
    return false;
  }

  return true;
}

void display_shutdown(void) {
  render_shutdown();
}
