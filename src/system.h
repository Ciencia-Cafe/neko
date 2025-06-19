// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#ifndef NEKO_system_H
#define NEKO_system_H

#include "math.h"
#include <stdbool.h>
#include <inttypes.h>

void  system_create_window(int32_t width, int32_t height, const char *name);
void  system_sleep(uint32_t miliseconds);
void  system_close_window();
bool  system_window_should_close();
vec2  system_window_size();
bool  system_window_is_visible();
void  system_swap_buffers();
void  system_panic(const char *msg);
void *system_load_file(const char *filename);

#endif