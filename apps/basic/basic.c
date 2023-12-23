#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rencache.h"
#include "plug.h"

struct Plug{
    fenster_t fen;
};

static Plug_t* p = NULL;
void init(void){
    ren_init(&p->fen);
}
fenster_t* plug_init(int w,int h){
    p = malloc(sizeof(Plug_t));
    assert(p != NULL && "Buy more RAM lol");
    memset(p, 0, sizeof(*p));
    #define W 320
    #define H 240
    fenster_t fen = {.title="cosmo-basic",.width=W,.height=H};
    fen.buf = malloc(sizeof(uint32_t) * fen.width * fen.height);
    memcpy(&p->fen,&fen,sizeof(fenster_t));
    init();
    return &p->fen;
}

Plug_t *plug_pre_reload(void){
    return p;
}

void plug_post_reload(Plug_t *pp) {
    p = pp;
    init();
}
void plug_update(void){

    static uint32_t t = 0;
    t++;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < H; j++) {
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //~~~~~~~~~~~~~~~~~~IMPORTANT: To test dll hotreload, use `ctrl+shift+b` and press `r` on app~~~~~~~~~~~~
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            /* White noise: */
            // fenster_pixel(&p->fen, i, j) = (rand() << 16) ^ (rand() << 8) ^ rand();

            /* Colourful and moving: */
            // fenster_pixel(&p->fen, i, j) = i * j * t;

            /* Munching squares: */
            fenster_pixel(&p->fen, i, j) = i ^ j ^ t;
        }
    }
}