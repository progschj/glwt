#ifndef GLWT_glwt_linux_input_h
#define GLWT_glwt_linux_input_h

struct glwt_abs_axis {
    int index, min, max;
};

struct glwt_device_capabilities {
    int total_keys;
    int keyboard_keys;
    int gamepad_buttons;
    int joystick_buttons;
    int mouse_buttons;
    int rel_axes;
    int abs_axes;
    struct glwt_abs_axis *axes;
    int *axis_indices;
};

struct glwt_input_device {
    char name[32];
    int fd;
    char device_name[128];
    struct glwt_device_capabilities capabilities;
    struct glwt_input_device *next;
};

struct glwt_linux_input
{
    int epoll_fd;
    int inotify_fd;
    struct glwt_input_device *devices;
    int device_count;
    struct {
        int x, y;
        int buttons;
    } mouse;
    struct {
        int mod;
    } keyboard;
};

int glwtInitLinux();
void glwtQuitLinux();
int glwtEventHandleLinux(GLWTWindow *win, int wait);


#endif
