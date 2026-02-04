# librepower-compat

AIX 7.1/7.2 compatibility shim for LibrePower packages.

## Why is this needed?

Some packages (nano, mariadb11) compiled on AIX 7.3 use the `single_locale` 
symbol from libc, which doesn't exist on AIX 7.1 and 7.2.

This package provides a safe shim library that supplies this symbol.

## Installation

```bash
dnf install librepower-compat
```

## Usage

On AIX 7.1 or 7.2, set `LDR_PRELOAD64` before running affected programs:

```bash
export LDR_PRELOAD64="/opt/freeware/lib/librepower/libcompat.a(shr_64.o)"
nano file.txt
```

Or for a single command:

```bash
LDR_PRELOAD64="/opt/freeware/lib/librepower/libcompat.a(shr_64.o)" nano file.txt
```

## Affected Packages

| Package | AIX 7.1 | AIX 7.2 | AIX 7.3 |
|---------|---------|---------|---------|
| nano | Needs shim | Needs shim | Native |
| mariadb11 | Not supported* | Needs shim | Native |

*mariadb11 has AIX 7.1 runtime incompatibility (not hardware - tested on POWER9).
Binary uses AIX 7.3 runtime features unavailable in AIX 7.1 kernel.

## Safety

- Installs to `/opt/freeware/lib/librepower/` (own directory)
- Does NOT modify system libraries
- Does NOT conflict with IBM AIX Toolbox
- Preserves existing `LDR_PRELOAD64` settings

## Add to shell profile (optional)

For convenience, add to `~/.profile` or `~/.bashrc`:

```bash
# LibrePower compatibility for AIX 7.1/7.2
_aix_ver=$(oslevel 2>/dev/null | cut -d. -f1,2)
if [[ "$_aix_ver" < "7.3" ]] && [[ -f "/opt/freeware/lib/librepower/libcompat.a" ]]; then
    export LDR_PRELOAD64="/opt/freeware/lib/librepower/libcompat.a(shr_64.o)${LDR_PRELOAD64:+:$LDR_PRELOAD64}"
fi
```

## License

MIT
