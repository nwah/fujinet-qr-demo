# qr-demo

A cross-platform FujiNet demo that generates and displays QR codes using the
fujinet-lib QR commands.

## Building

This project uses the [MekkoGX](https://github.com/fozzTexx/MekkoGX) modular
Makefile framework. All project-specific configuration lives in the top-level
[Makefile](Makefile); everything under [mekkogx/](mekkogx) is reusable framework
code that should not need editing.

Make sure the toolchain for each platform is on your `PATH` (cc65 for atari and
apple2), then:

```shell
# Build every platform in PLATFORMS
make

# Build a single platform
make atari
make apple2

# Build a specific platform target (executable, disk image, clean, ...)
make atari/disk
make apple2/r2r

# Remove all build artifacts
make clean
```

Build outputs are written to `r2r/<platform>/`:

- `r2r/atari/qr-demo.com` and a bootable `qr-demo.atr`
- `r2r/apple2/qr-demo.a2s` and a bootable `qr-demo.po`

### Configuring the build

The top-level `Makefile` declares *what* to build:

- `PRODUCT` / `PLATFORMS` — the app name and the platforms to build.
- `SRC_DIRS` / `INCLUDE_DIRS` — where sources and headers live (`%PLATFORM%`
  expands to the platform being built).
- `FUJINET_LIB` — which fujinet-lib to link against. It accepts a version
  number (downloaded from GitHub releases), a directory, a zip, or a git URL.
- `LDFLAGS_EXTRA_<PLATFORM>` / `CFLAGS_EXTRA_<PLATFORM>` — per-platform flags.

See [mekkogx/README.md](mekkogx/README.md) for the full framework documentation.

### fujinet-lib

The QR commands this demo uses are not yet in a released fujinet-lib, so
`FUJINET_LIB` points at a local directory (`fujinet-lib-local/`) containing a
locally built, QR-enabled library plus its headers. Rebuild that library from
the [fujinet-lib](https://github.com/FujiNetWIFI/fujinet-lib) repo and drop the
per-platform `.lib` files and headers into `fujinet-lib-local/`.

Once QR support ships in a release, simply set `FUJINET_LIB` to that version
number in the Makefile, e.g.:

```make
FUJINET_LIB = 4.13.0
```

### Disk-image tools

Creating bootable disk images needs extra host tools:

- **atari** — [dir2atr](https://github.com/HiassofT/AtariSIO) and
  [atr](https://github.com/jhallen/atari-tools)
- **apple2** — [AppleCommander](https://github.com/AppleCommander/AppleCommander/releases/)
  (`ac` and `acx` on the `PATH`)

Building just the executable only requires the compiler.
