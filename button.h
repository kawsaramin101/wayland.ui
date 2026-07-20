#pragma once

#include "widget.h"

typedef void (*button_cb_t)(widget_t *w, void *userdata);

widget_t *button_new(const char *label, button_cb_t on_click, void *userdata);
void      button_set_label(widget_t *w, const char *label);
