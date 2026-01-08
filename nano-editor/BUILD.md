# Building GNU nano for AIX

This document describes how to compile GNU nano 8.3 for AIX 7.3.

## Prerequisites

### Required Packages (from AIX Toolbox)

```bash
dnf install gcc ncurses ncurses-devel xz
```

### Environment Setup

```bash
export PATH=/opt/freeware/bin:$PATH
export CC=gcc
export CFLAGS="-maix64 -O2"
export LDFLAGS="-L/opt/freeware/lib"
export CPPFLAGS="-I/opt/freeware/include"
export OBJECT_MODE=64
export HOME=/root
```

> **CRITICAL:** The `OBJECT_MODE=64` variable is essential for AIX 64-bit compilation. 
> Without it, the `ar` command will fail with "not valid with the current object file mode" errors.

## Download Source

```bash
cd /tmp
curl -LO https://www.nano-editor.org/dist/v8/nano-8.3.tar.xz

# Extract (AIX tar syntax - NOT tar -xzf!)
xz -dc nano-8.3.tar.xz | tar -xf -
cd nano-8.3
```

## Configure

```bash
./configure --prefix=/opt/freeware --disable-nls --disable-libmagic
```

**Notes:**
- `--disable-nls`: Disables internationalization (avoids gettext dependency)
- `--disable-libmagic`: Disables file type detection via libmagic (avoids header issues on AIX)

## Build

```bash
# Touch doc files to prevent texinfo rebuild requirement
touch doc/nano.1 doc/rnano.1 doc/nanorc.5
touch doc/nano.html doc/nano.info 2>/dev/null || true
echo "" > doc/nano.html 2>/dev/null || true

# Compile with parallel jobs (12 cores / 48 threads available)
make -j12
```

> **TIP:** Use `make -j12` or `make -j48` for significantly faster builds on multi-core AIX systems.

## Verify Build

```bash
./src/nano --version
# Should output: GNU nano, version 8.3
```

## Install (for testing)

```bash
make install
```

## Create RPM Package

### Copy Source to RPM Build Directory

```bash
cp /tmp/nano-8.3.tar.xz /opt/freeware/src/packages/SOURCES/
```

### Create SPEC File

Copy `nano.spec` from the SPECS directory to `/opt/freeware/src/packages/SPECS/`

The SPEC file includes:
- Global nanorc configuration with syntax highlighting enabled by default
- Smooth scrolling and auto-indent defaults

### Build RPM

```bash
cd /opt/freeware/src/packages/SPECS
rpmbuild -bb nano.spec
```

Output will be in: `/opt/freeware/src/packages/RPMS/ppc/nano-8.3-3.librepower.aix7.3.ppc.rpm`

## AIX-Specific Issues Encountered

### Issue 1: ar "object file mode" Error

**Symptom:**
```
ar: 0707-126 libgnu_a-xxx.o is not valid with the current object file mode.
        Use the -X option to specify the desired object mode.
```

**Root Cause:** AIX `ar` command defaults to 32-bit object mode, but GCC with `-maix64` produces 64-bit objects.

**Solution:** Set `OBJECT_MODE=64` before running configure and make:
```bash
export OBJECT_MODE=64
```

### Issue 2: texinfo/makeinfo Required

**Symptom:**
```
make[2]: *** [Makefile:2310: nano.html] Error 1
```

**Root Cause:** nano's Makefile tries to regenerate documentation using texinfo/makeinfo, which isn't installed.

**Solution:** Touch the doc files before make to prevent regeneration:
```bash
touch doc/nano.1 doc/rnano.1 doc/nanorc.5
touch doc/nano.html doc/nano.info
echo "" > doc/nano.html
```

### Issue 3: libmagic Header Issues

**Symptom:**
```
color.c:195:47: warning: implicit declaration of function 'magic_load'
```

**Root Cause:** libmagic headers on AIX don't match what nano expects.

**Solution:** Configure with `--disable-libmagic`. This only disables automatic file type detection, which isn't critical functionality.

### Issue 4: C compiler cannot create executables

**Symptom:**
```
configure: error: C compiler cannot create executables
```

**Root Cause:** Stale cache from previous configure attempts with different settings.

**Solution:** Clean and start fresh:
```bash
make distclean
rm -f config.cache config.status
./configure [options]
```

## Dependencies

The compiled binary depends on:
- libc.a(shr_64.o) - AIX base
- libpthreads.a(shr_xpg5_64.o) - AIX base
- libncursesw.a(libncursesw.so.6) - AIX Toolbox

All dependencies are standard AIX/Toolbox libraries.

## Verification

```bash
# Check file type
file /opt/freeware/bin/nano
# Should show: 64-bit XCOFF executable

# Check dependencies
export OBJECT_MODE=64
dump -H /opt/freeware/bin/nano | head -20

# Test execution
/opt/freeware/bin/nano --version
```

## Files Installed

```
/opt/freeware/bin/nano           - Main binary
/opt/freeware/bin/rnano          - Restricted mode symlink
/opt/freeware/etc/nanorc         - Global config (syntax highlighting enabled)
/opt/freeware/share/nano/*.nanorc - Syntax highlighting files (40+)
/opt/freeware/share/man/man1/nano.1
/opt/freeware/share/man/man1/rnano.1
/opt/freeware/share/man/man5/nanorc.5
```

---

**Built by:** LibrePower  
**Project:** LibrePower - https://librepower.org
