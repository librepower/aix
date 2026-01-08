# Building RPM Package for AIX

This document describes how to build the C-Sentinel RPM package for AIX 7.1/7.2/7.3.

## Prerequisites

On your AIX build system, ensure you have:

```bash
# Install build tools
dnf install rpm-build gcc make

# Verify installations
which rpmbuild
/opt/freeware/bin/gcc --version
```

## Building the RPM

### Method 1: Using the build script (recommended)

```bash
# Clone the repository
git clone https://github.com/librepower/c-sentinel4aix.git
cd c-sentinel4aix

# Run the build script
chmod +x build-rpm.sh
./build-rpm.sh
```

### Method 2: Manual build

```bash
# Create RPM build directories
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Or use system-wide directory
mkdir -p /opt/freeware/src/packages/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Create source tarball
mkdir -p /tmp/rpm-build/csentinel4aix-1.0.0
cp -r src include Makefile.aix README.md README.AIX.md AIX_PORT_STATUS.md LICENSE \
      /tmp/rpm-build/csentinel4aix-1.0.0/

cd /tmp/rpm-build
/opt/freeware/bin/tar czf /opt/freeware/src/packages/SOURCES/csentinel4aix-1.0.0.tar.gz \
      csentinel4aix-1.0.0

# Copy spec file
cp csentinel4aix.spec /opt/freeware/src/packages/SPECS/

# Build RPM
cd /opt/freeware/src/packages/SPECS
rpmbuild -ba csentinel4aix.spec
```

## Build Output

Successful build will produce:

```
/opt/freeware/src/packages/RPMS/ppc/csentinel4aix-1.0.0-1.aix7.3.ppc.rpm   (Binary RPM)
/opt/freeware/src/packages/SRPMS/csentinel4aix-1.0.0-1.src.rpm              (Source RPM)
```

## Installing the RPM

```bash
# Install
rpm -ivh csentinel4aix-1.0.0-1.aix7.3.ppc.rpm

# Verify installation
rpm -qi csentinel4aix
sentinel -q -n

# Check installed files
rpm -ql csentinel4aix
```

## Package Contents

The RPM installs:

```
/opt/freeware/bin/sentinel           - Main binary
/opt/freeware/bin/sentinel-diff      - Diff tool
/etc/sentinel/config              - Configuration file
/usr/share/doc/csentinel/         - Documentation
/var/lib/sentinel/                - Data directory
```

## Post-Installation

The package automatically:

1. Creates `/var/lib/sentinel` directory for baseline storage
2. Creates default configuration in `/etc/sentinel/config`
3. Attempts to create SRC subsystem for background monitoring (optional)

## Usage After Installation

```bash
# Quick analysis
sentinel -q -n

# Learn baseline
sentinel -l -n

# Compare against baseline
sentinel -b -q -n

# Continuous monitoring (optional - SRC service)
startsrc -s csentinel
lssrc -s csentinel
stopsrc -s csentinel
```

## Uninstalling

```bash
# Remove package
rpm -e csentinel4aix

# Clean up data (optional)
rm -rf /var/lib/sentinel
```

## Troubleshooting

### Build Issues

**Error: "gcc not found"**
```bash
dnf install gcc
```

**Error: "rpmbuild not found"**
```bash
dnf install rpm-build
```

**Error: "libperfstat not found"**
```bash
# This is a system library, should be present by default
# If missing, install from AIX installation media:
installp -a -d /dev/cd0 bos.perf.libperfstat
```

### Installation Issues

**Error: "SRC subsystem creation failed"**

This is not critical - the package will still work. The error occurs if:
- System Resource Controller is not available
- User doesn't have permission to create subsystems

You can still run sentinel manually without the SRC service.

## Creating Repository for DNF

To add this RPM to your AIX repository:

```bash
# Copy RPM to repository directory
cp csentinel4aix-1.0.0-1.aix7.3.ppc.rpm /path/to/your/repo/

# Create/update repository metadata
cd /path/to/your/repo
createrepo .

# Users can then install with:
dnf install csentinel4aix
```

## Package Information

- **Name:** csentinel4aix
- **Version:** 1.0.0
- **Release:** 1
- **Architecture:** ppc (POWER)
- **Size:** ~90 KB (compressed), ~230 KB (installed)
- **License:** MIT
- **Dependencies:**
  - libc.a(shr_64.o)
  - libperfstat.a(shr_64.o)

## Compatibility

- AIX 7.1 (POWER7+)
- AIX 7.2 (POWER7+)
- AIX 7.3 (POWER8+)

Built with `-maix64` for maximum compatibility across POWER architectures.

## Support

- **GitHub:** https://github.com/librepower/c-sentinel4aix
- **Issues:** https://github.com/librepower/c-sentinel4aix/issues
- **Documentation:** See README.AIX.md and AIX_PORT_STATUS.md
