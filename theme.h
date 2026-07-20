#pragma once

#include <stdint.h>
#include "../wayland.c/build/wayland_dot_c.h"

/* -------------------------------------------------- */
/* Color palette                                      */
/* -------------------------------------------------- */

#define WL_COLOR_BG             0xFF1E1E2E
#define WL_COLOR_BG_WIDGET      0xFF2A2A3E
#define WL_COLOR_BG_HOVER       0xFF313145
#define WL_COLOR_BG_ACTIVE      0xFF3D3D55

#define WL_COLOR_BORDER         0xFF45455A
#define WL_COLOR_BORDER_FOCUSED 0xFF7AA2F7

#define WL_COLOR_TEXT           0xFFCDD6F4
#define WL_COLOR_TEXT_DIM       0xFF6C7086
#define WL_COLOR_TEXT_DISABLED  0xFF45455A

#define WL_COLOR_ACCENT         0xFF7AA2F7

/* -------------------------------------------------- */
/* Theme struct                                       */
/* -------------------------------------------------- */

typedef struct {
    uint32_t bg;
    uint32_t bg_widget;
    uint32_t bg_hover;
    uint32_t bg_active;
    uint32_t border;
    uint32_t border_focused;
    uint32_t text;
    uint32_t text_dim;
    uint32_t text_disabled;
    uint32_t accent;
    wl_font_t *font;
    int padding;
    int spacing;
    int border_width;
    double scale;
} wl_theme_t;

/* default dark theme */
wl_theme_t wl_theme_default(wl_font_t *font);
