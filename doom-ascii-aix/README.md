# DOOM-ASCII for AIX

**Text-based DOOM running on IBM AIX/POWER!**

This is a port of [doom-ascii](https://github.com/wojciech-graj/doom-ascii) to IBM AIX on POWER architecture (big-endian).

![DOOM ASCII](screenshots/logo.png)

## Quick Install

```bash
# Install via DNF (recommended)
dnf install doom-ascii

# Download shareware WAD
mkdir -p ~/.doom
/opt/freeware/bin/curl -L -o ~/.doom/doom1.wad \
  https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad

# Play!
doom
```

## Requirements

- IBM AIX 7.2 or later on POWER architecture
- For Unicode block/braille characters: UTF-8 locale (`bos.loc.utf.EN_US`)

## Manual Install

Download the RPM from [RPMS/](RPMS/) and install:

```bash
rpm -ivh doom-ascii-0.3.1-1.aix.aix7.3.ppc.rpm
```

## Building from Source

Requires GCC from AIX Toolbox (`/opt/freeware/bin/gcc`).

```bash
# Set up environment
export PATH=/opt/freeware/bin:$PATH
export CC=/opt/freeware/bin/gcc
export OBJECT_MODE=64

# Build
cd doom-ascii
make PLATFORM=unix \
  CFLAGS="-maix64 -DNORMALUNIX -DLINUX -O2 -Wall -D_DEFAULT_SOURCE -DVERSION=0.3.1 -std=c99" \
  LDFLAGS="-maix64"

# Binary will be at _unix/game/doom-ascii
```

## Running

You need a WAD file (game data). The shareware version (doom1.wad) is freely available.

```bash
cd _unix/game

# Download shareware WAD
/opt/freeware/bin/curl -L -o doom1.wad "https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad"

# Run with ASCII characters (always works)
./doom-ascii -iwad doom1.wad

# Run with Unicode block characters (best quality, requires UTF-8)
export LANG=EN_US.UTF-8
./doom-ascii -iwad doom1.wad -chars block -scaling 2
```

## UTF-8 Locale Setup (Required for -chars block/braille)

To use Unicode block or braille characters, install the UTF-8 locale:

```bash
# Install from AIX media
installp -aXYgd /dev/cd0 bos.loc.utf.EN_US

# Set locale
export LANG=EN_US.UTF-8
export LC_ALL=EN_US.UTF-8
```

Note: AIX uses `EN_US.UTF-8` (uppercase) instead of `en_US.UTF-8`.

## Display Quality Settings

| Option | Resolution | Requirements |
|--------|-----------|--------------|
| `-chars braille -scaling 1` | Maximum (2x4 dots/char) | Large terminal + UTF-8 |
| `-chars block -scaling 1` | Very high (2 px/char) | Large terminal + UTF-8 |
| `-chars block -scaling 2` | High | Normal terminal + UTF-8 |
| `-chars ascii -scaling 2` | Medium | Any terminal |
| `-chars ascii -scaling 4` | Low (default) | Any terminal |

**Recommended for best experience:**
```bash
export LANG=EN_US.UTF-8
export TERM=xterm-256color
./doom-ascii -iwad doom1.wad -chars block -scaling 2
```

**Tips:**
- Use a smaller font in your terminal for higher resolution
- Maximize the terminal window
- Use a monospace font (Menlo, Monaco, SF Mono)
- iTerm2, xterm, or similar modern terminal recommended

## Controls

| Action | Key |
|--------|-----|
| Move forward/back | Arrow Up/Down |
| Turn left/right | Arrow Left/Right |
| Strafe left/right | , / . |
| Fire | Space |
| Use/Open door | E |
| Run | ] |
| Weapon select | 1-7 |
| Menu | Escape |

## AIX-Specific Fixes

This port includes the following fixes for AIX/POWER:

1. **Endianness**: Byte-swapping in `i_swap.h` for big-endian POWER architecture
2. **Type conflict**: Renamed `col_t` to `doom_col_t` to avoid conflict with AIX system headers
3. **Portable locale**: Multiple UTF-8 locale name fallbacks in `doomgeneric_ascii.c`
4. **64-bit build**: Uses `-maix64` flags for 64-bit XCOFF executable
5. **No LTO**: Removed `-flto` flag (not supported by AIX GCC)

## Credits

- Original doom-ascii: [wojciech-graj](https://github.com/wojciech-graj/doom-ascii)
- doomgeneric: [ozkl](https://github.com/ozkl/doomgeneric)
- AIX port: [LibrePower](https://gitlab.com/librepower)

## License

Same as original doom-ascii (GPL-2.0)
