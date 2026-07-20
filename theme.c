#include "theme.h"

wl_theme_t wl_theme_default(wl_font_t *font) {
    return (wl_theme_t){
        .bg             = WL_COLOR_BG,
        .bg_widget      = WL_COLOR_BG_WIDGET,
        .bg_hover       = WL_COLOR_BG_HOVER,
        .bg_active      = WL_COLOR_BG_ACTIVE,
        .border         = WL_COLOR_BORDER,
        .border_focused = WL_COLOR_BORDER_FOCUSED,
        .text           = WL_COLOR_TEXT,
        .text_dim       = WL_COLOR_TEXT_DIM,
        .text_disabled  = WL_COLOR_TEXT_DISABLED,
        .accent         = WL_COLOR_ACCENT,
        .font           = font,
        .padding        = 6,
        .spacing        = 8,
        .border_width   = 1,
        .scale          = 1.0,
    };
}
