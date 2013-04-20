#include <GLWT/glwt.h>
#include <glwt_internal.h>

#include <linux/keymap.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <errno.h>

static void glwtHandlePlugEvent(struct glwt_input_device *device);
static void glwtHandleUnplugEvent(struct glwt_input_device *device);

static inline int get_bit(uint32_t *bits, uint32_t bit)
{
   return (bits[bit/32] >> (bit%32)) & 1;
}
static void find_device_capabilities(int fd, struct glwt_device_capabilities *capabilities)
{
    uint32_t types[EV_MAX];
    uint32_t events[(KEY_MAX-1)/32+1];

    // query supported event types
    memset(types, 0, sizeof(types));
    ioctl(fd, EVIOCGBIT(0, EV_MAX), types);

    capabilities->axes = 0;
    if(get_bit(types, EV_ABS))
    {
        memset(events, 0, sizeof(events));
        ioctl(fd, EVIOCGBIT(EV_ABS, KEY_MAX), events);

        capabilities->abs_axes = 0;
        capabilities->axis_indices = 0;
        for(int j=0;j<ABS_MAX;++j)
        capabilities->abs_axes += get_bit(events, j);
        if(capabilities->abs_axes > 0)
        {
            capabilities->axes = malloc(sizeof(struct glwt_abs_axis)*capabilities->abs_axes);
            capabilities->axis_indices = malloc(sizeof(int)*ABS_MAX);

            int i = 0;
            for(int j=0;j<ABS_MAX;++j)
            {
                capabilities->axis_indices[j] = ABS_MAX;
                if(get_bit(events, j))
                {
                    struct input_absinfo abs;
                    ioctl(fd, EVIOCGABS(j), &abs);
                    capabilities->axes[i].index = j;
                    capabilities->axes[i].min = abs.minimum;
                    capabilities->axes[i].max = abs.maximum;
                    capabilities->axis_indices[j] = i;
                    ++i;
                }
            }
        }
    }

    if(get_bit(types, EV_REL))
    {
        memset(events, 0, sizeof(events));
        ioctl(fd, EVIOCGBIT(EV_REL, KEY_MAX), events);

        capabilities->rel_axes = 0;
        for(int j=0;j<32;++j)
            capabilities->rel_axes += get_bit(events, j);
    }

    if(get_bit(types, EV_KEY))
    {
        memset(events, 0, sizeof(events));
        ioctl(fd, EVIOCGBIT(EV_KEY, KEY_MAX), events);

        capabilities->total_keys = 0;
        capabilities->keyboard_keys = 0;
        capabilities->mouse_buttons = 0;
        capabilities->joystick_buttons = 0;
        capabilities->gamepad_buttons = 0;
        for(int j=0;j<KEY_MAX;++j)
        {
            capabilities->total_keys += get_bit(events, j);
            if(get_bit(events, j))
            {
                if(j<BTN_MOUSE)
                    capabilities->keyboard_keys += 1;
                else if(j<BTN_JOYSTICK)
                    capabilities->mouse_buttons += 1;
                else if(j<BTN_GAMEPAD)
                    capabilities->joystick_buttons += 1;
                else if(j<BTN_DIGI)
                    capabilities->gamepad_buttons += 1;
            }
        }
    }
}

static int add_device(const char *name)
{
    int fd = open(name, O_RDONLY | O_NONBLOCK);

    if(fd < 0)
    {
        if((errno == EACCES || errno == ENOENT))
            return 0;
        else
        {
            glwtErrorPrintf("failed opening device %s", name);
            return -1;
        }
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;

    epoll_ctl(glwt.linux_input.epoll_fd, EPOLL_CTL_ADD, fd, &event);

    struct glwt_input_device *device = malloc(sizeof(struct glwt_input_device));

    device->fd = fd;
    strncpy(device->name, name, 32);
    ioctl(fd, EVIOCGNAME(128), device->device_name);

    find_device_capabilities(fd, &(device->capabilities));

    fprintf(stderr, "%s\n", device->device_name);

    device->next = glwt.linux_input.devices;

    glwt.linux_input.devices = device;

    glwt.linux_input.device_count++;

    glwtHandlePlugEvent(device);

    return 0;
}

static struct glwt_input_device* find_device_by_name(const char *name)
{
    struct glwt_input_device *current = glwt.linux_input.devices;
    while(current != 0)
    {
        if(strcmp(current->name, name) == 0)
            return current;
        else
            current = current->next;
    }
    return 0;
}

static struct glwt_input_device* find_device_by_fd(int fd)
{
    struct glwt_input_device *current = glwt.linux_input.devices;
    while(current != 0)
    {
        if(current->fd == fd)
            return current;
        else
            current = current->next;
    }
    return 0;
}

static int remove_device(const char *name)
{
    struct glwt_input_device **prev = &glwt.linux_input.devices;
    struct glwt_input_device *device = glwt.linux_input.devices;
    while(device != 0)
    {
        if(strcmp(device->name, name) == 0)
            break;
        else
        {
            prev = &(device->next);
            device = device->next;
        }
    }

    if(device == 0)
    {
        glwtErrorPrintf("removing unknown device %s", name);
        return -1;
    }

    glwtHandleUnplugEvent(device);

    if(epoll_ctl(glwt.linux_input.epoll_fd, EPOLL_CTL_DEL, device->fd, 0) != 0)
    {
        glwtErrorPrintf("failed to remove device %s from epoll", name);
        return -1;
    }
    /*
    if(close(device->fd) != 0)
    {
        int err = errno;
        glwtErrorPrintf("failed to close file descriptor %s %i", name, err);
        return -1;
    }
    */
    *prev = device->next;
    free(device);

    glwt.linux_input.device_count--;
    return 0;
}

static void free_devices()
{
    struct glwt_input_device *current = glwt.linux_input.devices;
    while(current != 0)
    {
        struct glwt_input_device *next = current->next;
        free(current->capabilities.axes);
        free(current->capabilities.axis_indices);
        free(current);
        current = next;
    }
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
            if(add_device(name) < 0)
                goto error;
        }
    }

    closedir(dir);

    glwt.linux_input.inotify_fd = inotify_init1(IN_NONBLOCK);

    inotify_add_watch(glwt.linux_input.inotify_fd, "/dev/input/", IN_CREATE | IN_DELETE | IN_ATTRIB);

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = glwt.linux_input.inotify_fd;

    epoll_ctl(glwt.linux_input.epoll_fd, EPOLL_CTL_ADD, glwt.linux_input.inotify_fd, &event);

    return 0;
error:
    glwtQuitLinux();
    return -1;
}

void glwtQuitLinux()
{
    if(glwt.linux_input.inotify_fd > 0)
        close(glwt.linux_input.inotify_fd);

    if(glwt.linux_input.epoll_fd > 0)
        close(glwt.linux_input.epoll_fd);

    for(int i = 0; i<glwt.linux_input.device_count; ++i)
        close(glwt.linux_input.devices[i].fd);

    free_devices();
    memset(&glwt.linux_input, 0, sizeof(struct glwt_linux_input));
}

static void glwtHandleRelEvent(GLWTWindow *win, struct input_event event)
{
    GLWTWindowEvent wevent;
    switch(event.code)
    {
    case REL_X:
        glwt.linux_input.mouse.x += event.value;
        if(win && win->win_callback)
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
        if(win && win->win_callback)
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
    (void)win; (void)event;
}

static void glwtHandlePlugEvent(struct glwt_input_device *device)
{
    (void)device;
}

static void glwtHandleUnplugEvent(struct glwt_input_device *device)
{
    (void)device;
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
    {
        if(value)
            glwt.linux_input.keyboard.mod |= mod;
        else
            glwt.linux_input.keyboard.mod &= ~mod;
    }
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
    else if(BTN_JOYSTICK <= event.code && event.code < BTN_GAMEPAD)
    {
        // joystick button
    }
    else if(BTN_GAMEPAD <= event.code && event.code < BTN_DIGI)
    {
        // gamepad button
    }

}

static int glwtHandleInputEvent(GLWTWindow *win, struct epoll_event *ep_event)
{
    struct input_event event;

    int result = 0;

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

    return result;
}

static int glwtHandleHangup(struct epoll_event *ep_event)
{
    if(ep_event->data.fd == glwt.linux_input.inotify_fd)
    {
        glwtErrorPrintf("inotify hung up\n");
        goto error;
    }

    struct glwt_input_device *device = find_device_by_fd(ep_event->data.fd);

    if(device != 0)
    {
        remove_device(device->name);
    }
    return 0;
error:
    return -1;
}

static int glwtHandleInotifyEvent(struct epoll_event *ep_event)
{
    char buffer[256];

    while(1)
    {
        int length = read(ep_event->data.fd, buffer, sizeof(buffer));
        if(length<=0)
            break;
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
                    if(find_device_by_name(name) == 0)
                        if(add_device(name) != 0)
                            goto error;
                }
            }

            pos += sizeof(struct inotify_event) + event->len;
        }
    }
    return 0;
error:
    return -1;
}


#define GLWT_EPOLL_EVENT_BATCH_SIZE 16
int glwtEventHandleLinux(GLWTWindow *win, int wait)
{
    int result = 0;
    struct epoll_event events[GLWT_EPOLL_EVENT_BATCH_SIZE];
    int count;
    while((count = epoll_wait(glwt.linux_input.epoll_fd, events, GLWT_EPOLL_EVENT_BATCH_SIZE, -wait)) > 0)
    {
        for(int i = 0; i<count; ++i)
        {
            if((events[i].events & EPOLLHUP) != 0)
                result = glwtHandleHangup(events+i);
            else if(events[i].data.fd == glwt.linux_input.inotify_fd)
                result = glwtHandleInotifyEvent(events+i);
            else
                result = glwtHandleInputEvent(win, events+i);

            if(result != 0) break;
        }
        if(wait != 0 || result != 0) break;
    }
    return result;
}
