
#include "load/init.h"
#include "utilities/types.h"
#include "ui/ui.h"
#include "ui/ui_callbacks.h"




UICallback ACTION_CALLBACKS[MAX_CALLBACKS] = {0};
UICallback HOVER_CALLBACKS[MAX_CALLBACKS] = {0};
int ACTION_CALLBACK_COUNT = 0;
int HOVER_CALLBACK_COUNT = 0;








void init_action_callbacks()
{
    
    UICallback * ac = &ACTION_CALLBACKS[ACTION_CALLBACK_COUNT];
    sprintf(ac->id, "none");
    ac->val.type = UIVAL_NONE;
    ACTION_CALLBACK_COUNT++;

    ac = &ACTION_CALLBACKS[ACTION_CALLBACK_COUNT];
    sprintf(ac->id, "test");
    ac->val.type = UIVAL_BOOL;
    ac->val.data = &EDITOR_SETTINGS.LUT;
    ac->one_shot = false;
    ACTION_CALLBACK_COUNT++;

    ac = &ACTION_CALLBACKS[ACTION_CALLBACK_COUNT];
    sprintf(ac->id, "pos");
    ac->val.type = UIVAL_FLOAT;
    ac->val.data = &player.pos.z;
    ac->val.behavior = UIVALBEHAVIOR_NONE;
    ac->val.inc_val.inc_flt = 0.2f;
    ACTION_CALLBACK_COUNT++;

}

void init_hover_callbacks()
{
    
    UICallback * hc = &HOVER_CALLBACKS[HOVER_CALLBACK_COUNT];

    sprintf(hc->id, "none");
    hc->val.type = UIVAL_NONE;
    HOVER_CALLBACK_COUNT++;

    hc = &HOVER_CALLBACKS[HOVER_CALLBACK_COUNT];
    sprintf(hc->id, "test");
    hc->val.type = UIVAL_BOOL;
    hc->val.data = &EDITOR_SETTINGS.DOF;
    hc->one_shot = false;
    HOVER_CALLBACK_COUNT++;
}
