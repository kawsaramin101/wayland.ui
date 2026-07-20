#include <stdlib.h>
#include <string.h>
#include "widget.h"

/* -------------------------------------------------- */
/* Lifecycle                                          */
/* -------------------------------------------------- */

widget_t *widget_new(void) {
    widget_t *w = calloc(1, sizeof(*w));
    if (!w) return NULL;
    w->visible  = true;
    w->hpolicy  = SIZE_FIXED;
    w->vpolicy  = SIZE_FIXED;
    w->halign   = ALIGN_START;
    w->valign   = ALIGN_START;
    return w;
}

void widget_destroy(widget_t *w) {
    if (!w) return;
    for (int i = 0; i < w->child_count; i++)
        widget_destroy(w->children[i]);
    free(w->children);
    if (w->destroy) w->destroy(w);
    free(w);
}

void widget_add_child(widget_t *parent, widget_t *child) {
    if (!parent || !child) return;
    if (parent->child_count >= parent->child_cap) {
        int new_cap = parent->child_cap == 0 ? 4 : parent->child_cap * 2;
        parent->children = realloc(parent->children, new_cap * sizeof(widget_t *));
        parent->child_cap = new_cap;
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

/* -------------------------------------------------- */
/* Sizing helpers                                     */
/* -------------------------------------------------- */

void widget_set_margin(widget_t *w, int top, int right, int bottom, int left) {
    w->margin_top    = top;
    w->margin_right  = right;
    w->margin_bottom = bottom;
    w->margin_left   = left;
}

void widget_set_hpolicy(widget_t *w, wl_size_policy_t policy) { w->hpolicy = policy; }
void widget_set_vpolicy(widget_t *w, wl_size_policy_t policy) { w->vpolicy = policy; }
void widget_set_halign (widget_t *w, wl_align_t align)        { w->halign  = align;  }
void widget_set_valign (widget_t *w, wl_align_t align)        { w->valign  = align;  }

/* -------------------------------------------------- */
/* Internal helpers                                   */
/* -------------------------------------------------- */

static void clear_focus(widget_t *w) {
    if (!w) return;
    w->focused = false;
    for (int i = 0; i < w->child_count; i++)
        clear_focus(w->children[i]);
}

static void clear_hover(widget_t *w) {
    if (!w) return;
    w->hovered = false;
    for (int i = 0; i < w->child_count; i++)
        clear_hover(w->children[i]);
}

/* -------------------------------------------------- */
/* Measure pass                                       */
/* -------------------------------------------------- */

void widget_measure(widget_t *w, wl_theme_t *theme) {
    if (!w) return;
    for (int i = 0; i < w->child_count; i++)
        widget_measure(w->children[i], theme);
    if (w->measure)
        w->measure(w, theme);
    else {
        if (w->pref_w == 0) w->pref_w = w->w > w->min_w ? w->w : w->min_w;
        if (w->pref_h == 0) w->pref_h = w->h > w->min_h ? w->h : w->min_h;
    }
}

/* -------------------------------------------------- */
/* Layout pass                                        */
/* -------------------------------------------------- */

void widget_layout(widget_t *w, wl_theme_t *theme) {
    if (!w) return;
    widget_measure(w, theme);
    if (w->layout) w->layout(w);
}

/* -------------------------------------------------- */
/* Draw pass                                          */
/* -------------------------------------------------- */

void widget_draw(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme) {
    if (!w || !w->visible) return;
    double s = theme->scale;

    wl_canvas_set_clip(canvas,
                       (int)(w->x * s), (int)(w->y * s),
                       (int)(w->w * s), (int)(w->h * s));

    if (w->draw) w->draw(w, canvas, theme);

    /* skip generic child recursion for widgets that manually draw their
       own children (e.g. list rows are drawn with custom scroll clipping
       inside list_draw) — marked via the no_auto_draw_children flag */
    if (!w->no_auto_draw_children) {
        for (int i = 0; i < w->child_count; i++)
            widget_draw(w->children[i], canvas, theme);
    }

    wl_canvas_reset_clip(canvas);
}

/* -------------------------------------------------- */
/* Hit testing                                        */
/* -------------------------------------------------- */

widget_t *widget_at(widget_t *w, int x, int y) {
    if (!w || !w->visible) return NULL;
    if (x < w->x || x >= w->x + w->w) return NULL;
    if (y < w->y || y >= w->y + w->h) return NULL;
    for (int i = w->child_count - 1; i >= 0; i--) {
        widget_t *hit = widget_at(w->children[i], x, y);
        if (hit) return hit;
    }
    return w;
}

/* -------------------------------------------------- */
/* Event dispatch                                     */
/* -------------------------------------------------- */

bool widget_dispatch_mouse_button(widget_t *w, wl_mouse_button_event_t *e) {
    if (!w || !w->visible) return false;
    widget_t *hit = widget_at(w, (int)e->x, (int)e->y);
    if (!hit) return false;
    clear_focus(w);
    hit->focused = true;

    /* walk up to find nearest ancestor that actually handles clicks */
    widget_t *handler = hit;
    while (handler && !handler->on_mouse_button)
        handler = handler->parent;

    if (handler && handler->on_mouse_button) {
        handler->on_mouse_button(handler, e);
        return true;
    }
    return true;
}

void widget_dispatch_mouse_move(widget_t *w, wl_mouse_move_event_t *e) {
    if (!w || !w->visible) return;
    widget_t *hit = widget_at(w, (int)e->x, (int)e->y);
    clear_hover(w);
    if (hit) hit->hovered = true;
    if (hit && hit->on_mouse_move)
        hit->on_mouse_move(hit, e);
}

bool widget_dispatch_key(widget_t *w, wl_key_event_t *e) {
    if (!w || !w->visible) return false;
    if (w->focused && w->on_key) {
        w->on_key(w, e);
        return true;
    }
    for (int i = 0; i < w->child_count; i++) {
        if (widget_dispatch_key(w->children[i], e))
            return true;
    }
    return false;
}
