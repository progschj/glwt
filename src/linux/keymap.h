#ifndef GLWT_linux_keymap_h
#define GLWT_linux_keymap_h

#include <stdlib.h>
#include <limits.h>
#include <linux/input.h>

struct keymap_entry
{
    int from, to;
};

static const struct keymap_entry unsorted_keymap[] =
{
    { KEY_BACKSPACE, GLWT_KEY_BACKSPACE },
    { KEY_TAB, GLWT_KEY_TAB },
    { KEY_ENTER, GLWT_KEY_RETURN },
    { KEY_ESC, GLWT_KEY_ESCAPE },
    { KEY_SPACE, GLWT_KEY_SPACE },

    { KEY_COMMA, GLWT_KEY_COMMA },
    { KEY_MINUS, GLWT_KEY_MINUS },
    { KEY_SLASH, GLWT_KEY_SLASH },

    { KEY_0, GLWT_KEY_0 },
    { KEY_1, GLWT_KEY_1 },
    { KEY_2, GLWT_KEY_2 },
    { KEY_3, GLWT_KEY_3 },
    { KEY_4, GLWT_KEY_4 },
    { KEY_5, GLWT_KEY_5 },
    { KEY_6, GLWT_KEY_6 },
    { KEY_7, GLWT_KEY_7 },
    { KEY_8, GLWT_KEY_8 },
    { KEY_9, GLWT_KEY_9 },

    { KEY_A, GLWT_KEY_A },
    { KEY_B, GLWT_KEY_B },
    { KEY_C, GLWT_KEY_C },
    { KEY_D, GLWT_KEY_D },
    { KEY_E, GLWT_KEY_E },
    { KEY_F, GLWT_KEY_F },
    { KEY_G, GLWT_KEY_G },
    { KEY_H, GLWT_KEY_H },
    { KEY_I, GLWT_KEY_I },
    { KEY_J, GLWT_KEY_J },
    { KEY_K, GLWT_KEY_K },
    { KEY_L, GLWT_KEY_L },
    { KEY_M, GLWT_KEY_M },
    { KEY_N, GLWT_KEY_N },
    { KEY_O, GLWT_KEY_O },
    { KEY_P, GLWT_KEY_P },
    { KEY_Q, GLWT_KEY_Q },
    { KEY_R, GLWT_KEY_R },
    { KEY_S, GLWT_KEY_S },
    { KEY_T, GLWT_KEY_T },
    { KEY_U, GLWT_KEY_U },
    { KEY_V, GLWT_KEY_V },
    { KEY_W, GLWT_KEY_W },
    { KEY_X, GLWT_KEY_X },
    { KEY_Y, GLWT_KEY_Y },
    { KEY_Z, GLWT_KEY_Z },

    { KEY_DELETE, GLWT_KEY_DELETE },
    { KEY_KPSLASH, GLWT_KEY_KEYPAD_DIVIDE },
    { KEY_KPASTERISK, GLWT_KEY_KEYPAD_MULTIPLY },
    { KEY_KPPLUS, GLWT_KEY_KEYPAD_PLUS },
    { KEY_KPMINUS, GLWT_KEY_KEYPAD_MINUS },
    { KEY_KPENTER, GLWT_KEY_KEYPAD_ENTER },

    { KEY_UP, GLWT_KEY_UP },
    { KEY_DOWN, GLWT_KEY_DOWN },
    { KEY_LEFT, GLWT_KEY_LEFT },
    { KEY_RIGHT, GLWT_KEY_RIGHT },
    { KEY_PAGEUP, GLWT_KEY_PAGE_UP },
    { KEY_PAGEDOWN, GLWT_KEY_PAGE_DOWN },
    { KEY_HOME, GLWT_KEY_HOME },
    { KEY_END, GLWT_KEY_END },
    { KEY_INSERT, GLWT_KEY_INSERT },

    { KEY_F1, GLWT_KEY_F1 },
    { KEY_F2, GLWT_KEY_F2 },
    { KEY_F3, GLWT_KEY_F3 },
    { KEY_F4, GLWT_KEY_F4 },
    { KEY_F5, GLWT_KEY_F5 },
    { KEY_F6, GLWT_KEY_F6 },
    { KEY_F7, GLWT_KEY_F7 },
    { KEY_F8, GLWT_KEY_F8 },
    { KEY_F9, GLWT_KEY_F9 },
    { KEY_F10, GLWT_KEY_F10 },
    { KEY_F11, GLWT_KEY_F11 },
    { KEY_F12, GLWT_KEY_F12 },

    { KEY_LEFTSHIFT, GLWT_KEY_LSHIFT },
    { KEY_RIGHTSHIFT, GLWT_KEY_RSHIFT },
    { KEY_LEFTCTRL, GLWT_KEY_LCTRL },
    { KEY_RIGHTCTRL, GLWT_KEY_RCTRL },
    { KEY_LEFTALT, GLWT_KEY_LALT },
    { KEY_RIGHTALT, GLWT_KEY_RALT },

    { KEY_NUMLOCK, GLWT_KEY_NUM_LOCK },
    { KEY_CAPSLOCK, GLWT_KEY_CAPS_LOCK },
    { KEY_SCROLLLOCK, GLWT_KEY_SCROLL_LOCK },
};

static struct {
    int *map;
    int max_key;
    int min_key;
} keymap = {0,0,0};

static inline int keymap_init()
{
    int mappings = sizeof(unsorted_keymap)/sizeof(struct keymap_entry);
    keymap.max_key = INT_MIN;
    keymap.min_key = INT_MAX;
    for(int i = 0; i<mappings; ++i)
    {
        if(unsorted_keymap[i].from > keymap.max_key)
            keymap.max_key = unsorted_keymap[i].from;

        if(unsorted_keymap[i].from < keymap.min_key)
            keymap.min_key = unsorted_keymap[i].from;
    }

    keymap.map = malloc(sizeof(int)*(keymap.max_key-keymap.min_key+1));

    if(keymap.map == 0)
        return -1;

    memset(keymap.map, GLWT_KEY_UNKNOWN, keymap.max_key-keymap.min_key+1);

    for(int i = 0; i<mappings; ++i)
    {
        keymap.map[unsorted_keymap[i].from-keymap.min_key] = unsorted_keymap[i].to;
    }

    return 0;
}

static inline void keymap_free()
{
    free(keymap.map);
}

static inline int keymap_lookup(int key)
{
    if(key > keymap.max_key || key < keymap.min_key)
        return GLWT_KEY_UNKNOWN;
    return keymap.map[key-keymap.min_key];
}

#endif
