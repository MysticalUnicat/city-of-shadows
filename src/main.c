#include "log.h"
#include "string.h"
#include "configuration_local.h"
#include "script.h"
#include "platform.h"
#include "display.h"

#include "game_local.h"

#include <stdlib.h>

#define SOURCE_NAMESPACE core.main

CONFIGURATION_BOOLEAN(
  SOURCE_NAMESPACE, editor, false
#ifndef ENABLE_TOOLS
  , .flag_immutable = true
#endif
);

static CONFIGURATION_BOOLEAN(application.run, headless, false);
static CONFIGURATION_STRING(application.run, main_scene, NULL);

static inline void _initialization_log_callback(int level, const char * file, unsigned int line, const char * namespace, const char * message) {
  static const char * log_level_code[] = {
    [LogLevel_DEBUG] = "D",
    [LogLevel_WARNING] = "W",
    [LogLevel_INFO] = "I",
    [LogLevel_ERROR] = "E",
    [LogLevel_TRACE] = "T",
  };
  platform_printf("%s:%u %s %s %s\n", file, line, namespace, log_level_code[level], message);
}

int ecs_initialize(void);
void ecs_shutdown(void);

int main(int argc, char * argv []) {
  
  uint32_t initialization_log_callback_index = log_add_callback(_initialization_log_callback);

  INFO(SOURCE_NAMESPACE, "initialization begin");

  // some platforms may need to massage the arguments
  platform_initialize_arguments(argc, argv);

  for(int i = 1; i < argc; ) {
    if(string_equals(platform_argv(i), "-set")) {
      const char * key = platform_argv(i + 1);
      const char * value = platform_argv(i + 2);
      if(key == NULL) {
        ERROR(SOURCE_NAMESPACE, "-set requires at least one argument");
        continue;
      }
      if(value == NULL) {
        ERROR(SOURCE_NAMESPACE, "-set requires two arguments");
        continue;
      }
      configuration_set(key, value);
      i += 3;
      continue;
    }
  }

  // nothing in the platform depends on having configuration loaded from the project,
  // loaded here so variables can be set
  // nothing before here has used the platform specifically
  platform_initialize();

  // load the project configuration, overrides variables again
  configuration_initialize();

  script_initialize();

  ecs_initialize();

  display_initialize();

  script_load("test.c");

  INFO(SOURCE_NAMESPACE, "initialization complete");

  // log_remove_callback(initialization_log_callback_index);

  platform_run();

  INFO(SOURCE_NAMESPACE, "shutdown begin");

  display_shutdown();

  ecs_shutdown();

  configuration_save();

  INFO(SOURCE_NAMESPACE, "shutdown complete");

  return EXIT_SUCCESS;
}

