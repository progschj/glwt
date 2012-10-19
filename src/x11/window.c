#include <stdlib.h>

#include <glwt_internal.h>

GLWTWindow *glwtWindowCreate(
    const char *title,
    int width, int height,
    const GLWTWindowCallbacks *win_callbacks,
    GLWTWindow *share)
{
    GLWTWindow *win = calloc(1, sizeof(GLWTWindow));
    if(!win)
        return 0;
    
    win->x11.transparentCursor = 0;

    (void)title; // TODO: set window title
    if(win_callbacks)
        win->win_callbacks = *win_callbacks;

    XSetWindowAttributes attrib;
    attrib.colormap = glwt.x11.colormap;
    attrib.event_mask = 0
        | StructureNotifyMask
        | PointerMotionMask
        | ButtonPressMask
        | ButtonReleaseMask
        | KeyPressMask
        | KeyReleaseMask
        | EnterWindowMask
        | LeaveWindowMask
        | FocusChangeMask
        | ExposureMask;
    unsigned long attrib_mask = CWColormap | CWEventMask;

    win->x11.window = XCreateWindow(
        glwt.x11.display,
        RootWindow(glwt.x11.display, glwt.x11.screen_num),
        0, 0, width, height,
        0,
        glwt.x11.depth,
        InputOutput,
        glwt.x11.visual,
        attrib_mask,
        &attrib);

    Atom protocols[] = {
        glwt.x11.atoms.WM_DELETE_WINDOW,
        glwt.x11.atoms._NET_WM_PING,
    };
    int num_protocols = sizeof(protocols)/sizeof(*protocols);
    if(XSetWMProtocols(glwt.x11.display, win->x11.window, protocols, num_protocols) == 0)
    {
        glwtErrorPrintf("XSetWMProtocols failed");
        goto error;
    }

#ifdef GLWT_USE_EGL
    if(glwtWindowCreateEGL(win, share) != 0)
#else
    if(glwtWindowCreateGLX(win, share) != 0)
#endif
        goto error;

    if(XSaveContext(glwt.x11.display, win->x11.window, glwt.x11.xcontext, (XPointer)win) != 0)
    {
        glwtErrorPrintf("XSaveContext failed");
        goto error;
    }

    return win;
error:
    glwtWindowDestroy(win);
    return 0;
}

void glwtWindowDestroy(GLWTWindow *win)
{
    if(!win)
        return;

    if(XDeleteContext(glwt.x11.display, win->x11.window, glwt.x11.xcontext) != 0)
        glwtErrorPrintf("XDeleteContext failed");

#ifdef GLWT_USE_EGL
    glwtWindowDestroyEGL(win);
#else
    glwtWindowDestroyGLX(win);
#endif

    if(win->x11.window)
        XDestroyWindow(glwt.x11.display, win->x11.window);

    free(win);
}

void glwtWindowShow(GLWTWindow *win, int show)
{
    if(show)
        XMapRaised(glwt.x11.display, win->x11.window);
    else
        XUnmapWindow(glwt.x11.display, win->x11.window);
    XFlush(glwt.x11.display);
}

void glwtGrabPointer(GLWTWindow *win, int grab)
{
    unsigned int mask 
        = ButtonPressMask 
        | ButtonReleaseMask 
        | PointerMotionMask 
        | FocusChangeMask 
        | EnterWindowMask 
        | LeaveWindowMask;
        
    if(grab)
    {
        int err = XGrabPointer(glwt.x11.display, win->x11.window, True, mask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        switch(err)
        {
            case BadCursor:
                glwtErrorPrintf("XGrabPointer failed (BadCursor)");
                return;
            case BadValue:
                glwtErrorPrintf("XGrabPointer failed (BadValue)");
                return;
            case BadWindow:
                glwtErrorPrintf("XGrabPointer failed (BadWindow)");
                return;
            default: // OK
                return;
        }
    }
    else
        XUngrabPointer(glwt.x11.display, CurrentTime);
}

void createTransparentCursor(GLWTWindow *win)
{
    static const char bitmap_data[] = { 0 };
    Pixmap pixmap = XCreateBitmapFromData(glwt.x11.display, win->x11.window, bitmap_data, 1, 1);
    if(pixmap == None)
        glwtErrorPrintf("failed to create pixmap for empty cursor");
    
    XColor black;
    black.pixel = BlackPixel(glwt.x11.display, glwt.x11.screen_num);
    XQueryColor(glwt.x11.display, glwt.x11.colormap, &black);
            
    win->x11.transparentCursor = XCreatePixmapCursor(glwt.x11.display, pixmap, pixmap, &black, &black, 0, 0);
    XFreePixmap(glwt.x11.display, pixmap);

    if(!win->x11.transparentCursor)
        glwtErrorPrintf("failed to create empty cursor");

}

void glwtShowPointer(GLWTWindow *win, int show)
{
    if(!show)
    {
        if(win->x11.transparentCursor == 0)
            createTransparentCursor(win);
        XDefineCursor(glwt.x11.display, win->x11.window, win->x11.transparentCursor);
    }
    else
    {
        XUndefineCursor(glwt.x11.display, win->x11.window);
    }
}

void glwtWarpPointer(GLWTWindow *win, int x, int y)
{
    if(XWarpPointer(glwt.x11.display, None, win->x11.window, 0, 0, 0, 0, x, y) == BadWindow)
        glwtErrorPrintf("failed to warp pointer (BadWindow)"); 
}
