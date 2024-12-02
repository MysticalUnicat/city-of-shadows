#include "read.h"
#include "memory.h"
#include "string.h"
#include "format.h"
#include "platform.h"

struct Reader {
  enum {
    Reader_file,
    Reader_resource
  } kind;
  union {
    struct {
      struct platform_File * platform;
      uint64_t offset;
    } file;
    struct {
      struct platform_File * source_file;
      uint64_t start, end;
      uint64_t offset;
    } resource;
  };
};

struct Reader * read_from_file(const char * path) {
  struct Reader * result = memory_alloc(sizeof(*result) + platform_File_size(), alignof(*result));
  result->file.platform = (struct platform_File *)(result + 1);
  if(!platform_File_open_synchronous(result->file.platform, path, platform_File_READ)) {
    return NULL;
  }
  return result;
}

struct Reader * read_from_pack(struct platform_File * source_file, uint64_t start, uint64_t length) {
  struct Reader * result = memory_alloc(sizeof(*result), alignof(*result));
  result->resource.source_file = source_file;
  result->resource.start = start;
  result->resource.end = start + length;
  result->resource.offset = start;
  return result;
}

static inline int64_t do_read_synchronous(struct Reader * r, void * buffer, size_t size) {
  uint64_t written_size = 0;
  switch(r->kind) {
  case Reader_file:
    if(platform_File_read_synchronous(r->file.platform, r->file.offset, buffer, size, &written_size)) {
      r->file.offset += written_size;
    }
    break;
  case Reader_resource:
    if(r->resource.offset + size > r->resource.end) {
      size = r->resource.end - r->resource.offset;
    }
    if(platform_File_read_synchronous(r->resource.source_file, r->resource.offset, buffer, size, &written_size)) {
      r->resource.offset += written_size;
    }
    break;
  default:
    break;
  }
  return written_size;
}

size_t read_buffer(struct Reader * r, void * buffer, size_t size) {
  return do_read_synchronous(r, buffer, size);
}

int read_getc(struct Reader * r) {
  char c = 0;
  return do_read_synchronous(r, &c, 1) > 0 ? c : -1;
}

uint32_t read_uint32(struct Reader * r) {
  uint32_t value;
  do_read_synchronous(r, &value, sizeof(value));
  return value;
}

MStr read_string(struct Reader * r) {
  uint8_t byte;
  MStr result = string_allocate(0);
  for(;;) {
    do_read_synchronous(r, &byte, 1);
    if(byte != 0) {
      result = string_append(result, byte);
    }
  }
  return result;
}

MStr read_string_size_prefixed(struct Reader * r) {
  uint32_t size = read_uint32(r);
  MStr result = string_allocate(size);
  do_read_synchronous(r, result, size);
  return result;
}

void read_finish(struct Reader * r) {
  size_t size = sizeof(*r);
  switch(r->kind) {
  case Reader_file:
    platform_File_close_synchronous(r->file.platform);
    size += platform_File_size();
    break;
  case Reader_resource:
    break;
  default:
    break;
  }
  memory_free(r, size, alignof(*r));
}

