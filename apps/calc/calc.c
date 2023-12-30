#include <string.h>
#include <math.h>
#include <assert.h>

#define KNOB_IMPLEMENTATION
#include "knob.h"

#include "rencache.h"
#include "plug.h"
#define INPUT_IMPL
#include "input.h"

//ASCII Table: https://theasciicode.com.ar/

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
void deinit(void){
    for(int i = 0; i < NUM_FONTS;++i){
        ren_free_font(p->fonts[i]);
    }
}
fenster_t* plug_init(int w,int h){
    p = malloc(sizeof(Plug_t));
    assert(p != NULL && "Buy more RAM lol");
    memset(p, 0, sizeof(*p));

    fenster_t fen = {.title="cosmo-calc",.width=w*0.25f,.height=h*0.6f};
    fen.buf = malloc(sizeof(uint32_t) * fen.width * fen.height);
    memcpy(&p->fen,&fen,sizeof(fenster_t));
    p->curr_cal.count = 0;
    knob_sb_append_cstr(&p->curr_cal,"0");
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
#define CLEAR "C"
#define PI "π"
#define PI_NUM 3.141592653
#define EQUAL "="
#define SQR_RT "√"
#define OP_PAREN "("
#define CL_PAREN ")"
#define MOD "%"
#define DIV "/"
#define MUL "*"
#define SUB "-"
#define ADD "+"
#define SQUARED "x²"
#define DOT "."
#define MAX_RESULT_LENGTH 256
#define EXPONENT "xª"

RenRect window_rect = {0};
RenColor win_bg = {.r=37,.g=37,.b=38,.a=255};
RenColor frame_bg = {.r=51,.g=51,.b=55,.a=255};
RenColor button_col = {.r=51,.g=51,.b=55,.a=255};
RenColor button_hovered_col = {.r=29,.g=150,.b=237,.a=255};
RenColor button_click_col = {.r=0,.g=119,.b=200,.a=255};
RenColor text_color = {.r=255,.g=255,.b=255,.a=255};
RenColor text_disabled_col = {.r=151,.g=151,.b=151,.a=255};
RenColor butt_equl_col = {.r=133,.g=76,.b=199,.a=255};

char* butt_content[5][5] = {
    {CLEAR,OP_PAREN,CL_PAREN,MOD,PI},
    {"7","8","9",DIV,SQR_RT},
    {"4","5","6",MUL,SQUARED},
    {"1","2","3",SUB,EQUAL},
    {"0",DOT,EXPONENT,ADD,""}
};

int IsOp(char c){
    if(c == '+' || c == '-' || c == '%' || c == '*' || c == '/' || c == '^'){
        return 1;
    }
    return 0;
}


double calc(char* left, char* right, char op){
    // if(right[0] == '\0') return 0;
    char* end;
    double num0 = left[0] != '\0' ? strtod(left,&end) : 0;
    double num1 = strtod(right,&end);
    double res = 0.0;
    if(op == '+'){
        res = num0 + num1;
    }
    else if(op == '-'){
        res = num0 - num1;
    }
    else if(op == '*'){
        res = num0 * num1;
    }
    else if(op == '/'){
        res = num0 / num1;
    }
    else if(op == '^'){
        res = num0;
        for(int i = num1-1;i > 0; --i){
            res *= num0;
        }
    }
    else if(op == '%'){
        res = (int)num0 % (int)num1;
    }
    else if(op == 's'){

        res = sqrt(num1);
    }
    return res;
}
typedef struct OpNode OpNode_t;
struct OpNode{
    char* left;
    char* right;
    char* op;
    int switched;
    OpNode_t* next;
    OpNode_t* last;
};
typedef struct {
    OpNode_t** items;
    size_t count;
    size_t capacity;
} OpNodeList;

typedef enum {
    AS,
    MD,
    E,
    P
} OpType;
int OpPriority(char left,char right){
    OpType ltype = (left == '+' || left == '-') ? AS : (left == '*' || left == '/' || left == '%') ? MD : E;
    OpType rtype = (right == '+' || right == '-') ? AS : (right == '*' || right == '/' || right == '%') ? MD : E;  
    if(ltype < rtype){
        return 1;
    }
    return 0;

}
void addNode(OpNodeList* list, char* left, char* right, char* op){
    OpNode_t* node = knob_temp_alloc(sizeof(OpNode_t));
    node->switched =0;
    node->left = knob_temp_strdup(left);
    node->right = knob_temp_strdup(right);
    node->op = knob_temp_strdup(op);
    node->last = node->next = NULL;
    if(list->count > 0){
        node->last = list->items[list->count-1];
        node->last->next = node;
    }
    knob_da_append(list,node);
}
void parse_and_calc(char* curr,size_t curr_len,char* result){
    OpNodeList list = {0};
    char num[64] = {0};
    int num_len = 0;
    double nums[2] = {0};
    char lastOp[2] = {0};
    for(int i=0; i < curr_len;++i){
        if(strcmp(num,SQR_RT) == 0){
            lastOp[1] = lastOp[0];
            lastOp[0] = 's';
            memset(num,0,num_len);
            num_len = 0;
        }
        if(IsOp(curr[i])){
            if(lastOp[0] != '\0' && num_len > 0){
                if(lastOp[0] != 's'){
                    addNode(&list,result,num,lastOp);
                    memset(result,0,strlen(result));
                    memset(num,0,num_len);
                }
                else {
                    double res = calc("",num,'s');
                    snprintf(result,MAX_RESULT_LENGTH,"%.9f",res);
                    lastOp[0] = lastOp[1];
                    lastOp[1] = '\0';
                    addNode(&list,"",num,lastOp);
                }
            }
            else if(result[0] == '\0'){
                strcpy(result,num);
            }
            lastOp[0] = curr[i];
            memset(num,0,num_len);
            num_len = 0;
        }
        else if((unsigned char)curr[i] == (unsigned char)L'²'){
            lastOp[0] = '+';
            double res = calc(num,"2",'^');
            snprintf(result,MAX_RESULT_LENGTH,"%.9f",res);
            addNode(&list,"",result,lastOp);
            memset(result,0,strlen(result));
            memset(num,0,num_len);
            num_len = 0;
            lastOp[0] = '\0';
        }
        else {
            num[num_len++] = curr[i];
        }
    }
    if(lastOp[0] != '\0'){
        if(lastOp[0] != 's'){
            addNode(&list,result,num,lastOp);
        }
        else {
            double res = calc("",num,'s');
            snprintf(result,MAX_RESULT_LENGTH,"%.9f",res);
            lastOp[0] = lastOp[1] != '\0' ? lastOp[1] : '+';
            lastOp[1] = '\0';
            addNode(&list,"",result,lastOp);
        }
        memset(result,0,strlen(result));
    }
    //@TODO: Using a doubly-linked list works, but it's overengineered IMO.
    // Have a look at doing this in a better way in the futur. Doing test's would invariably show that it doesn't always work
    // Also, it's not really easily readable...
    OpNode_t* first = NULL;    
    if(list.count > 0){
        first = list.items[0];
        OpNode_t* curr = first;
        while(curr != NULL){
            if(curr->next != NULL && OpPriority(curr->op[0],curr->next->op[0])){
                OpNode_t* priority = curr->next; 
                curr->next = priority->next;
                priority->next = curr;
                if(curr->last != NULL){
                    curr->last->next = priority;
                }
                curr->last = priority;
                if(curr == first){
                    first = priority;
                }
                if(!curr->switched){
                    priority->left = curr->right;
                    curr->right = curr->left;
                    curr->left = "";
                }
                curr->switched = 1;
            }
            else {
                curr = curr->next;
            }
        }
    }
    if(first != NULL){
        OpNode_t* curr = first;
        double res = 0.0;
        while(curr != NULL){
            char * left = curr->left;
            double lastRes = 0.0;
            if(knob_cstr_match(curr->left,"")){
                left = result; 
            }
            else {
                char* endptr;
                lastRes = strtod(result,&endptr);
            }
            res = calc(left,curr->right,curr->op[0]);
            snprintf(result,MAX_RESULT_LENGTH,"%.9f",res + lastRes);
            curr = curr->next;
        }
    }
    knob_temp_reset();
    //Filter out 0's
    int dotPos = -1;
    int i =0;
    while(result[i] != '\0'){
        if(result[i] == '.'){
            dotPos =i;
        }
        else if(dotPos > 0 && result[i] != '0'){
            dotPos = -1;
            break;
        }
        ++i;
    }
    i = dotPos;
    while(result[i] != '\0'){
        result[i] = '\0';
        ++i;
    }
}
void plug_update(void){
    static mouse_t last_mouse = {0}; 
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
    char result[256] = {0};
    for(int i =0; i < p->curr_cal.count;++i){
        assert(i != 256);
        result[i] = p->curr_cal.items[i];
    }
    rencache_draw_text(p->fonts[BIG_FONT],result,show_rect.width - cal_w,show_rect.y+show_rect.height-(cal_h+padding),text_color);
    int grid_y = 0;
    float curr_x = show_rect.x;
    #define NUM_BUTTS 5
    float b_pad = padding * 0.15f;
    float butt_w = p->fen.width / NUM_BUTTS - (b_pad *  NUM_BUTTS);
    float current_y = show_rect.height+show_rect.y+padding;
    int button_text_h = ren_get_font_height(p->fonts[BASIC_FONT]) * 0.5f;
    char* pi = PI;

    for(int i =0; i < NUM_BUTTS;){
        char* text = butt_content[grid_y][i];
        if(text[0] == '\0'){
            goto ITERATE;
        }
        float butt_h = butt_w; 
        RenColor butt_col = button_col;
        if(text[0] == '='){
            butt_h = butt_h * 2.0f + b_pad * 3.0f;
            butt_col = butt_equl_col;
        }
        RenRect butt_rect = {.x=curr_x + i*(butt_w+b_pad*2.5),.y=grid_y*(butt_w+b_pad*2.5)+current_y,.width=butt_w,.height=butt_h};
        if(mouse_on_rect(mouse,butt_rect)){
            butt_col = button_hovered_col;
            if(mouse.l && last_mouse.l != mouse.l){
                butt_col = button_click_col;
                if(text == CLEAR){
                    memset(p->curr_cal.items,0,sizeof(char) * p->curr_cal.count);
                    p->curr_cal.count = 0;
                    knob_sb_append_cstr(&p->curr_cal,"0");
                }
                else if(text == OP_PAREN){
                    int count = p->curr_cal.count;
                    while(count - 1 > -1 && (isdigit(p->curr_cal.items[count-1]) || p->curr_cal.items[count-1] == '.')){
                        count--;
                    }
                    char temp[256] = {0};
                    int c = 0;
                    while(count+c < p->curr_cal.count){
                        temp[c] = p->curr_cal.items[count+c];
                        c++;
                    }
                    p->curr_cal.count = count;
                    knob_sb_append_cstr(&p->curr_cal,OP_PAREN);
                    knob_sb_append_cstr(&p->curr_cal,temp);
                }
                else if(text == CL_PAREN){
                    int isDigit = isdigit(p->curr_cal.items[p->curr_cal.count-1]);
                    int hasOpen =0;
                    int count = 0;
                    while (count < p->curr_cal.count){
                        if(p->curr_cal.items[count] == '('){
                            hasOpen = 1;
                            break;
                        }
                        count++;
                    }
                    if(!isDigit){
                        //@TODO: Add error message
                    }
                    else if(!hasOpen){
                        //@TODO: Add error message
                    }
                    else {
                        knob_sb_append_cstr(&p->curr_cal,CL_PAREN);
                    }
                }
                else if(text == EQUAL){
                    char c = p->curr_cal.items[p->curr_cal.count-1];
                    int isDigit = isdigit(c);
                    int hasOp = 0;
                    int count = 0;
                    char sub[4] = {0};
                    while (count < p->curr_cal.count){
                        c = p->curr_cal.items[count];
                        if(IsOp(c)){
                            hasOp = 1;
                            // break;
                        }
                        else if((unsigned char)c == (unsigned char)L'²' && count >= 2){
                            hasOp = 1;
                            isDigit = isdigit(p->curr_cal.items[count-2]);
                            break;
                        }
                        else if(strcmp(sub,SQR_RT) == 0 && count >= 3){
                            hasOp = 1;
                            isDigit = p->curr_cal.count > count && isdigit(p->curr_cal.items[count]);
                            break;
                        }
                        int sub_len = strlen(sub);
                        if(sub_len < 4){
                            sub[sub_len] = c;
                        }
                        count++;
                    }
                    if(!isDigit){
                        //@TODO: Add error message
                    }
                    else if(!hasOp){
                        //@TODO: Add error message
                    }
                    else {
                        //@TODO: Append history
                        char res[MAX_RESULT_LENGTH] = {0}; 
                        parse_and_calc(p->curr_cal.items,p->curr_cal.count, res);
                        memset(p->curr_cal.items,0,sizeof(char) * p->curr_cal.count);
                        p->curr_cal.count = 0;
                        knob_sb_append_cstr(&p->curr_cal,res);
                    }
                }
                else {
                    int hasDot = 0;
                    if(text == DOT){
                        for(int y=0; y < p->curr_cal.count; ++y){
                            if(p->curr_cal.items[y] == '.'){
                                hasDot = 1;
                                break;
                            }
                        }
                    }
                    if(text == SQUARED){
                        char lastC = p->curr_cal.items[p->curr_cal.count-1];
                        if(!isdigit(lastC)){
                            goto ITERATE;
                        }
                        text = &text[1];
                    }
                    if((IsOp(p->curr_cal.items[p->curr_cal.count-1]) && IsOp(text[0])) || hasDot ){
                        goto ITERATE;
                    }
                    if(text == EXPONENT){
                        text = "^";
                    }
                    if(p->curr_cal.count == 1 && p->curr_cal.items[0] == '0' && text != DOT && isdigit(text[0])){
                        p->curr_cal.count = 0;
                    }
                    if(text == SQR_RT){
                        char lastC = p->curr_cal.items[p->curr_cal.count-1];
                        if(!isdigit(lastC)){
                            goto ITERATE;
                        }
                        char num[64] = {0};
                        int lastCount = p->curr_cal.count;
                        char op = '\0';
                        while(p->curr_cal.count > 0 && !IsOp(op)){
                            op = p->curr_cal.items[--p->curr_cal.count];
                        }
                        if(IsOp(op)){
                            p->curr_cal.count++;
                        }
                        memcpy(num,&p->curr_cal.items[p->curr_cal.count],lastCount);
                        knob_sb_append_cstr(&p->curr_cal,text);
                        knob_sb_append_cstr(&p->curr_cal,num);
                    }
                    else {
                        knob_sb_append_cstr(&p->curr_cal,text);
                    }
                }
            }
        }
        rencache_draw_rect(butt_rect,butt_col);
        int button_text_w = ren_get_font_width(p->fonts[BASIC_FONT],text,strlen(text)) * 0.5f; 
        rencache_draw_text(p->fonts[BASIC_FONT],text,
            butt_rect.x+butt_rect.width*0.5f - button_text_w,//x
            butt_rect.y+butt_rect.height*0.5f - button_text_h, //y
            text_color
        );
        ITERATE:
        ++i;
        if(i >= NUM_BUTTS && grid_y + 1 < NUM_BUTTS){
            grid_y++;
            i=0;
        }
    }

    RenRect rect = {0};
    rect.x = 100;
    rect.y = 100;
    rect.width = 200;
    rect.height = 200;
    RenColor red = {0};
    red.r = red.a = 255;
    
    rencache_end_frame();
    last_mouse = mouse;
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