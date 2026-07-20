#pragma once

#include "widget.h"

/* Single-line text input widget */
widget_t   *textfield_new(const char *placeholder);
const char *textfield_get_text(widget_t *w);
void        textfield_set_text(widget_t *w, const char *text);

/* Callback fired when user presses Enter */
typedef void (*textfield_submit_fn)(widget_t *w, const char *text, void *userdata);
void textfield_on_submit(widget_t *w, textfield_submit_fn fn, void *userdata);
void textfield_on_change(widget_t *w, textfield_submit_fn fn, void *userdata);
