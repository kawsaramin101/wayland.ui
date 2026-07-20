#pragma once

#include "widget.h"

widget_t   *label_new(const char *text);
void        label_set_text(widget_t *w, const char *text);
const char *label_get_text(widget_t *w);
