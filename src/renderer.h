// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#ifndef NEKO_renderer_H
#define NEKO_renderer_H

#include <inttypes.h>
#include "math.h"
#include "stb/stb_truetype.h"

#define WHITE (Color){1.0f, 1.0f, 1.0f, 1.0f}
#define BLACK (Color){0.0f, 0.0f, 0.0f, 1.0f}

typedef struct { float r, g, b, a; } Color;

typedef struct
{
	uint32_t id;
	int32_t  width;
	int32_t  height;
}
Image;

void renderer_init();
void renderer_frame();
void renderer_flush();

void renderer_set_image(Image i);
void renderer_set_color(Color c);

void renderer_push_mat4();
void renderer_pop_mat4();
void renderer_translate(float x, float y);
void renderer_scale(float sx, float sy);
void renderer_rotate(float r);

Image renderer_load_image(const char *filename); 
Image renderer_mem_image(int32_t width, int32_t height, const uint8_t *pixels);

void renderer_push_quad(float x1, float y1, float x2, float y2, float u0, float u1, float v0, float v1);

#endif