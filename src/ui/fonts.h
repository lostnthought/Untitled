
#ifndef FONTS_H
#define FONTS_H

#include "utilities/types.h"
#include "ui/ui.h"


int find_empty_text_box_slot(UIMenu * m);
Font * read_json_msdf(const char * json_path, const char * png_path);
void move_text_box_to_origin(TextBox * box);

void determine_text_box_dimensions(TextBox * tb, float * width, float * height);
void add_all_text_boxes_to_draw_buffer();
int create_text_box(int menu_id);
void delete_text_box(int menu, int id);
void add_text_box(Vector * tvs, Vector * tis, TextBox * tb, int current_box);

extern Vector FONTS;

static inline void FONTS_INIT()
{
    if (FONTS.buffer) free(FONTS.buffer);
    init_vector(&FONTS, 0, sizeof(Font));
}

#endif
