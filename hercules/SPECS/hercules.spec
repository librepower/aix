Summary: IBM mainframe (S/370, ESA/390, z/Architecture) emulator for AIX
Name: hercules
Version: 4.9.1
Release: 1.librepower
License: QPL
Group: System/Emulators
URL: https://github.com/SDL-Hercules-390/hyperion
Packager: LibrePower <hello@librepower.org>

%description
Hercules is an open-source IBM mainframe emulator that runs on AIX POWER systems.
It emulates System/370, ESA/390, and z/Architecture hardware, allowing you to run
mainframe operating systems such as MVS 3.8j, VM/370, z/OS, z/VSE, and z/VM.

This is a native 64-bit AIX ppc64 build of SDL Hyperion (the actively maintained
fork of Hercules). Big-endian POWER hardware provides natural compatibility with
the big-endian z/Architecture.

Features:
  - S/370, ESA/390, and z/Architecture emulation
  - Up to 128 CPU engines
  - CKD/FBA DASD, 3420 tape, 3270 console, card reader/punch
  - HTTP server for web-based operator console
  - DASD utilities: dasdinit, dasdload, dasdcopy, dasdls, etc.
  - Tape utilities: hetinit, hetget, hetmap, tapecopy, tapemap

Note: Guest networking (CTC/LCS/QETH) is not available on AIX (no TUN/TAP).
Console access is via 3270 terminal emulator (tn3270 to port 3270).

%prep
# Pre-built from patched source on AIX

%build
# Built with GCC 13.3 -maix64 on AIX 7.3 POWER9

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/lib/hercules
mkdir -p %{buildroot}/opt/freeware/share/hercules
mkdir -p %{buildroot}/opt/freeware/share/man/man1
mkdir -p %{buildroot}/opt/freeware/share/man/man4

# Binaries
for f in /tmp/hercules-install/opt/freeware/bin/*; do
    cp "$f" %{buildroot}/opt/freeware/bin/
done

# Libraries
for f in /tmp/hercules-install/opt/freeware/lib/libherc*.a \
         /tmp/hercules-install/opt/freeware/lib/libhdt*.a; do
    cp "$f" %{buildroot}/opt/freeware/lib/
done

# Device modules
for f in /tmp/hercules-install/opt/freeware/lib/hercules/*.a; do
    cp "$f" %{buildroot}/opt/freeware/lib/hercules/
done

# HTML docs and man pages
cp -r /tmp/hercules-install/opt/freeware/share/hercules/* %{buildroot}/opt/freeware/share/hercules/ 2>/dev/null || true
cp /tmp/hercules-install/opt/freeware/share/man/man1/* %{buildroot}/opt/freeware/share/man/man1/ 2>/dev/null || true
cp /tmp/hercules-install/opt/freeware/share/man/man4/* %{buildroot}/opt/freeware/share/man/man4/ 2>/dev/null || true

%post
echo ""
echo "Hercules %{version} (SDL Hyperion) installed successfully."
echo ""
echo "Quick start:"
echo "  hercules -f your_config.cnf    # Start emulator with config file"
echo "  dasdinit -a vol.3390 3390-1 VOL001  # Create a 3390 DASD image"
echo ""
echo "Connect a 3270 terminal to localhost:3270 for console access."
echo "Note: Guest networking is not available on AIX (no TUN/TAP)."
echo ""
echo "Documentation: https://sdl-hercules-390.github.io/html/"
echo ""

%files
%defattr(-,root,system)
/opt/freeware/bin/card2txt
/opt/freeware/bin/cckd2ckd
/opt/freeware/bin/cckd642ckd
/opt/freeware/bin/cckdcdsk
/opt/freeware/bin/cckdcdsk64
/opt/freeware/bin/cckdcomp
/opt/freeware/bin/cckdcomp64
/opt/freeware/bin/cckddiag
/opt/freeware/bin/cckddiag64
/opt/freeware/bin/cckdmap
/opt/freeware/bin/cckdswap
/opt/freeware/bin/cckdswap64
/opt/freeware/bin/cfba2fba
/opt/freeware/bin/cfba642fba
/opt/freeware/bin/ckd2cckd
/opt/freeware/bin/ckd2cckd64
/opt/freeware/bin/convto64
/opt/freeware/bin/dasdcat
/opt/freeware/bin/dasdconv
/opt/freeware/bin/dasdconv64
/opt/freeware/bin/dasdcopy
/opt/freeware/bin/dasdcopy64
/opt/freeware/bin/dasdinit
/opt/freeware/bin/dasdinit64
/opt/freeware/bin/dasdisup
/opt/freeware/bin/dasdlist
/opt/freeware/bin/dasdload
/opt/freeware/bin/dasdload64
/opt/freeware/bin/dasdls
/opt/freeware/bin/dasdpdsu
/opt/freeware/bin/dasdseq
/opt/freeware/bin/dasdser
/opt/freeware/bin/dmap2hrc
/opt/freeware/bin/fba2cfba
/opt/freeware/bin/fba2cfba64
/opt/freeware/bin/herclin
/opt/freeware/bin/hercules
/opt/freeware/bin/hetget
/opt/freeware/bin/hetinit
/opt/freeware/bin/hetmap
/opt/freeware/bin/hetupd
/opt/freeware/bin/maketape
/opt/freeware/bin/tapecopy
/opt/freeware/bin/tapemap
/opt/freeware/bin/tapesplt
/opt/freeware/bin/tfprint
/opt/freeware/bin/tfswap
/opt/freeware/bin/txt2card
/opt/freeware/bin/vmfplc2
/opt/freeware/lib/libherc.a
/opt/freeware/lib/libhercd.a
/opt/freeware/lib/libhercs.a
/opt/freeware/lib/libherct.a
/opt/freeware/lib/libhercu.a
/opt/freeware/lib/libhdt3420_not_mod.a
%dir /opt/freeware/lib/hercules
/opt/freeware/lib/hercules/*.a
/opt/freeware/share/hercules
/opt/freeware/share/man/man1/*
/opt/freeware/share/man/man4/*

%changelog
* Thu Mar 20 2026 LibrePower <hello@librepower.org> - 4.9.1-1.librepower
- Initial AIX port of SDL Hercules Hyperion 4.9.1
- Native 64-bit ppc64 XCOFF build
- Patched for AIX: symbol renames, TUN/TAP stubs, networking header conflicts
- Guest networking disabled (AIX has no TUN/TAP support)
- All DASD/tape utilities included
