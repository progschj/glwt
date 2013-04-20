#ifndef GLWT_linux_keymap_h
#define GLWT_linux_keymap_h

#include <stdlib.h>
#include <limits.h>
#include <linux/input.h>

static inline int keymap_lookup(int key)
{
    switch(key)
    {
        case KEY_BACKSPACE: return GLWT_KEY_BACKSPACE;
        case KEY_TAB: return GLWT_KEY_TAB;
        case KEY_ENTER: return GLWT_KEY_RETURN;
        case KEY_ESC: return GLWT_KEY_ESCAPE;
        case KEY_SPACE: return GLWT_KEY_SPACE;

        case KEY_COMMA: return GLWT_KEY_COMMA;
        case KEY_MINUS: return GLWT_KEY_MINUS;
        case KEY_SLASH: return GLWT_KEY_SLASH;

        case KEY_0: return GLWT_KEY_0;
        case KEY_1: return GLWT_KEY_1;
        case KEY_2: return GLWT_KEY_2;
        case KEY_3: return GLWT_KEY_3;
        case KEY_4: return GLWT_KEY_4;
        case KEY_5: return GLWT_KEY_5;
        case KEY_6: return GLWT_KEY_6;
        case KEY_7: return GLWT_KEY_7;
        case KEY_8: return GLWT_KEY_8;
        case KEY_9: return GLWT_KEY_9;

        case KEY_A: return GLWT_KEY_A;
        case KEY_B: return GLWT_KEY_B;
        case KEY_C: return GLWT_KEY_C;
        case KEY_D: return GLWT_KEY_D;
        case KEY_E: return GLWT_KEY_E;
        case KEY_F: return GLWT_KEY_F;
        case KEY_G: return GLWT_KEY_G;
        case KEY_H: return GLWT_KEY_H;
        case KEY_I: return GLWT_KEY_I;
        case KEY_J: return GLWT_KEY_J;
        case KEY_K: return GLWT_KEY_K;
        case KEY_L: return GLWT_KEY_L;
        case KEY_M: return GLWT_KEY_M;
        case KEY_N: return GLWT_KEY_N;
        case KEY_O: return GLWT_KEY_O;
        case KEY_P: return GLWT_KEY_P;
        case KEY_Q: return GLWT_KEY_Q;
        case KEY_R: return GLWT_KEY_R;
        case KEY_S: return GLWT_KEY_S;
        case KEY_T: return GLWT_KEY_T;
        case KEY_U: return GLWT_KEY_U;
        case KEY_V: return GLWT_KEY_V;
        case KEY_W: return GLWT_KEY_W;
        case KEY_X: return GLWT_KEY_X;
        case KEY_Y: return GLWT_KEY_Y;
        case KEY_Z: return GLWT_KEY_Z;

        case KEY_DELETE: return GLWT_KEY_DELETE;
        case KEY_KPSLASH: return GLWT_KEY_KEYPAD_DIVIDE;
        case KEY_KPASTERISK: return GLWT_KEY_KEYPAD_MULTIPLY;
        case KEY_KPPLUS: return GLWT_KEY_KEYPAD_PLUS;
        case KEY_KPMINUS: return GLWT_KEY_KEYPAD_MINUS;
        case KEY_KPENTER: return GLWT_KEY_KEYPAD_ENTER;

        case KEY_UP: return GLWT_KEY_UP;
        case KEY_DOWN: return GLWT_KEY_DOWN;
        case KEY_LEFT: return GLWT_KEY_LEFT;
        case KEY_RIGHT: return GLWT_KEY_RIGHT;
        case KEY_PAGEUP: return GLWT_KEY_PAGE_UP;
        case KEY_PAGEDOWN: return GLWT_KEY_PAGE_DOWN;
        case KEY_HOME: return GLWT_KEY_HOME;
        case KEY_END: return GLWT_KEY_END;
        case KEY_INSERT: return GLWT_KEY_INSERT;

        case KEY_F1: return GLWT_KEY_F1;
        case KEY_F2: return GLWT_KEY_F2;
        case KEY_F3: return GLWT_KEY_F3;
        case KEY_F4: return GLWT_KEY_F4;
        case KEY_F5: return GLWT_KEY_F5;
        case KEY_F6: return GLWT_KEY_F6;
        case KEY_F7: return GLWT_KEY_F7;
        case KEY_F8: return GLWT_KEY_F8;
        case KEY_F9: return GLWT_KEY_F9;
        case KEY_F10: return GLWT_KEY_F10;
        case KEY_F11: return GLWT_KEY_F11;
        case KEY_F12: return GLWT_KEY_F12;

        case KEY_LEFTSHIFT: return GLWT_KEY_LSHIFT;
        case KEY_RIGHTSHIFT: return GLWT_KEY_RSHIFT;
        case KEY_LEFTCTRL: return GLWT_KEY_LCTRL;
        case KEY_RIGHTCTRL: return GLWT_KEY_RCTRL;
        case KEY_LEFTALT: return GLWT_KEY_LALT;
        case KEY_RIGHTALT: return GLWT_KEY_RALT;

        case KEY_NUMLOCK: return GLWT_KEY_NUM_LOCK;
        case KEY_CAPSLOCK: return GLWT_KEY_CAPS_LOCK;
        case KEY_SCROLLLOCK: return GLWT_KEY_SCROLL_LOCK;
        default: break;
    }
    return GLWT_KEY_UNKNOWN;
}

#endif
