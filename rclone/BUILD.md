# Building rclone for AIX

This document describes how to compile rclone for AIX from source.

## Prerequisites

### Go Toolchain

rclone requires Go 1.24+ (official toolchain from go.dev, NOT gccgo).

```bash
# Verify Go installation
/opt/freeware/bin/go version
# Should show: go version go1.24.x aix/ppc64
```

If Go is not installed:
```bash
dnf install golang
```

### Other Requirements

- AIX 7.1+ or VIOS 3.x
- Internet access (to download Go modules)
- ~500MB free space in /tmp

## Build Steps

### Step 1: Set Environment

```bash
export PATH=/opt/freeware/bin:$PATH
export GOPATH=/root/go
export HOME=/root
export CGO_ENABLED=0

# Verify
go version
```

### Step 2: Clone Source

```bash
cd /tmp
rm -rf rclone-build
mkdir rclone-build && cd rclone-build

# Clone latest stable
git clone --depth 1 https://github.com/rclone/rclone.git
cd rclone

# Check version
cat VERSION
```

### Step 3: Compile

```bash
cd /tmp/rclone-build/rclone

# Build with optimizations (strip symbols, reduce size)
go build -ldflags "-s -w" -o rclone

# Verify binary
ls -la rclone
file rclone
# Should show: 64-bit XCOFF executable or object module
```

Build time: ~4 minutes on POWER9

### Step 4: Test

```bash
./rclone version
./rclone help

# Quick functionality test
mkdir -p /tmp/test-src /tmp/test-dst
echo "test" > /tmp/test-src/file.txt
./rclone copy /tmp/test-src /tmp/test-dst
cat /tmp/test-dst/file.txt
rm -rf /tmp/test-src /tmp/test-dst
```

## Creating the RPM

### Step 1: Prepare Package Directory

```bash
mkdir -p /tmp/rclone-pkg
cp /tmp/rclone-build/rclone/rclone /tmp/rclone-pkg/
```

### Step 2: Create Spec File

Save to `/root/rpmbuild/SPECS/rclone.spec`:

```spec
Name:           rclone
Version:        1.73.0
Release:        1.librepower
Summary:        Rsync for cloud storage
License:        MIT
Group:          Applications/Internet
URL:            https://rclone.org
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

%description
Rclone is a command-line program to manage files on cloud storage.
Over 70 cloud storage products supported.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
cp /tmp/rclone-pkg/rclone %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/rclone

%files
%defattr(-,root,system,-)
/opt/freeware/bin/rclone

%post
echo "rclone installed. Run 'rclone config' to configure."

%changelog
* Sun Jan 19 2026 LibrePower <hello@librepower.org> - 1.73.0-1
- Initial AIX port
```

### Step 3: Build RPM

```bash
cd /root/rpmbuild/SPECS
rpmbuild -bb rclone.spec

# Output location
ls /opt/freeware/src/packages/RPMS/ppc/rclone*.rpm
```

## What Gets Excluded on AIX

The following features are automatically excluded when building for AIX:

| Feature | Reason |
|---------|--------|
| `mount` | AIX has no FUSE support |
| `ncdu` | Clipboard library not supported |

### Alternatives

- **Instead of mount**: Use `rclone serve nfs` and mount via standard AIX NFS
- **Instead of ncdu**: Use `rclone size` or `rclone tree`

## Troubleshooting

### "go: command not found"

```bash
export PATH=/opt/freeware/bin:$PATH
```

### Build fails with module errors

Clear module cache:
```bash
rm -rf /root/go/pkg/mod/cache
go clean -modcache
```

### Binary won't run

Verify it's 64-bit XCOFF:
```bash
file rclone
# Must show: 64-bit XCOFF executable
```

## Build Information

This package was built with:
- AIX 7.3 TL4 (7300-04-00-2546)
- Go 1.24.11 (official toolchain)
- rclone 1.73.0

Build date: January 19, 2026
Built by: LibrePower (https://librepower.org)
