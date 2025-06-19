// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include <assert.h>

#include "system.h"
#include "opengl.h"

#define X(type, name) type name;
GL_FUNCTIONS(X)
#undef X

static struct
{
	HINSTANCE instance;
	HWND      win_handler;
	HDC       device_ctx;
	HGLRC     gl_ctx;
}
self = { 0 };


// Display a message box with an error text
void system_panic(const char* message) {
	MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
	ExitProcess(0);
}

// NOTE(ellora): I only use that thing to convert char window title to WSTR
// and I'm not even freeing that shit lol.
static LPWSTR utf8_to_utf16(const char* utf8) {
	int32_t len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	LPWSTR utf16 = calloc(len, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, len);

	return utf16;
}

#ifdef GL_DEBUG
// Cool way to handle opengl debug errors using a debugger (or not)
static void debug_callback(
	GLenum source, GLenum type, GLuint32 id, GLenum severity,
	GLsizei length, const GLchar* message, const void* user)
{
	(void)source;
	(void)type;
	(void)id;
	(void)length;
	(void)user;

	OutputDebugStringA(message);
	OutputDebugStringA("\n");
	if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) {
		if (IsDebuggerPresent()) {
			assert(!"OpenGL error - check the callstack in debugger");
		}
		system_panic(message);
	}
}
#endif

// That is where all our window messages will be processed
static LRESULT CALLBACK win_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(wnd, msg, wparam, lparam);
}

// To use modern functions from OpenGL we need to create
// a fake window to load the wgl function than finally load
// all gl desire functions through wgl calls.
static void load_gl( void ) {
	HWND dummy = CreateWindowExW(
		0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);
	assert(dummy && "Failed to create dummy window");

	HDC dc = GetDC(dummy);
	assert(dc && "Failed to get device context for dummy window");

	PIXELFORMATDESCRIPTOR desc = {
		.nSize = sizeof(desc),
		.nVersion = 1,
		.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		.iPixelType = PFD_TYPE_RGBA,
		.cColorBits = 24,
	};

	int32_t format = ChoosePixelFormat(dc, &desc);
	if (!format) {
		system_panic("Cannot choose OpenGL pixel format for dummy window!");
	}

	int32_t ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
	assert(ok && "Failed to describe OpenGL pixel format");

	if (!SetPixelFormat(dc, format, &desc)) {
		system_panic("Cannot set OpenGL pixel format for dummy window!");
	}

	HGLRC rc = wglCreateContext(dc);
	assert(rc && "Failed to create OpenGL context for dummy window");

	ok = wglMakeCurrent(dc, rc);
	assert(ok && "Failed to make current OpenGL context for dummy window");

	// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
	wglChoosePixelFormatARB = (void*)wglGetProcAddress("wglChoosePixelFormatARB");
	// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
	wglCreateContextAttribsARB = (void*)wglGetProcAddress("wglCreateContextAttribsARB");
	// https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
	wglSwapIntervalEXT = (void*)wglGetProcAddress("wglSwapIntervalEXT");

	if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT) {
		system_panic("OpenGL does not support required WGL extensions for modern context!");
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(rc);
	ReleaseDC(dummy, dc);
	DestroyWindow(dummy);
}

void system_create_window(int32_t width, int32_t height, const char *name) {
	assert(self.win_handler == NULL && "Window already created");

	// Register window class to have custom WindowProc callback
	WNDCLASSEXW wc =
	{
		.cbSize = sizeof(wc),
		.lpfnWndProc = win_proc,
		.hInstance = self.instance,
		.hIcon = LoadIcon(NULL, IDI_APPLICATION),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = L"opengl_window_class",
	};
	ATOM atom = RegisterClassExW(&wc);
	assert(atom && "Failed to register window class");

	// Create window
	DWORD exstyle = WS_EX_APPWINDOW;
	DWORD style = WS_OVERLAPPEDWINDOW;

	self.win_handler = CreateWindowExW(
		exstyle, wc.lpszClassName, utf8_to_utf16(name), style,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, wc.hInstance, NULL);
	assert(self.win_handler && "Failed to create window");

	// Lets create an OpenGL context
	self.device_ctx = GetDC(self.win_handler);
	assert(self.device_ctx && "Failed to window device context");

	{
		int32_t attrib[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
			WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB,     24,
			WGL_ALPHA_BITS_ARB,     8,
			WGL_DEPTH_BITS_ARB,     24,
			WGL_STENCIL_BITS_ARB,   8,

			// uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
			//WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

			// uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
			//WGL_SAMPLE_BUFFERS_ARB, 1,
			//WGL_SAMPLES_ARB,        4, // 4x MSAA
			0,
		};

		int32_t format;
		UINT formats;
		if (!wglChoosePixelFormatARB(self.device_ctx, attrib, NULL, 1, &format, &formats) || formats == 0) {
			system_panic("OpenGL does not support required pixel format!");
		}

		PIXELFORMATDESCRIPTOR desc = { .nSize = sizeof(desc) };
		int32_t ok = DescribePixelFormat(self.device_ctx, format, sizeof(desc), &desc);
		assert(ok && "Failed to describe OpenGL pixel format");

		if (!SetPixelFormat(self.device_ctx, format, &desc)) {
			system_panic("Cannot set OpenGL selected pixel format!");
		}
	}

	{
		int attrib[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, GL_MAJOR,
			WGL_CONTEXT_MINOR_VERSION_ARB, GL_MINOR,
			WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef GL_DEBUG
			// Ask for debug context for non "Release" builds
			// this is so we can enable debug callback
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
			0,
		};

		HGLRC rc = wglCreateContextAttribsARB(self.device_ctx, NULL, attrib);
		if (!rc) {
			system_panic("Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");
		}
		self.gl_ctx = rc;

		BOOL ok = wglMakeCurrent(self.device_ctx, rc);
		assert(ok && "Failed to make current OpenGL context");

		// Load OpenGL functions
#define X(type, name) name = (type)wglGetProcAddress(#name); assert(name);
		GL_FUNCTIONS(X)
#undef X

#ifdef GL_DEBUG
		// Enable debug callback
		glDebugMessageCallback(&debug_callback, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
	}

	// Show the window
	wglSwapIntervalEXT(true);
	ShowWindow(self.win_handler, SW_SHOWDEFAULT);
}

bool system_window_should_close() {
	MSG msg;
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return true;
		}
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return false;
}

vec2 system_window_size() {
	RECT rect;
	GetClientRect(self.win_handler, &rect);
	int32_t width = rect.right - rect.left;
	int32_t height = rect.bottom - rect.top;

	return (vec2){ .x = (float)width, .y = (float)height };
}

bool system_window_is_visible() {
	vec2 size = system_window_size();
	return size.x != 0 && size.y != 0;
}

void system_swap_buffers() {
	if (!SwapBuffers(self.device_ctx)) {
		system_panic("Failed to swap OpenGL buffers!");
	}
}

void system_sleep(uint32_t miliseconds) {
	Sleep(miliseconds);
}

void *system_load_file(const char *filename) {
	HANDLE file = CreateFileA(
		filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	LARGE_INTEGER file_size;
	if (!GetFileSizeEx(file, &file_size)) {
		CloseHandle(file);
		return NULL;
	}

	void *data = malloc(file_size.QuadPart);
	if (!data) {
		CloseHandle(file);
		return NULL;
	}

	DWORD read;
	if (!ReadFile(file, data, file_size.QuadPart, &read, NULL)) {
		CloseHandle(file);
		free(data);
		return NULL;
	}

	CloseHandle(file);
	return data;
}

void system_close_window() {
	DestroyWindow(self.win_handler);
	self.win_handler = NULL;
}


// Entry point defined by the user
int32_t entry_point ( void );

int32_t WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int32_t show) {
	(void)prev;
	(void)cmd;
	(void)show;

	load_gl();
	self.instance = instance;
	return entry_point();
}