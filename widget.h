#pragma once

#include <stdbool.h>
#include "../wayland.c/build/wayland_dot_c.h"
#include "theme.h"

typedef struct widget widget_t;

typedef enum {
    SIZE_FIXED,   /* stays at preferred size */
    SIZE_EXPAND,  /* takes as much space as available */
    SIZE_SHRINK,  /* can shrink below preferred size */
} wl_size_policy_t;

typedef enum {
    ALIGN_START,
    ALIGN_CENTER,
    ALIGN_END,
} wl_align_t;

struct widget {
    /* geometry — set by layout */
    int x, y, w, h;

    /* size hints */
    int min_w,  min_h;   /* minimum size */
    int pref_w, pref_h;  /* preferred/natural size */

    /* margins — space outside the widget */
    int margin_top, margin_bottom, margin_left, margin_right;

    /* sizing policy */
    wl_size_policy_t hpolicy;
    wl_size_policy_t vpolicy;

    /* alignment within allocated cell */
    wl_align_t halign;
    wl_align_t valign;

    /* tree */
    widget_t  *parent;
    widget_t **children;
    int        child_count;
    int        child_cap;

    /* state */
    bool focused;
    bool hovered;
    bool visible;
    bool no_auto_draw_children;  /* set true if widget draws its own children manually (e.g. list) */

    /* virtual functions */
    void (*draw)           (widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme);
    void (*layout)         (widget_t *w);
    void (*measure)        (widget_t *w, wl_theme_t *theme);
    void (*on_key)         (widget_t *w, wl_key_event_t *e);
    void (*on_mouse_button)(widget_t *w, wl_mouse_button_event_t *e);
    void (*on_mouse_move)  (widget_t *w, wl_mouse_move_event_t *e);
    void (*on_scroll)      (widget_t *w, wl_scroll_event_t *e);
    void (*destroy)        (widget_t *w);

    /* userdata for concrete widget types */
    void *data;
};

/* -------------------------------------------------- */
/* Lifecycle                                          */
/* -------------------------------------------------- */

widget_t *widget_new(void);
void      widget_destroy(widget_t *w);
void      widget_add_child(widget_t *parent, widget_t *child);

/* -------------------------------------------------- */
/* Sizing helpers                                     */
/* -------------------------------------------------- */

void widget_set_margin(widget_t *w, int top, int right, int bottom, int left);
void widget_set_hpolicy(widget_t *w, wl_size_policy_t policy);
void widget_set_vpolicy(widget_t *w, wl_size_policy_t policy);
void widget_set_halign(widget_t *w, wl_align_t align);
void widget_set_valign(widget_t *w, wl_align_t align);

/* -------------------------------------------------- */
/* Tree operations                                    */
/* -------------------------------------------------- */

void      widget_measure(widget_t *w, wl_theme_t *theme);
void      widget_layout(widget_t *w, wl_theme_t *theme);
void      widget_draw(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme);
widget_t *widget_at(widget_t *w, int x, int y);

bool widget_dispatch_mouse_button(widget_t *w, wl_mouse_button_event_t *e);
void widget_dispatch_mouse_move  (widget_t *w, wl_mouse_move_event_t *e);
bool widget_dispatch_key         (widget_t *w, wl_key_event_t *e);
