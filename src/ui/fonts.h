
/**
 * @file fonts.h
 * @brief Functions that modify TextBox%es and load Font%s.
 */
#ifndef FONTS_H
#define FONTS_H

#include "utilities/types.h"
#include "ui/ui.h"

/**
 * @brief All loaded Font%s.
 */
extern Vector FONTS;


/**
 * @brief Simply clears the ::FONTS buffer and re-initializes the Vector.
 */
static inline void FONTS_INIT()
{
    if (FONTS.buffer) free(FONTS.buffer);
    init_vector(&FONTS, 0, sizeof(Font));
}

/**
 * @brief Finds an empty slot for a TextBox that is marked as free.
 * @param m The UIMenu to search.
 */
int find_empty_text_box_slot(UIMenu * m);

/**
 * @brief Reads the JSON for an MSDF Font, and creates a Font from it.
 * @param json_path The path to the json.
 * @param png_path The path to the png.
 * @returns The pointer to the resulting Font.
 */
Font * read_json_msdf(const char * json_path, const char * png_path);

/**
 * @brief Determines TextBox dimensions by iterating through its text as if it was rendering the Font.
 * @param tb The TextBox.
 * @param width Pointer to out width.
 * @param height Pointer to out height.
 */
void determine_text_box_dimensions(TextBox * tb, float * width, float * height);
/**
 * @brief Moves a TextBox to \f$(0, 0)\f$, while accounting for Justify.
 * @param box The TextBox to move.
 */
void move_text_box_to_origin(TextBox * box);

/**
 * @brief Creates a ::DEFAULT_TEXT_BOX.
 * @param menu_id The target UIMenu.
 * @returns The index to the TextBox.
 */
int create_text_box(int menu_id);

/**
 * @brief Deletes a TextBox and its text and frees it from the UIMenu.
 * @param menu The UIMenu the TextBox is in.
 * @param id The index of the TextBox.
 */
void delete_text_box(int menu, int id);



#endif
