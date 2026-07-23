#include <stdlib.h>
#include "waylandui.h"

struct waylandui {
    wl_app_t   *app;
    wl_font_t  *font;
    wl_theme_t  theme;
    widget_t   *root;
     wl_key_fn   fallback_key_fn;
    void       *fallback_key_userdata;
    double      scale;
};

static waylandui_t *g_ui = NULL;

static void draw_cb(wl_canvas_t *canvas) {
    if (!g_ui || !g_ui->root) return;
    g_ui->scale = canvas->scale;
    g_ui->theme.scale = canvas->scale;
    wl_draw_fill(canvas, g_ui->theme.bg);
    widget_draw(g_ui->root, canvas, &g_ui->theme);
}

static void key_cb(wl_key_event_t *e, void *userdata) {
    waylandui_t *ui = userdata;
    if (!ui->root) return;
    if (widget_dispatch_key(ui->root, e)) {
        waylandui_redraw(ui);
        return;
    }
    /* not consumed by any widget — fire fallback */
    if (ui->fallback_key_fn)
        ui->fallback_key_fn(e, ui->fallback_key_userdata);
}

static void mouse_button_cb(wl_mouse_button_event_t *e, void *userdata) {
    waylandui_t *ui = userdata;
    if (!ui->root) return;
    if (widget_dispatch_mouse_button(ui->root, e))
        waylandui_redraw(ui);
}

static void mouse_move_cb(wl_mouse_move_event_t *e, void *userdata) {
    waylandui_t *ui = userdata;
    if (!ui->root) return;
    widget_dispatch_mouse_move(ui->root, e);
    waylandui_redraw(ui);
}

static void resize_cb(int w, int h, void *userdata) {
    waylandui_t *ui = userdata;
    if (!ui->root) return;
    ui->root->w = w;
    ui->root->h = h;
    widget_layout(ui->root, &ui->theme);
}

static void scroll_cb(wl_scroll_event_t *e, void *userdata) {
    waylandui_t *ui = userdata;
    if (!ui->root) return;

    widget_t *hit = widget_at(ui->root, (int)e->x, (int)e->y);

    /* walk up the tree until we find a widget that handles scroll */
    while (hit && !hit->on_scroll)
        hit = hit->parent;

    if (hit && hit->on_scroll) {
        hit->on_scroll(hit, e);
        waylandui_redraw(ui);
    }
}

waylandui_t *waylandui_create(const char *title, int w, int h,
                               const char *font_path, int font_size)
{
    waylandui_t *ui = calloc(1, sizeof(*ui));
    if (!ui) return NULL;
    ui->app   = wl_app_create(title, w, h);
    ui->font  = wl_font_load(font_path, font_size);
    ui->theme = wl_theme_default(ui->font);
    ui->scale = 1.0;

    g_ui = ui;

    wl_app_on_draw        (ui->app, draw_cb);
    wl_app_on_resize      (ui->app, resize_cb, ui);
    wl_app_on_key         (ui->app, key_cb, ui);
    wl_app_on_mouse_move  (ui->app, mouse_move_cb, ui);
    wl_app_on_mouse_button(ui->app, mouse_button_cb, ui);
    wl_app_on_scroll      (ui->app, scroll_cb, ui);

    return ui;
}

void waylandui_set_root(waylandui_t *ui, widget_t *root) {
    ui->root = root;
    /* give root the full window size so expand works */
    int w, h;
    wl_app_size(ui->app, &w, &h);
    root->w = w;
    root->h = h;
    widget_measure(root, &ui->theme);
    widget_layout(root, &ui->theme);
}

wl_theme_t *waylandui_theme(waylandui_t *ui) {
    return &ui->theme;
}

void waylandui_redraw(waylandui_t *ui) {
    wl_app_redraw(ui->app);
}

int waylandui_add_timer(waylandui_t *ui, int interval_ms,
                        wl_timer_fn fn, void *userdata) {
    return wl_app_add_timer(ui->app, interval_ms, fn, userdata);
}

void waylandui_remove_timer(waylandui_t *ui, int timer_id) {
    wl_app_remove_timer(ui->app, timer_id);
}

void waylandui_run(waylandui_t *ui) {
    wl_app_run(ui->app);
}

void waylandui_destroy(waylandui_t *ui) {
    if (!ui) return;
    widget_destroy(ui->root);
    wl_font_destroy(ui->font);
    wl_app_destroy(ui->app);
    free(ui);
}

void waylandui_on_key(waylandui_t *ui, wl_key_fn fn, void *userdata) {
    wl_app_on_key(ui->app, fn, userdata);
}

void waylandui_on_unhandled_key(waylandui_t *ui, wl_key_fn fn, void *userdata) {
    ui->fallback_key_fn       = fn;
    ui->fallback_key_userdata = userdata;
}

void waylandui_on_mouse_button(waylandui_t *ui, wl_mouse_button_fn fn, void *userdata) {
    wl_app_on_mouse_button(ui->app, fn, userdata);
}
