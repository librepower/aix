Name:           doom-ascii
Version:        0.3.1
Release:        1.librepower
Summary:        Text-based DOOM for terminal - AIX/POWER port
License:        GPL-2.0
URL:            https://github.com/wojciech-graj/doom-ascii
Group:          Amusements/Games

%description
DOOM ASCII renders the classic DOOM game entirely in your terminal using
ASCII or Unicode block characters. This is a port for IBM AIX on POWER
architecture with fixes for big-endian byte ordering.

Features:
- ASCII, Unicode block, or braille character rendering
- Multiple scaling options for different terminal sizes
- 256-color support
- Portable UTF-8 locale detection

Note: You need a WAD file (game data) to play. The shareware DOOM1.WAD
is freely available.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/doom-ascii
mkdir -p %{buildroot}/opt/freeware/share/doc/doom-ascii

# Install binary
cp /tmp/doom-ascii/_unix/game/doom-ascii %{buildroot}/opt/freeware/bin/doom-ascii
chmod 755 %{buildroot}/opt/freeware/bin/doom-ascii

# Install default config
cp /tmp/doom-ascii/_unix/game/.default.cfg %{buildroot}/opt/freeware/share/doom-ascii/default.cfg
chmod 644 %{buildroot}/opt/freeware/share/doom-ascii/default.cfg

# Create wrapper script with locale setup
cat > %{buildroot}/opt/freeware/bin/doom << 'WRAPPER'
#!/bin/bash
# DOOM ASCII wrapper - sets up environment for best experience

# Set UTF-8 locale for Unicode characters if available
if locale -a 2>/dev/null | grep -qi "EN_US.UTF-8"; then
    export LANG=EN_US.UTF-8
    export LC_ALL=EN_US.UTF-8
fi

# Set terminal type
export TERM=${TERM:-xterm-256color}

# Default WAD locations to search
WAD_PATHS=(
    "$HOME/.doom/doom1.wad"
    "$HOME/doom1.wad"
    "/opt/freeware/share/doom-ascii/doom1.wad"
    "./doom1.wad"
)

# Find WAD file
IWAD=""
for path in "${WAD_PATHS[@]}"; do
    if [ -f "$path" ]; then
        IWAD="$path"
        break
    fi
done

if [ -z "$IWAD" ] && [ -z "$1" ]; then
    echo "DOOM ASCII - Text-based DOOM for AIX/POWER"
    echo ""
    echo "No WAD file found. Download the shareware WAD:"
    echo "  mkdir -p ~/.doom"
    echo "  /opt/freeware/bin/curl -L -o ~/.doom/doom1.wad \\"
    echo "    https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad"
    echo ""
    echo "Then run: doom"
    echo ""
    echo "Or specify WAD location:"
    echo "  doom -iwad /path/to/doom1.wad"
    echo ""
    echo "Options:"
    echo "  -chars block -scaling 2    Best quality (UTF-8 required)"
    echo "  -scaling 2                 ASCII mode"
    exit 1
fi

# Run doom-ascii with found WAD or user arguments
if [ -n "$IWAD" ] && [ -z "$1" ]; then
    exec /opt/freeware/bin/doom-ascii -iwad "$IWAD" -chars block -scaling 2
else
    exec /opt/freeware/bin/doom-ascii "$@"
fi
WRAPPER
chmod 755 %{buildroot}/opt/freeware/bin/doom

# Create README
cat > %{buildroot}/opt/freeware/share/doc/doom-ascii/README << 'README'
DOOM ASCII for AIX/POWER
========================

Quick Start:
  1. Download shareware WAD:
     mkdir -p ~/.doom
     /opt/freeware/bin/curl -L -o ~/.doom/doom1.wad \
       https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad

  2. Run:
     doom                              # Auto-detect WAD, best settings
     doom -iwad /path/to/doom.wad      # Specify WAD location

Display Options:
  -chars block    Unicode block characters (requires UTF-8 locale)
  -chars braille  Braille patterns (highest resolution, requires UTF-8)
  -chars ascii    ASCII characters (always works)
  -scaling N      Resolution (1=highest, 4=lowest, default=4)
  -nocolor        Disable colors

Recommended:
  doom -chars block -scaling 2

For UTF-8 support, install: bos.loc.utf.EN_US

Controls:
  Arrow keys     Move/Turn
  Space          Fire
  E              Use/Open
  ,/.            Strafe
  1-7            Weapons
  Escape         Menu

Port by LibrePower - https://gitlab.com/librepower/aix
README
chmod 644 %{buildroot}/opt/freeware/share/doc/doom-ascii/README

%files
%defattr(-,root,system)
/opt/freeware/bin/doom-ascii
/opt/freeware/bin/doom
/opt/freeware/share/doom-ascii/default.cfg
/opt/freeware/share/doc/doom-ascii/README

%changelog
* Tue Jan 07 2025 LibrePower <hello@librepower.org> - 0.3.1-1.aix
- Initial AIX/POWER port
- Fixed big-endian byte ordering for POWER architecture
- Fixed col_t type conflict with AIX headers
- Added portable UTF-8 locale detection
- 64-bit XCOFF binary
