#pragma once

#include "../wayland.c/build/wayland_dot_c.h"
#include "theme.h"
#include "widget.h"
#include "layout.h"
#include "label.h"
#include "button.h"
#include "textfield.h"
#include "list.h"

typedef struct waylandui waylandui_t;

waylandui_t *waylandui_create(const char *title, int w, int h,
                               const char *font_path, int font_size);
void         waylandui_set_root(waylandui_t *ui, widget_t *root);
wl_theme_t  *waylandui_theme(waylandui_t *ui);
void         waylandui_redraw(waylandui_t *ui);
int          waylandui_add_timer(waylandui_t *ui, int interval_ms,
                                 wl_timer_fn fn, void *userdata);
void         waylandui_remove_timer(waylandui_t *ui, int timer_id);
void         waylandui_run(waylandui_t *ui);
void         waylandui_destroy(waylandui_t *ui);
void waylandui_on_key(waylandui_t *ui, wl_key_fn fn, void *userdata);
void waylandui_on_mouse_button(waylandui_t *ui, wl_mouse_button_fn fn, void *userdata);
