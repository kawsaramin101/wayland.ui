CC     = cc
CFLAGS = -Wall -Wextra -O2
LIBS   = -lwayland-client -lrt -lxkbcommon -lfreetype -lm

DEPS_DIR = dependencies
WAYLAND_C_REPO = https://github.com/kawsaramin101/wayland.c
WAYLAND_C_DIR  = $(DEPS_DIR)/wayland.c
WAYLAND_C_LIB  = $(WAYLAND_C_DIR)/build/libwayland_dot_c.a
WAYLAND_C_INC  = $(WAYLAND_C_DIR)/build

BUILD = build

SRCS = waylandui.c widget.c layout.c label.c button.c textfield.c list.c theme.c
OBJS = $(SRCS:%.c=$(BUILD)/%.o)

# -------------------------------------------------- #
# Targets                                            #
# -------------------------------------------------- #

all: lib example

lib: $(WAYLAND_C_LIB) $(BUILD)/libwaylandui.a

example: $(BUILD)/example

deps: $(WAYLAND_C_LIB)

# -------------------------------------------------- #
# Fetch and build wayland.c                          #
# -------------------------------------------------- #

$(WAYLAND_C_LIB):
	@echo "Fetching wayland.c..."
	@mkdir -p $(DEPS_DIR)
	@if [ ! -d "$(WAYLAND_C_DIR)/.git" ]; then \
		git clone $(WAYLAND_C_REPO) $(WAYLAND_C_DIR); \
	else \
		echo "wayland.c already fetched, pulling latest..."; \
		git -C $(WAYLAND_C_DIR) pull; \
	fi
	@echo "Building wayland.c..."
	$(MAKE) -C $(WAYLAND_C_DIR) lib

# -------------------------------------------------- #
# waylandui library                                  #
# -------------------------------------------------- #

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c | $(BUILD) $(WAYLAND_C_LIB)
	$(CC) $(CFLAGS) -I$(WAYLAND_C_INC) -c $< -o $@

$(BUILD)/libwaylandui.a: $(OBJS)
	ar rcs $@ $^

# -------------------------------------------------- #
# Example                                            #
# -------------------------------------------------- #

$(BUILD)/example.o: example.c waylandui.h | $(BUILD)
	$(CC) $(CFLAGS) -I$(WAYLAND_C_INC) -c example.c -o $@

$(BUILD)/example: $(BUILD)/example.o $(BUILD)/libwaylandui.a $(WAYLAND_C_LIB)
	$(CC) $(CFLAGS) -o $@ $< $(BUILD)/libwaylandui.a $(WAYLAND_C_LIB) $(LIBS)

# -------------------------------------------------- #
# Clean                                              #
# -------------------------------------------------- #

clean:
	rm -rf $(BUILD)

clean-deps:
	rm -rf $(DEPS_DIR)

distclean: clean clean-deps

.PHONY: all lib example deps clean clean-deps distclean
