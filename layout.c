#include <stdlib.h>
#include "layout.h"

/* -------------------------------------------------- */
/* VBox                                               */
/* -------------------------------------------------- */

typedef struct {
    int spacing;
} box_data_t;

static void vbox_measure(widget_t *w, wl_theme_t *theme) {
    (void)theme;
    box_data_t *d = w->data;
    int total_h = 0;
    int max_w   = 0;
    int visible = 0;

    for (int i = 0; i < w->child_count; i++) {
        widget_t *c = w->children[i];
        if (!c->visible) continue;
        int cw = c->pref_w + c->margin_left + c->margin_right;
        int ch = c->pref_h + c->margin_top  + c->margin_bottom;
        if (cw > max_w) max_w = cw;
        total_h += ch;
        visible++;
    }

    if (visible > 1) total_h += d->spacing * (visible - 1);

    w->pref_w = max_w   > w->min_w ? max_w   : w->min_w;
    w->pref_h = total_h > w->min_h ? total_h : w->min_h;
}

static void vbox_layout(widget_t *w) {
    box_data_t *d = w->data;

    /* count expanding children and fixed height used */
    int fixed_h   = 0;
    int n_expand  = 0;
    int n_visible = 0;

    for (int i = 0; i < w->child_count; i++) {
        widget_t *c = w->children[i];
        if (!c->visible) continue;
        n_visible++;
        if (c->vpolicy == SIZE_EXPAND) {
            n_expand++;
        } else {
            fixed_h += c->pref_h + c->margin_top + c->margin_bottom;
        }
    }

    if (n_visible > 1) fixed_h += d->spacing * (n_visible - 1);

    int leftover   = w->h - fixed_h;
    int expand_h   = n_expand > 0 ? leftover / n_expand : 0;

    int pen_y = w->y;

    for (int i = 0; i < w->child_count; i++) {
        widget_t *c = w->children[i];
        if (!c->visible) continue;

        pen_y += c->margin_top;

        int alloc_h = (c->vpolicy == SIZE_EXPAND) ? expand_h
                    : c->pref_h;
        int alloc_w = w->w - c->margin_left - c->margin_right;

        /* horizontal alignment */
        int cw = (c->hpolicy == SIZE_EXPAND) ? alloc_w : c->pref_w;
        if (cw > alloc_w) cw = alloc_w;

        int cx;
        if      (c->halign == ALIGN_CENTER) cx = w->x + c->margin_left + (alloc_w - cw) / 2;
        else if (c->halign == ALIGN_END)    cx = w->x + c->margin_left + alloc_w - cw;
        else                                cx = w->x + c->margin_left;

        /* vertical alignment within allocated cell */
        int ch = (c->vpolicy == SIZE_EXPAND) ? alloc_h : c->pref_h;
        if (ch > alloc_h) ch = alloc_h;

        int cy;
        if      (c->valign == ALIGN_CENTER) cy = pen_y + (alloc_h - ch) / 2;
        else if (c->valign == ALIGN_END)    cy = pen_y + alloc_h - ch;
        else                                cy = pen_y;

        c->x = cx;
        c->y = cy;
        c->w = cw;
        c->h = ch;

        /* recurse */
        if (c->layout) c->layout(c);

        pen_y += alloc_h + c->margin_bottom + d->spacing;
    }

    /* only shrink-to-fit if nothing is expanding into our given height */
    if (w->vpolicy != SIZE_EXPAND && n_expand == 0)
        w->h = pen_y - w->y - d->spacing;
}

static void box_draw(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme) {
    (void)w; (void)canvas; (void)theme;
}

widget_t *vbox_new(int x, int y, int w, int spacing) {
    widget_t *widget = widget_new();
    if (!widget) return NULL;

    box_data_t *d = malloc(sizeof(*d));
    d->spacing = spacing;

    widget->x       = x;
    widget->y       = y;
    widget->w       = w;
    widget->data    = d;
    widget->draw    = box_draw;
    widget->layout  = vbox_layout;
    widget->measure = vbox_measure;
    widget->hpolicy = SIZE_FIXED;
    widget->vpolicy = SIZE_FIXED;

    return widget;
}

/* -------------------------------------------------- */
/* HBox                                               */
/* -------------------------------------------------- */

static void hbox_measure(widget_t *w, wl_theme_t *theme) {
    (void)theme;
    box_data_t *d = w->data;
    int total_w = 0;
    int max_h   = 0;
    int visible = 0;

    for (int i = 0; i < w->child_count; i++) {
        widget_t *c = w->children[i];
        if (!c->visible) continue;
        int cw = c->pref_w + c->margin_left + c->margin_right;
        int ch = c->pref_h + c->margin_top  + c->margin_bottom;
        if (ch > max_h) max_h = ch;
        total_w += cw;
        visible++;
    }

    if (visible > 1) total_w += d->spacing * (visible - 1);

    w->pref_w = total_w > w->min_w ? total_w : w->min_w;
    w->pref_h = max_h   > w->min_h ? max_h   : w->min_h;
}

static void hbox_layout(widget_t *w) {
    box_data_t *d = w->data;

    int fixed_w   = 0;
    int n_expand  = 0;
    int n_visible = 0;

    for (int i = 0; i < w->child_count; i++) {
        widget_t *c = w->children[i];
        if (!c->visible) continue;
        n_visible++;
        if (c->hpolicy == SIZE_EXPAND) {
            n_expand++;
        } else {
            fixed_w += c->pref_w + c->margin_left + c->margin_right;
        }
    }

    if (n_visible > 1) fixed_w += d->spacing * (n_visible - 1);

    int leftover  = w->w - fixed_w;
    int expand_w  = n_expand > 0 ? leftover / n_expand : 0;

    int pen_x = w->x;

    for (int i = 0; i < w->child_count; i++) {
        widget_t *c = w->children[i];
        if (!c->visible) continue;

        pen_x += c->margin_left;

        int alloc_w = (c->hpolicy == SIZE_EXPAND) ? expand_w : c->pref_w;
        int alloc_h = w->h - c->margin_top - c->margin_bottom;

        int cw = (c->hpolicy == SIZE_EXPAND) ? alloc_w : c->pref_w;
        if (cw > alloc_w) cw = alloc_w;

        int ch = (c->vpolicy == SIZE_EXPAND) ? alloc_h : c->pref_h;
        if (ch > alloc_h) ch = alloc_h;

        int cx = pen_x;

        int cy;
        if      (c->valign == ALIGN_CENTER) cy = w->y + c->margin_top + (alloc_h - ch) / 2;
        else if (c->valign == ALIGN_END)    cy = w->y + c->margin_top + alloc_h - ch;
        else                                cy = w->y + c->margin_top;

        c->x = cx;
        c->y = cy;
        c->w = cw;
        c->h = ch;

        if (c->layout) c->layout(c);

        pen_x += alloc_w + c->margin_right + d->spacing;
    }

    if (w->hpolicy != SIZE_EXPAND && n_expand == 0)
        w->w = pen_x - w->x - d->spacing;
}

widget_t *hbox_new(int x, int y, int h, int spacing) {
    widget_t *widget = widget_new();
    if (!widget) return NULL;

    box_data_t *d = malloc(sizeof(*d));
    d->spacing = spacing;

    widget->x       = x;
    widget->y       = y;
    widget->h       = h;
    widget->data    = d;
    widget->draw    = box_draw;
    widget->layout  = hbox_layout;
    widget->measure = hbox_measure;
    widget->hpolicy = SIZE_FIXED;
    widget->vpolicy = SIZE_FIXED;

    return widget;
}

/* -------------------------------------------------- */
/* Spacer                                             */
/* -------------------------------------------------- */

widget_t *spacer_new(void) {
    widget_t *w = widget_new();
    if (!w) return NULL;
    w->hpolicy = SIZE_EXPAND;
    w->vpolicy = SIZE_EXPAND;
    w->pref_w  = 0;
    w->pref_h  = 0;
    return w;
}

/* -------------------------------------------------- */
/* Padding container                                  */
/* -------------------------------------------------- */

typedef struct {
    int top, right, bottom, left;
} padding_data_t;

static void padding_measure(widget_t *w, wl_theme_t *theme) {
    (void)theme;
    padding_data_t *d = w->data;
    if (w->child_count == 0) return;
    widget_t *child = w->children[0];
    w->pref_w = child->pref_w + d->left + d->right;
    w->pref_h = child->pref_h + d->top  + d->bottom;
}

static void padding_layout(widget_t *w) {
    padding_data_t *d = w->data;
    if (w->child_count == 0) return;
    widget_t *child = w->children[0];

    child->x = w->x + d->left;
    child->y = w->y + d->top;
    child->w = w->w - d->left - d->right;
    child->h = w->h - d->top  - d->bottom;

    if (child->hpolicy != SIZE_EXPAND) child->w = child->pref_w;
    if (child->vpolicy != SIZE_EXPAND) child->h = child->pref_h;

    if (child->layout) child->layout(child);
}

widget_t *padding_new(int top, int right, int bottom, int left) {
    widget_t *w = widget_new();
    if (!w) return NULL;

    padding_data_t *d = malloc(sizeof(*d));
    d->top    = top;
    d->right  = right;
    d->bottom = bottom;
    d->left   = left;

    w->data    = d;
    w->draw    = box_draw;
    w->layout  = padding_layout;
    w->measure = padding_measure;
    w->hpolicy = SIZE_EXPAND;
    w->vpolicy = SIZE_EXPAND;

    return w;
}

/* -------------------------------------------------- */
/* Center container                                   */
/* -------------------------------------------------- */

static void center_layout(widget_t *w) {
    if (w->child_count == 0) return;
    widget_t *child = w->children[0];

    int cw = (child->hpolicy == SIZE_EXPAND) ? w->w : child->pref_w;
    int ch = child->pref_h;

    int cx = (child->hpolicy == SIZE_EXPAND) ? w->x : w->x + (w->w - cw) / 2;
    int cy = w->y + (w->h - ch) / 2;

    child->x = cx;
    child->y = cy;
    child->w = cw;
    child->h = ch;

    if (child->layout) child->layout(child);
}

static void center_measure(widget_t *w, wl_theme_t *theme) {
    (void)theme;
    if (w->child_count == 0) return;
    widget_t *child = w->children[0];
    w->pref_w = child->pref_w;
    w->pref_h = child->pref_h;
}

widget_t *center_new(void) {
    widget_t *w = widget_new();
    if (!w) return NULL;
    w->draw    = box_draw;
    w->layout  = center_layout;
    w->measure = center_measure;
    w->hpolicy = SIZE_EXPAND;
    w->vpolicy = SIZE_EXPAND;
    return w;
}
