#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "rencache.h"
#include "plug.h"
#define INPUT_IMPL
#include "input.h"

typedef struct {
    float x;
    float y;
} Vec2;

enum Fonts{
    BASIC_FONT,
    BIG_FONT,
    NUM_FONTS
};

typedef enum {
    ASCENDING,
    DESCENDING,
    TIME,
    MODE_COUNT
} CounterMode;


const char* modes[MODE_COUNT] = {
    "ASCENDING",
    "DESCENDING",
    "TIME"
};

typedef enum {
    PAUSED,
    PLAYING,
    STATE_COUNT
} CounterState;

const char* states[STATE_COUNT] = {
    "||",
    "¦>",
};


typedef enum {
    UTC,
    PST,
    MST,
    CST,
    EST,
    BRT,
    MSK,
    IST,
    SGT,
    JST,
    AET,
    NZT,
    TIMEZONE_COUNT
} TimeZone;

const char* timezone_names[] = {
    "UTC",
    "PST",
    "MST",
    "CST",
    "EST",
    "BRT",
    "MSK",
    "IST",
    "SGT",
    "JST",
    "AET",
    "NZT"
};

const float time_offsets[] = {
    0,
    -8,
    -7,
    -6,
    -5,
    -3,
    3,
    5.5f,
    8,
    9,
    11,
    13
};

struct Plug{
    fenster_t fen;
    RenFont* fonts[16];
    RenImage img;
    Vec2 nums[10];
    float timer;
    CounterMode mode;
    CounterState state;
    TimeZone zone;
};

static Plug_t* p = NULL;
static int64_t now = 0;
void init(void){
    for(int i = 0; i < NUM_FONTS;++i){
        p->fonts[i] = ren_load_font("assets/FiraSans-Regular.ttf",14 * (i +1));
    }
    ren_init(&p->fen);
    now = fenster_time();
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

    fenster_t fen = {.title="cosmo-clock",.width=w * 0.25f,.height=h * 0.5f};
    fen.buf = malloc(sizeof(uint32_t) * fen.width * fen.height);
    memcpy(&p->fen,&fen,sizeof(fenster_t));
    int n_comps = 0;
    stbi_uc* pixels = stbi_load("./assets/fontedit_0.png",&p->img.width,&p->img.height,&n_comps,4);
    p->img.pixels = malloc(sizeof(RenColor) * p->img.width * p->img.height);
    p->timer = 0.0f;
    p->mode = p->zone = 0;
    int i = 0;
    for(int y=0; y < p->img.height;++y){
        for(int x = 0; x < p->img.width;++x){
            RenColor* curr = &p->img.pixels[i];
            curr->r = pixels[0];
            curr->g = pixels[1];
            curr->b = pixels[2];
            curr->a = pixels[3];
            i++;
            pixels += 4;
        }
    }
    int cell_w = p->img.width/16;
    int cell_h = p->img.height/6;
    p->nums[0].x = cell_w*1;
    p->nums[0].y = cell_h*2;
    p->nums[9].x = 0;
    p->nums[9].y = cell_h*2;
    for(int i =1; i < 9; ++i){
        p->nums[i].x = cell_w*(i-1);
        p->nums[i].y = cell_h*1;
    }
    init();
    return &p->fen;
}

Plug_t *plug_pre_reload(void){
    deinit();
    return p;
}

void plug_post_reload(Plug_t *pp) {
    p = pp;
    init();
}

#define WHITE (RenColor){.r=255,.b=255,.g=255,.a=255}
#define BLACK (RenColor){.r=0,.b=0,.g=0,.a=255}
#define RED (RenColor){.r=255,.b=0,.g=0,.a=255}
RenColor button_col = {.r=51,.g=51,.b=55,.a=255};
RenColor button_hovered_col = {.r=29,.g=150,.b=237,.a=255};
RenColor button_click_col = {.r=0,.g=119,.b=200,.a=255};
RenColor text_color = {.r=255,.g=255,.b=255,.a=255};
RenColor text_disabled_col = {.r=151,.g=151,.b=151,.a=255};
RenColor title_bg_col = {.r=133,.g=76,.b=199,.a=255};

#define BUTT_PAD 5
void plug_update(void){
    // rencache_show_debug(true);
    static mouse_t last_mouse = {0}; 
    mouse_t mouse = {.x=p->fen.x,.y=p->fen.y};
    mouse.l = (p->fen.mouse >> MOUSE_LEFT_BUTTON) & 1;
    mouse.m = (p->fen.mouse >> MOUSE_MIDDLE_BUTTON) & 1;
    mouse.r = (p->fen.mouse >> MOUSE_RIGHT_BUTTON) & 1;

    int64_t time = fenster_time();
    float dt = (time - now) * 0.001;
    now = time;
    p->timer += p->mode == ASCENDING ? dt : p->state == PLAYING ? -dt : 0;
    rencache_begin_frame();
    RenRect win = {
        .x=0,
        .y=0,
        .width=p->fen.width,
        .height=p->fen.height
    };
    rencache_draw_rect(win,BLACK);
    int cell_w = p->img.width/16;
    int cell_h = p->img.height/6;
    RenRect clock_rect = {
        .x=0,
        .y=p->fen.height * 0.5f - cell_h* 0.5f,
        .width = cell_w * 8,
        .height=cell_h 
    };
    clock_rect.x = clock_rect.width * 0.25f;
    // rencache_draw_rect(clock_rect,time % 2 ? BLACK : WHITE);
    RenRect reset_butt = {
        .x=clock_rect.x,
        .y=0,
        .width = cell_h,
        .height = cell_w
    };
    reset_butt.y = clock_rect.y - reset_butt.height;

    RenRect mode_butt = {
        .x= reset_butt.x + reset_butt.width + cell_w,
        .y = reset_butt.y,
        .width = reset_butt.width,
        .height = reset_butt.height
    };

    int mouse_on = mouse_on_rect(mouse,reset_butt);
    if(p->fen.keys[KEY_S] || (mouse_on && mouse.l && mouse.l != last_mouse.l)){
        p->timer = -1;
        //@TODO: I believe this shouldn't be needed to actually reset the UI correctly, but for now it will do.
        // Basically, what happens is when we click with the mouse, the image drawing lags for 2 seconds then starts at 2 seconds...
        // rencache_invalidate();
    }
    RenColor reset_col = button_col;
    if(mouse_on){
        reset_col = button_hovered_col;
        if(mouse.l){
            reset_col = button_click_col;
        }
    }
    mouse_on = mouse_on_rect(mouse,mode_butt);
    RenColor mode_col = button_col;
    if(mouse_on){
        mode_col = button_hovered_col;
        if(mouse.l && mouse.l != last_mouse.l){
            mode_col = button_click_col;
            p->mode++;
            if(p->mode == MODE_COUNT){
                p->mode = 0;
            }
        }
    }
    RenRect zone_butt = {
        .x=mode_butt.x,
        .y=mode_butt.y - mode_butt.height - BUTT_PAD,
        .width = mode_butt.width,
        .height = mode_butt.height
    };
    RenColor zone_col = button_col;
    RenColor zone_text_col = WHITE;
    zone_text_col.a *= 0.5f;
    RenColor reset_text_col = WHITE;
    if(p->mode == TIME){
        reset_text_col.a *= 0.5f;
        reset_col = button_col;
        zone_text_col = WHITE;
        mouse_on = mouse_on_rect(mouse,zone_butt);
        if(mouse_on){
            zone_col = button_hovered_col;
            if(mouse.l && mouse.l != last_mouse.l){
                zone_col = button_click_col;
                p->zone += 1;
                if(p->zone == TIMEZONE_COUNT){
                    p->zone = UTC;
                }
            }
        }
    }
    rencache_draw_rect(reset_butt,reset_col);
    int text_w = ren_get_font_width(p->fonts[BASIC_FONT],"RESET",6); 
    int text_h = ren_get_font_height(p->fonts[BASIC_FONT]);
    rencache_draw_text(p->fonts[BASIC_FONT],"RESET",reset_butt.x + reset_butt.width * 0.5f - text_w * 0.5f,reset_butt.y + reset_butt.height * 0.5f - text_h * 0.5f,reset_text_col);

    rencache_draw_rect(mode_butt,mode_col);
    text_w = ren_get_font_width(p->fonts[BASIC_FONT],modes[p->mode], p->mode == ASCENDING ? 10 : 11); 
    rencache_draw_text(p->fonts[BASIC_FONT],modes[p->mode],mode_butt.x + mode_butt.width * 0.5f - text_w * 0.5f,mode_butt.y + mode_butt.height * 0.5f - text_h * 0.5f,WHITE);

    rencache_draw_rect(zone_butt,zone_col);
    text_w = ren_get_font_width(p->fonts[BASIC_FONT],timezone_names[p->zone],3); 
    rencache_draw_text(p->fonts[BASIC_FONT],timezone_names[p->zone],zone_butt.x + zone_butt.width * 0.5f - text_w * 0.5f,zone_butt.y + zone_butt.height * 0.5f - text_h * 0.5f,zone_text_col);

    RenColor state_text_col = WHITE;
    state_text_col.a *= 0.5f;
    if(p->mode == DESCENDING){
        state_text_col =  WHITE;
    }
    RenRect up_butt = {
        .x = mode_butt.x + mode_butt.width + cell_w,
        .y = mode_butt.y,
        .width = mode_butt.width * 0.5f,
        .height = mode_butt.height * 0.5f - BUTT_PAD
    };
    RenColor up_butt_col = button_col;
    rencache_draw_rect(up_butt,up_butt_col);
    text_w = ren_get_font_width(p->fonts[BASIC_FONT],"+",2); 
    rencache_draw_text(p->fonts[BASIC_FONT],"+",up_butt.x + up_butt.width * 0.5f - text_w * 0.5f,up_butt.y + up_butt.height * 0.5f - text_h * 0.5f,state_text_col);

    RenRect down_butt = {
        .x = up_butt.x,
        .y = up_butt.y + up_butt.height + BUTT_PAD,
        .width = up_butt.width,
        .height = up_butt.height
    };
    RenColor down_butt_col = button_col;
    rencache_draw_rect(down_butt,down_butt_col);
    text_w = ren_get_font_width(p->fonts[BASIC_FONT],"-",2); 
    rencache_draw_text(p->fonts[BASIC_FONT],"-",down_butt.x + down_butt.width * 0.5f - text_w * 0.5f,down_butt.y + down_butt.height * 0.5f - text_h * 0.5f,state_text_col);

    RenRect state_butt = {
        .x = up_butt.x + up_butt.width + BUTT_PAD,
        .y = up_butt.y + up_butt.height * 0.5f + BUTT_PAD,
        .width = up_butt.width,
        .height = up_butt.height
    };
    RenColor state_butt_col = button_col;
    rencache_draw_rect(state_butt,state_butt_col);
    text_w = ren_get_font_width(p->fonts[BASIC_FONT],states[!p->state],2); 
    rencache_draw_text(p->fonts[BASIC_FONT],states[!p->state],state_butt.x + state_butt.width * 0.5f - text_w * 0.5f,state_butt.y + state_butt.height * 0.5f - text_h * 0.5f,state_text_col);

    int64_t t = time * 0.001;
    float time_offset = time_offsets[p->zone];
    if(p->mode != TIME){
        t = (int64_t)ceilf(fmaxf(p->timer,0.0f));
        time_offset =0;
    }
    int64_t hours = t / 60 / 60 % 24 + time_offset;
    if(hours > 24){
        hours -= 24; 
    }
    else if( hours < 0){
        hours += 24;
    }
    int hours0 = hours / 10;
    int hours1 = hours % 10;
    int64_t minutes = (int64_t)t / 60 % 60;
    int minutes0 = minutes / 10;
    int minutes1 = minutes % 10;
    int64_t seconds = (int64_t)t % 60;
    int seconds0 = seconds / 10;
    int seconds1 = seconds % 10;
    
    RenRect sub = {
        .x=0,
        .y=0,
        .width=cell_w,
        .height=cell_h 
    };
    sub.x=p->nums[hours0].x;
    sub.y=p->nums[hours0].y;
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 0,clock_rect.y,WHITE);
    sub.x=p->nums[hours1].x;
    sub.y=p->nums[hours1].y;
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 1,clock_rect.y,WHITE);
    sub.x=0,
    sub.y=0,
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 2,clock_rect.y,WHITE);
    sub.x=p->nums[minutes0].x;
    sub.y=p->nums[minutes0].y;
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 3,clock_rect.y,WHITE);
    sub.x=p->nums[minutes1].x;
    sub.y=p->nums[minutes1].y;
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 4,clock_rect.y,WHITE);
    sub.x=0,
    sub.y=0,
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 5,clock_rect.y,WHITE);
    sub.x=p->nums[seconds0].x;
    sub.y=p->nums[seconds0].y;
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 6,clock_rect.y,WHITE);
    sub.x=p->nums[seconds1].x;
    sub.y=p->nums[seconds1].y;
    rencache_draw_img(&p->img,sub,clock_rect.x + cell_w * 7,clock_rect.y,WHITE);
    rencache_end_frame();
    last_mouse = mouse;
}