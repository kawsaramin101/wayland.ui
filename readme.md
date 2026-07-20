# waylandui

A retained-mode GUI toolkit for Linux, built on top of [wayland.c](https://github.com/kawsaramin101/wayland.c). Provides widgets, layouts, theming, and input handling so you can build real desktop applications without touching raw Wayland protocol code.

Written by Claude (Anthropic). Use at your own responsibility.

---

## What it is

waylandui sits one level above wayland.c — it gives you widgets, layout containers, and a theme system. You describe your UI as a tree of widgets, set up callbacks, and the toolkit handles drawing, input routing, focus management, and layout on resize.

It is a **retained-mode** toolkit, meaning widgets are objects that live in memory. You create them once, update their state when things change, and call `waylandui_redraw()` to repaint.

---

## What it provides

**Widgets:**
- `label` — displays text
- `button` — clickable button with hover/focus states
- `textfield` — single-line text input with cursor, scrolling, placeholder text, `on_change` and `on_submit` callbacks
- `list` — scrollable list of arbitrary widget rows (custom widgets per row, not just strings)

**Layout containers:**
- `vbox` — stacks children vertically with spacing
- `hbox` — stacks children horizontally with spacing
- `padding` — wraps one child with inner space on all sides
- `center` — centers one child horizontally and vertically
- `spacer` — invisible expanding widget that pushes siblings apart

**Size policies** (per widget):
- `SIZE_FIXED` — stays at preferred size
- `SIZE_EXPAND` — takes all available space
- `SIZE_SHRINK` — can shrink below preferred size

**Alignment:** `ALIGN_START`, `ALIGN_CENTER`, `ALIGN_END` — per widget, horizontal and vertical

**Margins** — space outside a widget on any side

**Theme system** — a single `wl_theme_t` struct controls all colors, font, spacing, and border widths. Named color defines make customization easy:
```c
#define WL_COLOR_BG             0xFF1E1E2E
#define WL_COLOR_ACCENT         0xFF7AA2F7
// ... etc
```

**Timer support** — `waylandui_add_timer(ui, interval_ms, fn, userdata)` fires a callback on the main thread at regular intervals, integrated with the event loop (no busy polling).

**Input:** keyboard (with repeat), mouse buttons, mouse move, scroll wheel — all routed automatically to the correct widget.

---

## What it is not

- **Not a graphics engine** — rendering is CPU software rasterization. Fine for UI, not for games or real-time graphics.
- **Not cross-platform** — Wayland is Linux only.
- **Not complete** — missing multiline text input, checkboxes, dropdowns, drag-and-drop, clipboard, and many other common widgets. These can be added on top.
- **Not production hardened** — minimal error handling, no test suite. Review before shipping anything serious.

---

## Dependencies

- [wayland.c](https://github.com/kawsaramin101/wayland.c) — fetched automatically by `make deps`
- `libwayland-client`, `libxkbcommon`, `libfreetype`, `librt` — system libraries

On Debian/Ubuntu:
```sh
sudo apt install libwayland-dev libxkbcommon-dev libfreetype-dev wayland-protocols
```

On Arch:
```sh
sudo pacman -S wayland libxkbcommon freetype2 wayland-protocols
```

---

## Building

```sh
# fetch wayland.c into dependencies/
make deps

# build waylandui
make

# build example app
make example
```

Produces `build/libwaylandui.a`. Link your app against both:

```sh
cc -o myapp main.c \
   -I dependencies/wayland.c/build -I . \
   build/libwaylandui.a \
   dependencies/wayland.c/build/libwayland_dot_c.a \
   -lwayland-client -lrt -lxkbcommon -lfreetype -lm
```

---

## Usage

```c
#include "waylandui.h"

static waylandui_t *ui;
static widget_t    *counter_label;
static int          count = 0;

void on_click(widget_t *w, void *userdata) {
    (void)w; (void)userdata;
    count++;
    char buf[32];
    snprintf(buf, sizeof(buf), "Count: %d", count);
    label_set_text(counter_label, buf);
    waylandui_redraw(ui);
}

int main(void) {
    ui = waylandui_create("My App", 400, 300,
                          "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 13);

    widget_t *pad  = padding_new(16, 16, 16, 16);
    widget_t *root = vbox_new(0, 0, 0, 12);
    widget_set_hpolicy(root, SIZE_EXPAND);
    widget_set_vpolicy(root, SIZE_EXPAND);

    counter_label = label_new("Count: 0");
    widget_t *btn = button_new("Increment", on_click, NULL);

    widget_add_child(root, counter_label);
    widget_add_child(root, btn);
    widget_add_child(pad, root);

    waylandui_set_root(ui, pad);
    waylandui_run(ui);
    waylandui_destroy(ui);
    return 0;
}
```

---

## Custom theme

```c
waylandui_t *ui = waylandui_create(...);

wl_theme_t *theme     = waylandui_theme(ui);
theme->bg             = 0xFF1C1C1C;
theme->bg_widget      = 0xFF252525;
theme->accent         = 0xFF4EC9B0;
theme->text           = 0xFFD4D4D4;
theme->border_focused = 0xFF4EC9B0;
```

---

## Custom list rows

```c
widget_t *list = list_new(32);

widget_t *row    = hbox_new(0, 0, 32, 8);
widget_t *icon   = label_new("/");
widget_t *name   = label_new("Documents");
widget_t *spacer = spacer_new();
widget_t *count  = label_new("42");

widget_add_child(row, icon);
widget_add_child(row, name);
widget_add_child(row, spacer);
widget_add_child(row, count);

list_add_row(list, row);
list_on_select(list, on_select, NULL);
```

---

## Timers

```c
int id = waylandui_add_timer(ui, 1000, on_tick, userdata);
waylandui_remove_timer(ui, id);
```

---

## Files

| File | Purpose |
|---|---|
| `waylandui.c/h` | Main entry point, event loop integration, timer passthrough |
| `widget.c/h` | Base widget struct, tree operations, event dispatch |
| `layout.c/h` | VBox, HBox, Padding, Center, Spacer containers |
| `label.c/h` | Text display widget |
| `button.c/h` | Clickable button widget |
| `textfield.c/h` | Single-line text input widget |
| `list.c/h` | Scrollable list with custom widget rows |
| `theme.c/h` | Theme struct and default dark theme |
| `example.c` | Demo app showing all widgets |

---

## License

Do whatever you want with it.
