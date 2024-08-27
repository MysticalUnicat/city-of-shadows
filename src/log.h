#ifndef __LIBRARY_CORE_LOG_H__
#define __LIBRARY_CORE_LOG_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

enum {
  LogLevel_ERROR,
  LogLevel_WARNING,
  LogLevel_INFO,
  LogLevel_DEBUG,
  LogLevel_TRACE,
};

typedef void (* log_Callback)(int level, const char * file, unsigned int line, const char * namespace, const char * message);

uint32_t log_add_callback(log_Callback callback);
void log_remove_callback(uint32_t index);

void log_print_v(int level, const char * file, unsigned int line, const char * namespace, const char * format, va_list ap);

static void log_print(int level, const char * file, unsigned int line, const char * namespace, const char * format, ...) {
  va_list ap;
  va_start(ap, format);
  log_print_v(level, file, line, namespace, format, ap);
  va_end(ap);
}

#define LOG(LEVEL, FILE, LINE, NAMESPACE, FORMAT, ...) log_print(LEVEL, FILE, LINE, #NAMESPACE, FORMAT, ## __VA_ARGS__)

#define LOG_IF(LEVEL, FILE, LINE, NAMESPACE, CONDITION, FORMAT, ...) do { \
  if(CONDITION) { \
    log_print(LEVEL, FILE, LINE, #NAMESPACE, FORMAT, ## __VA_ARGS__); \
	} \
} while(0)

#define ERROR(NAMESPACE, FORMAT, ...) LOG(LogLevel_ERROR, __FILE__, __LINE__, NAMESPACE, FORMAT, ## __VA_ARGS__)
#define WARNING(NAMESPACE, FORMAT, ...) LOG(LogLevel_WARNING, __FILE__, __LINE__, NAMESPACE, FORMAT, ## __VA_ARGS__)
#define INFO(NAMESPACE, FORMAT, ...) LOG(LogLevel_INFO, __FILE__, __LINE__, NAMESPACE, FORMAT, ## __VA_ARGS__)
#define DEBUG(NAMESPACE, FORMAT, ...) LOG(LogLevel_DEBUG, __FILE__, __LINE__, NAMESPACE, FORMAT, ## __VA_ARGS__)
#define TRACE(NAMESPACE, FORMAT, ...) LOG(LogLevel_TRACE, __FILE__, __LINE__, NAMESPACE, FORMAT, ## __VA_ARGS__)

#define ERROR_IF(NAMESPACE, CONDITION, FORMAT, ...) LOG_IF(LogLevel_ERROR, __FILE__, __LINE__, NAMESPACE, CONDITION, FORMAT, ## __VA_ARGS__)
#define WARNING_IF(NAMESPACE, CONDITION, FORMAT, ...) LOG_IF(LogLevel_WARNING, __FILE__, __LINE__, NAMESPACE, CONDITION, FORMAT, ## __VA_ARGS__)
#define INFO_IF(NAMESPACE, CONDITION, FORMAT, ...) LOG_IF(LogLevel_INFO, __FILE__, __LINE__, NAMESPACE, CONDITION, FORMAT, ## __VA_ARGS__)
#define DEBUG_IF(NAMESPACE, CONDITION, FORMAT, ...) LOG_IF(LogLevel_DEBUG, __FILE__, __LINE__, NAMESPACE, CONDITION, FORMAT, ## __VA_ARGS__)
#define TRACE_IF(NAMESPACE, CONDITION, FORMAT, ...) LOG_IF(LogLevel_TRACE, __FILE__, __LINE__, NAMESPACE, CONDITION, FORMAT, ## __VA_ARGS__)

#endif
