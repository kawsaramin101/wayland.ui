CC     = cc
CFLAGS = -Wall -Wextra -O2
LIBS   = -lwayland-client -lrt -lxkbcommon -lfreetype -lm

LIBWL  = ../wayland.c/build/libwayland_dot_c.a
INCWL  = ../wayland.c/build

BUILD  = build

SRCS   = waylandui.c widget.c layout.c label.c button.c textfield.c list.c theme.c
OBJS   = $(SRCS:%.c=$(BUILD)/%.o)

all: $(BUILD)/libwaylandui.a $(BUILD)/example

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -I$(INCWL) -c $< -o $@

$(BUILD)/libwaylandui.a: $(OBJS)
	ar rcs $@ $^

$(BUILD)/example.o: example.c waylandui.h | $(BUILD)
	$(CC) $(CFLAGS) -I$(INCWL) -c example.c -o $@

$(BUILD)/example: $(BUILD)/example.o $(BUILD)/libwaylandui.a $(LIBWL)
	$(CC) $(CFLAGS) -o $@ $< $(BUILD)/libwaylandui.a $(LIBWL) $(LIBS)

clean:
	rm -rf $(BUILD)

.PHONY: all clean
