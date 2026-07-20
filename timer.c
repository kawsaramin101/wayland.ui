#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "waylandui.h"

static waylandui_t *ui;
static widget_t *timer_label;

typedef struct {
    int remaining;
} timer_state_t;
static void *timer_thread(void *arg) {
    timer_state_t *state = arg;

    while (state->remaining >= 0) {
        printf("%d\n", state->remaining);
        fflush(stdout);

        if (state->remaining == 0)
            break;

        sleep(1);
        state->remaining--;
    }

    return NULL;
}
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
        return 1;
    }

    int seconds = atoi(argv[1]);
    if (seconds < 0) {
        fprintf(stderr, "Seconds must be >= 0\n");
        return 1;
    }

    ui = waylandui_create(
        "Timer",
        320,
        160,
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        16);

    widget_t *root = vbox_new(20, 20, 280, 12);

    widget_t *title = label_new("Countdown Timer");
    timer_label = label_new("");

    widget_add_child(root, title);
    widget_add_child(root, timer_label);

    waylandui_set_root(ui, root);

    timer_state_t state = {
        .remaining = seconds
    };

    pthread_t thread;
    pthread_create(&thread, NULL, timer_thread, &state);

    waylandui_run(ui);

    pthread_join(thread, NULL);
    waylandui_destroy(ui);

    return 0;
}
