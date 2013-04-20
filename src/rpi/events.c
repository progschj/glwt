#include <glwt_internal.h>

int glwtEventHandle(int wait)
{
    return glwtEventHandleLinux(glwt.rpi.win, wait);
}
