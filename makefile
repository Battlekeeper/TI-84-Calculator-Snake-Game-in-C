# ----------------------------
# Makefile Options
# ----------------------------

NAME = DEMO
ICON = icon.png
DESCRIPTION = "CE C Toolchain Demo"
COMPRESSED = YES
COMPRESSED_MODE = zx7

CFLAGS = -Wall -Wextra -O2
CXXFLAGS = -Wall -Wextra -O2

# ----------------------------

include $(shell cedev-config --makefile)