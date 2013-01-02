#include <GLWT/glwt.h>
#include <glwt_internal.h>

#include <linux/keymap.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/epoll.h>
#include <sys/inotify.h>

static int add_device(const char *name)
{
    int fd = open(name, O_RDONLY | O_NONBLOCK);

    if(fd < 0)
        return -1;

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;

    epoll_ctl(glwt.linux_input.epoll_fd, EPOLL_CTL_ADD, fd, &event);

    glwt.linux_input.devices = realloc(glwt.linux_input.devices, sizeof(struct glwt_input_device)*(glwt.linux_input.device_count+1));

    glwt.linux_input.devices[glwt.linux_input.device_count].fd = fd;
    strncpy(glwt.linux_input.devices[glwt.linux_input.device_count].name, name, 10);
    glwt.linux_input.device_count++;

    return 0;
}

static int find_device(const char *name)
{
    int i = 0;
    for(; i<glwt.linux_input.device_count; ++i)
    {
        if(strcmp(glwt.linux_input.devices[i].name, name) == 0)
            break;
    }

    if(i<glwt.linux_input.device_count)
        return i;
    else
        return -1;
}

static void remove_device(const char *name)
{
    int i = find_device(name);

    if(i<0)
        return;

    epoll_ctl(glwt.linux_input.epoll_fd, EPOLL_CTL_DEL, glwt.linux_input.devices[i].fd, 0);
    close(glwt.linux_input.devices[i].fd);
    memcpy(glwt.linux_input.devices+i, glwt.linux_input.devices+i+1, sizeof(struct glwt_input_device)*(glwt.linux_input.device_count-i-1));
    glwt.linux_input.device_count--;

}

int glwtInitLinux()
{
    memset(&glwt.linux_input, 0, sizeof(glwt.linux_input));
    glwt.linux_input.epoll_fd = epoll_create1(0);
    if(glwt.linux_input.epoll_fd < 0)
    {
        glwtErrorPrintf("failed to create epoll");
        goto error;
    }

    DIR *dir;
    dir = opendir("/dev/input/");
    if(dir == 0)
        goto error;

    struct dirent *entry;

    while((entry = readdir(dir)))
    {
        if(strncmp(entry->d_name, "event", 5) == 0)
        {
            char name[20];
            snprintf(name, sizeof(name), "/dev/input/%s", entry->d_name);
            add_device(name);
        }
    }

    closedir(dir);

    glwt.linux_input.inotify_fd = inotify_init1(IN_NONBLOCK);

    inotify_add_watch(glwt.linux_input.inotify_fd, "/dev/input/", IN_CREATE | IN_DELETE | IN_ATTRIB);

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = glwt.linux_input.inotify_fd;

    epoll_ctl(glwt.linux_input.epoll_fd, EPOLL_CTL_ADD, glwt.linux_input.inotify_fd, &event);

    if(keymap_init() < 0)
        goto error;

    return 0;
error:
    glwtQuitLinux();
    return -1;
}

void glwtQuitLinux()
{
    keymap_free();

    if(glwt.linux_input.inotify_fd > 0)
        close(glwt.linux_input.inotify_fd);

    if(glwt.linux_input.epoll_fd > 0)
        close(glwt.linux_input.epoll_fd);

    for(int i = 0; i<glwt.linux_input.device_count; ++i)
        close(glwt.linux_input.devices[i].fd);

    free(glwt.linux_input.devices);
    memset(&glwt.linux_input, 0, sizeof(struct glwt_linux_input));
}

static void glwtHandleRelEvent(GLWTWindow *win, struct input_event event)
{
    GLWTWindowEvent wevent;
    switch(event.code)
    {
    case REL_X:
        glwt.linux_input.mouse.x += event.value;
        if(win->win_callback)
        {
            wevent.type = GLWT_WINDOW_MOUSE_MOTION;
            wevent.motion.buttons = glwt.linux_input.mouse.buttons;
            wevent.motion.x = glwt.linux_input.mouse.x;
            wevent.motion.y = glwt.linux_input.mouse.y;

            win->win_callback(
                win,
                &wevent,
                win->userdata
            );
        }
        break;
    case REL_Y:
        glwt.linux_input.mouse.y += event.value;
	if(win->win_callback)
        {
            wevent.type = GLWT_WINDOW_MOUSE_MOTION;
            wevent.motion.buttons = glwt.linux_input.mouse.buttons;
            wevent.motion.x = glwt.linux_input.mouse.x;
            wevent.motion.y = glwt.linux_input.mouse.y;
        
            win->win_callback(
                win,
                &wevent,
                win->userdata
            );
        }
        break;
    default:
        return;
    }
}

static void glwtHandleAbsEvent(GLWTWindow *win, struct input_event event)
{

}


static void glwtUpdateMod(int key, int value)
{
    int mod = 0;
    switch(key)
    {
    case GLWT_KEY_LSHIFT:
    case GLWT_KEY_RSHIFT:
        mod = GLWT_MOD_SHIFT;
        break;
    case GLWT_KEY_LCTRL:
    case GLWT_KEY_RCTRL:
        mod = GLWT_MOD_CTRL;
        break;
    case GLWT_KEY_LALT:
    case GLWT_KEY_RALT:
        mod = GLWT_MOD_ALT;
        break;
    case GLWT_KEY_LSUPER:
    case GLWT_KEY_RSUPER:
        mod = GLWT_MOD_SUPER;
        break;
    case GLWT_KEY_ALTGR:
        mod = GLWT_MOD_ALTGR;
        break;
    case GLWT_KEY_NUM_LOCK:
        mod = GLWT_MOD_NUM_LOCK;
        break;
    case GLWT_KEY_CAPS_LOCK:
        mod = GLWT_MOD_CAPS_LOCK;
        break;
    }
    if(mod != 0)
        if(value)
            glwt.linux_input.keyboard.mod |= mod;
        else
            glwt.linux_input.keyboard.mod &= ~mod;

}

static void glwtHandleKeyEvent(GLWTWindow *win, struct input_event event)
{
    int key = keymap_lookup(event.code);

    glwtUpdateMod(key, event.value);

    if(key != GLWT_KEY_UNKNOWN)
    {
        if(win && win->win_callback)
        {
            GLWTWindowEvent wevent;
            wevent.type = event.value==0 ? GLWT_WINDOW_KEY_UP : GLWT_WINDOW_KEY_DOWN;
            wevent.key.scancode = key;
            wevent.key.mod = glwt.linux_input.keyboard.mod;

            win->win_callback(
                win,
                &wevent,
                win->userdata
            );
        }
    }
    else if((unsigned)(event.code-BTN_MOUSE)<8)
    {
        if(win && win->win_callback)
        {
            GLWTWindowEvent wevent;
            wevent.type = event.value==0 ? GLWT_WINDOW_BUTTON_UP : GLWT_WINDOW_BUTTON_DOWN;
            wevent.button.button = event.code-BTN_MOUSE;
            wevent.button.mod = glwt.linux_input.keyboard.mod;
            wevent.button.x = glwt.linux_input.mouse.x;
            wevent.button.y = glwt.linux_input.mouse.y;
 
            win->win_callback(
                win,
                &wevent,
                win->userdata
            );
	}
        if(event.value)
            glwt.linux_input.mouse.buttons |= 1<<(event.code-BTN_MOUSE);
        else
            glwt.linux_input.mouse.buttons &= ~(1<<(event.code-BTN_MOUSE));
    }

}

static void glwtHandleInputEvent(GLWTWindow *win, struct epoll_event *ep_event)
{
    struct input_event event;

    while(read(ep_event->data.fd, &event, sizeof(event)) > 0)
    {
        switch(event.type)
        {
        case EV_KEY:
            glwtHandleKeyEvent(win, event);
            break;
        case EV_ABS:
            glwtHandleAbsEvent(win, event);
            break;
        case EV_REL:
            glwtHandleRelEvent(win, event);
            break;

        }
    }
}

static void glwtHandleInotifyEvent(struct epoll_event *ep_event)
{
    char buffer[256];

    while(1)
    {
        int length = read(ep_event->data.fd, buffer, sizeof(buffer));
        if(length<=0)
            return;
        int pos = 0;
        while(pos<length)
        {
            struct inotify_event *event = (struct inotify_event*)(buffer+pos);

            if(strncmp(event->name, "event", 5) == 0)
            {
                char name[20];
                snprintf(name, sizeof(name), "/dev/input/%s", event->name);

                if(event->mask & (IN_CREATE | IN_ATTRIB))
                {
                    if(find_device(name)<0)
                        add_device(name);
                }
                else if(event->mask & IN_DELETE)
                {
                    remove_device(name);
                }
            }

            pos += sizeof(struct inotify_event) + event->len;
        }
    }

}


#define GLWT_EPOLL_EVENT_BATCH_SIZE 16
int glwtEventHandleLinux(GLWTWindow *win, int wait)
{
    struct epoll_event events[GLWT_EPOLL_EVENT_BATCH_SIZE];
    int count;
    while((count = epoll_wait(glwt.linux_input.epoll_fd, events, GLWT_EPOLL_EVENT_BATCH_SIZE, -wait)) > 0)
    {
        for(int i = 0; i<count; ++i)
        {
            if(events[i].data.fd == glwt.linux_input.inotify_fd)
                glwtHandleInotifyEvent(events+i);
            else
                glwtHandleInputEvent(win, events+i);

        }
        if(wait != 0) break;
    }
    return 0;
}
