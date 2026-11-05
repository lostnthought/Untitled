#ifndef UI_H
#define UI_H

#include "utilities/utilities.h"
#include "SDL2/SDL.h"


#define MAX_UI_ELEMENTS 1024
#define MAX_UI_CONTAINERS 1024
#define UI_SCALE_WIDTH 2560
#define UI_SCALE_HEIGHT 1920
#define MAX_TEXT_BOXES 1024
#define MAX_TEXT_BOX_CHARACTERS 256
#define MAX_UI_CHILDREN 1024
#define MAX_GLYPHS 256

typedef struct {
    int id;
    float u0, v0, u1, v1;
    int width, height;
    int xoffset, yoffset;
    int xadvance;
} Glyph;

typedef struct Font {

    int id;
    uint32_t texture;
    Glyph * glyphs;
    size_t glyph_count;
    int tex_w, tex_h;
    int highest_y; // used to determine the amount of space to cursor shift for newlines
    int highest_dist;
    size_t image_id;
    char name[256];

} Font;

typedef struct TextBoxGPU
{
    uint32_t visible;
    uint32_t hovered;
    uint32_t shadow;
    float shadow_dist;
    uint32_t container_id;
    uint32_t mask;
    vec2 shadow_dir;
    uint32_t state;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    vec4 color;
    vec4 fill;
    mat4 model;
    
} TextBoxGPU;

typedef struct UIElementGPU
{
    uint32_t visible;
    uint32_t hovered;
    uint32_t shadow;
    float shadow_dist;
    vec2 pos;
    vec2 size;
    vec2 shadow_dir;
    uint32_t state;
    uint32_t type;
    vec4 color;
    vec4 fill;
    vec4 padding;
    vec2 translation;
    vec2 scale;
    float radius;
    uint32_t masked;
    uint32_t container_id;
    uint32_t pad0;
    mat4 model;
    
    
} UIElementGPU;

typedef struct UIElement UIElement;

typedef enum Justify
{

    JUSTIFY_CENTER,
    JUSTIFY_LEFT,
    JUSTIFY_RIGHT,
    JUSTIFY_TOP_CENTER,
    JUSTIFY_TOP_LEFT,
    JUSTIFY_TOP_RIGHT,
    JUSTIFY_BOTTOM_CENTER,
    JUSTIFY_BOTTOM_LEFT,
    JUSTIFY_BOTTOM_RIGHT,
    JUSTIFY_MAX
   
} Justify;

static const char * JUSTIFY_NAMES[] =
{
    "JUSTIFY_CENTER",
    "JUSTIFY_LEFT",
    "JUSTIFY_RIGHT",
    "JUSTIFY_TOP_CENTER",
    "JUSTIFY_TOP_LEFT",
    "JUSTIFY_TOP_RIGHT",
    "JUSTIFY_BOTTOM_CENTER",
    "JUSTIFY_BOTTOM_LEFT",
    "JUSTIFY_BOTTOM_RIGHT"
};

typedef enum UIState
{
    UISTATE_NONE,
    UISTATE_HOVERED,
    UISTATE_PRESSED,
    UISTATE_SELECTED
} UIState;

typedef enum UIEditorState
{
    UIEDITORSTATE_NONE,
    UIEDITORSTATE_HOVERED,
    UIEDITORSTATE_SELECTED
} UIEditorState;

typedef enum UIInteractableType
{
    UI_INTERACTABLE_NONE,
    UI_INTERACTABLE_SLIDER,
    UI_INTERACTABLE_DROPDOWN,
    UI_INTERACTABLE_LABEL,
    UI_INTERACTABLE_MAX
} UIInteractableType;



typedef struct UIContainer
{

    int uID;
    bool free;

    vec2 scroll_offset;
    vec2 delta_scroll_offset;
    vec2 scroll_dir;

    bool wrap;
    vec4 padding; // left, right, up, down
    vec2 min, max;

    bool mask;
    
    int children[MAX_UI_CHILDREN];
    size_t child_count;
    int box_children[MAX_UI_CHILDREN];
    size_t box_count;

    int element;
    int scrollbar;
    char id[256];
    
} UIContainer;

typedef struct TextBox
{

    bool hovered;
    bool shadow;
    bool visible;
    bool free;
    int font_id;
    uint32_t uID;
    Justify justify;
    char * text;
    int text_cap;
    float text_size;
    float w;
    float h;
    float angle;
    float shadow_dist;
    vec2 shadow_dir;
    vec2 translation;
    vec2 scale;
    vec2 center;
    vec2 start;
    vec4 color;
    vec4 fill;
    uint32_t layer;
    UIState state;
    
    int menu;
    int parent_container;
    char id[256];
    
} TextBox;


typedef enum UIElementType
{
    UIELEMENT_RECT, UIELEMENT_CIRCLE, UIELEMENT_TYPE_MAX
} UIElementType;


typedef enum UIValueType
{

    UIVAL_NONE,
    UIVAL_BOOL,
    UIVAL_INT,
    UIVAL_FLOAT,
    UIVAL_CALLBACK
    
} UIValueType;

typedef enum UIValueBehavior
{

    UIVALBEHAVIOR_NONE,
    UIVALBEHAVIOR_WRAP,
    UIVALBEHAVIOR_CLAMP
    
} UIValueBehavior;


typedef struct {
    
    UIValueType type;
    UIValueBehavior behavior;
    union
    {
        int min_int;
        float min_flt;
    } min_val;
    union
    {
        int max_int;
        float max_flt;
    } max_val;
    union
    {
        int inc_int;
        float inc_flt;
    } inc_val;
    void * data;
    
} UIValue;

typedef struct {

    bool bind_self;
    bool one_shot;
    UIValue val;
    void (*action_fn)(void *);
    void (*unaction_fn)(void *);
    void * (*bind_fn)(void*, void *);
    void *data;
    void (*free_data)(void*);
    char id[256];
    
} UICallback;



static const char * UI_INTERACTABLE_NAMES[] =
{
    "UI_INTERACTABLE_NONE",
    "UI_INTERACTABLE_SLIDER",
    "UI_INTERACTABLE_DROPDOWN",
    "UI_INTERACTABLE_LABEL"
};


typedef struct UISlider {

    vec2 dir;
    vec2 min, max;
    float len, val, prev_val;
    float min_val, max_val;
    float scroll_scale;
    bool dragging;
    vec2 drag_mouse_start;
    vec2 drag_center_start;
    float drag_start_scalar;
    float max_proj;

    bool draw_slider_extent;
    float extent_width;
    int container;
    
} UISlider;

typedef struct UIDropdown {
    
    // int selected_index;
    // struct TextBox * selected_option;
    // char selected_option_id[128];
    // char options[MAX_DROPDOWN_ELEMENTS][MAX_CHARACTERS];
    // size_t option_count;
    // bool expanded;
    // struct UIElement * children[MAX_DROPDOWN_ELEMENTS];
    // size_t child_count;

    // TextAlign alignment;
    // float text_size;
    // int font_id;
    // RGBA color;
    // RGBA fill;
    // int render_group_id;
    // int material_id;
    

} UIDropdown;

typedef struct UILabel {

    int box;
    Justify align;
    int parent_dropdown;
    
} UILabel;

struct UIElement
{

    bool shadow;
    bool visible;
    bool free;
    UIElementType type;
    uint32_t uID;
    float angle;
    float shadow_dist;
    float radius;
    vec2 shadow_dir;
    vec2 translation;
    vec2 base_pos;
    vec2 scale;
    vec2 center;
    vec2 size;
    vec4 color;
    vec4 fill;

    uint32_t layer; // lower = lowest priority, drawn first
    UICallback action_callback;
    UICallback hover_callback;
    bool already_hovered;
    UIState state;
    UIEditorState editor_state;

    int menu;
    int container;
    int parent_container;

    char id[256];

    UIInteractableType interactable;
    union
    {
        UIDropdown dropdown;
        UILabel label;
        UISlider slider;
    } as;
    
};

#define MAX_UI_ELEMENTS_PER_MENU 1024

typedef struct UIMenu
{
    
    uint32_t uID;
    char id[256];
    UIElement elements[MAX_UI_ELEMENTS];
    TextBox text_boxes[MAX_TEXT_BOXES];
    UIContainer containers[MAX_UI_CONTAINERS];
    
} UIMenu;




static UICallback DEFAULT_CALLBACK =
{
    .id = "empty",
    .action_fn = NULL,
    .unaction_fn = NULL,
    .bind_fn = NULL,
    .bind_self = true,
    .data = NULL,
    .free_data = NULL,
    .val.data = NULL,
    .val.type = UIVAL_NONE,
    .one_shot = false
};

static UIElement DEFAULT_UI_ELEMENT=
{
    .free = 0,
    .color = (vec4){0,0,1,1},
    .fill = (vec4){0,0,0,0},
    .size = (vec2){0.5,0.5},
    .center = (vec2){0,0},
    .uID = 0,
    .angle = 0,
    .scale = (vec2){50,50},
    .shadow_dir = (vec2){0.5, 0.5},
    .shadow = true,
    .shadow_dist = 20,
    .translation = (vec2){20,100},
    .base_pos = (vec2){20,100},
    .type = UIELEMENT_RECT,
    .radius = 0.0f,
    .visible = true,
    .layer = 0,
    .already_hovered = false,
    .state = UISTATE_NONE,
    .container = -1,
    .parent_container = -1,
    .id = "default",
    .interactable = UI_INTERACTABLE_NONE
};

static UISlider DEFAULT_SLIDER =
{

    .container = -1,
    .len = 100,
    .dir = (vec2){0, 1},
    .scroll_scale = 1,
    .min_val = 0,
    .max_val = 100
    
};

static UILabel DEFAULT_LABEL =
{

    .box = -1,
    .parent_dropdown = -1,
    .align = JUSTIFY_CENTER
    
};


static UIContainer DEFAULT_UI_CONTAINER =
{
    .free = 0,
    .uID = -1,
    .scroll_offset = (vec2){0,0},
    .delta_scroll_offset = (vec2){0,0},
    .scroll_dir = (vec2){0,1},
    .wrap = false,
    .padding = (vec4){0,0,0,0},
    .min = (vec2){0,0},
    .max = (vec2){0,0},
    .mask  = true,
    .children = {0},
    .child_count = 0,
    .box_children = {0},
    .box_count = 0,
    .element = -1,
    .scrollbar = -1,
    .id = "default"
};

static TextBox DEFAULT_TEXT_BOX =
{
    .free = 0,
    .color = (vec4){1,1,1,1},
    .fill = (vec4){0,0,0,0},
    .font_id = 0,
    .w = 500,
    .h = 500,
    .start = (vec2){0,0},
    .center = (vec2){0,0},
    .uID = 0,
    .text_size = 0.6,
    .angle = 0,
    .scale = (vec2){1,1},
    .shadow_dir = (vec2){0.5, -0.5},
    .shadow = true,
    .shadow_dist = 20,
    .translation = (vec2){0,0},
    .text = NULL,
    .justify = JUSTIFY_CENTER,
    .visible = true,
    .parent_container = -1,
    .menu = -1,
    .id = "default",
    .text_cap = 0,
    .layer = 0
};


int move_element_to_menu(int element_idx, int current_menu, int target_menu);
int move_text_box_to_menu(int tb_idx, int current_menu, int target_menu);
int get_text_box_from_coords(vec2 pos, int * menu_idx);
int get_ui_element_from_coords(vec2 pos, int * menu_idx);
int create_ui_element(int menu_id);
void handle_ui_event(SDL_Event event, int mouse_x, int mouse_y);
void update_ui_state(float mouse_x, float mouse_y);
UIContainer * create_ui_container(UIElement * el, int menu_id);
void delete_ui_container(UIElement * el);
void delete_ui_element(int menu, int id);
void compute_slider_bounds(UIElement * element);
void create_slider(UIElement * element, vec2 dir, float len);
void remove_element_from_container(UIContainer * c, UIElement * el);
void remove_text_box_from_container(UIContainer * c, TextBox * tb);
int find_ui_element_in_menu(UIMenu * m, const char * id);
int find_text_box_in_menu(UIMenu * m, const char * id);
void align_text_box_label(UIElement * el);
void assign_element_to_container(UIElement * el, UIContainer * c);
void assign_text_box_to_container(TextBox * tb, UIContainer * c);
int find_ui_element_in_menu(UIMenu * m, const char * id);
int find_text_box_in_menu(UIMenu * m, const char * id);
int find_container_in_menu(UIMenu * m, const char * id);
void reassign_elements_to_container(UIElement * el);


extern int TEXT_WIDTH;
extern int TEXT_HEIGHT;
extern Vector MENUS;


static inline void UI_INIT()
{

    if (MENUS.buffer) free(MENUS.buffer);
    init_vector(&MENUS, 0, sizeof(UIMenu));

}

static inline UIMenu * create_menu() {

    UIMenu * menu = vector_expand(&MENUS, 1);

    for (int i = 0; i < MAX_UI_ELEMENTS; i++)
    {
        menu->elements[i].free = true;
    }
    for (int i = 0; i < MAX_TEXT_BOXES; i++)
    {
        menu->text_boxes[i].free = true;
    }
    for (int i = 0; i < MAX_UI_CONTAINERS; i++)
    {
        menu->containers[i].free = true;
    }
    
    menu->uID = MENUS.count-1;
    menu->id[0] = '\0';

    return menu;
}

static inline bool container_is_valid(int container, int menu)
{
    if (menu < 0 || menu >= MENUS.count) return false;

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[menu];
    
    if (container >= 0 && container < MAX_UI_CONTAINERS)
    {

        UIContainer * conts = m->containers;

        UIContainer * c = &conts[container];
        if (c->uID != -1 && !c->free)
        {
            return true;
        }
        
    }

    return false;
    
}
static inline vec2 screenspace_to_ui(vec2 pos)
{
    return (vec2)
    {
        pos.x * ((float)UI_SCALE_WIDTH / TEXT_WIDTH),
        pos.y * ((float)UI_SCALE_HEIGHT / TEXT_HEIGHT)
    };
}



#endif
