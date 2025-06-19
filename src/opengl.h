// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#ifndef NEKO_OPENGL_H
#define NEKO_OPENGL_H

#include <inttypes.h>

#ifndef GL_DEBUG
#define GL_DEBUG 0
#endif
#define GL_MAJOR 3
#define GL_MINOR 3

#if defined(__linux__) || defined(__unix__)
#include <GL/glx.h>

// GLX extension constants
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define GLX_CONTEXT_FLAGS_ARB 0x2094
#define GLX_CONTEXT_DEBUG_BIT_ARB 0x0001

// Make sure we have GLX 1.3+ constants
#ifndef GLX_SAMPLE_BUFFERS
#define GLX_SAMPLE_BUFFERS 0x186a0
#endif
#ifndef GLX_SAMPLES
#define GLX_SAMPLES 0x186a1
#endif

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_ALPHA_BITS_ARB 0x201B

typedef const char * (*PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC hdc);
typedef BOOL (*PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int32_t* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int32_t* piFormats, UINT* nNumFormats);
typedef HGLRC (*PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int32_t* attribList);
typedef BOOL (*PFNWGLSWAPINTERVALEXTPROC)(int32_t int32_terval);

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
#endif

// OpenGL constants
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_BYTE 0x1401
#define GL_ALPHA 0x1906
#define GL_RGBA 0x1908
#define GL_RGBA8 0x8058
#define GL_RED 0x1903
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_DEPTH_TEST 0x0B71
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_REPEAT 0x2901
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_TRIANGLES 0x0004
#define GL_LINK_STATUS 0x8B82
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_VERSION 0x1F02
#define GL_COMPILE_STATUS 0x8B81
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNSIGNED_INT 0x1405
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_LINEAR 0x2601
#define GL_NEAREST 0x2600
#define GL_ARRAY_BUFFER_BINDING 0x8894

// OpenGL type definitions
typedef uint32_t GLenum;
typedef uint32_t GLuint32;
typedef uint8_t GLboolean;
typedef uint32_t GLbitfield;
typedef char GLchar;
typedef int32_t GLint;
typedef int32_t GLsizei;
typedef uint32_t GLuint;
typedef uintptr_t GLsizeiptr;
typedef uintptr_t GLintptr;
typedef float GLfloat;
typedef uint8_t GLubyte;

// OpenGL 1.0 functions (available through system OpenGL)
void glEnable(GLenum cap);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClear(GLbitfield mask);
void glDisable(GLenum cap);
void glBindTexture(GLenum target, GLuint texture);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
const GLubyte *glGetString(GLenum name);
void glActiveTexture(GLenum texture);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glGenTextures(GLsizei n, GLuint *textures);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void glDeleteTextures(GLsizei n, const GLuint *textures);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);

// OpenGL 3.3+ function pointer types
typedef void (*PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef GLuint (*PFNGLCREATESHADERPROGRAMVPROC)(GLenum type, GLsizei count, const GLchar* const* strings);
typedef void (*PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (*PFNGLDEBUGMESSAGECALLBACKPROC)(void* callback, const void* userParam);
typedef void (*PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (*PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (*PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void (*PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef GLuint (*PFNGLCREATESHADERPROC)(GLenum type);
typedef void (*PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void (*PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void (*PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint (*PFNGLCREATEPROGRAMPROC)(void);
typedef void (*PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (*PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (*PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (*PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (*PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (*PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (*PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (*PFNGLGENERATEMIPMAPPROC)(GLenum target);
typedef void (*PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (*PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef GLint (*PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);

// Macro to define all OpenGL function pointers
#define GL_FUNCTIONS(X) \
	X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
	X(PFNGLCREATESHADERPROGRAMVPROC, glCreateShaderProgramv) \
	X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
	X(PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback) \
	X(PFNGLATTACHSHADERPROC, glAttachShader) \
	X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
	X(PFNGLSHADERSOURCEPROC, glShaderSource) \
	X(PFNGLCOMPILESHADERPROC, glCompileShader) \
	X(PFNGLCREATESHADERPROC, glCreateShader) \
	X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
	X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
	X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
	X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
	X(PFNGLDELETESHADERPROC, glDeleteShader) \
	X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
	X(PFNGLBINDBUFFERPROC, glBindBuffer) \
	X(PFNGLBUFFERDATAPROC, glBufferData) \
	X(PFNGLGENBUFFERSPROC, glGenBuffers) \
	X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
	X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
	X(PFNGLUSEPROGRAMPROC, glUseProgram) \
	X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
	X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap) \
	X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
	X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \
	X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)

// Declare all OpenGL function pointers
#define X(type, name) extern type name;
GL_FUNCTIONS(X)
#undef X

#endif