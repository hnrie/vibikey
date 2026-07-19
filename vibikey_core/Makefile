# VibiKey - Vietnamese Input Method
# Cross-platform Makefile
#
# Targets:
#   make              - Build for current platform
#   make linux        - Build for Linux (requires libX11-dev, libncurses-dev)
#   make windows      - Cross-compile for Windows (requires MinGW-w64)
#   make clean        - Remove build artifacts
#
# Variables:
#   CC                - C compiler (default: gcc)
#   MSVC              - MSVC cl.exe path (for Windows MSVC build)

CC ?= gcc
MSVC ?= cl.exe

# Common source files
COMMON_SRCS = config.c vietnamese_tep.c vibikey_input_method_dialog.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

# Output
TARGET = vibikey

.PHONY: all linux windows clean test help

all: linux

help:
	@echo "VibiKey - Vietnamese Input Method"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build for current platform (Linux)"
	@echo "  make linux        - Build for Linux"
	@echo "  make windows      - Cross-compile for Windows with MinGW"
	@echo "  make windows-msvc - Build for Windows with MSVC"
	@echo "  make test         - Build and run test"
	@echo "  make clean        - Remove build artifacts"

# ============================================================================
# Linux build
# ============================================================================

HAS_X11 := $(shell $(CC) -lX11 -x c /dev/null -o /dev/null 2>/dev/null && echo yes)
HAS_NCURSES := $(shell $(CC) -lncurses -x c /dev/null -o /dev/null 2>/dev/null && echo yes)

linux: CFLAGS += -DVIBIKEY_LINUX -Wall -Wextra -O2 -std=c11
ifeq ($(HAS_X11),yes)
linux: LDFLAGS += -lX11
endif
ifeq ($(HAS_NCURSES),yes)
linux: LDFLAGS += -lncurses
endif
linux: $(TARGET)

# ============================================================================
# Windows build (MinGW cross-compile)
# ============================================================================

windows: CC = x86_64-w64-mingw32-gcc
windows: CFLAGS += -DVIBIKEY_WINDOWS -Wall -Wextra -O2 -std=c11 -mwindows
windows: LDFLAGS += -luser32 -lgdi32 -lkernel32
windows: $(TARGET).exe

# ============================================================================
# Windows build (MSVC)
# ============================================================================

windows-msvc: CC = $(MSVC)
windows-msvc: CFLAGS = /nologo /DVIBIKEY_WINDOWS /W3 /O2 /std:c11
windows-msvc: LDFLAGS = /nologo user32.lib gdi32.lib kernel32.lib
windows-msvc: $(TARGET)_msvc.exe

# ============================================================================
# Object files
# ============================================================================

# Common objects
config.o: config.c config.h
	$(CC) $(CFLAGS) -c $< -o $@

vietnamese_tep.o: vietnamese_tep.c vietnamese_tep.h
	$(CC) $(CFLAGS) -c $< -o $@

vibikey_input_method_dialog.o: vibikey_input_method_dialog.c vibikey_input_method_dialog.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

# Platform-specific objects
vibikey_linux_daemon.o: linux/vibikey_linux_daemon.c linux/vibikey_linux_daemon.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

vibikey_linux_dialog.o: linux/vibikey_linux_dialog.c vibikey_input_method_dialog.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

vibikey_windows_hook.o: windows/vibikey_windows_hook.c windows/vibikey_windows_hook.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

vibikey_windows_dialog.o: windows/vibikey_windows_dialog.c vibikey_input_method_dialog.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

# Main objects
main.o: main.c config.h vibikey_input_method_dialog.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

# ============================================================================
# Link targets
# ============================================================================

# Linux objects
LINUX_OBJS = main.o $(COMMON_OBJS) vibikey_linux_daemon.o vibikey_linux_dialog.o

$(TARGET): $(LINUX_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Built $(TARGET) for Linux"

# Windows objects (MinGW)
WIN_OBJS = main.o $(COMMON_OBJS) vibikey_windows_hook.o vibikey_windows_dialog.o

$(TARGET).exe: $(WIN_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Built $(TARGET).exe for Windows (MinGW)"

# Windows objects (MSVC)
WIN_MSVC_OBJS = main.obj config.obj vietnamese_tep.obj vibikey_input_method_dialog.obj vibikey_windows_hook.obj vibikey_windows_dialog.obj

$(TARGET)_msvc.exe: $(WIN_MSVC_OBJS)
	$(CC) $(LDFLAGS) /Fe:$@ $^
	@echo "Built $(TARGET)_msvc.exe for Windows (MSVC)"

# MSVC object files
%.obj: %.c
	$(CC) $(CFLAGS) /c $<

# ============================================================================
# Test
# ============================================================================

test: CFLAGS += -DVIBIKEY_TEST
test:
	$(CC) $(CFLAGS) -o test_runner vietnamese_tep.c
	./test_runner
	rm -f test_runner

# ============================================================================
# Clean
# ============================================================================

clean:
	rm -f $(TARGET) $(TARGET).exe $(TARGET)_msvc.exe
	rm -f *.o *.obj
	rm -f windows/*.o windows/*.obj
	rm -f linux/*.o linux/*.obj