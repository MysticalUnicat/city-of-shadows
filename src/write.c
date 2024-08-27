#include "write.h"
#include "memory.h"
#include "string.h"
#include "format.h"
#include "platform.h"

struct Writer {
  enum {
    Writer_file,
  } kind;
  union {
    struct {
      struct platform_File * platform;
      uint64_t offset;
    } file;
  };
};

struct Writer * write_to_file(const char * path, bool create) {
  struct Writer * result = memory_alloc(sizeof(*result) + platform_File_size(), alignof(*result));
  result->file.platform = (struct platform_File *)(result + 1);
  result->file.offset = 0;
  if(!platform_File_open_synchronous(result->file.platform, path, (create ? platform_File_CREATE : 0) | platform_File_WRITE | platform_File_TRUNCATE)) {
    return NULL;
  }
  result->kind = Writer_file;
  return result;
}

static inline int64_t do_write(struct Writer * w, const void * buffer, size_t size) {
  uint64_t written_size = 0;
  switch(w->kind) {
  case Writer_file:
    if(platform_File_write_synchronous(w->file.platform, w->file.offset, buffer, size, &written_size)) {
      w->file.offset += written_size;
    }
    break;
  default:
    break;
  }
}

void write_uint32(struct Writer * w, uint32_t value) {
  do_write(w, &value, sizeof(value));
}

void write_string(struct Writer * w, const char * string) {
  size_t size = string_size(string);
  do_write(w, string, size);
}

void write_string_size_prefixed(struct Writer * w, const char * string) {
  size_t size = string_size(string);
  write_uint32(w, size);
  do_write(w, string, size);
}

void write_format_v(struct Writer * w, const char * format, va_list ap) {
  MStr string = format_alloc_v(format, ap);
  write_string(w, string);
  string_free(string);
}

uint64_t write_hole(struct Writer * w, size_t size) {
  uint64_t result;
  switch(w->kind) {
  case Writer_file:
    result = w->file.offset;
    w->file.offset += size;
    break;
  default:
    break;
  }
}

void write_fill(struct Writer * w, uint64_t hole, const void * data, size_t data_size) {
  uint64_t written_size = 0;
  switch(w->kind) {
  case Writer_file:
    platform_File_write_synchronous(w->file.platform, hole, data, data_size, &written_size);
    break;
  default:
    break;
  }
}

void write_finish(struct Writer * w) {
  size_t size = sizeof(*w);
  switch(w->kind) {
  case Writer_file:
    platform_File_close_synchronous(w->file.platform);
    size += platform_File_size();
    break;
  default:
    break;
  }
  memory_free(w, size, alignof(*w));
}

