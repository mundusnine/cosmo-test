#include <string.h>
#include <assert.h>

#include "rencache.h"
#include "plug.h"

struct Plug{
    fenster_t fen;
    double history[64];
};

static Plug_t* p = NULL; 
fenster_t* plug_init(void){
    p = malloc(sizeof(Plug_t));
    assert(p != NULL && "Buy more RAM lol");
    memset(p, 0, sizeof(*p));

    fenster_t fen = {.title="cosmo-calc",.width=1280,.height=720};
    fen.buf = malloc(sizeof(uint32_t) * fen.width * fen.height);
    memcpy(&p->fen,&fen,sizeof(fenster_t));
    ren_init(&p->fen);

    return &p->fen;
}

Plug_t *plug_pre_reload(void){
    return p;
}

void plug_post_reload(Plug_t *pp) {
    p = pp;
    ren_init(&p->fen);
}
RenRect window_rect = {0};
RenColor bg_color = {.r=0,.b=0,.g=0,.a=255};
void plug_update(void){
    window_rect.width = p->fen.width;
    window_rect.height = p->fen.height;
    rencache_begin_frame();
    rencache_draw_rect(window_rect,bg_color);
    RenColor col = {.a=255,.r=0,.b=255};
    ren_draw_circle(p->fen.width * 0.5f,p->fen.height * 0.5f,100,col);
    rencache_end_frame();
}