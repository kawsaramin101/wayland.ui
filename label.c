#include <stdlib.h>
#include <string.h>
#include "label.h"

typedef struct {
    char *text;
} label_data_t;

static void label_measure(widget_t *w, wl_theme_t *theme) {
    (void)theme;
    w->pref_h = w->min_h > 0 ? w->min_h : 24;
    w->pref_w = w->min_w > 0 ? w->min_w : 100;
}

static void label_draw(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme) {
    label_data_t *d = w->data;
    if (!d->text || !theme->font) return;

    double s        = theme->scale;
    int x           = (int)(w->x * s);
    int y           = (int)(w->y * s);
    int ww          = (int)(w->w * s);
    int wh          = (int)(w->h * s);
    int baseline    = y + wh / 2 + (int)((theme->padding / 2 + 2) * s);
    int text_x      = x + (int)(theme->padding * s);
    int max_w       = ww - (int)(theme->padding * 2 * s);

    /* check if text fits */
    int text_w = wl_text_width(theme->font, d->text);

    if (text_w <= max_w || max_w <= 0) {
        /* fits — draw normally */
        wl_draw_text(canvas, theme->font, text_x, baseline, d->text, theme->text);
        return;
    }

    /* doesn't fit — truncate with ellipsis */
    int ellipsis_w = wl_text_width(theme->font, "...");
    int avail_w    = max_w - ellipsis_w;
    if (avail_w <= 0) {
        wl_draw_text(canvas, theme->font, text_x, baseline, "...", theme->text);
        return;
    }

    /* find how many bytes fit within avail_w */
    char buf[4096];
    int  len  = strlen(d->text);
    int  pos  = 0;
    int  prev = 0;

    while (pos < len) {
        /* advance one utf-8 char */
        int next = pos + 1;
        while (next < len && ((unsigned char)d->text[next] & 0xC0) == 0x80)
            next++;

        /* measure up to next */
        memcpy(buf, d->text, next);
        buf[next] = '\0';
        if (wl_text_width(theme->font, buf) > avail_w)
            break;

        prev = next;
        pos  = next;
    }

    /* build truncated string */
    memcpy(buf, d->text, prev);
    strcpy(buf + prev, "...");

    wl_draw_text(canvas, theme->font, text_x, baseline, buf, theme->text);
}

static void label_destroy(widget_t *w) {
    label_data_t *d = w->data;
    free(d->text);
    free(d);
}

widget_t *label_new(const char *text) {
    widget_t *w = widget_new();
    if (!w) return NULL;

    label_data_t *d = calloc(1, sizeof(*d));
    d->text = text ? strdup(text) : NULL;

    w->data    = d;
    w->draw    = label_draw;
    w->measure = label_measure;
    w->destroy = label_destroy;
    w->min_h   = 24;
    w->min_w   = 0;
    w->pref_h  = 24;
    w->hpolicy = SIZE_EXPAND;
    w->vpolicy = SIZE_FIXED;

    return w;
}

void label_set_text(widget_t *w, const char *text) {
    label_data_t *d = w->data;
    free(d->text);
    d->text = text ? strdup(text) : NULL;
}

const char *label_get_text(widget_t *w) {
    return ((label_data_t *)w->data)->text;
}
