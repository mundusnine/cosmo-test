#include <string.h>
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
    Knob_String_Builder curr_cal;
    RenFont* fonts[16];
    double history[64];
};

static Plug_t* p = NULL;
void init(void){
    for(int i = 0; i < NUM_FONTS;++i){
        p->fonts[i] = ren_load_font("assets/FiraSans-Regular.ttf",14 * (i +1));
    }
    ren_init(&p->fen);
}
fenster_t* plug_init(int w,int h){
    p = malloc(sizeof(Plug_t));
    assert(p != NULL && "Buy more RAM lol");
    memset(p, 0, sizeof(*p));

    fenster_t fen = {.title="cosmo-calc",.width=w*0.15f,.height=h*0.6f};
    fen.buf = malloc(sizeof(uint32_t) * fen.width * fen.height);
    memcpy(&p->fen,&fen,sizeof(fenster_t));
    p->curr_cal.count = 0;
    knob_sb_append_cstr(&p->curr_cal,"0");
    knob_sb_append_null(&p->curr_cal);
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
RenRect window_rect = {0};
RenColor win_bg = {.r=37,.g=37,.b=38,.a=255};
RenColor frame_bg = {.r=51,.g=51,.b=55,.a=255};
RenColor button_col = {.r=51,.g=51,.b=55,.a=255};
RenColor button_hovered_col = {.r=29,.g=150,.b=237,.a=255};
RenColor button_click_col = {.r=0,.g=119,.b=200,.a=255};
RenColor text_color = {.r=255,.g=255,.b=255,.a=255};
char* butt_content[5][5] = {
    {"C","(",")","mod","π"},
    {"7","8","9","÷","√"},
    {"4","5","6","×","x²"},
    {"1","2","3","-","="},
    {"0",".","%","+","="}
};
typedef struct {
    float x,y;
    int l,m,r;
} mouse_t;
void plug_update(void){
    mouse_t mouse = {.x=p->fen.x,.y=p->fen.y};
    mouse.l = (p->fen.mouse >> MOUSE_LEFT_BUTTON) & 1;
    mouse.m = (p->fen.mouse >> MOUSE_MIDDLE_BUTTON) & 1;
    mouse.r = (p->fen.mouse >> MOUSE_RIGHT_BUTTON) & 1;

    window_rect.width = p->fen.width;
    window_rect.height = p->fen.height;
    rencache_begin_frame();
    rencache_draw_rect(window_rect,win_bg);
    float padding = 10;
    RenRect show_rect = {padding,padding,p->fen.width - padding * 2 ,p->fen.height*0.1f};
    rencache_draw_rect(show_rect,frame_bg);
    int cal_w = ren_get_font_width(p->fonts[BIG_FONT],p->curr_cal.items,p->curr_cal.count);
    int cal_h = ren_get_font_height(p->fonts[BIG_FONT]);
    rencache_draw_text(p->fonts[BIG_FONT],p->curr_cal.items,show_rect.width - cal_w,show_rect.y+show_rect.height-(cal_h+padding),text_color);
    int grid_y = 0;
    float curr_x = show_rect.x;
    #define NUM_BUTTS 5
    float b_pad = padding * 0.15f;
    float butt_w = p->fen.width / NUM_BUTTS - (b_pad *  NUM_BUTTS);
    float current_y = show_rect.height+show_rect.y+padding;
    int button_text_h = ren_get_font_height(p->fonts[BASIC_FONT]) * 0.5f;
    for(int i =0; i < NUM_BUTTS;){
        RenRect butt_rect = {.x=curr_x + i*(butt_w+b_pad*2.5),.y=grid_y*(butt_w+b_pad*2.5)+current_y,.width=butt_w,.height=butt_w};
        RenColor butt_col = button_col;
        if(mouse.x > butt_rect.x && mouse.x < butt_rect.x + butt_rect.width && mouse.y > butt_rect.y && mouse.y < butt_rect.y + butt_rect.height){
            butt_col = button_hovered_col;
            if(mouse.l){
                butt_col = button_click_col;
            }
        }
        rencache_draw_rect(butt_rect,butt_col);
        char* text = butt_content[grid_y][i];
        int button_text_w = ren_get_font_width(p->fonts[BASIC_FONT],text,strlen(text)) * 0.5f; 
        rencache_draw_text(p->fonts[BASIC_FONT],text,
            butt_rect.x+butt_rect.width*0.5f - button_text_w,//x
            butt_rect.y+butt_rect.height*0.5f - button_text_h, //y
            text_color
        );
        ++i;
        if(i >= NUM_BUTTS && grid_y + 1 < NUM_BUTTS){
            grid_y++;
            i=0;
        }
    }


    
    rencache_end_frame();
}

// Theme rounded Visual Studio
// 1. ImGuiCol_Text: RGBA(255, 255, 255, 255)
// 2. ImGuiCol_TextDisabled: RGBA(151, 151, 151, 255)
// 3. ImGuiCol_WindowBg: RGBA(37, 37, 38, 255)
// 4. ImGuiCol_ChildBg: RGBA(37, 37, 38, 255)
// 5. ImGuiCol_PopupBg: RGBA(37, 37, 38, 255)
// 6. ImGuiCol_Border: RGBA(78, 78, 78, 255)
// 7. ImGuiCol_BorderShadow: RGBA(78, 78, 78, 255)
// 8. ImGuiCol_FrameBg: RGBA(51, 51, 55, 255)
// 9. ImGuiCol_FrameBgHovered: RGBA(29, 150, 237, 255)
// 10. ImGuiCol_FrameBgActive: RGBA(0, 119, 200, 255)
// 11. ImGuiCol_TitleBg: RGBA(37, 37, 38, 255)
// 12. ImGuiCol_TitleBgActive: RGBA(37, 37, 38, 255)
// 13. ImGuiCol_TitleBgCollapsed: RGBA(37, 37, 38, 255)
// 14. ImGuiCol_MenuBarBg: RGBA(51, 51, 55, 255)
// 15. ImGuiCol_ScrollbarBg: RGBA(51, 51, 55, 255)
// 16. ImGuiCol_ScrollbarGrab: RGBA(82, 82, 85, 255)
// 17. ImGuiCol_ScrollbarGrabHovered: RGBA(90, 90, 95, 255)
// 18. ImGuiCol_ScrollbarGrabActive: RGBA(90, 90, 95, 255)
// 19. ImGuiCol_CheckMark: RGBA(0, 119, 200, 255)
// 20. ImGuiCol_SliderGrab: RGBA(29, 150, 237, 255)
// 21. ImGuiCol_SliderGrabActive: RGBA(0, 119, 200, 255)
// 22. ImGuiCol_Button: RGBA(51, 51, 55, 255)
// 23. ImGuiCol_ButtonHovered: RGBA(29, 150, 237, 255)
// 24. ImGuiCol_ButtonActive: RGBA(29, 150, 237, 255)
// 25. ImGuiCol_Header: RGBA(51, 51, 55, 255)
// 26. ImGuiCol_HeaderHovered: RGBA(29, 150, 237, 255)
// 27. ImGuiCol_HeaderActive: RGBA(0, 119, 200, 255)
// 28. ImGuiCol_Separator: RGBA(78, 78, 78, 255)
// 29. ImGuiCol_SeparatorHovered: RGBA(78, 78, 78, 255)
// 30. ImGuiCol_SeparatorActive: RGBA(78, 78, 78, 255)
// 31. ImGuiCol_ResizeGrip: RGBA(37, 37, 38, 255)
// 32. ImGuiCol_ResizeGripHovered: RGBA(51, 51, 55, 255)
// 33. ImGuiCol_ResizeGripActive: RGBA(82, 82, 85, 255)
// 34. ImGuiCol_Tab: RGBA(37, 37, 38, 255)
// 35. ImGuiCol_TabHovered: RGBA(29, 150, 237, 255)
// 36. ImGuiCol_TabActive: RGBA(0, 119, 200, 255)
// 37. ImGuiCol_TabUnfocused: RGBA(37, 37, 38, 255)
// 38. ImGuiCol_TabUnfocusedActive: RGBA(0, 119, 200, 255)
// 39. ImGuiCol_PlotLines: RGBA(0, 119, 200, 255)
// 40. ImGuiCol_PlotLinesHovered: RGBA(29, 150, 237, 255)
// 41. ImGuiCol_PlotHistogram: RGBA(0, 119, 200, 255)
// 42. ImGuiCol_PlotHistogramHovered: RGBA(29, 150, 237, 255)
// 43. ImGuiCol_TableHeaderBg: RGBA(48, 48, 51, 255)
// 44. ImGuiCol_TableBorderStrong: RGBA(79, 79, 89, 255)
// 45. ImGuiCol_TableBorderLight: RGBA(58, 58, 63, 255)
// 46. ImGuiCol_TableRowBg: RGBA(0, 0, 0, 0)
// 47. ImGuiCol_TableRowBgAlt: RGBA(255, 255, 255, 15)
// 48. ImGuiCol_TextSelectedBg: RGBA(0, 119, 200, 255)
// 49. ImGuiCol_DragDropTarget: RGBA(37, 37, 38, 255)
// 50. ImGuiCol_NavHighlight: RGBA(37, 37, 38, 255)
// 51. ImGuiCol_NavWindowingHighlight: RGBA(255, 255, 255, 178)
// 52. ImGuiCol_NavWindowingDimBg: RGBA(204, 204, 204, 51)
// 53. ImGuiCol_ModalWindowDimBg: RGBA(37, 37, 38, 255)