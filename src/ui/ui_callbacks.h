#ifndef UI_CALLBACKS_H
#define UI_CALLBACKS_H

#include "ui.h"

#define MAX_CALLBACKS 256
extern UICallback ACTION_CALLBACKS[MAX_CALLBACKS];
extern UICallback HOVER_CALLBACKS[MAX_CALLBACKS];
extern int ACTION_CALLBACK_COUNT;
extern int HOVER_CALLBACK_COUNT;
void init_action_callbacks();
void init_hover_callbacks();

#endif
