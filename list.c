#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "theme.h"

#define SCROLLBAR_W 8

typedef struct {
    int            item_height;  /* logical pixels per row */
    int            scroll_y;     /* logical pixels scrolled */
    int            selected;     /* selected row index, -1 = none */
    int            hovered_row;  /* row under mouse, -1 = none */
    list_select_fn on_select;
    void          *userdata;
} list_data_t;

/* -------------------------------------------------- */
/* Internal helpers                                   */
/* -------------------------------------------------- */

static int total_height(widget_t *w) {
    list_data_t *d = w->data;
    return w->child_count * d->item_height;
}

static int visible_count(widget_t *w) {
    list_data_t *d = w->data;
    if (d->item_height <= 0) return 0;
    return w->h / d->item_height;
}

static void clamp_scroll(widget_t *w) {
    list_data_t *d = w->data;
    int max_scroll = total_height(w) - w->h;
    if (max_scroll < 0) max_scroll = 0;
    if (d->scroll_y < 0)          d->scroll_y = 0;
    if (d->scroll_y > max_scroll) d->scroll_y = max_scroll;
}

static void layout_rows(widget_t *w) {
    list_data_t *d = w->data;
    int row_w = w->w - SCROLLBAR_W;

    for (int i = 0; i < w->child_count; i++) {
        widget_t *row = w->children[i];
        row->x = w->x;
        row->y = w->y + i * d->item_height - d->scroll_y;
        row->w = row_w;
        row->h = d->item_height;
        row->pref_w = row_w;
        row->pref_h = d->item_height;
        if (row->layout) row->layout(row);
    }
}

/* -------------------------------------------------- */
/* Draw                                               */
/* -------------------------------------------------- */

static void list_draw(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme) {
    list_data_t *d = w->data;
    double s = theme->scale;

    int x  = (int)(w->x * s);
    int y  = (int)(w->y * s);
    int ww = (int)(w->w * s);
    int wh = (int)(w->h * s);

    /* background */
    wl_draw_rect(canvas, x, y, ww, wh, theme->bg_widget);

    /* border */
    int bw = theme->border_width;
    wl_draw_rect(canvas, x,          y,          ww, bw,  theme->border);
    wl_draw_rect(canvas, x,          y + wh - bw, ww, bw,  theme->border);
    wl_draw_rect(canvas, x,          y,          bw,  wh, theme->border);
    wl_draw_rect(canvas, x + ww - bw, y,         bw,  wh, theme->border);

    /* clip to inner area (excluding scrollbar) */
    int sb_w = (int)(SCROLLBAR_W * s);
    int clip_x = x + bw, clip_y = y + bw;
    int clip_w = ww - bw*2 - sb_w, clip_h = wh - bw*2;
    wl_canvas_set_clip(canvas, clip_x, clip_y, clip_w, clip_h);

    /* draw each visible row */
    int first = d->scroll_y / d->item_height;
    int last  = first + visible_count(w) + 1;
    if (last > w->child_count) last = w->child_count;

    for (int i = first; i < last; i++) {
        widget_t *row = w->children[i];
        int ry = (int)(row->y * s);
        int rh = (int)(d->item_height * s);
        int rw = (int)((w->w - SCROLLBAR_W) * s);

        /* row background */
        uint32_t bg = (i == d->selected)    ? theme->accent    :
                      (i == d->hovered_row) ? theme->bg_hover  :
                                              theme->bg_widget;
        uint32_t fg = (i == d->selected) ? theme->bg : theme->text;

        wl_draw_rect(canvas, x + bw, ry, rw - bw, rh, bg);

        /* draw row's own draw fn directly — NOT widget_draw, since that
           resets clip to full canvas and would break our scroll clipping */
        uint32_t saved_text = theme->text;
        theme->text = fg;
        if (row->draw) row->draw(row, canvas, theme);
        for (int c = 0; c < row->child_count; c++)
            if (row->children[c]->draw)
                row->children[c]->draw(row->children[c], canvas, theme);
        theme->text = saved_text;

        /* re-assert list's own clip in case row drawing touched it */
        wl_canvas_set_clip(canvas, clip_x, clip_y, clip_w, clip_h);
    }

    wl_canvas_reset_clip(canvas);

    /* scrollbar */
    int total_h = total_height(w);
    if (total_h > w->h) {
        int sb_x    = x + ww - sb_w;
        int track_h = wh;
        int thumb_h = (int)((double)w->h / total_h * track_h);
        if (thumb_h < (int)(16 * s)) thumb_h = (int)(16 * s);
        int thumb_y = y + (int)((double)d->scroll_y / total_h * track_h);

        wl_draw_rect(canvas, sb_x, y,       sb_w, track_h, theme->bg);
        wl_draw_rect(canvas, sb_x, thumb_y, sb_w, thumb_h, theme->border);
    }
}

/* -------------------------------------------------- */
/* Layout                                             */
/* -------------------------------------------------- */

static void list_layout(widget_t *w) {
    clamp_scroll(w);
    layout_rows(w);
}

/* -------------------------------------------------- */
/* Events                                             */
/* -------------------------------------------------- */

static void list_on_mouse_button(widget_t *w, wl_mouse_button_event_t *e) {
    list_data_t *d = w->data;
    if (e->button != 1 || !e->pressed) return;

    /* which row was clicked? */
    int rel_y = (int)e->y - w->y + d->scroll_y;
    int index = rel_y / d->item_height;

    if (index < 0 || index >= w->child_count) return;

    d->selected = index;
    if (d->on_select)
        d->on_select(w, index, d->userdata);
}

static void list_on_mouse_move(widget_t *w, wl_mouse_move_event_t *e) {
    list_data_t *d = w->data;
    int rel_y = (int)e->y - w->y + d->scroll_y;
    d->hovered_row = rel_y / d->item_height;
    if (d->hovered_row >= w->child_count) d->hovered_row = -1;
}

static void list_on_key(widget_t *w, wl_key_event_t *e) {
    list_data_t *d = w->data;
    if (!e->pressed) return;

    int vis = visible_count(w);

    switch (e->sym) {
        case 0xFF52: /* up */
            if (d->selected > 0) d->selected--;
            break;
        case 0xFF54: /* down */
            if (d->selected < w->child_count - 1) d->selected++;
            break;
        case 0xFF55: /* page up */
            d->selected -= vis;
            if (d->selected < 0) d->selected = 0;
            break;
        case 0xFF56: /* page down */
            d->selected += vis;
            if (d->selected >= w->child_count) d->selected = w->child_count - 1;
            break;
        case 0xFF0D: /* enter */
            if (d->selected >= 0 && d->on_select)
                d->on_select(w, d->selected, d->userdata);
            return;
        default: return;
    }

    /* scroll to keep selection visible */
    if (d->selected >= 0) {
        int sel_y = d->selected * d->item_height;
        if (sel_y < d->scroll_y)
            d->scroll_y = sel_y;
        if (sel_y + d->item_height > d->scroll_y + w->h)
            d->scroll_y = sel_y + d->item_height - w->h;
        clamp_scroll(w);
    }
}

static void list_on_scroll(widget_t *w, wl_scroll_event_t *e) {
    list_data_t *d = w->data;
    d->scroll_y += (int)e->dy; /* dy is already in reasonable pixel-like units */
    clamp_scroll(w);
    layout_rows(w);
}

/* -------------------------------------------------- */
/* Measure                                            */
/* -------------------------------------------------- */

static void list_measure(widget_t *w, wl_theme_t *theme) {
    (void)theme;
    w->pref_w = w->min_w > 0 ? w->min_w : 200;
    w->pref_h = w->min_h > 0 ? w->min_h : 200;
}

/* -------------------------------------------------- */
/* Destroy                                            */
/* -------------------------------------------------- */

static void list_destroy(widget_t *w) {
    free(w->data);
    /* children are destroyed by widget_destroy */
}

/* -------------------------------------------------- */
/* Public API                                         */
/* -------------------------------------------------- */

widget_t *list_new(int item_height) {
    widget_t *w = widget_new();
    if (!w) return NULL;

    list_data_t *d = calloc(1, sizeof(*d));
    d->item_height = item_height > 0 ? item_height : 28;
    d->selected    = -1;
    d->hovered_row = -1;

    w->data            = d;
    w->draw            = list_draw;
    w->layout          = list_layout;
    w->measure         = list_measure;
    w->on_key          = list_on_key;
    w->on_mouse_button = list_on_mouse_button;
    w->on_mouse_move   = list_on_mouse_move;
    w->on_scroll       = list_on_scroll;
    w->destroy         = list_destroy;
    w->min_h           = 100;
    w->pref_h          = 200;
    w->min_w           = 100;
    w->pref_w          = 200;
    w->hpolicy         = SIZE_EXPAND;
    w->vpolicy         = SIZE_EXPAND;
    w->no_auto_draw_children = true;

    return w;
}

void list_add_row(widget_t *w, widget_t *row) {
    widget_add_child(w, row);
    layout_rows(w);
}

void list_clear(widget_t *w) {
    list_data_t *d = w->data;
    for (int i = 0; i < w->child_count; i++)
        widget_destroy(w->children[i]);
    w->child_count = 0;
    d->selected    = -1;
    d->scroll_y    = 0;
    d->hovered_row = -1;
}

void list_on_select(widget_t *w, list_select_fn fn, void *userdata) {
    list_data_t *d = w->data;
    d->on_select = fn;
    d->userdata  = userdata;
}

int list_get_selected(widget_t *w) {
    return ((list_data_t *)w->data)->selected;
}
