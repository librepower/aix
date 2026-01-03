# Building fzf for AIX

This document describes how to compile fzf for AIX. This was the first Go-based tool successfully compiled for AIX as part of LibrePower.

## Prerequisites

### ⚠️ CRITICAL: Use Go Toolchain, NOT gccgo

AIX has two Go compilers available. **Only one works:**

| Compiler | Source | Status | Notes |
|----------|--------|--------|-------|
| **Go toolchain** | go.dev | ✅ Works | Use this one |
| gccgo | AIX Toolbox | ❌ Broken | Fatal runtime errors |

**gccgo from AIX Toolbox (GCC 13.3.0) crashes with any program using goroutines:**

```
fatal error: gogo setcontext returned
[signal SIGSEGV: segmentation violation code=0x32 addr=0xbadc0ffee0ddf00d pc=0x0]
```

This is a known issue: gccgo relies on POSIX `ucontext` functions (`makecontext`, `setcontext`, `getcontext`) which were removed from POSIX.1-2008 and are broken on AIX 64-bit.

### Install Go Toolchain from go.dev

```bash
# Download Go for AIX (adjust version as needed)
cd /tmp
/opt/freeware/bin/curl -k -LO https://go.dev/dl/go1.21.6.aix-ppc64.tar.gz

# Verify download (~62MB)
ls -la go1.21.6.aix-ppc64.tar.gz

# Extract to /usr/local
# NOTE: AIX tar syntax differs from GNU tar!
cd /usr/local
/opt/freeware/bin/gzip -dc /tmp/go1.21.6.aix-ppc64.tar.gz | tar -xf -

# Verify installation
/usr/local/go/bin/go version
# Output: go version go1.21.6 aix/ppc64
```

### Other Requirements

- AIX 7.1+ or VIOS 3.x
- AIX Toolbox installed (for git, curl, gzip)
- Internet access (to download dependencies)

## Build Steps

### Step 1: Set Environment

```bash
export PATH=/usr/local/go/bin:/opt/freeware/bin:$PATH
export HOME=/root
export GOPATH=/root/go
export GOTOOLCHAIN=local

# Verify Go is working
go version
```

### Step 2: Clone fzf Source

```bash
cd /tmp
rm -rf fzf

# Clone specific version (0.46.1 works with Go 1.21)
git clone --depth 1 --branch 0.46.1 https://github.com/junegunn/fzf.git

cd fzf

# Verify Go version requirement
grep "^go " go.mod
# Should show: go 1.17 (compatible with Go 1.21)
```

### Step 3: Compile

```bash
cd /tmp/fzf

# Set version info for fzf --version
export FZF_VERSION=0.46.1
export FZF_REVISION=aix-port

# Build
go build -ldflags "-s -w -X main.version=$FZF_VERSION -X main.revision=$FZF_REVISION" -o fzf

# Verify binary
ls -la fzf
file fzf
# Should show: 64-bit XCOFF executable or object module

# Test
./fzf --version
# Should show: 0.46.1 (aix-port)
```

### Step 4: Test Functionality

```bash
# Basic test
echo -e "apple\nbanana\norange" | ./fzf --filter="ora"
# Output: orange

# Performance test
time (awk 'BEGIN{for(i=1;i<=100000;i++)print i}' | ./fzf --filter="99999")
# Should complete in < 0.2 seconds
```

## Creating the RPM

### Step 1: Create SPEC File

See `SPECS/fzf.spec` in this repository for the complete spec file.

Key points:
- Use `%defattr(-,root,system,-)` (AIX uses 'system' group, not 'users')
- Include shell integration scripts from upstream (`shell/` directory)
- Add AIX-specific helper scripts (fzf-rpm, fzf-proc, fzf-svc, fzf-hist)

### Step 2: Build RPM

```bash
# Ensure rpmbuild directories exist
mkdir -p /root/rpmbuild/{SPECS,RPMS,BUILD,SOURCES,SRPMS}

# Copy spec file
cp fzf.spec /root/rpmbuild/SPECS/

# Build (binary is expected in /tmp/fzf/)
export PATH=/opt/freeware/bin:$PATH
cd /root/rpmbuild/SPECS
rpmbuild -bb fzf.spec

# Output location
ls /opt/freeware/src/packages/RPMS/ppc/fzf*.rpm
```

## Version Compatibility

| fzf Version | Minimum Go | Go 1.21.6 Compatible |
|-------------|------------|----------------------|
| 0.46.x | Go 1.17 | ✅ Yes |
| 0.47.x | Go 1.20 | ✅ Yes |
| 0.48.x+ | Go 1.21 | ✅ Yes |
| 0.55.x+ | Go 1.22 | ⚠️ May work |
| 0.60.x+ | Go 1.23 | ❌ No (needs newer Go) |

If you need a newer fzf version, first upgrade the Go toolchain from go.dev.

## Troubleshooting

### "go: command not found"

```bash
export PATH=/usr/local/go/bin:$PATH
```

### "go: cannot find main module"

Make sure you're in the fzf source directory that contains `go.mod`.

### Download fails with certificate errors

Use `-k` flag with curl to skip certificate verification:
```bash
/opt/freeware/bin/curl -k -LO https://...
```

### Build fails with "requires go >= X.Y"

The fzf version you're trying to build requires a newer Go. Either:
1. Download newer Go toolchain from go.dev
2. Use an older fzf version (0.46.1 recommended for Go 1.21)

### Runtime crash with "setcontext returned"

You accidentally used gccgo instead of the Go toolchain. Verify:
```bash
which go
# Must show: /usr/local/go/bin/go
# NOT: /opt/freeware/bin/go (this is gccgo wrapper)
```

## Why This Matters

Before this work, it was commonly believed that modern Go tools couldn't run on AIX because:

1. gccgo is broken (goroutine crashes)
2. No prebuilt binaries for AIX exist
3. Limited documentation on building Go for AIX

This build process proves that the **official Go toolchain works perfectly on AIX**. This opens the door to hundreds of modern CLI tools:

- fzf ✅ (this package)
- yq (YAML processor)
- lazygit (Git TUI)
- glow (Markdown viewer)
- hugo (Static site generator)
- gh (GitHub CLI)
- And many more...

## References

- Go AIX support: https://go.dev/wiki/AIX
- fzf repository: https://github.com/junegunn/fzf
- Go downloads: https://go.dev/dl/
- gccgo issues: https://github.com/golang/go/issues (search "AIX setcontext")

## Build Information

This package was built with:
- AIX 7.3 TL4 SP0 (7300-04-00-2546)
- Go 1.21.6 (go.dev official toolchain)
- fzf 0.46.1

Build date: January 2, 2026
Built by: SIXE - IBM Business Partner (https://sixe.eu)
Part of: LibrePower (https://librepower.org)
