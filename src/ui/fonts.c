
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "utilities/types.h"
#include "ui/ui.h"
#include "utilities/utilities.h"
#include "ui/fonts.h"



Vector FONTS = {0};

uint32_t TEXT_INDEX_COUNT = 0;




int find_empty_text_box_slot(UIMenu * m)
{
    for (int i = 0; i < MAX_TEXT_BOXES; i++)
    {

        if (m->text_boxes[i].free) return i;
        
    }
    return -1;
}


Font * read_json_msdf(const char * json_path, const char * png_path) {

    FILE *f = fopen(json_path, "rb");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char *json_data = malloc(len + 1);
    fread(json_data, 1, len, f);
    json_data[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(json_data);
    free(json_data);
    if (!root) {
        fprintf(stderr, "Failed to parse font json\n");
        return NULL;
    }

    cJSON *common = cJSON_GetObjectItem(root, "common");
    int tex_w = cJSON_GetObjectItem(common, "scaleW")->valueint;
    int tex_h = cJSON_GetObjectItem(common, "scaleH")->valueint;

    Font * font = vector_expand(&FONTS, 1);
    font->tex_w = tex_w;
    font->tex_h = tex_h;
    strncpy(font->name, png_path, 255);
    font->name[255] = '\0';
    cJSON *chars = cJSON_GetObjectItem(root, "chars");
    int count = cJSON_GetArraySize(chars);
    font->glyphs = malloc(sizeof(Glyph) * count);
    int highest_y = INT_MAX;
    int highest_dist = 1;

    for (int i = 0; i < count; i++) {

        cJSON * ch = cJSON_GetArrayItem(chars, i);
        Glyph * g = &font->glyphs[i];

        g->id = cJSON_GetObjectItem(ch, "id")->valueint;
        int x = cJSON_GetObjectItem(ch, "x")->valueint;
        int y = cJSON_GetObjectItem(ch, "y")->valueint;
        int width = cJSON_GetObjectItem(ch, "width")->valueint;
        int height = cJSON_GetObjectItem(ch, "height")->valueint;
        g->xoffset = cJSON_GetObjectItem(ch, "xoffset")->valueint;
        g->yoffset = cJSON_GetObjectItem(ch, "yoffset")->valueint;
        g->xadvance = cJSON_GetObjectItem(ch, "xadvance")->valueint;

        g->width = width;
        g->height = height;
        if (g->yoffset < highest_y && g->height > 0) highest_y = g->yoffset;
        if (g->yoffset + g->height > font->highest_dist) font->highest_dist = g->yoffset + g->height;
        g->u0 = (float)x / tex_w;
        g->v0 = (float)y / tex_h;
        g->u1 = (float)(x + width) / tex_w;
        g->v1 = (float)(y + height) / tex_h;
    }
    font->highest_y = highest_y;
    font->glyph_count = count;
    cJSON_Delete(root);
    return font;
}

void determine_text_box_dimensions(TextBox * tb, float * width, float * height){
    
    int len = strlen(tb->text);

    if (len <= 0) return;

    float x = tb->start.x, y = tb->start.y;
    Font * fonts = CAST_VEC(FONTS, Font);
    Font * f = &fonts[tb->font_id];
    int tex_w = f->tex_w;
    int tex_h = f->tex_h;
    float scale = tb->text_size;
    
    float max_x = 0, max_y = 0;
    Glyph * glyphs = f->glyphs;

    for (int i = 0; i < len; i++){ 
        char ch = tb->text[i];

        Glyph *g = NULL;
        for (int j = 0; j < f->glyph_count; j++) {
            if (glyphs[j].id == ch) {
                g = &glyphs[j];
                break;
            }
        }
        if (!g) continue; 

        float x0 = x + g->xoffset * scale;
        float y0 = (y - 7 * scale) + g->yoffset * scale;
        float x1 = x0 + g->width * scale;
        float y1 = y0 + g->height * scale;

        float u0 = g->u0;
        float v0 = g->v0;
        float u1 = g->u1;
        float v1 = g->v1;
        x += g->xadvance * scale;
        if (x > max_x) max_x = x;
        if (y1 > max_y) max_y = y1;
        if (x > tb->start.x + tb->w) {
            y += f->highest_dist * scale;
            x = tb->start.x;
        }
    }
    if (width) *width = max_x - tb->start.x;
    if (height) *height = max_y - tb->start.y;

    float dx = (max_x - tb->start.x) / 2 + tb->start.x;
    float dy = fabsf(max_y - tb->start.y) / 2 + tb->start.y;
}


void move_text_box_to_origin(TextBox * box){

    float w, h;
    determine_text_box_dimensions(box, &w, &h);
    switch(box->justify)
    {
        case JUSTIFY_CENTER:
            box->start.x = -w/2;
            box->start.y = -h/2;
            box->center = (vec2){0,0};
            box->w = w;
            box->h = h;
            break;
        case JUSTIFY_LEFT:
            box->start.x = 0;
            box->start.y = 0;
            box->center = (vec2){w/2,h/2};
            box->w = w;
            box->h = h;
            break;
        case JUSTIFY_RIGHT:
            box->start.x = -w;
            box->start.y = -h;
            box->center = (vec2){-w/2, -h/2};
            box->w = w;
            box->h = h;
            break;
        default:
            break;
    }
    
}


void delete_text_box(int menu, int id)
{
    
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[menu];

    TextBox * tb = &m->text_boxes[id];

    if (tb->parent_container != -1)
    {
        remove_text_box_from_container(&m->containers[tb->parent_container], tb);
    }
    if (tb->text) free(tb->text);
    tb->text = NULL;
    *tb = DEFAULT_TEXT_BOX;
    tb->free = true;
}

int create_text_box(int menu_id)
{
    
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);

    UIMenu * m = &ms[menu_id];

    int empty = find_empty_text_box_slot(m);
    if (empty == -1)
    {
        printf("No empty text box slot found.\n");
        return -1;
    }
    printf("TB: %d %d %d\n", empty, m->uID, menu_id);

    TextBox * tb = &m->text_boxes[empty];

    *tb = DEFAULT_TEXT_BOX;
    tb->text = malloc(10 * sizeof(char));
    tb->text_cap = 10;
    
    sprintf(tb->text, "test");


    tb->uID = empty;
    tb->menu = menu_id;
    
    move_text_box_to_origin(tb);

    return tb->uID;
    
}

