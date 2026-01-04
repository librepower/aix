#============================================================================
# nano.spec - GNU nano text editor for AIX
#============================================================================
# Part of LibrePower - Open Source for IBM Power
# https://github.com/librepower/aix
#============================================================================

Name:           nano
Version:        8.3
Release:        3.librepower
Summary:        GNU nano - A simple, friendly text editor
License:        GPL-3.0
Group:          Applications/Editors
URL:            https://www.nano-editor.org/
Vendor:         SIXE - IBM Business Partner
Packager:       Hugo Blanco <hugo.blanco@sixe.eu>

Source0:        %{name}-%{version}.tar.xz

BuildRequires:  gcc
BuildRequires:  ncurses-devel
Requires:       ncurses

%description
GNU nano is a small and friendly text editor. It aims to replace Pico,
the default editor included in the Pine package, while also offering
additional functionality.

Features:
- Syntax highlighting for 40+ file types (enabled by default)
- Multiple buffers
- Search and replace with regex support
- Line numbering
- Auto-indentation
- UTF-8 support

Compiled for AIX by SIXE - IBM Business Partner
https://sixe.eu | Part of LibrePower initiative
https://librepower.org

%prep
%setup -q

%build
export PATH=/opt/freeware/bin:$PATH
export CC=gcc
export CFLAGS="-maix64 -O2"
export LDFLAGS="-L/opt/freeware/lib"
export CPPFLAGS="-I/opt/freeware/include"
export OBJECT_MODE=64

./configure --prefix=/opt/freeware --disable-nls --disable-libmagic

# Touch doc files to prevent texinfo rebuild
touch doc/nano.1 doc/rnano.1 doc/nanorc.5
touch doc/nano.html doc/nano.info 2>/dev/null || true
echo "" > doc/nano.html 2>/dev/null || true

export OBJECT_MODE=64
make -j12

%install
export OBJECT_MODE=64
make DESTDIR=%{buildroot} install

# Create rnano symlink
cd %{buildroot}/opt/freeware/bin
ln -sf nano rnano

# Create global nanorc with syntax highlighting enabled
mkdir -p %{buildroot}/opt/freeware/etc
cat > %{buildroot}/opt/freeware/etc/nanorc << 'NANORC'
## Global nanorc configuration for AIX
## Part of LibrePower - https://librepower.org
##
## User settings in ~/.nanorc override these defaults

## Enable syntax highlighting for all supported file types
include "/opt/freeware/share/nano/*.nanorc"

## Auto-indent new lines
set autoindent

## Show line numbers (uncomment to enable)
# set linenumbers

## Enable mouse support (uncomment if your terminal supports it)
# set mouse

## Convert tabs to spaces (uncomment to enable)
# set tabstospaces
# set tabsize 4
NANORC

%files
%defattr(-,root,system,-)
%doc AUTHORS COPYING NEWS README THANKS
%config(noreplace) /opt/freeware/etc/nanorc
/opt/freeware/bin/nano
/opt/freeware/bin/rnano
/opt/freeware/share/nano/
/opt/freeware/share/man/man1/nano.1
/opt/freeware/share/man/man1/rnano.1
/opt/freeware/share/man/man5/nanorc.5

%post
echo "========================================================"
echo " GNU nano %{version} installed successfully"
echo ""
echo " Syntax highlighting is ENABLED by default for 40+ languages"
echo ""
echo " Usage: nano filename"
echo " Help:  nano --help"
echo ""
echo " Config: /opt/freeware/etc/nanorc (global)"
echo "         ~/.nanorc (user overrides)"
echo ""
echo " SIXE - IBM Business Partner (https://sixe.eu)"
echo " Part of LibrePower (https://librepower.org)"
echo "========================================================"

%changelog
* Sat Jan 04 2025 Hugo Blanco <hugo.blanco@sixe.eu> - 8.3-3.librepower
- Fix nanorc: remove deprecated 'set smooth' option

* Sat Jan 04 2025 Hugo Blanco <hugo.blanco@sixe.eu> - 8.3-2.librepower
- Enable syntax highlighting by default via global nanorc

* Sat Jan 04 2025 Hugo Blanco <hugo.blanco@sixe.eu> - 8.3-1.librepower
- Initial AIX port for LibrePower
