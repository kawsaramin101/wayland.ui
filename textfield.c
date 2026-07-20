#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include "textfield.h"
#include "theme.h"

#define TF_BUFSIZE 4096
#define TF_PAD     6

typedef struct {
    char               buf[TF_BUFSIZE];
    int                len;
    int                cursor;
    int                scroll_x;
    const char        *placeholder;
    textfield_submit_fn submit_fn;
    void               *submit_userdata;
    textfield_submit_fn change_fn;
    void               *change_userdata;
    wl_theme_t        *last_theme;
} tf_data_t;

/* -------------------------------------------------- */
/* UTF-8 helpers                                      */
/* -------------------------------------------------- */

/* byte length of utf-8 char starting at buf[pos] */
static int utf8_char_len(const char *buf, int pos) {
    unsigned char c = (unsigned char)buf[pos];
    if (c < 0x80) return 1;
    if (c < 0xE0) return 2;
    if (c < 0xF0) return 3;
    return 4;
}

/* move cursor left one utf-8 char */
static int cursor_left(const char *buf, int cursor) {
    if (cursor == 0) return 0;
    cursor--;
    while (cursor > 0 && (((unsigned char)buf[cursor]) & 0xC0) == 0x80)
        cursor--;
    return cursor;
}

/* move cursor right one utf-8 char */
static int cursor_right(const char *buf, int len, int cursor) {
    if (cursor >= len) return len;
    return cursor + utf8_char_len(buf, cursor);
}

/* insert utf-8 encoded codepoint at cursor */
static void insert_codepoint(tf_data_t *d, uint32_t cp) {
    char enc[5] = {0};
    int  enc_len;

    if      (cp < 0x80)    { enc[0] = cp;                                              enc_len = 1; }
    else if (cp < 0x800)   { enc[0] = 0xC0|(cp>>6);   enc[1] = 0x80|(cp&0x3F);        enc_len = 2; }
    else if (cp < 0x10000) { enc[0] = 0xE0|(cp>>12);  enc[1] = 0x80|((cp>>6)&0x3F);
                             enc[2] = 0x80|(cp&0x3F);                                  enc_len = 3; }
    else                   { enc[0] = 0xF0|(cp>>18);  enc[1] = 0x80|((cp>>12)&0x3F);
                             enc[2] = 0x80|((cp>>6)&0x3F); enc[3] = 0x80|(cp&0x3F);   enc_len = 4; }

    if (d->len + enc_len >= TF_BUFSIZE) return;

    memmove(d->buf + d->cursor + enc_len,
            d->buf + d->cursor,
            d->len - d->cursor + 1);
    memcpy(d->buf + d->cursor, enc, enc_len);
    d->len    += enc_len;
    d->cursor += enc_len;
}

/* delete char before cursor */
static void backspace(tf_data_t *d) {
    if (d->cursor == 0) return;
    int old = d->cursor;
    d->cursor = cursor_left(d->buf, d->cursor);
    int clen = old - d->cursor;
    memmove(d->buf + d->cursor, d->buf + old, d->len - old + 1);
    d->len -= clen;
}

/* delete char after cursor */
static void delete_forward(tf_data_t *d) {
    if (d->cursor >= d->len) return;
    int clen = utf8_char_len(d->buf, d->cursor);
    memmove(d->buf + d->cursor, d->buf + d->cursor + clen, d->len - d->cursor - clen + 1);
    d->len -= clen;
}

/* -------------------------------------------------- */
/* Scroll helpers                                     */
/* -------------------------------------------------- */

static void update_scroll(tf_data_t *d, wl_font_t *font, int inner_w, double scale) {
    if (!font) return;

    /* measure text up to cursor in physical pixels */
    char tmp[TF_BUFSIZE];
    memcpy(tmp, d->buf, d->cursor);
    tmp[d->cursor] = '\0';
    int cursor_px = wl_text_width(font, tmp);

    /* keep cursor visible */
    if (cursor_px - d->scroll_x > inner_w - (int)(TF_PAD * scale))
        d->scroll_x = cursor_px - inner_w + (int)(TF_PAD * scale);
    if (cursor_px - d->scroll_x < 0)
        d->scroll_x = cursor_px;
    if (d->scroll_x < 0) d->scroll_x = 0;
}

/* -------------------------------------------------- */
/* Draw                                               */
/* -------------------------------------------------- */

static void tf_draw(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme) {
    tf_data_t *d = w->data;
    double s = theme->scale;

    int x  = (int)(w->x * s);
    int y  = (int)(w->y * s);
    int ww = (int)(w->w * s);
    int wh = (int)(w->h * s);
    int pad = (int)(TF_PAD * s);

    /* background */
    uint32_t bg = w->focused ? theme->bg_widget : theme->bg;
    wl_draw_rect(canvas, x, y, ww, wh, bg);

    /* border */
    uint32_t border = w->focused ? theme->border_focused : theme->border;
    int bw = theme->border_width;
    wl_draw_rect(canvas, x,          y,          ww, bw,  border);
    wl_draw_rect(canvas, x,          y + wh - bw, ww, bw,  border);
    wl_draw_rect(canvas, x,          y,          bw,  wh, border);
    wl_draw_rect(canvas, x + ww - bw, y,         bw,  wh, border);

    /* inner clip */
    wl_canvas_set_clip(canvas, x + bw, y + bw, ww - bw*2, wh - bw*2);

    int text_x   = x + pad - d->scroll_x;
    int baseline = y + wh / 2 + (int)(5 * s);

    if (d->len == 0 && !w->focused && d->placeholder) {
        wl_draw_text(canvas, theme->font, text_x, baseline,
                     d->placeholder, theme->text_dim);
    } else {
        wl_draw_text(canvas, theme->font, text_x, baseline,
                     d->buf, theme->text);

        /* cursor */
        if (w->focused) {
            char tmp[TF_BUFSIZE];
            memcpy(tmp, d->buf, d->cursor);
            tmp[d->cursor] = '\0';
            int cursor_px = wl_text_width(theme->font, tmp);
            int cx = text_x + cursor_px;
            wl_draw_rect(canvas, cx, y + pad, 1, wh - pad * 2, theme->accent);
        }
    }

    wl_canvas_reset_clip(canvas);
}

/* -------------------------------------------------- */
/* Key input                                          */
/* -------------------------------------------------- */

static void tf_on_key(widget_t *w, wl_key_event_t *e) {
    tf_data_t *d = w->data;
    if (!e->pressed) return;

    bool text_changed = false;

    switch (e->sym) {
        case XKB_KEY_Left:
            d->cursor = cursor_left(d->buf, d->cursor);
            break;
        case XKB_KEY_Right:
            d->cursor = cursor_right(d->buf, d->len, d->cursor);
            break;
        case XKB_KEY_Home:
            d->cursor = 0;
            break;
        case XKB_KEY_End:
            d->cursor = d->len;
            break;
        case XKB_KEY_BackSpace:
            backspace(d);
            text_changed = true;
            break;
        case XKB_KEY_Delete:
            delete_forward(d);
            text_changed = true;
            break;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
            if (d->submit_fn)
                d->submit_fn(w, d->buf, d->submit_userdata);
            break;
        default:
            if (e->codepoint >= 32 && e->codepoint != 127) {
                insert_codepoint(d, e->codepoint);
                text_changed = true;
            }
            break;
    }

    if (text_changed && d->change_fn)
        d->change_fn(w, d->buf, d->change_userdata);
}

/* -------------------------------------------------- */
/* Mouse input                                        */
/* -------------------------------------------------- */

static void tf_on_mouse_button(widget_t *w, wl_mouse_button_event_t *e) {
    if (e->button != 1 || !e->pressed) return;
    tf_data_t *d = w->data;

    if (!d->last_theme || !d->last_theme->font) {
        d->cursor = d->len;
        return;
    }

    /* click_x in physical pixels relative to text start */
    double s = d->last_theme->scale;
    int click_x = (int)(e->x * s) - (int)(w->x * s) - (int)(TF_PAD * s) + d->scroll_x;
    if (click_x < 0) click_x = 0;

    /* walk utf-8 chars, find the one whose midpoint is closest to click_x */
    int pos = 0;
    int best = 0;
    int best_dist = click_x; /* distance from char start to click */

    while (pos < d->len) {
        char tmp[TF_BUFSIZE];
        memcpy(tmp, d->buf, pos);
        tmp[pos] = '\0';
        int px = wl_text_width(d->last_theme->font, tmp);

        int dist = click_x - px;
        if (dist < 0) dist = -dist;
        if (dist < best_dist) {
            best_dist = dist;
            best = pos;
        }

        int next = cursor_right(d->buf, d->len, pos);
        if (next == pos) break;
        pos = next;
    }

    /* also check end of string */
    char tmp[TF_BUFSIZE];
    memcpy(tmp, d->buf, d->len);
    tmp[d->len] = '\0';
    int end_px = wl_text_width(d->last_theme->font, tmp);
    int dist = click_x - end_px;
    if (dist < 0) dist = -dist;
    if (dist < best_dist) best = d->len;

    d->cursor = best;
}

/* -------------------------------------------------- */
/* Measure                                            */
/* -------------------------------------------------- */

static void tf_measure(widget_t *w, wl_theme_t *theme) {
    (void)theme;
    w->pref_h = w->min_h > 0 ? w->min_h : 32;
    w->pref_w = w->min_w > 0 ? w->min_w : 200;
}

/* -------------------------------------------------- */
/* Destroy                                            */
/* -------------------------------------------------- */

static void tf_destroy(widget_t *w) {
    free(w->data);
}

/* -------------------------------------------------- */
/* Draw wrapper that updates scroll first             */
/* -------------------------------------------------- */

static void tf_draw_wrapper(widget_t *w, wl_canvas_t *canvas, wl_theme_t *theme) {
    tf_data_t *d = w->data;
    d->last_theme = theme;
    double s = theme->scale;
    int inner_w = (int)(w->w * s) - (int)(TF_PAD * 2 * s);
    update_scroll(d, theme->font, inner_w, s);
    tf_draw(w, canvas, theme);
}

/* -------------------------------------------------- */
/* Public API                                         */
/* -------------------------------------------------- */

widget_t *textfield_new(const char *placeholder) {
    widget_t *w = widget_new();
    if (!w) return NULL;

    tf_data_t *d = calloc(1, sizeof(*d));
    d->placeholder = placeholder;

    w->data            = d;
    w->draw            = tf_draw_wrapper;
    w->measure         = tf_measure;
    w->on_key          = tf_on_key;
    w->on_mouse_button = tf_on_mouse_button;
    w->destroy         = tf_destroy;
    w->min_h           = 32;
    w->pref_h          = 32;
    w->min_w           = 200;
    w->pref_w          = 200;
    w->hpolicy         = SIZE_EXPAND;
    w->vpolicy         = SIZE_FIXED;

    return w;
}

const char *textfield_get_text(widget_t *w) {
    return ((tf_data_t *)w->data)->buf;
}

void textfield_set_text(widget_t *w, const char *text) {
    tf_data_t *d = w->data;
    int len = strlen(text);
    if (len >= TF_BUFSIZE) len = TF_BUFSIZE - 1;
    memcpy(d->buf, text, len);
    d->buf[len] = '\0';
    d->len      = len;
    d->cursor   = len;
    d->scroll_x = 0;
}

void textfield_on_submit(widget_t *w, textfield_submit_fn fn, void *userdata) {
    tf_data_t *d = w->data;
    d->submit_fn       = fn;
    d->submit_userdata = userdata;
}

void textfield_on_change(widget_t *w, textfield_submit_fn fn, void *userdata) {
    tf_data_t *d = w->data;
    d->change_fn       = fn;
    d->change_userdata = userdata;
}
