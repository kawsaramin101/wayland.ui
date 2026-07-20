#include <stdio.h>
#include <string.h>
#include "waylandui.h"

static waylandui_t *ui;
static widget_t    *status_label;
static widget_t    *search_field;
static widget_t    *file_list;

static const char *files[] = {
    "main.c", "wayland.c", "wayland.h", "input.c", "input.h",
    "font.c", "font.h", "widget.c", "widget.h", "layout.c",
    "layout.h", "label.c", "button.c", "textfield.c", "list.c",
    "theme.c", "waylandui.c", "waylandui.h", "Makefile", "README.md",
    "main.c", "wayland.c", "wayland.h", "input.c", "input.h",
    "font.c", "font.h", "widget.c", "widget.h", "layout.c",
    "layout.h", "label.c", "button.c", "textfield.c", "list.c",
    "theme.c", "waylandui.c", "waylandui.h", "Makefile", "README.md",
};
static int file_count = 20;

/* build a row widget for a file entry */
static widget_t *make_row(const char *name) {
    widget_t *row   = hbox_new(0, 0, 28, 8);
    widget_t *label = label_new(name);
    widget_set_margin(label, 0, 0, 0, 8);
    widget_add_child(row, label);
    return row;
}

static void rebuild_list(const char *filter) {
    list_clear(file_list);
    for (int i = 0; i < file_count; i++) {
        if (filter && filter[0] && !strstr(files[i], filter)) continue;
        list_add_row(file_list, make_row(files[i]));
    }
    waylandui_redraw(ui);
}

static void on_select(widget_t *w, int index, void *userdata) {
    (void)w; (void)userdata;
    /* get label text from row's first child */
    widget_t *row = file_list->children[index];
    if (row && row->child_count > 0) {
        const char *name = label_get_text(row->children[0]);
        char buf[256];
        snprintf(buf, sizeof(buf), "Selected: %s", name);
        label_set_text(status_label, buf);
        waylandui_redraw(ui);
    }
}

static void on_search(widget_t *w, const char *text, void *userdata) {
    (void)w; (void)userdata;
    rebuild_list(text);
}

int main(void) {
    ui = waylandui_create("File List Demo", 400, 500,
                          "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 13);

    widget_t *pad  = padding_new(12, 12, 12, 12);
    widget_t *root = vbox_new(0, 0, 400, 8);
    widget_set_vpolicy(root, SIZE_EXPAND);
    widget_set_hpolicy(root, SIZE_EXPAND);

    widget_t *title = label_new("Files");

    search_field = textfield_new("Search...");
    textfield_on_change(search_field, on_search, NULL);

    file_list = list_new(28);
    list_on_select(file_list, on_select, NULL);
    widget_set_vpolicy(file_list, SIZE_EXPAND);

    status_label = label_new("Click a file to select");

    widget_add_child(root, title);
    widget_add_child(root, search_field);
    widget_add_child(root, file_list);
    widget_add_child(root, status_label);
    widget_add_child(pad, root);

    waylandui_set_root(ui, pad);

    rebuild_list(NULL);
    widget_layout(pad, waylandui_theme(ui));
    waylandui_run(ui);
    waylandui_destroy(ui);
    return 0;
}
