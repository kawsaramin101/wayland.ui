#pragma once

#include "widget.h"

/* VBox — stacks children vertically */
widget_t *vbox_new(int x, int y, int w, int spacing);

/* HBox — stacks children horizontally */
widget_t *hbox_new(int x, int y, int h, int spacing);

/* Spacer — invisible expanding widget, pushes siblings apart */
widget_t *spacer_new(void);

/* Padding — wraps one child with inner space on all sides */
widget_t *padding_new(int top, int right, int bottom, int left);

/* Center — centers one child both horizontally and vertically */
widget_t *center_new(void);
