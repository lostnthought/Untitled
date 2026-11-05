
#include "utilities/types.h"
#include "utilities/utilities.h"
#include <math.h>
#include <omp.h>
#include "ui/ui.h"
#include "ui/fonts.h"



int TEXT_WIDTH = 1280;
int TEXT_HEIGHT = 720;
Vector MENUS = {0};


int find_empty_element_slot(UIMenu * m)
{
    for (int i = 0; i < MAX_UI_ELEMENTS; i++)
    {

        if (m->elements[i].free) return i;
        
    }
    return -1;
}

int find_empty_container_slot(UIMenu * m)
{
    for (int i = 0; i < MAX_UI_CONTAINERS; i++)
    {

        if (m->containers[i].free) return i;
        
    }
    return -1;
}


int create_ui_element(int menu_id)
{
    
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);

    UIMenu * m = &ms[menu_id];

    

    int empty = find_empty_element_slot(m);

    if (empty == -1)
    {
        printf("No empty element slot found\n");
        return -1;
    }
    UIElement * el = &m->elements[empty];
    *el = DEFAULT_UI_ELEMENT;
    el->uID = empty;
    el->action_callback = DEFAULT_CALLBACK;
    el->hover_callback = DEFAULT_CALLBACK;
    el->menu = menu_id;
    
    return empty;
}




void delete_ui_container(UIElement * el)
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    if (el->menu == -1) return;
    UIMenu * m = &ms[el->menu];

    UIContainer * cs = m->containers;
    UIContainer * c = &cs[el->container];

    UIElement * els = m->elements;

    for (int i = 0; i < MAX_UI_ELEMENTS; i++)
    {
        UIElement * el = &els[i];
        if (el->free) continue;
        if (el->parent_container == c->uID)
        {
            el->parent_container = -1;
        }
    }

    TextBox * boxes = m->text_boxes;

    for (int i = 0; i < MAX_TEXT_BOXES; i++)
    {
        TextBox * box = &boxes[i];
        if (box->free) continue;
        if (box->parent_container == c->uID)
        {
            box->parent_container = -1;
        }
    }

    
    el->container = -1;

    
    *c = DEFAULT_UI_CONTAINER;
    c->free = true;
    
    // write function that shifts indices on deletion. for now this will just create stale entries but shouldn't matter for now

}


void reassign_elements_to_container(UIElement * el)
{
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);

    UIMenu * m = &ms[el->menu];
    if (el->container == -1) return;
    UIContainer * c = &m->containers[el->container];

    for (int i = 0; i < c->child_count; i++)
    {
        UIElement * e = &m->elements[c->children[i]];
        e->parent_container = c->uID;
    }

    for (int i = 0; i < c->box_count; i++)
    {
        TextBox * tb = &m->text_boxes[c->box_children[i]];
        tb->parent_container = c->uID;
    }
    
}




void remove_element_from_container(UIContainer * c, UIElement * el)
{
    int id = -1;
    for (int i = 0; i < c->child_count; i++)
    {
        
        if (c->children[i] == el->uID)
        {
            id = i;
        }
    }
    if (id != -1)
    {
        REMOVE_FROM_ARR(c->children, id, &c->child_count);
    }
    el->parent_container = -1;
    
}
void remove_text_box_from_container(UIContainer * c, TextBox * tb)
{
    int id = -1;
    for (int i = 0; i < c->box_count; i++)
    {
        
        if (c->box_children[i] == tb->uID)
        {
            id = i;
        }
    }
    if (id != -1)
    {
        REMOVE_FROM_ARR(c->box_children, id, &c->box_count);
    }
    
}


void delete_ui_element(int menu, int id)
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[menu];

    UIElement * el = &m->elements[id];
    if (el->parent_container != -1)
    {
        remove_element_from_container(&m->containers[el->parent_container], el);
    }
    if (el->container != -1)
    {
        delete_ui_container(el);
    }

    // handle label here
    
    el->free = true;
}

UIContainer * create_ui_container(UIElement * el, int menu_id)
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[menu_id];


    
    int empty = find_empty_container_slot(m);
    if (empty == -1)
    {
        printf("No empty container slot found\n");
        return NULL;
    }

    UIContainer * container = &m->containers[empty];

    *container = DEFAULT_UI_CONTAINER;
    container->uID = empty;
    el->container = container->uID;
    container->element = el->uID;
    
    return container;

}


void scroll_container(UIContainer * c, UISlider * slider, UIMenu * m)
{
    
    vec2 new_scroll_offset = vec2_scale((vec2){c->scroll_dir.x, c->scroll_dir.y}, lerp(slider->min_val, slider->max_val, slider->val) * slider->scroll_scale);


    c->delta_scroll_offset = vec2_sub(new_scroll_offset, c->scroll_offset);
    c->scroll_offset = new_scroll_offset;
    
    UIElement * els = m->elements;
    for(int i = 0; i < c->child_count; i++) {

        int el_idx = c->children[i];
        UIElement * el = &els[el_idx];
        el->translation = vec2_add(el->base_pos, c->scroll_offset);
    }
}

int find_ui_element_in_menu(UIMenu * m, const char * id)
{

    UIElement * els = m->elements;
    for (int i = 0; i < MAX_UI_ELEMENTS; i++)
    {

        UIElement * el = &els[i];
        if (el->free) continue;

        if (strcmp(el->id, id) == 0)
        {
            return el->uID;
        }
        
    }
    return -1;
}

int find_text_box_in_menu(UIMenu * m, const char * id)
{

    TextBox * tbs = m->text_boxes;
    for (int i = 0; i < MAX_TEXT_BOXES; i++)
    {

        TextBox * tb = &tbs[i];
        if (tb->free) continue;

        if (strcmp(tb->id, id) == 0)
        {
            return tb->uID;
        }
        
    }
    return -1;
}

int find_container_in_menu(UIMenu * m, const char * id)
{

    UIContainer * cs = m->containers;
    for (int i = 0; i < MAX_UI_CONTAINERS; i++)
    {

        UIContainer* c = &cs[i];
        if (c->free) continue;

        if (strcmp(c->id, id) == 0)
        {
            return c->uID;
        }
        
    }
    return -1;
}

int move_element_to_menu(int element_idx, int current_menu, int target_menu)
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    if (current_menu == -1 || element_idx == -1 || target_menu == -1) return -1;
    UIMenu * m = &ms[current_menu];
    UIElement * el = &m->elements[element_idx];
    UIMenu * target = &ms[target_menu];

    int target_slot = find_empty_element_slot(target);
    if (target_slot == -1) return -1;
    if (el->container != -1)
    {
        delete_ui_container(el);
    }

    UIElement * new_el = &target->elements[target_slot];
    int uID = new_el->uID;
    memcpy(new_el, el, sizeof(UIElement));

    // need to maybe copy the text box if label to new menu
    new_el->parent_container = -1;
    new_el->menu = target_menu;
    new_el->uID = target_slot;
    
    delete_ui_element(current_menu, element_idx);
    return target_slot;
    
}

int move_text_box_to_menu(int tb_idx, int current_menu, int target_menu)
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    if (current_menu == -1 || tb_idx == -1 || target_menu == -1) return -1;

    UIMenu * m = &ms[current_menu];
    TextBox * tb = &m->text_boxes[tb_idx];
    UIMenu * target = &ms[target_menu];
    int target_slot = find_empty_text_box_slot(target);
    if (target_slot == -1) return -1;

    TextBox * new_tb = &target->text_boxes[target_slot];
    int uID = new_tb->uID;
    memcpy(new_tb, tb, sizeof(TextBox));

    if (tb->text)
    {
        new_tb->text = malloc(sizeof(char) * tb->text_cap);
        memcpy(new_tb->text, tb->text, sizeof(char) * tb->text_cap);
    }
    else
    {
        new_tb->text = NULL;
        new_tb->text_cap = 0;
    }

    delete_text_box(current_menu, tb_idx);
    new_tb->menu = target_menu;
    new_tb->uID = target_slot;


    return target_slot;
    
    
}


void assign_element_to_container(UIElement * el, UIContainer * c)
{

    bool already_assigned = false;
    for (int j = 0; j < c->child_count; j++)
    {
        if (c->children[j] == el->uID)
        {
            already_assigned = true;
            break;
        }
        
    }
    if (!already_assigned)
    {
        c->children[c->child_count++] = el->uID;
    }
    el->parent_container = c->uID;
    
}

void assign_text_box_to_container(TextBox * tb, UIContainer * c)
{

    bool already_assigned = false;
    for (int j = 0; j < c->box_count; j++)
    {
        if (c->box_children[j] == tb->uID)
        {
            already_assigned = true;
            break;
        }
        
    }
    if (!already_assigned)
    {
        c->box_children[c->box_count++] = tb->uID;
    }
    tb->parent_container = c->uID;
    
}


void align_text_box_label(UIElement * el)
{
    
    UILabel * l = &el->as.label;
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[el->menu];
    TextBox * tb = &m->text_boxes[l->box];
    float hw = tb->w/2;
    float hh = tb->h/2;
    float el_hw = el->scale.x;
    float el_hh = el->scale.y;

    float scale_x = (float)TEXT_WIDTH / UI_SCALE_WIDTH;
    float scale_y = (float)TEXT_HEIGHT / UI_SCALE_HEIGHT;
    float top_start = el->translation.y - el_hh;
    float bottom_start = el->translation.y + el_hh;
    float left_start = el->translation.x - el_hw;
    float right_start = el->translation.x + el_hw;
    float left_tr = left_start + hw;
    float right_tr = right_start - hw;
    float top_tr = top_start;
    float bottom_tr = bottom_start;

    
    switch(l->align)
    {
        case JUSTIFY_TOP_LEFT:
            tb->translation = (vec2){left_tr, top_tr};
            break;
        case JUSTIFY_TOP_CENTER:
            tb->translation = (vec2){el->translation.x, top_tr};
            break;
        case JUSTIFY_TOP_RIGHT:
            tb->translation = (vec2){right_tr, top_tr};
            break;
        case JUSTIFY_LEFT:
            tb->translation = (vec2){left_tr, el->translation.y};
            break;
        case JUSTIFY_CENTER:
            tb->translation = el->translation;
            break;
        case JUSTIFY_RIGHT:
            tb->translation = (vec2){right_tr, el->translation.y};
            break;
        case JUSTIFY_BOTTOM_LEFT:
            tb->translation = (vec2){left_tr, bottom_tr};
            break;
        case JUSTIFY_BOTTOM_CENTER:
            tb->translation = (vec2){el->translation.x, bottom_tr};
            break;
        case JUSTIFY_BOTTOM_RIGHT:
            tb->translation = (vec2){right_tr, bottom_tr};
            break;
        default:
            break;
    }

    
}

bool point_inside_ui_element(UIElement * el, vec2 pos)
{
    

    float d = 0.0f;
    switch(el->type)
    {
        case UIELEMENT_RECT:
            d = SDF_rect(pos, el->center, el->size, el->radius);
            return d <= 0.0f;
            break;
        case UIELEMENT_CIRCLE:
            d = SDF_circle(pos, el->center, el->radius);
            return d <= 0.0f;
            break;
        default:
            return false;
            break;
    }
    
}

vec2 screenspace_to_ui_element_local(UIElement * el, float scale_x, float scale_y, vec2 pos)
{
    
    vec2 text_tr = el->translation;
    text_tr.x *= scale_x;
    text_tr.y *= scale_y;
    vec2 local = vec2_sub(pos, text_tr);
    local.x *= 1.0f/el->scale.x;
    local.y *= 1.0f/el->scale.y;
    return local;
}



bool is_hovering_ui_element(UIElement * el, vec2 pos)
{
    float scale_x = (float)TEXT_WIDTH / UI_SCALE_WIDTH;
    float scale_y = (float)TEXT_HEIGHT / UI_SCALE_HEIGHT;
    vec2 local = screenspace_to_ui_element_local(el, scale_x, scale_y, pos);
    return point_inside_ui_element(el, local);
    
}

int get_text_box_from_coords(vec2 pos, int * menu_idx)
{
    
    float scale_x = (float)TEXT_WIDTH / UI_SCALE_WIDTH;
    float scale_y = (float)TEXT_HEIGHT / UI_SCALE_HEIGHT;
    
    int highest_layer = -INFINITY;
    int highest_tb_index = -1;

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);

    vec2 ui_coords = screenspace_to_ui(pos);
    // vec2 ui_coords = pos;
    for (int i = 0; i < MENUS.count; i++)
    {
        
        UIMenu * m = &ms[i];
        TextBox * tbs = m->text_boxes;
        for (int j = 0; j < MAX_TEXT_BOXES; j++)
        {
            TextBox * tb = &tbs[j];
            if (!tb->visible || tb->free) continue;

            vec2 scale = tb->scale;
            // vec2 start = tb->start;
            vec2 start = screenspace_to_ui(tb->start);
            start.x *= tb->scale.x;
            start.y *= tb->scale.y;
            // vec2 bounds = (vec2){tb->w, tb->h};
            vec2 bounds = screenspace_to_ui((vec2){tb->w, tb->h});

            bounds.x *= tb->scale.x;
            bounds.y *= tb->scale.y;
            vec2 min = vec2_add(start, tb->translation);
            vec2 max = vec2_add(min, bounds);
            bool inside = vec2_inside_bounds(ui_coords, min, max);
            if (inside)
            {
                
                if (highest_layer < (int)tb->layer)
                {
                    highest_layer = tb->layer;
                    highest_tb_index = j;
                    *menu_idx = i;
                }
            }
            
        }
    }

    return highest_tb_index;
    
}


int get_ui_element_from_coords(vec2 pos, int * menu_idx)
{
    
    
    float scale_x = (float)TEXT_WIDTH / UI_SCALE_WIDTH;
    float scale_y = (float)TEXT_HEIGHT / UI_SCALE_HEIGHT;
    
    int highest_layer = -INFINITY;
    int highest_element_index = -1;

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
            
    for (int i = 0; i < MENUS.count; i++)
    {
        
        UIMenu * m = &ms[i];
        UIElement * els = m->elements;
        for (int j = 0; j < MAX_UI_ELEMENTS; j++)
        {

            UIElement * el = &els[j];
            if (!el->visible || el->free) continue;

            vec2 local = screenspace_to_ui_element_local(el, scale_x, scale_y, pos);
            if (point_inside_ui_element(el, local))
            {

                if (highest_layer < (int)el->layer)
                {
                    highest_layer = el->layer;
                    highest_element_index = j;
                    *menu_idx = i;
                }

            }
        
        }
    }

    return highest_element_index;
}
void process_ui_event(UICallback * callback)
{

    UIValue * val = &callback->val;
    switch(callback->val.type)
    {
        case UIVAL_NONE:
            break;
        case UIVAL_BOOL:
            *(bool*)callback->val.data = !(*(bool*)callback->val.data);
            break;
        case UIVAL_INT:
            {
                switch(val->behavior)
                {
                    case UIVALBEHAVIOR_NONE:
                        *(int*)val->data = (*(int*)val->data) + val->inc_val.inc_int;
                        break;
                    case UIVALBEHAVIOR_WRAP:
                        *(int*)val->data = ((*(int*)val->data) + val->inc_val.inc_int + val->max_val.max_int) % val->max_val.max_int;
                        break;
                    case UIVALBEHAVIOR_CLAMP:
                        *(int*)val->data = MAX(MIN(((*(int*)val->data) + val->inc_val.inc_int), val->max_val.max_int), val->min_val.min_int);
                        break;
                }
            }
            break;
        case UIVAL_FLOAT:
            {
                
                switch(val->behavior)
                {
                    case UIVALBEHAVIOR_NONE:
                        *(float*)val->data = ((*(float*)val->data) + val->inc_val.inc_flt);
                        break;
                    case UIVALBEHAVIOR_WRAP:
                        *(float*)val->data = fmodf(((*(float*)val->data) + val->inc_val.inc_flt + val->max_val.max_flt), val->max_val.max_flt);
                        break;
                    case UIVALBEHAVIOR_CLAMP:
                        *(float*)val->data = fmaxf(fminf(((*(float*)val->data) + val->inc_val.inc_flt), val->max_val.max_flt), val->min_val.min_flt);
                        break;
                }
            }
            break;
        case UIVAL_CALLBACK:
            if (callback->action_fn) {
                callback->action_fn(callback->val.data);
            }
            break;
    }
}

void compute_slider_bounds(UIElement * element){
    UISlider * slider = &element->as.slider;
    
    float r = slider->len / 2;
    slider->min = vec2_sub(element->translation, vec2_scale(slider->dir, r));
    slider->max = vec2_add(element->translation, vec2_scale(slider->dir, r));
    slider->max_proj = vec2_length(vec2_sub(slider->max, slider->min));
    
}

void create_label(UIElement * element, int box, int parent_dropdown, Justify justify)
{

    element->as.label.box = box;
    element->as.label.parent_dropdown = parent_dropdown;
    element->as.label.align = justify;

}

void create_slider(UIElement * element, vec2 dir, float len)
{

    element->as.slider.container = -1;
    element->interactable = UI_INTERACTABLE_SLIDER;
    element->as.slider.len = len;
    element->as.slider.dir = dir;
    element->as.slider.scroll_scale = 1;
    element->as.slider.min_val = 0;
    element->as.slider.max_val = 100;
    compute_slider_bounds(element);

}

void handle_ui_event(SDL_Event event, int mouse_x, int mouse_y)
{

    int menu_idx = -1;
    int highest_ui_element_idx = get_ui_element_from_coords((vec2){mouse_x, mouse_y}, &menu_idx);

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);


    UIElement * highest_el = NULL;
    if (menu_idx != -1 && highest_ui_element_idx != -1)
    {
        UIElement * els = ms[menu_idx].elements;
        highest_el = &els[highest_ui_element_idx];
    }

    
    for (int i = 0; i < MENUS.count; i++)
    {
        UIMenu * m = &ms[i];

        UIElement * els = m->elements;
        
        for (int j = 0; j < MAX_UI_ELEMENTS; j++)
        {
        
            UIElement * el = &els[j];
        
            if (!el->visible || el->free) continue;

            bool hovering = is_hovering_ui_element(el, (vec2){mouse_x, mouse_y});
            bool processed_press = false;
            if (highest_el)
            {
                if (highest_el->uID == el->uID && menu_idx == i && event.type == SDL_MOUSEBUTTONDOWN)
                {
                    processed_press = true;
                    if (el->action_callback.one_shot)
                    {
                
                        if (el->state == UISTATE_PRESSED)
                        {
                            if (hovering)
                            {
                                process_ui_event(&el->hover_callback);
                                el->state = UISTATE_HOVERED;
                            }
                            else
                            {
                                el->state = UISTATE_NONE;
                            }
                        }
                        else
                        {
                    
                            process_ui_event(&el->action_callback);
                            el->state = UISTATE_NONE;
                        }
                    }
                    else
                    {
                        process_ui_event(&el->action_callback);
                        el->state = UISTATE_PRESSED;
                    }
                }
                
            }
            if (!processed_press)
            {
                
                if (hovering)
                {
                    if (event.type != SDL_MOUSEBUTTONUP && el->state == UISTATE_PRESSED && !el->action_callback.one_shot) continue;
                    if (el->already_hovered) continue;

                    if (el->hover_callback.one_shot)
                    {
                
                        if (el->state == UISTATE_HOVERED)
                        {
                    
                            el->already_hovered = true;
                            el->state = UISTATE_NONE;
                        }
                        else
                        {
                    
                            process_ui_event(&el->hover_callback);
                            el->state = UISTATE_NONE;
                        }
                    }
                    else
                    {
                
                        process_ui_event(&el->hover_callback);
                        el->state = UISTATE_HOVERED;
                    }
                }
                else
                {

                    if (event.type != SDL_MOUSEBUTTONUP && el->state == UISTATE_PRESSED && !el->action_callback.one_shot) continue;

                    if (el->state == UISTATE_HOVERED)
                    {
                        el->already_hovered = false;
                    }
                    el->state = UISTATE_NONE;

                }
            }
        
        }
    }

    
}



void update_ui_state(float mouse_x, float mouse_y)
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    for (int i = 0; i < MENUS.count; i++)
    {
        UIMenu * m = &ms[i];
        UIElement * els = m->elements;

        for (int j = 0; j < MAX_UI_ELEMENTS; j++)
        {

            UIElement * el = &els[j];
            if (el->free) continue;

            switch(el->state)
            {
                case UISTATE_NONE:
                    break;
                case UISTATE_PRESSED:
                    {
                        
                        float scale_x = (float)UI_SCALE_WIDTH / TEXT_WIDTH;
                        float scale_y = (float)UI_SCALE_HEIGHT / TEXT_HEIGHT;
                        vec2 mouse = {mouse_x * scale_x, mouse_y * scale_y};
                        if (el->interactable == UI_INTERACTABLE_SLIDER){
                            UISlider * slider = &el->as.slider;
                            if (slider->dragging){
                
                                vec2 delta = vec2_sub(mouse, slider->drag_mouse_start);
                                float t = vec2_dot(delta, slider->dir);
                                float new_scalar = t + slider->drag_start_scalar;
                                new_scalar = fmaxf(0.0f, fminf(slider->max_proj, new_scalar));
                                vec2 new_pos = vec2_add(slider->min, vec2_scale(slider->dir, new_scalar));
                                slider->prev_val = slider->val;
                                slider->val = new_scalar / slider->max_proj;
                                el->translation = new_pos;
                
                            } else {
                
                                slider->dragging = true;
                                vec2  from_min = vec2_sub(el->translation, slider->min);
                                float start_scalar = vec2_dot(from_min, slider->dir);      
                                start_scalar = fmaxf(0.0f, fminf(slider->max_proj, start_scalar));
                                slider->prev_val = start_scalar / slider->max_proj;
                                slider->val = start_scalar / slider->max_proj;
                                slider->drag_start_scalar = start_scalar; 
                                slider->drag_center_start = el->translation;
                                slider->drag_mouse_start = mouse;

                            }

                            if (slider->container != -1)
                            {
                                
                                UIContainer * cs = m->containers;
                                UIContainer * c = &cs[slider->container];
                                scroll_container(c, slider, m);
                                
                                
                            }

                        }
                        else
                        {
                        
                            process_ui_event(&el->action_callback);
                        }
                    }

                    
                    break;
                case UISTATE_HOVERED:
                    process_ui_event(&el->hover_callback);
                    break;
                default:
                    break;
            }

        
        }
        
    }

}



