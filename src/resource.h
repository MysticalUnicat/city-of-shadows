#pragma once

#include "string.h"

bool resource_load_pack(CStr path, bool overwrite);

struct Reader;

struct Reader * resource_read(CStr path);
