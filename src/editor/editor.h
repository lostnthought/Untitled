#ifndef EDITOR_H
#define EDITOR_H

#include "core/engine_types.h"
void UNDO_INIT();
void render_editor_menu();

void init_file_directory_registries();
void toggle_performance_overlay();

void execute_undo(Undo * undo);
void update_performance_overlay(GLuint64 query_s[PQUERY_MAX]);
void create_performance_overlay();
void perform_newest_undo();
void perform_newest_redo();

#endif
