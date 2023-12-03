# ----------------------------
# Makefile Options
# ----------------------------

NAME = SNAKE_GAME
ICON = icon.png
DESCRIPTION = "C Snake Game"
COMPRESSED = YES
COMPRESSED_MODE = zx7

CFLAGS = -Wall -Wextra -O2
CXXFLAGS = -Wall -Wextra -O2

# ----------------------------

include $(shell cedev-config --makefile)