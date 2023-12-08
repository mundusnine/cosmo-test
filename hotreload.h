#pragma once

#include "plug.h"
#include <stdbool.h>
//@TODO: Add config.h
// #include "config.h"
#define HOTRELOAD
#ifdef HOTRELOAD
    #define PLUG(name, ...) extern name##_t *name;
    LIST_OF_PLUGS
    #undef PLUG
    void* reload_libplug(void);
#else
    #define PLUG(name, ...) name##_t name;
    LIST_OF_PLUGS
    #undef PLUG
    #define reload_libplug() true
#endif // HOTRELOAD