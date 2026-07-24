PRODUCT = qr-demo
PLATFORMS = atari apple2

# Compile the shared main.c (src) plus the per-platform driver (src/<platform>/qr.c).
# %PLATFORM% expands to the platform being built.
SRC_DIRS = src src/%PLATFORM%

# Header search paths: main.h (src), qr.h (src/common), any per-platform headers.
INCLUDE_DIRS = src src/common src/%PLATFORM%

# The QR commands aren't in a released fujinet-lib yet, so build against a local
# copy (fujinet-lib-local/ holds the per-platform libs + headers). Once a release
# includes QR support, replace this with a version number, e.g. FUJINET_LIB = 4.13.0
FUJINET_LIB = fujinet-lib-local

# Atari: reserve memory for the player-missile QR bitmap (PM_BASE = 0xA000)
LDFLAGS_EXTRA_ATARI += -Wl -D,__RESERVED_MEMORY__=0x2000

include mekkogx/toplevel-rules.mk
