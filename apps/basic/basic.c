#include <stdio.h>
#include <stdlib.h>
#ifdef NOT_COSMO
#include <dlfcn.h>
#else
#include <cosmo.h>
#define _COSMO_SOURCE
#include<libc/dlopen/dlfcn.h>
#define dlsym cosmo_dlsym
#define dlopen cosmo_dlopen
#endif
#define DLL_COMPILE
#include "dll.h"

#define LOAD_FUN(name) name = dlsym(lib,#name)
int main(int argc, char* argv[]){
    void* lib = dlopen("./fenster.so",RTLD_LAZY);
    if(lib == NULL){
        fprintf(stderr,"Couldn't load windowing library,quitting the app.");
        return 1;
    }
    LOAD_FUN(fenster_open);
    LOAD_FUN(fenster_loop);
    LOAD_FUN(fenster_close);
    LOAD_FUN(fenster_screen_size);
    LOAD_FUN(fenster_sleep);
    LOAD_FUN(fenster_time);


    #define W 320
    #define H 240
    uint32_t buf[W * H];
    fenster_t f = {
        .title = "hello",
        .width = W,
        .height = H,
        .buf = buf,
    };
    fenster_open(&f);
    uint32_t t = 0;
    int64_t now = fenster_time();
    while (fenster_loop(&f) == 0) {
        t++;
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < H; j++) {
                /* White noise: */
                /* fenster_pixel(&f, i, j) = (rand() << 16) ^ (rand() << 8) ^ rand(); */

                /* Colourful and moving: */
                /* fenster_pixel(&f, i, j) = i * j * t; */

                /* Munching squares: */
                fenster_pixel(&f, i, j) = i ^ j ^ t;
            }
        }
        int64_t time = fenster_time();
        if (time - now < 1000 / 60) {
            fenster_sleep(time - now);
        }
        now = time;
    }
    fenster_close(&f);
    return 0;

}