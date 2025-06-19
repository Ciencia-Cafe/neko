// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "system.h"
#include "opengl.h"

#define X(type, name) type name;
GL_FUNCTIONS(X)
#undef X

static struct {
    Display *display;
    Window window;
    int screen;
    GLXContext gl_context;
    XVisualInfo *visual_info;
    Colormap colormap;
    Atom wm_delete_window;
    bool should_close;
    int width, height;
} self = {0};

// Display an error message and exit
void system_panic(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

#ifdef GL_DEBUG
// OpenGL debug callback for X11
static void debug_callback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* user)
{
    (void)source;
    (void)type;
    (void)id;
    (void)length;
    (void)user;

    fprintf(stderr, "OpenGL Debug: %s\n", message);
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) {
        system_panic(message);
    }
}
#endif

// Load OpenGL functions using glXGetProcAddress
static void load_gl_functions(void) {
#define X(type, name) \
    name = (type)glXGetProcAddress((const GLubyte*)#name); \
    if (!name) { \
        fprintf(stderr, "Failed to load OpenGL function: %s\n", #name); \
        system_panic("Failed to load required OpenGL functions"); \
    }
    GL_FUNCTIONS(X)
#undef X

#ifdef GL_DEBUG
    // Enable debug callback if available
    if (glDebugMessageCallback) {
        glDebugMessageCallback(&debug_callback, NULL);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
#endif
}

void system_create_window(int32_t width, int32_t height, const char *name) {
    assert(self.display == NULL && "Window already created");

    // Open X11 display
    self.display = XOpenDisplay(NULL);
    if (!self.display) {
        system_panic("Failed to open X11 display");
    }

    self.screen = DefaultScreen(self.display);
    self.width = width;
    self.height = height;

    // GLX attributes for modern OpenGL context
    int glx_attribs[] = {
        GLX_X_RENDERABLE,    True,
        GLX_DRAWABLE_TYPE,   GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,     GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE,   GLX_TRUE_COLOR,
        GLX_RED_SIZE,        8,
        GLX_GREEN_SIZE,      8,
        GLX_BLUE_SIZE,       8,
        GLX_ALPHA_SIZE,      8,
        GLX_DEPTH_SIZE,      24,
        GLX_STENCIL_SIZE,    8,
        GLX_DOUBLEBUFFER,    True,
        None
    };

    // Get framebuffer configs
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(self.display, self.screen, glx_attribs, &fbcount);
    if (!fbc || fbcount == 0) {
        system_panic("Failed to retrieve framebuffer config");
    }

    // Pick the FB config/visual with the most samples per pixel
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
    for (int i = 0; i < fbcount; i++) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(self.display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(self.display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(self.display, fbc[i], GLX_SAMPLES, &samples);

            if ((best_fbc < 0 || samp_buf) && samples > best_num_samp) {
                best_fbc = i;
                best_num_samp = samples;
            }
            if ((worst_fbc < 0 || !samp_buf) && samples < worst_num_samp) {
                worst_fbc = i;
                worst_num_samp = samples;
            }
        }
        XFree(vi);
    }

    GLXFBConfig bestFbc = fbc[best_fbc];
    XFree(fbc);

    // Get visual info from the selected framebuffer config
    self.visual_info = glXGetVisualFromFBConfig(self.display, bestFbc);
    if (!self.visual_info) {
        system_panic("Failed to get XVisualInfo");
    }

    // Create colormap
    self.colormap = XCreateColormap(
        self.display,
        RootWindow(self.display, self.screen),
        self.visual_info->visual,
        AllocNone
    );

    // Create window
    XSetWindowAttributes swa;
    swa.colormap = self.colormap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | 
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                     StructureNotifyMask;

    self.window = XCreateWindow(
        self.display,
        RootWindow(self.display, self.screen),
        0, 0, width, height, 0,
        self.visual_info->depth,
        InputOutput,
        self.visual_info->visual,
        CWColormap | CWEventMask,
        &swa
    );

    if (!self.window) {
        system_panic("Failed to create X11 window");
    }

    // Set window title
    XStoreName(self.display, self.window, name);
    XSetIconName(self.display, self.window, name);

    // Handle window close button
    self.wm_delete_window = XInternAtom(self.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(self.display, self.window, &self.wm_delete_window, 1);

    // Create OpenGL context
    // First check if we can create a modern context
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
        glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

    if (glXCreateContextAttribsARB) {
        int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, GL_MAJOR,
            GLX_CONTEXT_MINOR_VERSION_ARB, GL_MINOR,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef GL_DEBUG
            GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
            None
        };

        self.gl_context = glXCreateContextAttribsARB(
            self.display, bestFbc, 0, True, context_attribs
        );
    }

    // Fallback to legacy context creation
    if (!self.gl_context) {
        self.gl_context = glXCreateContext(self.display, self.visual_info, 0, GL_TRUE);
    }

    if (!self.gl_context) {
        system_panic("Failed to create OpenGL context");
    }

    // Make context current
    if (!glXMakeCurrent(self.display, self.window, self.gl_context)) {
        system_panic("Failed to make OpenGL context current");
    }

    // Load OpenGL functions
    load_gl_functions();

    // Show window
    XMapWindow(self.display, self.window);

    // Enable VSync if available
    typedef int (*glXSwapIntervalEXTProc)(Display*, GLXDrawable, int);
    glXSwapIntervalEXTProc glXSwapIntervalEXT = 
        (glXSwapIntervalEXTProc)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(self.display, self.window, 1);
    }

    XFlush(self.display);
    self.should_close = false;
}

bool system_window_should_close() {
    if (!self.display) return true;

    // Process pending X11 events
    while (XPending(self.display)) {
        XEvent event;
        XNextEvent(self.display, &event);

        switch (event.type) {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == self.wm_delete_window) {
                    self.should_close = true;
                }
                break;
            case ConfigureNotify:
                // Update window size
                self.width = event.xconfigure.width;
                self.height = event.xconfigure.height;
                break;
            case DestroyNotify:
                self.should_close = true;
                break;
        }
    }

    return self.should_close;
}

vec2 system_window_size() {
    return (vec2){ .x = (float)self.width, .y = (float)self.height };
}

bool system_window_is_visible() {
    return self.width > 0 && self.height > 0 && !self.should_close;
}

void system_swap_buffers() {
    if (self.display && self.window) {
        glXSwapBuffers(self.display, self.window);
    }
}

void system_sleep(uint32_t milliseconds) {
    usleep(milliseconds * 1000);
}

void *system_load_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 0) {
        fclose(file);
        return NULL;
    }

    // Allocate memory and read file
    void *data = malloc(size + 1); // +1 for null terminator if needed
    if (!data) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(data, 1, size, file);
    fclose(file);

    if (read_size != (size_t)size) {
        free(data);
        return NULL;
    }

    // Add null terminator for text files
    ((char*)data)[size] = '\0';
    return data;
}

void system_close_window() {
    if (self.gl_context) {
        glXMakeCurrent(self.display, None, NULL);
        glXDestroyContext(self.display, self.gl_context);
        self.gl_context = NULL;
    }

    if (self.window) {
        XDestroyWindow(self.display, self.window);
        self.window = 0;
    }

    if (self.colormap) {
        XFreeColormap(self.display, self.colormap);
        self.colormap = 0;
    }

    if (self.visual_info) {
        XFree(self.visual_info);
        self.visual_info = NULL;
    }

    if (self.display) {
        XCloseDisplay(self.display);
        self.display = NULL;
    }

    self.should_close = true;
}

// Entry point defined by the user
int32_t entry_point(void);

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    return entry_point();
}