#include <stdlib.h>
#include <string.h>
#include "button.h"

typedef struct {
    char        *label;
    button_cb_t  on_click;
    void        *userdata;
} button_data_t;

static void button_measure(widget_t *w, wl_theme_t *theme) {
    button_data_t *d = w->data;
    w->pref_h = w->min_h > 0 ? w->min_h : 32;
    int text_w = (theme && theme->font && d->label)
               ? wl_text_width(theme->font, d->label) + theme->padding * 4
               : 80;
    w->pref_w = text_w > w->min_w ? text_w : w->min_w;
}

static void button_draw(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme) {
    button_data_t *d = w->data;
    double s = theme->scale;

    int x  = (int)(w->x * s);
    int y  = (int)(w->y * s);
    int ww = (int)(w->w * s);
    int wh = (int)(w->h * s);

    uint32_t bg = w->hovered ? theme->bg_hover  :
                  w->focused ? theme->bg_active  :
                               theme->bg_widget;
    wl_draw_rect(canvas, x, y, ww, wh, bg);

    uint32_t border = w->focused ? theme->border_focused : theme->border;
    int bw = theme->border_width;
    wl_draw_rect(canvas, x,           y,           ww, bw,  border);
    wl_draw_rect(canvas, x,           y + wh - bw, ww, bw,  border);
    wl_draw_rect(canvas, x,           y,           bw,  wh, border);
    wl_draw_rect(canvas, x + ww - bw, y,           bw,  wh, border);

    if (d->label && theme->font) {
        int text_w   = wl_text_width(theme->font, d->label);
        int text_x   = x + (ww - text_w) / 2;
        if (text_x < x) text_x = x + (int)(theme->padding * s);
        int baseline = y + wh / 2 + (int)(5 * s);
        wl_draw_text(canvas, theme->font, text_x, baseline, d->label, theme->text);
    }
}

static void button_on_mouse_button(widget_t *w, wl_mouse_button_event_t *e) {
    button_data_t *d = w->data;
    if (e->button == 1 && e->pressed && d->on_click)
        d->on_click(w, d->userdata);
}

static void button_destroy(widget_t *w) {
    button_data_t *d = w->data;
    free(d->label);
    free(d);
}

widget_t *button_new(const char *label, button_cb_t on_click, void *userdata) {
    widget_t *w = widget_new();
    if (!w) return NULL;

    button_data_t *d = calloc(1, sizeof(*d));
    d->label    = label ? strdup(label) : NULL;
    d->on_click = on_click;
    d->userdata = userdata;

    w->data            = d;
    w->draw            = button_draw;
    w->measure         = button_measure;
    w->on_mouse_button = button_on_mouse_button;
    w->destroy         = button_destroy;
    w->min_h           = 32;
    w->pref_h          = 32;
    w->min_w           = 80;
    w->pref_w          = 80;
    w->hpolicy         = SIZE_EXPAND;
    w->vpolicy         = SIZE_FIXED;

    return w;
}

void button_set_label(widget_t *w, const char *label) {
    button_data_t *d = w->data;
    free(d->label);
    d->label = label ? strdup(label) : NULL;
}
