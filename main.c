#include <stdio.h>
#include <stdlib.h>
#ifdef NOT_COSMO
#define KNOB_IMPLEMENTATION
#include "knob.h"
#define dlsym dynlib_loadfunc
#define dlopen dynlib_load
#define dlclose dynlib_unload
#define dlerror() dynlib_last_err
#else
#include <cosmo.h>
#define _COSMO_SOURCE
#include<libc/dlopen/dlfcn.h>
#define dlsym cosmo_dlsym
#define dlopen cosmo_dlopen
#endif
#define DLL_COMPILE
#include "dll.h"
#include "hotreload.h"

enum KEY {
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

#define LOAD_FUN(name) name = dlsym(lib,#name)
void load_fenster(void* lib){
    LOAD_FUN(fenster_open);
    LOAD_FUN(fenster_loop);
    LOAD_FUN(fenster_close);
    LOAD_FUN(fenster_screen_size);
    LOAD_FUN(fenster_sleep);
    LOAD_FUN(fenster_time);
}
int main(int argc, char* argv[]){
    #ifdef HOTRELOAD
    void* lib = reload_libplug();
    if (!lib) return 1;
    #else
    void* lib = dlopen("./fenster.so",RTLD_LAZY);
    #endif
    if(lib == NULL){
        fprintf(stderr,"Couldn't load windowing library,quitting the app %s.",dlerror());
        return 1;
    }
    load_fenster(lib);
    int w,h;
    //@TODO: Fix linux behaviour, we get the two diplay's size together instead of just the current Display.
    fenster_screen_size(&w,&h);
    fenster_t* f = plug_init(w,h);
    // f->buf = malloc(f->width * f->height);
    fenster_open(f);
    uint32_t t = 0;
    int64_t now = fenster_time();
    while (fenster_loop(f) == 0) {
        // int has_keys = 0;
        // char s[32];
        // char *p = s;
        // for (int i = 0; i < 128; i++) {
        // if (f->keys[i]) {
        //     has_keys = 1;
        //     *p++ = i;
        // }
        // }
        // *p = '\0';
        // if(s[0] != '\0')
        //     fprintf(stdout,"Keys that were touched: %s\n",s);
        if(f->keys[KEY_R]){
            void *state = plug_pre_reload();
            lib = reload_libplug();
            if (!lib) return 1;
            load_fenster(lib);

            plug_post_reload(state);
        }
        plug_update();
        int64_t time = fenster_time();
        if (time - now < 1000 / 60) {
            fenster_sleep(time - now);
        }
        now = time;
    }
    fenster_close(f);
    return 0;

}