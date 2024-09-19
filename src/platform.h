#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void platform_initialize_arguments(int argc, char ** argv);

void platform_initialize(void);

int platform_argc(void);

const char * platform_argv(int index);

void platform_printf(const char * format, ...);

void platform_run(void);

void platform_schedule(void (*f)(void));
void platform_schedule_interval(bool (*f)(void), uint64_t milliseconds);

struct platform_File;
size_t platform_File_size(void);

enum {
  platform_File_APPEND     = 1 << 0,
  platform_File_CREATE     = 1 << 1,
  platform_File_TRUNCATE   = 1 << 2,
  platform_File_READ       = 1 << 3,
  platform_File_WRITE      = 1 << 4,
  platform_File_READ_WRITE = platform_File_READ | platform_File_WRITE,
};

void platform_File_open(struct platform_File * file, const char * path, int flags, void (* result)(struct platform_File *, bool));
bool platform_File_open_synchronous(struct platform_File * file, const char * path, int flags);

void platform_File_write(struct platform_File * file, int64_t offset, const void * data, size_t size, void (* result)(struct platform_File *, bool, uint64_t));
bool platform_File_write_synchronous(struct platform_File * file, int64_t offset, const void * data, size_t size, uint64_t *out_written);

void platform_File_read(struct platform_File * file, int64_t offset, void * data, size_t size, void (* result)(struct platform_File *, bool, uint64_t));
bool platform_File_read_synchronous(struct platform_File * file, int64_t offset, void * data, size_t size, uint64_t *out_read);

void platform_File_close(struct platform_File * file, void (* result)(struct platform_File *, bool));
bool platform_File_close_synchronous(struct platform_File * file);

void platform_file_exists(const char * path, void (* result)(bool));
bool platform_file_exists_synchronous(const char * path);
