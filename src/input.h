#pragma once

#include "renderer.h"

typedef struct {
    float x,y;
    int l,m,r;
} mouse_t;

int mouse_on_rect(mouse_t mouse,RenRect rect);

// FENSTER_KEYCODES[124] 
//= {XK_apostrophe,39,XK_backslash,92,XK_bracketleft,91,XK_bracketright,93,XK_comma,44,XK_equal,61,XK_grave,96,XK_minus,45,XK_period,46,XK_semicolon,59,XK_slash,47,XK_space,32,XK_0,48,XK_1,49,XK_2,50,XK_3,51,XK_4,52,XK_5,53,XK_6,54,XK_7,55,XK_8,56,XK_9,57};
// @TODO: Add Function keys to fenster.
enum KEY {
    KEY_HOME = 2,
    KEY_PAGE_UP = 3,
    KEY_PAGE_DOWN = 4,
    KEY_END = 5,
    KEY_BACK = 8,
    KEY_TAB = 9,
    KEY_RET = 10,
    KEY_DEL = 127,
    KEY_UP = 17,
    KEY_DOWN = 18,
    KEY_RIGHT = 19,
    KEY_LEFT = 20,
    KEY_INS = 26,
    KEY_ESC = 27,
    KEY_A = 65,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z
};

#ifdef INPUT_IMPL
int mouse_on_rect(mouse_t mouse,RenRect rect){
    return mouse.x > rect.x && mouse.x < rect.x + rect.width && mouse.y > rect.y && mouse.y < rect.y + rect.height;
}
#endif