#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define KNOB_IMPLEMENTATION
#include "knob.h"

#include "rencache.h"
#include "plug.h"

enum Fonts{
    BASIC_FONT,
    BIG_FONT,
    NUM_FONTS
};

struct Plug{
    fenster_t fen;
    int selected_line;
    Knob_String_Builder contents;
    RenFont* fonts[16];
};

static Plug_t* p = NULL;
void init(void){
    for(int i = 0; i < NUM_FONTS;++i){
        p->fonts[i] = ren_load_font("assets/FiraSans-Regular.ttf",14 * (i +1));
    }
    p->selected_line = -1;
    ren_init(&p->fen);
}
void deinit(void){
    for(int i = 0; i < NUM_FONTS;++i){
        ren_free_font(p->fonts[i]);
    }
}
fenster_t* plug_init(int w,int h){
    p = malloc(sizeof(Plug_t));
    assert(p != NULL && "Buy more RAM lol");
    memset(p, 0, sizeof(*p));
    fenster_t fen = {.title="cosmo-todo",.width=w*0.2f,.height=h*0.5f};
    fen.buf = malloc(sizeof(uint32_t) * fen.width * fen.height);
    memcpy(&p->fen,&fen,sizeof(fenster_t));
    p->contents.capacity = p->contents.count = 0;
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

typedef struct {
    float x,y;
    int l,m,r;
} mouse_t;

RenRect window_rect = {0};
RenColor win_bg = {.r=37,.g=37,.b=38,.a=255};
RenColor frame_bg = {.r=51,.g=51,.b=55,.a=255};
RenColor button_col = {.r=51,.g=51,.b=55,.a=255};
RenColor button_hovered_col = {.r=29,.g=150,.b=237,.a=255};
RenColor button_click_col = {.r=0,.g=119,.b=200,.a=255};
RenColor text_color = {.r=255,.g=255,.b=255,.a=255};
RenColor text_disabled_col = {.r=151,.g=151,.b=151,.a=255};
RenColor title_bg_col = {.r=133,.g=76,.b=199,.a=255};

int mouse_on_rect(mouse_t mouse,RenRect rect){
    return mouse.x > rect.x && mouse.x < rect.x + rect.width && mouse.y > rect.y && mouse.y < rect.y + rect.height;
}
int get_next_line(Knob_String_Builder lines,size_t* pos, char* out_line){
    size_t i = *pos;
    while(i < lines.count && lines.items[i] != '\n'){
        out_line[i - *pos] = lines.items[i];
        ++i;
    }
    if(i - *pos > 0){
        *pos = i+1;
        return i;
    }
    return 0;
}
#define PAD_W 10
void plug_update(void){
    static mouse_t last_mouse = {0}; 
    mouse_t mouse = {.x=p->fen.x,.y=p->fen.y};
    mouse.l = (p->fen.mouse >> MOUSE_LEFT_BUTTON) & 1;
    mouse.m = (p->fen.mouse >> MOUSE_MIDDLE_BUTTON) & 1;
    mouse.r = (p->fen.mouse >> MOUSE_RIGHT_BUTTON) & 1;

    if(mouse.l && p->selected_line != -1){
        p->selected_line = -1;
    }

    window_rect.width = p->fen.width;
    window_rect.height = p->fen.height;
    int big_text_h = ren_get_font_height(p->fonts[BIG_FONT]) * 0.5f;
    int basic_text_h = ren_get_font_height(p->fonts[BASIC_FONT]) * 0.5f;
    rencache_begin_frame();
    rencache_draw_rect(window_rect,win_bg);

    RenColor title_col = text_color;
    char title_text[] = "TODO";
    RenRect title_rect = {.x=window_rect.x,.y=window_rect.y};
    title_rect.width = ren_get_font_width(p->fonts[BIG_FONT],title_text,17);
    title_rect.height = ren_get_font_height(p->fonts[BIG_FONT]);
    title_rect.y += PAD_W * 0.25f;
    RenRect title_bg_rect = {.x=0,.y=0,.width=window_rect.width,.height=title_rect.height};
    rencache_draw_rect(title_bg_rect,title_bg_col);

    title_rect.x = window_rect.width * 0.5f - title_rect.width * 0.5f;
    if(mouse_on_rect(mouse,title_rect)){
        title_col = text_disabled_col;
    }
    rencache_draw_text(p->fonts[BIG_FONT],title_text,title_rect.x,title_rect.y,title_col);

    RenRect plus_butt_rect = {.x=0+PAD_W,.y=title_rect.height+title_rect.y};
    plus_butt_rect.height = plus_butt_rect.width = ren_get_font_width(p->fonts[BIG_FONT],"+",2) + PAD_W;
    RenColor plus_butt_col = button_col;
    if(mouse_on_rect(mouse,plus_butt_rect)){
        plus_butt_col = button_hovered_col;
        if(mouse.l){
            plus_butt_col = button_click_col;
            knob_sb_append_cstr(&p->contents,"This is a new line biche\n");
        }
    }
    int plus_w = (ren_get_font_width(p->fonts[BIG_FONT],"+",2) ) * 0.5f ;
    int plus_h = ren_get_font_height(p->fonts[BIG_FONT]);
    rencache_draw_rect(plus_butt_rect,plus_butt_col);
    rencache_draw_text(p->fonts[BIG_FONT],"+",
        plus_butt_rect.x+plus_butt_rect.width*0.5f - plus_w,//x
        plus_butt_rect.y+plus_butt_rect.height*0.5f - big_text_h, //y
        text_color
    );
    size_t pos = 0;
    char line[512] = {0};
    int line_count = 0;
    RenRect lines_rect = {
        .x=0,
        .y=plus_butt_rect.y+plus_butt_rect.height + PAD_W * 0.25f,
        .width=window_rect.width,
        .height=basic_text_h * 2
    };
    while(get_next_line(p->contents,&pos,line)){
        RenColor bg_col = button_col;
        if(line_count % 2 == 0){
            bg_col.r += 10;
            bg_col.b += 10;
            bg_col.g += 10;
        }
        if(mouse_on_rect(mouse,lines_rect)){
            bg_col = button_hovered_col;
            if(mouse.l){
                p->selected_line = line_count;
                bg_col = button_click_col;
            }
        }
        if(line_count == p->selected_line){
            bg_col = title_bg_col;
        }
        rencache_draw_rect(lines_rect,bg_col);
        rencache_draw_text(p->fonts[BASIC_FONT],line,
            lines_rect.x,//x
            lines_rect.y, //y
            text_color
        );
        lines_rect.y +=  lines_rect.height + PAD_W * 0.5f;
        line_count++;
    }
    rencache_end_frame();
    last_mouse = mouse;   
}