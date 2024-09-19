#include "platform.h"
#include "configuration.h"
#include "format.h"

#include <stdarg.h>

#include <uv.h>

#define SOURCE_NAMESPACE core.platform

static int _argc;
static char ** _argv;
static uv_loop_t _loop;

static CONFIGURATION_INTEGER(SOURCE_NAMESPACE.libuv, threadpool_size, 8); 

void platform_initialize_arguments(int argc, char ** argv) {
  _argc = argc;
  _argv = uv_setup_args(argc, argv);
}

void platform_initialize(void) {
  if(threadpool_size == 0) {
    threadpool_size = uv_available_parallelism();
  }

  char buf[16];
  format_string(buf, 16, "%u", threadpool_size);
  uv_os_setenv("UV_THREADPOOL_SIZE", buf);

  uv_loop_init(&_loop);
}

int platform_argc(void) {
  return _argc;
}

const char * platform_argv(int index) {
  return index < _argc ? _argv[index] : NULL;
}

void platform_printf(const char * format, ...) {
  va_list ap;
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
}

void platform_run(void) {
  uv_run(&_loop, UV_RUN_DEFAULT);
}

static void _free_uv_timer(uv_handle_t * timer) {
  memory_free(timer, sizeof(uv_timer_t), alignof(uv_timer_t));
}

static void _schedule_cb(uv_timer_t * timer) {
  void (*f)(void) = (void (*)(void))timer->data;
  f();
  uv_close((uv_handle_t *)timer, _free_uv_timer);
}

static void _schedule_interval_cb(uv_timer_t * timer) {
  bool (*f)(void) = (bool (*)(void))timer->data;
  if(!f()) {
    uv_close((uv_handle_t *)timer, _free_uv_timer);
  }
}

void platform_schedule(void (*f)(void)) {
  uv_timer_t * timer = memory_new(uv_timer_t);
  timer->data = f;
  uv_timer_init(&_loop, timer);
  uv_timer_start(timer, _schedule_cb, 0, 0);
}

void platform_schedule_interval(bool (*f)(void), uint64_t milliseconds) {
  uv_timer_t * timer = memory_new(uv_timer_t);
  timer->data = f;
  uv_timer_init(&_loop, timer);
  uv_timer_start(timer, _schedule_interval_cb, 0, milliseconds);
}

struct platform_File {
  uv_fs_t req;
  uv_file file;
  void * user_data;
  union {
    struct {
      void (* result)(struct platform_File * file, bool ok);
    } open, close;
    struct {
      void (* result)(struct platform_File * file, bool ok, uint64_t bytes_written);
    } write;
    struct {
      void (* result)(struct platform_File * file, bool ok, uint64_t bytes_read);
    } read;
  };
};

size_t platform_File_size(void) {
  return sizeof(struct platform_File);
}

// file open
static void open_callback(uv_fs_t * req) {
  struct platform_File * file = (struct platform_File *)uv_req_get_data((uv_req_t *)req);
  if(req->result > 0) {
    file->file = req->result;
    file->open.result(file, true);
  } else {
    file->open.result(file, false);
  }
}

static inline int convert_open_flags(int _flags) {
  int flags =
    (_flags & platform_File_APPEND     ? UV_FS_O_APPEND : 0) |
    (_flags & platform_File_CREATE     ? UV_FS_O_CREAT  : 0) |
    (_flags & platform_File_TRUNCATE   ? UV_FS_O_TRUNC  : 0) |
    (
      _flags & platform_File_READ_WRITE ? UV_FS_O_RDWR   : 
      _flags & platform_File_WRITE      ? UV_FS_O_WRONLY : 
      _flags & platform_File_READ       ? UV_FS_O_RDONLY : 0
    ) ;
  return flags;
}

void platform_File_open(struct platform_File * file, const char * path, int _flags, void (* result)(struct platform_File *, bool)) {
  int flags = convert_open_flags(_flags);
  file->open.result = result;
  uv_req_set_data((uv_req_t *)&file->req, file);
  uv_fs_open(&_loop, &file->req, path, flags, 0640, open_callback);
}

bool platform_File_open_synchronous(struct platform_File * file, const char * path, int _flags) {
  int flags = convert_open_flags(_flags);
  uv_req_set_data((uv_req_t *)&file->req, file);
  int result = uv_fs_open(&_loop, &file->req, path, flags, 0640, NULL);
  if(result >= 0) {
    file->file = result;
    return true;
  }
  return false;
}

// file write
static void write_callback(uv_fs_t * req) {
  struct platform_File * file = (struct platform_File *)uv_req_get_data((uv_req_t *)req);
  if(req->result >= 0) {
    file->write.result(file, true, req->result);
  } else {
    file->write.result(file, false, 0);
  }
}

void platform_File_write(struct platform_File * file, int64_t offset, const void * data, size_t size, void (* result)(struct platform_File *, bool, uint64_t)) {
  uv_buf_t buf = uv_buf_init((char *)data, size);
  file->write.result = result;
  uv_req_set_data((uv_req_t *)&file->req, file);
  uv_fs_write(&_loop, &file->req, file->file, &buf, 1, offset, write_callback);
}

bool platform_File_write_synchronous(struct platform_File * file, int64_t offset, const void * data, size_t size, uint64_t * out_size) {
  uv_buf_t buf = uv_buf_init((char *)data, size);
  uv_req_set_data((uv_req_t *)&file->req, file);
  int result = uv_fs_write(&_loop, &file->req, file->file, &buf, 1, offset, NULL);
  if(result >= 0) {
    if(out_size != NULL) {
      *out_size = result;
    }
    return true;
  }
  if(out_size != NULL) {
    *out_size = 0;
  }
  return false;
}


// file read
static void read_callback(uv_fs_t * req) {
  struct platform_File * file = (struct platform_File *)uv_req_get_data((uv_req_t *)req);
  if(req->result >= 0) {
    file->read.result(file, true, req->result);
  } else {
    file->read.result(file, false, 0);
  }
}

void platform_File_read(struct platform_File * file, int64_t offset, void * data, size_t size, void (* result)(struct platform_File *, bool, uint64_t)) {
  uv_buf_t buf = uv_buf_init((char *)data, size);
  file->read.result = result;
  uv_req_set_data((uv_req_t *)&file->req, file);
  uv_fs_read(&_loop, &file->req, file->file, &buf, 1, offset, read_callback);
}

bool platform_File_read_synchronous(struct platform_File * file, int64_t offset, void * data, size_t size, uint64_t * out_size) {
  uv_buf_t buf = uv_buf_init((char *)data, size);
  uv_req_set_data((uv_req_t *)&file->req, file);
  int result = uv_fs_read(&_loop, &file->req, file->file, &buf, 1, offset, NULL);
  if(result >= 0) {
    if(out_size != NULL) {
      *out_size = result;
    }
    return true;
  }
  if(out_size != NULL) {
    *out_size = 0;
  }
  return false;
}

// file close
static void close_callback(uv_fs_t * req) {
  struct platform_File * file = (struct platform_File *)uv_req_get_data((uv_req_t *)req);
  if(req->result >= 0) {
    file->close.result(file, true);
  } else {
    file->close.result(file, false);
  }
}

void platform_File_close(struct platform_File * file, void (* result)(struct platform_File *, bool)) {
  file->close.result = result;
  uv_req_set_data((uv_req_t *)&file->req, file);
  uv_fs_close(&_loop, &file->req, file->file, close_callback);
}

bool platform_File_close_synchronous(struct platform_File * file) {
  uv_req_set_data((uv_req_t *)&file->req, file);
  return uv_fs_close(&_loop, &file->req, file->file, NULL) >= 0;
}

