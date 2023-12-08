#include <stdio.h>

#ifdef NOT_COSMO
#include <dlfcn.h>
#else
#include <cosmo.h>
#define _COSMO_SOURCE
#include<libc/dlopen/dlfcn.h>
#define dlsym cosmo_dlsym
#define dlopen cosmo_dlopen
#define dlclose cosmo_dlclose
#endif

#include "hotreload.h"

static const char *libplug_file_name = "./libplug.so";
static void *libplug = NULL;

#define PLUG(name, ...) name##_t *name = NULL;
LIST_OF_PLUGS
#undef PLUG

void* reload_libplug(void)
{
    if (libplug != NULL) dlclose(libplug);

    libplug = dlopen(libplug_file_name, RTLD_LAZY);
    if (libplug == NULL) {
        fprintf(stderr, "HOTRELOAD: could not load %s: %s", libplug_file_name, dlerror());
        return false;
    }

    #define PLUG(name, ...) \
        name = dlsym(libplug, #name); \
        if (name == NULL) { \
            fprintf(stderr, "HOTRELOAD: could not find %s symbol in %s: %s", \
                     #name, libplug_file_name, dlerror()); \
            return false; \
        }
    LIST_OF_PLUGS
    #undef PLUG

    return libplug;
}