#include "gl.h"
#include <unordered_map>
#include <time.h>
#include <vector>
#define GLAD_GL_IMPLEMENTATION
#define GLAD_WGL_IMPLEMENTATION
#include <glad/wgl.h>

HDC hdc;

// Start of auto generated
#pragma GCC push_options
#pragma GCC optimize ("O0")
#include "gl-inl.h"
#pragma GCC pop_options
// End of auto generated

// Start of GLX

#define XWindow int
#define XStatus int
#define XPixmap int
#define XDrawable int
#define XGC int
#define XCursor int
#define XBool int
#define GLXDrawable int
#define GLXContext int

#pragma push(pack, 1)
struct XScreen
{
    char pad[12];  // 0
    int width;     // 12
    int height;    // 16
    char pad2[60]; // 20
};

struct XColor
{
    char pad[12];
};

struct XVisual
{
    char pad[32];
};

struct XSetWindowAttributes
{
    char pad[60];
};

struct XTextProperty
{
    char pad[16];
};

struct XSizeHints
{
    char pad[72];
};

struct XWMHints
{
    char pad[36];
};

struct XClassHint
{
    char pad[8];
};

struct XVisualInfo
{
    char pad[40];
};

struct XDisplay
{
    // our own
    std::vector<HGLRC> glContext = {}; // 0

    char pad[124];    // 12
    int nscreens;     // 136
    XScreen *screens; // 140
    char pad2[2256];  // 144
};

struct GLXFBConfig
{
    int a1;
};

struct XWindowAttributes
{
    int x;
    int y;      // 4
    int width;  // 8
    int height; // 12
    char pad[76];
};
#pragma pop(pack)

GLXContext glxCurrentContext = 0;
XDisplay *glxCurrentDisplay = NULL;
GLXDrawable glxCurrentDrawable = 0;

void *jmp_glXGetProcAddress(const char *name)
{
    return gl_resolve(name);
}

int jmp_glXGetVideoSyncSGI(int *a1)
{
    *a1 = 0;
    return 0;
}

long targetFrameTime = 1000000000 / 60;;
struct timespec oldTimestamp,
    newTimestamp,
    sleepyTime,
    remainingTime;
const clockid_t clockType = CLOCK_MONOTONIC;
int jmp_glXWaitVideoSyncSGI(int a1, int a2, int *a3)
{
    if (clock_gettime(clockType, &newTimestamp) == 0)
    {
        sleepyTime.tv_nsec = targetFrameTime - newTimestamp.tv_nsec + oldTimestamp.tv_nsec;
       while( sleepyTime.tv_nsec > 0 && sleepyTime.tv_nsec < targetFrameTime ) {

            // sleep in smaller and smaller intervals
            sleepyTime.tv_nsec /= 2;
            nanosleep( &sleepyTime, &remainingTime );
            clock_gettime( clockType, &newTimestamp );
            sleepyTime.tv_nsec = targetFrameTime - newTimestamp.tv_nsec + oldTimestamp.tv_nsec;

            // For FPS == 1 this is needed as tv_nsec cannot exceed 999999999
            sleepyTime.tv_nsec += newTimestamp.tv_sec*1000000000 - oldTimestamp.tv_sec*1000000000;
        }
        clock_gettime(clockType, &oldTimestamp);
    }
    return 0;
}

XDisplay *jmp_XOpenDisplay(const char *name)
{
    XScreen *screen = new XScreen();
    memset(screen, 0, sizeof(*screen));

    screen->width = 1360;
    screen->height = 768;

    XDisplay *display = new XDisplay();
    memset(display, 0, sizeof(*display));

    display->nscreens = 1;
    display->screens = screen;

    return display;
}

XWindow jmp_XRootWindow(XDisplay *display, int screen_number)
{
    return 1;
}

XStatus jmp_XGetWindowAttributes(XDisplay *display, XWindow w, XWindowAttributes *window_attributes_return)
{
    window_attributes_return->x = 0;
    window_attributes_return->y = 0;
    window_attributes_return->width = 1360;
    window_attributes_return->height = 768;
    return 1;
}

int jmp_XCloseDisplay(XDisplay *display)
{
    return 0;
}

XStatus jmp_XInitThreads()
{
    return 1;
}

XPixmap jmp_XCreatePixmap(XDisplay *display, XDrawable d, uint32_t width, uint32_t height, uint32_t depth)
{
    return 1;
}
XGC jmp_XCreateGC(XDisplay *display, XDrawable d, uint32_t valuemask, void *values)
{
    return 1;
}
int jmp_XSetForeground(XDisplay *display, XGC gc, uint32_t foreground)
{
    return 0;
}
int jmp_XFillRectangle(XDisplay *display, XDrawable d, XGC gc, int x, int y, uint32_t width, uint32_t height)
{
    return 0;
}
int jmp_XFreeGC(XDisplay *display, XGC gc)
{
    return 0;
}
XCursor jmp_XCreatePixmapCursor(XDisplay *display, XPixmap source, XPixmap mask, XColor *foreground_color, XColor *background_color, uint32_t x, uint32_t y)
{
    return 1;
}

XWindow jmp_XCreateWindow(XDisplay *display, XWindow parent, int x, int y, uint32_t width, uint32_t height, uint32_t border_width, int depth,
                          uint32_t _class, XVisual *visual, uint32_t valuemask, XSetWindowAttributes *attributes)
{
    return 2;
}

int jmp_XSetWMProperties(XDisplay *display, XWindow w, XTextProperty *window_name, XTextProperty *icon_name, char **argv, int argc, XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints)
{
    return 0;
}

int jmp_XMapWindow(XDisplay *display, XWindow w)
{
    return 0;
}

int jmp_XRaiseWindow(XDisplay *display, XWindow w)
{
    return 0;
}

int jmp_XFlush(XDisplay *display)
{
    return 0;
}

int jmp_XFree(void *data)
{
    return 0;
}

XVisualInfo *jmp_glXChooseVisual(XDisplay *display, int screen, int *attribList)
{
    XVisualInfo *visualInfo = new XVisualInfo();
    return visualInfo;
}

HGLRC createContext() {
    return wglCreateContext(hdc);
}

GLXContext jmp_glXCreateContext(XDisplay *display, XVisualInfo *vis, GLXContext shareList, XBool direct)
{
    // create base context
    if (display->glContext.empty())
    {
        HGLRC context = createContext();
        display->glContext.push_back(context);
    }

    HGLRC context = createContext();
    display->glContext.push_back(context);

    if (!wglShareLists(display->glContext[0], context))
    {
        printf("cant share glx context\n");
    }

    return display->glContext.size();
}

XBool jmp_glXMakeCurrent(XDisplay *dpy, GLXDrawable drawable, GLXContext ctx)
{
    glxCurrentDisplay = dpy;
    glxCurrentContext = ctx;
    glxCurrentDrawable = drawable;

    wglMakeCurrent(hdc, dpy->glContext[ctx - 1]);
    return 1;
}

int jmp_glXQueryContext(XDisplay *dpy, GLXContext ctx, int attribute, int *value)
{
    return 0;
}

GLXFBConfig *jmp_glXChooseFBConfig(XDisplay *dpy, int screen, const int *attrib_list, int *nelements)
{
    GLXFBConfig *cfg = new GLXFBConfig();
    if (attrib_list[0] == 0x8013)
    {
        *nelements = 1;
        cfg->a1 = 1;
    }
    else
    {
        printf("choosefbconfig invalid attrib %d\n", attrib_list[0]);
    }
    return cfg;
}

// THESE SHOULD BE PER THREAD

XDisplay *jmp_glXGetCurrentDisplay()
{
    return glxCurrentDisplay;
}

GLXContext jmp_glXGetCurrentContext()
{
    return glxCurrentContext;
}

GLXDrawable jmp_glXGetCurrentDrawable()
{
    return glxCurrentDrawable;
}

GLXDrawable jmp_glXGetCurrentReadDrawable()
{
    return glxCurrentDrawable;
}

GLXContext jmp_glXCreateNewContext(XDisplay *dpy, GLXFBConfig config, int render_type, GLXContext share_list, XBool direct)
{
    if (render_type != 0x8014)
    {
        printf("Only rgba is supported\n");
        return 0;
    }

    HGLRC context = createContext();
    if (share_list != 0)
    {
        if (!wglShareLists(dpy->glContext[0], context))
        {
            printf("cant share glx context\n");
        }
    }
    dpy->glContext.push_back(context);
    return dpy->glContext.size();
}

void jmp_glXWaitGL()
{
}

void jmp_glXSwapBuffers(XDisplay *dpy, GLXDrawable drawable)
{
    if (!SwapBuffers(hdc))
    {
        printf("swapbuffers error\n");
    }
}

XBool jmp_glXMakeContextCurrent(XDisplay *display, GLXDrawable draw, GLXDrawable read, GLXContext context)
{
    glxCurrentDrawable = draw;
    glxCurrentContext = context;

    wglMakeCurrent(hdc, display->glContext[context - 1]);
    return 1;
}

// End of GLX

std::unordered_map<const char *, void *> GL_FUNCS_STUB = {
    {"glXGetProcAddress", (void *)jmp_glXGetProcAddress},
    {"glXGetVideoSyncSGI", (void *)jmp_glXGetVideoSyncSGI},
    {"glXWaitVideoSyncSGI", (void *)jmp_glXWaitVideoSyncSGI},
    {"XOpenDisplay", (void *)jmp_XOpenDisplay},
    {"XRootWindow", (void *)jmp_XRootWindow},
    {"XGetWindowAttributes", (void *)jmp_XGetWindowAttributes},
    {"XCloseDisplay", (void *)jmp_XCloseDisplay},
    {"XInitThreads", (void *)jmp_XInitThreads},
    {"XCreatePixmap", (void *)jmp_XCreatePixmap},
    {"XCreateGC", (void *)jmp_XCreateGC},
    {"XSetForeground", (void *)jmp_XSetForeground},
    {"XFillRectangle", (void *)jmp_XFillRectangle},
    {"XFreeGC", (void *)jmp_XFreeGC},
    {"XCreatePixmapCursor", (void *)jmp_XCreatePixmapCursor},
    {"XCreateWindow", (void *)jmp_XCreateWindow},
    {"XSetWMProperties", (void *)jmp_XSetWMProperties},
    {"XMapWindow", (void *)jmp_XMapWindow},
    {"XRaiseWindow", (void *)jmp_XRaiseWindow},
    {"XFlush", (void *)jmp_XFlush},
    {"XFree", (void *)jmp_XFree},
    {"glXChooseVisual", (void *)jmp_glXChooseVisual},
    {"glXCreateContext", (void *)jmp_glXCreateContext},
    {"glXMakeCurrent", (void *)jmp_glXMakeCurrent},
    {"glXQueryContext", (void *)jmp_glXQueryContext},
    {"glXChooseFBConfig", (void *)jmp_glXChooseFBConfig},
    {"glXGetCurrentDisplay", (void *)jmp_glXGetCurrentDisplay},
    {"glXGetCurrentContext", (void *)jmp_glXGetCurrentContext},
    {"glXGetCurrentDrawable", (void *)jmp_glXGetCurrentDrawable},
    {"glXGetCurrentReadDrawable", (void *)jmp_glXGetCurrentReadDrawable},
    {"glXCreateNewContext", (void *)jmp_glXCreateNewContext},
    {"glXWaitGL", (void *)jmp_glXWaitGL},
    {"glXSwapBuffers", (void *)jmp_glXSwapBuffers},
    {"glXMakeContextCurrent", (void *)jmp_glXMakeContextCurrent}
};

void *gl_resolve(const char *name)
{
    for (auto &it : GL_FUNCS_STUB)
    {
        if (strcmp(it.first, name) == 0)
            return it.second;
    }
    for (auto &it : GL_WRAP_FUNCS)
        if (strcmp(it.first, name) == 0)
            return it.second;
    return NULL;
}

void gl_init(void *hWnd_)
{
    HWND hWnd = (HWND)hWnd_;

    hdc = GetDC(hWnd);
    if (hdc == NULL)
    {
        printf("Failed to get window device context\n");
        DestroyWindow(hWnd);
        return;
    }
    // Set the pixel format for the device context:
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR); // Set the size of the PFD to the size of the class
    pfd.dwFlags = PFD_SUPPORT_OPENGL           // GLX_USE_GL
                  | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;  // GLX_RGBA
    pfd.iLayerType = PFD_MAIN_PLANE; // GLX_LEVEL
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;  // GLX_DEPTH_SIZE
    pfd.cStencilBits = 8; // GLX_STENCIL_SIZE
    pfd.cAuxBuffers = 4;
    int format = ChoosePixelFormat(hdc, &pfd);
    if (format == 0 || SetPixelFormat(hdc, format, &pfd) == FALSE)
    {
        ReleaseDC(hWnd, hdc);
        DestroyWindow(hWnd);

        printf("Failed to set a compatible pixel format\n");
        return;
    }

    HGLRC temp_context = wglCreateContext(hdc);
    if (temp_context == NULL)
    {
        printf("Failed to create a base wgl context\n");
        ReleaseDC(hWnd, hdc);
        DestroyWindow(hWnd);
        return;
    }
    wglMakeCurrent(hdc, temp_context);

    // Load WGL Extensions:
    gladLoaderLoadWGL(hdc);

    wglSwapIntervalEXT(0);

    int version = gladLoaderLoadGL();
    if (!version)
    {
        printf("error loading glad\n");
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(temp_context);
        ReleaseDC(hWnd, hdc);
        DestroyWindow(hWnd);
        return;
    }

    // Successfully loaded OpenGL
    printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
}