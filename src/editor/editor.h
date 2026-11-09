/**
 * @file editor.h
 * @brief The debug editor UI for the engine.
 */

#ifndef EDITOR_H
#define EDITOR_H

#include "core/engine_types.h"

/**
 * @brief Initializes Undo%s inside of ::UNDOS to free state.
 */
void UNDO_INIT();

/**
 * @brief Renders the editor menus.
 */
void render_editor_menu();

/**
 * @brief Initializes each FileDirectoryRegistry based on initial paths.
 */
void FILE_DIR_REGISTRY_INIT();

/**
 * @brief Toggles the performance overlay menu: ::PERFORMANCE_OVERLAY_MENU.
 */
void toggle_performance_overlay();

/**
 * @brief Fills performance overlay with text based on current GPU profiling information.
 * @param query_s The result of glQueries for the current frame.
 */
void update_performance_overlay(GLuint64 query_s[PQUERY_MAX]);

/**
 * @brief Allocates and initializes position of the TextBox%es for the performance overlay.
 */
void create_performance_overlay();

/**
 * @brief Performs an Undo (switch-case into each UndoStructureType).
 * @param The Undo to perform.
 */
void execute_undo(Undo * undo);


/**
 * @brief Performs top-most Undo and moves the top-most pointer to the previous.
 */
void perform_newest_undo();

/**
 * @brief Performs top-most Redo (Undo->next) and moves pointers upward.
 */
void perform_newest_redo();

#endif
