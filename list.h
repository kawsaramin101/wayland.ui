#pragma once

#include "widget.h"

typedef void (*list_select_fn)(widget_t *w, int index, void *userdata);

widget_t *list_new(int item_height);
void      list_add_row   (widget_t *w, widget_t *row);
void      list_clear     (widget_t *w);
void      list_on_select (widget_t *w, list_select_fn fn, void *userdata);
int       list_get_selected(widget_t *w);
