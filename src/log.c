#include "log.h"
#include "memory.h"
#include "format.h"
#include "configuration.h"

#include <stdio.h>

#define SOURCE_NAMESPACE core.log

static CONFIGURATION_STRING(SOURCE_NAMESPACE, log_file, NULL);

static FILE * active_log_file = NULL;
static uint32_t active_log_file_callback_index = -1;

static inline void active_log_callback(int level, const char * file, unsigned int line, const char * namespace, const char * message) {
  static const char * log_level_code[] = {
    [LogLevel_DEBUG] = "DEBUG",
    [LogLevel_WARNING] = "WARNING",
    [LogLevel_INFO] = "INFO",
    [LogLevel_ERROR] = "ERROR",
    [LogLevel_TRACE] = "TRACE",
  };
  fprintf(active_log_file, "%s:%u %s %s %s\n", file, line, namespace, log_level_code[level], message);
}

static struct {
  uint32_t num_callbacks;
  log_Callback * callbacks;
} _log = {0, NULL};

uint32_t log_add_callback(log_Callback callback) {
  for(uint32_t i = 0; i < _log.num_callbacks; i++) {
    if(_log.callbacks[i] == NULL) {
      _log.callbacks[i] = callback;
      return i;
    }
  }
  _log.callbacks = memory_realloc(
    _log.callbacks,
    sizeof(*_log.callbacks) * _log.num_callbacks,
    sizeof(*_log.callbacks) * (_log.num_callbacks + 1),
    alignof(*_log.callbacks)
  );
  _log.callbacks[_log.num_callbacks] = callback;
  return _log.num_callbacks++;
}

void log_remove_callback(uint32_t index) {
  if(index < _log.num_callbacks) {
    WARNING_IF(SOURCE_NAMESPACE, _log.callbacks[index] == NULL, "log index %i already removed", index);
    _log.callbacks[index] = NULL;
  } else {
    WARNING(SOURCE_NAMESPACE, "removing an invalid log index %i out of %i", index, _log.num_callbacks);
  }
}

void log_print_v(int level, const char * file, unsigned int line, const char * namespace, const char * format, va_list ap) {
  MStr message = format_alloc_v(format, ap);

  if((active_log_file == NULL) != (log_file == NULL)) {
    if(active_log_file != NULL) {
      fclose(active_log_file);
      if(active_log_file_callback_index != (uint32_t)-1) {
        log_remove_callback(active_log_file_callback_index);
        active_log_file_callback_index = (uint32_t)-1;
      }
    }
    if(log_file != NULL) {
      active_log_file = fopen(log_file, "w");
      if(active_log_file != NULL) {
        active_log_file_callback_index = log_add_callback(active_log_callback);
      }
    }
  }

  for(uint32_t i = 0; i < _log.num_callbacks; i++) {
    if(_log.callbacks[i] != NULL) {
      (_log.callbacks[i])(level, file, line, namespace, message);
    }
  }

  string_free(message);
}
