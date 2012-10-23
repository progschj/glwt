#include <glwt_internal.h>

int glwtEventHandle(int wait)
{
    glwtEventHandleLinux(glwt.rpi.win, wait);
    return 0;
}
