// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#ifndef NEKO_MATH_H
#define NEKO_MATH_H

typedef union
{
	struct { float x; float y; };
	struct { float u; float v; };
	float e[2];
}
vec2;

typedef struct
{
	float m0, m4, m8, m12;
	float m1, m5, m9, m13;
	float m2, m6, m10, m14;
	float m3, m7, m11, m15;
}
mat4;

mat4 math_mat4_mul(mat4 a, mat4 b);
mat4 math_mat4_identity();
mat4 math_mat4_ortho(float left, float right, float bottom, float top, float near, float far);
mat4 math_mat4_transpose(mat4 m);

#endif