#ifndef GLWT_glwt_linux_input_h
#define GLWT_glwt_linux_input_h

struct glwt_input_device {
    char name[32];
    int fd;
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
