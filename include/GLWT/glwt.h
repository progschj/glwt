#ifndef GLWT_glwt_h
#define GLWT_glwt_h

#include <GLWT/glwt_keysym.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GLWT_API_ANY                    0x00
#define GLWT_API_OPENGL                 0x01
#define GLWT_API_OPENGL_ES              0x02
#define GLWT_API_MASK                   0x0f

#define GLWT_PROFILE_CORE               0x00
#define GLWT_PROFILE_COMPATIBILITY      0x10
#define GLWT_PROFILE_DEBUG              0x20
#define GLWT_PROFILE_ROBUSTNESS         0x40
#define GLWT_PROFILE_MASK               0xf0

typedef struct GLWTConfig
{
    int red_bits, green_bits, blue_bits, alpha_bits;
    int depth_bits, stencil_bits;
    int samples, sample_buffers;
    int api;
    int api_version_major, api_version_minor;
} GLWTConfig;

typedef struct GLWTWindow GLWTWindow;

typedef struct GLWTWindowEvent
{
    GLWTWindow *window;

    enum {
        GLWT_WINDOW_NO_EVENT = 0,
        GLWT_WINDOW_CLOSE,
        GLWT_WINDOW_EXPOSE,
        GLWT_WINDOW_RESIZE,
        GLWT_WINDOW_SHOW,
        GLWT_WINDOW_HIDE,
        GLWT_WINDOW_FOCUS_IN,
        GLWT_WINDOW_FOCUS_OUT,
        GLWT_WINDOW_KEY_UP,
        GLWT_WINDOW_KEY_DOWN,
        GLWT_WINDOW_BUTTON_UP,
        GLWT_WINDOW_BUTTON_DOWN,
        GLWT_WINDOW_MOUSE_MOTION,
        GLWT_WINDOW_MOUSE_ENTER,
        GLWT_WINDOW_MOUSE_LEAVE,
    } type;

    union {
        struct { int width, height; } resize;
        struct { int keysym, scancode, mod; } key;
        struct { int x, y, buttons; } motion;
        struct { int x, y, button, mod; } button;
        struct { int dummy; } dummy;
    };
} GLWTWindowEvent;

typedef struct GLWTAppEvent
{
    int deviceID;

    enum {
        GLWT_APP_BUTTON_UP,
        GLWT_APP_BUTTON_DOWN,
        GLWT_APP_KEY_UP,
        GLWT_APP_KEY_DOWN,
        GLWT_APP_AXIS_RELATIVE_MOTION,
        GLWT_APP_AXIS_ABSOLUTE_MOTION,
        GLWT_APP_INPUT_CONNECT,
        GLWT_APP_INPUT_DISCONNECT,
    } type;

    union {
        struct { int button; } button;
        struct { int keysym, scancode, mod; } key;
        struct { int axis, value; } relative;
        struct { int axis, value, min, max; } absolute;
        struct { int dummy; } dummy;
    };
} GLWTAppEvent;

int glwtInit(
    const GLWTConfig *config,
    void (*error_callback)(const char *msg, void *userdata),
    void (*app_callback)(const GLWTAppEvent *event, void *userdata),
    void *userdata
    );
void glwtQuit();

GLWTWindow *glwtWindowCreate(
    const char *title,
    int width, int height,
    GLWTWindow *share,
    void (*win_callback)(GLWTWindow *window, const GLWTWindowEvent *event, void *userdata),
    void *userdata
    );
void glwtWindowDestroy(GLWTWindow *window);

int glwtWindowClosed(GLWTWindow *window);
void glwtWindowShow(GLWTWindow *window, int show);
void glwtWindowSetTitle(GLWTWindow *window, const char *title);

void glwtMakeCurrent(GLWTWindow *win);
void glwtSwapBuffers(GLWTWindow *win);
void glwtSwapInterval(GLWTWindow *win, int interval);
void glwtWindowGetSize(GLWTWindow *win, int *width, int *height);

int glwtEventHandle(int wait);

#ifdef __cplusplus
}
#endif

#endif
